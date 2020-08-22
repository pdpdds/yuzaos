#ifndef RTC_H

#define RTC_H


#define DEF_ABS(x) ((x >= 0) ? x : -x)

#define DEF_NBR_BASE_DEC 10u
#define  DEF_MIN(a, b)                                  (((a) < (b)) ? (a) : (b))

/*
*********************************************************************************************************
*                                       CLK STR FORMAT DEFINES
*********************************************************************************************************
*/

#define  CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_UTC               1u   /* Fmt date/time as "YYYY-MM-DD HH:MM:SS UTC+TZ" :      */
                                                                /*           ...    "YYYY-MM-DD HH:MM:SS UTC+hh:mm"     */
                                                                /*           ... or "YYYY-MM-DD HH:MM:SS UTC-hh:mm".    */
#define  CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS                   2u   /* Fmt date/time as "YYYY-MM-DD HH:MM:SS".              */
#define  CLK_STR_FMT_MM_DD_YY_HH_MM_SS                     3u   /* Fmt date/time as "MM-DD-YY HH:MM:SS".                */
#define  CLK_STR_FMT_YYYY_MM_DD                            4u   /* Fmt date/time as "YYYY-MM-DD".                       */
#define  CLK_STR_FMT_MM_DD_YY                              5u   /* Fmt date/time as "MM-DD-YY".                         */
#define  CLK_STR_FMT_DAY_MONTH_DD_YYYY                     6u   /* Fmt date/time as "Day Month DD, YYYY".               */
#define  CLK_STR_FMT_DAY_MONTH_DD_HH_MM_SS_YYYY            7u   /* Fmt date/time as "Day Mon DD HH:MM:SS YYYY".         */
#define  CLK_STR_FMT_HH_MM_SS                              8u   /* Fmt date/time as "HH:MM:SS".                         */
#define  CLK_STR_FMT_HH_MM_SS_AM_PM                        9u   /* Fmt date/time as "HH:MM:SS AM|PM".                   */

                                                                                    /*           1         2         3  */
                                                                                    /* 0123456789012345678901234567890  */
#define  CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_UTC_LEN          30u   /*     Str len of fmt "YYYY-MM-DD HH:MM:SS UTC+TZ" :    */
                                                                /*             ...    "YYYY-MM-DD HH:MM:SS UTC+hh:mm"   */
                                                                /*             ... or "YYYY-MM-DD HH:MM:SS UTC-hh:mm".  */
#define  CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_LEN              20u   /*     Str len of fmt "YYYY-MM-DD HH:MM:SS".            */
#define  CLK_STR_FMT_MM_DD_YY_HH_MM_SS_LEN                18u   /*     Str len of fmt "MM-DD-YY HH:MM:SS".              */
#define  CLK_STR_FMT_YYYY_MM_DD_LEN                       11u   /*     Str len of fmt "YYYY-MM-DD".                     */
#define  CLK_STR_FMT_MM_DD_YY_LEN                          9u   /*     Str len of fmt "MM-DD-YY".                       */
#define  CLK_STR_FMT_DAY_MONTH_DD_YYYY_MAX_LEN            29u   /* Max str len of fmt "Day Month DD, YYYY".             */
#define  CLK_STR_FMT_DAY_MONTH_DD_HH_MM_SS_YYYY_LEN       25u   /*     Str len of fmt "Day Mon DD HH:MM:SS YYYY".       */
#define  CLK_STR_FMT_HH_MM_SS_LEN                          9u   /*     Str len of fmt "HH:MM:SS".                       */
#define  CLK_STR_FMT_HH_MM_SS_AM_PM_LEN                   15u   /*     Str len of fmt "HH:MM:SS AM|PM".                 */

#define  CLK_STR_FMT_MAX_LEN                              30u   /* Max str len of all clk str fmts.                     */


