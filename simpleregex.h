#pragma once

#include <utility>

std::pair<const char*,const char*> match(const char* regexp,const char* text);
std::pair<const wchar_t*, const wchar_t*> match(wchar_t* regexp, wchar_t* text);
std::pair<const char16_t*, const char16_t*> match(char16_t* regexp, char16_t* text);
std::pair<const char32_t*, const char32_t*> match(char32_t* regexp, char32_t* text);