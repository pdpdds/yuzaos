
#include <windows.h>
#include <string.h>

static const char *locale_for_search = NULL;
static LCID decide_lcid = 0;

static BOOL CALLBACK enum_lcids(LPSTR lcid_string)
{
	char language_name[64], country_name[64];
	char *endp;
	LCID lcid;

	if (locale_for_search == NULL) {
		return FALSE;
	}

	lcid = strtoul(lcid_string, &endp, 16);
	GetLocaleInfoA(lcid, LOCALE_SENGLANGUAGE, language_name, sizeof(language_name));
	endp = strrchr(language_name, '(');
	if (endp != NULL) *endp = '\0';
	GetLocaleInfoA(lcid, LOCALE_SENGCOUNTRY,  country_name, sizeof(country_name));
	if (strstr(locale_for_search, language_name) != NULL && strstr(locale_for_search, country_name) != NULL) {
		decide_lcid = lcid;
		return FALSE;
	}
	return TRUE;
}
	
LCID get_lcid(const char *locale)
{
	locale_for_search = locale;
	decide_lcid = 0;
	EnumSystemLocalesA(enum_lcids, LCID_SUPPORTED);
	locale_for_search = NULL;
	return decide_lcid;
}
