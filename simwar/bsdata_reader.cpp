#include "bsdata.h"
#include "crt.h"
#include "io.h"

class textfile {
	const char*			url;
	const char*			content;
public:
	textfile(const char* url) : content(loadt(url)), url(url) {}
	~textfile() { delete content; }
	operator bool() const { return begin() != 0; }
	const char*			begin() const { return content; }
	const char*			geturl() const { return url; }
};

struct bsdata_serial {
	
	const textfile*		file;
	arem<const char*>	names;
	char				buffer[1024];
	int					value;
	const bsreq*		value_type;
	void*				value_object;
	const char*			p;
	bsdata::parser&		parser;

	bsdata_serial(bsdata::parser& parser) : parser(parser), p(0) {
	}

	void setup(const textfile& e) {
		file = &e;
		p = e.begin();
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
			} else if(p[0] == '/' && p[1] == '/') {
				// Comments
				return skipline(p + 2);
			}
			break;
		}
		return p;
	}

	const char* getbasename(const bsreq* type) const {
		auto p = parser.findbase(type);
		if(!p)
			return "";
		return p->id;
	}

	void getpos(const char* p, int& line, int& column) {
		line = 0;
		column = 0;
		if(!file)
			return;
		auto ps = file->begin();
		while(*ps) {
			line++;
			auto pe = skipcr(skipline(ps));
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
		if(!file)
			return;
		parser.error(id, file->geturl(), line, column, xva_start(id));
		parser.add();
		p = skipline(p);
	}

	void warning(bsparse_error_s id, ...) {
		int line, column;
		getpos(p, line, column);
		if(!file)
			return;
		parser.error(id, file->geturl(), line, column, xva_start(id));
		parser.add();
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
		p = skipws(p + 1);
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

	bool readvalue(const bsreq* hint_type) {
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
			auto value_data = parser.findbase(hint_type);
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
			readvalue(req ? req->type : 0);
			storevalue(object, req, index);
			if(skip(','))
				index++;
		}
		skipws();
		return true;
	}

	void directive_include() {
		if(p[0] != '\"') {
			error(ErrorExpected1p, "\"");
			return;
		}
		p++;
		readstring('\"');
		auto ps = szdup(buffer);
		if(names.indexof(ps) != -1)
			return; // Файл уже загружался
		names.add(ps);
		ps = parser.getinclude(zend(buffer), zendof(buffer), ps);
		if(!ps)
			return;
		textfile new_file(ps);
		if(!new_file) {
			error(ErrorFile1pNotFound, buffer);
			return;
		}
		auto old_file = file;
		auto old_p = p;
		setup(new_file);
		parse();
		p = old_p;
		file = old_file;
		clearvalue();
	}

	bool directive() {
		if(strcmp(buffer, "include") == 0)
			directive_include();
		else
			return false;
		return true;
	}

	int getspacecount(const char* p) {
		auto p1 = p;
		while(*p == ' ')
			p++;
		return p - p1;
	}

	void readobject(void* object, const bsreq* type, int level) {
		while(*p && !islinefeed()) {
			const bsreq* req = 0;
			if(readidentifier())
				req = parser.getrequisit(type, buffer);
			if(!req)
				warning(ErrorNotFoundMember1pInBase2p, buffer, getbasename(type));
			readreq(object, req, 0);
		}
		auto index = 0;
		while(*p && islinefeed()) {
			auto p1 = skipcr(p);
			auto new_level = getspacecount(p1);
			if(new_level <= level)
				return;
			p = p1 + new_level;
			if(!readidentifier()) {
				error(ErrorExpectedIdentifier);
				return;
			}
			auto req = parser.getrequisit(type, buffer);
			if(!req) {
				error(ErrorNotFoundMember1pInBase2p, buffer, getbasename(type));
				return;
			}
			skipws();
			readobject((void*)req->ptr(object, index), req->type, level+1);
			index++;
		}
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
			// Прочитаем имя базы
			if(!readidentifier()) {
				error(ErrorExpectedIdentifier);
				continue;
			}
			skipws();
			if(directive()) {
				p = skipline(p);
				continue;
			}
			// Найдем ее поля и саму базу
			const bsreq* fields = 0;
			auto pd = parser.findbase(buffer);
			if(pd)
				fields = pd->fields;
			else {
				error(ErrorNotFoundBase1p, buffer);
				continue;
			}
			// Read key value
			if(iskey(p))
				readvalue(fields);
			else if(pd)
				value_object = pd->add();
			else
				value_object = 0;
			readobject(value_object, fields, 0);
		}
	}

};

void bsdata::read(const char* url, bsdata::parser& parser) {
	textfile file(url);
	if(!file)
		return;
	bsdata_serial e(parser);
	e.setup(file);
	e.parse();
}

void bsdata::read(const char* url, parser& errors, const char* loc, const char** requisits, const bsdata_strings* ranges) {
	textfile file(url);
	if(!file)
		return;
	bsdata_serial e(errors);
	e.setup(file);
	e.parse();
	for(auto p : e.names) {
		zprint(e.buffer, "text/%1/%2.txt", loc, p);
		readl(e.buffer, requisits, ranges);
	}
}

void bsdata::read(const char* url) {
	parser errors;
	read(url, errors);
}

const bsreq* bsdata::parser::getrequisit(const bsreq* fields, const char* buffer) const {
	return fields->find(buffer);
}

bsdata* bsdata::parser::findbase(const bsreq* type) const {
	return bsdata::find(type);
}

bsdata* bsdata::parser::findbase(const char* id) const {
	return bsdata::find(id);
}