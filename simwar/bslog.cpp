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
"File %2 not found",
};
assert_enum(bsparse_error, ErrorFile2pNotFound);

void bslog::head(const char* url, int line, int column) {
	if(line == 0 && column == 0)
		return;
	file << url << "(" << line << ", " << column << "): ";
}

void bslog::linefeed() {
	file << "\r\n";
}

void bslog::error(bsparse_error_s id, const char* url, int line, int column, const char* format_param) {
	auto parameters = (const char**)format_param;
	if(id == ErrorNotFilled1pIn2pRecord3p && strcmp(parameters[0], "name") == 0) {
		if(last_error != id)
			file << "In file " << url << " you need add next lines" << "\r\n";
		file << parameters[0] << ":";
	} else {
		char temp[4096];
		head(url, line, column);
		zprint(temp, bsparse_error_data[id], parameters[0], parameters[1], parameters[2], parameters[3], parameters[4]);
		file << temp;
	}
	linefeed();
	last_error = id;
}