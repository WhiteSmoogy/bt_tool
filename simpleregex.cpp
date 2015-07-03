#include "simpleregex.h"

template<typename Char_Type>
const Char_Type* match_here(const Char_Type* regexp, const Char_Type* text);

template<typename Char_Type>
const Char_Type* match_star(Char_Type c, const Char_Type* regexp, const Char_Type* text);

template<typename Char_Type>
const Char_Type* match_plus(Char_Type c, const Char_Type* regexp, const Char_Type* text);

template<typename Char_Type>
const Char_Type* match_here(const Char_Type* regexp, const Char_Type* text) {
	if (regexp[0] == Char_Type())
		return text;
	if (regexp[1] == Char_Type('*'))
		return match_star(regexp[0], regexp + 2, text);
	if (regexp[1] == Char_Type('+'))
		return match_plus(regexp[0], regexp + 2, text);
	if (regexp[0] == Char_Type('$') && regexp[1] == Char_Type())
		return *text == Char_Type() ? text : nullptr;
	if (*text != Char_Type() && (regexp[0] == '.' || regexp[0] == *text))
		return match_here(regexp + 1, text + 1);

	return nullptr;
}


template<typename Char_Type>
const Char_Type* match_star(Char_Type c, const Char_Type* regexp, const Char_Type* text) {
	const Char_Type* result = nullptr;
	do {
		/* a * matches zero or more instances */
		auto ptr = match_here(regexp, text);
		if (ptr)
			result = ptr;
	} while (*text != Char_Type() && (*text++ == c || c == Char_Type('.')));

	return result;

}

template<typename Char_Type>
const Char_Type* match_plus(Char_Type c, const Char_Type* regexp, const Char_Type* text) {
	const Char_Type* result = nullptr;
	while (*text != Char_Type() && (*text++ == c || c == Char_Type('.'))) {
		/* a * matches zero or more instances */
		auto ptr = match_here(regexp, text);
		if (ptr)
			result = ptr;
	}

	return result;

}


template<typename Char_Type>
std::pair<const Char_Type*, const Char_Type*> match_impl(const Char_Type* regexp, const Char_Type* text) {
	if (regexp[0] == Char_Type('^'))
		return{ text,match_here(regexp + 1, text) };

	do {
		auto ptr = match_here(regexp, text);
		if (ptr)
			return{ text,ptr };
	} while (*text++ != Char_Type());

	return{ nullptr,nullptr };
}

std::pair<const char*, const char*> match(const char* regexp, const char* text) {
	return match_impl(regexp, text);
}
std::pair<const wchar_t*, const wchar_t*> match(wchar_t* regexp, wchar_t* text) {
	return match_impl(regexp, text);
}
std::pair<const char16_t*, const char16_t*> match(char16_t* regexp, char16_t* text) {
	return match_impl(regexp, text);
}
std::pair<const char32_t*, const char32_t*> match(char32_t* regexp, char32_t* text) {
	return match_impl(regexp, text);
}