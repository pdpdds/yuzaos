#include "chobits_api.h"
#include <memory.h>
#include <key_def.h>
/* 
 * codes by Yeori
 *
 * title : Hellow World!
 * e-mail : alphamcu@hanmail.net
 * www : www.zap.pe.kr (english : zap.pe.kr/eng)
 */

/*
 * DEFINITIONS
 */
#define BLOCK_X_NUM				10
#define BLOCK_Y_NUM				14

#define BOARD_LEFT				5
#define BOARD_RIGHT				205
#define BOARD_TOP				5
#define BOARD_BOTTOM			195

#define DIRECTION_UP			0x00
#define DIRECTION_DOWN			0x01
#define DIRECTION_LEFT			0x02
#define DIRECTION_RIGHT			0x03

#define BAR_CX					20
#define BAR_CY					5
#define BAR_SCREEN_Y			190
#define BAR_MIN_POS				BOARD_LEFT
#define BAR_MAX_POS				(BOARD_RIGHT-BAR_CX)
#define BAR_INIT_POS			((BAR_MAX_POS+BAR_MIN_POS)/2)
#define BAR_MOVE_GAP			5

#define LEFT_KEY				'j'
#define RIGHT_KEY				'l'
#define RETRY_KEY				'r'
#define QUIT_KEY				'q'

typedef struct _RECT {
	WORD	left, top, right, bottom;
} RECT, *PRECT;

typedef struct _POINT {
	WORD	x, y;
} POINT, *PPOINT;

/*
 * FUNCTIONS
 */
static void start_new_stage(void);
static void print_stage(void);
static void print_score(void);
static void print_number(WORD x, WORD y, BYTE number);
static BOOL get_key(BYTE *pKey);

static void move_bar(BYTE direction);
static BOOL move_ball(void);

static BOOL point_in_rect(POINT *point, RECT *rect);
static BOOL hit_block(void);
static BOOL hit_bar(void);

/*
 * VARIABLES
 */
static HANDLE m_hBitmap;
static WORD m_Stage;
static WORD m_Score;

static POINT m_BarPos;
static POINT m_BallPos;
static short m_BallXMoveGap, m_BallYMoveGap;

static BYTE m_BlockMap[BLOCK_Y_NUM][BLOCK_X_NUM];

int main(int argc, char** argv)
{
	BYTE key_data;

	/* load bitmap */
	API_ClearScreen();
	API_PrintText("Image file loading... wait a second. \r\n");

#define BITMAP_FILENAME		"sexy.bmp"
	m_hBitmap = API_LoadBitmap(BITMAP_FILENAME);
	if(m_hBitmap == NULL) {
		API_PrintText("Loading error. \r\n");
		return -1;
	}	

	/* game system */
	start_new_stage();
	while(1) {
		/* get key */
		if(get_key(&key_data)) {
			switch(key_data) {
				case LEFT_KEY:
					move_bar(DIRECTION_LEFT);
					break;
				case RIGHT_KEY:
					move_bar(DIRECTION_RIGHT);
					break;
				case QUIT_KEY:
					goto $exit;
					break;
			}
		}

		if(!move_ball()) {
#define RETRY_SIGN_START_X			55
#define RETRY_SIGN_START_Y			150
#define RETRY_SIGN_X_POS_IN_IMG		200
#define RETRY_SIGN_Y_POS_IN_IMG		200
#define RETRY_SIGN_CX				100
#define RETRY_SIGN_CY				40
			while(1) {
				if(get_key(&key_data)) {
					if(key_data == RETRY_KEY) {
						start_new_stage();
						break;
					} else {
						goto $exit;
					}
				}
				API_BitBlt(m_hBitmap, RETRY_SIGN_START_X, RETRY_SIGN_START_Y, RETRY_SIGN_X_POS_IN_IMG, RETRY_SIGN_Y_POS_IN_IMG,
					RETRY_SIGN_CX, RETRY_SIGN_CY, NULL);
				API_Delay(500);
				API_BitBlt(m_hBitmap, RETRY_SIGN_START_X, RETRY_SIGN_START_Y, RETRY_SIGN_START_X, RETRY_SIGN_START_Y,
					RETRY_SIGN_CX, RETRY_SIGN_CY, NULL);
				API_Delay(500);
			}
		}
		API_Delay(25);
	}

$exit:
	/* end */
	API_CloseBitmapHandle(m_hBitmap);
	API_ClearScreen();
	
	return 0;
}

