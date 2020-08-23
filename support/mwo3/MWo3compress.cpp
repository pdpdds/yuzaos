/*
	SRWZ Compresser

	Author:		kid, K_I_D@126.com
	Function:	MWo3 compress 

	History:
		2008.10.4	first version, MWo3compress_org.cpp
		2008.10.17	参照gzip思想优化
		2008.10.18	修正window size
*/

#include "MWo3.h"
#include <vector>
#include <memory.h>
#include <stdio.h>

using namespace std;

UCHAR codebuf[0x3ffff];
int codeptr = 0;

void inline writesize(ULONG size, UCHAR*& dst)
{
	int srcsize = size;
	size <<= 1;
	vector<UCHAR> temp;
	while (size >> 1)
	{
		UCHAR t = (size & 0xfe) ;
		temp.push_back(t);
		size >>= 7;
	}
	while (!temp.empty())
	{
		*dst++ = temp.back();
		temp.pop_back();
	}
	*(dst - 1) |= 1;

	int v = 1;
	while (srcsize >>= 1)
	{++v;}
	*dst++ = ((v - 8) << 2) | 3;		//window size
			
	*dst++ = 1;			//unknow
}
void inline writedirect(unsigned int& direct, UCHAR*& dst, UCHAR*& flag)
{
	if (direct > 0xf)
	{
		direct <<= 1;
		vector<UCHAR> temp;
		while (direct >> 1)
		{
			//*dst++ = (direct & 0xfe) | ((direct >> 8) ? 0 : 1);
			UCHAR t = (direct & 0xfe) ;
			temp.push_back(t);
			direct >>= 7;
		}
		while (!temp.empty())
		{
			*dst++ = temp.back();
			temp.pop_back();
		}
		*(dst - 1) |= 1;
	}
	else
	{
		*flag |= direct;
	}
	direct = 0;
}
void inline writehighnum(int num, UCHAR*& dst, UCHAR*& flag)
{
	if (num > 0xf)
	{
		num <<= 1;
		vector<UCHAR> temp;
		while (num >> 1)
		{
			//*dst++ = (num & 0xfe) | ((num >> 8) ? 0 : 1);
			UCHAR t = (num & 0xfe) ;
			temp.push_back(t);
			num >>= 7;
		}
		while (!temp.empty())
		{
			*dst++ = temp.back();
			temp.pop_back();
		}
		*(dst - 1) |= 1;
	}
	else
	{
		*flag |= num << 4;
	}
}
void inline writewindownum(int num, int pos)
{
	if (num < 0 || pos < 0 /*|| pos >= 0x3fff*/)
	{
		printf("error: num=%d,pos=%d\n",num, pos);
		for (;;);
	}
	int flag = codeptr++;
	codebuf[flag] = 0;

	if (pos > 0x7)
	{
		pos <<= 1;
		vector<UCHAR> temp;
		while (pos > 0xe)		//1110
		{
			//codebuf[codeptr++] = (pos & 0xfe) | ((pos >> 8) > 0x7 ? 0 : 1);
			UCHAR t = (pos & 0xfe) ;
			temp.push_back(t);
			pos >>= 7;
		}
		pos >>= 1;
		while (!temp.empty())
		{
			codebuf[codeptr++] = temp.back();
			temp.pop_back();
		}
		codebuf[codeptr - 1] |= 1;
	}
	else
	{
		codebuf[flag] |= 1;
	}
	if (pos)
	{
		codebuf[flag] |= pos << 1;
	}

	if (num > 0xf)
	{
		num <<= 1;
		vector<UCHAR> temp;
		while (num >> 1)
		{
			UCHAR t = (num & 0xfe) ;
			//codebuf[codeptr++] = (num & 0xfe) | ((num >> 8) ? 0 : 1);
			num >>= 7;
			temp.push_back(t);
		}
		while (!temp.empty())
		{
			codebuf[codeptr++] = temp.back();
			temp.pop_back();
		}
		codebuf[codeptr - 1] |= 1;
	}
	else
	{
		codebuf[flag] |= num << 4;
	}
}

#define HASH_BITS  16
#define WSIZE 0x2000000
#define H_SHIFT  ((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)
/* Number of bits by which ins_h and del_h must be shifted at each
 * input step. It must be such that after MIN_MATCH steps, the oldest
 * byte no longer takes part in the hash key, that is:
 *   H_SHIFT * MIN_MATCH >= HASH_BITS
 */
#define HASH_SIZE (ULONG)(1<<HASH_BITS)
#define HASH_MASK (HASH_SIZE-1)
#define WMASK     (WSIZE-1)

#define MIN_MATCH 2
#define MAX_MATCH 0xffffff//1537
#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
#define NIL 0
#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)
#define MAX_DIRECT_LEN 0x3fff