#define  CLK_STR_DIG_YR_LEN                                4u   /*     Str len of       yr dig.                         */
#define  CLK_STR_DIG_YR_TRUNC_LEN                          2u   /*     Str len of trunc yr dig.                         */
#define  CLK_STR_DIG_MONTH_LEN                             2u   /*     Str len of mon      dig.                         */
#define  CLK_STR_DIG_DAY_LEN                               2u   /*     Str len of day      dig.                         */
#define  CLK_STR_DIG_HR_LEN                                2u   /*     Str len of hr       dig.                         */
#define  CLK_STR_DIG_MIN_LEN                               2u   /*     Str len of min      dig.                         */
#define  CLK_STR_DIG_SEC_LEN                               2u   /*     Str len of sec      dig.                         */
#define  CLK_STR_DIG_TZ_HR_LEN                             2u   /*     Str len of tz hr    dig.                         */
#define  CLK_STR_DIG_TZ_MIN_LEN                            2u   /*     Str len of tz min   dig.                         */
#define  CLK_STR_DIG_TZ_MAX_LEN                            2u   /* Max str len of tz       digs.                        */
#define  CLK_STR_DAY_OF_WK_MAX_LEN                         9u   /* Max str len of day of wk       str (e.g. Wednesday). */
#define  CLK_STR_DAY_OF_WK_TRUNC_LEN                       3u   /*     Str len of day of wk trunc str.                  */
#define  CLK_STR_MONTH_MAX_LEN                             9u   /* Max str len of month           str (e.g. September). */
#define  CLK_STR_MONTH_TRUNC_LEN                           3u   /*     Str len of month     trunc str.                  */
#define  CLK_STR_AM_PM_LEN                                 2u   /*     Str len of am-pm           str.                  */


/*$PAGE*/
/*
*********************************************************************************************************
*                                            CLOCK DEFINES
*
* Note(s) : (1) Year 2038 problem (e.g. Unix Millennium bug, Y2K38 or Y2.038K) may cause some computer
*               software to fail before or in the year 2038. The problem affects all software and
*               systems that both store time as a signed 32-bit integer and interpret this number as
*               the number of seconds since 00:00:00 UTC on 1970/01/1.
*
*               There is no straightforward and general fix for this problem. Changing timestamp
*               datatype to an unsigned 32-bit integer have been chosen to avoid this problem. Thus
*               timestamp will be accurate until the year 2106, but dates before 1970 are not possible.
*********************************************************************************************************
*/

#define  CLK_FIRST_MONTH_OF_YR                             1u   /* First month of a yr    [1 to  12].                   */
#define  CLK_FIRST_DAY_OF_MONTH                            1u   /* First day   of a month [1 to  31].                   */
#define  CLK_FIRST_DAY_OF_YR                               1u   /* First day   of a yr    [1 to 366].                   */
#define  CLK_FIRST_DAY_OF_WK                               1u   /* First day   of a wk    [1 to   7].                   */


#define  CLK_MONTH_PER_YR                                 12
#define  CLK_HR_PER_HALF_DAY                              12

#define  CLK_YR_NONE                                       0u

#define  CLK_MONTH_NONE                                    0u
#define  CLK_MONTH_JAN                                     1u
#define  CLK_MONTH_FEB                                     2u
#define  CLK_MONTH_MAR                                     3u
#define  CLK_MONTH_APR                                     4u
#define  CLK_MONTH_MAY                                     5u
#define  CLK_MONTH_JUN                                     6u
#define  CLK_MONTH_JUL                                     7u
#define  CLK_MONTH_AUG                                     8u
#define  CLK_MONTH_SEP                                     9u
#define  CLK_MONTH_OCT                                    10u
#define  CLK_MONTH_NOV                                    11u
#define  CLK_MONTH_DEC                                    12u

#define  CLK_DAY_NONE                                      0u
#define  CLK_DAY_OF_WK_NONE                                0u
#define  CLK_DAY_OF_WK_SUN                                 1u
#define  CLK_DAY_OF_WK_MON                                 2u
#define  CLK_DAY_OF_WK_TUE                                 3u
#define  CLK_DAY_OF_WK_WED                                 4u
#define  CLK_DAY_OF_WK_THU                                 5u
#define  CLK_DAY_OF_WK_FRI                                 6u
#define  CLK_DAY_OF_WK_SAT                                 7u

