#include "bsdata.h"
#include "crt.h"
#include "io.h"

class bsfile {
	const bsfile* parent;
	const char* url;
	const char* start;
public:
	bsfile(const char* url, const bsfile* parent = 0) : parent(0), url(url), start(loadt(url)) {}
	~bsfile() { delete start; }
	operator bool() const { return start != 0; }
	const char* getstart() const { return start; }
	const char* geturl() const { return url; }
};

struct bsdata_serial : bsfile {

	char			buffer[128 * 256];
	int				value;
	const bsreq*	value_type;
	void*			value_object;
	const char*		p;
	bsdata::parser*	parser;

	bsdata_serial(const char* url, const bsfile* parent = 0) : bsfile(url, parent), p(getstart()), parser(0) {
		clearvalue();
		buffer[0] = 0;
	}

	static const char* skipline(const char* p) {
		while(p[0] && p[0] != 10 && p[0] != 13)
			p++;
		return skipws(szskipcr(p));
	}

	static const char* skipws(const char* p) {
		while(*p) {
			if(p[0] == 9 || p[0] == 0x20) {
				p++;
				continue;
			} else if(p[0] == '\\') {
				p++;
				if(p[0] == 10 || p[0] == 13)
					p = szskipcr(p);
				else
					p++;
				continue;
			} else if(p[0] == '/' && p[1] == '/') {
				// Comments
				p = skipline(p + 2);
				continue;
			}
			break;
		}
		return p;
	}

	bsdata* findbase(const bsreq* type) const {
		if(parser && parser->custom) {
			for(auto p = parser->custom; *p; p++) {
				if((*p)->fields == type)
					return *p;
			}
		}
		return bsdata::find(type);
	}

	bsdata* findbase(const char* id) const {
		if(parser && parser->custom) {
			for(auto p = parser->custom; *p; p++) {
				if(strcmp((*p)->id, id)==0)
					return *p;
			}
		}
		return bsdata::find(id);
	}

	const char* getbasename(const bsreq* type) const {
		auto p = findbase(type);
		if(!p)
			return "";
		return p->id;
	}

	void getpos(const char* p, int& line, int& column) {
		line = 0;
		column = 0;
		auto ps = getstart();
		while(*ps) {
			line++;
			auto pe = skipline(ps);
			if(p >= ps && p < pe) {
				column = p - ps + 1;
				return;
			}
			ps = pe;
		}
	}

	void error(bsparse_error_s id, ...) {
		if(!parser)
			return;
		int line, column;
		getpos(p, line, column);
		parser->error(id, geturl(), line, column, xva_start(id));
		skipline();
	}

	void warning(bsparse_error_s id, ...) {
		if(!parser)
			return;
		int line, column;
		getpos(p, line, column);
		parser->error(id, geturl(), line, column, xva_start(id));
	}

	void clearvalue() {
		value = 0;
		value_type = 0;
		value_object = 0;
	}

	bool islinefeed() const {
		return *p == 13 || *p == 10;
	}

	void skip() {
		p++;
	}

	bool skip(char sym) {
		if(*p != sym)
			return false;
		p++;
		skipws();
		return true;
	}

	bool skip(const char* sym) {
		auto i = 0;
		while(sym[i]) {
			if(p[i] != sym[i])
				return false;
			i++;
		}
		p += i;
		return true;
	}

	void skipws() {
		p = skipws(p);
	}

	void skipline() {
		p = skipline(p);
	}

	void readstring(const char end) {
		auto ps = buffer;
		auto pe = ps + sizeof(buffer) / sizeof(buffer[0]) - 1;
		for(; p[0] && p[0] != end; p++) {
			if(p[0] == '\\')
				continue;
			if(ps < pe)
				*ps++ = p[0];
		}
		if(p[0] == end)
			p++;
		ps[0] = 0;
	}

	bool iskey(const char* p) {
		if((p[0] >= 'a' && p[0] <= 'z') || (p[0] >= 'A' && p[0] <= 'Z')) {
			while(*p && ((*p >= '0' && *p <= '9') || *p == '_' || ischa(*p)))
				p++;
			p = skipws(p);
			return p[0] != '(';
		}
		return true;
	}

	bool readidentifier() {
		if((p[0] >= 'a' && p[0] <= 'z') || (p[0] >= 'A' && p[0] <= 'Z')) {
			auto ps = buffer;
			auto pe = ps + sizeof(buffer) / sizeof(buffer[0]) - 1;
			while(*p && ((*p >= '0' && *p <= '9') || *p == '_' || ischa(*p))) {
				if(ps < pe)
					*ps++ = *p;
				p++;
			}
			ps[0] = 0;
		} else
			return false;
		return true;
	}

