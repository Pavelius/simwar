#include "bslog.h"
#include "crt.h"

static const char* bsparse_error_data[] = {"No errors",
"Expected identifier",
"Expected array or field",
"Expected symbol %1",
"Expected %1",
//
"Not found base %1",
"Not found type",
"Not found identifier %1",
"Not found member %1 in base %2",
"Not found function %1",
//
"Not filled %1 in %2 %3",
"Requisit %1 in %2 %3 must be in interval of %4i...%5i",
"File '%1' not found",
};
assert_enum(bsparse_error, ErrorFile1pNotFound);

bslog::~bslog() {
	if(stream)
		delete stream;
	stream = 0;
}

void bslog::head(const char* url, int line, int column) {
	if(line == 0 && column == 0)
		return;
	*stream << url << "(" << line << ", " << column << "): ";
}

void bslog::linefeed() {
	*stream << "\r\n";
}

void bslog::error(bsparse_error_s id, const char* url, int line, int column, const char* format_param) {
	auto parameters = (const char**)format_param;
	if(!stream)
		stream = new io::file(this->url, StreamWrite | StreamText);
	if(id == ErrorNotFilled1pIn2pRecord3p
		&& (strcmp(parameters[0], "name") == 0 || (parameters[0][0]==0 && parameters[1][0] == 0))) {
		if(last_error != id)
			*stream << "In file " << url << " you need add next lines" << "\r\n";
		*stream << parameters[2] << ":";
	} else {
		char temp[4096];
		head(url, line, column);
		zprint(temp, bsparse_error_data[id], parameters[0], parameters[1], parameters[2], parameters[3], parameters[4]);
		*stream << temp;
	}
	linefeed();
	last_error = id;
}