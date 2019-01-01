#include "bstream.h"
#include "crt.h"
#include "converter.h"

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285
// 256 literals, the end code, some length codes, and 2 unused codes
#define NUM_DEFLATE_CODE_SYMBOLS 288
// the distance codes have their own symbols, 30 used, 2 unused
#define NUM_DISTANCE_SYMBOLS 32
// the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros
#define NUM_CODE_LENGTH_CODES 19
// the base lengths represented by codes 257-285
static const unsigned LENGTHBASE[29] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
	67, 83, 99, 115, 131, 163, 195, 227, 258
};
// the extra bits used by codes 257-285 (added to base length)
static const unsigned LENGTHEXTRA[29] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
	4, 4, 4, 4, 5, 5, 5, 5, 0
};
// the base backwards distances (the bits of distance codes appear after length codes and use their own huffman tree)
static const unsigned DISTANCEBASE[30] = {
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
	769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};
// the extra bits of backwards distances (added to base)
static const unsigned DISTANCEEXTRA[30] = {
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
	8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};
// the order in which "code length alphabet code lengths" are stored, out of this
// the huffman tree of the dynamic huffman tree lengths is generated
static const unsigned CLCL_ORDER[NUM_CODE_LENGTH_CODES] = {
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

struct zip_huffman_tree {
	unsigned tree2d[NUM_DEFLATE_CODE_SYMBOLS * 2];
	unsigned tree1d[NUM_DEFLATE_CODE_SYMBOLS];
	unsigned lengths[NUM_DEFLATE_CODE_SYMBOLS]; /*the lengths of the codes of the 1d-tree*/
	unsigned maxbitlen; /*maximum number of bits a single code can get*/
	unsigned numcodes; /*number of symbols in the alphabet = number of codes*/
	zip_huffman_tree() : maxbitlen(0), numcodes(0) {
	}
};

static unsigned read(bstream& in, unsigned nbits) {
	unsigned result = 0;
	for(unsigned i = 0; i < nbits; i++) {
		result += ((unsigned)((in.data[in.pos >> 3] >> (in.pos & 7)) & 1)) << i;
		in.pos++;
	}
	return result;
}

static unsigned huffman_decode_symbol(bstream& in, const zip_huffman_tree& tree) {
	unsigned treepos = 0, ct;
	while(true) {
		if(in.pos >= in.maximum) {
			assert(0);
			return -1; // error: the bit pointer is or will go past the memory
		}
		// decode the symbol from the tree. The "readBitFromStream" code is inlined in
		// the expression below because this is the biggest bottleneck while decoding
		ct = tree.tree2d[(treepos << 1) + ((in.data[in.pos >> 3] >> (in.pos & 0x7)) & (unsigned char)1)];
		in.pos++;
		if(ct < tree.numcodes)
			return ct; // the symbol is decoded, return it
		else
			treepos = ct - tree.numcodes; // symbol not yet decoded, instead move tree position
		if(treepos >= tree.numcodes) {
			assert(0);
			return -1; // error: it appeared outside the codetree
		}
	}
}

/*the tree representation used by the decoder. return value is error*/
static bool huffman_tree_make2D_tree(zip_huffman_tree& tree) {
	unsigned nodefilled = 0; /*up to which node it is filled*/
	unsigned treepos = 0; /*position in the tree (1 of the numcodes columns)*/
	unsigned n, i;

	assert((tree.numcodes * 2) <= (sizeof(tree.tree2d) / sizeof(tree.tree2d[0])));
	/*
	convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means
	uninited, a value >= numcodes is an address to another bit, a value < numcodes
	is a code. The 2 rows are the 2 possible bit values (0 or 1), there are as
	many columns as codes - 1.
	A good huffmann tree has N * 2 - 1 nodes, of which N - 1 are internal nodes.
	Here, the internal nodes are stored (what their 0 and 1 option point to).
	There is only memory for such good tree currently, if there are more nodes
	(due to too long length codes), error 55 will happen
	*/
	for(n = 0; n < tree.numcodes * 2; n++)
		tree.tree2d[n] = 32767; /*32767 here means the tree2d isn't filled there yet*/

	for(n = 0; n < tree.numcodes; n++) /*the codes*/
	{
		for(i = 0; i < tree.lengths[n]; i++) /*the bits for this code*/
		{
			unsigned char bit = (unsigned char)((tree.tree1d[n] >> (tree.lengths[n] - i - 1)) & 1);
			if(treepos > tree.numcodes - 2)
				return false; // oversubscribed, see comment in lodepng_error_text
			if(tree.tree2d[2 * treepos + bit] == 32767) // not yet filled in
			{
				if(i + 1 == tree.lengths[n]) /*last bit*/
				{
					tree.tree2d[2 * treepos + bit] = n; /*put the current code in it*/
					treepos = 0;
				} else {
					/*put address of the next step in here, first that address has to be found of course
					(it's just nodefilled + 1)...*/
					nodefilled++;
					/*addresses encoded with numcodes added to it*/
					tree.tree2d[2 * treepos + bit] = nodefilled + tree.numcodes;
					treepos = nodefilled;
				}
			} else
				treepos = tree.tree2d[2 * treepos + bit] - tree.numcodes;
		}
	}

	// remove possible remaining 32767's
	for(n = 0; n < tree.numcodes * 2; n++) {
		if(tree.tree2d[n] == 32767)
			tree.tree2d[n] = 0;
	}
	return true;
}

// This algoritm use stack variable, so get it in separate function
static void huffman_tree_makes_from_lenght_step_by_steps(zip_huffman_tree& tree) {
	unsigned blcount[32];
	unsigned nextcode[32];
	assert((tree.maxbitlen + 1) <= sizeof(blcount) / sizeof(blcount[0]));
	memset(blcount, 0, (tree.maxbitlen + 1) * sizeof(unsigned));
	memset(nextcode, 0, (tree.maxbitlen + 1) * sizeof(unsigned));
	// step 1: count number of instances of each code length
	for(unsigned bits = 0; bits < tree.numcodes; bits++)
		blcount[tree.lengths[bits]]++;
	// step 2: generate the nextcode values
	for(unsigned bits = 1; bits <= tree.maxbitlen; bits++)
		nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1;
	/*step 3: generate all the codes*/
	for(unsigned n = 0; n < tree.numcodes; n++) {
		if(tree.lengths[n] != 0)
			tree.tree1d[n] = nextcode[tree.lengths[n]]++;
	}
}

static bool huffman_tree_makes_from_lenght2(zip_huffman_tree& tree) {
	assert((tree.numcodes) <= (sizeof(tree.tree1d) / sizeof(tree.tree1d[0])));
	huffman_tree_makes_from_lenght_step_by_steps(tree);
	return huffman_tree_make2D_tree(tree);
}

static bool huffman_tree_makes_from_lenght(zip_huffman_tree& tree, unsigned* bitlen, unsigned numcodes, unsigned maxbitlen) {
	unsigned i;
	assert(numcodes <= sizeof(tree.lengths) / sizeof(tree.lengths[0]));
	for(i = 0; i < numcodes; i++)
		tree.lengths[i] = bitlen[i];
	tree.numcodes = numcodes; // number of symbols*/
	tree.maxbitlen = maxbitlen;
	return huffman_tree_makes_from_lenght2(tree);
}

static bool get_tree_inflate_dynamic(bstream& in, zip_huffman_tree& tree_ll, zip_huffman_tree& tree_d) {
	// make sure that length values that aren't filled in will be 0
	// or a wrong tree will be generated
	unsigned n, i;

	// see comments in deflateDynamic for explanation of the context and these variables
	// it is analogous
	// code length code lengths ("clcl"), the bit lengths of the huffman tree used to compress
	// bitlen_ll and bitlen_d
	zip_huffman_tree tree_cl; // the code tree for code length codes (the huffman tree for compressed huffman trees)

	if(in.pos >= in.maximum) {
		assert(0);
		return false; // error: the bit pointer is or will go past the memory
	}

	// number of literal/length codes + 257 so it's maximum (257-289)
	// Unlike the spec, the value 257 is added to it here already
	unsigned HLIT = read(in, 5) + 257;
	// number of distance codes. Unlike the spec, the value 1 is added to it here already (1-32)
	unsigned HDIST = read(in, 5) + 1;
	// number of code length codes. Unlike the spec, the value 4 is added to it here already (4-20)
	unsigned HCLEN = read(in, 4) + 4;

	// read the code length codes out of 3 * (amount of code length codes) bits

	// TODO: uncomment
	unsigned bitlen_cl[NUM_CODE_LENGTH_CODES];
	for(i = 0; i < NUM_CODE_LENGTH_CODES; i++) {
		if(i < HCLEN)
			bitlen_cl[CLCL_ORDER[i]] = read(in, 3);
		else
			bitlen_cl[CLCL_ORDER[i]] = 0; // if not, it must stay 0
	}

	if(!huffman_tree_makes_from_lenght(tree_cl, bitlen_cl, NUM_CODE_LENGTH_CODES, 7))
		return false;

	// now we can use this tree to read the lengths for the tree that this function will return*/
	unsigned bitlen_ll[NUM_DEFLATE_CODE_SYMBOLS] = {0};
	unsigned bitlen_d[NUM_DISTANCE_SYMBOLS] = {0};

	/*i is the current symbol we're reading in the part that contains the code lengths of lit/len and dist codes*/
	i = 0;
	while(i < HLIT + HDIST) {
		unsigned code = huffman_decode_symbol(in, tree_cl);
		if(code <= 15) //a length code
		{
			if(i < HLIT)
				bitlen_ll[i] = code;
			else
				bitlen_d[i - HLIT] = code;
			i++;
		} else if(code == 16) // repeat previous
		{
			unsigned replength = 3; // read in the 2 bits that indicate repeat length (3-6)
			unsigned value; // set value to the previous code
			if(in.pos >= in.maximum)
				return false; // error: the bit pointer is or will go past the memory
			if(i == 0)
				return false; // can't repeat previous if i is 0*/
			replength += read(in, 2);
			if(i < HLIT + 1)
				value = bitlen_ll[i - 1];
			else
				value = bitlen_d[i - HLIT - 1];
			// repeat this value in the next lengths
			for(n = 0; n < replength; n++) {
				if(i >= HLIT + HDIST)
					return false;
				if(i < HLIT)
					bitlen_ll[i] = value;
				else
					bitlen_d[i - HLIT] = value;
				i++;
			}
		} else if(code == 17) // repeat "0" 3-10 times
		{
			unsigned replength = 3; // read in the bits that indicate repeat length
			if(in.pos >= in.maximum)
				return false; // error: the bit pointer is or will go past the memory
			replength += read(in, 3);
			// repeat this value in the next lengths
			for(n = 0; n < replength; n++) {
				if(i >= HLIT + HDIST)
					return false;
				if(i < HLIT)
					bitlen_ll[i] = 0;
				else
					bitlen_d[i - HLIT] = 0;
				i++;
			}
		} else if(code == 18) // repeat "0" 11-138 times
		{
			unsigned replength = 11; /*read in the bits that indicate repeat length*/
			if(in.pos >= in.maximum)
				return false; // error: the bit pointer is or will go past the memory
			replength += read(in, 7);
			// repeat this value in the next lengths
			for(n = 0; n < replength; n++) {
				if(i >= HLIT + HDIST)
					return false;
				if(i < HLIT)
					bitlen_ll[i] = 0;
				else
					bitlen_d[i - HLIT] = 0;
				i++;
			}
		} else
			return false;
	}

	if(bitlen_ll[256] == 0)
		return false; // the length of the end code 256 must be larger than 0

	// now we've finally got HLIT and HDIST, so generate the code trees, and the function is done
	if(!huffman_tree_makes_from_lenght(tree_ll, bitlen_ll, NUM_DEFLATE_CODE_SYMBOLS, 15))
		return false;
	return huffman_tree_makes_from_lenght(tree_d, bitlen_d, NUM_DISTANCE_SYMBOLS, 15);
}

/*get the literal and length code tree of a deflated block with fixed tree, as per the deflate specification*/
static bool generate_fixed_litlen_tree(zip_huffman_tree& tree) {
	unsigned bitlen[NUM_DEFLATE_CODE_SYMBOLS];
	/*288 possible codes: 0-255=literals, 256=endcode, 257-285=lengthcodes, 286-287=unused*/
	for(unsigned i = 0; i <= 143; i++)
		bitlen[i] = 8;
	for(unsigned i = 144; i <= 255; i++)
		bitlen[i] = 9;
	for(unsigned i = 256; i <= 279; i++)
		bitlen[i] = 7;
	for(unsigned i = 280; i <= 287; i++)
		bitlen[i] = 8;
	if(!huffman_tree_makes_from_lenght(tree, bitlen, NUM_DEFLATE_CODE_SYMBOLS, 15))
		return false;
	return true;
}

/*get the distance code tree of a deflated block with fixed tree, as specified in the deflate specification*/
static bool generate_fixed_distance_tree(zip_huffman_tree& tree) {
	unsigned bitlen[NUM_DISTANCE_SYMBOLS];
	/*there are 32 distance codes, but 30-31 are unused*/
	for(unsigned i = 0; i < NUM_DISTANCE_SYMBOLS; i++)
		bitlen[i] = 5;
	if(!huffman_tree_makes_from_lenght(tree, bitlen, NUM_DISTANCE_SYMBOLS, 15))
		return false;
	return true;
}

static bool get_tree_inflate_fixed(bstream& in, zip_huffman_tree& tree_ll, zip_huffman_tree& tree_d) {
	if(!generate_fixed_litlen_tree(tree_ll))
		return false;
	if(!generate_fixed_distance_tree(tree_d))
		return false;
	return true;
}

// Inflate a block with dynamic of fixed Huffman tree
static unsigned char* inflate_huffman_block(unsigned char* pout, unsigned char* p, bstream& in, unsigned type) {
	zip_huffman_tree tree_ll; // the huffman tree for literal and length codes
	zip_huffman_tree tree_d; // the huffman tree for distance codes
	switch(type) {
	case 1:
		if(!get_tree_inflate_fixed(in, tree_ll, tree_d))
			return 0;
		break;
	case 2:
		if(!get_tree_inflate_dynamic(in, tree_ll, tree_d))
			return 0;
		break;
	default:
		assert(0);
		// Error: invalid type
		return 0;
	}
	// decode all symbols until end reached, breaks at end code
	while(true) {
		// code_ll is literal, length or end code
		unsigned code_ll = huffman_decode_symbol(in, tree_ll);
		if(code_ll <= 255) // literal symbol
		{
			if(pout)
				*p = (unsigned char)(code_ll); // WRITE to output
			p++;
		} else if(code_ll >= FIRST_LENGTH_CODE_INDEX && code_ll <= LAST_LENGTH_CODE_INDEX) /*length code*/
		{
			unsigned code_d, distance;
			unsigned start, forward, backward;

			/*part 1: get length base*/
			unsigned length = LENGTHBASE[code_ll - FIRST_LENGTH_CODE_INDEX];

			/*part 2: get extra bits and add the value of that to length*/
			unsigned numextrabits_l = LENGTHEXTRA[code_ll - FIRST_LENGTH_CODE_INDEX];
			if(!in)
				return 0; //ErrorReadAfterEnd;
			length += read(in, numextrabits_l);

			/*part 3: get distance code*/
			code_d = huffman_decode_symbol(in, tree_d);
			if(code_d > 29)
				return 0; // error: invalid distance code (30-31 are never used)
			distance = DISTANCEBASE[code_d];

			/*part 4: get extra bits from distance*/
			unsigned numextrabits_d = DISTANCEEXTRA[code_d]; /*extra bits for length and distance*/
			if(in.pos >= in.maximum)
				return 0;
			distance += read(in, numextrabits_d);

			/*part 5: fill in all the out[n] values based on the length and dist*/
			start = p - pout;
			if(distance > start)
				return 0; //ErrorReadBeforeBegin
			backward = start - distance;
			for(forward = 0; forward < length; forward++) {
				if(pout)
					*p = pout[backward++]; // WRITE to output
				p++;
				if(backward >= start)
					backward = start - distance;
			}
		} else if(code_ll == 256)
			break; /*end code, break the loop*/
		else {
			assert(0);
			/*return error code 10 or 11 depending on the situation that happened in huffmanDecodeSymbol
			(10=no endcode, 11=wrong jump outside of tree)*/
			return 0;
		}
	}
	return p;
}

static unsigned char* inflate_no_compression(unsigned char* pout, unsigned char* p, bstream& in) {
	/*go to first boundary of byte*/
	while((in.pos & 0x7) != 0)
		in.pos++;
	unsigned p1 = in.pos / 8; /*byte position*/
	unsigned len = in.data[p1] + 256 * in.data[p1 + 1]; p1 += 2;
	unsigned nlen = in.data[p1] + 256 * in.data[p1 + 1]; p1 += 2;
	/*check if 16-bit NLEN is really the one's complement of LEN*/
	if(len + nlen != 65535)
		return 0; /*error: NLEN is not one's complement of LEN*/
	for(unsigned n = 0; n < len; n++) {
		if(pout)
			*p = in.data[p1++]; // WRITE to output
		p++;
	}
	in.pos = p1 * 8;
	return p;
}

static int decode_stream(unsigned char* output, const unsigned char* input, int input_size) {
	bstream binp((unsigned char*)input, input_size);
	int	stop_bit = 0;
	unsigned char* p = output;
	// run until we get stop bit
	while(!stop_bit) {
		if(binp.pos + 2 >= binp.maximum)
			return false;
		stop_bit = read(binp, 1);
		int type = read(binp, 2);
		switch(type) {
		case 0:
			p = inflate_no_compression(output, p, binp);
			break;
		case 1:
		case 2:
			p = inflate_huffman_block(output, p, binp, type);
			break;
		default:
			p = 0;
			break;
		}
		if(!p)
			return 0;
	}
	return p - output;
}

static int decode_deflate(unsigned char* output, const unsigned char* input, int input_size) {
	if(input_size < 2) {
		// error: too small size
		assert(0);
		return 0;
	}
	// read information from zlib header
	if((input[0] * 256 + input[1]) % 31 != 0) {
		assert(0);
		// error: 256 * in[0] + in[1] must be a multiple of 31
		// the FCHECK value is supposed to be made that way
		return 0;
	}
	unsigned CM = input[0] & 15;
	unsigned CINFO = (input[0] >> 4) & 15;
	unsigned FDICT = (input[1] >> 5) & 1;
	//unsigned FLEVEL = (input[1] >> 6) & 3;
	if(CM != 8 || CINFO > 7) {
		assert(0);
		// error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec
		return 0;
	}
	if(FDICT != 0) {
		assert(0);
		/*error: the specification of PNG says about the zlib stream:
		"The additional flags shall not specify a preset dictionary."*/
		return 0;
	}
	return decode_stream(output, input + 2, input_size - 2);
}

static struct zip_plugin : public converter {

	zip_plugin() : converter("zip") {
	}

	unsigned decode(void* output, int output_size, const void* input, int input_size) const override {
		return decode_deflate((unsigned char*)output, (const unsigned char*)input, input_size);
	}

	unsigned encode(void* output, int output_size, const void* input, int input_size) const override {
		return 0;
	}

} instance;