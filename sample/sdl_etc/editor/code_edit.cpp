/*
** SDL Code Edit
**
** Copyright (C) 2018 Wang Renxin
**
** A code edit widget in plain SDL.
**
** For the latest info, see https://github.com/paladin-t/sdl_code_edit/
*/

#define NOMINMAX
#include "code_edit.h"
#include <SDL2_gfxPrimitives.h>
#include <SDL.h>
#undef EA_PLATFORM_MICROSOFT
#include <algorithm.h>
#include <chrono.h>

typedef jpcre2::select<char> jp;


void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}


/*
** {========================================================
** Code edit
*/

#ifndef CODE_EDIT_EPSILON
#	define CODE_EDIT_EPSILON 0.0001f
#endif /* CODE_EDIT_EPSILON */

#ifndef CODE_EDIT_UTF8_CHAR_FACTOR
#	define CODE_EDIT_UTF8_CHAR_FACTOR 2
#endif /* CODE_EDIT_UTF8_CHAR_FACTOR */

#ifndef CODE_EDIT_MERGE_UNDO_REDO
#	define CODE_EDIT_MERGE_UNDO_REDO 1
#endif /* CODE_EDIT_MERGE_UNDO_REDO */

#ifndef CODE_EDIT_CASE_FUNC
#	define CODE_EDIT_CASE_FUNC ::tolower
#endif /* CODE_EDIT_CASE_FUNC */

#ifndef countof
#	define countof(A) (sizeof(A) / sizeof(*(A)))
#endif /* countof */

static_assert(sizeof(CodeEdit::Keycode) == sizeof(SDL_Keycode), "Wrong type size.");

static const int COLORIZE_DELAY_FRAME_COUNT = 60;

static bool isPrintable(int cp) {
	if (cp > 255) return false;

	return isprint(cp);
}

static int countUtf8BytesFromChar(unsigned int c) {
	if (c < 0x80) return 1;
	if (c < 0x800) return 2;
	if (c >= 0xdc00 && c < 0xe000) return 0;
	if (c >= 0xd800 && c < 0xdc00) return 4;

	return 3;
}

static int countUtf8BytesFromStr(const CodeEdit::CodePoint* in_text, const CodeEdit::CodePoint* in_text_end) {
	int bytes_count = 0;
	while ((!in_text_end || in_text < in_text_end) && *in_text) {
		unsigned int c = (unsigned int)(*in_text++);
		if (c < 0x80)
			bytes_count++;
		else
			bytes_count += countUtf8BytesFromChar(c);
	}

	return bytes_count;
}

static int charFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end) {
	unsigned int c = (unsigned int)-1;
	const unsigned char* str = (const unsigned char*)in_text;
	if (!(*str & 0x80)) {
		c = (unsigned int)(*str++);
		*out_char = c;

		return 1;
	}
	if ((*str & 0xe0) == 0xc0) {
		*out_char = 0xFFFD; // Will be invalid but not end of string.
		if (in_text_end && in_text_end - (const char*)str < 2) return 1;
		if (*str < 0xc2) return 2;
		c = (unsigned int)((*str++ & 0x1f) << 6);
		if ((*str & 0xc0) != 0x80) return 2;
		c += (*str++ & 0x3f);
		*out_char = c;

		return 2;
	}
	if ((*str & 0xf0) == 0xe0) {
		*out_char = 0xFFFD; // Will be invalid but not end of string.
		if (in_text_end && in_text_end - (const char*)str < 3) return 1;
		if (*str == 0xe0 && (str[1] < 0xa0 || str[1] > 0xbf)) return 3;
		if (*str == 0xed && str[1] > 0x9f) return 3; // str[1] < 0x80 is checked below.
		c = (unsigned int)((*str++ & 0x0f) << 12);
		if ((*str & 0xc0) != 0x80) return 3;
		c += (unsigned int)((*str++ & 0x3f) << 6);
		if ((*str & 0xc0) != 0x80) return 3;
		c += (*str++ & 0x3f);
		*out_char = c;

		return 3;
	}
	if ((*str & 0xf8) == 0xf0) {
		*out_char = 0xFFFD; // Will be invalid but not end of string.
		if (in_text_end && in_text_end - (const char*)str < 4) return 1;
		if (*str > 0xf4) return 4;
		if (*str == 0xf0 && (str[1] < 0x90 || str[1] > 0xbf)) return 4;
		if (*str == 0xf4 && str[1] > 0x8f) return 4; // str[1] < 0x80 is checked below.
		c = (unsigned int)((*str++ & 0x07) << 18);
		if ((*str & 0xc0) != 0x80) return 4;
		c += (unsigned int)((*str++ & 0x3f) << 12);
		if ((*str & 0xc0) != 0x80) return 4;
		c += (unsigned int)((*str++ & 0x3f) << 6);
		if ((*str & 0xc0) != 0x80) return 4;
		c += (*str++ & 0x3f);
		// UTF-8 encodings of values used in surrogate pairs are invalid.
		if ((c & 0xFFFFF800) == 0xD800) return 4;
		*out_char = c;

		return 4;
	}
	*out_char = 0;

	return 0;
}

static int charToUtf8(char* buf, int buf_size, unsigned int c) {
	if (c < 0x80) {
		buf[0] = (char)c;

		return 1;
	}
	if (c < 0x800) {
		if (buf_size < 2) return 0;
		buf[0] = (char)(0xc0 + (c >> 6));
		buf[1] = (char)(0x80 + (c & 0x3f));

		return 2;
	}
	if (c >= 0xdc00 && c < 0xe000) {
		return 0;
	}
	if (c >= 0xd800 && c < 0xdc00) {
		if (buf_size < 4) return 0;
		buf[0] = (char)(0xf0 + (c >> 18));
		buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
		buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
		buf[3] = (char)(0x80 + ((c) & 0x3f));

		return 4;
	}
	//else if (c < 0x10000)
	{
		if (buf_size < 3) return 0;
		buf[0] = (char)(0xe0 + (c >> 12));
		buf[1] = (char)(0x80 + ((c >> 6) & 0x3f));
		buf[2] = (char)(0x80 + ((c) & 0x3f));

		return 3;
	}
}

static int strFromUtf8(CodeEdit::CodePoint* buf, int buf_size, const char* in_text, const char* in_text_end, const char** in_text_remaining = nullptr) {
	CodeEdit::CodePoint* buf_out = buf;
	CodeEdit::CodePoint* buf_end = buf + buf_size;
	while (buf_out < buf_end - 1 && (!in_text_end || in_text < in_text_end) && *in_text) {
		unsigned int c;
		in_text += charFromUtf8(&c, in_text, in_text_end);
		if (c == 0)
			break;
		if (c < 0x10000) // FIXME: Losing characters that don't fit in 2 bytes.
			*buf_out++ = (CodeEdit::CodePoint)c;
	}
	*buf_out = 0;
	if (in_text_remaining)
		*in_text_remaining = in_text;

	return (int)(buf_out - buf);
}

static int strToUtf8(char* buf, int buf_size, const CodeEdit::CodePoint* in_text, const CodeEdit::CodePoint* in_text_end) {
	char* buf_out = buf;
	const char* buf_end = buf + buf_size;
	while (buf_out < buf_end - 1 && (!in_text_end || in_text < in_text_end) && *in_text) {
		unsigned int c = (unsigned int)(*in_text++);
		if (c < 0x80)
			*buf_out++ = (char)c;
		else
			buf_out += charToUtf8(buf_out, (int)(buf_end - buf_out - 1), c);
	}
	*buf_out = 0;

	return (int)(buf_out - buf);
}

static eastl::string strToUtf8StdStr(const CodeEdit::CodePoint* in_text, const CodeEdit::CodePoint* in_text_end) {
	int sz = countUtf8BytesFromStr(in_text, in_text_end);
	eastl::string result;
	result.resize((size_t)(sz + 1));
	strToUtf8(&(*result.begin()), (int)result.length(), in_text, in_text_end);

	return result;
}

static int expectUtf8Char(const char* ch) {
#define _TAKE(__ch, __c, __r) do { __c = *__ch++; __r++; } while(0)
#define _COPY(__ch, __c, __r, __cp) do { _TAKE(__ch, __c, __r); __cp = (__cp << 6) | ((unsigned char)__c & 0x3fu); } while(0)
#define _TRANS(__m, __cp, __g) do { __cp &= ((__g[(unsigned char)c] & __m) != 0); } while(0)
#define _TAIL(__ch, __c, __r, __cp, __g) do { _COPY(__ch, __c, __r, __cp); _TRANS(0x70, __cp, __g); } while(0)

	static const unsigned char range[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
		0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
		8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 11, 6, 6, 6, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
	};

	int result = 0;
	unsigned codepoint = 0;
	unsigned char type = 0;
	char c = 0;

	if (!ch)
		return 0;

	_TAKE(ch, c, result);
	if (!(c & 0x80)) {
		codepoint = (unsigned char)c;

		return 1;
	}

	type = range[(unsigned char)c];
	codepoint = (0xff >> type) & (unsigned char)c;

	switch (type) {
	case 2: _TAIL(ch, c, result, codepoint, range); return result;
	case 3: _TAIL(ch, c, result, codepoint, range); _TAIL(ch, c, result, codepoint, range); return result;
	case 4: _COPY(ch, c, result, codepoint); _TRANS(0x50, codepoint, range); _TAIL(ch, c, result, codepoint, range); return result;
	case 5: _COPY(ch, c, result, codepoint); _TRANS(0x10, codepoint, range); _TAIL(ch, c, result, codepoint, range); _TAIL(ch, c, result, codepoint, range); return result;
	case 6: _TAIL(ch, c, result, codepoint, range); _TAIL(ch, c, result, codepoint, range); _TAIL(ch, c, result, codepoint, range); return result;
	case 10: _COPY(ch, c, result, codepoint); _TRANS(0x20, codepoint, range); _TAIL(ch, c, result, codepoint, range); return result;
	case 11: _COPY(ch, c, result, codepoint); _TRANS(0x60, codepoint, range); _TAIL(ch, c, result, codepoint, range); _TAIL(ch, c, result, codepoint, range); return result;
	default: return 0;
	}

#undef _TAKE
#undef _COPY
#undef _TRANS
#undef _TAIL
}

static CodeEdit::Char takeUtf8Bytes(const char* str, int n) {
	union { CodeEdit::Char ui; char ch[4]; } u;
	u.ui = 0;
	for (int i = 0; i < n; ++i)
		u.ch[i] = str[i];
	for (int i = n; i < 4; ++i)
		u.ch[i] = '\0';

	return u.ui;
}

static int countUtf8Bytes(CodeEdit::Char chr) {
	int ret = 0;
	union { CodeEdit::Char ui; char ch[4]; } u;
	u.ui = chr;
	for (int i = 0; i < 4; ++i) {
		if (u.ch[i])
			ret = i + 1;
		else
			break;
	}

	return ret;
}

static int appendUtf8ToStdStr(eastl::string& buf, CodeEdit::Char chr) {
	int ret = 0;
	union { CodeEdit::Char ui; char ch[4]; } u;
	u.ui = chr;
	for (int i = 0; i < 4; ++i) {
		if (u.ch[i]) {
			buf.push_back(u.ch[i]);

			ret = i + 1;
		}
		else {
			break;
		}
	}

	return ret;
}

struct Clipper {
private:
	SDL_Renderer* renderer = nullptr;
	SDL_Rect clip;

public:
	Clipper(SDL_Renderer* rnd, const SDL_Rect& rect) : renderer(rnd) {
		SDL_RenderGetClipRect(renderer, &clip);
		SDL_RenderSetClipRect(renderer, &rect);
	}
	~Clipper() {
		if (clip.w == 0 || clip.h == 0)
			SDL_RenderSetClipRect(renderer, nullptr);
		else
			SDL_RenderSetClipRect(renderer, &clip);
	}
};

CodeEdit::LanguageDefinition CodeEdit::LanguageDefinition::AngelScript(void) {
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited) {
		static const char* const keywords[] = {
			"and", "abstract", "auto", "bool", "break", "case", "cast", "class", "const", "continue", "default", "do", "double", "else", "enum", "false", "final", "float", "for",
			"from", "funcdef", "function", "get", "if", "import", "in", "inout", "int", "interface", "int8", "int16", "int32", "int64", "is", "mixin", "namespace", "not",
			"null", "or", "out", "override", "private", "protected", "return", "set", "shared", "super", "switch", "this ", "true", "typedef", "uint", "uint8", "uint16", "uint32",
			"uint64", "void", "while", "xor"
		};

		for (const char* const k : keywords)
			langDef.keys.insert(k);

		static const char* const identifiers[] = {
			"cos", "sin", "tab", "acos", "asin", "atan", "atan2", "cosh", "sinh", "tanh", "log", "log10", "pow", "sqrt", "abs", "ceil", "floor", "fraction", "closeTo", "fpFromIEEE", "fpToIEEE",
			"complex", "opEquals", "opAddAssign", "opSubAssign", "opMulAssign", "opDivAssign", "opAdd", "opSub", "opMul", "opDiv"
		};
		for (const char* const k : identifiers) {
			Identifier id;
			id.declaration = "Built-in function";
			langDef.ids.insert(eastl::make_pair(eastl::string(k), id));
		}

		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("//.*", PaletteIndex::Comment));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

		langDef.commentStart = "/*";
		langDef.commentEnd = "*/";

		langDef.caseSensitive = true;

		langDef.name = "AngelScript";

		inited = true;
	}

	return langDef;
}

CodeEdit::LanguageDefinition CodeEdit::LanguageDefinition::C(void) {
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited) {
		static const char* const keywords[] = {
			"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short",
			"signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
			"_Noreturn", "_Static_assert", "_Thread_local"
		};
		for (const char* const k : keywords)
			langDef.keys.insert(k);

		static const char* const identifiers[] = {
			"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
			"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper"
		};
		for (const char* const k : identifiers) {
			Identifier id;
			id.declaration = "Built-in function";
			langDef.ids.insert(eastl::make_pair(eastl::string(k), id));
		}

		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("//.*", PaletteIndex::Comment));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[ \t]*#[ \\t]*[a-zA-Z_]+", PaletteIndex::Preprocessor));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::CharLiteral));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

		langDef.commentStart = "/*";
		langDef.commentEnd = "*/";

		langDef.caseSensitive = true;

		langDef.name = "C";

		inited = true;
	}

	return langDef;
}

CodeEdit::LanguageDefinition CodeEdit::LanguageDefinition::CPlusPlus(void) {
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited) {
		static const char* const cppKeywords[] = {
			"alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char16_t", "char32_t", "class",
			"compl", "concept", "const", "constexpr", "const_cast", "continue", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float",
			"for", "friend", "goto", "if", "import", "inline", "int", "long", "module", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
			"register", "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "synchronized", "template", "this", "thread_local",
			"throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
		};
		for (const char* const k : cppKeywords)
			langDef.keys.insert(k);

		static const char* const identifiers[] = {
			"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
			"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "printf", "sprintf", "snprintf", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper",
			"std", "string", "vector", "map", "unordered_map", "set", "unordered_set", "min", "max"
		};
		for (const char* const k : identifiers) {
			Identifier id;
			id.declaration = "Built-in function";
			langDef.ids.insert(eastl::make_pair(eastl::string(k), id));
		}

		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("//.*", PaletteIndex::Comment));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[ \t]*#[ \\t]*[a-zA-Z_]+", PaletteIndex::Preprocessor));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::CharLiteral));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

		langDef.commentStart = "/*";
		langDef.commentEnd = "*/";

		langDef.caseSensitive = true;

		langDef.name = "C++";

		inited = true;
	}

	return langDef;
}

