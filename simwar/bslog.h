#include "bsdata.h"
#include "io.h"

#pragma once

struct bslog : bsdata::parser {
	io::file			file;
	bsparse_error_s		last_error;
	bslog(const char* url) : file(url, StreamWrite | StreamText), last_error(NoParserError) {}
	void				error(bsparse_error_s id, const char* url, int line, int column, const char* format_param) override;
	void				head(const char* url, int line, int column);
	bool				iserrors() const { return last_error != NoParserError; }
	void				linefeed();
};
