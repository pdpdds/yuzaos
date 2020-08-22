#include "cmd.h"

LCID GetUserDefaultLCID(void) {
	return 1;
}

int GetLocaleInfo(
  LCID   Locale,
  LCTYPE LCType,
  LPTSTR lpLCData,
  int    cchData
) {
	/*
	switch( LCType ) {
	// https://msdn.microsoft.com/en-us/library/windows/desktop/dd373757(v=vs.85).aspx
	case LOCALE_IDATE:
		strncpy(lpLCData, "1", cchData);
		break;

	// https://msdn.microsoft.com/en-us/library/windows/desktop/dd373829(v=vs.85).aspx
	case LOCALE_S2359:
		strncpy(lpLCData, "p", cchData);
		break;

	case LOCALE_S1159:
		strncpy(lpLCData, "a", cchData);
		break;

	case LOCALE_STIME:
		strncpy(lpLCData, ":", cchData);
		break;

	case LOCALE_SDATE:
		strncpy(lpLCData, "/", cchData);
		break;

	case LOCALE_SABBREVDAYNAME1:
		strncpy(lpLCData, "Mon", cchData);
		break;

	default:
		return FAILURE;
	}

	return SUCCESS;
	*/
	return 0;
}