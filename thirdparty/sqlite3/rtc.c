#include <raw_api.h>
#include <rtc_config.h>
#include <rtc.h>


static  const  CLK_DAY  Clk_DaysInYr[2u] = {
    DEF_TIME_NBR_DAY_PER_YR, DEF_TIME_NBR_DAY_PER_YR_LEAP
};

static  const  CLK_DAY  Clk_DaysInMonth[2u][CLK_MONTH_PER_YR] = {
  /* Jan  Feb  Mar  Apr  May  Jun  Jul  Aug  Sep  Oct  Nov  Dec */
   { 31u, 28u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u },
   { 31u, 29u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u }
};

#if (CLK_CFG_STR_CONV_EN == DEF_ENABLED)
static  const  CPU_CHAR *  const  Clk_StrMonth[CLK_MONTH_PER_YR] = {
                    /*           1 */
                    /* 01234567890 */
   (const  CPU_CHAR *)"January",
   (const  CPU_CHAR *)"February",
   (const  CPU_CHAR *)"March",
   (const  CPU_CHAR *)"April",
   (const  CPU_CHAR *)"May",
   (const  CPU_CHAR *)"June",
   (const  CPU_CHAR *)"July",
   (const  CPU_CHAR *)"August",
   (const  CPU_CHAR *)"September",
   (const  CPU_CHAR *)"October",
   (const  CPU_CHAR *)"November",
   (const  CPU_CHAR *)"December"
};

static  const  CPU_CHAR *  const  Clk_StrDayOfWk[DEF_TIME_NBR_DAY_PER_WK] = {
                    /*           1 */
                    /* 01234567890 */
   (const  CPU_CHAR *)"Sunday",
   (const  CPU_CHAR *)"Monday",
   (const  CPU_CHAR *)"Tuesday",
   (const  CPU_CHAR *)"Wednesday",
   (const  CPU_CHAR *)"Thursday",
   (const  CPU_CHAR *)"Friday",
   (const  CPU_CHAR *)"Saturday"
};
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if    (CLK_CFG_EXT_EN    != DEF_ENABLED)
static  CLK_TS_SEC    Clk_TS_UTC_sec;
#if    (CLK_CFG_SIGNAL_EN == DEF_ENABLED)
static  CLK_TICK_CTR  Clk_TickCtr;
#endif
#endif
static  CLK_TZ_SEC    Clk_TZ_sec;

static  CLK_YR        Clk_CacheYr;
static  CLK_MONTH     Clk_CacheMonth;
static  CLK_NBR_DAYS  Clk_CacheYrDays;
static  CLK_DAY       Clk_CacheMonthDays;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTIONS PROTOTYPES
*********************************************************************************************************
*/

static  CPU_BOOLEAN  Clk_IsLeapYr              (CLK_YR          yr);


static  CPU_BOOLEAN  Clk_IsDateValid           (CLK_YR          yr,
                                                CLK_MONTH       month,
                                                CLK_DAY         day,
                                                CLK_YR          yr_start,
                                                CLK_YR          yr_end);


static  CPU_BOOLEAN  Clk_IsDayOfYrValid        (CLK_YR          yr,
                                                CLK_DAY         day_of_yr);

static  CPU_BOOLEAN  Clk_IsDayOfWkValid        (CLK_DAY         day_of_wk);


static  CPU_BOOLEAN  Clk_IsTimeValid           (CLK_YR          hr,
                                                CLK_MONTH       min,
                                                CLK_DAY         sec);

static  CPU_BOOLEAN  Clk_IsTZValid             (CLK_TZ_SEC      tz_sec);


static  CPU_BOOLEAN  Clk_IsDateTimeValidHandler(CLK_DATE_TIME  *p_date_time,
                                                CLK_YR          yr_start,
                                                CLK_YR          yr_end);



static  CLK_DAY      Clk_GetDayOfYrHandler     (CLK_YR          yr,
                                                CLK_MONTH       month,
                                                CLK_DAY         day);

static  CLK_DAY      Clk_GetDayOfWkHandler     (CLK_YR          yr,
                                                CLK_MONTH       month,
                                                CLK_DAY         day);


static  void         Clk_SetTZ_Handler         (CLK_TZ_SEC      tz_sec);


static  CPU_BOOLEAN  Clk_TS_ToDateTimeHandler  (CLK_TS_SEC      ts_sec,
                                                CLK_TZ_SEC      tz_sec,
                                                CLK_DATE_TIME  *p_date_time,
                                                CLK_YR          yr_start,
                                                CLK_YR          yr_end);

static  CPU_BOOLEAN  Clk_DateTimeToTS_Handler  (CLK_TS_SEC     *p_ts_sec,
                                                CLK_DATE_TIME  *p_date_time,
                                                CLK_YR          yr_start);

static  CPU_BOOLEAN  Clk_DateTimeMakeHandler   (CLK_DATE_TIME  *p_date_time,
                                                CLK_YR          yr,
                                                CLK_MONTH       month,
                                                CLK_DAY         day,
                                                CLK_HR          hr,
                                                CLK_MIN         min,
                                                CLK_SEC         sec,
                                                CLK_TZ_SEC      tz_sec,
                                                CLK_YR          yr_start,
                                                CLK_YR          yr_end);





#define  ASCII_CHAR_SPACE                               0x20    /* ' '                                                  */
#define  ASCII_CHAR_TILDE                               0x7E    /* '~'                                                  */

#define  ASCII_IS_PRINT(c)             ((((c) >= ASCII_CHAR_SPACE) && ((c) <= ASCII_CHAR_TILDE)) ? (DEF_YES) : (DEF_NO))


CPU_BOOLEAN  ASCII_IsPrint (CPU_CHAR  c)
{
    CPU_BOOLEAN  print;


    print = ASCII_IS_PRINT(c);

    return (print);
}


static  CPU_CHAR  *Str_FmtNbr_Int32 (CPU_INT32U    nbr,
                                     CPU_INT08U    nbr_dig,
                                     CPU_INT08U    nbr_base,
                                     CPU_BOOLEAN   nbr_neg,
                                     CPU_CHAR      lead_char,
                                     CPU_BOOLEAN   lower_case,
                                     CPU_BOOLEAN   nul,
                                     CPU_CHAR     *pstr)
{
    CPU_CHAR     *pstr_fmt;
    CPU_DATA      i;
    CPU_INT32U    nbr_fmt;
    CPU_INT32U    nbr_log;
    CPU_INT08U    nbr_dig_max;
    CPU_INT08U    nbr_dig_min;
    CPU_INT08U    nbr_dig_fmtd;
    CPU_INT08U    nbr_neg_sign;
    CPU_INT08U    nbr_lead_char;
    CPU_INT08U    dig_val;
    CPU_INT08U    lead_char_delta_0;
    CPU_INT08U    lead_char_delta_a;
    CPU_BOOLEAN   lead_char_dig;
    CPU_BOOLEAN   lead_char_0;
    CPU_BOOLEAN   fmt_invalid;
    CPU_BOOLEAN   print_char;
    CPU_BOOLEAN   nbr_neg_fmtd;


/*$PAGE*/
                                                                /* ---------------- VALIDATE FMT ARGS ----------------- */
    if (pstr == (CPU_CHAR *)0) {                                /* Rtn NULL if str ptr NULL (see Note #6a).             */
        return ((CPU_CHAR *)0);
    }

    fmt_invalid = DEF_NO;

    if (nbr_dig < 1) {                                          /* If nbr digs = 0, ...                                 */
        fmt_invalid = DEF_YES;                                  /* ... fmt invalid str (see Note #6b).                  */
    }
                                                                /* If invalid base, ...                                 */
    if ((nbr_base <  2u) ||
        (nbr_base > 36u)) {
        fmt_invalid = DEF_YES;                                  /* ... fmt invalid str (see Note #6d).                  */
    }

    if (lead_char != (CPU_CHAR)'\0') {
        print_char =  ASCII_IsPrint(lead_char);
        if (print_char != DEF_YES) {                            /* If lead char non-printable (see Note #3a1), ...      */
            fmt_invalid = DEF_YES;                              /* ... fmt invalid str        (see Note #6e).           */

        } else if (lead_char != '0') {                          /* Chk lead char for non-0 nbr base dig.                */
            lead_char_delta_0 = (CPU_INT08U)(lead_char - '0');
            if (lower_case != DEF_YES) {
                lead_char_delta_a = (CPU_INT08U)(lead_char - 'A');
            } else {
                lead_char_delta_a = (CPU_INT08U)(lead_char - 'a');
            }

            lead_char_dig = (((nbr_base <= 10u) &&  (lead_char_delta_0 <  nbr_base))      ||
                             ((nbr_base >  10u) && ((lead_char_delta_0 <             10u) ||
                                                    (lead_char_delta_a < (nbr_base - 10u))))) ? DEF_YES : DEF_NO;

            if (lead_char_dig == DEF_YES) {                     /* If lead char non-0 nbr base dig (see Note #3a2A), ...*/
                fmt_invalid = DEF_YES;                          /* ... fmt invalid str             (see Note #6e).      */
            }
        }
    }


                                                                /* ----------------- PREPARE NBR FMT ------------------ */
    pstr_fmt = pstr;

    if (fmt_invalid == DEF_NO) {
        nbr_fmt     = nbr;
        nbr_log     = nbr;
        nbr_dig_max = 1u;
        while (nbr_log >= nbr_base) {                           /* While nbr base digs avail, ...                       */
            nbr_dig_max++;                                      /* ... calc max nbr digs.                               */
            nbr_log /= nbr_base;
        }

        nbr_neg_sign = (nbr_neg == DEF_YES) ? 1u : 0u;
        if (nbr_dig >= (nbr_dig_max + nbr_neg_sign)) {          /* If req'd nbr digs >= (max nbr digs + neg sign), ...  */
            nbr_neg_fmtd = DEF_NO;
            nbr_dig_min  = DEF_MIN(nbr_dig_max, nbr_dig);
                                                                /* ... calc nbr digs to fmt & nbr lead chars.           */
            if (lead_char != (CPU_CHAR)'\0') {
                nbr_dig_fmtd  = nbr_dig;
                nbr_lead_char = nbr_dig     -
                                nbr_dig_min - nbr_neg_sign;
            } else {
                nbr_dig_fmtd  = nbr_dig_min + nbr_neg_sign;
                nbr_lead_char = 0u;
            }

            if (nbr_lead_char > 0) {                            /* If lead chars to fmt, ...                            */
                lead_char_0 = (lead_char == '0')                /* ... chk if lead char a '0' dig (see Note #3a2B).     */
                            ?  DEF_YES : DEF_NO;
            } else {
                lead_char_0 =  DEF_NO;
            }

        } else {                                                /* Else if nbr trunc'd, ...                             */
            fmt_invalid = DEF_YES;                              /* ... fmt invalid str (see Note #6c).                  */
        }
    }

    if (fmt_invalid != DEF_NO) {
        nbr_dig_fmtd = nbr_dig;
    }


/*$PAGE*/
                                                                /* ------------------- FMT NBR STR -------------------- */
    pstr_fmt += nbr_dig_fmtd;                                   /* Start fmt @ least-sig dig.                           */

    if (nul != DEF_NO) {                                        /* If NOT DISABLED, append NULL char (see Note #4).     */
       *pstr_fmt = (CPU_CHAR)'\0';
    }
    pstr_fmt--;


    for (i = 0u; i < nbr_dig_fmtd; i++) {                       /* Fmt str for desired nbr digs :                       */
        if (fmt_invalid == DEF_NO) {
            if ((nbr_fmt > 0) ||                                /* If fmt nbr > 0                               ...     */
                (i == 0u)) {                                    /* ... OR on one's  dig to fmt (see Note #3c1), ...     */
                                                                /* ... calc & fmt dig val;                      ...     */
                dig_val = (CPU_INT08U)(nbr_fmt % nbr_base);
                if (dig_val < 10u) {
                   *pstr_fmt-- = (CPU_CHAR)(dig_val + '0');
                } else {
                    if (lower_case !=  DEF_YES) {
                       *pstr_fmt--  = (CPU_CHAR)((dig_val - 10u) + 'A');
                    } else {
                       *pstr_fmt--  = (CPU_CHAR)((dig_val - 10u) + 'a');
                    }
                }

                nbr_fmt /= nbr_base;                            /* Shift to next more-sig dig.                          */

            } else if ((nbr_neg      == DEF_YES)  &&            /* ... else if nbr neg             AND          ...     */
                     (((lead_char_0  == DEF_NO )  &&            /* ... lead char NOT a '0' dig                  ...     */
                       (nbr_neg_fmtd == DEF_NO )) ||            /* ... but neg sign NOT yet fmt'd  OR           ...     */
                      ((lead_char_0  != DEF_NO )  &&            /* ... lead char is  a '0' dig                  ...     */
                       (i == (nbr_dig_fmtd - 1u))))) {          /* ... & on most-sig dig to fmt,                ...     */

               *pstr_fmt--   = '-';                             /* ... prepend neg sign (see Note #3b);         ...     */
                nbr_neg_fmtd = DEF_YES;

            } else if (lead_char != (CPU_CHAR)'\0') {           /* ... else if avail,                           ...     */
               *pstr_fmt-- = lead_char;                         /* ... fmt lead char.                                   */
            }

        } else {                                                /* Else fmt '?' for invalid str (see Note #7).          */
           *pstr_fmt-- = '?';
        }
    }


    if (fmt_invalid != DEF_NO) {                                /* Rtn NULL for invalid str fmt (see Notes #6a - #6e).  */
        return ((CPU_CHAR *)0);
    }


    return (pstr);                                              /* Rtn ptr to fmt'd str (see Note #6f).                 */
}



CPU_CHAR  *Str_FmtNbr_Int32S (CPU_INT32S    nbr,
                              CPU_INT08U    nbr_dig,
                              CPU_INT08U    nbr_base,
                              CPU_CHAR      lead_char,
                              CPU_BOOLEAN   lower_case,
                              CPU_BOOLEAN   nul,
                              CPU_CHAR     *pstr)
{
    CPU_CHAR     *pstr_fmt;
    CPU_INT32S    nbr_fmt;
    CPU_BOOLEAN   nbr_neg;


    if (nbr < 0) {                                              /* If nbr neg, ...                                      */
        nbr_fmt = -nbr;                                         /* ... negate nbr.                                      */
        nbr_neg =  DEF_YES;
    } else {
        nbr_fmt =  nbr;
        nbr_neg =  DEF_NO;
    }

    pstr_fmt = Str_FmtNbr_Int32((CPU_INT32U)nbr_fmt,            /* Fmt signed int into str.                             */
                                            nbr_dig,
                                            nbr_base,
                                            nbr_neg,
                                            lead_char,
                                            lower_case,
                                            nul,
                                            pstr);

    return (pstr_fmt);
}


