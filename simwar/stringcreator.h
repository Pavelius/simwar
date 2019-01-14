#pragma once

#define xva_start(v) ((const char*)&v + sizeof(v))

struct stringcreator {
	stringcreator(char* pb, const char* pe) : pb(pb), p(pb), pe(pe) { pb[0] = 0; }
	template<unsigned N> stringcreator(char(&result)[N]) : stringcreator(result, result + N) {}
	constexpr operator char*() const { return pb; }
	void				add(const char* format, ...);
	virtual void		addidentifier(const char* identifier);
	static char*		addint(char* result, const char* result_maximum, int value, int precision, const int radix);
	void				addn(const char* format, ...);
	void				adds(const char* format, ...);
	void				addv(const char* format, const char* format_param);
	void				addx(const char* separator, const char* format, const char* format_param);
	static char*		adduint(char* result, const char* result_maximum, unsigned value, int precision, const int radix);
	char*				begin() { return pb; }
	const char*			begin() const { return pb; }
	void				clear() { pb[0] = 0; p = pb; }
	const char*			end() const { return pe; }
	char*				get() const { return p; }
	bool				ispos(const char* v) const { return p == v; }
private:
	char*				p;
	char*				pb;
	const char*			pe;
	const char*			readformat(const char* format, const char* format_param);
	const char*			readvariable(const char* format);
};