	bool readvalue(const bsreq* hint_type, bool create) {
		bool need_identifier = false;
		buffer[0] = 0;
		value = 0;
		value_type = 0;
		value_object = 0;
		if(p[0] == '-' || (p[0] >= '0' && p[0] <= '9')) {
			value = sz2num(p, &p);
			value_type = number_type;
		} else if(p[0] == '\"') {
			p++;
			readstring('\"');
			value_type = text_type;
		} else if(readidentifier()) {
			value_type = text_type;
			need_identifier = true;
		} else
			return false; // Not found value tag
		if(need_identifier) {
			auto value_data = findbase(hint_type);
			if(value_data)
				value_object = value_data->find(value_data->fields->getkey(), buffer);
			else
				warning(ErrorNotFoundType);
			// If not find create this
			if(!value_object && value_data && create) {
				if(value_data->getcount() < value_data->getmaxcount()) {
					auto f = value_data->fields->getkey();
					if(f) {
						value_object = value_data->add();
						f->set(f->ptr(value_object), (int)szdup(buffer));
					}
				}
			}
			if(value_data)
				value_type = value_data->fields;
			else
				value_type = number_type;
			if(value_object && value_data)
				value = value_data->indexof(value_object);
			if(!value_object)
				warning(ErrorNotFoundIdentifier1p, buffer);
		} else if(create && hint_type && value_type == number_type) {
			auto value_data = findbase(hint_type);
			value_type = hint_type;
			if(value_data) {
				if(value < (int)value_data->getmaxcount()) {
					if(value >= (int)value_data->getcount())
						value_data->setcount(value + 1);
					value_object = value_data->get(value);
				}
			}
		}
		skipws();
		return true;
	}

	void storevalue(void* object, const bsreq* req, unsigned index) {
		if(!object || !req)
			return;
		auto p = req->ptr(object, index);
		if(req->type == text_type) {
			if(!buffer[0])
				req->set(p, 0);
			else {
				auto pv = szdup(buffer);
				req->set(p, (int)pv);
			}
		} else if(req->type == number_type)
			req->set(p, value);
		else if(req->reference)
			req->set(p, (int)value_object);
		else if(req->subtype==bsreq::Enum)
			req->set(p, value);
		else
			storevalue((void*)req->ptr(object), req->type + index, 0);
	}

	bool readreq(void* object, const bsreq* req, unsigned index) {
		if(!skip('('))
			return false;
		while(*p) {
			if(skip(')'))
				break;
			readvalue(req ? req->type : 0, true);
			storevalue(object, req, index);
			if(skip(','))
				index++;
		}
		skipws();
		return true;
	}

	bool readfields(void* object, const bsreq* fields) {
		while(*p && !islinefeed()) {
			const bsreq* req = 0;
			if(readidentifier())
				req = fields->find(buffer);
			if(!req)
				warning(ErrorNotFoundMember1pInBase2p, buffer, getbasename(fields));
			readreq(object, req, 0);
		}
		return true;
	}

	bool readrecord() {
		if(!skip('#'))
			return false;
		// Read data base name
		if(!readidentifier()) {
			error(ErrorExpectedIdentifier);
			return true;
		}
		skipws();
		const bsreq* fields = 0;
		auto pd = findbase(buffer);
		if(pd)
			fields = pd->fields;
		else
			warning(ErrorNotFoundBase1p, buffer);
		// Read key value
		if(iskey(p))
			readvalue(fields, true);
		else if(pd)
			value_object = pd->add();
		else
			value_object = 0;
		auto parent_object = value_object;
		readfields(value_object, fields);
		readsubrecord(parent_object, fields);
		return true;
	}

	void readsubrecord(void* parent_object, const bsreq* parent_type) {
		auto index = 0;
		auto last_field = 0;
		while(skip("##")) {
			// Read data base name
			if(!readidentifier()) {
				error(ErrorExpectedIdentifier);
				return;
			}
			skipws();
			auto parent_field = parent_type->find(buffer);
			if(!parent_field) {
				error(ErrorNotFoundMember1pInBase2p, buffer, "");
				return;
			}
			if(parent_field->count <= 1 // Only array may be defined as ##
				|| parent_field->reference // No reference allowed
				|| parent_field->subtype==bsreq::Enum // Enumeratior must be initialized in row
				|| parent_field->type->issimple()) { // No Simple type
				error(ErrorExpectedArrayField);
			}
			readfields((void*)parent_field->ptr(parent_object, index),
				parent_field->type);
			index++;
		}
	}

	void readtrail() {
		auto pb = buffer;
		auto pe = pb + sizeof(buffer) - 1;
		while(true) {
			auto sym = *p;
			if(!sym)
				break;
			if(sym == '\n' || sym == '\r') {
				while(*p == '\n' || *p == '\r') {
					p = szskipcr(p);
					skipws();
				}
				if(*p == '#')
					break;
				if(pb != buffer && pb < pe)
					*pb++ = '\n';
				continue;
			}
			if(pb < pe)
				*pb++ = sym;
			p++;
		}
		*pb = 0;
	}