CPU_CHAR  *Str_FmtNbr_Int32U (CPU_INT32U    nbr,
                              CPU_INT08U    nbr_dig,
                              CPU_INT08U    nbr_base,
                              CPU_CHAR      lead_char,
                              CPU_BOOLEAN   lower_case,
                              CPU_BOOLEAN   nul,
                              CPU_CHAR     *pstr)
{
    CPU_CHAR  *pstr_fmt;


    pstr_fmt = Str_FmtNbr_Int32(nbr,                            /* Fmt unsigned int into str.                           */
                                nbr_dig,
                                nbr_base,
                                DEF_NO,
                                lead_char,
                                lower_case,
                                nul,
                                pstr);

    return (pstr_fmt);
}



CPU_CHAR  *Str_Copy_N (CPU_CHAR    *pdest,
                       CPU_CHAR    *psrc,
                       CPU_SIZE_T   len_max)
{
    CPU_CHAR    *pstr;
    CPU_CHAR    *pstr_next;
    CPU_SIZE_T   len_copy;

                                                                /* Rtn NULL if str ptr(s) NULL      (see Note #2a).     */
    if (pdest == (CPU_CHAR *)0) {
        return  ((CPU_CHAR *)0);
    }
    if (psrc  == (CPU_CHAR *)0) {
        return  ((CPU_CHAR *)0);
    }

    if (len_max == (CPU_SIZE_T)0) {                             /* Rtn NULL if copy len equals zero (see Note #2d).     */
        return  ((CPU_CHAR *)0);
    }


    pstr      = pdest;
    pstr_next = pstr;
    pstr_next++;
    len_copy  = 0;

    while (( pstr_next != (CPU_CHAR *)0) &&                     /* Copy str until NULL ptr(s)  (see Note #2b)  ...      */
           ( psrc      != (CPU_CHAR *)0) &&
           (*psrc      != (CPU_CHAR  )0) &&                     /* ... or NULL char found      (see Note #2c); ...      */
           ( len_copy  <  (CPU_SIZE_T)len_max)) {               /* ... or max nbr chars copied (see Note #2d).          */
       *pstr = *psrc;
        pstr++;
        pstr_next++;
        psrc++;
        len_copy++;
    }

   *pstr = (CPU_CHAR)0;                                         /* Append NULL char (see Note #2b2).                    */


    return (pdest);
}

CPU_CHAR  *Str_Cat_N (CPU_CHAR    *pdest,
                      CPU_CHAR    *pstr_cat,
                      CPU_SIZE_T   len_max)
{
    CPU_CHAR    *pstr;
    CPU_CHAR    *pstr_next;
    CPU_SIZE_T   len_cat;

                                                                /* Rtn NULL if str ptr(s) NULL     (see Note #2a).      */
    if (pdest == (CPU_CHAR *)0) {
        return  ((CPU_CHAR *)0);
    }
    if (pstr_cat == (CPU_CHAR *)0) {
        return  ((CPU_CHAR *)0);
    }

    if (len_max == (CPU_SIZE_T)0) {                             /* Rtn NULL if cat len equals zero (see Note #2e).      */
        return  ((CPU_CHAR *)0);
    }


    pstr = pdest;
    while (( pstr != (CPU_CHAR *)0) &&                          /* Adv to end of cur dest str until NULL ptr ...        */
           (*pstr != (CPU_CHAR  )0)) {                          /* ... or NULL char found..                             */
        pstr++;
    }
    if (pstr == (CPU_CHAR *)0) {                                /* If NULL str overrun, rtn NULL (see Note #2b).        */
        return ((CPU_CHAR *)0);
    }

    pstr_next = pstr;
    pstr_next++;
    len_cat   = 0;

    while (( pstr_next != (CPU_CHAR *)0) &&                     /* Cat str until NULL ptr(s)  (see Note #2c)  ...       */
           ( pstr_cat  != (CPU_CHAR *)0) &&
           (*pstr_cat  != (CPU_CHAR  )0) &&                     /* ... or NULL char found     (see Note #2d); ...       */
           ( len_cat   <  (CPU_SIZE_T)len_max)) {               /* ... or max nbr chars cat'd (see Note #2d).           */
       *pstr = *pstr_cat;
        pstr++;
        pstr_next++;
        pstr_cat++;
        len_cat++;
    }

   *pstr = (CPU_CHAR)0;                                         /* Append NULL char (see Note #2c2).                    */


    return (pdest);
}



void  clk_task_handler()
{
  
   RAW_SR_ALLOC();


    while (1) {
                                                              
    
		clk_os_wait();
                                            
		RAW_CPU_DISABLE();
		
		if (Clk_TS_UTC_sec < CLK_TS_SEC_MAX) {
			Clk_TS_UTC_sec++;
		}
		
		RAW_CPU_ENABLE();
		}

	
}


RAW_U16 Clk_Init()
{
	RAW_U16 ret;

	#if (CLK_CFG_EXT_EN == DEF_ENABLED)                             /* ------------------- INIT EXT TS -------------------- */
	Clk_ExtTS_Init();

	#else                                                           /* ------------------- CLK/OS INIT -------------------- */
	ret = rtc_init();

	if (ret != RAW_SUCCESS) {

		return ret;
	}

	                                                            /* ---------------- INIT CLK VARIABLES ---------------- */
	Clk_TS_UTC_sec = CLK_TS_SEC_NONE;                           /* Clk epoch = 2000-01-01 00:00:00 UTC                  */
	#if (CLK_CFG_SIGNAL_EN == DEF_ENABLED)
	Clk_TickCtr         = CLK_TICK_NONE;
	#endif
	#endif
	Clk_TZ_sec          = CLK_CFG_TZ_DFLT_SEC;                  /* Clk TZ = UTC offset                                  */
	Clk_CacheMonth      = CLK_MONTH_NONE;
	Clk_CacheMonthDays  = CLK_DAY_NONE;
	Clk_CacheYr         = CLK_YR_NONE;
	Clk_CacheYrDays     = CLK_DAY_NONE;

	return RAW_SUCCESS;

}


#if ((CLK_CFG_EXT_EN    != DEF_ENABLED) &&  \
     (CLK_CFG_SIGNAL_EN == DEF_ENABLED))
RAW_U16  Clk_SignalClk()
{
	CPU_BOOLEAN  signal;
	RAW_U16 ret;

	RAW_SR_ALLOC();


	signal = DEF_NO;
	RAW_CPU_DISABLE(); 
	
	Clk_TickCtr++;
	
	if (Clk_TickCtr >= CLK_CFG_SIGNAL_FREQ_HZ) {
		Clk_TickCtr -= CLK_CFG_SIGNAL_FREQ_HZ;                  /* See Note #1.                                         */
		signal       = DEF_YES;
	}
	
	RAW_CPU_ENABLE();

	if (signal == DEF_YES) {
		ret = clk_os_signal();
		if (ret != RAW_SUCCESS) {
			return ret;
		}
	}

	return RAW_SUCCESS;
   
}
#endif


CLK_TS_SEC  Clk_GetTS (void)
{
	CLK_TS_SEC  ts_sec;
	
	#if (CLK_CFG_EXT_EN != DEF_ENABLED)
	RAW_SR_ALLOC();

														  /* ---------------------- GET TS ---------------------- */
	RAW_CPU_DISABLE(); 
	ts_sec = Clk_TS_UTC_sec;
	RAW_CPU_ENABLE();
	
	#else
	ts_sec = Clk_ExtTS_Get();
	
	#endif

	return (ts_sec);
}


CPU_BOOLEAN  Clk_SetTS (CLK_TS_SEC  ts_sec)
{
     CPU_BOOLEAN  valid;
#if (CLK_CFG_EXT_EN != DEF_ENABLED)
    RAW_SR_ALLOC();

                                                                /* ---------------------- SET TS ---------------------- */
    RAW_CPU_DISABLE();
    Clk_TS_UTC_sec = ts_sec;
    RAW_CPU_ENABLE();

    valid          = DEF_OK;
#else
    valid          = Clk_ExtTS_Set(ts_sec);
#endif

    return (valid);
}


CLK_TZ_SEC  Clk_GetTZ (void)
{
	CLK_TZ_SEC  tz_sec;
	RAW_SR_ALLOC();

	                                                            /* ---------------------- GET TZ ---------------------- */
	RAW_CPU_DISABLE();
	tz_sec = Clk_TZ_sec;
	RAW_CPU_ENABLE();

	return (tz_sec);
}


CPU_BOOLEAN  Clk_SetTZ (CLK_TZ_SEC  tz_sec)
{
    CPU_BOOLEAN  valid;

                                                                /* ------------------- VALIDATE TZ -------------------- */
    valid = Clk_IsTZValid(tz_sec);
    if (valid != DEF_YES) {
        return  (DEF_FAIL);
    }

                                                                /* ---------------------- SET TZ ---------------------- */
    Clk_SetTZ_Handler(tz_sec);

    return (DEF_OK);
}


static  void  Clk_SetTZ_Handler (CLK_TZ_SEC  tz_sec)
{
    RAW_SR_ALLOC();

                                                                /* ---------------------- SET TZ ---------------------- */
    RAW_CPU_DISABLE();
    Clk_TZ_sec = tz_sec;
    RAW_CPU_ENABLE();
	
}


static  CPU_BOOLEAN  Clk_IsTZValid (CLK_TZ_SEC  tz_sec)
{
    CLK_TS_SEC  tz_sec_abs;                                     /* See Note #2.                                         */

                                                                /* -------------- VALIDATE TZ PRECISION --------------- */
    tz_sec_abs = DEF_ABS(tz_sec);;
    if ((tz_sec_abs % CLK_TZ_SEC_PRECISION) != 0u) {            /* See Note #1b.                                        */
        CLK_TRACE_DBG(("Invalid time zone, must be multiple of %d seconds\n\r",
                       CLK_TZ_SEC_PRECISION));
        return (DEF_NO);
    }

                                                                /* --------------- VALIDATE TZ MIN-MAX ---------------- */
    if (tz_sec_abs > CLK_TZ_SEC_MAX) {                          /* See Note #1a.                                        */
        CLK_TRACE_DBG(("Invalid time zone, must be > %d & < %u\n\r",
                       CLK_TZ_SEC_MIN, CLK_TZ_SEC_MAX));
        return (DEF_NO);
    }

    return (DEF_YES);
}


CPU_BOOLEAN  Clk_GetDateTime (CLK_DATE_TIME  *p_date_time)
{
    CLK_TS_SEC   ts_sec;
    CLK_TZ_SEC   tz_sec;
    CPU_BOOLEAN  valid;


#if 0                                                           /* Validated in Clk_TS_ToDateTime().                    */
    if (p_date_time == (CLK_DATE_TIME *)0) {                    /* -------------- VALIDATE DATE/TIME PTR -------------- */
        return (DEF_FAIL);
    }
#endif


    ts_sec = Clk_GetTS();                                       /* ---------------------- GET TS ---------------------- */
    tz_sec = Clk_GetTZ();                                       /* ---------------------- GET TZ ---------------------- */

                                                                /* ------------- CONV CLK TS TO DATE/TIME ------------- */
    valid  = Clk_TS_ToDateTime(ts_sec,
                               tz_sec,
                               p_date_time);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}


CPU_BOOLEAN  Clk_SetDateTime (CLK_DATE_TIME  *p_date_time)
{
    CLK_TS_SEC   ts_sec;
    CPU_BOOLEAN  valid;


#if 0                                                           /* Validated in Clk_DateTimeToTS().                     */
    if (p_date_time == (CLK_DATE_TIME *)0) {                    /* -------------- VALIDATE DATE/TIME PTR -------------- */
        return (DEF_FAIL);
    }
#endif

                                                                /* ------------- CONV DATE/TIME TO CLK TS ------------- */
    valid = Clk_DateTimeToTS(&ts_sec, p_date_time);             /* Validates date/time & TZ (see Note #1c).             */
    if (valid != DEF_OK) {
        CLK_TRACE_DBG(("Date/time is not valid"));
        return  (DEF_FAIL);
    }

                                                                /* ---------------------- SET TS ---------------------- */
    valid = Clk_SetTS(ts_sec);                                  /* See Note #1a.                                        */
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

                                                                /* ---------------------- SET TZ ---------------------- */
    Clk_SetTZ_Handler(p_date_time->TZ_sec);                     /* See Note #1b.                                        */

    return (DEF_OK);
}