/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.
 */
#define UPDATE_HASH(h,c) (h = (((h)<<8) | (c)) & HASH_MASK)

#define INSERT_STRING(s, match_head) \
   (UPDATE_HASH(ins_h, window[(s) + MIN_MATCH-1]), \
    preva[(s) & WMASK] = match_head = head[ins_h], \
    head[ins_h] = (s))


const ULONG window_size = 2*WSIZE;
UCHAR window[window_size]; 
ULONG preva[WSIZE]; 
ULONG head[1L<<HASH_BITS];
ULONG max_lazy_match;
ULONG good_match;
ULONG nice_match;
ULONG max_chain_length;
ULONG strstart;      /* start of string to insert */
ULONG match_start;   /* start of matching string */
ULONG lookahead;     /* number of valid bytes ahead in window */
ULONG ins_h;  /* hash index of string to be inserted */
ULONG prev_length;
ULONG endsize = 0;

typedef struct config {
   ULONG good_length; /* reduce lazy search above this match length */
   ULONG max_lazy;    /* do not perform lazy search above this match length */
   ULONG nice_length; /* quit search above this match length */
   ULONG max_chain;
} config;

config configuration_table[10] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0},  /* store only */
/* 1 */ {4,    4,  8,    4},  /* maximum speed, no lazy matches */
/* 2 */ {4,    5, 16,    8},
/* 3 */ {4,    6, 32,   32},
/* 4 */ {4,    4, 16,   16},  /* lazy matches */
/* 5 */ {8,   16, 32,   32},
/* 6 */ {8,   16, 256, 256},
/* 7 */ {8,   32, 1024, 1024},
/* 8 */ {32, 128, 4096, 4096},
/* 9 */ {32, 258, 0xffffff, 0xffff}}; /* maximum compression */

const UCHAR* srcdata = NULL;
ULONG srcsize = 0;

void fill_window()
{
    register unsigned n, m;
    unsigned more = (unsigned)(window_size - lookahead - strstart);
    /* Amount of free space at the end of the window. */

    /* If the window is almost full and there is insufficient lookahead,
     * move the upper half to the lower one to make room in the upper half.
     */
    if (more == (unsigned)EOF) {
        /* Very unlikely, but possible on 16 bit machine if strstart == 0
         * and lookahead == 1 (input done one byte at time)
         */
        more--;
    } else if (strstart >= WSIZE+MAX_DIST) {
        /* By the IN assertion, the window is not empty so we can't confuse
         * more == 0 with more == 64K on a 16 bit machine.
         */

        memcpy((char*)window, (char*)window+WSIZE, (unsigned)WSIZE);
        match_start -= WSIZE;
        strstart    -= WSIZE; /* we now have strstart >= MAX_DIST: */


        for (n = 0; n < HASH_SIZE; n++) {
            m = head[n];
            head[n] = (unsigned short)(m >= WSIZE ? m-WSIZE : NIL);
        }
        for (n = 0; n < WSIZE; n++) {
            m = preva[n];
			preva[n] = (unsigned short)(m >= WSIZE ? m-WSIZE : NIL);
            /* If n is not on any hash chain, prev[n] is garbage but
             * its value will never be used.
             */
        }
        more += WSIZE;
    }
    /* At this point, more >= 2 */
    if (srcsize > 0) {
		endsize = n = more > srcsize ? srcsize : more;
		memcpy((char*)window+strstart+lookahead, srcdata, n);
        lookahead += n;
		srcdata += n;
		srcsize -= n;        
    }
}

void init (int pack_level) /* 0: store, 1: best speed, 9: best compression */
{
    register unsigned j;

    /* Initialize the hash table. */
    memset((char*)head, 0, HASH_SIZE*sizeof(*head));

	/* prev will be initialized on the fly */

    /* Set the default configuration parameters:
     */
    max_lazy_match   = configuration_table[pack_level].max_lazy;
    good_match       = configuration_table[pack_level].good_length;
    nice_match       = configuration_table[pack_level].nice_length;
    max_chain_length = configuration_table[pack_level].max_chain;

    strstart = 0;

	endsize = lookahead = 2*WSIZE > srcsize ? srcsize : 2*WSIZE;
	memcpy((char*)window, srcdata, lookahead);
	srcdata += lookahead;
	srcsize -= lookahead;

    /* Make sure that we always have enough lookahead. This is important
     * if input comes from a device such as a tty.
     */
    while (lookahead < MIN_LOOKAHEAD && srcsize > 0) 
		fill_window();

    ins_h = 0;
    for (j=0; j<MIN_MATCH-1; j++) 
		UPDATE_HASH(ins_h, window[j]);
    /* If lookahead < MIN_MATCH, ins_h is garbage, but this is
     * not important since only literal bytes will be emitted.
     */
}