	void parse() {
		while(*p) {
			if(!readrecord())
				return;
			readtrail();
		}
	}

};

static bool isidentifier(const char* p) {
	if((p[0] >= 'a' && p[0] <= 'z') || (p[0] >= 'A' && p[0] <= 'Z')) {
		while(*p) {
			if(!((*p >= '0' && *p <= '9') || *p == '_' || ischa(*p)))
				return false;
			p++;
		}
		return true;
	}
	return false;
}

static bool isempthy(const void* object, const bsreq* req, bool check_array = true) {
	auto ps = req->ptr(object);
	if(check_array && req->count > 1) {
		for(auto i = 0; i < (int)req->count; i++)
			if(!isempthy((void*)req->ptr(object, i), req, false))
				return false;
	} else {
		for(unsigned i = 0; i < req->size; i++) {
			if(ps[i])
				return false;
		}
		if(req->type == text_type && req->count == 1 && req->reference) {
			auto value = (const char*)req->get(req->ptr(object));
			if(value && value[0] == 0)
				return false;
		}
	}
	return true;
}

static void write_identifier(io::stream& e, const char* value) {
	if(isidentifier(value))
		e << value;
	else
		e << "'" << value << "'";
}

static void write_number(io::stream& e, int value) {
	e << value;
}

static const bsreq* write_key(io::stream& e, const void* object, const bsreq* type) {
	auto f = type->getkey();
	auto v = "";
	if(f)
		v = (const char*)f->get(f->ptr(object));
	if(v && v[0])
		write_identifier(e, v);
	else {
		auto pd = bsdata::findbyptr(object);
		if(!pd)
			return 0;
		write_number(e, pd->indexof(object));
	}
	return f;
}

static void write_value(io::stream& e, const void* object, const bsreq* req, int level);

static void write_array(io::stream& e, const void* object, const bsreq* req, int level) {
	if(level > 0 && req->count > 1)
		e << "(";
	for(unsigned index = 0; index < req->count; index++) {
		if(index > 0)
			e << ", ";
		write_value(e, req->ptr(object, index), req, level);
	}
	if(level > 0 && req->count > 1)
		e << ")";
}

static void write_value(io::stream& e, const void* object, const bsreq* req, int level) {
	if(req->type == number_type) {
		auto value = req->get(object);
		e << value;
	} else if(req->type == text_type) {
		auto value = (const char*)req->get(object);
		e << "\"" << value << "\"";
	} else if(req->reference) {
		auto value = (const void*)req->get(object);
		write_key(e, value, req->type);
	} else if(req->subtype == bsreq::Enum) {
		auto value = req->get(object);
		auto pd = bsdata::find(req->type);
		if(pd)
			write_key(e, pd->get(value), req->type);
		else
			e << value;
	} else {
		if(level > 0)
			e << "(";
		auto count = 0;
		for(auto f = req->type; *f; f++) {
			if(count)
				e << ", ";
			write_array(e, object, f, level + 1);
			count++;
		}
		if(level > 0)
			e << ")";
	}
}

void write_fields(io::stream& e, const void* object, const bsreq* req, const bsreq* skip = 0) {
	auto count = 0;
	for(auto f = req; *f; f++) {
		if(skip && skip == f)
			continue;
		if(isempthy(object, req))
			continue;
		if(count > 0)
			e << " ";
		e << req->id;
		e << "(";
		write_array(e, object, req, 0);
		e << ")";
		count++;
	}
	e << "\r\n";
}

static void write_object(io::stream& e, const void* object) {
	auto pd = bsdata::findbyptr(object);
	if(!pd)
		return;
	e << "#" << pd->id << " ";
	auto skip = write_key(e, object, pd->fields);
	e << " ";
	write_fields(e, object, pd->fields, skip);
}

static void write_data(io::stream& e, bsdata* pd, bool(*comparer)(void* object, const bsreq* type)) {
	if(!pd)
		return;
	for(int index = 0; index < (int)pd->getcount(); index++) {
		auto object = pd->get(index);
		if(comparer && !comparer(object, pd->fields))
			write_object(e, object);
	}
}

void bsdata::write(const char* url, const char** bases, bool(*comparer)(void* object, const bsreq* type)) {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	for(auto pname = bases; *pname; pname++)
		write_data(file, bsdata::find(*pname), comparer);
}

void bsdata::write(const char* url, const char* baseid) {
	const char* source[] = {baseid, 0};
	write(url, source);
}

void bsdata::read(const char* url, bsdata::parser* parser) {
	bsdata_serial e(url);
	e.parser = parser;
	if(e)
		e.parse();
}