#define IMAGE_WIDTH				320
#define IMAGE_HEIGHT			200

#define BLOCK_START_POS_X		5
#define BLOCK_START_POS_Y		5
#define BLOCK_CX				20
#define BLOCK_CY				10
#define BLOCK_X_POS_IN_IMG		300
#define BLOCK_Y_POS_IN_IMG		200
static void start_new_stage(void)
{
	WORD x, y;
	BYTE key_data;

	/* reset stage & score */
	m_Stage = 1;
	m_Score = 0;
	m_BarPos.x = BAR_INIT_POS-BAR_MOVE_GAP;
	m_BarPos.y = BAR_SCREEN_Y;
	m_BallPos.x = BAR_INIT_POS;
	m_BallPos.y = BAR_SCREEN_Y-10;
	m_BallXMoveGap = 1;
	m_BallYMoveGap = -1;

	memset(m_BlockMap, 1, sizeof(m_BlockMap));

	/* draw board */
	API_BitBlt(m_hBitmap, 0, 0, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, NULL);

	/* put block */
	for(y=0; y<BLOCK_Y_NUM; y++) {
		for(x=0; x<BLOCK_X_NUM; x++) {
			if(m_BlockMap[y][x] == 0)
				continue;
			API_BitBlt(m_hBitmap, BLOCK_START_POS_X + x*BLOCK_CX, BLOCK_START_POS_Y + y*BLOCK_CY,
				BLOCK_X_POS_IN_IMG, BLOCK_Y_POS_IN_IMG, BLOCK_CX, BLOCK_CY, NULL);
		}
	}

	/* print stage and score */
	print_stage();
	print_score();
	
	while(1) {
#define NEW_SIGN_START_X			5
#define NEW_SIGN_START_Y			160
#define NEW_SIGN_START_X_IN_IMG		0
#define NEW_SIGN_START_Y_IN_IMG		200
#define NEW_SIGN_CX					200
#define NEW_SIGN_CY					20
		if(get_key(&key_data)) break;
		API_BitBlt(m_hBitmap, NEW_SIGN_START_X, NEW_SIGN_START_Y, NEW_SIGN_START_X_IN_IMG, NEW_SIGN_START_Y_IN_IMG,
			NEW_SIGN_CX, NEW_SIGN_CY, NULL);
		API_Delay(500);
		if(get_key(&key_data)) break;
		API_BitBlt(m_hBitmap, NEW_SIGN_START_X, NEW_SIGN_START_Y, NEW_SIGN_START_X, NEW_SIGN_START_Y,
			NEW_SIGN_CX, NEW_SIGN_CY, NULL);
		API_Delay(500);
	}
	API_BitBlt(m_hBitmap, NEW_SIGN_START_X, NEW_SIGN_START_Y, NEW_SIGN_START_X, NEW_SIGN_START_Y,
		NEW_SIGN_CX, NEW_SIGN_CY, NULL);

	/* draw initial bar */
	move_bar(DIRECTION_RIGHT);
}

#define NUMBER_CX				20
#define NUMBER_CY				20
static void print_stage(void)
{
	WORD stage;
	BYTE n1000, n100, n10, n1;

	stage	= m_Stage;
	stage	%= 10000;
	n1000	= (BYTE)(stage/1000);
	stage	%= 1000;
	n100	= (BYTE)(stage/100);
	stage	%= 100;
	n10		= (BYTE)(stage/10);
	stage	%= 10;
	n1		= (BYTE)stage;

#define STAGE_START_X			223
#define STAGE_START_Y			32
	print_number(STAGE_START_X + (NUMBER_CX*0), STAGE_START_Y, n1000);
	print_number(STAGE_START_X + (NUMBER_CX*1), STAGE_START_Y, n100);
	print_number(STAGE_START_X + (NUMBER_CX*2), STAGE_START_Y, n10);
	print_number(STAGE_START_X + (NUMBER_CX*3), STAGE_START_Y, n1);
}