int longest_match(unsigned cur_match)     /* current match */
{
    unsigned chain_length = max_chain_length;   /* max hash chain length */
    register UCHAR *scan = window + strstart + 2;     /* current string */
    register UCHAR *match;                        /* matched string */
    register unsigned int len;                           /* length of current match */
    unsigned int best_len = prev_length;                 /* best match length so far */
    unsigned limit = strstart > (unsigned)MAX_DIST ? strstart - (unsigned)MAX_DIST : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
	int best_flag = 0xff;

	register UCHAR *strend = window + ((strstart + MAX_MATCH) > endsize ? endsize : (strstart + MAX_MATCH));
    register UCHAR scan_end1  = scan[best_len-1];
    register UCHAR scan_end   = scan[best_len];

    /* Do not waste too much time if we already have a good match: */
    if (prev_length >= good_match) {
        chain_length >>= 2;
    }

    do {
		len = 1;
        match = window + cur_match + 2;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2:
         */

        do {
			++len;
        } while (*scan++ == *match++ && scan <= strend);

        //len = MAX_MATCH - (int)(strend - scan) - 1;
        scan -= len - 1;

        if (len > best_len) {
            match_start = cur_match;
            best_len = len;
            if (len >= nice_match) break;
        }
    } while ((cur_match = preva[cur_match & WMASK]) > limit
	     && --chain_length != 0);

    return best_len;
}


void compress(const UCHAR* src, ULONG srcSize, UCHAR* dst, ULONG& dstsize, int level)
{
	srcdata = src;
	srcsize = srcSize;
	const UCHAR* dstbegin = dst;
	writesize(srcSize, dst);
	init(level);

	unsigned  hash_head;          /* head of hash chain */
    unsigned  prev_match;         /* previous match */
    int match_available = 0;	 /* set if previous match exists */
    register unsigned match_length = MIN_MATCH-1; /* length of best match */

	unsigned int direct = 0;
	unsigned int matchnum = 0;
	UCHAR* flag = dst++;
	*flag = 0;

    /* Process the input block. */
    while (lookahead != 0) {
        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        INSERT_STRING(strstart, hash_head);

        /* Find the longest match, discarding those <= prev_length.
         */
        prev_length = match_length, prev_match = match_start;
        match_length = MIN_MATCH-1;

        if (hash_head != NIL && prev_length < max_lazy_match &&
            strstart - hash_head <= MAX_DIST) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */

            match_length = longest_match (hash_head);

            /* longest_match() sets match_start */
            if (match_length > lookahead) match_length = lookahead;

            /* Ignore a length 3 match if it is too distant: */
			if (match_length == 2 && strstart-match_start > 7)
                match_length--;
			else if (match_length == 3 && strstart-match_start > 0x3ff)
                match_length=1;
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        if (prev_length >= MIN_MATCH && match_length <= prev_length) {
			matchnum++;
			writewindownum(prev_length - 1, strstart-2-prev_match);

			/* Insert in hash table all strings up to the end of the match.
             * strstart-1 and strstart are already inserted.
             */
            lookahead -= prev_length-1;
            prev_length -= 2;
			while (prev_length != 0)
			{
                strstart++;
                INSERT_STRING(strstart, hash_head);
                /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                 * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
                 * these bytes are garbage, but it does not matter since the
                 * next lookahead bytes will always be emitted as literals.
                 */
				--prev_length;
            };
            match_available = 0;
            match_length = MIN_MATCH-1;
            strstart++;
        } else if (match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
			if (matchnum)
			{
				writedirect(direct,dst,flag);
				writehighnum(matchnum, dst, flag);
				for (int i = 0; i < codeptr; i++)
					*dst++ = codebuf[i];

				codeptr = 0;
				matchnum = 0;
				direct = 0;
				codeptr = 0;
				flag = dst++;
				*flag = 0;
			}

			direct++;
			codebuf[codeptr++] = window[strstart-1];
			if (direct >= MAX_DIRECT_LEN)
			{
				printf("Too many direct num.\n");
				for (;;);
			}

            strstart++;
            lookahead--;
        } else {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            match_available = 1;
            strstart++;
            lookahead--;
        }

        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        while (lookahead < MIN_LOOKAHEAD && srcsize > 0) 
			fill_window();
    }

	if (matchnum || direct)
	{
		writedirect(direct,dst,flag);
		writehighnum(matchnum, dst, flag);
		for (int i = 0; i < codeptr; i++)
			*dst++ = codebuf[i];

		codeptr = 0;
		matchnum = 0;
		direct = 0;
		codeptr = 0;
		/*flag = dst++;
		*flag = 0;*/
	}
	dstsize = dst - dstbegin;
}
