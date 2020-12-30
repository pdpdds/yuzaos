#pragma once
#include <stdint.h>
#include <stdio.h>
#ifdef EMSCRIPTEN
#include <emscripten/bind.h>
#endif

#define	_VERSION_			14	// Engine Version

#define DF_PANEL_WIDTH      9	// 장기판 크기 설정
#define DF_PANEL_HEIGHT     10

#define RANK_TOP     3
#define RANK_BOTTOM 12
#define FILE_LEFT    3
#define FILE_RIGHT  11

#define ADD     false
#define DEL     true

#define DEFAULT_HASH_LEVEL  16
#define HASH_ALPHA  1       // Alpha-cutoff
#define HASH_BETA   2       // Beta-cutoff
#define HASH_PV     3

#define DEFAULT_CUTOFF_DEPTH	3

// 각 기물의 평가치
#define DF_SCORE_JANG       500000
#define DF_SCORE_PO         25000
#define DF_SCORE_MA         15000
#define DF_SCORE_SA         10000
#define DF_SCORE_SANG       9000
#define DF_SCORE_JOL        5000

extern int SCORE_CHO_CHA;
extern int SCORE_HAN_CHA;


#define DF_INFINITY         9999999

#define PS_NOTHING			0
#define PS_CHO_JANG    		1    // J
#define PS_CHO_SA      		2    // A
#define PS_CHO_PO    		3    // P
#define PS_CHO_JOL    		4    // B
#define PS_CHO_MA    		5    // S
#define PS_CHO_SANG      	6    // M
#define PS_CHO_CHA     		7    // C
#define PS_BOUNDARY			8	 // Side boundary
#define PS_HAN_JANG    		9    // j
#define PS_HAN_SA      		10   // a
#define PS_HAN_PO    		11   // p
#define PS_HAN_JOL      	12   // b
#define PS_HAN_MA    		13   // s
#define PS_HAN_SANG    		14   // m
#define PS_HAN_CHA     		15   // c

#define SD_NOTHING			 0
#define SD_CHO				 1
#define SD_HAN				 2

#define PHASE_OPENING		 0
#define PHASE_MIDGAME		 1
#define PHASE_ENDGAME		 2

#define MAKE_SQ(x, y) ((x+FILE_LEFT) + ((y+RANK_TOP) << 4))
#define GET_X(sq)     ((sq & 0x0f) - FILE_LEFT)
#define GET_Y(sq)	  ((sq >> 4) - RANK_TOP)
#define SRC(mv)		  (mv & 255)
#define DST(mv)		  (mv >> 8)
#define MAKE_MV(sqSrc, sqDst) (sqSrc + (sqDst << 8))

typedef uint32_t MV;
typedef uint32_t PIECE;
typedef uint32_t SIDE;

// Evaluation Advantages
static bool g_bPoBeforeJang = true;		// JANG은 PO 뒤에 있도록
static bool g_bGatherJol = true;	// JOL은 모이도록
static bool g_bAggressivePO = false;	// PO는 공격성을 가지도록. PO는 원격지원용이므로 false 처리함
static bool g_bProtectGung = true;		// PO는 궁을 보호하도록
static bool g_bAggressiveCHA = true;		// CHA는 공격성을 가지도록
static bool g_bAggressiveJOL = true;		// JOL이 공격성을 가지도록
static bool g_bAggressiveSANG = true;	// SANG이 공격성을 가지도록
static bool g_bAggressiveMA = true;		// MA가 공격성을 가지도록

typedef struct tagSHistory {
	int         nFromX;
	int         nFromY;
	int         nToX;
	int         nToY;
	PIECE      enEatenPiece;
	struct tagSHistory* pPrev;
} SHistory;