CPU_BOOLEAN  Clk_TS_ToDateTime (CLK_TS_SEC      ts_sec,
                                CLK_TZ_SEC      tz_sec,
                                CLK_DATE_TIME  *p_date_time)
{
    CPU_BOOLEAN  valid;

                                                                /* ------------- CONV CLK TS TO DATE/TIME ------------- */
    CLK_TRACE_DBG(("\n\rConvert TS to Date/time:\n\r"
                   "    TS to convert= %u\n\n\r",
                   (unsigned int)ts_sec));

    valid = Clk_TS_ToDateTimeHandler(ts_sec,
                                     tz_sec,
                                     p_date_time,
                                     CLK_EPOCH_YR_START,
                                     CLK_EPOCH_YR_END);
    if (valid != DEF_OK) {
        CLK_TRACE_DBG(("    Date/time conversion has failed\n\r"));
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}


static  CPU_BOOLEAN  Clk_TS_ToDateTimeHandler (CLK_TS_SEC      ts_sec,
                                               CLK_TZ_SEC      tz_sec,
                                               CLK_DATE_TIME  *p_date_time,
                                               CLK_YR          yr_start,
                                               CLK_YR          yr_end)
{
    CLK_TS_SEC    ts_sec_rem;
    CLK_TS_SEC    tz_sec_abs;                                   /* See Note #2.                                         */
    CLK_TS_SEC    sec_to_remove;
    CLK_NBR_DAYS  days_in_yr;
    CLK_DAY       days_in_month;
    CPU_INT08U    leap_yr_ix;
    CPU_BOOLEAN   leap_yr;
    CPU_BOOLEAN   valid;


#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* -------------- VALIDATE DATE/TIME PTR -------------- */
    if (p_date_time == (CLK_DATE_TIME *)0) {
        return (DEF_FAIL);
    }
#endif


                                                                /* ------------ ADJ INIT TS FOR TZ OFFSET ------------- */
    ts_sec_rem = ts_sec;
    tz_sec_abs = DEF_ABS(tz_sec);
    if (tz_sec < 0) {
        if (ts_sec_rem < tz_sec_abs) {                          /* Chk for ovf when tz is neg.                          */
            CLK_TRACE_DBG(("    Timestamp is too small to substract time zone offset\n\r"));
            return (DEF_FAIL);
        }
        ts_sec_rem -= tz_sec_abs;                               /* See Note #1c1.                                       */

    } else {
        ts_sec_rem += tz_sec_abs;                               /* See Note #1c2.                                       */
        if (ts_sec_rem < tz_sec_abs) {                          /* Chk for ovf when tz is pos.                          */
            CLK_TRACE_DBG(("    Timestamp is too big to add time zone offset\n\r"));
            return (DEF_FAIL);
        }
    }


/*$PAGE*/
                                                                /* ---------------------- GET YR ---------------------- */
    p_date_time->Yr =  yr_start;
    leap_yr         =  Clk_IsLeapYr(p_date_time->Yr);
    leap_yr_ix      = (leap_yr == DEF_YES) ? 1u : 0u;
    days_in_yr      =  Clk_DaysInYr[leap_yr_ix];
    sec_to_remove   =  days_in_yr * DEF_TIME_NBR_SEC_PER_DAY;
    while ((ts_sec_rem      >= sec_to_remove) &&
           (p_date_time->Yr <  yr_end)) {
        ts_sec_rem    -=  sec_to_remove;
        p_date_time->Yr++;
        leap_yr        =  Clk_IsLeapYr(p_date_time->Yr);
        leap_yr_ix     = (leap_yr == DEF_YES) ? 1u : 0u;
        days_in_yr     =  Clk_DaysInYr[leap_yr_ix];
        sec_to_remove  =  days_in_yr * DEF_TIME_NBR_SEC_PER_DAY;
    }

    if (p_date_time->Yr >= yr_end) {
        CLK_TRACE_DBG(("    Year conversion has failed\n\r"));
        return (DEF_FAIL);
    }

                                                                /* -------------------- GET MONTH --------------------- */
    p_date_time->Month =  CLK_FIRST_MONTH_OF_YR;
#if 0                                                           /* Already determined in 'GET YR'.                      */
    leap_yr            =  Clk_IsLeapYr(p_date_time->Yr);
    leap_yr_ix         = (leap_yr == DEF_YES) ? 1u : 0u;
#endif
    days_in_month      =  Clk_DaysInMonth[leap_yr_ix][p_date_time->Month - CLK_FIRST_MONTH_OF_YR];
    sec_to_remove      =  days_in_month * DEF_TIME_NBR_SEC_PER_DAY;
    while ((ts_sec_rem         >= sec_to_remove) &&
           (p_date_time->Month <  CLK_MONTH_PER_YR)) {
        ts_sec_rem    -=  sec_to_remove;
        p_date_time->Month++;
        days_in_month  =  Clk_DaysInMonth[leap_yr_ix][p_date_time->Month - CLK_FIRST_MONTH_OF_YR];
        sec_to_remove  =  days_in_month * DEF_TIME_NBR_SEC_PER_DAY;
    }

    if (p_date_time->Month > CLK_MONTH_PER_YR) {
        CLK_TRACE_DBG(("    Month conversion has failed\n\r"));
        return (DEF_FAIL);
    }

                                                                /* --------------------- GET DAY ---------------------- */
    sec_to_remove     = DEF_TIME_NBR_SEC_PER_DAY;
    p_date_time->Day  = ts_sec_rem / sec_to_remove;
    p_date_time->Day += CLK_FIRST_DAY_OF_MONTH;
    ts_sec_rem        = ts_sec_rem % sec_to_remove;
#if 0                                                           /* Already determined in 'GET MONTH'.                   */
    days_in_month     = Clk_DaysInMonth[leap_yr_ix][p_date_time->Month - CLK_FIRST_MONTH_OF_YR];
#endif

    if (p_date_time->Day > days_in_month) {
         CLK_TRACE_DBG(("    Day conversion has failed\n\r"));
         return (DEF_FAIL);
    }

                                                                /* ------------------ GET DAY OF WK ------------------- */
    p_date_time->DayOfWk = Clk_GetDayOfWkHandler(p_date_time->Yr,
                                                 p_date_time->Month,
                                                 p_date_time->Day);
    valid                = Clk_IsDayOfWkValid(p_date_time->DayOfWk);
    if (p_date_time->Day > days_in_month) {
        CLK_TRACE_DBG(("    Day conversion has failed\n\r"));
        return (DEF_FAIL);
    }


    p_date_time->DayOfYr = Clk_GetDayOfYrHandler(p_date_time->Yr,
                                                 p_date_time->Month,
                                                 p_date_time->Day);
    valid                = Clk_IsDayOfYrValid(p_date_time->Yr,
                                              p_date_time->DayOfYr);
    if (valid != DEF_OK) {
        CLK_TRACE_DBG(("    Day of year conversion has failed\n\r"));
        return (DEF_FAIL);
    }

                                                                /* --------------------- GET HR ----------------------- */
    sec_to_remove       = DEF_TIME_NBR_SEC_PER_HR;
    p_date_time->Hr     = ts_sec_rem / sec_to_remove;
    ts_sec_rem          = ts_sec_rem % sec_to_remove;

                                                                /* --------------------- GET MIN ---------------------- */
    sec_to_remove       = DEF_TIME_NBR_SEC_PER_MIN;
    p_date_time->Min    = ts_sec_rem / sec_to_remove;
    ts_sec_rem          = ts_sec_rem % sec_to_remove;

                                                                /* --------------------- GET SEC ---------------------- */
    p_date_time->Sec    = ts_sec_rem;

                                                                /* --------------------- GET TZ ----------------------- */
    p_date_time->TZ_sec = tz_sec;

/*$PAGE*/

                                                                /* ------------------ VALIDATE TIME ------------------- */
    valid = Clk_IsTimeValid(p_date_time->Hr, p_date_time->Min, p_date_time->Sec);
    if (valid != DEF_OK) {
        CLK_TRACE_DBG(("    Time conversion has failed\n\r"));
        return  (DEF_FAIL);
    }

    CLK_TRACE_DBG(("Date/time converted:  \n\r"
                   "     Year         = %u\n\r"
                   "     Month        = %u\n\r"
                   "     Day          = %u\n\r"
                   "     Hour         = %u\n\r"
                   "     Minutes      = %u\n\r"
                   "     Seconds      = %u\n\r"
                   "     Time zone    = %d\n\r",
                   (unsigned int)p_date_time->Yr,
                   (unsigned int)p_date_time->Month,
                   (unsigned int)p_date_time->Day,
                   (unsigned int)p_date_time->Hr,
                   (unsigned int)p_date_time->Min,
                   (unsigned int)p_date_time->Sec,
                   (  signed int)p_date_time->TZ_sec));

    return (DEF_OK);
}


static  CPU_BOOLEAN  Clk_IsLeapYr (CLK_YR  yr)
{
    CPU_BOOLEAN  leap_yr;


    leap_yr = ( ((yr %   4u) == 0u) &&                          /* Chk for leap yr (see Note #1).                       */
               (((yr % 100u) != 0u) || ((yr % 400u) == 0u))) ? DEF_YES : DEF_NO;

    return (leap_yr);
}


static  CPU_BOOLEAN  Clk_IsDayOfYrValid (CLK_YR   yr,
                                         CLK_DAY  day_of_yr)
{
    CPU_BOOLEAN  leap_yr;
    CPU_INT08U   leap_yr_ix;
    CLK_DAY      yr_days_max;

                                                                /* ---------------- VALIDATE DAY OF YR ---------------- */
    leap_yr     =  Clk_IsLeapYr(yr);
    leap_yr_ix  = (leap_yr == DEF_YES) ? 1u : 0u;
    yr_days_max =  Clk_DaysInYr[leap_yr_ix];
    if ((day_of_yr < CLK_FIRST_DAY_OF_YR) ||
        (day_of_yr > yr_days_max        )) {
        CLK_TRACE_DBG(("Invalid day of year, must be >= %u & < %u\n\r",
                       (unsigned int)CLK_FIRST_DAY_OF_YR,
                       (unsigned int)yr_days_max));
        return (DEF_NO);
    }

    return (DEF_YES);
}

static  CLK_DAY  Clk_GetDayOfWkHandler (CLK_YR     yr,
                                        CLK_MONTH  month,
                                        CLK_DAY    day)
{
    CPU_BOOLEAN   leap_yr;
    CPU_INT08U    leap_yr_ix;
    CLK_YR        yr_ix;
    CLK_MONTH     month_ix;
    CLK_DAY       day_of_wk;
    CLK_DAY       days_in_month;
    CLK_DAY       days_month;
    CLK_NBR_DAYS  days_yr;
    CLK_NBR_DAYS  days;


    CLK_TRACE_DBG(("Day of week of  %u, %u, %u = ",
                   (unsigned int)day,
                   (unsigned int)month,
                   (unsigned int)yr));

    days       =  day - CLK_FIRST_DAY_OF_MONTH;
    leap_yr    =  Clk_IsLeapYr(yr);
    leap_yr_ix = (leap_yr == DEF_YES) ? 1u : 0u;

    if (Clk_CacheMonth != month) {                              /* See Note #2a.                                        */
        days_month = 0u;
        for (month_ix = CLK_FIRST_MONTH_OF_YR; month_ix < month; month_ix++) {
            days_in_month  = Clk_DaysInMonth[leap_yr_ix][month_ix - CLK_FIRST_MONTH_OF_YR];
            days_month    += days_in_month;
        }

        Clk_CacheMonth     = month;                             /* See Note #2a.                                        */
        Clk_CacheMonthDays = days_month;                        /* See Note #2b.                                        */

    } else {
        days_month = Clk_CacheMonthDays;                        /* See Note #2b.                                        */
    }


    if (Clk_CacheYr != yr) {                                    /* See Note #2c.                                        */
        days_yr = 0u;
                                                                /* See Note #1a.                                        */
        for (yr_ix = CLK_NTP_EPOCH_YR_START; yr_ix < yr; yr_ix++) {
            leap_yr     =  Clk_IsLeapYr(yr_ix);
            leap_yr_ix  = (leap_yr == DEF_YES) ? 1u : 0u;
            days_yr    +=  Clk_DaysInYr[leap_yr_ix];
        }

        Clk_CacheYr     = yr;                                   /* See Note #2c.                                        */
        Clk_CacheYrDays = days_yr;                              /* See Note #2d.                                        */

    } else {
        days_yr = Clk_CacheYrDays;                              /* See Note #2b.                                        */
    }


    days      += days_month;
    days      += days_yr;
    days      += CLK_NTP_EPOCH_DAY_OF_WK;                       /* See Note #1b.                                        */
    days      -= CLK_FIRST_DAY_OF_WK;
    day_of_wk  = days % DEF_TIME_NBR_DAY_PER_WK;
    day_of_wk += CLK_FIRST_DAY_OF_WK;
    CLK_TRACE_DBG(("Day of week = %u)\n\r", (unsigned int)day_of_wk));

    return (day_of_wk);
}


static  CLK_DAY  Clk_GetDayOfYrHandler (CLK_YR     yr,
                                        CLK_MONTH  month,
                                        CLK_DAY    day)
{
    CPU_BOOLEAN  leap_yr;
    CPU_INT08U   leap_yr_ix;
    CLK_MONTH    month_ix;
    CLK_DAY      day_of_yr;
    CLK_DAY      days_month;
    CLK_DAY      days_in_month;


    CLK_TRACE_DBG(("Day of year of  %u, %u, %u = ",
                   (unsigned int)day,
                   (unsigned int)month,
                   (unsigned int)yr));

    day_of_yr  =  day - CLK_FIRST_DAY_OF_MONTH;
    leap_yr    =  Clk_IsLeapYr(yr);
    leap_yr_ix = (leap_yr == DEF_YES) ? 1u : 0u;

    if (Clk_CacheMonth != month) {                              /* See Note #1a.                                        */
        days_month = 0u;
        for (month_ix = CLK_FIRST_MONTH_OF_YR; month_ix < month; month_ix++) {
            days_in_month  = Clk_DaysInMonth[leap_yr_ix][month_ix - CLK_FIRST_MONTH_OF_YR];
            days_month    += days_in_month;
        }

        Clk_CacheMonth     = month;                             /* See Note #1a.                                        */
        Clk_CacheMonthDays = days_month;                        /* See Note #1b.                                        */

    } else {
        days_month = Clk_CacheMonthDays;                        /* See Note #1b.                                        */
    }

    day_of_yr += days_month;
    day_of_yr += CLK_FIRST_DAY_OF_YR;
    CLK_TRACE_DBG(("Day of year = %u\n\r", (unsigned int)day_of_yr));

    return (day_of_yr);
}


static  CPU_BOOLEAN  Clk_IsTimeValid (CLK_YR     hr,
                                      CLK_MONTH  min,
                                      CLK_DAY    sec)
{
                                                                /* ------------------ VALIDATE HOUR ------------------- */
    if (hr  >= DEF_TIME_NBR_HR_PER_DAY) {
        CLK_TRACE_DBG(("Invalid hour, must be < %u\n\r",    (unsigned int)DEF_TIME_NBR_HR_PER_DAY));
        return (DEF_NO);
    }

                                                                /* ------------------- VALIDATE MIN ------------------- */
    if (min >= DEF_TIME_NBR_MIN_PER_HR) {
        CLK_TRACE_DBG(("Invalid minute, must be < %u\n\r",  (unsigned int)DEF_TIME_NBR_MIN_PER_HR));
        return (DEF_NO);
    }

                                                                /* ------------------- VALIDATE SEC ------------------- */
    if (sec >  DEF_TIME_NBR_SEC_PER_MIN) {
        CLK_TRACE_DBG(("Invalid second, must be =< %u\n\r", (unsigned int)DEF_TIME_NBR_SEC_PER_MIN));
        return (DEF_NO);
    }

    return (DEF_YES);
}


static  CPU_BOOLEAN  Clk_IsDayOfWkValid (CLK_DAY  day_of_wk)
{
                                                                /* ---------------- VALIDATE DAY OF WK ---------------- */
    if ((day_of_wk < CLK_FIRST_DAY_OF_WK    ) ||
        (day_of_wk > DEF_TIME_NBR_DAY_PER_WK)) {
        CLK_TRACE_DBG(("Invalid day of week, must be >= %u & < %u\n\r",
                       (unsigned int)CLK_FIRST_DAY_OF_WK,
                       (unsigned int)DEF_TIME_NBR_DAY_PER_WK));
        return (DEF_NO);
    }

    return (DEF_YES);
}


CPU_BOOLEAN  Clk_DateTimeToTS (CLK_TS_SEC     *p_ts_sec,
                               CLK_DATE_TIME  *p_date_time)
{
    CPU_BOOLEAN  valid;


#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* ----------------- VALIDATE TS PTR ------------------ */
    if (p_ts_sec == (CLK_TS_SEC *)0) {
        return (DEF_FAIL);
    }
#endif

   *p_ts_sec = CLK_TS_SEC_NONE;                                 /* Init to ts none for err (see Note #3).               */


#if 0                                                           /* Validated in Clk_IsDateTimeValid().                  */
    if (p_date_time == (CLK_DATE_TIME *)0) {                    /* -------------- VALIDATE DATE/TIME PTR -------------- */
        return (DEF_FAIL);
    }
#endif

                                                                /* ---------------- VALIDATE DATE/TIME ---------------- */
    valid = Clk_IsDateTimeValid(p_date_time);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

                                                                /* ------------- CONV DATE/TIME TO CLK TS ------------- */
    CLK_TRACE_DBG(("Convert Date/time to TS:\n\r"));
    valid = Clk_DateTimeToTS_Handler(p_ts_sec,
                                     p_date_time,
                                     CLK_EPOCH_YR_START);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}


static  CPU_BOOLEAN  Clk_DateTimeToTS_Handler (CLK_TS_SEC     *p_ts_sec,
                                               CLK_DATE_TIME  *p_date_time,
                                               CLK_YR          yr_start)
{
    CPU_BOOLEAN   leap_yr;
    CPU_INT08U    leap_yr_ix;
    CLK_YR        yr_ix;
    CLK_MONTH     month_ix;
    CLK_NBR_DAYS  nbr_days;
    CLK_TS_SEC    ts_sec;
    CLK_TS_SEC    tz_sec_abs;                                   /* See Note #3.                                         */


#if 0                                                           /* See Note #2a.                                        */
   *p_ts_sec = CLK_TS_SEC_NONE;                                 /* Init to ts none for err (see Note #2).               */
#endif

    CLK_TRACE_DBG(("Date/time converted:  \n\r"
                   "     Year         = %u\n\r"
                   "     Month        = %u\n\r"
                   "     Day          = %u\n\r"
                   "     Hour         = %u\n\r"
                   "     Minutes      = %u\n\r"
                   "     Seconds      = %u\n\r"
                   "     Time zone    = %d\n\r",
                   (unsigned int)p_date_time->Yr,
                   (unsigned int)p_date_time->Month,
                   (unsigned int)p_date_time->Day,
                   (unsigned int)p_date_time->Hr,
                   (unsigned int)p_date_time->Min,
                   (unsigned int)p_date_time->Sec,
                   (  signed int)p_date_time->TZ_sec));

                                                                /* ------------- CONV DATE/TIME TO CLK TS ------------- */
    nbr_days   =  p_date_time->Day - CLK_FIRST_DAY_OF_MONTH;
    leap_yr    =  Clk_IsLeapYr(p_date_time->Yr);
    leap_yr_ix = (leap_yr == DEF_YES) ? 1u : 0u;
    for (month_ix = CLK_FIRST_MONTH_OF_YR; month_ix < p_date_time->Month; month_ix++) {
        nbr_days += Clk_DaysInMonth[leap_yr_ix][month_ix - CLK_FIRST_MONTH_OF_YR];
    }

    for (yr_ix = yr_start; yr_ix < p_date_time->Yr; yr_ix++) {
        leap_yr     =  Clk_IsLeapYr(yr_ix);
        leap_yr_ix  = (leap_yr == DEF_YES) ? 1u : 0u;
        nbr_days   +=  Clk_DaysInYr[leap_yr_ix];
    }

    ts_sec  = nbr_days         * DEF_TIME_NBR_SEC_PER_DAY;
    ts_sec += p_date_time->Hr  * DEF_TIME_NBR_SEC_PER_HR;
    ts_sec += p_date_time->Min * DEF_TIME_NBR_SEC_PER_MIN;
    ts_sec += p_date_time->Sec;

                                                                /* ------------ ADJ FINAL TS FOR TZ OFFSET ------------ */
	tz_sec_abs = DEF_ABS(p_date_time->TZ_sec);
    if (p_date_time->TZ_sec < 0) {
        ts_sec += tz_sec_abs;                                   /* See Note #1c1.                                       */
        if (ts_sec < tz_sec_abs) {                              /* Chk for ovf when tz is neg.                          */
            CLK_TRACE_DBG(("    Timestamp is too big to add time zone offset\n\r"));
            return (DEF_FAIL);
        }

    } else {
        if (ts_sec < tz_sec_abs) {                              /* Chk for ovf when tz is pos.                          */
            CLK_TRACE_DBG(("    Timestamp is too small to substract time zone offset\n\r"));
            return (DEF_FAIL);
        }
        ts_sec -= tz_sec_abs;                                   /* See Note #1c2.                                       */
    }

   *p_ts_sec = ts_sec;
    CLK_TRACE_DBG(("    TS converted       = %u\n\r", (unsigned int)*p_ts_sec));

    return (DEF_OK);
}


CPU_BOOLEAN  Clk_IsDateTimeValid (CLK_DATE_TIME  *p_date_time)
{
    CPU_BOOLEAN  valid;

                                                                /* -------------- VALIDATE CLK DATE/TIME -------------- */
    CLK_TRACE_DBG(("Validate Clock date/time: "));

    valid = Clk_IsDateTimeValidHandler(p_date_time,
                                       CLK_EPOCH_YR_START,
                                       CLK_EPOCH_YR_END);
    if (valid != DEF_YES) {
        CLK_TRACE_DBG(("Fail\n\r"));
        return  (DEF_NO);
    }

    CLK_TRACE_DBG(("Ok\n\r"));

    return (DEF_YES);
}


static  CPU_BOOLEAN  Clk_IsDateTimeValidHandler (CLK_DATE_TIME  *p_date_time,
                                                 CLK_YR          yr_start,
                                                 CLK_YR          yr_end)
{
    CPU_BOOLEAN  valid;


#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* -------------- VALIDATE DATE/TIME PTR -------------- */
    if (p_date_time == (CLK_DATE_TIME *)0) {
        return (DEF_NO);
    }
#endif

                                                                /* ------------------ VALIDATE DATE ------------------- */
    valid = Clk_IsDateValid(p_date_time->Yr, p_date_time->Month, p_date_time->Day, yr_start, yr_end);
    if (valid != DEF_YES) {
        return  (DEF_NO);
    }

                                                                /* ------------------ VALIDATE TIME ------------------- */
    valid = Clk_IsTimeValid(p_date_time->Hr, p_date_time->Min, p_date_time->Sec);
    if (valid != DEF_YES) {
        return  (DEF_NO);
    }

                                                                /* ---------------- VALIDATE DAY OF WK ---------------- */
    valid = Clk_IsDayOfWkValid(p_date_time->DayOfWk);
    if (valid != DEF_YES) {
        return  (DEF_NO);
    }

                                                                /* ---------------- VALIDATE DAY OF YR ---------------- */
    valid = Clk_IsDayOfYrValid(p_date_time->Yr, p_date_time->DayOfWk);
    if (valid != DEF_YES) {
        return  (DEF_NO);
    }

                                                                /* ------------------- VALIDATE TZ -------------------- */
    valid = Clk_IsTZValid(p_date_time->TZ_sec);
    if (valid != DEF_YES) {
        return  (DEF_NO);
    }


    return (DEF_YES);
}


static  CPU_BOOLEAN  Clk_IsDateValid (CLK_YR     yr,
                                      CLK_MONTH  month,
                                      CLK_DAY    day,
                                      CLK_YR     yr_start,
                                      CLK_YR     yr_end)
{
    CPU_BOOLEAN  leap_yr;
    CPU_INT08U   leap_yr_ix;
    CLK_DAY      days_in_month;

                                                                /* ------------------- VALIDATE YR -------------------- */
    if ((yr <  yr_start) ||
        (yr >= yr_end  )) {
         CLK_TRACE_DBG(("Invalid year, must be > %u & < %u\n\r",
                        (unsigned int)yr_end,
                        (unsigned int)yr_start));
         return (DEF_NO);
    }

                                                                /* ------------------ VALIDATE MONTH ------------------ */
    if ((month < CLK_FIRST_MONTH_OF_YR) ||
        (month > CLK_MONTH_PER_YR     )) {
        CLK_TRACE_DBG(("Invalid year, must be >= %u & < %u\n\r",
                       (unsigned int)CLK_FIRST_MONTH_OF_YR,
                       (unsigned int)CLK_MONTH_PER_YR));
        return (DEF_NO);
    }

                                                                /* ------------------- VALIDATE DAY ------------------- */
    leap_yr       =  Clk_IsLeapYr(yr);
    leap_yr_ix    = (leap_yr == DEF_YES) ? 1u : 0u;
    days_in_month =  Clk_DaysInMonth[leap_yr_ix][month - CLK_FIRST_MONTH_OF_YR];
    if ((day < CLK_FIRST_DAY_OF_MONTH) ||
        (day > days_in_month         )) {
         CLK_TRACE_DBG(("Invalid day, must be > %u & < %u\n\r",
                        (unsigned int)CLK_FIRST_DAY_OF_MONTH,
                        (unsigned int)days_in_month));
         return (DEF_NO);
    }


    return (DEF_YES);
}



CLK_DAY  Clk_GetDayOfWk (CLK_YR     yr,
                         CLK_MONTH  month,
                         CLK_DAY    day)
{
    CLK_DAY      day_of_wk;
    CPU_BOOLEAN  valid;

                                                                /* ------------------ VALIDATE DATE ------------------- */
    valid = Clk_IsDateValid(yr,
                            month,
                            day,
                            CLK_NTP_EPOCH_YR_START,
                            CLK_EPOCH_YR_END);
    if (valid != DEF_YES) {
        return (CLK_DAY_OF_WK_NONE);
    }

                                                                /* ------------------ GET DAY OF WK ------------------- */
    day_of_wk = Clk_GetDayOfWkHandler(yr, month, day);

    return (day_of_wk);
}


static  CPU_BOOLEAN  Clk_DateTimeMakeHandler (CLK_DATE_TIME  *p_date_time,
                                              CLK_YR          yr,
                                              CLK_MONTH       month,
                                              CLK_DAY         day,
                                              CLK_HR          hr,
                                              CLK_MIN         min,
                                              CLK_SEC         sec,
                                              CLK_TZ_SEC      tz_sec,
                                              CLK_YR          yr_start,
                                              CLK_YR          yr_end)
{
    CPU_BOOLEAN  valid;


#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* -------------- VALIDATE DATE/TIME PTR -------------- */
    if (p_date_time == (CLK_DATE_TIME *)0) {
        return (DEF_FAIL);
    }
#endif

                                                                /* ------------------ VALIDATE DATE ------------------- */
    valid = Clk_IsDateValid(yr, month, day, yr_start, yr_end);
    if (valid != DEF_YES) {
        return  (DEF_FAIL);
    }

    p_date_time->Yr      = yr;
    p_date_time->Month   = month;
    p_date_time->Day     = day;
    p_date_time->DayOfWk = Clk_GetDayOfWkHandler(yr, month, day);
    p_date_time->DayOfYr = Clk_GetDayOfYrHandler(yr, month, day);


                                                                /* ---------------- VALIDATE DAY OF WK ---------------- */
    valid = Clk_IsDayOfWkValid(p_date_time->DayOfWk);
    if (valid != DEF_YES) {
        return  (DEF_FAIL);
    }

                                                                /* ---------------- VALIDATE DAY OF YR ---------------- */
    valid = Clk_IsDayOfYrValid(yr, p_date_time->DayOfYr);
    if (valid != DEF_YES) {
        return  (DEF_FAIL);
    }

                                                                /* ------------------ VALIDATE TIME ------------------- */
    valid = Clk_IsTimeValid(hr, min, sec);
    if (valid != DEF_YES) {
        return  (DEF_FAIL);
    }

    p_date_time->Hr  = hr;
    p_date_time->Min = min;
    p_date_time->Sec = sec;

                                                                /* ------------------- VALIDATE TZ -------------------- */
    valid = Clk_IsTZValid(tz_sec);
    if (valid != DEF_YES) {
        return  (DEF_FAIL);
    }

    p_date_time->TZ_sec = tz_sec;

    return (DEF_OK);
}


CPU_BOOLEAN  Clk_DateTimeMake (CLK_DATE_TIME  *p_date_time,
                               CLK_YR          yr,
                               CLK_MONTH       month,
                               CLK_DAY         day,
                               CLK_HR          hr,
                               CLK_MIN         min,
                               CLK_SEC         sec,
                               CLK_TZ_SEC      tz_sec)
{
    CPU_BOOLEAN  valid;

                                                                /* ---------- VALIDATE & CONV CLK DATE/TIME ----------- */
    valid = Clk_DateTimeMakeHandler(p_date_time,
                                    yr,
                                    month,
                                    day,
                                    hr,
                                    min,
                                    sec,
                                    tz_sec,
                                    CLK_EPOCH_YR_START,
                                    CLK_EPOCH_YR_END);
    if (valid != DEF_YES) {
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}




/*
*********************************************************************************************************
*                                         Clk_DateTimeToStr()
*
* Description : Converts a date/time structure to an ASCII string.
*
* Argument(s) : p_date_time     Pointer to variable that contains the date/time structure to convert.
*
*               fmt             Desired string format :
*
*                                   CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_UTC     "YYYY-MM-DD HH:MM:SS UTC+TZ"
*                                   CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS         "YYYY-MM-DD HH:MM:SS"
*                                   CLK_STR_FMT_MM_DD_YY_HH_MM_SS           "MM-DD-YY HH:MM:SS"
*                                   CLK_STR_FMT_YYYY_MM_DD                  "YYYY-MM-DD"
*                                   CLK_STR_FMT_MM_DD_YY                    "MM-DD-YY"
*                                   CLK_STR_FMT_DAY_MONTH_DD_YYYY           "Day Month DD, YYYY"
*                                   CLK_STR_FMT_DAY_MONTH_DD_HH_MM_SS_YYYY  "Day Mon DD HH:MM:SS YYYY"
*                                   CLK_STR_FMT_HH_MM_SS                    "HH:MM:SS"
*                                   CLK_STR_FMT_HH_MM_SS_AM_PM              "HH:MM:SS AM|PM"
*
*               p_str           Pointer to buffer that will receive the formated string (see Note #2).
*
*               str_len         Maximum number of characters the string can contains.
*
* Return(s)   : DEF_OK,   if string successfully returned.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function & MAY
*               be called by application function(s).
*
* Note(s)     : (1) It's only possible to convert date supported by Clock :
*
*                   (a) Earliest year is the NTP   epoch start year, thus Year ('yr') MUST be greater
*                       than or equal to 'CLK_NTP_EPOCH_YR_START'.
*
*                   (b) Latest   year is the Clock epoch end   year, thus Year ('yr') MUST be less
*                       than 'CLK_EPOCH_YR_END'.
*
*               (2) The size of the string buffer that will receive the returned string address MUST be
*                   greater than or equal to CLK_STR_FMT_MAX_LEN.
*
*               (3) Pointers to variables that return values MUST be initialized PRIOR to all other
*                   validation or function handling in case of any error(s).
*
*               (4) Absolute value of the time zone offset is stored into 'CLK_TS_SEC' data type to be
*                   compliant with unsigned integer verification/operations.
*********************************************************************************************************
*/

#if (CLK_CFG_STR_CONV_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_DateTimeToStr (CLK_DATE_TIME  *p_date_time,
                                CLK_STR_FMT     fmt,
                                CPU_CHAR       *p_str,
                                CPU_SIZE_T      str_len)
{
    CPU_CHAR     yr   [CLK_STR_DIG_YR_LEN     + 1u];
    CPU_CHAR     month[CLK_STR_DIG_MONTH_LEN  + 1u];
    CPU_CHAR     day  [CLK_STR_DIG_DAY_LEN    + 1u];
    CPU_CHAR     hr   [CLK_STR_DIG_HR_LEN     + 1u];
    CPU_CHAR     min  [CLK_STR_DIG_MIN_LEN    + 1u];
    CPU_CHAR     sec  [CLK_STR_DIG_SEC_LEN    + 1u];
    CPU_CHAR     tz   [CLK_STR_DIG_TZ_MAX_LEN + 1u];
    CPU_CHAR     am_pm[CLK_STR_AM_PM_LEN      + 1u];
    CPU_BOOLEAN  valid;
    CLK_HR       half_day_hr;
    CLK_TS_SEC   tz_hr_abs;                                     /* See Note #4.                                         */
    CLK_TS_SEC   tz_min_abs;                                    /* See Note #4.                                         */
    CLK_TS_SEC   tz_sec_rem_abs;                                /* See Note #4.                                         */


#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* ----------------- VALIDATE STR PTR ----------------- */
    if (p_str == (CPU_CHAR *)0) {
        return (DEF_FAIL);
    }
    if (str_len < sizeof((CPU_CHAR)'\0')) {
        return (DEF_FAIL);
    }
#endif

   *p_str = '\0';                                               /* Init to NULL str for err (see Note #3).              */


/*$PAGE*/
#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* ----------------- VALIDATE STR LEN ----------------- */
    switch (fmt) {
        case CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_UTC:
             if (str_len < CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_UTC_LEN) {
                 return (DEF_FAIL);
             }
             break;

        case CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS:
             if (str_len < CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_LEN) {
                 return (DEF_FAIL);
             }
             break;

        case CLK_STR_FMT_MM_DD_YY_HH_MM_SS:
             if (str_len < CLK_STR_FMT_MM_DD_YY_HH_MM_SS_LEN) {
                 return (DEF_FAIL);
             }
             break;

        case CLK_STR_FMT_YYYY_MM_DD:
             if (str_len < CLK_STR_FMT_YYYY_MM_DD_LEN) {
                 return (DEF_FAIL);
             }
             break;

        case CLK_STR_FMT_MM_DD_YY:
             if (str_len < CLK_STR_FMT_MM_DD_YY_LEN) {
                 return (DEF_FAIL);
             }
             break;

        case CLK_STR_FMT_DAY_MONTH_DD_YYYY:
             if (str_len < CLK_STR_FMT_DAY_MONTH_DD_YYYY_MAX_LEN) {
                 return (DEF_FAIL);
             }
             break;

        case CLK_STR_FMT_DAY_MONTH_DD_HH_MM_SS_YYYY:
             if (str_len < CLK_STR_FMT_DAY_MONTH_DD_HH_MM_SS_YYYY_LEN) {
                 return (DEF_FAIL);
             }
             break;

        case CLK_STR_FMT_HH_MM_SS:
             if (str_len < CLK_STR_FMT_HH_MM_SS_LEN) {
                 return (DEF_FAIL);
             }
             break;

        case CLK_STR_FMT_HH_MM_SS_AM_PM:
             if (str_len < CLK_STR_FMT_HH_MM_SS_AM_PM_LEN) {
                 return (DEF_FAIL);
             }
             break;

        default:
             return (DEF_FAIL);
    }
#else
  (void)&str_len;                                               /* Prevent 'variable unused' compiler warning.          */
#endif


                                                                /* ---------------- VALIDATE DATE/TIME ---------------- */
#if 0                                                           /* Validated in Clk_IsDateTimeValidHandler().           */
    if (p_date_time == (CLK_DATE_TIME *)0) {
        return (DEF_FAIL);
    }
#endif

    valid = Clk_IsDateTimeValidHandler(p_date_time,
                                       CLK_NTP_EPOCH_YR_START,
                                       CLK_EPOCH_YR_END);
    if (valid != DEF_YES) {
        return  (DEF_FAIL);
    }



/*$PAGE*/
                                                                /* -------------- CREATE DATE/TIME STRS --------------- */
   (void)Str_FmtNbr_Int32U((CPU_INT32U )p_date_time->Yr,        /* Create yr str.                                       */
                           (CPU_INT08U )CLK_STR_DIG_YR_LEN,
                           (CPU_INT08U )DEF_NBR_BASE_DEC,
                           (CPU_CHAR   )'\0',
                           (CPU_BOOLEAN)DEF_NO,
                           (CPU_BOOLEAN)DEF_YES,
                           (CPU_CHAR  *)yr);

   (void)Str_FmtNbr_Int32U((CPU_INT32U )p_date_time->Month,     /* Create month (dig) str.                              */
                           (CPU_INT08U )CLK_STR_DIG_MONTH_LEN,
                           (CPU_INT08U )DEF_NBR_BASE_DEC,
                           (CPU_CHAR   )'0',
                           (CPU_BOOLEAN)DEF_NO,
                           (CPU_BOOLEAN)DEF_YES,
                           (CPU_CHAR  *)month);

   (void)Str_FmtNbr_Int32U((CPU_INT32U )p_date_time->Day,       /* Create day str.                                      */
                           (CPU_INT08U )CLK_STR_DIG_DAY_LEN,
                           (CPU_INT08U )DEF_NBR_BASE_DEC,
                           (CPU_CHAR   )'0',
                           (CPU_BOOLEAN)DEF_NO,
                           (CPU_BOOLEAN)DEF_YES,
                           (CPU_CHAR  *)day);

   (void)Str_FmtNbr_Int32U((CPU_INT32U )p_date_time->Hr,        /* Create hr str.                                       */
                           (CPU_INT08U )CLK_STR_DIG_HR_LEN,
                           (CPU_INT08U )DEF_NBR_BASE_DEC,
                           (CPU_CHAR   )'0',
                           (CPU_BOOLEAN)DEF_NO,
                           (CPU_BOOLEAN)DEF_YES,
                           (CPU_CHAR  *)hr);

   (void)Str_FmtNbr_Int32U((CPU_INT32U )p_date_time->Min,       /* Create min str.                                      */
                           (CPU_INT08U )CLK_STR_DIG_MIN_LEN,
                           (CPU_INT08U )DEF_NBR_BASE_DEC,
                           (CPU_CHAR   )'0',
                           (CPU_BOOLEAN)DEF_NO,
                           (CPU_BOOLEAN)DEF_YES,
                           (CPU_CHAR  *)min);

   (void)Str_FmtNbr_Int32U((CPU_INT32U )p_date_time->Sec,       /* Create sec str.                                      */
                           (CPU_INT08U )CLK_STR_DIG_SEC_LEN,
                           (CPU_INT08U )DEF_NBR_BASE_DEC,
                           (CPU_CHAR   )'0',
                           (CPU_BOOLEAN)DEF_NO,
                           (CPU_BOOLEAN)DEF_YES,
                           (CPU_CHAR  *)sec);



/*$PAGE*/
    switch (fmt) {                                              /* ---------------- FMT DATE/TIME STR ----------------- */

        case CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_UTC:               /* ------ BUILD STR "YYYY-MM-DD HH:MM:SS UTC+TZ" ------ */
             CLK_TRACE_DBG(("Date/time to string : YYYY-MM-DD HH:MM:SS UTC+TZ (+|-hh:mm)\n\r"));

            (void)Str_Copy_N(p_str, yr,     CLK_STR_DIG_YR_LEN + 1u);
            (void)Str_Cat_N (p_str, "-",    1u);

            (void)Str_Cat_N (p_str, month,  CLK_STR_DIG_MONTH_LEN);
            (void)Str_Cat_N (p_str, "-",    1u);

            (void)Str_Cat_N (p_str, day,    CLK_STR_DIG_DAY_LEN);
            (void)Str_Cat_N (p_str, " ",    1u);

            (void)Str_Cat_N (p_str, hr,     CLK_STR_DIG_HR_LEN);
            (void)Str_Cat_N (p_str, ":",    1u);

            (void)Str_Cat_N (p_str, min,    CLK_STR_DIG_MIN_LEN);
            (void)Str_Cat_N (p_str, ":",    1u);

            (void)Str_Cat_N (p_str, sec,    CLK_STR_DIG_SEC_LEN);
            (void)Str_Cat_N (p_str, " ",    1u);

            (void)Str_Cat_N (p_str, "UTC",  3u);

             if (p_date_time->TZ_sec >= 0) {
                (void)Str_Cat_N(p_str, "+", 1u);
             } else {
                (void)Str_Cat_N(p_str, "-", 1u);
             }

             tz_sec_rem_abs = DEF_ABS(p_date_time->TZ_sec);
             tz_hr_abs      = tz_sec_rem_abs / DEF_TIME_NBR_SEC_PER_HR;
             tz_sec_rem_abs = tz_sec_rem_abs % DEF_TIME_NBR_SEC_PER_HR;
            (void)Str_FmtNbr_Int32U((CPU_INT32S )tz_hr_abs,
                                    (CPU_INT08U )CLK_STR_DIG_TZ_HR_LEN,
                                    (CPU_INT08U )DEF_NBR_BASE_DEC,
                                    (CPU_CHAR   )'0',
                                    (CPU_BOOLEAN)DEF_NO,
                                    (CPU_BOOLEAN)DEF_YES,
                                    (CPU_CHAR  *)tz);

            (void)Str_Cat_N(p_str, tz,  CLK_STR_DIG_TZ_HR_LEN);
            (void)Str_Cat_N(p_str, ":", 1u);

             tz_min_abs = tz_sec_rem_abs / DEF_TIME_NBR_SEC_PER_MIN;
            (void)Str_FmtNbr_Int32U((CPU_INT32S )tz_min_abs,
                                    (CPU_INT08U )CLK_STR_DIG_TZ_MIN_LEN,
                                    (CPU_INT08U )DEF_NBR_BASE_DEC,
                                    (CPU_CHAR   )'0',
                                    (CPU_BOOLEAN)DEF_NO,
                                    (CPU_BOOLEAN)DEF_YES,
                                    (CPU_CHAR  *)tz);

            (void)Str_Cat_N(p_str, tz,  CLK_STR_DIG_TZ_MIN_LEN);
             break;



        case CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS:                   /* --------- BUILD STR "YYYY-MM-DD HH:MM:SS" ---------- */
             CLK_TRACE_DBG(("Date/time to string : YYYY-MM-DD HH:MM:SS\n\r"));

            (void)Str_Copy_N(p_str, yr,    CLK_STR_DIG_YR_LEN + 1u);
            (void)Str_Cat_N (p_str, "-",   1u);

            (void)Str_Cat_N (p_str, month, CLK_STR_DIG_MONTH_LEN);
            (void)Str_Cat_N (p_str, "-",   1u);

            (void)Str_Cat_N (p_str, day,   CLK_STR_DIG_DAY_LEN);
            (void)Str_Cat_N (p_str, " ",   1u);

            (void)Str_Cat_N (p_str, hr,    CLK_STR_DIG_HR_LEN);
            (void)Str_Cat_N (p_str, ":",   1u);

            (void)Str_Cat_N (p_str, min,   CLK_STR_DIG_MIN_LEN);
            (void)Str_Cat_N (p_str, ":",   1u);

            (void)Str_Cat_N (p_str, sec,   CLK_STR_DIG_SEC_LEN);
             break;


/*$PAGE*/
        case CLK_STR_FMT_MM_DD_YY_HH_MM_SS:                     /* ---------- BUILD STR "MM-DD-YY HH:MM:SS" ----------- */
             CLK_TRACE_DBG(("Date/time to string : MM-DD-YY HH:MM:SS\n\r"));

            (void)Str_FmtNbr_Int32U((CPU_INT32U )p_date_time->Yr,
                                    (CPU_INT08U )2u,
                                    (CPU_INT08U )DEF_NBR_BASE_DEC,
                                    (CPU_CHAR   )' ',
                                    (CPU_BOOLEAN)DEF_NO,
                                    (CPU_BOOLEAN)DEF_YES,
                                    (CPU_CHAR  *)yr);

            (void)Str_Copy_N(p_str, month, CLK_STR_DIG_MONTH_LEN + 1u);
            (void)Str_Cat_N (p_str, "-",   1u);

            (void)Str_Cat_N (p_str, day,   CLK_STR_DIG_DAY_LEN);
            (void)Str_Cat_N (p_str, "-",   1u);

            (void)Str_Cat_N (p_str, yr,    CLK_STR_DIG_YR_TRUNC_LEN);
            (void)Str_Cat_N (p_str, " ",   1u);

            (void)Str_Cat_N (p_str, hr,    CLK_STR_DIG_HR_LEN);
            (void)Str_Cat_N (p_str, ":",   1u);

            (void)Str_Cat_N (p_str, min,   CLK_STR_DIG_MIN_LEN);
            (void)Str_Cat_N (p_str, ":",   1u);

            (void)Str_Cat_N (p_str, sec,   CLK_STR_DIG_SEC_LEN);
             break;



        case CLK_STR_FMT_YYYY_MM_DD:                            /* -------------- BUILD STR "YYYY-MM-DD" -------------- */
             CLK_TRACE_DBG(("Date/time to string : YYYY-MM-DD\n\r"));

            (void)Str_Copy_N(p_str, yr,    CLK_STR_DIG_YR_LEN + 1u);
            (void)Str_Cat_N (p_str, "-",   1u);

            (void)Str_Cat_N (p_str, month, CLK_STR_DIG_MONTH_LEN);
            (void)Str_Cat_N (p_str,  "-",  1u);

            (void)Str_Cat_N (p_str, day,   CLK_STR_DIG_DAY_LEN);
             break;



        case CLK_STR_FMT_MM_DD_YY:                              /* --------------- BUILD STR ""MM-DD-YY" -------------- */
             CLK_TRACE_DBG(("Date/time to string : MM-DD-YY\n\r"));

            (void)Str_FmtNbr_Int32U((CPU_INT32U )p_date_time->Yr,
                                    (CPU_INT08U )2u,
                                    (CPU_INT08U )DEF_NBR_BASE_DEC,
                                    (CPU_CHAR   )' ',
                                    (CPU_BOOLEAN)DEF_NO,
                                    (CPU_BOOLEAN)DEF_YES,
                                    (CPU_CHAR  *)yr);

            (void)Str_Copy_N(p_str, month, CLK_STR_DIG_MONTH_LEN + 1u);
            (void)Str_Cat_N (p_str, "-",   1u);

            (void)Str_Cat_N (p_str, day,   CLK_STR_DIG_DAY_LEN);
            (void)Str_Cat_N (p_str, "-",   1u);

            (void)Str_Cat_N (p_str, yr,    CLK_STR_DIG_YR_TRUNC_LEN);
             break;



        case CLK_STR_FMT_DAY_MONTH_DD_YYYY:                     /* ---------- BUILD STR "Day Month DD, YYYY" ---------- */
             CLK_TRACE_DBG(("Date/time to string : Day Month DD, YYYY\n\r"));

            (void)Str_Copy_N((CPU_CHAR *)p_str,
                             (CPU_CHAR *)Clk_StrDayOfWk[p_date_time->DayOfWk - CLK_FIRST_DAY_OF_WK],
                             (CPU_SIZE_T)CLK_STR_DAY_OF_WK_MAX_LEN + 1u);
            (void)Str_Cat_N(  p_str, " ",  1u);

            (void)Str_Cat_N( (CPU_CHAR *)p_str,
                             (CPU_CHAR *)Clk_StrMonth[p_date_time->Month - CLK_FIRST_DAY_OF_MONTH],
                             (CPU_SIZE_T)CLK_STR_MONTH_MAX_LEN);
            (void)Str_Cat_N(  p_str, " ",  1u);

            (void)Str_Cat_N(  p_str, day,  CLK_STR_DIG_DAY_LEN);
            (void)Str_Cat_N(  p_str, ", ", 2u);

            (void)Str_Cat_N(  p_str, yr,   CLK_STR_DIG_YR_LEN);
             break;


/*$PAGE*/
        case CLK_STR_FMT_DAY_MONTH_DD_HH_MM_SS_YYYY:            /* ------- BUILD STR "Day Mon DD HH:MM:SS YYYY" ------- */
             CLK_TRACE_DBG(("Date/time to string : Day Mon DD HH:MM:SS YYYY\n\r"));

            (void)Str_Copy_N((CPU_CHAR *)p_str,
                             (CPU_CHAR *)Clk_StrDayOfWk[p_date_time->DayOfWk - CLK_FIRST_DAY_OF_WK],
                             (CPU_SIZE_T)CLK_STR_DAY_OF_WK_TRUNC_LEN + 1u);
             p_str[3] = '\0';
            (void)Str_Cat_N (p_str, " ", 1u);

            (void)Str_Cat_N((CPU_CHAR  *)p_str,
                            (CPU_CHAR  *)Clk_StrMonth[p_date_time->Month - CLK_FIRST_DAY_OF_MONTH],
                            (CPU_SIZE_T )CLK_STR_MONTH_TRUNC_LEN);
            (void)Str_Cat_N (p_str, " ", 1u);

            (void)Str_Cat_N (p_str, day, CLK_STR_DIG_DAY_LEN);
            (void)Str_Cat_N (p_str, " ", 1u);

            (void)Str_Cat_N (p_str, hr,  CLK_STR_DIG_HR_LEN);
            (void)Str_Cat_N (p_str, ":", 1u);

            (void)Str_Cat_N (p_str, min, CLK_STR_DIG_MIN_LEN);
            (void)Str_Cat_N (p_str, ":", 1u);

            (void)Str_Cat_N (p_str, sec, CLK_STR_DIG_SEC_LEN);
            (void)Str_Cat_N (p_str, " ", 1u);

            (void)Str_Cat_N (p_str, yr,  CLK_STR_DIG_YR_LEN);
             break;



        case CLK_STR_FMT_HH_MM_SS:                              /* --------------- BUILD STR "HH:MM:SS" --------------- */
             CLK_TRACE_DBG(("Date/time to string : HH:MM:SS\n\r"));

            (void)Str_Copy_N(p_str, hr,  CLK_STR_DIG_HR_LEN + 1u);
            (void)Str_Cat_N (p_str, ":", 1u);

            (void)Str_Cat_N (p_str, min, CLK_STR_DIG_MIN_LEN);
            (void)Str_Cat_N (p_str, ":", 1u);

            (void)Str_Cat_N (p_str, sec, CLK_STR_DIG_SEC_LEN);
             break;



        case CLK_STR_FMT_HH_MM_SS_AM_PM:                        /* ------------ BUILD STR "HH:MM:SS AM|PM" ------------ */
             CLK_TRACE_DBG(("Date/time to string : HH:MM:SS AM|PM\n\r"));

             if (p_date_time->Hr < CLK_HR_PER_HALF_DAY) {       /* Chk for AM or PM.                                    */
                (void)Str_Copy_N(am_pm, "AM", CLK_STR_AM_PM_LEN + 1u);
                 if (p_date_time->Hr == 0u) {
                     half_day_hr = CLK_HR_PER_HALF_DAY;
                 } else {
                     half_day_hr = p_date_time->Hr;
                 }
             } else {
                (void)Str_Copy_N(am_pm, "PM", CLK_STR_AM_PM_LEN + 1u);
                 half_day_hr = p_date_time->Hr - CLK_HR_PER_HALF_DAY;
             }

            (void)Str_FmtNbr_Int32U((CPU_INT32U )half_day_hr,
                                    (CPU_INT08U )CLK_STR_DIG_HR_LEN,
                                    (CPU_INT08U )DEF_NBR_BASE_DEC,
                                    (CPU_CHAR   )'0',
                                    (CPU_BOOLEAN)DEF_NO,
                                    (CPU_BOOLEAN)DEF_YES,
                                    (CPU_CHAR  *)hr);

            (void)Str_Copy_N(p_str, hr,    CLK_STR_DIG_HR_LEN + 1u);
            (void)Str_Cat_N (p_str, ":",   1u);

            (void)Str_Cat_N (p_str, min,   CLK_STR_DIG_MIN_LEN);
            (void)Str_Cat_N (p_str, ":",   1u);

            (void)Str_Cat_N (p_str, sec,   CLK_STR_DIG_SEC_LEN);
            (void)Str_Cat_N (p_str, " ",   1u);

            (void)Str_Cat_N (p_str, am_pm, CLK_STR_AM_PM_LEN);
             break;



        default:                                                /* ------------------- INVALID FMT -------------------- */
             CLK_TRACE_DBG(("Date/time to string : Invalid format\n\r"));
             return (DEF_FAIL);
    }

    CLK_TRACE_DBG(("    %s\n\r", p_str));

    return (DEF_OK);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                           Clk_GetTS_NTP()
*
* Description : Get current Clock timestamp as an NTP timestamp.
*
* Argument(s) : p_ts_ntp_sec    Pointer to variable that will receive the NTP timestamp :
*
*                                   In seconds UTC+00,  if NO error(s);
*                                   CLK_TS_SEC_NONE,    otherwise.
*
* Return(s)   : DEF_OK,   if timestamp successfully returned.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) (a) NTP timestamp is converted from the internal Clock timestamp which SHOULD
*                       be set for UTC+00. Thus the NTP timestamp is returned at UTC+00.
*
*                   (b) NTP timestamp does NOT include the internal Clock time zone. Thus any
*                       local time zone offset MUST be applied after calling Clk_GetTS_NTP().
*
*               (2) NTP timestamp will eventually overflow, thus it's not possible to get NTP
*                   timestamp for years on or after CLK_NTP_EPOCH_YR_END.
*
*               (3) Pointers to variables that return values MUST be initialized PRIOR to all
*                   other validation or function handling in case of any error(s).
*********************************************************************************************************
*/

#if (CLK_CFG_NTP_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_GetTS_NTP (CLK_TS_SEC  *p_ts_ntp_sec)
{
    CLK_TS_SEC   ts_sec;
    CPU_BOOLEAN  valid;


#if 0                                                           /* Validated & init'd in Clk_TS_ToTS_NTP().             */
    if (p_ts_ntp_sec == (CLK_TS_SEC *)0) {                      /* ----------------- VALIDATE TS PTR ------------------ */
        return (DEF_FAIL);
    }

   *p_ts_ntp_sec = CLK_TS_SEC_NONE;                             /* Init to ts none for err (see Note #3).               */
#endif

                                                                /* -------------------- GET CLK TS -------------------- */
    ts_sec = Clk_GetTS();

                                                                /* -------------- CONV CLK TS TO NTP TS --------------- */
    valid  = Clk_TS_ToTS_NTP(ts_sec, p_ts_ntp_sec);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           Clk_SetTS_NTP()
*
* Description : Set Clock timestamp from an NTP timestamp.
*
* Argument(s) : ts_ntp_sec      Current NTP timestamp to set (in seconds, UTC+00).
*
* Return(s)   : DEF_OK,   if timestamp successfully set.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function & MAY
*               be called by application function(s).
*
* Note(s)     : (1) Internal Clock timestamp SHOULD be set for UTC+00 and should NOT include any local
*                   time zone offset.
*
*               (2) Only years supported by Clock & NTP can be set, thus the timestamp date MUST be :
*
*                   (a) >= CLK_EPOCH_YR_START
*                   (b) <  CLK_NTP_EPOCH_YR_END
*********************************************************************************************************
*/

#if (CLK_CFG_NTP_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_SetTS_NTP (CLK_TS_SEC  ts_ntp_sec)
{
    CLK_TS_SEC   ts_sec;
    CPU_BOOLEAN  valid;

                                                                /* -------------- CONV NTP TS TO CLK TS --------------- */
    valid = Clk_TS_NTP_ToTS(&ts_sec, ts_ntp_sec);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

                                                                /* -------------------- SET CLK TS -------------------- */
    Clk_SetTS(ts_sec);

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                          Clk_TS_ToTS_NTP()
*
* Description : Convert Clock timestamp to NTP timestamp.
*
* Argument(s) : ts_sec          Timestamp to convert (in seconds, UTC+00).
*
*               p_ts_ntp_sec    Pointer to variable that will receive the NTP timestamp :
*
*                                   In seconds UTC+00,  if NO error(s);
*                                   CLK_TS_SEC_NONE,    otherwise.
*
* Return(s)   : DEF_OK,   if timestamp successfully converted.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Clk_GetTS_NTP(),
*               Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) Returned timestamp does NOT include any time zone offset. Thus any local time
*                   zone offset SHOULD be applied before or after calling Clk_TS_ToTS_NTP().
*
*               (2) Only years supported by Clock & NTP can be converted, thus the timestamp date
*                   MUST be :
*
*                   (a) >= CLK_EPOCH_YR_START
*                   (b) <  CLK_NTP_EPOCH_YR_END
*
*               (3) Pointers to variables that return values MUST be initialized PRIOR to all
*                   other validation or function handling in case of any error(s).
*********************************************************************************************************
*/

#if (CLK_CFG_NTP_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_TS_ToTS_NTP (CLK_TS_SEC   ts_sec,
                              CLK_TS_SEC  *p_ts_ntp_sec)
{
    CLK_TS_SEC  ts_ntp_sec;


#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* ----------------- VALIDATE TS PTR ------------------ */
    if (p_ts_ntp_sec == (CLK_TS_SEC *)0) {
        return (DEF_FAIL);
    }
#endif

   *p_ts_ntp_sec = CLK_TS_SEC_NONE;                             /* Init to ts none for err (see Note #3).               */

    CLK_TRACE_DBG(("\n\rConvert TS to NTP TS:\n\r"
                   "    TS = %u\n\r", (unsigned int)ts_sec));

                                                                /* -------------- CONV CLK TS TO NTP TS --------------- */
    ts_ntp_sec = ts_sec + CLK_NTP_EPOCH_OFFSET_SEC;
    if (ts_ntp_sec < CLK_NTP_EPOCH_OFFSET_SEC) {                /* Chk for ovf.                                         */
        CLK_TRACE_DBG(("    NTP TS conversion has failed\n\r"));
        return (DEF_FAIL);
    }

   *p_ts_ntp_sec = ts_ntp_sec;
    CLK_TRACE_DBG(("    NTP TS converted = %u\n\r", (unsigned int)*p_ts_ntp_sec));

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                          Clk_TS_NTP_ToTS()
*
* Description : Convert NTP timestamp to Clock timestamp.
*
* Argument(s) : p_ts_sec        Pointer to variable that will receive the Clock timestamp :
*
*                                   In seconds UTC+00,  if NO error(s);
*                                   CLK_TS_SEC_NONE,    otherwise.
*
*               ts_ntp_sec      NTP timestamp value to convert (in seconds, UTC+00).
*
* Return(s)   : DEF_OK,   if timestamp successfully converted.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Clk_SetTS_NTP(),
*               Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) Returned timestamp does NOT include any time zone offset. Thus any local time
*                   zone offset SHOULD be applied before or after calling Clk_TS_NTP_ToTS().
*
*               (2) Only years supported by Clock & NTP can be converted, thus the timestamp date
*                   MUST be :
*
*                   (a) >= CLK_EPOCH_YR_START
*                   (b) <  CLK_NTP_EPOCH_YR_END
*
*               (3) Pointers to variables that return values MUST be initialized PRIOR to all other
*                   validation or function handling in case of any error(s).
*********************************************************************************************************
*/

#if (CLK_CFG_NTP_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_TS_NTP_ToTS (CLK_TS_SEC  *p_ts_sec,
                              CLK_TS_SEC   ts_ntp_sec)
{
#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* ----------------- VALIDATE TS PTR ------------------ */
    if (p_ts_sec == (CLK_TS_SEC *)0) {
        return (DEF_FAIL);
    }
#endif

   *p_ts_sec = CLK_TS_SEC_NONE;                                 /* Init to ts none for err (see Note #3).               */

    CLK_TRACE_DBG(("Convert NTP TS to TS:\n\r"
                   "    NTP TS = %u\n\r", (unsigned int)ts_ntp_sec));

    if (ts_ntp_sec < CLK_NTP_EPOCH_OFFSET_SEC) {                /* Chk for ovf.                                         */
        CLK_TRACE_DBG(("TS overflow\n\r"));
        return (DEF_FAIL);
    }

                                                                /* -------------- CONV NTP TS TO CLK TS --------------- */
   *p_ts_sec = ts_ntp_sec - CLK_NTP_EPOCH_OFFSET_SEC;
    CLK_TRACE_DBG(("    TS converted = %u\n\r", (unsigned int)*p_ts_sec));

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       Clk_TS_NTP_ToDateTime()
*
* Description : Convert NTP timestamp to date/time structure.
*
* Argument(s) : ts_ntp_sec      Timestamp to convert (in seconds,          UTC+00).
*
*               tz_sec          Time zone offset     (in seconds, +|- from UTC).
*
*               p_date_time     Pointer to variable that will receive the date/time structure.
*
* Return(s)   : DEF_OK,   if date/time structure successfully returned.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function & MAY
*               be called by application function(s).
*
* Note(s)     : (1) (a) Timestamp ('ts_ntp_sec') MUST be set for UTC+00 & SHOULD NOT include the time
*                       zone offset ('tz_sec') since Clk_TS_NTP_ToDateTime() includes the time zone
*                       offset in its date/time calculation. Thus the time zone offset SHOULD NOT be
*                       applied before or after calling Clk_TS_NTP_ToDateTime().
*
*                   (b) Time zone field of the date/time structure ('p_date_time->TZ_sec') is set to
*                       the value of the time zone argument ('tz_sec').
*
*                   (c) Time zone is based on Coordinated Universal Time (UTC) & has valid values :
*
*                       (1) Between  +|- 12 hours (+|- 43200 seconds)
*                       (2) Multiples of 15 minutes
*********************************************************************************************************
*/

#if (CLK_CFG_NTP_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_TS_NTP_ToDateTime (CLK_TS_SEC      ts_ntp_sec,
                                    CLK_TZ_SEC      tz_sec,
                                    CLK_DATE_TIME  *p_date_time)
{
    CPU_BOOLEAN  valid;

                                                                /* ------------- CONV NTP TS TO DATE/TIME ------------- */
    CLK_TRACE_DBG(("\n\rConvert TS NTP to Date/time:\n\r"
                   "    TS to convert= %u\n\r\n\r",
                   (unsigned int)ts_ntp_sec));

    valid = Clk_TS_ToDateTimeHandler(ts_ntp_sec,
                                     tz_sec,
                                     p_date_time,
                                     CLK_NTP_EPOCH_YR_START,
                                     CLK_NTP_EPOCH_YR_END);
    if (valid != DEF_OK) {
        CLK_TRACE_DBG(("    Date/time conversion has failed\n\r"));
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       Clk_DateTimeToTS_NTP()
*
* Description : Convert a date/time structure to NTP timestamp.
*
* Argument(s) : p_ts_ntp_sec    Pointer to variable that will receive the NTP timestamp :
*
*                                   In seconds UTC+00,  if NO error(s);
*                                   CLK_TS_SEC_NONE,    otherwise.
*
*               p_date_time     Pointer to variable that contains date/time structure to convert.
*
* Return(s)   : DEF_OK,   if timestamp successfully returned.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) Date/time structure ('p_date_time') MUST be representable in NTP timestamp.
*                   Thus date to convert MUST be :
*
*                   (a) >= CLK_NTP_EPOCH_YR_START
*                   (b) <  CLK_NTP_EPOCH_YR_END
*
*               (2) (a) Date/time ('p_date_time') SHOULD be set to local time with correct time zone
*                       offset ('p_date_time->TZ_sec'). Clk_DateTimeToTS_NTP() removes the time zone
*                       offset from the date/time to calculate & return a Clock timestamp at UTC+00.
*
*                   (b) Time zone is based on Coordinated Universal Time (UTC) & has valid values :
*
*                       (1) Between  +|- 12 hours (+|- 43200 seconds)
*                       (2) Multiples of 15 minutes
*
*               (3) Pointers to variables that return values MUST be initialized PRIOR to all other
*                   validation or function handling in case of any error(s).
*********************************************************************************************************
*/

#if (CLK_CFG_NTP_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_DateTimeToTS_NTP (CLK_TS_SEC     *p_ts_ntp_sec,
                                   CLK_DATE_TIME  *p_date_time)
{
    CPU_BOOLEAN  valid;


#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* ----------------- VALIDATE TS PTR ------------------ */
    if (p_ts_ntp_sec == (CLK_TS_SEC *)0) {
        return (DEF_FAIL);
    }
#endif

   *p_ts_ntp_sec = CLK_TS_SEC_NONE;                             /* Init to ts none for err (see Note #3).               */


#if 0                                                           /* Validated in Clk_IsNTP_DateTimeValid().              */
    if (p_date_time == (CLK_DATE_TIME *)0) {                    /* -------------- VALIDATE DATE/TIME PTR -------------- */
        return (DEF_FAIL);
    }
#endif

                                                                /* ---------------- VALIDATE DATE/TIME ---------------- */
    valid = Clk_IsNTP_DateTimeValid(p_date_time);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

                                                                /* ------------- CONV DATE/TIME TO NTP TS ------------- */
    CLK_TRACE_DBG(("Convert Date/time to TS:\n\r"));
    valid = Clk_DateTimeToTS_Handler(p_ts_ntp_sec,
                                     p_date_time,
                                     CLK_NTP_EPOCH_YR_START);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       Clk_NTP_DateTimeMake()
*
* Description : Build a valid NTP epoch date/time structure.
*
* Argument(s) : p_date_time     Pointer to variable that will receive the date/time structure.
*
*               yr              Year    value [CLK_NTP_EPOCH_YR_START to CLK_NTP_EPOCH_YR_END) (see Note #1).
*
*               month           Month   value [         CLK_MONTH_JAN to        CLK_MONTH_DEC].
*
*               day             Day     value [                     1 to                   31].
*
*               hr              Hours   value [                     0 to                   23].
*
*               min             Minutes value [                     0 to                   59].
*
*               sec             Seconds value [                     0 to                   60] (see Note #3).
*
*               tz_sec          Time zone offset (in seconds, +|- from UTC) [-43200 to 43200].
*
* Return(s)   : DEF_OK,   if date/time structure successfully returned.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) Date/time structure ('p_date_time') MUST be representable in NTP timestamp.
*                   Thus date to convert MUST be :
*
*                   (a) >= CLK_NTP_EPOCH_YR_START
*                   (b) <  CLK_NTP_EPOCH_YR_END
*
*               (2) Day of week ('p_date_time->DayOfWk') and Day of year ('p_date_time->DayOfYr')
*                   are internally calculated and set in the date/time structure if date is valid.
*
*               (3) Seconds value of 60 is valid to be compatible with leap second adjustment and
*                   the atomic clock time structure.
*
*               (4) Time zone is based ('tz_sec') on Coordinated Universal Time (UTC) & has valid
*                   values :
*
*                   (a) Between  +|- 12 hours (+|- 43200 seconds)
*                   (b) Multiples of 15 minutes
*********************************************************************************************************
*/

#if (CLK_CFG_NTP_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_NTP_DateTimeMake (CLK_DATE_TIME  *p_date_time,
                                   CLK_YR          yr,
                                   CLK_MONTH       month,
                                   CLK_DAY         day,
                                   CLK_HR          hr,
                                   CLK_MIN         min,
                                   CLK_SEC         sec,
                                   CLK_TZ_SEC      tz_sec)
{
    CPU_BOOLEAN  valid;

                                                                /* ---------- VALIDATE & CONV NTP DATE/TIME ----------- */
    valid = Clk_DateTimeMakeHandler(p_date_time,
                                    yr,
                                    month,
                                    day,
                                    hr,
                                    min,
                                    sec,
                                    tz_sec,
                                    CLK_NTP_EPOCH_YR_START,
                                    CLK_NTP_EPOCH_YR_END);
    if (valid != DEF_YES) {
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                      Clk_IsNTP_DateTimeValid()
*
* Description : Determine if date/time structure is valid in NTP epoch.
*
* Argument(s) : p_date_time     Pointer to variable that contains the date/time structure to validate.
*
* Return(s)   : DEF_YES, if date/time structure is valid.
*
*               DEF_NO,  otherwise.
*
* Caller(s)   : Clk_DateTimeToTS_NTP(),
*               Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) Date/time structure ('p_date_time') MUST be representable in NTP epoch. Thus
*                   date to validate MUST be :
*
*                   (a) >= CLK_NTP_EPOCH_YR_START
*                   (b) <  CLK_NTP_EPOCH_YR_END
*
*               (2) Time zone is based ('p_date_time->TZ_sec') on Coordinated Universal Time (UTC)
*                   & has valid values :
*
*                   (a) Between  +|- 12 hours (+|- 43200 seconds)
*                   (b) Multiples of 15 minutes
*********************************************************************************************************
*/

#if (CLK_CFG_NTP_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_IsNTP_DateTimeValid (CLK_DATE_TIME  *p_date_time)
{
    CPU_BOOLEAN  valid;

                                                                /* -------------- VALIDATE NTP DATE/TIME -------------- */
    CLK_TRACE_DBG(("Validate NTP date/time: "));

    valid = Clk_IsDateTimeValidHandler(p_date_time,
                                       CLK_NTP_EPOCH_YR_START,
                                       CLK_NTP_EPOCH_YR_END);
    if (valid != DEF_YES) {
        CLK_TRACE_DBG(("Fail\n\r"));
        return  (DEF_NO);
    }

    CLK_TRACE_DBG(("Ok\n\r"));

    return (DEF_YES);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                          Clk_GetTS_Unix()
*
* Description : Get current Clock timestamp as a Unix timestamp.
*
* Argument(s) : p_ts_unix_sec   Pointer to variable that will receive the Unix timestamp :
*
*                                   In seconds UTC+00,  if NO error(s);
*                                   CLK_TS_SEC_NONE,    otherwise.
*
* Return(s)   : DEF_OK,   if timestamp successfully returned.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) (a) Unix timestamp is converted from the internal Clock timestamp which SHOULD
*                       be set for UTC+00. Thus the Unix timestamp is returned at UTC+00.
*
*                   (b) Unix timestamp does NOT include the internal Clock time zone. Thus any
*                       local time zone offset MUST be applied after calling Clk_GetTS_Unix().
*
*               (2) Unix timestamp will eventually overflow, thus it's not possible to get Unix
*                   timestamp for years on or after CLK_UNIX_EPOCH_YR_END.
*
*               (3) Pointers to variables that return values MUST be initialized PRIOR to all
*                   other validation or function handling in case of any error(s).
*********************************************************************************************************
*/

#if (CLK_CFG_UNIX_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_GetTS_Unix (CLK_TS_SEC  *p_ts_unix_sec)
{
    CLK_TS_SEC   ts_sec;
    CPU_BOOLEAN  valid;


#if 0                                                           /* Validated & init'd in Clk_TS_ToTS_Unix().            */
    if (p_ts_unix_sec == (CLK_TS_SEC *)0) {                     /* ----------------- VALIDATE TS PTR ------------------ */
        return (DEF_FAIL);
    }

   *p_ts_unix_sec = CLK_TS_SEC_NONE;                            /* Init to ts none for err (see Note #3).               */
#endif

                                                                /* -------------------- GET CLK TS -------------------- */
    ts_sec = Clk_GetTS();

                                                                /* -------------- CONV CLK TS TO UNIX TS -------------- */
    valid  = Clk_TS_ToTS_Unix(ts_sec, p_ts_unix_sec);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                          Clk_SetTS_Unix()
*
* Description : Set Clock timestamp from a Unix timestamp.
*
* Argument(s) : ts_unix_sec     Current Unix timestamp to set (in seconds, UTC+00).
*
* Return(s)   : DEF_OK,   if timestamp successfully set.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function & MAY
*               be called by application function(s).
*
* Note(s)     : (1) Internal Clock timestamp SHOULD be set for UTC+00 and should NOT include any local
*                   time zone offset.
*
*               (2) Only years supported by Clock & Unix can be set, thus the timestamp date MUST be :
*
*                   (a) >= CLK_EPOCH_YR_START
*                   (b) <  CLK_UNIX_EPOCH_YR_END
*********************************************************************************************************
*/

#if (CLK_CFG_UNIX_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_SetTS_Unix (CLK_TS_SEC  ts_unix_sec)
{
    CLK_TS_SEC   ts_sec;
    CPU_BOOLEAN  valid;

                                                                /* -------------- CONV UNIX TS TO CLK TS -------------- */
    valid = Clk_TS_UnixToTS(&ts_sec, ts_unix_sec);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

                                                                /* -------------------- SET CLK TS -------------------- */
    Clk_SetTS(ts_sec);

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         Clk_TS_ToTS_Unix()
*
* Description : Convert Clock timestamp to Unix timestamp.
*
* Argument(s) : ts_sec          Timestamp to convert (in seconds, UTC+00).
*
*               p_ts_unix_sec   Pointer to variable that will receive the Unix timestamp :
*
*                                   In seconds UTC+00,  if NO error(s);
*                                   CLK_TS_SEC_NONE,    otherwise.
*
* Return(s)   : DEF_OK,   if timestamp successfully converted.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Clk_GetTS_Unix(),
*               Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) Returned timestamp does NOT include any time zone offset. Thus any local time
*                   zone offset SHOULD be applied before or after calling Clk_TS_ToTS_Unix().
*
*               (2) Only years supported by Clock & Unix can be converted, thus the timestamp date
*                   MUST be :
*
*                   (a) >= CLK_EPOCH_YR_START
*                   (b) <  CLK_UNIX_EPOCH_YR_END
*
*               (3) Pointers to variables that return values MUST be initialized PRIOR to all other
*                   validation or function handling in case of any error(s).
*********************************************************************************************************
*/

#if (CLK_CFG_UNIX_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_TS_ToTS_Unix (CLK_TS_SEC   ts_sec,
                               CLK_TS_SEC  *p_ts_unix_sec)
{
    CLK_TS_SEC  ts_unix_sec;


#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* ----------------- VALIDATE TS PTR ------------------ */
    if (p_ts_unix_sec == (CLK_TS_SEC *)0) {
        return (DEF_FAIL);
    }
#endif

   *p_ts_unix_sec = CLK_TS_SEC_NONE;                            /* Init to ts none for err (see Note #3).               */

    CLK_TRACE_DBG(("\n\rConvert TS to Unix TS:\n\r"
                   "    TS = %u\n\r", (unsigned int)ts_sec));

                                                                /* -------------- CONV CLK TS TO UNIX TS -------------- */
    ts_unix_sec = ts_sec + CLK_UNIX_EPOCH_OFFSET_SEC;
    if (ts_unix_sec < CLK_UNIX_EPOCH_OFFSET_SEC) {              /* Chk for ovf.                                         */
        CLK_TRACE_DBG(("    Unix TS conversion has failed\n\r"));
        return (DEF_FAIL);
    }

   *p_ts_unix_sec = ts_unix_sec;
    CLK_TRACE_DBG(("    Unix TS converted = %u\n\r", (unsigned int)*p_ts_unix_sec));

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                          Clk_TS_UnixToTS()
*
* Description : Convert Unix timestamp to Clock timestamp.
*
* Argument(s) : p_ts_sec        Pointer to variable that will receive the Clock timestamp :
*
*                                   In seconds UTC+00,  if NO error(s);
*                                   CLK_TS_SEC_NONE,    otherwise.
*
*               ts_unix_sec     Unix timestamp value to convert (in seconds, UTC+00).
*
* Return(s)   : DEF_OK,   if timestamp successfully converted.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Clk_SetTS_Unix(),
*               Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) Returned timestamp does NOT include any time zone offset. Thus any local time
*                   zone offset SHOULD be applied before or after calling Clk_TS_UnixToTS().
*
*               (2) Only years supported by Clock & Unix can be converted, thus the timestamp date
*                   MUST be :
*
*                   (a) >= CLK_EPOCH_YR_START
*                   (b) <  CLK_UNIX_EPOCH_YR_END
*
*               (3) Pointers to variables that return values MUST be initialized PRIOR to all other
*                   validation or function handling in case of any error(s).
*********************************************************************************************************
*/

#if (CLK_CFG_UNIX_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_TS_UnixToTS (CLK_TS_SEC  *p_ts_sec,
                              CLK_TS_SEC   ts_unix_sec)
{
#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* ----------------- VALIDATE TS PTR ------------------ */
    if (p_ts_sec == (CLK_TS_SEC *)0) {
        return (DEF_FAIL);
    }
#endif

   *p_ts_sec = CLK_TS_SEC_NONE;                                 /* Init to ts none for err (see Note #3).               */

    CLK_TRACE_DBG(("Convert Unix TS to TS:\n\r"
                   "    Unix TS = %u\n\r", (unsigned int)ts_unix_sec));

    if (ts_unix_sec < CLK_UNIX_EPOCH_OFFSET_SEC) {              /* Chk for ovf.                                         */
        CLK_TRACE_DBG(("TS overflow\n\r"));
        return (DEF_FAIL);
    }

                                                                /* -------------- CONV UNIX TS TO CLK TS -------------- */
   *p_ts_sec = ts_unix_sec - CLK_UNIX_EPOCH_OFFSET_SEC;
    CLK_TRACE_DBG(("    TS converted = %u\n\r", (unsigned int)*p_ts_sec));

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       Clk_TS_UnixToDateTime()
*
* Description : Convert Unix timestamp to a date/time structure.
*
* Argument(s) : ts_unix_sec     Timestamp to convert (in seconds,          UTC+00).
*
*               tz_sec          Time zone offset     (in seconds, +|- from UTC).
*
*               p_date_time     Pointer to variable that will receive the date/time structure.
*
* Return(s)   : DEF_OK,   if date/time structure successfully returned.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function & MAY
*               be called by application function(s).
*
* Note(s)     : (1) (a) Timestamp ('ts_unix_sec') MUST be set for UTC+00 & SHOULD NOT include the time
*                       zone offset ('tz_sec') since Clk_TS_UnixToDateTime() includes the time zone
*                       offset in its date/time calculation. Thus the time zone offset SHOULD NOT be
*                       applied before or after calling Clk_TS_UnixToDateTime().
*
*                   (b) Time zone field of the date/time structure ('p_date_time->TZ_sec') is set to
*                       the value of the time zone argument ('tz_sec').
*
*                   (c) Time zone is based on Coordinated Universal Time (UTC) & has valid values :
*
*                       (1) Between  +|- 12 hours (+|- 43200 seconds)
*                       (2) Multiples of 15 minutes
*********************************************************************************************************
*/

#if (CLK_CFG_UNIX_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_TS_UnixToDateTime (CLK_TS_SEC      ts_unix_sec,
                                    CLK_TZ_SEC      tz_sec,
                                    CLK_DATE_TIME  *p_date_time)
{
    CPU_BOOLEAN  valid;

                                                                /* ------------ CONV UNIX TS TO DATE/TIME ------------- */
    CLK_TRACE_DBG(("\n\rConvert TS Unix to Date/time:\n\r"
                   "    TS to convert= %u\n\n\r",
                   (unsigned int)ts_unix_sec));

    valid = Clk_TS_ToDateTimeHandler(ts_unix_sec,
                                     tz_sec,
                                     p_date_time,
                                     CLK_UNIX_EPOCH_YR_START,
                                     CLK_UNIX_EPOCH_YR_END);
    if (valid != DEF_OK) {
        CLK_TRACE_DBG(("    Date/time conversion has failed\n\r"));
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       Clk_DateTimeToTS_Unix()
*
* Description : Convert a date/time structure to Unix timestamp.
*
* Argument(s) : p_ts_sec        Pointer to variable that will receive the Unix timestamp :
*
*                                   In seconds UTC+00,  if NO error(s);
*                                   CLK_TS_SEC_NONE,    otherwise.
*
*               p_date_time     Pointer to variable that contains date/time structure to convert.
*
* Return(s)   : DEF_OK,   if timestamp successfully returned.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) Date/time structure ('p_date_time') MUST be representable in Unix timestamp.
*                   Thus date to convert MUST be :
*
*                   (a) >= CLK_UNIX_EPOCH_YR_START
*                   (b) <  CLK_UNIX_EPOCH_YR_END
*
*               (2) (a) Date/time ('p_date_time') SHOULD be set to local time with correct  time zone
*                       offset ('p_date_time->TZ_sec'). Clk_DateTimeToTS_Unix() removes the time zone
*                       offset from the date/time to calculate & return a Clock timestamp at UTC+00.
*
*                   (b) Time zone is based on Coordinated Universal Time (UTC) & has valid values :
*
*                       (1) Between  +|- 12 hours (+|- 43200 seconds)
*                       (2) Multiples of 15 minutes
*
*               (3) Pointers to variables that return values MUST be initialized PRIOR to all other
*                   validation or function handling in case of any error(s).
*********************************************************************************************************
*/

#if (CLK_CFG_UNIX_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_DateTimeToTS_Unix (CLK_TS_SEC     *p_ts_unix_sec,
                                    CLK_DATE_TIME  *p_date_time)
{
    CPU_BOOLEAN  valid;


#if (CLK_CFG_ARG_CHK_EN == DEF_ENABLED)                         /* ----------------- VALIDATE TS PTR ------------------ */
    if (p_ts_unix_sec == (CLK_TS_SEC *)0) {
        return (DEF_FAIL);
    }
#endif

   *p_ts_unix_sec = CLK_TS_SEC_NONE;                            /* Init to ts none for err (see Note #3).               */


#if 0                                                           /* Validated in Clk_IsUnixDateTimeValid().              */
    if (p_date_time == (CLK_DATE_TIME *)0) {                    /* -------------- VALIDATE DATE/TIME PTR -------------- */
        return (DEF_FAIL);
    }
#endif

                                                                /* ---------------- VALIDATE DATE/TIME ---------------- */
    valid = Clk_IsUnixDateTimeValid(p_date_time);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

                                                                /* ------------ CONV DATE/TIME TO UNIX TS ------------- */
    CLK_TRACE_DBG(("Convert Date/time to TS:\n\r"));
    valid = Clk_DateTimeToTS_Handler(p_ts_unix_sec,
                                     p_date_time,
                                     CLK_UNIX_EPOCH_YR_START);
    if (valid != DEF_OK) {
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       Clk_UnixDateTimeMake()
*
* Description : Build a valid Unix epoch date/time structure.
*
* Argument(s) : p_date_time     Pointer to variable that will receive the date/time structure.
*
*               yr              Year    value [CLK_UNIX_EPOCH_YR_START to CLK_UNIX_EPOCH_YR_END) (see Note #1).
*
*               month           Month   value [          CLK_MONTH_JAN to         CLK_MONTH_DEC].
*
*               day             Day     value [                      1 to                    31].
*
*               hr              Hours   value [                      0 to                    23].
*
*               min             Minutes value [                      0 to                    59].
*
*               sec             Seconds value [                      0 to                    60] (see Note #3).
*
*               tz_sec          Time zone offset (in seconds, +|- from UTC) [-43200 to 43200].
*
* Return(s)   : DEF_OK,   if date/time structure successfully returned.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) Date/time structure ('p_date_time') MUST be representable in Unix timestamp.
*                   Thus date to convert MUST be :
*
*                   (a) >= CLK_UNIX_EPOCH_YR_START
*                   (b) <  CLK_UNIX_EPOCH_YR_END
*
*               (2) Day of week ('p_date_time->DayOfWk') and Day of year ('p_date_time->DayOfYr')
*                   are internally calculated and set in the date/time structure if date is valid.
*
*               (3) Seconds value of 60 is valid to be compatible with leap second adjustment and
*                   the atomic clock time structure.
*
*               (4) Time zone is based ('tz_sec') on Coordinated Universal Time (UTC) & has valid
*                   values :
*
*                   (a) Between  +|- 12 hours (+|- 43200 seconds)
*                   (b) Multiples of 15 minutes
*********************************************************************************************************
*/

#if (CLK_CFG_UNIX_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_UnixDateTimeMake (CLK_DATE_TIME  *p_date_time,
                                   CLK_YR          yr,
                                   CLK_MONTH       month,
                                   CLK_DAY         day,
                                   CLK_HR          hr,
                                   CLK_MIN         min,
                                   CLK_SEC         sec,
                                   CLK_TZ_SEC      tz_sec)
{
    CPU_BOOLEAN  valid;

                                                                /* ---------- VALIDATE & CONV UNIX DATE/TIME ---------- */
    valid = Clk_DateTimeMakeHandler(p_date_time,
                                    yr,
                                    month,
                                    day,
                                    hr,
                                    min,
                                    sec,
                                    tz_sec,
                                    CLK_UNIX_EPOCH_YR_START,
                                    CLK_UNIX_EPOCH_YR_END);
    if (valid != DEF_YES) {
        return  (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                      Clk_IsUnixDateTimeValid()
*
* Description : Determine if date/time structure is valid in Unix epoch.
*
* Argument(s) : p_date_time     Pointer to variable that contains the date/time structure to validate.
*
* Return(s)   : DEF_YES, if date/time structure is valid.
*
*               DEF_NO,  otherwise.
*
* Caller(s)   : Clk_DateTimeToTS_Unix(),
*               Application.
*
*               This function is a Clock module application programming interface (API) function
*               & MAY be called by application function(s).
*
* Note(s)     : (1) Date/time structure ('p_date_time') MUST be representable in Unix epoch. Thus
*                   date to validate MUST be :
*
*                   (a) >= CLK_UNIX_EPOCH_YR_START
*                   (b) <  CLK_UNIX_EPOCH_YR_END
*
*               (2) Time zone is based ('p_date_time->TZ_sec') on Coordinated Universal Time (UTC)
*                   & has valid values :
*
*                   (a) Between  +|- 12 hours (+|- 43200 seconds)
*                   (b) Multiples of 15 minutes
*********************************************************************************************************
*/

#if (CLK_CFG_UNIX_EN == DEF_ENABLED)
CPU_BOOLEAN  Clk_IsUnixDateTimeValid (CLK_DATE_TIME  *p_date_time)
{
    CPU_BOOLEAN  valid;

                                                                /* ------------- VALIDATE UNIX DATE/TIME -------------- */
    CLK_TRACE_DBG(("Validate Unix date/time: "));

    valid = Clk_IsDateTimeValidHandler(p_date_time,
                                       CLK_UNIX_EPOCH_YR_START,
                                       CLK_UNIX_EPOCH_YR_END);
    if (valid != DEF_YES) {
        CLK_TRACE_DBG(("Fail\n\r"));
        return  (DEF_FAIL);
    }

    CLK_TRACE_DBG(("Ok\n\r"));

    return (DEF_YES);
}
#endif



