#include "crt.h"
#include "converter.h"
#include "draw.h"

enum colortypes {
	PngColorGrey = 0, // greyscale: 1,2,4,8,16 bit
	PngColorRGB = 2, // RGB: 8,16 bit
	PngColorPalette = 3, // palette: 1,2,4,8 bit
	PngColorGreyAlpha = 4, // greyscale with alpha: 8,16 bit
	PngColorRGBA = 6 // RGB with alpha: 8,16 bit
};

static int getNumColorChannels(colortypes colortype) {
	switch(colortype) {
	case 0:
		return 1; /*grey*/
	case 2:
		return 3; /*RGB*/
	case 3:
		return 1; /*palette*/
	case 4:
		return 2; /*grey + alpha*/
	case 6:
		return 4; /*RGBA*/
	}
	return 0; /*unexisting color type*/
}

static int get_bpp(colortypes colortype, unsigned bit_depth) {
	/*bits per pixel is amount of channels * bits per channel*/
	return getNumColorChannels(colortype) * bit_depth;
}

static unsigned get_raw_size(unsigned w, unsigned h, const colortypes color, int bit_depth) {
	return (w * (h + 1) * get_bpp(color, bit_depth) + 7) / 8;
}

static int read32(const unsigned char* p) {
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

static unsigned get_chunk_length(const unsigned char* chunk) {
	return read32(chunk);
}

static const unsigned char* get_chunk_data(const unsigned char* chunk) {
	return chunk + 8;
}

static const unsigned char* get_chunk_next(const unsigned char* chunk) {
	unsigned total_chunk_length = get_chunk_length(chunk) + 12;
	return &chunk[total_chunk_length];
}

static bool chunk_type_equals(const unsigned char* chunk, const char* type) {
	return (chunk[4] == type[0]
		&& chunk[5] == type[1]
		&& chunk[6] == type[2]
		&& chunk[7] == type[3]);
}

static int read_idat(unsigned char* out, const unsigned char* p, const unsigned char* pe, const unsigned char** single_input) {
	int size = 0;
	int blocks = 0;
	if(single_input)
		*single_input = 0;
	/*loop through the chunks, ignoring unknown chunks and stopping at IEND chunk.
	IDAT data is put at the start of the in buffer*/
	while(p < pe) {
		unsigned p_size = pe - p;
		/*error: size of the in buffer too small to contain next chunk*/
		if(p_size < 12)
			return 0;
		/*length of the data of the chunk, excluding the length bytes, chunk type and CRC bytes*/
		unsigned chunk_length = get_chunk_length(p);
		if(chunk_length > 2147483647)
			return 0; //error: chunk length larger than the max PNG chunk size
		else if(p_size < 12 + chunk_length)
			return 0; // error: size of the in buffer too small to contain next chunk
		/*IDAT chunk, containing compressed image data*/
		if(chunk_type_equals(p, "IDAT")) {
			blocks++;
			if(out)
				memcpy(out + size, get_chunk_data(p), chunk_length);
			if(single_input) {
				if(blocks <= 1)
					*single_input = get_chunk_data(p);
				else
					*single_input = 0;
			}
			size += chunk_length;
		} else if(chunk_type_equals(p, "IEND"))
			break;
		p = (unsigned char*)get_chunk_next(p);
	}
	return size;
}

static unsigned char read_bit_from_reversed_stream(unsigned* bitpointer, const unsigned char* bitstream) {
	unsigned char result = (unsigned char)((bitstream[(*bitpointer) >> 3] >> (7 - ((*bitpointer) & 0x7))) & 1);
	(*bitpointer)++;
	return result;
}

static void set_bit_of_reversed_stream(unsigned* bitpointer, unsigned char* bitstream, unsigned char bit) {
	/*the current bit in bitstream may be 0 or 1 for this to work*/
	if(bit == 0)
		bitstream[(*bitpointer) >> 3] &= (unsigned char)(~(1 << (7 - ((*bitpointer) & 0x7))));
	else
		bitstream[(*bitpointer) >> 3] |= (1 << (7 - ((*bitpointer) & 0x7)));
	(*bitpointer)++;
}

static void remove_padding_bits(unsigned char* out, const unsigned char* in,
	unsigned olinebits, unsigned ilinebits, unsigned h) {
	/*
	After filtering there are still padding bits if scanlines have non multiple of 8 bit amounts. They need
	to be removed (except at last scanline of (Adam7-reduced) image) before working with pure image buffers
	for the Adam7 code, the color convert code and the output to the user.
	in and out are allowed to be the same buffer, in may also be higher but still overlapping; in must
	have >= ilinebits*h bits, out must have >= olinebits*h bits, olinebits must be <= ilinebits
	also used to move bits after earlier such operations happened, e.g. in a sequence of reduced images from Adam7
	only useful if (ilinebits - olinebits) is a value in the range 1..7
	*/
	unsigned y;
	unsigned diff = ilinebits - olinebits;
	unsigned ibp = 0, obp = 0; /*input and output bit pointers*/
	for(y = 0; y < h; y++) {
		unsigned x;
		for(x = 0; x < olinebits; x++) {
			unsigned char bit = read_bit_from_reversed_stream(&ibp, in);
			set_bit_of_reversed_stream(&obp, out, bit);
		}
		ibp += diff;
	}
}

/*
Paeth predicter, used by PNG filter type 4
The parameters are of type short, but should come from unsigned chars, the shorts
are only needed to make the paeth calculation correct.
*/
static unsigned char paethPredictor(short a, short b, short c) {
	short pa = iabs(b - c);
	short pb = iabs(a - c);
	short pc = iabs(a + b - c - c);
	if(pc < pa && pc < pb)
		return (unsigned char)c;
	else if(pb < pa)
		return (unsigned char)b;
	else
		return (unsigned char)a;
}

static bool unfilter_scanline(unsigned char* recon, const unsigned char* scanline, const unsigned char* precon,
	unsigned bytewidth, unsigned char filterType, unsigned length) {
	/*
	For PNG filter method 0
	unfilter a PNG image scanline by scanline. when the pixels are smaller than 1 byte,
	the filter works byte per byte (bytewidth = 1)
	precon is the previous unfiltered scanline, recon the result, scanline the current one
	the incoming scanlines do NOT include the filtertype byte, that one is given in the parameter filterType instead
	recon and scanline MAY be the same memory address! precon must be disjoint.
	*/

	unsigned i;
	switch(filterType) {
	case 0:
		for(i = 0; i < length; i++)
			recon[i] = scanline[i];
		break;
	case 1:
		for(i = 0; i < bytewidth; i++)
			recon[i] = scanline[i];
		for(i = bytewidth; i < length; i++)
			recon[i] = scanline[i] + recon[i - bytewidth];
		break;
	case 2:
		if(precon) {
			for(i = 0; i < length; i++)
				recon[i] = scanline[i] + precon[i];
		} else {
			for(i = 0; i < length; i++)
				recon[i] = scanline[i];
		}
		break;
	case 3:
		if(precon) {
			for(i = 0; i < bytewidth; i++)
				recon[i] = scanline[i] + precon[i] / 2;
			for(i = bytewidth; i < length; i++)
				recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
		} else {
			for(i = 0; i < bytewidth; i++)
				recon[i] = scanline[i];
			for(i = bytewidth; i < length; i++)
				recon[i] = scanline[i] + recon[i - bytewidth] / 2;
		}
		break;
	case 4:
		if(precon) {
			for(i = 0; i < bytewidth; i++)
				recon[i] = (scanline[i] + precon[i]); /*paethPredictor(0, precon[i], 0) is always precon[i]*/
			for(i = bytewidth; i < length; i++)
				recon[i] = (scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
		} else {
			for(i = 0; i < bytewidth; i++)
				recon[i] = scanline[i];
			for(i = bytewidth; i < length; i++) {
				/*paethPredictor(recon[i - bytewidth], 0, 0) is always recon[i - bytewidth]*/
				recon[i] = (scanline[i] + recon[i - bytewidth]);
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

static bool unfilter(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp) {
	/*
	For PNG filter method 0
	this function unfilters a single image (e.g. without interlacing this is called once, with Adam7 seven times)
	out must have enough bytes allocated already, in must have the scanlines + 1 filtertype byte per scanline
	w and h are image dimensions or dimensions of reduced image, bpp is bits per pixel
	in and out are allowed to be the same memory address (but aren't the same size since in has the extra filter bytes)
	*/

	unsigned y;
	unsigned char* prevline = 0;

	/*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise*/
	unsigned bytewidth = (bpp + 7) / 8;
	unsigned linebytes = (w * bpp + 7) / 8;

	for(y = 0; y < h; y++) {
		unsigned outindex = linebytes * y;
		unsigned inindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
		unsigned char filterType = in[inindex];

		if(!unfilter_scanline(&out[outindex], &in[inindex + 1], prevline, bytewidth, filterType, linebytes))
			return false;

		prevline = &out[outindex];
	}

	return true;
}

/*shared values used by multiple Adam7 related functions*/

static const unsigned ADAM7_IX[7] = {0, 4, 0, 2, 0, 1, 0}; /*x start values*/
static const unsigned ADAM7_IY[7] = {0, 0, 4, 0, 2, 0, 1}; /*y start values*/
static const unsigned ADAM7_DX[7] = {8, 8, 4, 4, 2, 2, 1}; /*x delta values*/
static const unsigned ADAM7_DY[7] = {8, 8, 8, 4, 4, 2, 2}; /*y delta values*/

static unsigned char readBitFromReversedStream(size_t* bitpointer, const unsigned char* bitstream) {
	unsigned char result = (unsigned char)((bitstream[(*bitpointer) >> 3] >> (7 - ((*bitpointer) & 0x7))) & 1);
	++(*bitpointer);
	return result;
}

static void setBitOfReversedStream0(size_t* bitpointer, unsigned char* bitstream, unsigned char bit) {
	/*the current bit in bitstream must be 0 for this to work*/
	if(bit) {
		/*earlier bit of huffman code is in a lesser significant bit of an earlier byte*/
		bitstream[(*bitpointer) >> 3] |= (bit << (7 - ((*bitpointer) & 0x7)));
	}
	++(*bitpointer);
}

/*
Outputs various dimensions and positions in the image related to the Adam7 reduced images.
passw: output containing the width of the 7 passes
passh: output containing the height of the 7 passes
filter_passstart: output containing the index of the start and end of each
reduced image with filter bytes
padded_passstart output containing the index of the start and end of each
reduced image when without filter bytes but with padded scanlines
passstart: output containing the index of the start and end of each reduced
image without padding between scanlines, but still padding between the images
w, h: width and height of non-interlaced image
bpp: bits per pixel
"padded" is only relevant if bpp is less than 8 and a scanline or image does not
end at a full byte
*/
static void Adam7_getpassvalues(unsigned passw[7], unsigned passh[7], size_t filter_passstart[8],
	size_t padded_passstart[8], size_t passstart[8], unsigned w, unsigned h, unsigned bpp) {
	/*the passstart values have 8 values: the 8th one indicates the byte after the end of the 7th (= last) pass*/
	unsigned i;

	/*calculate width and height in pixels of each pass*/
	for(i = 0; i != 7; ++i) {
		passw[i] = (w + ADAM7_DX[i] - ADAM7_IX[i] - 1) / ADAM7_DX[i];
		passh[i] = (h + ADAM7_DY[i] - ADAM7_IY[i] - 1) / ADAM7_DY[i];
		if(passw[i] == 0) passh[i] = 0;
		if(passh[i] == 0) passw[i] = 0;
	}

	filter_passstart[0] = padded_passstart[0] = passstart[0] = 0;
	for(i = 0; i != 7; ++i) {
		/*if passw[i] is 0, it's 0 bytes, not 1 (no filtertype-byte)*/
		filter_passstart[i + 1] = filter_passstart[i]
			+ ((passw[i] && passh[i]) ? passh[i] * (1 + (passw[i] * bpp + 7) / 8) : 0);
		/*bits padded if needed to fill full byte at end of each scanline*/
		padded_passstart[i + 1] = padded_passstart[i] + passh[i] * ((passw[i] * bpp + 7) / 8);
		/*only padded at end of reduced image*/
		passstart[i + 1] = passstart[i] + (passh[i] * passw[i] * bpp + 7) / 8;
	}
}

/*
in: Adam7 interlaced image, with no padding bits between scanlines, but between
reduced images so that each reduced image starts at a byte.
out: the same pixels, but re-ordered so that they're now a non-interlaced image with size w*h
bpp: bits per pixel
out has the following size in bits: w * h * bpp.
in is possibly bigger due to padding bits between reduced images.
out must be big enough AND must be 0 everywhere if bpp < 8 in the current implementation
(because that's likely a little bit faster)
NOTE: comments about padding bits are only relevant if bpp < 8
*/
static void Adam7_deinterlace(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp) {
	unsigned passw[7], passh[7];
	size_t filter_passstart[8], padded_passstart[8], passstart[8];
	unsigned i;

	Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

	if(bpp >= 8) {
		for(i = 0; i != 7; ++i) {
			unsigned x, y, b;
			size_t bytewidth = bpp / 8;
			for(y = 0; y < passh[i]; ++y)
				for(x = 0; x < passw[i]; ++x) {
					size_t pixelinstart = passstart[i] + (y * passw[i] + x) * bytewidth;
					size_t pixeloutstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * bytewidth;
					for(b = 0; b < bytewidth; ++b) {
						out[pixeloutstart + b] = in[pixelinstart + b];
					}
				}
		}
	} else /*bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers*/
	{
		for(i = 0; i != 7; ++i) {
			unsigned x, y, b;
			unsigned ilinebits = bpp * passw[i];
			unsigned olinebits = bpp * w;
			size_t obp, ibp; /*bit pointers (for out and in buffer)*/
			for(y = 0; y < passh[i]; ++y)
				for(x = 0; x < passw[i]; ++x) {
					ibp = (8 * passstart[i]) + (y * ilinebits + x * bpp);
					obp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olinebits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
					for(b = 0; b < bpp; ++b) {
						unsigned char bit = readBitFromReversedStream(&ibp, in);
						/*note that this function assumes the out buffer is completely 0, use setBitOfReversedStream otherwise*/
						setBitOfReversedStream0(&obp, out, bit);
					}
				}
		}
	}
}

// out must be buffer big enough to contain full image, and in must contain the full decompressed data from
// the IDAT chunks (with filter index bytes and possible padding bits)
// return value is error
static void postprocess_scanlines(unsigned char* out, unsigned char* in, unsigned w, unsigned h, unsigned bpp, int interlace) {
	/*
	This function converts the filtered-padded-interlaced data into pure 2D image buffer with the PNG's colortype.
	Steps:
	*) if no Adam7: 1) unfilter 2) remove padding bits (= posible extra bits per scanline if bpp < 8)
	*) if adam7: 1) 7x unfilter 2) 7x remove padding bits 3) Adam7_deinterlace
	NOTE: the in buffer will be overwritten with intermediate data!
	*/
	if(interlace == 0) {
		if(bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8) {
			unfilter(in, in, w, h, bpp);
			remove_padding_bits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
		}
		// we can immediatly filter into the out buffer, no other steps needed
		else
			unfilter(out, in, w, h, bpp);
	} else // interlace_method is 1 (Adam7)
	{
		// FIXME: В этом случае изображение выводится некорректно
		unsigned passw[7], passh[7];
		unsigned filter_passstart[8], padded_passstart[8], passstart[8];
		unsigned i;
		Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);
		for(i = 0; i < 7; i++) {
			unfilter(&in[padded_passstart[i]], &in[filter_passstart[i]], passw[i], passh[i], bpp);
			// TODO: possible efficiency improvement: if in this reduced image the bits fit nicely in 1 scanline,
			// move bytes instead of bits or move not at all
			if(bpp < 8) {
				// remove padding bits in scanlines; after this there still may be padding
				// bits between the different reduced images: each reduced image still starts nicely at a byte*/
				remove_padding_bits(&in[passstart[i]], &in[padded_passstart[i]], passw[i] * bpp,
					((passw[i] * bpp + 7) / 8) * 8, passh[i]);
			}
		}
		Adam7_deinterlace(out, in, w, h, bpp);
	}
}

static struct png_bitmap_plugin : public draw::surface::plugin {

	png_bitmap_plugin() : plugin("png", "PNG images\0*.png\0") {}

	bool decode(unsigned char* output, int output_bpp, const unsigned char* input, unsigned input_size) override {
		//return decode_test(output, input, size);
		int image_width, image_height, input_bpp, image_size;
		if(!output)
			return false;
		if(!inspect(image_width, image_height, input_bpp, input, input_size))
			return false;
		auto bpp = iabs(input_bpp);
		// Get encoder plugin
		auto zlib = converter::find("zip"); assert(zlib);
		auto colortype = (colortypes)input[25];
		auto compression = input[26];
		auto filter = input[27];
		auto interlace = input[28];
		input += 33;
		// retrevie size
		const unsigned char* single_input;
		int size_compressed = read_idat(0, input, input + input_size, &single_input);
		if(!size_compressed)
			return false;
		// decompress image data
		// try to optimize input if only one 'idat' section
		image_size = image_width * image_height*(bpp / 8);
		if(single_input) {
			if(!zlib->decode(output, image_size, single_input, size_compressed))
				return false;
		} else {
			unsigned char* ptemp = new unsigned char[size_compressed];
			if(!read_idat(ptemp, input, input + input_size, 0)) {
				delete ptemp;
				return false;
			}
			unsigned result = zlib->decode(output, image_size, ptemp, size_compressed);
			if(!result) {
				delete ptemp;
				return false;
			}
			delete ptemp;
		}
		postprocess_scanlines(output, output, image_width, image_height, bpp, interlace);
		color::convert(output, image_width, image_height, output_bpp, 0, output, input_bpp, 0);
		return true;
	}

	bool inspect(int& w, int& h, int& bpp, const unsigned char* input, unsigned size) override {
		if(input[0] != 137 || input[1] != 80 || input[2] != 78 || input[3] != 71
			|| input[4] != 13 || input[5] != 10 || input[6] != 26 || input[7] != 10)
			return false;
		if(input[12] != 'I' || input[13] != 'H' || input[14] != 'D' || input[15] != 'R')
			return false;
		colortypes colortype = colortypes(input[25]);
		int bit_depth = input[24];
		w = read32(input + 16);
		h = read32(input + 20);
		bpp = -get_bpp(colortype, bit_depth);
		return true;
	}

} instance;