static void print_score(void)
{
	WORD score;
	BYTE n1000, n100, n10, n1;

	score	= m_Score;
	score	%= 10000;
	n1000	= (BYTE)(score/1000);
	score	%= 1000;
	n100	= (BYTE)(score/100);
	score	%= 100;
	n10		= (BYTE)(score/10);
	score	%= 10;
	n1		= (BYTE)score;

#define SCORE_START_X			223
#define SCORE_START_Y			77
	print_number(SCORE_START_X + (NUMBER_CX*0), SCORE_START_Y, n1000);
	print_number(SCORE_START_X + (NUMBER_CX*1), SCORE_START_Y, n100);
	print_number(SCORE_START_X + (NUMBER_CX*2), SCORE_START_Y, n10);
	print_number(SCORE_START_X + (NUMBER_CX*3), SCORE_START_Y, n1);
}

#define NUMBER_START_X_POS		0
#define NUMBER_START_Y_POS		220
static void print_number(WORD x, WORD y, BYTE number)
{
	API_BitBlt(m_hBitmap, x, y, NUMBER_START_X_POS + (NUMBER_CX*number), NUMBER_START_Y_POS, NUMBER_CX, NUMBER_CY, NULL);
}

#define BAR_START_X_IN_IMG		300
#define BAR_START_Y_IN_IMG		210
static void move_bar(BYTE direction)
{
	API_BitBlt(m_hBitmap, m_BarPos.x, m_BarPos.y, m_BarPos.x, m_BarPos.y, BAR_CX, BAR_CY, NULL);

	if(direction == DIRECTION_LEFT)
		m_BarPos.x -= BAR_MOVE_GAP;
	else /* right */
		m_BarPos.x += BAR_MOVE_GAP;

	if(m_BarPos.x < BAR_MIN_POS)	m_BarPos.x = BAR_MIN_POS;
	if(m_BarPos.x > BAR_MAX_POS)	m_BarPos.x = BAR_MAX_POS;

	API_BitBlt(m_hBitmap, m_BarPos.x, m_BarPos.y, BAR_START_X_IN_IMG, BAR_START_Y_IN_IMG, BAR_CX, BAR_CY, NULL);
}

#define BALL_START_X_IN_IMG		300
#define BALL_START_Y_IN_IMG		215
#define BALL_CX					5
#define BALL_CY					5
static BOOL move_ball(void)
{
	API_BitBlt(m_hBitmap, m_BallPos.x, m_BallPos.y, m_BallPos.x, m_BallPos.y, BALL_CX, BALL_CY, NULL);
$retry:
	m_BallPos.x = (WORD)((short)m_BallPos.x + (short)m_BallXMoveGap);
	m_BallPos.y = (WORD)((short)m_BallPos.y + (short)m_BallYMoveGap);

	if(m_BallPos.x > BOARD_RIGHT-BALL_CX || m_BallPos.x < BOARD_LEFT) {
		m_BallXMoveGap = -m_BallXMoveGap;
		m_BallPos.y += -m_BallYMoveGap;
		goto $retry;
	}
	if(m_BallPos.y > BOARD_BOTTOM-BALL_CY || m_BallPos.y < BOARD_TOP) {
		m_BallYMoveGap = -m_BallYMoveGap;
		m_BallPos.x += -m_BallXMoveGap;
		goto $retry;
	}
	API_BitBlt(m_hBitmap, m_BallPos.x, m_BallPos.y, BALL_START_X_IN_IMG, BALL_START_Y_IN_IMG, BALL_CX, BALL_CY, NULL);

	/* hit block? */
	if(hit_block() || hit_bar()) {
		m_BallYMoveGap = -m_BallYMoveGap;
	}

	/* dead? */
	if(m_BallPos.y+BALL_CY > BOARD_BOTTOM-5)
		return FALSE;

	return TRUE;
}

