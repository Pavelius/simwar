#include "bsdata.h"
#include "io.h"

#pragma once

struct bslog : bsdata::parser {
	constexpr bslog(const char* url) : url(url), stream(0), last_error(NoParserError) {}
	~bslog();
	void				error(bsparse_error_s id, const char* url, int line, int column, const char* format_param) override;
	const char*			getinclude(char* result, const char* result_end, const char* id) const;
	const char*			getlocale(char* result, const char* result_end, const char* id, const char* loc) const;
	void				head(const char* url, int line, int column);
	void				linefeed();
private:
	const char*			url;
	io::file*			stream;
	bsparse_error_s		last_error;
};
