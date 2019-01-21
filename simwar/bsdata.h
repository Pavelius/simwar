#include "bsreq.h"

#pragma once

// Declare metadata for type 'c'. Variables 'c_type' and 'c_data' must exist.
// Datasource can be fixed array, adat, or single declared element.
#define BSMETA(c) \
bsdata c##_manager(#c, c##_data, c##_type);

struct bsdata_strings {
	const char*		id;
	int				range[2];
	const char**	requisits;
	constexpr explicit operator bool() const { return id != 0; }
	constexpr bool	isrange() const { return range[0] || range[1]; }
};
enum bsparse_error_s {
	NoParserError,
	ErrorExpectedIdentifier, ErrorExpectedArrayField, ErrorExpectedSymbol1p, ErrorExpected1p,
	ErrorNotFoundBase1p, ErrorNotFoundType, ErrorNotFoundIdentifier1p, ErrorNotFoundMember1pInBase2p, ErrorNotFoundFunction1p,
	ErrorNotFilled1pIn2pRecord3p, ErrorValue1pIn2pRecord3pMustBeIn4pAnd5p,
	ErrorFile1pNotFound,
};
struct bsdata : array {
	struct requisit {
		const char*		id;
		bool			required;
		int				values[2];
		bsreq*			type;
	};
	struct parser {
		constexpr parser() : count(0) {}
		void			add() { count++; }
		void			add(bsparse_error_s id, const char* url, int line, int column, ...);
		bool			check(const requisit* requisits, unsigned requisits_count);
		bool			check(const char* url, bsval source);
		virtual const char* getinclude(char* result, const char* result_end, const char* id) { return id; }
		virtual void	error(bsparse_error_s id, const char* url, int line, int column, const char* format_param) {}
		virtual bsdata*	findbase(const bsreq* type) const;
		virtual bsdata*	findbase(const char* id) const;
		int				getcount() const { return count; }
		virtual const bsreq* getrequisit(const bsreq* fields, const char* buffer) const;
	private:
		int				count;
	};
	const char*			id;
	const bsreq*		fields;
	bsdata*				next;
	static bsdata*		first;
	//
	constexpr bsdata(const bsreq* fields) : id(""), fields(fields), next(0) {}
	constexpr bsdata(const char* id, const bsreq* fields, void* source, unsigned size, unsigned maximum) : array(source, size, maximum), id(id), fields(fields), next() {}
	constexpr bsdata(const char* id, const bsreq* fields, void* source, unsigned size, unsigned maximum, unsigned& count) : array(source, size, maximum, count), id(id), fields(fields), next() {}
	template<typename T, unsigned N> bsdata(const char* id, adat<T, N>& e, const bsreq* fields) : array(e.data, sizeof(T), N, e.count), id(id), fields(fields) { globalize(); }
	template<typename T, unsigned N> bsdata(const char* id, T(&e)[N], const bsreq* fields) : array(e, sizeof(T), N), id(id), fields(fields) { globalize(); }
	template<typename T> bsdata(const char* id, T& e, const bsreq* fields) : array(&e, sizeof(T), 1), id(id), fields(fields) { globalize(); }
	//
	void				globalize();
	static bsdata*		find(const char* id);
	static bsdata*		find(const bsreq* id);
	void*				find(const bsreq* id, const char* value);
	static bsdata*		findbyptr(const void* object);
	static bsval		findbyid(const char* value);
	static void			read(const char* url);
	static void			read(const char* url, parser& errors);
	static bool			readl(const char* url, const char** requisits, const bsdata_strings* tables);
	static bool			readl(const char* url, const char** requisits, void* object, const bsreq* type);
	static bool			readl(const char* url, const char* locale_name, parser& log, const char** prefixs, const char** requisits, const bsdata_strings* tables, const char** skip_name);
	static void			write(const char* url, const char** baseids, bool(*comparer)(void* object, const bsreq* type) = 0);
	static void			write(const char* url, const char* baseid);
	static void			write(const char* url, const array& source, const bsreq* fields);
};