#define DEF_INT_32U_MIN_VAL                                0u
#define DEF_INT_32U_MAX_VAL                                0xffffffff

                                                                /* ------------------ CLK TS DEFINES ------------------ */
#define  CLK_TS_SEC_MIN                 DEF_INT_32U_MIN_VAL
#define  CLK_TS_SEC_MAX                 DEF_INT_32U_MAX_VAL
#define  CLK_TS_SEC_NONE                CLK_TS_SEC_MIN

#define DEF_TIME_NBR_SEC_PER_MIN        60u
#define DEF_TIME_NBR_SEC_PER_HR         (60 * 60)
                                                                /* ------------------ CLK TZ DEFINES ------------------ */
#define  CLK_TZ_MIN_PRECISION                             15uL
#define  CLK_TZ_SEC_PRECISION          (CLK_TZ_MIN_PRECISION * DEF_TIME_NBR_SEC_PER_MIN)
#define  CLK_TZ_SEC_MIN              (-(CLK_HR_PER_HALF_DAY  * DEF_TIME_NBR_SEC_PER_HR))
#define  CLK_TZ_SEC_MAX                (CLK_HR_PER_HALF_DAY  * DEF_TIME_NBR_SEC_PER_HR)

#define DEF_TIME_NBR_SEC_PER_YR      (365 * 24 * 60 * 60)

#define DEF_TIME_NBR_SEC_PER_DAY     (24 * 60 * 60)
                                                                /* ----------------- CLK TICK DEFINES ----------------- */
#define  CLK_TICK_NONE                                     0u


/*$PAGE*/
                                                                /* ---------------- CLK EPOCH DEFINES ----------------- */
#define  CLK_EPOCH_YR_START                             2000u   /* Clk epoch starts = 2000-01-01 00:00:00 UTC.          */
#define  CLK_EPOCH_YR_END                               2136u   /*           ends   = 2135-12-31 23:59:59 UTC.          */
#define  CLK_EPOCH_DAY_OF_WK                               7u   /*                    2000-01-01 is Sat.                */


                                                                /* -------------- NTP EPOCH DATE DEFINES -------------- */
#define  CLK_NTP_EPOCH_YR_START                         1900u   /* NTP epoch starts = 1900-01-01 00:00:00 UTC.          */
#define  CLK_NTP_EPOCH_YR_END                           2036u   /*           ends   = 2035-12-31 23:59:59 UTC.          */
#define  CLK_NTP_EPOCH_DAY_OF_WK                           2u   /*                    1900-01-01 is Mon.                */
#define  CLK_NTP_EPOCH_OFFSET_YR_CNT           (CLK_EPOCH_YR_START - CLK_NTP_EPOCH_YR_START)

                                                                /* Only 24 leap yrs because 1900 is NOT a leap yr.      */
#define  CLK_NTP_EPOCH_OFFSET_LEAP_DAY_CNT    ((CLK_NTP_EPOCH_OFFSET_YR_CNT / 4u) - 1u)

                                                                /*     100 yrs * 365 * 24 * 60 * 60 = 3153600000        */
                                                                /*   +  24 leap days * 24 * 60 * 60 =    2073600        */
                                                                /* CLK_NTP_OFFSET_SEC               = 3155673600        */
#define  CLK_NTP_EPOCH_OFFSET_SEC             ((CLK_NTP_EPOCH_OFFSET_YR_CNT       * DEF_TIME_NBR_SEC_PER_YR ) +  \
                                               (CLK_NTP_EPOCH_OFFSET_LEAP_DAY_CNT * DEF_TIME_NBR_SEC_PER_DAY))


                                                                /* ------------- UNIX EPOCH DATE DEFINES -------------- */
                                                                /* See Note #1.                                         */
