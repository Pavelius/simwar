#pragma once

#define xva_start(v) ((const char*)&v + sizeof(v))

struct stringcreator {
	const char*			parseformat(char* result, const char* result_max, const char* format, const char* format_param);
	virtual void		parseidentifier(char* result, const char* result_max, const char* identifier);
	static char*		parseint(char* result, const char* result_max, int value, int precision, const int radix);
	static char*		parsenumber(char* result, const char* result_max, unsigned value, int precision, const int radix);
	virtual void		parsevariable(char* result, const char* result_max, const char** format);
	void				print(char* result, const char* result_maximum, const char* format, ...);
	char*				printv(char* result, const char* result_maximum, const char* src, const char* vl);
};
struct stringbuilder {
	constexpr stringbuilder(stringcreator& driver, char* result, const char* result_maximum) : p(result), result(result), result_maximum(result_maximum), driver(driver) {}
	template<unsigned N> constexpr stringbuilder(stringcreator& driver, char(&result)[N]) : stringbuilder(driver, result, result + N) {}
	operator const char*() const { return result; }
	explicit operator bool() const { return p>result; }
	void				add(const char* format, ...);
	void				addh(const char* format, ...);
	void				addn(const char* format, ...) { addx("\n", format, xva_start(format)); }
	void				adds(const char* format, ...) { addx(" ", format, xva_start(format)); }
	void				addv(const char* format, const char* format_param);
	void				addx(const char* separator, const char* format, const char* format_param);
	void				clear() { result[0] = 0; p = result; }
	const char*			getpos() const { return p; }
	bool				ispos(const char* v) const { return p == v; }
	void				sep(const char* header, const char* p);
private:
	stringcreator & driver;
	char				*p, *result;
	const char*			result_maximum;
};