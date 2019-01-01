#include "bsreq.h"

#pragma once

// Declare metadata for type 'c'. Variables 'c_type' and 'c_data' must exist.
// Datasource can be fixed array, adat, or single declared element.
#define BSMETA(c) \
bsdata c##_manager(#c, c##_data, c##_type);

enum bsparse_error_s {
	NoParserError,
	ErrorExpectedIdentifier, ErrorExpectedArrayField, ErrorExpectedSymbol1p, ErrorExpected1p,
	ErrorNotFoundBase1p, ErrorNotFoundType, ErrorNotFoundIdentifier1p, ErrorNotFoundMember1pInBase2p, ErrorNotFoundFunction1p,
	ErrorNotFilled1p, ErrorNotFilled1pIn2pRecord3p,
	ErrorFile2pNotFound,
};
struct bsdata : array {
	struct parser {
		bsdata**		custom;
		bool			create_record_when_not_found;
		constexpr parser() : custom(0), create_record_when_not_found(false) {}
		bool			check_required(const char* url);
		bool			check_required(const char* url, bsval source);
		virtual void	error(bsparse_error_s id, const char* url, int line, int column, const char* format_param) {}
		void			errornp(bsparse_error_s id, const char* url, int line, int column, ...);
	};
	const char*			id;
	const bsreq*		fields;
	bsdata*				next;
	static bsdata*		first;
	//
	constexpr bsdata(const bsreq* fields) : id(""), fields(fields), next(0) {}
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
	static void			read(const char* url, parser* callback = 0);
	static void			write(const char* url, const char** baseids, bool(*comparer)(void* object, const bsreq* type) = 0);
	static void			write(const char* url, const char* baseid);
	static void			write(const char* url, const array& source, const bsreq* fields);
};