#define  CLK_UNIX_EPOCH_YR_START                        1970u   /* Unix epoch starts = 1970-01-01 00:00:00 UTC.         */
#define  CLK_UNIX_EPOCH_YR_END                          2106u   /*            ends   = 2105-12-31 23:59:59 UTC.         */
#define  CLK_UNIX_EPOCH_DAY_OF_WK                          5u   /*                     1970-01-01 is Thu.               */
#define  CLK_UNIX_EPOCH_OFFSET_YR_CNT          (CLK_EPOCH_YR_START - CLK_UNIX_EPOCH_YR_START)
#define  CLK_UNIX_EPOCH_OFFSET_LEAP_DAY_CNT    (CLK_UNIX_EPOCH_OFFSET_YR_CNT / 4u)

                                                                /*     30 yrs * 365 * 24 * 60 * 60 = 946080000          */
                                                                /*   +  7 leap days * 24 * 60 * 60 =    604800          */
                                                                /* CLK_UNIX_OFFSET_SEC             = 946684800          */
#define  CLK_UNIX_EPOCH_OFFSET_SEC            ((CLK_UNIX_EPOCH_OFFSET_YR_CNT       * DEF_TIME_NBR_SEC_PER_YR ) +  \
                                               (CLK_UNIX_EPOCH_OFFSET_LEAP_DAY_CNT * DEF_TIME_NBR_SEC_PER_DAY))


#define  DEF_TIME_NBR_DAY_PER_YR               365u


#define DEF_TIME_NBR_DAY_PER_YR_LEAP           366u

#define DEF_TIME_NBR_DAY_PER_WK                7u

#define DEF_NO                                 1u

#define DEF_YES                                0u

#define DEF_OK                                 0u

#define DEF_FAIL                               1u

#define DEF_TIME_NBR_HR_PER_DAY                24u

#define DEF_TIME_NBR_MIN_PER_HR                60u

/*$PAGE*/
/*
*********************************************************************************************************
*                                              DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                     CLOCK ERROR CODES DATA TYPE
*********************************************************************************************************
*/

typedef  RAW_U8  CLK_ERR;


/*
*********************************************************************************************************
*                                    CLOCK FORMAT STRING DATA TYPE
*********************************************************************************************************
*/

typedef  RAW_U8  CLK_STR_FMT;


/*
*********************************************************************************************************
*                                        CLOCK DATE DATA TYPES
*********************************************************************************************************
*/

typedef  RAW_U16  CLK_YR;
typedef  RAW_U8  CLK_MONTH;
typedef  RAW_U16  CLK_DAY;
typedef  RAW_U32  CLK_NBR_DAYS;


/*
*********************************************************************************************************
*                                        CLOCK TIME DATA TYPES
*********************************************************************************************************
*/

typedef  RAW_U8   CLK_HR;
typedef  RAW_U8   CLK_MIN;
typedef  RAW_U8   CLK_SEC;
typedef  RAW_U32  CPU_INT32U;         


/*
*********************************************************************************************************
*                                      CLOCK TIMESTAMP DATA TYPE
*********************************************************************************************************
*/

typedef  RAW_U32  CLK_TS_SEC;


/*
*********************************************************************************************************
*                                      CLOCK TIME ZONE DATA TYPE
*********************************************************************************************************
*/

typedef  RAW_S32  CLK_TZ_SEC;


/*
*********************************************************************************************************
*                                CLOCK PERIODIC TICK COUNTER DATA TYPE
*********************************************************************************************************
*/

typedef  RAW_U32  CLK_TICK_CTR;


typedef  RAW_S8                                    CPU_CHAR;
typedef  RAW_U8	                                   CPU_BOOLEAN;
typedef  RAW_U8                                    CPU_INT08U;
typedef  RAW_U32                                   CPU_SIZE_T;
typedef  RAW_S32                                   CPU_INT32S;
typedef  RAW_U32                                CPU_DATA;


/*$PAGE*/
/*
*********************************************************************************************************
*                                     CLOCK DATE/TIME DATA TYPE
*
* Note(s) : (1) Same date/time structure is used for all epoch. Thus Year value ('Yr') should be a value
*               between the epoch start and end years.
*
*           (2) Seconds value of 60 is valid to be compatible with leap second adjustment and the atomic
*               clock time stucture.
*
*           (3) Time zone is based on Coordinated Universal Time (UTC) & has valid values :
*
*               (a) Between  +|- 12 hours (+|- 43200 seconds)
*               (b) Multiples of 15 minutes
*********************************************************************************************************
*/