CodeEdit::LanguageDefinition CodeEdit::LanguageDefinition::GLSL(void) {
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited) {
		static const char* const keywords[] = {
			"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short",
			"signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
			"_Noreturn", "_Static_assert", "_Thread_local"
		};
		for (const char* const k : keywords)
			langDef.keys.insert(k);

		static const char* const identifiers[] = {
			"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
			"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper"
		};
		for (const char* const k : identifiers) {
			Identifier id;
			id.declaration = "Built-in function";
			langDef.ids.insert(eastl::make_pair(eastl::string(k), id));
		}

		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("//.*", PaletteIndex::Comment));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[ \t]*#[ \\t]*[a-zA-Z_]+", PaletteIndex::Preprocessor));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::CharLiteral));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

		langDef.commentStart = "/*";
		langDef.commentEnd = "*/";

		langDef.caseSensitive = true;

		langDef.name = "GLSL";

		inited = true;
	}

	return langDef;
}

CodeEdit::LanguageDefinition CodeEdit::LanguageDefinition::HLSL(void) {
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited) {
		static const char* const keywords[] = {
			"AppendStructuredBuffer", "asm", "asm_fragment", "BlendState", "bool", "break", "Buffer", "ByteAddressBuffer", "case", "cbuffer", "centroid", "class", "column_major", "compile", "compile_fragment",
			"CompileShader", "const", "continue", "ComputeShader", "ConsumeStructuredBuffer", "default", "DepthStencilState", "DepthStencilView", "discard", "do", "double", "DomainShader", "dword", "else",
			"export", "extern", "false", "float", "for", "fxgroup", "GeometryShader", "groupshared", "half", "Hullshader", "if", "in", "inline", "inout", "InputPatch", "int", "interface", "line", "lineadj",
			"linear", "LineStream", "matrix", "min16float", "min10float", "min16int", "min12int", "min16uint", "namespace", "nointerpolation", "noperspective", "NULL", "out", "OutputPatch", "packoffset",
			"pass", "pixelfragment", "PixelShader", "point", "PointStream", "precise", "RasterizerState", "RenderTargetView", "return", "register", "row_major", "RWBuffer", "RWByteAddressBuffer", "RWStructuredBuffer",
			"RWTexture1D", "RWTexture1DArray", "RWTexture2D", "RWTexture2DArray", "RWTexture3D", "sample", "sampler", "SamplerState", "SamplerComparisonState", "shared", "snorm", "stateblock", "stateblock_state",
			"static", "string", "struct", "switch", "StructuredBuffer", "tbuffer", "technique", "technique10", "technique11", "texture", "Texture1D", "Texture1DArray", "Texture2D", "Texture2DArray", "Texture2DMS",
			"Texture2DMSArray", "Texture3D", "TextureCube", "TextureCubeArray", "true", "typedef", "triangle", "triangleadj", "TriangleStream", "uint", "uniform", "unorm", "unsigned", "vector", "vertexfragment",
			"VertexShader", "void", "volatile", "while",
			"bool1","bool2","bool3","bool4","double1","double2","double3","double4", "float1", "float2", "float3", "float4", "int1", "int2", "int3", "int4", "in", "out", "inout",
			"uint1", "uint2", "uint3", "uint4", "dword1", "dword2", "dword3", "dword4", "half1", "half2", "half3", "half4",
			"float1x1","float2x1","float3x1","float4x1","float1x2","float2x2","float3x2","float4x2",
			"float1x3","float2x3","float3x3","float4x3","float1x4","float2x4","float3x4","float4x4",
			"half1x1","half2x1","half3x1","half4x1","half1x2","half2x2","half3x2","half4x2",
			"half1x3","half2x3","half3x3","half4x3","half1x4","half2x4","half3x4","half4x4",
		};
		for (const char* const k : keywords)
			langDef.keys.insert(k);

		static const char* const identifiers[] = {
			"abort", "abs", "acos", "all", "AllMemoryBarrier", "AllMemoryBarrierWithGroupSync", "any", "asdouble", "asfloat", "asin", "asint", "asint", "asuint",
			"asuint", "atan", "atan2", "ceil", "CheckAccessFullyMapped", "clamp", "clip", "cos", "cosh", "countbits", "cross", "D3DCOLORtoUBYTE4", "ddx",
			"ddx_coarse", "ddx_fine", "ddy", "ddy_coarse", "ddy_fine", "degrees", "determinant", "DeviceMemoryBarrier", "DeviceMemoryBarrierWithGroupSync",
			"distance", "dot", "dst", "errorf", "EvaluateAttributeAtCentroid", "EvaluateAttributeAtSample", "EvaluateAttributeSnapped", "exp", "exp2",
			"f16tof32", "f32tof16", "faceforward", "firstbithigh", "firstbitlow", "floor", "fma", "fmod", "frac", "frexp", "fwidth", "GetRenderTargetSampleCount",
			"GetRenderTargetSamplePosition", "GroupMemoryBarrier", "GroupMemoryBarrierWithGroupSync", "InterlockedAdd", "InterlockedAnd", "InterlockedCompareExchange",
			"InterlockedCompareStore", "InterlockedExchange", "InterlockedMax", "InterlockedMin", "InterlockedOr", "InterlockedXor", "isfinite", "isinf", "isnan",
			"ldexp", "length", "lerp", "lit", "log", "log10", "log2", "mad", "max", "min", "modf", "msad4", "mul", "noise", "normalize", "pow", "printf",
			"Process2DQuadTessFactorsAvg", "Process2DQuadTessFactorsMax", "Process2DQuadTessFactorsMin", "ProcessIsolineTessFactors", "ProcessQuadTessFactorsAvg",
			"ProcessQuadTessFactorsMax", "ProcessQuadTessFactorsMin", "ProcessTriTessFactorsAvg", "ProcessTriTessFactorsMax", "ProcessTriTessFactorsMin",
			"radians", "rcp", "reflect", "refract", "reversebits", "round", "rsqrt", "saturate", "sign", "sin", "sincos", "sinh", "smoothstep", "sqrt", "step",
			"tan", "tanh", "tex1D", "tex1D", "tex1Dbias", "tex1Dgrad", "tex1Dlod", "tex1Dproj", "tex2D", "tex2D", "tex2Dbias", "tex2Dgrad", "tex2Dlod", "tex2Dproj",
			"tex3D", "tex3D", "tex3Dbias", "tex3Dgrad", "tex3Dlod", "tex3Dproj", "texCUBE", "texCUBE", "texCUBEbias", "texCUBEgrad", "texCUBElod", "texCUBEproj", "transpose", "trunc"
		};
		for (const char* const k : identifiers) {
			Identifier id;
			id.declaration = "Built-in function";
			langDef.ids.insert(eastl::make_pair(eastl::string(k), id));
		}

		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("//.*", PaletteIndex::Comment));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[ \t]*#[ \\t]*[a-zA-Z_]+", PaletteIndex::Preprocessor));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::CharLiteral));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

		langDef.commentStart = "/*";
		langDef.commentEnd = "*/";

		langDef.caseSensitive = true;

		langDef.name = "HLSL";

		inited = true;
	}

	return langDef;
}

CodeEdit::LanguageDefinition CodeEdit::LanguageDefinition::Lua(void) {
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited) {
		static const char* const keywords[] = {
			"and", "break", "do", "", "else", "elseif", "end", "false", "for", "function", "if", "in", "", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until", "while"
		};

		for (const char* const k : keywords)
			langDef.keys.insert(k);

		static const char* const identifiers[] = {
			"assert", "collectgarbage", "dofile", "error", "getmetatable", "ipairs", "loadfile", "load", "loadstring",  "next",  "pairs",  "pcall",  "print",  "rawequal",  "rawlen",  "rawget",  "rawset",
			"select",  "setmetatable",  "tonumber",  "tostring",  "type",  "xpcall",  "_G",  "_VERSION","arshift", "band", "bnot", "bor", "bxor", "btest", "extract", "lrotate", "lshift", "replace",
			"rrotate", "rshift", "create", "resume", "running", "status", "wrap", "yield", "isyieldable", "debug","getuservalue", "gethook", "getinfo", "getlocal", "getregistry", "getmetatable",
			"getupvalue", "upvaluejoin", "upvalueid", "setuservalue", "sethook", "setlocal", "setmetatable", "setupvalue", "traceback", "close", "flush", "input", "lines", "open", "output", "popen",
			"read", "tmpfile", "type", "write", "close", "flush", "lines", "read", "seek", "setvbuf", "write", "__gc", "__tostring", "abs", "acos", "asin", "atan", "ceil", "cos", "deg", "exp", "tointeger",
			"floor", "fmod", "ult", "log", "max", "min", "modf", "rad", "random", "randomseed", "sin", "sqrt", "string", "tan", "type", "atan2", "cosh", "sinh", "tanh",
			"pow", "frexp", "ldexp", "log10", "pi", "huge", "maxinteger", "mininteger", "loadlib", "searchpath", "seeall", "preload", "cpath", "path", "searchers", "loaded", "module", "require", "clock",
			"date", "difftime", "execute", "exit", "getenv", "remove", "rename", "setlocale", "time", "tmpname", "byte", "char", "dump", "find", "format", "gmatch", "gsub", "len", "lower", "match", "rep",
			"reverse", "sub", "upper", "pack", "packsize", "unpack", "concat", "maxn", "insert", "pack", "unpack", "remove", "move", "sort", "offset", "codepoint", "char", "len", "codes", "charpattern",
			"coroutine", "table", "io", "os", "string", "utf8", "bit32", "math", "debug", "package"
		};
		for (const char* const k : identifiers) {
			Identifier id;
			id.declaration = "Built-in function";
			langDef.ids.insert(eastl::make_pair(eastl::string(k), id));
		}

		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("\\-\\-.*", PaletteIndex::Comment));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("\\\'[^\\\']*\\\'", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

		langDef.commentStart = "\\-\\-\\[\\[";
		langDef.commentEnd = "\\]\\]";

		langDef.caseSensitive = true;

		langDef.name = "Lua";

		inited = true;
	}

	return langDef;
}

CodeEdit::LanguageDefinition CodeEdit::LanguageDefinition::SQL(void) {
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited) {
		static const char* const keywords[] = {
			"ADD", "EXCEPT", "PERCENT", "ALL", "EXEC", "PLAN", "ALTER", "EXECUTE", "PRECISION", "AND", "EXISTS", "PRIMARY", "ANY", "EXIT", "PRINT", "AS", "FETCH", "PROC", "ASC", "FILE", "PROCEDURE",
			"AUTHORIZATION", "FILLFACTOR", "PUBLIC", "BACKUP", "FOR", "RAISERROR", "BEGIN", "FOREIGN", "READ", "BETWEEN", "FREETEXT", "READTEXT", "BREAK", "FREETEXTTABLE", "RECONFIGURE",
			"BROWSE", "FROM", "REFERENCES", "BULK", "FULL", "REPLICATION", "BY", "FUNCTION", "RESTORE", "CASCADE", "GOTO", "RESTRICT", "CASE", "GRANT", "RETURN", "CHECK", "GROUP", "REVOKE",
			"CHECKPOINT", "HAVING", "RIGHT", "CLOSE", "HOLDLOCK", "ROLLBACK", "CLUSTERED", "IDENTITY", "ROWCOUNT", "COALESCE", "IDENTITY_INSERT", "ROWGUIDCOL", "COLLATE", "IDENTITYCOL", "RULE",
			"COLUMN", "IF", "SAVE", "COMMIT", "IN", "SCHEMA", "COMPUTE", "INDEX", "SELECT", "CONSTRAINT", "INNER", "SESSION_USER", "CONTAINS", "INSERT", "SET", "CONTAINSTABLE", "INTERSECT", "SETUSER",
			"CONTINUE", "INTO", "SHUTDOWN", "CONVERT", "IS", "SOME", "CREATE", "JOIN", "STATISTICS", "CROSS", "KEY", "SYSTEM_USER", "CURRENT", "KILL", "TABLE", "CURRENT_DATE", "LEFT", "TEXTSIZE",
			"CURRENT_TIME", "LIKE", "THEN", "CURRENT_TIMESTAMP", "LINENO", "TO", "CURRENT_USER", "LOAD", "TOP", "CURSOR", "NATIONAL", "TRAN", "DATABASE", "NOCHECK", "TRANSACTION",
			"DBCC", "NONCLUSTERED", "TRIGGER", "DEALLOCATE", "NOT", "TRUNCATE", "DECLARE", "NULL", "TSEQUAL", "DEFAULT", "NULLIF", "UNION", "DELETE", "OF", "UNIQUE", "DENY", "OFF", "UPDATE",
			"DESC", "OFFSETS", "UPDATETEXT", "DISK", "ON", "USE", "DISTINCT", "OPEN", "USER", "DISTRIBUTED", "OPENDATASOURCE", "VALUES", "DOUBLE", "OPENQUERY", "VARYING","DROP", "OPENROWSET", "VIEW",
			"DUMMY", "OPENXML", "WAITFOR", "DUMP", "OPTION", "WHEN", "ELSE", "OR", "WHERE", "END", "ORDER", "WHILE", "ERRLVL", "OUTER", "WITH", "ESCAPE", "OVER", "WRITETEXT"
		};

		for (const char* const k : keywords)
			langDef.keys.insert(k);

		static const char* const identifiers[] = {
			"ABS",  "ACOS",  "ADD_MONTHS",  "ASCII",  "ASCIISTR",  "ASIN",  "ATAN",  "ATAN2",  "AVG",  "BFILENAME",  "BIN_TO_NUM",  "BITAND",  "CARDINALITY",  "CASE",  "CAST",  "CEIL",
			"CHARTOROWID",  "CHR",  "COALESCE",  "COMPOSE",  "CONCAT",  "CONVERT",  "CORR",  "COS",  "COSH",  "COUNT",  "COVAR_POP",  "COVAR_SAMP",  "CUME_DIST",  "CURRENT_DATE",
			"CURRENT_TIMESTAMP",  "DBTIMEZONE",  "DECODE",  "DECOMPOSE",  "DENSE_RANK",  "DUMP",  "EMPTY_BLOB",  "EMPTY_CLOB",  "EXP",  "EXTRACT",  "FIRST_VALUE",  "FLOOR",  "FROM_TZ",  "GREATEST",
			"GROUP_ID",  "HEXTORAW",  "INITCAP",  "INSTR",  "INSTR2",  "INSTR4",  "INSTRB",  "INSTRC",  "LAG",  "LAST_DAY",  "LAST_VALUE",  "LEAD",  "LEAST",  "LENGTH",  "LENGTH2",  "LENGTH4",
			"LENGTHB",  "LENGTHC",  "LISTAGG",  "LN",  "LNNVL",  "LOCALTIMESTAMP",  "LOG",  "LOWER",  "LPAD",  "LTRIM",  "MAX",  "MEDIAN",  "MIN",  "MOD",  "MONTHS_BETWEEN",  "NANVL",  "NCHR",
			"NEW_TIME",  "NEXT_DAY",  "NTH_VALUE",  "NULLIF",  "NUMTODSINTERVAL",  "NUMTOYMINTERVAL",  "NVL",  "NVL2",  "POWER",  "RANK",  "RAWTOHEX",  "REGEXP_COUNT",  "REGEXP_INSTR",
			"REGEXP_REPLACE",  "REGEXP_SUBSTR",  "REMAINDER",  "REPLACE",  "ROUND",  "ROWNUM",  "RPAD",  "RTRIM",  "SESSIONTIMEZONE",  "SIGN",  "SIN",  "SINH",
			"SOUNDEX",  "SQRT",  "STDDEV",  "SUBSTR",  "SUM",  "SYS_CONTEXT",  "SYSDATE",  "SYSTIMESTAMP",  "TAN",  "TANH",  "TO_CHAR",  "TO_CLOB",  "TO_DATE",  "TO_DSINTERVAL",  "TO_LOB",
			"TO_MULTI_BYTE",  "TO_NCLOB",  "TO_NUMBER",  "TO_SINGLE_BYTE",  "TO_TIMESTAMP",  "TO_TIMESTAMP_TZ",  "TO_YMINTERVAL",  "TRANSLATE",  "TRIM",  "TRUNC", "TZ_OFFSET",  "UID",  "UPPER",
			"USER",  "USERENV",  "VAR_POP",  "VAR_SAMP",  "VARIANCE",  "VSIZE "
		};
		for (const char* const k : identifiers) {
			Identifier id;
			id.declaration = "Built-in function";
			langDef.ids.insert(eastl::make_pair(eastl::string(k), id));
		}

		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("\\-\\-.*", PaletteIndex::Comment));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("\\\'[^\\\']*\\\'", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

		langDef.commentStart = "/*";
		langDef.commentEnd = "*/";

		langDef.caseSensitive = false;

		langDef.name = "SQL";

		inited = true;
	}

	return langDef;
}

