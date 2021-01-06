/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -m 100 SMP/src/fcobjshash.gperf  */
/* Computed positions: -k'2-3' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "SMP/src/fcobjshash.gperf"

#line 13 "SMP/src/fcobjshash.gperf"
struct FcObjectTypeInfo {
int name;
int id;
};
#include <string.h>
/* maximum key range = 65, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
FcObjectTypeHash (register const char *str, register size_t len)
{
  static const unsigned char asso_values[] =
    {
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69,  9, 21, 18,
      33, 21, 69,  6, 36,  0, 69, 69,  0, 24,
       9,  0, 21, 69, 33, 15, 18,  0, 69, 69,
       0, 21,  6, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69
    };
  return len + asso_values[(unsigned char)str[2]] + asso_values[(unsigned char)str[1]];
}

struct FcObjectTypeNamePool_t
  {
    char FcObjectTypeNamePool_str4[sizeof("file")];
    char FcObjectTypeNamePool_str5[sizeof("color")];
    char FcObjectTypeNamePool_str7[sizeof("foundry")];
    char FcObjectTypeNamePool_str8[sizeof("fullname")];
    char FcObjectTypeNamePool_str9[sizeof("pixelsize")];
    char FcObjectTypeNamePool_str10[sizeof("size")];
    char FcObjectTypeNamePool_str12[sizeof("fullnamelang")];
    char FcObjectTypeNamePool_str13[sizeof("globaladvance")];
    char FcObjectTypeNamePool_str14[sizeof("slant")];
    char FcObjectTypeNamePool_str16[sizeof("hinting")];
    char FcObjectTypeNamePool_str17[sizeof("minspace")];
    char FcObjectTypeNamePool_str18[sizeof("hintstyle")];
    char FcObjectTypeNamePool_str19[sizeof("fontformat")];
    char FcObjectTypeNamePool_str20[sizeof("fontversion")];
    char FcObjectTypeNamePool_str21[sizeof("fontfeatures")];
    char FcObjectTypeNamePool_str22[sizeof("lang")];
    char FcObjectTypeNamePool_str23[sizeof("fontvariations")];
    char FcObjectTypeNamePool_str24[sizeof("dpi")];
    char FcObjectTypeNamePool_str25[sizeof("outline")];
    char FcObjectTypeNamePool_str26[sizeof("autohint")];
    char FcObjectTypeNamePool_str27[sizeof("weight")];
    char FcObjectTypeNamePool_str28[sizeof("hash")];
    char FcObjectTypeNamePool_str29[sizeof("postscriptname")];
    char FcObjectTypeNamePool_str31[sizeof("rgba")];
    char FcObjectTypeNamePool_str32[sizeof("scale")];
    char FcObjectTypeNamePool_str33[sizeof("matrix")];
    char FcObjectTypeNamePool_str34[sizeof("rasterizer")];
    char FcObjectTypeNamePool_str35[sizeof("scalable")];
    char FcObjectTypeNamePool_str36[sizeof("antialias")];
    char FcObjectTypeNamePool_str37[sizeof("spacing")];
    char FcObjectTypeNamePool_str38[sizeof("width")];
    char FcObjectTypeNamePool_str39[sizeof("family")];
    char FcObjectTypeNamePool_str40[sizeof("capability")];
    char FcObjectTypeNamePool_str41[sizeof("namelang")];
    char FcObjectTypeNamePool_str42[sizeof("aspect")];
    char FcObjectTypeNamePool_str43[sizeof("familylang")];
    char FcObjectTypeNamePool_str44[sizeof("style")];
    char FcObjectTypeNamePool_str46[sizeof("prgname")];
    char FcObjectTypeNamePool_str47[sizeof("index")];
    char FcObjectTypeNamePool_str48[sizeof("stylelang")];
    char FcObjectTypeNamePool_str49[sizeof("decorative")];
    char FcObjectTypeNamePool_str50[sizeof("variable")];
    char FcObjectTypeNamePool_str51[sizeof("symbol")];
    char FcObjectTypeNamePool_str52[sizeof("charset")];
    char FcObjectTypeNamePool_str53[sizeof("embolden")];
    char FcObjectTypeNamePool_str54[sizeof("charwidth")];
    char FcObjectTypeNamePool_str55[sizeof("charheight")];
    char FcObjectTypeNamePool_str59[sizeof("embeddedbitmap")];
    char FcObjectTypeNamePool_str60[sizeof("lcdfilter")];
    char FcObjectTypeNamePool_str68[sizeof("verticallayout")];
  };
static const struct FcObjectTypeNamePool_t FcObjectTypeNamePool_contents =
  {
    "file",
    "color",
    "foundry",
    "fullname",
    "pixelsize",
    "size",
    "fullnamelang",
    "globaladvance",
    "slant",
    "hinting",
    "minspace",
    "hintstyle",
    "fontformat",
    "fontversion",
    "fontfeatures",
    "lang",
    "fontvariations",
    "dpi",
    "outline",
    "autohint",
    "weight",
    "hash",
    "postscriptname",
    "rgba",
    "scale",
    "matrix",
    "rasterizer",
    "scalable",
    "antialias",
    "spacing",
    "width",
    "family",
    "capability",
    "namelang",
    "aspect",
    "familylang",
    "style",
    "prgname",
    "index",
    "stylelang",
    "decorative",
    "variable",
    "symbol",
    "charset",
    "embolden",
    "charwidth",
    "charheight",
    "embeddedbitmap",
    "lcdfilter",
    "verticallayout"
  };
#define FcObjectTypeNamePool ((const char *) &FcObjectTypeNamePool_contents)
const struct FcObjectTypeInfo *
FcObjectTypeLookup (register const char *str, register size_t len)
{
  enum
    {
      TOTAL_KEYWORDS = 50,
      MIN_WORD_LENGTH = 3,
      MAX_WORD_LENGTH = 14,
      MIN_HASH_VALUE = 4,
      MAX_HASH_VALUE = 68
    };

  static const struct FcObjectTypeInfo wordlist[] =
    {
      {-1}, {-1}, {-1}, {-1},
#line 38 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str4,FC_FILE_OBJECT},
#line 64 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str5,FC_COLOR_OBJECT},
      {-1},
#line 31 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str7,FC_FOUNDRY_OBJECT},
#line 22 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str8,FC_FULLNAME_OBJECT},
#line 29 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str9,FC_PIXEL_SIZE_OBJECT},
#line 27 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str10,FC_SIZE_OBJECT},
      {-1},
#line 23 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str12,FC_FULLNAMELANG_OBJECT},
#line 37 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str13,FC_GLOBAL_ADVANCE_OBJECT},
#line 24 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str14,FC_SLANT_OBJECT},
      {-1},
#line 34 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str16,FC_HINTING_OBJECT},
#line 46 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str17,FC_MINSPACE_OBJECT},
#line 33 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str18,FC_HINT_STYLE_OBJECT},
#line 54 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str19,FC_FONTFORMAT_OBJECT},
#line 52 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str20,FC_FONTVERSION_OBJECT},
#line 60 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str21,FC_FONT_FEATURES_OBJECT},
#line 51 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str22,FC_LANG_OBJECT},
#line 66 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str23,FC_FONT_VARIATIONS_OBJECT},
#line 43 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str24,FC_DPI_OBJECT},
#line 41 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str25,FC_OUTLINE_OBJECT},
#line 36 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str26,FC_AUTOHINT_OBJECT},
#line 25 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str27,FC_WEIGHT_OBJECT},
#line 62 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str28,FC_HASH_OBJECT},
#line 63 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str29,FC_POSTSCRIPT_NAME_OBJECT},
      {-1},
#line 44 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str31,FC_RGBA_OBJECT},
#line 45 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str32,FC_SCALE_OBJECT},
#line 49 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str33,FC_MATRIX_OBJECT},
#line 40 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str34,FC_RASTERIZER_OBJECT},
#line 42 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str35,FC_SCALABLE_OBJECT},
#line 32 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str36,FC_ANTIALIAS_OBJECT},
#line 30 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str37,FC_SPACING_OBJECT},
#line 26 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str38,FC_WIDTH_OBJECT},
#line 18 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str39,FC_FAMILY_OBJECT},
#line 53 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str40,FC_CAPABILITY_OBJECT},
#line 59 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str41,FC_NAMELANG_OBJECT},
#line 28 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str42,FC_ASPECT_OBJECT},
#line 19 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str43,FC_FAMILYLANG_OBJECT},
#line 20 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str44,FC_STYLE_OBJECT},
      {-1},
#line 61 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str46,FC_PRGNAME_OBJECT},
#line 39 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str47,FC_INDEX_OBJECT},
#line 21 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str48,FC_STYLELANG_OBJECT},
#line 57 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str49,FC_DECORATIVE_OBJECT},
#line 67 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str50,FC_VARIABLE_OBJECT},
#line 65 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str51,FC_SYMBOL_OBJECT},
#line 50 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str52,FC_CHARSET_OBJECT},
#line 55 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str53,FC_EMBOLDEN_OBJECT},
#line 47 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str54,FC_CHARWIDTH_OBJECT},
#line 48 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str55,FC_CHAR_HEIGHT_OBJECT},
      {-1}, {-1}, {-1},
#line 56 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str59,FC_EMBEDDED_BITMAP_OBJECT},
#line 58 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str60,FC_LCD_FILTER_OBJECT},
      {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 35 "SMP/src/fcobjshash.gperf"
      {(int)(size_t)&((struct FcObjectTypeNamePool_t *)0)->FcObjectTypeNamePool_str68,FC_VERTICAL_LAYOUT_OBJECT}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = FcObjectTypeHash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register int o = wordlist[key].name;
          if (o >= 0)
            {
              register const char *s = o + FcObjectTypeNamePool;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[key];
            }
        }
    }
  return 0;
}