typedef  struct  clk_date_time {
    CLK_YR      Yr;                                             /* Yr        [epoch start to end yr), (see Note #1).    */
    CLK_MONTH   Month;                                          /* Month     [          1 to     12], (Jan to Dec).     */
    CLK_DAY     Day;                                            /* Day       [          1 to     31].                   */
    CLK_DAY     DayOfWk;                                        /* Day of wk [          1 to      7], (Sun to Sat).     */
    CLK_DAY     DayOfYr;                                        /* Day of yr [          1 to    366].                   */
    CLK_HR      Hr;                                             /* Hr        [          0 to     23].                   */
    CLK_MIN     Min;                                            /* Min       [          0 to     59].                   */
    CLK_SEC     Sec;                                            /* Sec       [          0 to     60], (see Note #2).    */
    CLK_TZ_SEC  TZ_sec;                                         /* TZ        [     -43200 to  43200], (see Note #3).    */
} CLK_DATE_TIME;





/*$PAGE*/
/*
*********************************************************************************************************
*                                              TRACING
*********************************************************************************************************
*/

                                                                /* Trace level, default to TRACE_LEVEL_OFF.             */
#ifndef  TRACE_LEVEL_OFF
#define  TRACE_LEVEL_OFF                                   0
#endif

#ifndef  TRACE_LEVEL_INFO
#define  TRACE_LEVEL_INFO                                  1
#endif

#ifndef  TRACE_LEVEL_DBG
#define  TRACE_LEVEL_DBG                                   2
#endif


#if ((defined(CLK_TRACE))       && \
     (defined(CLK_TRACE_LEVEL)) && \
     (CLK_TRACE_LEVEL >= TRACE_LEVEL_INFO))


    #if (CLK_TRACE_LEVEL >= TRACE_LEVEL_DBG)
        #define  CLK_TRACE_DBG(msg)     CLK_TRACE  msg
    #else
        #define  CLK_TRACE_DBG(msg)
    #endif

    #define  CLK_TRACE_INFO(msg)        CLK_TRACE  msg

#else
    #define  CLK_TRACE_LOG(msg)
    #define  CLK_TRACE_DBG(msg)
    #define  CLK_TRACE_INFO(msg)
#endif


CLK_TZ_SEC   Clk_GetTZ              (void);                         /* Get clk TZ offset.                               */

CPU_BOOLEAN  Clk_SetTZ              (CLK_TZ_SEC      tz_sec);       /* Set clk TZ offset.                               */



                                                                    /* ----------- CLK TS & DATE/TIME UTIL ------------ */
CPU_BOOLEAN  Clk_GetDateTime        (CLK_DATE_TIME  *date_time);    /* Get clk TS using a CLK_DATE_TIME struct.         */

CPU_BOOLEAN  Clk_SetDateTime        (CLK_DATE_TIME  *date_time);    /* Set clk TS using a CLK_DATE_TIME struct.         */

CPU_BOOLEAN  Clk_TS_ToDateTime      (CLK_TS_SEC      ts_sec,
                                     CLK_TZ_SEC      tz_sec,
                                     CLK_DATE_TIME  *p_date_time);

CPU_BOOLEAN  Clk_DateTimeToTS       (CLK_TS_SEC     *p_ts_sec,
                                     CLK_DATE_TIME  *date_time);

CPU_BOOLEAN  Clk_DateTimeMake       (CLK_DATE_TIME  *date_time,     /* Make a date/time struct.                         */
                                     CLK_YR          yr,
                                     CLK_MONTH       month,
                                     CLK_DAY         day,
                                     CLK_HR          hr,
                                     CLK_MIN         min,
                                     CLK_SEC         sec,
                                     CLK_TZ_SEC      tz_sec);

CPU_BOOLEAN  Clk_IsDateTimeValid    (CLK_DATE_TIME  *p_date_time);


CPU_BOOLEAN  Clk_DateTimeToStr (CLK_DATE_TIME  *p_date_time,
                                CLK_STR_FMT     fmt,
                                CPU_CHAR       *p_str,
                                CPU_SIZE_T      str_len);


#if (CLK_CFG_NTP_EN == DEF_ENABLED)
                                                                    /* --------------- NTP TS GET & SET --------------- */