CodeEdit::LanguageDefinition CodeEdit::LanguageDefinition::BASIC8(void) {
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited) {
		static const char* const keywords[] = {
			/* Constants. */
			"nil", "true", "false",
			/* Operators. */
			"mod", "and", "or", "not", "is", "...",
			/* Keywords. */
			"let", "dim", "if", "then", "elseif", "else", "endif",
			"for", "in", "to", "step", "next", "while", "wend", "do", "until", "exit",
			"goto", "gosub", "return", "call", "def", "enddef",
			"class", "endclass", "new", "var", "reflect", "lambda",
			"import", "type",
			/* Extra keywords. */
			"typeof"
		};

		for (const char* const k : keywords)
			langDef.keys.insert(k);

		static const char* const identifiers[] = {
			/* Built-in basic. */
			"abs", "sgn", "sqr", "floor", "ceil", "fix", "round", "srnd", "rnd", "sin", "cos", "tan", "asin", "acos", "atan", "exp", "log",
			"asc", "chr", "left", "mid", "right", "str", "val",
			"mem", "end", "len", "get", "set", "print", "input",
			"list", "dict", "push", "pop", "back", "insert", "sort", "exists", "index_of", "remove", "clear", "clone", "to_array", "iterator", "move_next",
			/* Built-in advanced. */
			"ticks", "cpu_core_count", "now", "trace", "raise", "gc", "beep",
			/* Common. */
			"open", "close", "lock", "unlock", "read", "read_u8", "read_s8", "read_u16", "read_s16", "read_int", "read_real", "read_all", "end_of_stream",
			"peek", "poke", "pack", "unpack", "parse", "serialize", "load", "save",
			"send", "recv", "broadcast", "poll",
			"start", "abort", "play", "stop", "parent", "children",
			"create", "valid", "find", "resize", "copy_to",
			/* Algorithm. */
			"pather", "set_diagonal_cost",
			/* Audio. */
			"wave", "sfx", "use_sound_font", "set_volume",
			/* Bytes. */
			"bytes", "push_u8", "push_s8", "push_u16", "push_s16", "push_int", "push_real", "pop_u8", "pop_s8", "pop_u16", "pop_s16", "pop_int", "pop_real",
			/* Coroutine. */
			"coroutine", "yield", "get_error", "wait_for",
			/* Complex. */
			"rgba",
			/* Database. */
			"data", "restore",
			"database", "has_table", "query", "exec",
			/* Date time. */
			"sleep",
			/* Driver. */
			"driver",
			"set_clearer", "set_interpolator", "set_orderby",
			"update_with",
			"load_resource", "load_blank",
			"flip_x", "flip_y", "set_flip_condition",
			/* File. */
			"file", "read_line", "write", "write_u8", "write_s8", "write_u16", "write_s16", "write_int", "write_real", "write_line",
			/* GUI. */
			"msgbox", "open_file_dialog", "save_file_dialog", "pick_directory_dialog",
			/* Image. */
			"image",
			/* IO. */
			"get_document_path", "combine_path",
			"file_info", "directory_info", "get_files", "get_directories", "get_full_path", "get_parent_path", "get_file_name", "get_ext_name", "get_dir_name", "is_blank",
			/* JSON. */
			"json", "json_bool",
			/* Math. */
			"pi", "deg", "rad", "min", "max",
			"vec2", "vec3", "vec4", "vec4_from_euler", "vec4_from_axis_angle", "mat4x4", "mat4x4_from_euler", "mat4x4_from_axis_angle", "mat4x4_from_scale", "mat4x4_from_translation",
			"mat4x4_lookat", "mat4x4_ortho", "mat4x4_perspective", "mat4x4_perspective_fov",
			"dot", "cross", "normalize", "length_square", "length", "distance_square", "distance", "lerp", "identity", "conjugate", "transpose", "inverse",
			/* Network. */
			"net",
			/* Persistence. */
			"persist",
			/* System. */
			"set_output_visible",
			"get_app_directory", "get_current_directory",
			"set_clipboard_text", "get_clipboard_text", "has_clipboard_text",
			"os", "sys",
			/* Text. */
			"lcase", "ucase", "split", "starts_with", "ends_with", "like", "regex",
			/* Utils. */
			"assert", "swap", "iif",
			"band", "bor", "bnot", "bxor", "shl", "shr",
			/* Web. */
			"web", "web_request",
			/* Zip. */
			"zip", "unpack_all",
			"compress", "decompress",
			/* Plugin primitives. */
			"cls", "sync", "col", "clip", "camera", "font",
			"text", "plot", "line", "circ", "circfill", "ellipse", "ellipsefill", "arc", "arcfill", "rect", "rectfill", "tri", "trifill", "tritex", "quad", "quadfill",
			"spr", "sspr", "step_on", "map", "img", "simg",
			"pget", "pset", "sget", "sset", "mget", "mset", "iget", "iset",
			"key", "keyp", "touch", "btn", "btnp"
		};
		for (const char* const k : identifiers) {
			Identifier id;
			//id.declaration = "Library function";
			langDef.ids.insert(eastl::make_pair(eastl::string(k), id));
		}

		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("'.*|rem$|rem[ \t](.*)?", PaletteIndex::Comment));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("0[x][0-9a-f]+", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([e][+-]?[0-9]+)?", PaletteIndex::Number));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[_]*[a-z_][a-z0-9_]*[$]?", PaletteIndex::Identifier));
		langDef.tokenRegexPatterns.push_back(eastl::make_pair<eastl::string, PaletteIndex>("[\\~\\*\\/\\+\\-\\^\\(\\)\\=\\<\\>\\.]", PaletteIndex::Punctuation));

		langDef.commentStart = "'[";
		langDef.commentEnd = "']";
		langDef.commentException = '\'';

		langDef.caseSensitive = false;

		langDef.name = "BASIC8";

		inited = true;
	}

	return langDef;
}

bool CodeEdit::UndoRecord::similar(const UndoRecord* o) const {
	if (o == nullptr)
		return false;
	if (type != o->type)
		return false;
	if (start.line != o->start.line || end.line != o->end.line)
		return false;

	auto isalpha = [](char ch) {
		return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
	};
	auto isnum = [](char ch) {
		return ch >= '0' && ch <= '9';
	};
	auto isblank = [](char ch) {
		return ch >= ' ' && ch <= '\t';
	};
	if ((content.length() == 1 && isalpha(*content.begin())) &&
		(o->content.length() == 1 && isalpha(*o->content.begin()))) {
		return true;
	}
	if ((content.length() == 1 && isnum(*content.begin())) &&
		(o->content.length() == 1 && isnum(*o->content.begin()))) {
		return true;
	}
	if ((content.length() == 1 && isblank(*content.begin())) &&
		(o->content.length() == 1 && isblank(*o->content.begin()))) {
		return true;
	}
	if (content.length() > 1 && content.length() <= 4 && o->content.length() > 1 && o->content.length() <= 4) {
		int l = expectUtf8Char(content.c_str());
		int r = expectUtf8Char(o->content.c_str());
		if ((int)content.length() == l && (int)o->content.length() == r)
			return true;
	}

	return false;
}

void CodeEdit::UndoRecord::undo(CodeEdit* editor) {
	if (!content.empty()) {
		switch (type) {
		case UndoType::Add: {
			editor->_state = after;

			editor->removeRange(start, end);
			editor->colorize(start.line - 1, end.line - start.line + 2);

			if (!overwritten.empty()) {
				Coordinates st = start;
				editor->insertTextAt(st, overwritten.c_str());
				editor->colorize(st.line - 1, end.line - st.line + 1);
			}

			editor->onChanged(start, start, -1);
		}

						  break;
		case UndoType::Remove: {
			Coordinates st = start;
			editor->insertTextAt(st, content.c_str());
			editor->colorize(st.line - 1, end.line - st.line + 2);

			editor->onChanged(st, end, -1);
		}

							 break;
		case UndoType::indent: {
			assert(end.line - start.line + 1 == (int)content.length());

			for (int i = start.line; i <= end.line; ++i) {
				Line& line = editor->_codeLines[i];
				const char op = content[i - start.line];
				if (op == 0) {
					// Does nothing.
				}
				else if (op == eastl::numeric_limits<char>::max()) {
					const Glyph& g = *line.begin();
					if (g.character == '\t') {
						line.erase(line.begin());
					}
					else {
						assert(false);
					}

					Coordinates pos(i, 0);
					editor->onChanged(pos, pos, -1);
				}
				else {
					assert(false);
				}
			}
		}

							 break;
		case UndoType::unindent: {
			assert(end.line - start.line + 1 == (int)content.length());

			for (int i = start.line; i <= end.line; ++i) {
				Line& line = editor->_codeLines[i];
				char op = content[i - start.line];
				if (op == 0) {
					// Does nothing.
				}
				else if (op == eastl::numeric_limits<char>::max()) {
					line.insert(line.begin(), Glyph('\t', PaletteIndex::Default));

					Coordinates pos(i, 0);
					editor->onChanged(pos, pos, -1);
				}
				else if (op > 0) {
					while (op--) {
						line.insert(line.begin(), Glyph(' ', PaletteIndex::Default));
					}

					Coordinates pos(i, 0);
					editor->onChanged(pos, pos, -1);
				}
				else {
					assert(false);
				}
			}
		}

							   break;
		}
	}

	editor->_state = before;
	editor->ensureCursorVisible();

	editor->onModified();
}

void CodeEdit::UndoRecord::redo(CodeEdit* editor) {
	if (!content.empty()) {
		switch (type) {
		case UndoType::Add: {
			editor->_state = before;

			editor->removeSelection();

			Coordinates st = start;
			editor->insertTextAt(st, content.c_str());
			editor->colorize(st.line - 1, end.line - st.line + 1);

			editor->onChanged(st, end, 1);
		}

						  break;
		case UndoType::Remove: {
			editor->removeRange(start, end);
			editor->colorize(start.line - 1, end.line - start.line + 1);

			editor->onChanged(start, start, 1);
		}

							 break;
		case UndoType::indent: {
			assert(end.line - start.line + 1 == (int)content.length());

			for (int i = start.line; i <= end.line; ++i) {
				Line& line = editor->_codeLines[i];
				const char op = content[i - start.line];
				if (op == 0) {
					// Does nothing.
				}
				else if (op == eastl::numeric_limits<char>::max()) {
					line.insert(line.begin(), Glyph('\t', PaletteIndex::Default));

					Coordinates pos(i, 0);
					editor->onChanged(pos, pos, 1);
				}
				else {
					assert(false);
				}
			}
		}

							 break;
		case UndoType::unindent: {
			assert(end.line - start.line + 1 == (int)content.length());

			for (int i = start.line; i <= end.line; ++i) {
				Line& line = editor->_codeLines[i];
				char op = content[i - start.line];
				if (op == 0) {
					// Does nothing.
				}
				else if (op == eastl::numeric_limits<char>::max()) {
					const Glyph& g = *line.begin();
					if (g.character == '\t') {
						line.erase(line.begin());
					}
					else {
						assert(false);
					}

					Coordinates pos(i, 0);
					editor->onChanged(pos, pos, 1);
				}
				else if (op > 0) {
					while (op--) {
						const Glyph& g = *line.begin();
						if (g.character == ' ') {
							line.erase(line.begin());
						}
						else {
							assert(false);
						}
					}

					Coordinates pos(i, 0);
					editor->onChanged(pos, pos, 1);
				}
				else {
					assert(false);
				}
			}
		}

							   break;
		}
	}

	editor->_state = after;
	editor->ensureCursorVisible();

	editor->onModified();
}

CodeEdit::Glyph::Glyph(CodeEdit::Char ch, PaletteIndex idx) : character(ch), colorIndex(idx), multiLineComment(false) {
	if (ch <= 255) {
		codepoint = (CodeEdit::CodePoint)ch;
	}
	else {
		const char* txt = (const char*)(&ch);
		const char* tend = txt + sizeof(CodeEdit::Char);
		unsigned int cp = 0;
		charFromUtf8(&cp, txt, tend);
		codepoint = (CodeEdit::CodePoint)cp;
	}
}

void CodeEdit::Line::clear(void) {
	changed = LineState::None;
}

void CodeEdit::Line::change(void) {
	changed = LineState::Edited;
}

void CodeEdit::Line::save(void) {
	changed = LineState::EditedSaved;
}

void CodeEdit::Line::revert(void) {
	changed = LineState::EditedReverted;
}

CodeEdit::CodeEdit() {
	setPalette(DarkPalette());
	setLanguageDefinition(LanguageDefinition::C());
	_codeLines.push_back(Line());
}

CodeEdit::~CodeEdit() {
}

const CodeEdit::LanguageDefinition& CodeEdit::getLanguageDefinition(void) const {
	return _langDef;
}

CodeEdit::LanguageDefinition& CodeEdit::getLanguageDefinition(void) {
	return _langDef;
}

CodeEdit::LanguageDefinition& CodeEdit::setLanguageDefinition(const LanguageDefinition& langDef) {
	_langDef = langDef;

	_regexes.clear();

	//eastl::regex_constants::syntax_option_type opt = eastl::regex_constants::optimize;
	//if (!_langDef.caseSensitive)
		//opt |= eastl::regex_constants::icase;
	for (const LanguageDefinition::TokenRegexString& r : _langDef.tokenRegexPatterns)
	{
		_regexes.push_back(eastl::make_pair(jp::Regex(r.first), r.second));
		//_regexes.push_back(eastl::make_pair(eastl::regex(r.first, opt), r.second));
	}
	return _langDef;
}

