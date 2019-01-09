#include "bsdata.h"
#include "crt.h"
#include "io.h"

class bsfile {
	const bsfile*		parent;
	const char*			url;
	const char*			start;
public:
	bsfile(const char* url, const bsfile* parent = 0) : parent(0), url(url), start(loadt(url)) {}
	~bsfile() { delete start; }
	operator bool() const { return start != 0; }
	const char*			getstart() const { return start; }
	const char*			geturl() const { return url; }
};

struct bsdata_serial : bsfile {
	char				buffer[2048];
	int					value;
	const bsreq*		value_type;
	void*				value_object;
	const char*			p;
	bsdata::parser&		errors;

	bsdata_serial(bsdata::parser& errors, const char* url, const bsfile* parent = 0) : bsfile(url, parent), errors(errors), p(getstart()) {
		clearvalue();
		buffer[0] = 0;
	}

	static const char* skipcr(const char* p) {
		if(*p == 13) {
			p++;
			if(*p == 10)
				p++;
		} else if(*p == 10) {
			p++;
			if(*p == 13)
				p++;
		}
		return p;
	}

	static const char* skipline(const char* p) {
		while(p[0] && p[0] != 10 && p[0] != 13)
			p++;
		return p;
	}

	static const char* skipws(const char* p) {
		while(*p) {
			if(p[0] == 9 || p[0] == 0x20) {
				p++;
				continue;
			} else if(p[0] == '/' && p[1] == '/')
				return skipline(p + 2);
			break;
		}
		return p;
	}

	bsdata* findbase(const bsreq* type) const {
		if(errors.custom) {
			for(auto p = errors.custom; *p; p++) {
				if((*p)->fields == type)
					return *p;
			}
		}
		return bsdata::find(type);
	}

	bsdata* findbase(const char* id) const {
		if(errors.custom) {
			for(auto p = errors.custom; *p; p++) {
				if(strcmp((*p)->id, id) == 0)
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
		int line, column;
		getpos(p, line, column);
		errors.error(id, geturl(), line, column, xva_start(id));
		errors.add();
		p = skipline(p);
	}

	void warning(bsparse_error_s id, ...) {
		int line, column;
		getpos(p, line, column);
		errors.error(id, geturl(), line, column, xva_start(id));
		errors.add();
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

	bool isnumber() const {
		return p[0] == '-' || (p[0] >= '0' && p[0] <= '9');
	}

	bool isidentifier() const {
		return (p[0] >= 'a' && p[0] <= 'z') || (p[0] >= 'A' && p[0] <= 'Z');
	}

	bool readidentifier() {
		if(isidentifier()) {
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

	bool readvalue(const bsreq* requisit) {
		buffer[0] = 0;
		value = 0;
		value_type = 0;
		value_object = 0;
		bool need_identifier = false;
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
			auto value_data = findbase(requisit->type);
			if(value_data)
				value_object = value_data->find(value_data->fields->getkey(), buffer);
			else
				warning(ErrorNotFoundType);
			// If not find create this
			if(!value_object && value_data) {
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
		else if(req->subtype == bsreq::Enum)
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
			readvalue(req ? req->type : 0);
			storevalue(object, req, index);
			if(skip(','))
				index++;
		}
		skipws();
		return true;
	}

	void* findkey(array* placement, const bsreq* type, const char* id) const {
		auto pk = type->getkey();
		if(!pk)
			return 0;
		for(unsigned i = 0; i < placement->count; i++) {
			auto p = placement->get(i);
			auto ps = (const char*)pk->get(pk->ptr(p));
			if(!ps)
				continue;
			if(strcmp(ps, id) == 0)
				return p;
		}
		return 0;
	}

	void setkey(void* object, const bsreq* type, const char* id) {
		auto pk = type->getkey();
		if(pk)
			pk->set(pk->ptr(object), (int)szdup(id));
	}

	// Должно быть заполнено database и type
	bool readfields(array* placement, void* object, const bsreq* type) {
		while(true) {
			skipws();
			if(*p == 0 || *p == 10 || *p == 13)
				return true;
			if(!object) {
				if(readidentifier()) {
					if(p[0] != ' ' && p[0] != '\t') {
						error(ErrorExpected1p, "whitespace");
						return false;
					}
					object = findkey(placement, type, buffer);
					if(!object) {
						if(placement->getcount() > placement->getmaxcount()) {
							error(ErrorExpected1p, "[error] Exceed array index");
							return false;
						}
						object = placement->add();
						setkey(object, type, buffer);
					}
				} else {
					if(!isnumber()) {
						error(ErrorExpected1p, "number or identifier");
						return false;
					}
					readvalue(number_type);
					if(value >= (int)placement->count) {
						if(value >= (int)placement->count_maximum) {
							error(ErrorExpected1p, "lesser array index");
							return false;
						}
						placement->count = value + 1;
					}
					object = placement->get(value);
				}
				if(!object) {
					error(ErrorExpected1p, "[error] Need object");
					return false;
				}
				continue;
			}
			if(readidentifier()) {
				if(p[0] == '(') {
					p = skipws(p + 1);
					auto requisit = type->find(buffer);
					if(!requisit) {
						error(ErrorNotFoundMember1pInBase2p, buffer, findbase(type));
						return false;
					}
					readvalue(requisit);
					skip(')');
				}
			}
		}
		return true;
	}

	void parse() {
		while(true) {
			skipws();
			if(*p == 0)
				return;
			if(*p == 13 || *p == 10) {
				p = skipcr(p);
				clearvalue();
				continue;
			}
			// Read data base name
			if(!readidentifier()) {
				error(ErrorExpectedIdentifier);
				return;
			}
			skipws();
			auto value_base = findbase(buffer);
			if(!value_base) {
				error(ErrorNotFoundBase1p, buffer);
				continue;
			}
			readfields(value_base, 0, value_base->fields);
		}
	}

};

void bsdata::read(const char* url, bsdata::parser& parser) {
	bsdata_serial e(parser, url);
	if(e)
		e.parse();
}

void bsdata::read(const char* url) {
	parser errors;
	read(url, errors);
}