CPU_BOOLEAN  Clk_GetTS_NTP          (CLK_TS_SEC     *ts_sec_ntp);   /* Get clk TS using NTP TS.                         */

CPU_BOOLEAN  Clk_SetTS_NTP          (CLK_TS_SEC      ts_sec_ntp);   /* Set clk TS using NTP TS.                         */


                                                                    /* ----------- NTP TS & DATE/TIME UTIL ------------ */
CPU_BOOLEAN  Clk_TS_ToTS_NTP        (CLK_TS_SEC      ts_sec,
                                     CLK_TS_SEC     *p_ts_sec_ntp);

CPU_BOOLEAN  Clk_TS_NTP_ToTS        (CLK_TS_SEC     *p_ts_sec,
                                     CLK_TS_SEC      ts_sec_ntp);

CPU_BOOLEAN  Clk_TS_NTP_ToDateTime  (CLK_TS_SEC      ts_ntp_sec,
                                     CLK_TZ_SEC      tz_sec,
                                     CLK_DATE_TIME  *p_date_time);

CPU_BOOLEAN  Clk_DateTimeToTS_NTP   (CLK_TS_SEC     *p_ts_ntp_sec,
                                     CLK_DATE_TIME  *p_date_time);

CPU_BOOLEAN  Clk_NTP_DateTimeMake   (CLK_DATE_TIME  *p_date_time,
                                     CLK_YR          yr,
                                     CLK_MONTH       month,
                                     CLK_DAY         day,
                                     CLK_HR          hr,
                                     CLK_MIN         min,
                                     CLK_SEC         sec,
                                     CLK_TZ_SEC      tz_sec);

CPU_BOOLEAN  Clk_IsNTP_DateTimeValid(CLK_DATE_TIME  *p_date_time);

#endif

#if (CLK_CFG_UNIX_EN == DEF_ENABLED)
                                                                    /* -------------- UNIX TS GET & SET --------------- */
CPU_BOOLEAN  Clk_GetTS_Unix         (CLK_TS_SEC     *ts_unix_sec);  /* Get clk TS using Unix TS.                        */

CPU_BOOLEAN  Clk_SetTS_Unix         (CLK_TS_SEC      ts_unix_sec);  /* Set clk TS using Unix TS.                        */


                                                                    /* ----------- UNIX TS & DATE/TIME UTIL ----------- */
CPU_BOOLEAN  Clk_TS_ToTS_Unix       (CLK_TS_SEC      ts_sec,
                                     CLK_TS_SEC     *p_ts_unix_sec);

CPU_BOOLEAN  Clk_TS_UnixToTS        (CLK_TS_SEC     *p_ts_sec,
                                     CLK_TS_SEC      ts_unix_sec);

CPU_BOOLEAN  Clk_TS_UnixToDateTime  (CLK_TS_SEC      ts_unix_sec,
                                     CLK_TZ_SEC      tz_sec,
                                     CLK_DATE_TIME  *p_date_time);

CPU_BOOLEAN  Clk_DateTimeToTS_Unix  (CLK_TS_SEC     *p_ts_unix_sec,
                                     CLK_DATE_TIME  *p_date_time);

CPU_BOOLEAN  Clk_UnixDateTimeMake   (CLK_DATE_TIME  *p_date_time,
                                     CLK_YR          yr,
                                     CLK_MONTH       month,
                                     CLK_DAY         day,
                                     CLK_HR          hr,
                                     CLK_MIN         min,
                                     CLK_SEC         sec,
                                     CLK_TZ_SEC      tz_sec);

CPU_BOOLEAN  Clk_IsUnixDateTimeValid(CLK_DATE_TIME  *p_date_time);

#endif





void  clk_task_handler(void);
RAW_U16 clk_os_wait(void);
RAW_U16	rtc_init(void);
RAW_U16 clk_os_signal(void);
RAW_U16  Clk_SignalClk(void);
RAW_U16 Clk_Init(void);
RAW_U16  Clk_SignalClk (void);
CLK_TS_SEC  Clk_GetTS (void);



#endif