const CodeEdit::Palette& CodeEdit::getPalette(void) const {
	return _palette;
}

void CodeEdit::setPalette(const Palette& val) {
	_palette = val;
}

const CodeEdit::Vec2& CodeEdit::getCharacterSize(void) const {
	return _characterSize;
}

void CodeEdit::setCharacterSize(const Vec2& val) {
	_characterSize = val;
}

void CodeEdit::setErrorMarkers(const ErrorMarkers& val) {
	_errorMarkers = val;
}

void CodeEdit::clearErrorMarkers(void) {
	if (!_errorMarkers.empty())
		_errorMarkers.clear();
}

void CodeEdit::setBreakpoints(const Breakpoints& val) {
	_breakpoints = val;
}

void CodeEdit::clearBrakpoints(void) {
	if (!_breakpoints.empty())
		_breakpoints.clear();
}

void CodeEdit::render(void* rnd) {
	SDL_Renderer* renderer = (SDL_Renderer*)rnd;

	constexpr float heightOffset = 1.0f;
	const SDL_Rect rectContent{
		(int)getWidgetPos().x, (int)getWidgetPos().y,
		(int)getWidgetSize().x, (int)getWidgetSize().y
	};
	const float offsetCode = _charAdv.x * _textStart;
	const SDL_Rect rectCode{
		(int)(getWidgetPos().x + offsetCode), (int)getWidgetPos().y,
		(int)(getWidgetSize().x - offsetCode), (int)getWidgetSize().y
	};
	Clipper clipContent(renderer, rectContent);

	_withinRender = true;

	const float xadv = _characterSize.x;
	_charAdv = Vec2(xadv, _characterSize.y + _lineSpacing);
	if (_codeLines.size() >= 10000)
		_textStart = 7;
	else if (_codeLines.size() >= 1000)
		_textStart = 6;
	else
		_textStart = 5;
	++_textStart; // For edited states.

	const bool shift = isKeyShiftDown();
	const bool ctrl = isKeyCtrlDown();
	const bool alt = isKeyAltDown();

	if (isWidgetFocused()) {
		if (_mouseCursorInput != isWidgetHovered()) {
			if (_mouseCursorChangedHandler != nullptr)
				_mouseCursorChangedHandler(isWidgetHovered());
			_mouseCursorInput = isWidgetHovered();
		}

		if (isShortcutsEnabled(ShortcutType::UndoRedo)) {
			if (!isReadonly()) {
				if (ctrl && !shift && !alt && isKeyPressed(SDLK_z)) {
					undo();
				}
				else if (ctrl && !shift && !alt && isKeyPressed(SDLK_y)) {
					redo();
				}
			}
		}

		if (isShortcutsEnabled(ShortcutType::CopyCutPaste)) {
			if (ctrl && !shift && !alt && isKeyPressed(SDLK_c))
				copy();
			else if (!isReadonly() && ctrl && !shift && !alt && isKeyPressed(SDLK_v))
				paste();
			else if (ctrl && !shift && !alt && isKeyPressed(SDLK_x))
				cut();
			else if (ctrl && !shift && !alt && isKeyPressed(SDLK_a))
				selectAll();
		}

		if (!ctrl && !alt && isKeyPressed(SDLK_UP))
			moveUp(1, shift);
		else if (!ctrl && !alt && isKeyPressed(SDLK_DOWN))
			moveDown(1, shift);
		else if (!alt && isKeyPressed(SDLK_LEFT))
			moveLeft(1, shift, ctrl);
		else if (!alt && isKeyPressed(SDLK_RIGHT))
			moveRight(1, shift, ctrl);
		else if (!alt && isKeyPressed(SDLK_PAGEUP))
			moveUp(getPageSize() - 4, shift);
		else if (!alt && isKeyPressed(SDLK_PAGEDOWN))
			moveDown(getPageSize() - 4, shift);
		else if (!alt && ctrl && isKeyPressed(SDLK_HOME))
			moveTop(shift);
		else if (ctrl && !alt && isKeyPressed(SDLK_END))
			moveBottom(shift);
		else if (!ctrl && !alt && isKeyPressed(SDLK_HOME))
			moveHome(shift);
		else if (!ctrl && !alt && isKeyPressed(SDLK_END))
			moveEnd(shift);
		else if (!isReadonly() && !ctrl && !shift && !alt && isKeyPressed(SDLK_DELETE))
			remove();
		else if (!isReadonly() && !ctrl && !shift && !alt && isKeyPressed(SDLK_BACKSPACE))
			backspace();

		if (!isReadonly()) {
			if (isKeyPressed(SDLK_RETURN) || onKeyPressed(SDLK_RETURN)) {
				if (!alt) {
					unsigned int c = '\n'; // Inserts new line.
					addInputCharacter((CodeEdit::CodePoint)c);
				}
			}
			else if (isKeyPressed(SDLK_TAB)) {
				if (hasSelection() && getSelectionLines() > 1) {
					if (!ctrl && !alt && !shift) // Indents multi-lines.
						indent();
					else if (!ctrl && !alt && shift) // Unindents multi-lines.
						unindent();
					else if (ctrl && !alt && shift) // Unindents multi-lines.
						unindent();
				}
				else {
					if (!ctrl && !alt && !shift) {
						unsigned int c = '\t'; // Inserts tab.
						addInputCharacter((CodeEdit::CodePoint)c);
					}
					else if (!ctrl && !alt && shift) {
						CodeEdit::Char cc = getCharUnderCursor();
						if (cc == '\t' || cc == ' ')
							backspace(); // Unindents single line.
					}
					else if (ctrl && !alt && shift) {
						CodeEdit::Char cc = getCharUnderCursor();
						if (cc == '\t' || cc == ' ')
							backspace(); // Unindents single line.
					}
				}
			}
		}

		if (!isReadonly() && (!_inputCharacters.empty() && _inputCharacters[0])) {
			const eastl::string tmp = strToUtf8StdStr((const CodeEdit::CodePoint*)_inputCharacters.c_str(), nullptr);
			char* str = (char*)tmp.c_str();
			while (*str && str < (char*)tmp.c_str() + tmp.length()) {
				int n = expectUtf8Char(str);
				CodeEdit::Char c = takeUtf8Bytes(str, n);
				if (c != 0) {
					if (c == '\r')
						c = '\n';
					enterCharacter(c);
				}
				str += n;
			}
		}
		_inputCharacters.clear();
	}

	if (isWidgetHovered()) {
		if (!shift && !alt) {
			if (isMousePressed()) {
				_state.cursorPosition = _interactiveStart = _interactiveEnd = sanitizeCoordinates(screenPosToCoordinates(getMousePos()));
				if (ctrl)
					_wordSelectionMode = true;
				setSelection(_interactiveStart, _interactiveEnd, _wordSelectionMode);
			}
			if (isMouseDoubleClicked() && !ctrl) {
				_state.cursorPosition = _interactiveStart = _interactiveEnd = sanitizeCoordinates(screenPosToCoordinates(getMousePos()));
				_wordSelectionMode = true;
				setSelection(_interactiveStart, _interactiveEnd, _wordSelectionMode);
				_state.cursorPosition = _state.selectionEnd;
			}
			else if (isMouseDragging() && isMouseDown()) {
				_state.cursorPosition = _interactiveEnd = sanitizeCoordinates(screenPosToCoordinates(getMousePos()));
				setSelection(_interactiveStart, _interactiveEnd, _wordSelectionMode);
			}
		}
		else if (shift) {
			if (isMousePressed()) {
				_state.cursorPosition = _interactiveEnd = sanitizeCoordinates(screenPosToCoordinates(getMousePos()));
				setSelection(_interactiveStart, _interactiveEnd, _wordSelectionMode);
			}
		}

		if (!isMouseDown()) {
			_wordSelectionMode = false;
		}
	}

	colorizeInternal();

	static eastl::string buffer; // Shared.
	Vec2 contentSize = getWidgetSize();
	int appendIndex = 0;
	int longest = _textStart;

	const Vec2 cursorScreenPos = getWidgetPos();
	const float scrollX = getScrollX();
	const float scrollY = getScrollY();

	int lineNo = (int)floor(scrollY / _charAdv.y);
	const int lineMax = eastl::max(0, eastl::min((int)_codeLines.size() - 1, lineNo + (int)ceil(contentSize.y / _charAdv.y)));
	if (!_codeLines.empty()) {
		while (lineNo <= lineMax) {
			Vec2 lineStartScreenPos(
				cursorScreenPos.x - scrollX,
				cursorScreenPos.y - scrollY + lineNo * _charAdv.y
			);
			Vec2 textScreenPos(
				lineStartScreenPos.x + _charAdv.x * _textStart,
				lineStartScreenPos.y
			);

			const Line& line = _codeLines[lineNo];
			longest = eastl::max(_textStart + textDistanceToLineStart(Coordinates(lineNo, (int)line.size())), longest);
			int columnNo = 0;
			const Coordinates lineStartCoord(lineNo, 0);
			const Coordinates lineEndCoord(lineNo, (int)line.size());

			int sstart = -1;
			int ssend = -1;

			assert(_state.selectionStart <= _state.selectionEnd);
			if (_state.selectionStart <= lineEndCoord)
				sstart = _state.selectionStart > lineStartCoord ? textDistanceToLineStart(_state.selectionStart) : 0;
			if (_state.selectionEnd > lineStartCoord)
				ssend = textDistanceToLineStart(_state.selectionEnd < lineEndCoord ? _state.selectionEnd : lineEndCoord);

			if (_state.selectionEnd.line > lineNo)
				++ssend;

			if (sstart != -1 && ssend != -1 && sstart < ssend) {
				Clipper clipCode(renderer, rectCode);

				const Vec2 vstart(lineStartScreenPos.x + (_charAdv.x) * (sstart + _textStart), lineStartScreenPos.y);
				const Vec2 vend(lineStartScreenPos.x + (_charAdv.x) * (ssend + _textStart), lineStartScreenPos.y + _charAdv.y - heightOffset);
				boxColor(renderer, (Sint16)vstart.x, (Sint16)vstart.y, (Sint16)vend.x, (Sint16)vend.y, _palette[(int)PaletteIndex::Selection]);
			}

			const Vec2 start(lineStartScreenPos.x + scrollX, lineStartScreenPos.y);

			if (_breakpoints.find(lineNo + 1) != _breakpoints.end()) {
				Clipper clipCode(renderer, rectCode);

				const Vec2 end(lineStartScreenPos.x + contentSize.x + 2.0f * scrollX, lineStartScreenPos.y + _charAdv.y - heightOffset);
				boxColor(renderer, (Sint16)start.x, (Sint16)start.y, (Sint16)end.x, (Sint16)end.y, _palette[(int)PaletteIndex::Breakpoint]);
			}

			auto errorIt = _errorMarkers.find(lineNo + 1);
			if (errorIt != _errorMarkers.end()) {
				Clipper clipCode(renderer, rectCode);

				const Vec2 end(lineStartScreenPos.x + contentSize.x + 2.0f * scrollX, lineStartScreenPos.y + _charAdv.y - heightOffset);
				boxColor(renderer, (Sint16)start.x, (Sint16)start.y, (Sint16)end.x, (Sint16)end.y, _palette[(int)PaletteIndex::ErrorMarker]);

				//if (_tooltipEnabled) {
				//	if (ImGui::IsMouseHoveringRect(lineStartScreenPos, end)) {
				//		ImGui::BeginTooltip();
				//		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
				//		ImGui::Text("Error at line %d:", errorIt->first);
				//		ImGui::PopStyleColor();
				//		ImGui::Separator();
				//		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.2f, 1.0f));
				//		ImGui::Text("%s", errorIt->second.c_str());
				//		ImGui::PopStyleColor();
				//		ImGui::EndTooltip();
				//	}
				//}
			}

			static char buf[16]; // Shared.
			switch (_textStart - 1) {
			case 5: snprintf(buf, countof(buf), "%4d", lineNo + 1); break;
			case 6: snprintf(buf, countof(buf), "%5d", lineNo + 1); break;
			default: snprintf(buf, countof(buf), "%6d", lineNo + 1); break;
			}
			stringColor(renderer, (Sint16)(lineStartScreenPos.x + scrollX), (Sint16)lineStartScreenPos.y, buf, _palette[(int)PaletteIndex::LineNumber]);
			switch (line.changed) {
			case LineState::None:
				// Does nothing.

				break;
			case LineState::Edited:
				boxColor(
					renderer,
					(Sint16)(lineStartScreenPos.x + scrollX + _charAdv.x * (_textStart - 1)), (Sint16)lineStartScreenPos.y,
					(Sint16)(lineStartScreenPos.x + scrollX + _charAdv.x * (_textStart - 1) + _charAdv.x * 0.5f), (Sint16)(lineStartScreenPos.y + _charAdv.y - heightOffset),
					_palette[(int)PaletteIndex::LineEdited]
				);

				break;
			case LineState::EditedSaved:
				boxColor(
					renderer,
					(Sint16)(lineStartScreenPos.x + scrollX + _charAdv.x * (_textStart - 1)), (Sint16)lineStartScreenPos.y,
					(Sint16)(lineStartScreenPos.x + scrollX + _charAdv.x * (_textStart - 1) + _charAdv.x * 0.5f), (Sint16)(lineStartScreenPos.y + _charAdv.y - heightOffset),
					_palette[(int)PaletteIndex::LineEditedSaved]
				);

				break;
			case LineState::EditedReverted:
				boxColor(
					renderer,
					(Sint16)(lineStartScreenPos.x + scrollX + _charAdv.x * (_textStart - 1)), (Sint16)lineStartScreenPos.y,
					(Sint16)(lineStartScreenPos.x + scrollX + _charAdv.x * (_textStart - 1) + _charAdv.x * 0.5f), (Sint16)(lineStartScreenPos.y + _charAdv.y - heightOffset),
					_palette[(int)PaletteIndex::LineEditedReverted]
				);

				break;
			}

			if (_state.cursorPosition.line == lineNo) {
				const bool focused = isWidgetFocused();

				if (!hasSelection()) {
					const Vec2 end(start.x + contentSize.x, start.y + _charAdv.y - heightOffset);
					boxColor(renderer, (Sint16)start.x, (Sint16)start.y, (Sint16)end.x, (Sint16)end.y, _palette[(int)(focused ? PaletteIndex::CurrentLineFill : PaletteIndex::CurrentLineFillInactive)]);
					rectangleColor(renderer, (Sint16)start.x, (Sint16)start.y, (Sint16)end.x, (Sint16)end.y + 1, _palette[(int)PaletteIndex::CurrentLineEdge]);
				}

				const int cx = textDistanceToLineStart(_state.cursorPosition);

				if (focused) {
					static auto timeStart = eastl::chrono::system_clock::now(); // Shared.
					auto timeEnd = eastl::chrono::system_clock::now();
					auto diff = timeEnd - timeStart;
					const long long elapsed = eastl::chrono::duration_cast<eastl::chrono::milliseconds>(diff).count();
					const Vec2 cstart(lineStartScreenPos.x + _charAdv.x * (cx + _textStart), lineStartScreenPos.y);
					if (elapsed > 400) {
						Clipper clipCode(renderer, rectCode);

						const Vec2 cend(lineStartScreenPos.x + _charAdv.x * (cx + _textStart) + (_overwrite ? _charAdv.x : 1.0f), lineStartScreenPos.y + _charAdv.y - heightOffset);
						boxColor(renderer, (Sint16)cstart.x, (Sint16)cstart.y, (Sint16)cend.x, (Sint16)cend.y, _palette[(int)PaletteIndex::Cursor]);
						if (elapsed > 800)
							timeStart = timeEnd;
					}
				}
			}

			appendIndex = 0;
			PaletteIndex prevColor = line.empty() ? PaletteIndex::Default : (line[0].multiLineComment ? PaletteIndex::MultiLineComment : line[0].colorIndex);

			Clipper clipCode(renderer, rectCode);

			int width = 0;
			for (const Glyph& glyph : line) {
				const PaletteIndex color = glyph.multiLineComment ? PaletteIndex::MultiLineComment : glyph.colorIndex;

				if (color != prevColor && !buffer.empty()) {
					stringColor(renderer, (Sint16)textScreenPos.x, (Sint16)textScreenPos.y, buffer.c_str(), _palette[(uint8_t)prevColor]);
					textScreenPos.x += _charAdv.x * width;
					buffer.clear();
					prevColor = color;
					width = 0;
				}
				appendIndex = appendBuffer(buffer, glyph, appendIndex, width);
				++columnNo;
			}

			if (!buffer.empty()) {
				stringColor(renderer, (Sint16)textScreenPos.x, (Sint16)textScreenPos.y, buffer.c_str(), _palette[(uint8_t)prevColor]);
				buffer.clear();
			}
			appendIndex = 0;
			lineStartScreenPos.y += _charAdv.y;
			textScreenPos.x = lineStartScreenPos.x + _charAdv.x * _textStart;
			textScreenPos.y = lineStartScreenPos.y;
			++lineNo;
		}

		if (_tooltipEnabled) {
			//std::string id = getWordAt(screenPosToCoordinates(getMousePos()));
			//if (!id.empty()) {
			//	auto it = _langDef.ids.find(id);
			//	if (it != _langDef.ids.end() && !it->second.declaration.empty()) {
			//		ImGui::BeginTooltip();
			//		ImGui::TextUnformatted(it->second.declaration.c_str());
			//		ImGui::EndTooltip();
			//	} else {
			//		auto pi = _langDef.preprocIds.find(id);
			//		if (pi != _langDef.preprocIds.end() && !pi->second.declaration.empty()) {
			//			ImGui::BeginTooltip();
			//			ImGui::TextUnformatted(pi->second.declaration.c_str());
			//			ImGui::EndTooltip();
			//		}
			//	}
			//}
		}
	}

	_contentSize = Vec2((longest + 1) * _charAdv.x, _codeLines.size() * _charAdv.y);
	if (_scrollX > _contentSize.x - _widgetSize.x)
		_scrollX = eastl::max(_contentSize.x - _widgetSize.x, 0.0f);

	if (_scrollToCursor) {
		ensureCursorVisible(_scrollToCursor == -1);
		_widgetFocused = true;
		_scrollToCursor = 0;
	}

	_withinRender = false;
}

