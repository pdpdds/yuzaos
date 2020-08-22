#ifndef _LOCALE_H_
#define _LOCALE_H_

typedef int LCID;
typedef int LCTYPE;

// https://github.com/gasgas4/NT_4.0_SourceCode/blob/84a03f73738328ea66034dca7cda79a857623720/nt4/private/oleauto/src/inc/olenls.h
#define LOCALE_SDATE                0x001D    /* date separator */
#define LOCALE_STIME                0x001E    /* time separator */
#define LOCALE_SSHORTDATE           0x001F    /* short date-time separator */
#define LOCALE_SLONGDATE            0x0020    /* long date-time separator */
#define LOCALE_STIMEFORMAT          0x1003    /* time format string */
#define LOCALE_IDATE                0x0021    /* short date format ordering */
#define LOCALE_ILDATE               0x0022    /* long date format ordering */
#define LOCALE_ITIME                0x0023    /* time format specifier */
#define LOCALE_ITIMEMARKPOSN        0x1005    /* time marker position */
#define LOCALE_ICENTURY             0x0024    /* century format specifier */
#define LOCALE_ITLZERO              0x0025    /* leading zeros in time field */
#define LOCALE_IDAYLZERO            0x0026    /* leading zeros in day field */
#define LOCALE_IMONLZERO            0x0027    /* leading zeros in month field */
#define LOCALE_S1159                0x0028    /* AM designator */
#define LOCALE_S2359                0x0029    /* PM designator */

#define LOCALE_SABBREVDAYNAME1      0x0031    /* abbreviated name for Monday */   
#define LOCALE_SABBREVDAYNAME2      0x0032    /* abbreviated name for Tuesday */  
#define LOCALE_SABBREVDAYNAME3      0x0033    /* abbreviated name for Wednesday */
#define LOCALE_SABBREVDAYNAME4      0x0034    /* abbreviated name for Thursday */ 
#define LOCALE_SABBREVDAYNAME5      0x0035    /* abbreviated name for Friday */   
#define LOCALE_SABBREVDAYNAME6      0x0036    /* abbreviated name for Saturday */ 
#define LOCALE_SABBREVDAYNAME7      0x0037    /* abbreviated name for Sunday */   

                                            
#define LOCALE_SDECIMAL             0x000E    /* decimal separator */
#define LOCALE_STHOUSAND            0x000F    /* thousand separator */

LCID GetUserDefaultLCID(void);

int GetLocaleInfo(
  LCID   Locale,
  LCTYPE LCType,
  LPTSTR lpLCData,
  int    cchData
);

#endif