static BOOL hit_block(void)
{
	int block_x, block_y;
	RECT block;
	POINT ball;

	block.left		= BLOCK_START_POS_X;
	block.top		= BLOCK_START_POS_Y;
	block.right		= block.left + BLOCK_CX*BLOCK_X_NUM;
	block.bottom	= block.top + BLOCK_CY*BLOCK_Y_NUM;
	if(!point_in_rect(&m_BallPos, &block))
		return FALSE;

	for(block_y=0; block_y<BLOCK_Y_NUM; block_y++) {
		for(block_x=0; block_x<BLOCK_X_NUM; block_x++) {
			if(m_BlockMap[block_y][block_x] == 0)
				continue;

			block.left		= BLOCK_START_POS_X + BLOCK_CX*block_x;
			block.top		= BLOCK_START_POS_Y + BLOCK_CY*block_y;
			block.right		= block.left + BLOCK_CX;
			block.bottom	= block.top + BLOCK_CY;

			ball.x	= m_BallPos.x;
			ball.y	= m_BallPos.y;
			if(point_in_rect(&ball, &block))
				goto $found;

			ball.x	+= BALL_CX;
			if(point_in_rect(&ball, &block))
				goto $found;

			ball.y	+= BALL_CY;
			if(point_in_rect(&ball, &block))
				goto $found;

			ball.x	-= BALL_CX;
			if(point_in_rect(&ball, &block))
				goto $found;
		}
	}

	return FALSE;

$found:
	m_BlockMap[block_y][block_x] = 0;
	API_BitBlt(m_hBitmap, BLOCK_START_POS_X + block_x*BLOCK_CX, BLOCK_START_POS_Y + block_y*BLOCK_CY,
				BLOCK_START_POS_X + block_x*BLOCK_CX, BLOCK_START_POS_Y + block_y*BLOCK_CY, BLOCK_CX, BLOCK_CY, NULL);
	m_Score += 10;
	print_score();
	return TRUE;
}

static BOOL hit_bar(void)
{
	RECT bar;
	POINT ball;

	ball.x = m_BallPos.x;
	ball.y = m_BallPos.y + BALL_CY;

	bar.left		= m_BarPos.x;
	bar.top			= m_BarPos.y;
	bar.right		= bar.left + 5;
	bar.bottom		= bar.top + BAR_CY;
	if(point_in_rect(&ball, &bar)) {
		m_BallXMoveGap = -2;
		goto $found;
	}

	bar.left		= m_BarPos.x + BAR_CX - 5;
	bar.top			= m_BarPos.y;
	bar.right		= bar.left + 5;
	bar.bottom		= bar.top + BAR_CY;
	if(point_in_rect(&ball, &bar)) {
		m_BallXMoveGap = 2;
		goto $found;
	}

	bar.left		= m_BarPos.x;
	bar.top			= m_BarPos.y;
	bar.right		= bar.left + BAR_CX;
	bar.bottom		= bar.top + BAR_CY;
	if(point_in_rect(&ball, &bar)) {
		m_BallXMoveGap = (m_BallXMoveGap>0 ? 1 : -1);
		goto $found;
	}

	return FALSE;

$found:
	API_BitBlt(m_hBitmap, m_BarPos.x, m_BarPos.y, BAR_START_X_IN_IMG, BAR_START_Y_IN_IMG, BAR_CX, BAR_CY, NULL);
	return TRUE;

}

static BOOL point_in_rect(POINT *point, RECT *rect)
{
	if(point->x>=rect->left && point->x<=rect->right
		&& point->y>=rect->top && point->y<=rect->bottom)
		return TRUE;

	return FALSE;
}

static BOOL get_key(BYTE *pKey)
{
	KBD_KEY_DATA key_data;

	if(API_GetKey(&key_data)) {
		if(key_data.type == KBD_KTYPE_GENERAL) {
			*pKey = key_data.key;
			return TRUE;
		}
	}

	return FALSE;
}