void CodeEdit::setKeyPressedHandler(const KeyPressed& handler) {
	_keyPressedHandler = handler;
}

void CodeEdit::setColorizedHandler(const Colorized& handler) {
	_colorizedHandler = handler;
}

void CodeEdit::setModifiedHandler(const Modified& handler) {
	_modifiedHandler = handler;
}

void CodeEdit::setMouseCursorChangedHandler(const MouseCursorChanged& handler) {
	_mouseCursorChangedHandler = handler;
}

void CodeEdit::setChangesCleared(void) {
	for (Line& line : _codeLines) {
		if (line.changed == LineState::Edited || line.changed == LineState::EditedSaved || line.changed == LineState::EditedReverted)
			line.clear();
	}
}

void CodeEdit::setChangesSaved(void) {
	_savedIndex = _undoIndex;

	for (Line& line : _codeLines) {
		if (line.changed == LineState::Edited || line.changed == LineState::EditedReverted)
			line.save();
	}
}

bool CodeEdit::isChangesSaved(void) const {
	return _savedIndex == _undoIndex;
}

eastl::vector<eastl::string> CodeEdit::getTextLines(bool includeComment, bool includeString) const {
	eastl::vector<eastl::string> result;
	for (const Line& ln : _codeLines) {
		result.push_back(eastl::string());
		eastl::string& str = result.back();
		for (const Glyph& g : ln) {
			const bool multilinecomment =
				g.colorIndex == PaletteIndex::Comment || g.colorIndex == PaletteIndex::MultiLineComment ||
				ln[0].multiLineComment;
			if (!includeComment && multilinecomment)
				continue;
			if (!includeString && (g.colorIndex == PaletteIndex::String))
				continue;

			appendUtf8ToStdStr(str, g.character);
		}
	}

	return result;
}

eastl::string CodeEdit::getText(const char* newLine) const {
	return getText(Coordinates(), Coordinates((int)_codeLines.size(), 0), newLine);
}

void CodeEdit::setText(const eastl::string& txt) {
	_codeLines.clear();
	char* str = (char*)txt.c_str();
	while (str < (char*)txt.c_str() + txt.length()) {
		int n = expectUtf8Char(str);
		CodeEdit::Char c = takeUtf8Bytes(str, n);
		if (_codeLines.empty())
			_codeLines.push_back(Line());
		if (c == '\n')
			_codeLines.push_back(Line());
		else
			_codeLines.back().push_back(Glyph(c, PaletteIndex::Default));
		str += n;
	}
	if (_codeLines.empty())
		_codeLines.push_back(Line());

	clearUndoRedoStack();

	colorize();
}

void CodeEdit::insertText(const char* val) {
	if (val == nullptr)
		return;

	Coordinates pos = getActualCursorCoordinates();
	Coordinates start = eastl::min(pos, _state.selectionStart);
	int totalLines = pos.line - start.line;

	totalLines += insertTextAt(pos, val);

	setSelection(pos, pos);
	setCursorPosition(pos);
	colorize(start.line - 1, totalLines + 2);
}

int CodeEdit::getTotalLines(void) const {
	return (int)_codeLines.size();
}

int CodeEdit::getColumnsAt(int ln) const {
	if (ln < 0 || ln >= (int)_codeLines.size())
		return 0;

	const Line& l = _codeLines[ln];

	return (int)l.size();
}

CodeEdit::Coordinates CodeEdit::getCursorPosition(void) const {
	return getActualCursorCoordinates();
}

void CodeEdit::setCursorPosition(const Coordinates& pos) {
	if (_state.cursorPosition != pos) {
		_state.cursorPosition = pos;
		ensureCursorVisible();
	}
}

void CodeEdit::ensureCursorVisible(bool force) {
	if (!_withinRender) {
		_scrollToCursor = force ? -1 : 1;

		return;
	}

	float scrollX = getScrollX();
	float scrollY = getScrollY();

	float height = getWidgetSize().y;
	float width = getWidgetSize().x;

	int top = 1 + (int)ceil(scrollY / _charAdv.y);
	int bottom = (int)ceil((scrollY + height) / _charAdv.y);

	int left = (int)ceil(scrollX / _charAdv.x);
	int right = (int)ceil((scrollX + width) / _charAdv.x);

	Coordinates pos = getActualCursorCoordinates();
	int len = textDistanceToLineStart(pos);

	const int bottomBorder = 1;
	if (pos.line < top || force)
		setScrollY(eastl::max(0.0f, (pos.line - 1) * _charAdv.y));
	else if (pos.line > bottom - bottomBorder)
		setScrollY(eastl::max(0.0f, (pos.line + bottomBorder) * _charAdv.y - height));
	if (len < left)
		setScrollX(eastl::max(0.0f, len * _charAdv.x));
	else if (len > right - _textStart - 1)
		setScrollX(eastl::max(0.0f, (len + _textStart + 1) * _charAdv.x - width));
}

eastl::string CodeEdit::getWordUnderCursor(void) const {
	const Coordinates c = getCursorPosition();

	return getWordAt(c);
}

void CodeEdit::setSelectionStart(const Coordinates& pos) {
	_state.selectionStart = sanitizeCoordinates(pos);
	if (_state.selectionStart > _state.selectionEnd)
		eastl::swap(_state.selectionStart, _state.selectionEnd);
}

void CodeEdit::setSelectionEnd(const Coordinates& pos) {
	_state.selectionEnd = sanitizeCoordinates(pos);
	if (_state.selectionStart > _state.selectionEnd)
		eastl::swap(_state.selectionStart, _state.selectionEnd);
}

void CodeEdit::setSelection(const Coordinates& start, const Coordinates& end, bool wordMode) {
	_state.selectionStart = sanitizeCoordinates(start);
	_state.selectionEnd = sanitizeCoordinates(end);
	if (start > end)
		eastl::swap(_state.selectionStart, _state.selectionEnd);

	if (wordMode) {
		_state.selectionStart = findWordStart(_state.selectionStart);
		if (!isOnWordBoundary(_state.selectionEnd))
			_state.selectionEnd = findWordEnd(findWordStart(_state.selectionEnd));
	}
}

void CodeEdit::selectWordUnderCursor(void) {
	Coordinates c = getCursorPosition();
	setSelection(findWordStart(c), findWordEnd(c));
}

void CodeEdit::selectAll(void) {
	setSelection(Coordinates(0, 0), Coordinates((int)_codeLines.size(), 0));
}

bool CodeEdit::hasSelection(void) const {
	return _state.selectionEnd > _state.selectionStart;
}

void CodeEdit::getSelection(Coordinates& start, Coordinates& end) {
	start = _state.selectionStart;
	end = _state.selectionEnd;
}

eastl::string CodeEdit::getSelectionText(void) const {
	return getText(_state.selectionStart, _state.selectionEnd);
}

int CodeEdit::getSelectionLines(void) const {
	if (!hasSelection())
		return 0;

	return abs(_state.selectionEnd.line - _state.selectionStart.line) + 1;
}

bool CodeEdit::isUtf8SupportEnabled(void) const {
	return _utf8SupportEnabled;
}

void CodeEdit::setUtf8SupportEnabled(bool val) {
	_utf8SupportEnabled = val;
}

bool CodeEdit::isOverwrite(void) const {
	return _overwrite;
}

void CodeEdit::setOverwrite(bool val) {
	_overwrite = val;
}

bool CodeEdit::isReadonly(void) const {
	return _readonly;
}

void CodeEdit::setReadonly(bool val) {
	_readonly = val;
}

bool CodeEdit::isShortcutsEnabled(ShortcutType type) const {
	return !!(_shortcutsEnabled & type);
}

void CodeEdit::enableShortcut(ShortcutType type) {
	_shortcutsEnabled = (ShortcutType)(_shortcutsEnabled | type);
}

void CodeEdit::disableShortcut(ShortcutType type) {
	_shortcutsEnabled = (ShortcutType)(_shortcutsEnabled & ~type);
}

bool CodeEdit::isTooltipEnabled(void) const {
	return _tooltipEnabled;
}

void CodeEdit::setTooltipEnabled(bool en) {
	_tooltipEnabled = en;
}

void CodeEdit::moveUp(int amount, bool select) {
	Coordinates oldPos = _state.cursorPosition;
	_state.cursorPosition.line = eastl::max(0, _state.cursorPosition.line - amount);
	if (oldPos != _state.cursorPosition) {
		if (select) {
			if (oldPos == _interactiveStart) {
				_interactiveStart = _state.cursorPosition;
			}
			else if (oldPos == _interactiveEnd) {
				_interactiveEnd = _state.cursorPosition;
			}
			else {
				_interactiveStart = _state.cursorPosition;
				_interactiveEnd = oldPos;
			}
		}
		else {
			_interactiveStart = _interactiveEnd = _state.cursorPosition;
		}
		setSelection(_interactiveStart, _interactiveEnd);

		ensureCursorVisible();
	}
}

void CodeEdit::moveDown(int amount, bool select) {
	assert(_state.cursorPosition.column >= 0);
	Coordinates oldPos = _state.cursorPosition;
	_state.cursorPosition.line = eastl::max(0, eastl::min((int)_codeLines.size() - 1, _state.cursorPosition.line + amount));

	if (_state.cursorPosition != oldPos) {
		if (select) {
			if (oldPos == _interactiveEnd) {
				_interactiveEnd = _state.cursorPosition;
			}
			else if (oldPos == _interactiveStart) {
				_interactiveStart = _state.cursorPosition;
			}
			else {
				_interactiveStart = oldPos;
				_interactiveEnd = _state.cursorPosition;
			}
		}
		else {
			_interactiveStart = _interactiveEnd = _state.cursorPosition;
		}
		setSelection(_interactiveStart, _interactiveEnd);

		ensureCursorVisible();
	}
}

void CodeEdit::moveLeft(int amount, bool select, bool wordMode) {
	if (_codeLines.empty())
		return;

	Coordinates oldPos = _state.cursorPosition;
	_state.cursorPosition = getActualCursorCoordinates();

	while (amount-- > 0) {
		if (_state.cursorPosition.column == 0) {
			if (_state.cursorPosition.line > 0) {
				--_state.cursorPosition.line;
				_state.cursorPosition.column = (int)_codeLines[_state.cursorPosition.line].size();
			}
		}
		else {
			_state.cursorPosition.column = eastl::max(0, _state.cursorPosition.column - 1);
			if (wordMode)
				_state.cursorPosition = findWordStart(_state.cursorPosition);
		}
	}

	assert(_state.cursorPosition.column >= 0);
	if (select) {
		if (oldPos == _interactiveStart) {
			_interactiveStart = _state.cursorPosition;
		}
		else if (oldPos == _interactiveEnd) {
			_interactiveEnd = _state.cursorPosition;
		}
		else {
			_interactiveStart = _state.cursorPosition;
			_interactiveEnd = oldPos;
		}
	}
	else {
		_interactiveStart = _interactiveEnd = _state.cursorPosition;
	}
	setSelection(_interactiveStart, _interactiveEnd, select && wordMode);

	ensureCursorVisible();
}

void CodeEdit::moveRight(int amount, bool select, bool wordMode) {
	Coordinates oldPos = _state.cursorPosition;

	if (_codeLines.empty())
		return;

	while (amount-- > 0) {
		const Line& line = _codeLines[_state.cursorPosition.line];
		if (_state.cursorPosition.column >= (int)line.size()) {
			if (_state.cursorPosition.line < (int)_codeLines.size() - 1) {
				_state.cursorPosition.line = eastl::max(0, eastl::min((int)_codeLines.size() - 1, _state.cursorPosition.line + 1));
				_state.cursorPosition.column = 0;
			}
		}
		else {
			_state.cursorPosition.column = eastl::max(0, eastl::min((int)line.size(), _state.cursorPosition.column + 1));
			if (wordMode)
				_state.cursorPosition = findWordEnd(_state.cursorPosition);
		}
	}

	if (select) {
		if (oldPos == _interactiveEnd) {
			_interactiveEnd = sanitizeCoordinates(_state.cursorPosition);
		}
		else if (oldPos == _interactiveStart) {
			_interactiveStart = _state.cursorPosition;
		}
		else {
			_interactiveStart = oldPos;
			_interactiveEnd = _state.cursorPosition;
		}
	}
	else {
		_interactiveStart = _interactiveEnd = _state.cursorPosition;
	}
	setSelection(_interactiveStart, _interactiveEnd, select && wordMode);

	ensureCursorVisible();
}

void CodeEdit::moveTop(bool select) {
	Coordinates oldPos = _state.cursorPosition;
	setCursorPosition(Coordinates(0, 0));

	if (_state.cursorPosition != oldPos) {
		if (select) {
			_interactiveEnd = oldPos;
			_interactiveStart = _state.cursorPosition;
		}
		else {
			_interactiveStart = _interactiveEnd = _state.cursorPosition;
		}
		setSelection(_interactiveStart, _interactiveEnd);
	}
}

void CodeEdit::CodeEdit::moveBottom(bool select) {
	Coordinates oldPos = getCursorPosition();
	Coordinates newPos((int)_codeLines.size() - 1, (int)_codeLines.back().size());
	setCursorPosition(newPos);
	if (select) {
		_interactiveStart = oldPos;
		_interactiveEnd = newPos;
	}
	else {
		_interactiveStart = _interactiveEnd = newPos;
	}
	setSelection(_interactiveStart, _interactiveEnd);
}

void CodeEdit::moveHome(bool select) {
	Coordinates oldPos = _state.cursorPosition;
	setCursorPosition(Coordinates(_state.cursorPosition.line, 0));

	if (_state.cursorPosition != oldPos) {
		if (select) {
			if (oldPos == _interactiveStart) {
				_interactiveStart = _state.cursorPosition;
			}
			else if (oldPos == _interactiveEnd) {
				_interactiveEnd = _state.cursorPosition;
			}
			else {
				_interactiveStart = _state.cursorPosition;
				_interactiveEnd = oldPos;
			}
		}
		else {
			_interactiveStart = _interactiveEnd = _state.cursorPosition;
		}
		setSelection(_interactiveStart, _interactiveEnd);
	}
}

void CodeEdit::moveEnd(bool select) {
	Coordinates oldPos = _state.cursorPosition;
	setCursorPosition(Coordinates(_state.cursorPosition.line, (int)_codeLines[oldPos.line].size()));

	if (_state.cursorPosition != oldPos) {
		if (select) {
			if (oldPos == _interactiveEnd) {
				_interactiveEnd = _state.cursorPosition;
			}
			else if (oldPos == _interactiveStart) {
				_interactiveStart = _state.cursorPosition;
			}
			else {
				_interactiveStart = oldPos;
				_interactiveEnd = _state.cursorPosition;
			}
		}
		else {
			_interactiveStart = _interactiveEnd = _state.cursorPosition;
		}
		setSelection(_interactiveStart, _interactiveEnd);
	}
}

void CodeEdit::copy(void) {
	if (hasSelection()) {
		SDL_SetClipboardText(getSelectionText().c_str());
	}
	else {
		if (!_codeLines.empty()) {
			eastl::string str;
			const Line& line = _codeLines[getActualCursorCoordinates().line];
			for (const Glyph& g : line) {
				appendUtf8ToStdStr(str, g.character);
			}
			SDL_SetClipboardText(str.c_str());
		}
	}
}

void CodeEdit::cut(void) {
	if (isReadonly()) {
		copy();
	}
	else {
		if (hasSelection()) {
			UndoRecord u;
			u.type = UndoType::Remove;
			u.before = _state;

			u.content = getSelectionText();
			u.start = _state.selectionStart;
			u.end = _state.selectionEnd;

			copy();
			removeSelection();

			u.after = _state;
			addUndo(u);

			onModified();

			Coordinates pos = u.start < u.end ? u.start : u.end;
			onChanged(pos, pos, 0);
		}
	}
}

void CodeEdit::paste(void) {
	const char* const clipText = SDL_GetClipboardText();
	if (clipText != nullptr && strlen(clipText) > 0) {
		UndoRecord u;
		u.type = UndoType::Add;
		u.before = _state;

		if (hasSelection()) {
			u.overwritten = getSelectionText();
			removeSelection();
		}

		u.content = clipText;
		u.start = getActualCursorCoordinates();

		insertText(clipText);

		u.end = getActualCursorCoordinates();
		u.after = _state;
		addUndo(u);

		onModified();

		onChanged(u.start, u.end, 0);
	}
}

void CodeEdit::remove(void) {
	assert(!_readonly);

	if (_codeLines.empty())
		return;

	UndoRecord u;
	u.type = UndoType::Remove;
	u.before = _state;

	if (hasSelection()) {
		u.content = getSelectionText();
		u.start = _state.selectionStart;
		u.end = _state.selectionEnd;
		removeSelection();

		Coordinates pos = _state.selectionStart < _state.selectionEnd ? _state.selectionStart : _state.selectionEnd;
		onChanged(pos, pos, 0);
	}
	else {
		Coordinates pos = getActualCursorCoordinates();
		setCursorPosition(pos);
		Line& line = _codeLines[pos.line];

		if (pos.column == (int)line.size()) {
			if (pos.line == (int)_codeLines.size() - 1)
				return;

			u.content = '\n';
			u.start = u.end = getActualCursorCoordinates();
			advance(u.end);

			const Line& nextLine = _codeLines[pos.line + 1];
			line.insert(line.end(), nextLine.begin(), nextLine.end());
			removeLine(pos.line + 1);
		}
		else {
			u.content.clear();
			appendUtf8ToStdStr(u.content, line[pos.column].character);
			u.start = u.end = getActualCursorCoordinates();
			u.end.column++;

			line.erase(line.begin() + pos.column);
		}

		colorize(pos.line, 1);

		onChanged(pos, pos, 0);
	}

	u.after = _state;
	addUndo(u);

	onModified();
}

void CodeEdit::indent(void) {
	if (isReadonly())
		return;

	if (hasSelection() && getSelectionLines() > 1) {
		UndoRecord u;
		u.type = UndoType::indent;
		u.before = _state;

		u.start = _state.selectionStart;
		u.end = _state.selectionEnd;

		for (int i = u.start.line; i <= u.end.line; ++i) {
			Line& line = _codeLines[i];
			if (line.empty()) {
				u.content.push_back(0);

				continue;
			}
			line.insert(line.begin(), Glyph('\t', PaletteIndex::Default));
			u.content.push_back(eastl::numeric_limits<char>::max());

			Coordinates pos(i, 0);
			onChanged(pos, pos, 0);
		}

		_state.selectionEnd.column = (int)_codeLines[_state.selectionEnd.line].size();

		u.after = _state;
		addUndo(u);

		onModified();
	}
}

void CodeEdit::unindent(void) {
	if (isReadonly())
		return;

	if (hasSelection() && getSelectionLines() > 1) {
		UndoRecord u;
		u.type = UndoType::unindent;
		u.before = _state;

		u.start = _state.selectionStart;
		u.end = _state.selectionEnd;

		int affectedLines = 0;
		for (int i = u.start.line; i <= u.end.line; ++i) {
			Line& line = _codeLines[i];
			if (line.empty()) {
				u.content.push_back(0);

				continue;
			}

			const Glyph& g = *line.begin();
			if (g.character == '\t') {
				line.erase(line.begin());
				u.content.push_back(eastl::numeric_limits<char>::max());
				++affectedLines;

				Coordinates pos(i, 0);
				onChanged(pos, pos, 0);
			}
			else if (g.character == ' ') {
				int k = 0;
				for (int j = 0; j < _tabSize; ++j, ++k) {
					if (line.empty())
						break;
					const Glyph& h = *line.begin();
					if (h.character != ' ')
						break;
					line.erase(line.begin());
				}
				u.content.push_back((char)k);
				if (k)
					++affectedLines;

				Coordinates pos(i, 0);
				onChanged(pos, pos, 0);
			}
			else {
				u.content.push_back(0);
			}
		}
		if (affectedLines > 0) {
			const Line& line = _codeLines[_state.selectionEnd.line];
			if ((int)line.size() < _state.selectionEnd.column)
				_state.selectionEnd.column = (int)line.size();
		}

		u.after = _state;
		if (affectedLines > 0) {
			addUndo(u);

			onModified();
		}
	}
}

void CodeEdit::clearUndoRedoStack(void) {
	_undoBuf.clear();
	_undoIndex = 0;
	_savedIndex = 0;
}

bool CodeEdit::canUndo(void) const {
	return _undoIndex > 0;
}

bool CodeEdit::canRedo(void) const {
	return _undoIndex < (int)_undoBuf.size();
}

void CodeEdit::undo(int steps) {
#if CODE_EDIT_MERGE_UNDO_REDO
	if (steps == 1) {
		UndoRecord r;
		UndoRecord* p = nullptr;
		while (canUndo() && (!p || _undoBuf[_undoIndex - 1].similar(p))) {
			if (p == nullptr) {
				p = &r;
				*p = _undoBuf[_undoIndex - 1];
			}
			_undoBuf[--_undoIndex].undo(this);
		}

		return;
	}
#endif /* CODE_EDIT_MERGE_UNDO_REDO */

	while (canUndo() && steps-- > 0) {
		_undoBuf[--_undoIndex].undo(this);
	}
}

void CodeEdit::redo(int steps) {
#if CODE_EDIT_MERGE_UNDO_REDO
	if (steps == 1) {
		UndoRecord r;
		UndoRecord* p = nullptr;
		while (canRedo() && (!p || (_undoIndex + 1 <= (int)_undoBuf.size() && _undoBuf[_undoIndex].similar(p)))) {
			if (p == nullptr && _undoIndex + 1 < (int)_undoBuf.size()) {
				p = &r;
				*p = _undoBuf[_undoIndex + 1];
			}
			_undoBuf[_undoIndex++].redo(this);
		}

		return;
	}
#endif /* CODE_EDIT_MERGE_UNDO_REDO */

	while (canRedo() && steps-- > 0) {
		_undoBuf[_undoIndex++].redo(this);
	}
}

const CodeEdit::Vec2& CodeEdit::getWidgetPos(void) const {
	return _widgetPos;
}

void CodeEdit::setWidgetPos(const Vec2& pos) {
	_widgetPos = pos;
}

const CodeEdit::Vec2& CodeEdit::getWidgetSize(void) const {
	return _widgetSize;
}

void CodeEdit::setWidgetSize(const Vec2& sz) {
	_widgetSize = sz;
}

bool CodeEdit::isWidgetFocused(void) const {
	return _widgetFocused;
}

void CodeEdit::setWidgetFocused(bool val) {
	_widgetFocused = val;
}

bool CodeEdit::isWidgetHovered(void) const {
	if (!_widgetHoverable)
		return false;

	const Vec2& mpos = getMousePos();
	const Vec2& wpos = getWidgetPos();
	const Vec2& wsz = getWidgetSize();

	return (mpos.x > wpos.x && mpos.x <= wpos.x + wsz.x) && (mpos.y > wpos.y && mpos.y <= wpos.y + wsz.y);
}

void CodeEdit::setWidgetHoverable(bool val) {
	_widgetHoverable = val;
}

const CodeEdit::Vec2& CodeEdit::getContentSize(void) const {
	return _contentSize;
}

float CodeEdit::getScrollX(void) const {
	return _scrollX;
}

void CodeEdit::setScrollX(float val) {
	_scrollX = eastl::max(val, 0.0f);
}

float CodeEdit::getScrollY(void) const {
	return _scrollY;
}

void CodeEdit::setScrollY(float val) {
	_scrollY = eastl::max(val, 0.0f);
}

unsigned CodeEdit::getFrameCount(void) const {
	return _frameCount;
}

void CodeEdit::setFrameCount(unsigned val) {
	_frameCount = val;
}

void CodeEdit::addInputCharacter(CodeEdit::CodePoint cp) {
	_inputCharacters.push_back(cp);
}

void CodeEdit::addInputCharactersUtf8(const char* utf8Chars) {
	CodeEdit::CodePoint wchars[17];
	strFromUtf8(wchars, countof(wchars), utf8Chars, nullptr);
	for (int i = 0; i < countof(wchars) && wchars[i] != 0; i++)
		addInputCharacter(wchars[i]);
}

bool CodeEdit::isKeyShiftDown(void) const {
	return _keyShift;
}

bool CodeEdit::isKeyCtrlDown(void) const {
	return _keyCtrl;
}

bool CodeEdit::isKeyAltDown(void) const {
	return _keyAlt;
}

bool CodeEdit::isKeyDown(Keycode kbkey) const {
	const SDL_Keycode kc = kbkey;
	const SDL_Scancode scancode = SDL_GetScancodeFromKey(kc);
	if (scancode < 0 || scancode >= (SDL_Scancode)_keyStates1.size())
		return false;

	return !!_keyStates1[scancode];
}

bool CodeEdit::isKeyPressed(Keycode kbkey) const {
	bool result = true;
	int n = 0;
	const SDL_Keycode kc = kbkey;
	const SDL_Scancode scancode = SDL_GetScancodeFromKey(kc);

	do {
		if (scancode < 0 || scancode >= (SDL_Scancode)_keyStates0.size()) {
			result = false;
			break;
		}

		if (_keyStates0[scancode]) {
			++n;
		}
		else {
			result = false;
			break;
		}

		if (scancode < 0 || scancode >= (SDL_Scancode)_keyStates1.size()) {
			result = false;
			break;
		}

		if (_keyStates1[scancode]) {
			++n;
			result = false;
			break;
		}
	} while (false);

	do {
		if (_keyTimestamps.size() < _keyStates0.size())
			_keyTimestamps.resize(_keyStates0.size());
		if (scancode < 0 || scancode >= (SDL_Scancode)_keyStates0.size())
			break;
		if (result) {
			_keyTimestamps[scancode] = 0;
		}
		else if (!result && n == 2) {
			const long long now = eastl::chrono::steady_clock::now().time_since_epoch().count();
			if (_keyTimestamps[scancode] == 0)
				_keyTimestamps[scancode] = now + 300000000; // Delays 0.3s for the first continuous event.
			const long long diff = now - _keyTimestamps[scancode];
			if (diff > 50000000) { // Repeats once per 0.05s.
				_keyTimestamps[scancode] = now;
				result = true;
			}
		}
	} while (false);

	return result;
}

void CodeEdit::updateKeyStates(void) {
	int kc = 0;
	const Uint8* kbdState = SDL_GetKeyboardState(&kc);
	_keyStates0 = _keyStates1;
	if ((int)_keyStates1.size() < kc)
		_keyStates1.resize(kc, 0);
	memcpy(&_keyStates1.front(), kbdState, kc);

	const SDL_Keymod mod = SDL_GetModState();
	_keyShift = !!(KMOD_SHIFT & mod);
	_keyCtrl = !!(KMOD_CTRL & mod);
	_keyAlt = !!(KMOD_ALT & mod);
}

const CodeEdit::Vec2& CodeEdit::getMousePos(void) const {
	return _mousePos;
}

bool CodeEdit::isMousePressed(void) const {
	return _mousePressed;
}

bool CodeEdit::isMouseDoubleClicked(void) const {
	return _mouseClickCount == 2;
}

bool CodeEdit::isMouseDragging(void) const {
	return _mouseDragged;
}

bool CodeEdit::isMouseDown(void) const {
	return _mouseDown;
}

void CodeEdit::updateMouseStates(int mouseClickCount, const Vec4* frame, const Vec2* scale) {
	if (!_mouseDragged)
		_mouseClickCount = mouseClickCount;

	const Vec2 mscale = scale ? *scale : Vec2(1, 1);
	int tx = 0, ty = 0, tw = 0, th = 0;
	int x_ = 0, y_ = 0;
	int dispw = 0, disph = 0;
	tx = (int)getWidgetPos().x;
	ty = (int)getWidgetPos().y;
	tw = (int)getWidgetSize().x;
	th = (int)getWidgetSize().y;
	dispw = tw;
	disph = th;

	auto clicked = [](const Vec4* frame, const Vec2& widgetPos, const Vec2& widgetSz, const Vec2& mscale, Vec2& mousePos, Vec2& mouseDownPos, bool& mouseDown, bool& mouseClicked, bool& mouseDragged, bool& widgetFocused, int x, int y) {
		if (frame) {
			x -= (int)frame->x;
			y -= (int)frame->y;
		}
		mousePos.x = (float)x * mscale.x;
		mousePos.y = (float)y * mscale.y;
		if (!mouseDown) {
			mouseDownPos = mousePos;
			mouseClicked = true;
		}
		else {
			mouseDragged |= fabsf(mouseDownPos.x - mousePos.x) + fabsf(mouseDownPos.y - mousePos.y) > 8.0f;
			mouseClicked = false;
		}
		mouseDown = true;

		widgetFocused = (mouseDownPos.x > widgetPos.x && mouseDownPos.x <= widgetPos.x + widgetSz.x) && (mouseDownPos.y > widgetPos.y && mouseDownPos.y <= widgetPos.y + widgetSz.y);
	};

	const int n = SDL_GetNumTouchDevices();
	for (int m = 0; m < n; ++m) {
		SDL_TouchID tid = SDL_GetTouchDevice(m);
		if (tid == 0)
			continue;
		const int f = SDL_GetNumTouchFingers(tid);
		do {
			if (f < 1) break;
			SDL_Finger* finger = SDL_GetTouchFinger(tid, 0);
			if (!finger) break;

			x_ = (int)(finger->x * ((float)dispw - CODE_EDIT_EPSILON));
			y_ = (int)(finger->y * ((float)disph - CODE_EDIT_EPSILON));

			clicked(frame, getWidgetPos(), getWidgetSize(), mscale, _mousePos, _mouseDownPos, _mouseDown, _mousePressed, _mouseDragged, _widgetFocused, x_, y_);

			return;
		} while (false);
	}

	Uint32 btns = SDL_GetMouseState(&x_, &y_);
	if (!!(btns & SDL_BUTTON(SDL_BUTTON_LEFT))) {
		clicked(frame, getWidgetPos(), getWidgetSize(), mscale, _mousePos, _mouseDownPos, _mouseDown, _mousePressed, _mouseDragged, _widgetFocused, x_, y_);

		return;
	}

	if (frame) {
		x_ -= (int)frame->x;
		y_ -= (int)frame->y;
	}
	_mousePos.x = (float)x_ * mscale.x;
	_mousePos.y = (float)y_ * mscale.y;

	_mouseDown = false;
	_mouseDragged = false;
	_mousePressed = false;
}

const CodeEdit::Palette& CodeEdit::DarkPalette(void) {
	static Palette p = {
		0xffffffff, // None.
		0xffd69c56, // Keyword.
		0xffa8ceb5, // Number.
		0xff859dd6, // String.
		0xff70a0e0, // CodeEdit::Char literal.
		0xffb4b4b4, // Punctuation.
		0xff409090, // Preprocessor.
		0xffdadada, // Identifier.
		0xffb0c94e, // Known identifier.
		0xffc040a0, // Preproc identifier.
		0xff4aa657, // Comment (single line).
		0xff4aa657, // Comment (multi line).
		0xff2C2C2C, // Background.
		0xffe0e0e0, // Cursor.
		0xffa06020, // Selection.
		0x804d00ff, // ErrorMarker.
		0x40f08000, // Breakpoint.
		0xffaf912b, // Line number.
		0x40000000, // Current line fill.
		0x40808080, // Current line fill (inactive).
		0x40a0a0a0, // Current line edge.
		0xff84f2ef, // Line edited.
		0xff307457, // Line edited saved.
		0xfffa955f  // Line edited reverted.
	};

	return p;
}

const CodeEdit::Palette& CodeEdit::LightPalette(void) {
	static Palette p = {
		0xff000000, // None.
		0xffff0c06, // Keyword.
		0xff008000, // Number.
		0xff2020a0, // String.
		0xff304070, // CodeEdit::Char literal.
		0xff000000, // Punctuation.
		0xff409090, // Preprocessor.
		0xff404040, // Identifier.
		0xff606010, // Known identifier.
		0xffc040a0, // Preproc identifier.
		0xff205020, // Comment (single line).
		0xff405020, // Comment (multi line).
		0xffffffff, // Background.
		0xff000000, // Cursor.
		0xff600000, // Selection.
		0xa00010ff, // ErrorMarker.
		0x80f08000, // Breakpoint.
		0xff505000, // Line number.
		0x40000000, // Current line fill.
		0x40808080, // Current line fill (inactive).
		0x40000000, // Current line edge.
		0xff84f2ef, // Line edited.
		0xff307457, // Line edited saved.
		0xfffa955f  // Line edited reverted.
	};

	return p;
}

const CodeEdit::Palette& CodeEdit::RetroBluePalette(void) {
	static Palette p = {
		0xff00ffff, // None.
		0xffffff00, // Keyword.
		0xff00ff00, // Number.
		0xff808000, // String.
		0xff808000, // CodeEdit::Char literal.
		0xffffffff, // Punctuation.
		0xff008000, // Preprocessor.
		0xff00ffff, // Identifier.
		0xffffffff, // Known identifier.
		0xffff00ff, // Preproc identifier.
		0xff808080, // Comment (single line).
		0xff404040, // Comment (multi line).
		0xff800000, // Background.
		0xff0080ff, // Cursor.
		0xffffff00, // Selection.
		0xa00000ff, // ErrorMarker.
		0x80ff8000, // Breakpoint.
		0xff808000, // Line number.
		0x40000000, // Current line fill.
		0x40808080, // Current line fill (inactive).
		0x40000000, // Current line edge.
		0xff84f2ef, // Line edited.
		0xff307457, // Line edited saved.
		0xfffa955f  // Line edited reverted.
	};

	return p;
}

void CodeEdit::colorize(int fromLine, int lines) {
	int toLine = lines == -1 ? (int)_codeLines.size() : eastl::min((int)_codeLines.size(), fromLine + lines);
	_colorRangeMin = eastl::min(_colorRangeMin, fromLine);
	_colorRangeMax = eastl::max(_colorRangeMax, toLine);
	_colorRangeMin = eastl::max(0, _colorRangeMin);
	_colorRangeMax = eastl::max(_colorRangeMin, _colorRangeMax);
	_checkMultilineComments = getFrameCount() + COLORIZE_DELAY_FRAME_COUNT;
}

void CodeEdit::colorizeRange(int fromLine, int toLine) {
	if (_codeLines.empty() || fromLine >= toLine)
		return;

	eastl::string buffer;
	int endLine = eastl::max(0, eastl::min((int)_codeLines.size(), toLine));
	for (int i = fromLine; i < endLine; ++i) {
		bool preproc = false;
		Line& line = _codeLines[i];
		buffer.clear();
		for (Glyph& g : _codeLines[i]) {
			appendUtf8ToStdStr(buffer, g.character);
			g.colorIndex = PaletteIndex::Default;
		}
		//20201107
		//eastl::match_results<eastl::string::const_iterator> results;
		//auto last = buffer.cend();
		//for (auto first = buffer.cbegin(); first != last; ++first) {
		for (auto& p : _regexes) {
			//const eastl::regex_constants::match_flag_type flag = eastl::regex_constants::match_continuous;
			//if (eastl::regex_search<eastl::string::const_iterator>(first, last, results, p.first, flag)) 
			size_t count = 0;
			jp::RegexMatch rm;
			jp::VecNum vec_num;   //Vector to store numbered substring vectors.
			jp::VecNas vec_nas;   //Vector to store named substring Maps.
			jp::VecNtN vec_ntn;   //Vector to store Named substring to Number Maps.
			jpcre2::VecOff vec_off;
			count = rm.setRegexObject(&p.first)                          //set associated Regex object
				.addModifier("gi")                            //set various parameters
				//'invalid modifier: i' error (i is a compile modifier)
				//modifier error is harmless
				.setSubject(buffer.cbegin())                          //...
				.setNumberedSubstringVector(&vec_num)         //...
				.setMatchStartOffsetVector(&vec_off)
				.setNamedSubstringVector(&vec_nas)            //...
				.setNameToNumberMapVector(&vec_ntn)           //...
				.addPcre2Option(0)                            //...
				.match();                                     //Finally perform the match

			for (size_t i = 0; i < vec_num.size(); ++i)
			{
				
				int startIndex = vec_off[i];

				for (size_t j = 0; j < vec_num[i].size(); ++j)
				{	
					//int a = vec_off.size();
					auto start = vec_num[i][j].begin() - buffer.cbegin();
					auto end = vec_num[i][j].end() - buffer.cbegin();

					eastl::string id = buffer.substr(start, end - start);
					//eastl::string id = vec_num[i][j];

					PaletteIndex color = p.second;
					if (color == PaletteIndex::Identifier) {
						if (!_langDef.caseSensitive)
							eastl::transform(id.begin(), id.end(), id.begin(), CODE_EDIT_CASE_FUNC);

						if (!preproc) {
							if (_langDef.keys.find(id) != _langDef.keys.end())
								color = PaletteIndex::Keyword;
							else if (_langDef.ids.find(id) != _langDef.ids.end())
								color = PaletteIndex::KnownIdentifier;
							else if (_langDef.preprocIds.find(id) != _langDef.preprocIds.end())
								color = PaletteIndex::PreprocIdentifier;
						}
						else {
							if (_langDef.preprocIds.find(id) != _langDef.preprocIds.end())
								color = PaletteIndex::PreprocIdentifier;
							else
								color = PaletteIndex::Identifier;
						}
					}
					else if (color == PaletteIndex::Preprocessor) {
						preproc = true;
					}
					//for (int j = (int)start; j < (int)end; ++j)
					//	line[j].colorIndex = color;
					int k = 0;
					for (int j = 0; j < (int)line.size(); ++j) {
						Glyph& g = line[j];
						if (k >= (int)startIndex)
						{
							g.colorIndex = color;
						}
						k += countUtf8Bytes(g.character);

						if (k >= (int)startIndex + end - start)
							break;
					}
					//index += (int)end - (int)start - 1;

					//break;

				}


			}
			//}
		}
	}
}

void CodeEdit::colorizeInternal(void) {
	if (_codeLines.empty())
		return;

	if (_checkMultilineComments && (int)getFrameCount() > _checkMultilineComments) {
		Coordinates end((int)_codeLines.size(), 0);
		Coordinates commentStart = end;
		bool withinString = false;
		for (Coordinates i = Coordinates(0, 0); i < end; advance(i)) {
			Line& line = _codeLines[i.line];
			if (!line.empty()) {
				const Glyph& g = line[i.column];
				CodeEdit::Char c = g.character;

				bool inComment = commentStart <= i;

				if (withinString) {
					line[i.column].multiLineComment = inComment;

					if (c == '\"') {
						if (i.column + 1 < (int)line.size() && line[i.column + 1].character == '\"') {
							advance(i);
							if (i.column < (int)line.size())
								line[i.column].multiLineComment = inComment;
						}
						else {
							withinString = false;
						}
					}
					else if (c == '\\') {
						advance(i);
						if (i.column < (int)line.size())
							line[i.column].multiLineComment = inComment;
					}
				}
				else {
					if (c == '\"') {
						withinString = true;
						line[i.column].multiLineComment = inComment;
					}
					else {
						auto pred = [](const char& a, const Glyph& b) {
							return a == (const char)b.character;
						};
						bool except = false;
						const eastl::string& startStr = _langDef.commentStart;
						auto from = line.begin() + i.column;
						if (i.column + startStr.size() <= line.size()) {
							if (_langDef.commentException != '\0' && from != line.begin()) {
								auto prev = from - 1;
								if (prev->character == _langDef.commentException)
									except = true;
							}
							if (!except) {
								//20201107
								//if (eastl::equal(startStr.begin(), startStr.end(), from, from + startStr.size(), pred)) {
								if (eastl::equal(startStr.begin(), startStr.end(), from, pred)) {
									commentStart = i;
								}
							}
						}

						inComment = commentStart <= i;

						line[i.column].multiLineComment = inComment;

						except = false;
						const eastl::string& endStr = _langDef.commentEnd;
						if (i.column + 1 >= (int)endStr.size()) {
							auto till = from + 1 - endStr.size();
							if (_langDef.commentException != '\0' && till != line.begin()) {
								auto prev = till - 1;
								if (prev->character == _langDef.commentException)
									except = true;
							}
							if (!except) {
								//20201107
								//if (eastl::equal(endStr.begin(), endStr.end(), till, from + 1, pred)) {
								if (eastl::equal(endStr.begin(), endStr.end(), till, pred)) {
									commentStart = end;
								}
							}
						}
					}
				}
			}
		}
		_checkMultilineComments = 0;

		onColorized(true);

		return;
	}

	if (_colorRangeMin < _colorRangeMax) {
		int to = eastl::min(_colorRangeMin + 10, _colorRangeMax);
		colorizeRange(_colorRangeMin, to);
		_colorRangeMin = to;

		if (_colorRangeMax == _colorRangeMin) {
			_colorRangeMin = eastl::numeric_limits<int>::max();
			_colorRangeMax = 0;
		}

		onColorized(false);

		return;
	}
}

int CodeEdit::textDistanceToLineStart(const Coordinates& from) const {
	const Line& line = _codeLines[from.line];
	int len = 0;
	for (size_t it = 0u; it < line.size() && it < (unsigned)from.column; ++it) {
		const Glyph& g = line[it];
		if (g.character == '\t') {
			len = (len / _tabSize) * _tabSize + _tabSize;
		}
		else {
			if (g.character <= 255) {
				++len;
			}
			else {
				const int w = getCharacterWidth(g);
				len += w;
			}
		}
	}

	return len;
}

int CodeEdit::getPageSize(void) const {
	float height = getWidgetSize().y - 20.0f;

	return (int)floor(height / _charAdv.y);
}

CodeEdit::Coordinates CodeEdit::getActualCursorCoordinates(void) const {
	return sanitizeCoordinates(_state.cursorPosition);
}

CodeEdit::Coordinates CodeEdit::sanitizeCoordinates(const Coordinates& val) const {
	int line = eastl::max(0, eastl::min((int)_codeLines.size() - 1, val.line));
	int column = 0;
	if (!_codeLines.empty()) {
		if (line < val.line)
			column = (int)_codeLines[line].size();
		else
			column = eastl::min((int)_codeLines[line].size(), val.column);
	}

	return Coordinates(line, column);
}

void CodeEdit::advance(Coordinates& val) const {
	if (val.line < (int)_codeLines.size()) {
		const Line& line = _codeLines[val.line];

		if (val.column + 1 < (int)line.size()) {
			++val.column;
		}
		else {
			++val.line;
			val.column = 0;
		}
	}
}

int CodeEdit::getCharacterWidth(const Glyph& g) const {
	CodeEdit::CodePoint cp = g.codepoint;
	if (cp == 0) {
		const char* txt = (const char*)(&g.character);
		const char* tend = txt + sizeof(CodeEdit::Char);
		unsigned int codepoint = 0;
		charFromUtf8(&codepoint, txt, tend);
		cp = (CodeEdit::CodePoint)codepoint;
	}

	if (!isPrintable(cp)) {
		const float cadvx = _characterSize.x;
		if (cadvx > _charAdv.x)
			return CODE_EDIT_UTF8_CHAR_FACTOR;
		else
			return 1;
	}
	else {
		return 1;
	}
}

CodeEdit::Coordinates CodeEdit::screenPosToCoordinates(const Vec2& pos) const {
	float originX = getWidgetPos().x - getScrollX();
	float originY = getWidgetPos().y - getScrollY();
	Vec2 local(pos.x - originX, pos.y - originY);

	int lineNo = eastl::max(0, (int)floor(local.y / _charAdv.y));
	int columnCoord = eastl::max(0, (int)floor(local.x / _charAdv.x) - _textStart);

	int column = 0;
	if (lineNo >= 0 && lineNo < (int)_codeLines.size()) {
		const Line& line = _codeLines[lineNo];
		int distance = 0;
		while (distance < columnCoord && column < (int)line.size()) {
			const Glyph& g = line[column];
			if (g.character == '\t') {
				distance = (distance / _tabSize) * _tabSize + _tabSize;
			}
			else {
				if (g.character <= 255) {
					++distance;
				}
				else {
					const int w = getCharacterWidth(g);
					distance += w;
				}
			}
			++column;
		}
	}

	return Coordinates(lineNo, column);
}

bool CodeEdit::isOnWordBoundary(const Coordinates& at) const {
	if (at.line >= (int)_codeLines.size() || at.column == 0)
		return true;

	const Line& line = _codeLines[at.line];
	if (at.column >= (int)line.size())
		return true;

	return line[at.column].colorIndex != line[at.column - 1].colorIndex;
}

void CodeEdit::addUndo(UndoRecord& val) {
	assert(!_readonly);

	_undoBuf.resize(_undoIndex + 1);
	_undoBuf.back() = val;
	++_undoIndex;
}

eastl::string CodeEdit::getText(const Coordinates& start, const Coordinates& end, const char* newLine) const {
	eastl::string result;

	int prevLineNo = start.line;
	for (Coordinates it = start; it <= end; advance(it)) {
		if (prevLineNo != it.line && it.line < (int)_codeLines.size())
			result += newLine;

		if (it == end)
			break;

		prevLineNo = it.line;
		const Line& line = _codeLines[it.line];
		if (!line.empty() && it.column < (int)line.size()) {
			const Glyph& g = line[it.column];
			appendUtf8ToStdStr(result, g.character);
		}
	}

	return result;
}

int CodeEdit::appendBuffer(eastl::string& buf, const Glyph& g, int idx, int& width) {
	CodeEdit::Char chr = g.character;
	if (chr == '\t') {
		int num = _tabSize - idx % _tabSize;
		for (int j = num; j > 0; --j)
			buf.push_back(' ');
		width += num;

		return idx + num;
	}
	else {
		if (_utf8SupportEnabled) {
			if (appendUtf8ToStdStr(buf, chr) <= 1) {
				++width;

				return idx + 1;
			}
			else {
				const int w = getCharacterWidth(g);
				width += w;

				return idx + w;
			}
		}
		else {
			if (isPrintable(chr)) {
				appendUtf8ToStdStr(buf, chr);
			}
			else {
				const int n = appendUtf8ToStdStr(buf, chr);
				for (int i = 0; i < n; ++i)
					buf.pop_back();
				buf.push_back('?');
			}
			++width;

			return idx + 1;
		}
	}
}

int CodeEdit::insertTextAt(Coordinates& /* inout */ where, const char* val) {
	assert(!_readonly);

	int totalLines = 0;
	const char* str = val;
	while (*str != '\0') {
		if (_codeLines.empty())
			_codeLines.push_back(Line());

		int n = expectUtf8Char(str);
		CodeEdit::Char c = takeUtf8Bytes(str, n);
		if (c == '\r') {
			// Does nothing.
		}
		else if (c == '\n') {
			if (where.column < (int)_codeLines[where.line].size()) {
				Line& newLine = insertLine(where.line + 1);
				Line& line = _codeLines[where.line];
				newLine.insert(newLine.begin(), line.begin() + where.column, line.end());
				line.erase(line.begin() + where.column, line.end());
			}
			else {
				insertLine(where.line + 1);
			}
			++where.line;
			where.column = 0;
			++totalLines;
		}
		else {
			Line& line = _codeLines[where.line];
			line.insert(line.begin() + where.column, Glyph(c, PaletteIndex::Default));
			++where.column;
		}
		str += n;
	}

	return totalLines;
}

void CodeEdit::removeRange(const Coordinates& start, const Coordinates& end) {
	assert(end >= start);
	assert(!_readonly);

	if (end == start)
		return;

	if (start.line == end.line) {
		Line& line = _codeLines[start.line];
		if (end.column >= (int)line.size())
			line.erase(line.begin() + start.column, line.end());
		else
			line.erase(line.begin() + start.column, line.begin() + end.column);
	}
	else {
		Line& firstLine = _codeLines[start.line];
		Line& lastLine = _codeLines[end.line];

		firstLine.erase(firstLine.begin() + start.column, firstLine.end());
		lastLine.erase(lastLine.begin(), lastLine.begin() + end.column);

		if (start.line < end.line)
			firstLine.insert(firstLine.end(), lastLine.begin(), lastLine.end());

		if (start.line < end.line)
			removeLine(start.line + 1, end.line + 1);
	}
}

void CodeEdit::removeSelection(void) {
	assert(_state.selectionEnd >= _state.selectionStart);

	if (_state.selectionEnd == _state.selectionStart)
		return;

	removeRange(_state.selectionStart, _state.selectionEnd);

	setSelection(_state.selectionStart, _state.selectionStart);
	setCursorPosition(_state.selectionStart);
	colorize(_state.selectionStart.line, 1);

	_interactiveStart = _interactiveEnd = _state.cursorPosition;
}

CodeEdit::Line& CodeEdit::insertLine(int idx) {
	assert(!_readonly);

	Line& result = *_codeLines.insert(_codeLines.begin() + idx, Line());

	ErrorMarkers etmp;
	for (auto& i : _errorMarkers)
		etmp.insert(ErrorMarkers::value_type(i.first >= idx ? i.first + 1 : i.first, i.second));
	_errorMarkers = eastl::move(etmp);

	Breakpoints btmp;
	for (int i : _breakpoints)
		btmp.insert(i >= idx ? i + 1 : i);
	_breakpoints = eastl::move(btmp);

	return result;
}

void CodeEdit::removeLine(int start, int end) {
	assert(!_readonly);

	ErrorMarkers etmp;
	for (auto& i : _errorMarkers) {
		ErrorMarkers::value_type e(i.first >= start ? i.first - 1 : i.first, i.second);
		if (e.first >= start && e.first <= end)
			continue;

		etmp.insert(e);
	}
	_errorMarkers = eastl::move(etmp);

	Breakpoints btmp;
	for (int i : _breakpoints) {
		if (i >= start && i <= end)
			continue;

		btmp.insert(i >= start ? i - 1 : i);
	}
	_breakpoints = eastl::move(btmp);

	_codeLines.erase(_codeLines.begin() + start, _codeLines.begin() + end);
}

void CodeEdit::removeLine(int idx) {
	assert(!_readonly);

	ErrorMarkers etmp;
	for (auto& i : _errorMarkers) {
		ErrorMarkers::value_type e(i.first >= idx ? i.first - 1 : i.first, i.second);
		if (e.first == idx)
			continue;

		etmp.insert(e);
	}
	_errorMarkers = eastl::move(etmp);

	Breakpoints btmp;
	for (int i : _breakpoints) {
		if (i == idx)
			continue;

		btmp.insert(i >= idx ? i - 1 : i);
	}
	_breakpoints = eastl::move(btmp);

	_codeLines.erase(_codeLines.begin() + idx);
}

void CodeEdit::backspace(void) {
	assert(!_readonly);

	if (_codeLines.empty())
		return;

	UndoRecord u;
	u.type = UndoType::Remove;
	u.before = _state;

	if (hasSelection()) {
		u.content = getSelectionText();
		u.start = _state.selectionStart;
		u.end = _state.selectionEnd;
		removeSelection();

		onChanged(_state.selectionStart, _state.selectionStart, 0);
	}
	else {
		Coordinates pos = getActualCursorCoordinates();
		setCursorPosition(pos);

		if (_state.cursorPosition.column == 0) {
			if (_state.cursorPosition.line == 0)
				return;

			const Line& line = _codeLines[_state.cursorPosition.line];
			Line& prevLine = _codeLines[_state.cursorPosition.line - 1];
			int prevSize = (int)prevLine.size();
			prevLine.insert(prevLine.end(), line.begin(), line.end());
			removeLine(_state.cursorPosition.line);
			--_state.cursorPosition.line;
			_state.cursorPosition.column = prevSize;

			u.content = '\n';
			u.start = getActualCursorCoordinates();
			u.end = Coordinates(u.start.line + 1, 0);

			onChanged(_state.cursorPosition, _state.cursorPosition, 0);
		}
		else {
			Line& line = _codeLines[_state.cursorPosition.line];

			u.content.clear();
			appendUtf8ToStdStr(u.content, line[pos.column - 1].character);
			u.start = u.end = getActualCursorCoordinates();
			--u.start.column;

			--_state.cursorPosition.column;
			if (_state.cursorPosition.column < (int)line.size())
				line.erase(line.begin() + _state.cursorPosition.column);

			onChanged(_state.cursorPosition, _state.cursorPosition, 0);
		}
		ensureCursorVisible();
		colorize(_state.cursorPosition.line, 1);
	}

	u.after = _state;
	addUndo(u);

	onModified();
}

void CodeEdit::enterCharacter(CodeEdit::Char ch) {
	assert(!_readonly);

	UndoRecord u;
	u.type = UndoType::Add;
	u.before = _state;

	if (hasSelection()) {
		u.overwritten = getSelectionText();
		removeSelection();
	}

	const Coordinates coord = getActualCursorCoordinates();
	u.start = coord;

	if (_codeLines.empty())
		_codeLines.push_back(Line());

	if (ch == '\n') {
		insertLine(coord.line + 1);
		Line& line = _codeLines[coord.line];
		Line& newLine = _codeLines[coord.line + 1];
		newLine.insert(newLine.begin(), line.begin() + coord.column, line.end());
		line.erase(line.begin() + coord.column, line.begin() + line.size());
		_state.cursorPosition = Coordinates(coord.line + 1, 0);

		appendUtf8ToStdStr(u.content, ch);

		// Gets indent spaces from the last line.
		int indent = 0;
		bool broken = false;
		for (size_t i = 0; i < line.size(); ++i) {
			const Glyph& g = line[i];
			if (g.character == ' ') {
				++indent;
			}
			else if (g.character == '\t') {
				indent += _tabSize;
			}
			else {
				broken = true;

				break;
			}
		}
		// Automatically indents for the new line.
		const int spacec = indent % _tabSize;
		const int tabs = indent / _tabSize;
		for (int i = 0; i < spacec; ++i) {
			newLine.insert(newLine.begin(), Glyph(' ', PaletteIndex::Default));
			++_state.cursorPosition.column;
		}
		for (int i = 0; i < tabs; ++i) {
			newLine.insert(newLine.begin(), Glyph('\t', PaletteIndex::Default));
			++_state.cursorPosition.column;
		}
		for (int i = 0; i < tabs; ++i) {
			appendUtf8ToStdStr(u.content, '\t');
		}
		for (int i = 0; i < spacec; ++i) {
			appendUtf8ToStdStr(u.content, ' ');
		}

		onChanged(coord, Coordinates(coord.line + 1, 0), 0);
	}
	else {
		Line& line = _codeLines[coord.line];
		if (_overwrite && (int)line.size() > coord.column)
			line[coord.column] = Glyph(ch, PaletteIndex::Default);
		else
			line.insert(line.begin() + coord.column, Glyph(ch, PaletteIndex::Default));
		_state.cursorPosition = coord;
		++_state.cursorPosition.column;

		appendUtf8ToStdStr(u.content, ch);

		onChanged(coord, coord, 0);
	}

	u.end = getActualCursorCoordinates();
	u.after = _state;

	addUndo(u);

	colorize(coord.line - 1, 3);
	ensureCursorVisible();

	onModified();
}

CodeEdit::Coordinates CodeEdit::findWordStart(const Coordinates& from) const {
	Coordinates at = from;
	if (at.line >= (int)_codeLines.size())
		return at;

	const Line& line = _codeLines[at.line];

	if (at.column >= (int)line.size())
		return at;

	PaletteIndex cstart = (PaletteIndex)line[at.column].colorIndex;
	while (at.column > 0) {
		if (cstart != (PaletteIndex)line[at.column - 1].colorIndex)
			break;

		--at.column;
	}

	return at;
}

CodeEdit::Coordinates CodeEdit::findWordEnd(const Coordinates& from) const {
	Coordinates at = from;
	if (at.line >= (int)_codeLines.size())
		return at;

	const Line& line = _codeLines[at.line];

	if (at.column >= (int)line.size())
		return at;

	PaletteIndex cstart = (PaletteIndex)line[at.column].colorIndex;
	while (at.column < (int)line.size()) {
		if (cstart != (PaletteIndex)line[at.column].colorIndex)
			break;

		++at.column;
	}

	return at;
}

eastl::string CodeEdit::getWordAt(const Coordinates& coords) const {
	Coordinates start = findWordStart(coords);
	Coordinates end = findWordEnd(coords);

	eastl::string r;

	for (Coordinates it = start; it < end; advance(it)) {
		const Glyph& g = _codeLines[it.line][it.column];
		appendUtf8ToStdStr(r, g.character);
	}

	return r;
}

CodeEdit::Char CodeEdit::getCharUnderCursor(void) const {
	Coordinates c = getCursorPosition();
	if (--c.column < 0)
		return '\0';

	const Glyph& g = _codeLines[c.line][c.column];

	return g.character;
}

bool CodeEdit::onKeyPressed(Key key) {
	if (_keyPressedHandler == nullptr)
		return false;

	return _keyPressedHandler(key);
}

void CodeEdit::onColorized(bool multilineComment) const {
	if (_colorizedHandler == nullptr)
		return;

	_colorizedHandler(multilineComment);
}

void CodeEdit::onModified(void) const {
	if (_modifiedHandler == nullptr)
		return;

	_modifiedHandler();
}

void CodeEdit::onChanged(const Coordinates& start, const Coordinates& end, int offset) {
	Coordinates s, e;
	if (start < end) {
		s = start;
		e = end;
	}
	else {
		s = end;
		e = start;
	}
	for (int ln = s.line; ln <= e.line && ln < (int)_codeLines.size(); ++ln) {
		if (ln < 0)
			continue;

		Line& line = _codeLines[ln];
		if (offset && _savedIndex == _undoIndex) {
			line.revert();
		}
		else {
			line.change();
		}
	}
}

/* ========================================================} */
