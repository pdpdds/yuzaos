#include "chobits_api.h"

/* 
 * codes by Yeori
 *
 * title : Snake game!
 * e-mail : alphamcu@hanmail.net
 * www : www.zap.pe.kr (english : zap.pe.kr/eng)
 */


/*
 * DEFINITIONS
 */
#define BORDER_LEFT_POS		2
#define BORDER_TOP_POS		3
#define BORDER_RIGHT_POS	(80-1-BORDER_LEFT_POS)
#define BORDER_BOTTOM_POS	(24-2)

#define DIRECTION_UP		0x00
#define DIRECTION_DOWN		0x01
#define DIRECTION_LEFT		0x02
#define DIRECTION_RIGHT		0x03

#define MSG_BOX_XPOS		5
#define MSG_BOX_YPOS		(BORDER_BOTTOM_POS+2)

#define LEFT_KEY			'j'
#define RIGHT_KEY			'l'
#define UP_KEY				'i'
#define DOWN_KEY			'k'
#define QUIT_KEY			'q'

#define APPLE_PLUS_CELL		5
#define SNAKE_TOTAL_CELL	100

#define TIME_DELAY			100 /* 0.1 sec */

typedef struct _SNAKE_CELL {
	BOOL		draw;
	WORD		x, y;
	BYTE		direction;
} SNAKE_CELL, *PSNAKE_CELL;

typedef struct _APPLE_POS {
	WORD		x, y;
} APPLE_POS, *PAPPLE_POS;

/*
 * INTERNEL FUNCTIONS
 */
static void draw_border(void);
static void draw_apple(void);
static BOOL move_snake(void);
static void move_snake_cell(SNAKE_CELL *pCell);
static void draw_snake_with_this(BYTE *pThis);
static void print_score(void);
static void got_apple(void);
static void retry_game(void);
static BOOL is_snake_corrupt(void);
static BOOL lengthen_snake(BYTE new_pos);

static BOOL get_key(BYTE *pKey);

/*
 * GLOBAL VARIABLES
 */
static BYTE body[2]={'#', 0x00};
static BYTE apple[2]={0x01, 0x00};
static BYTE block[2]={'*', 0x00};
static BYTE blank[2]={' ', 0x00};

static SNAKE_CELL m_SnakeCell[SNAKE_TOTAL_CELL];
static APPLE_POS  m_ApplePos;
static BYTE       m_CurDirection;
static DWORD      m_Score;
static BYTE       m_RemainedLength;

/***************************************************************************************************/
int main()
{
	BYTE key_data;

	/* init */
	retry_game();

	while(1) {
		/* get key */
		if(get_key(&key_data)) {
			switch(key_data) {
				case LEFT_KEY:
					m_CurDirection = DIRECTION_LEFT;
					break;
				case RIGHT_KEY:
					m_CurDirection = DIRECTION_RIGHT;
					break;
				case UP_KEY:
					m_CurDirection = DIRECTION_UP;
					break;
				case DOWN_KEY:
					m_CurDirection = DIRECTION_DOWN;
					break;
				case QUIT_KEY:
					goto $exit;
					break;
				default:
					break;
			}
		}

		/* score */
		print_score();

		/* draw apple */
		draw_apple();

		/* draw snake */
		if(!move_snake()) {
			API_PrintTextXY("YOU GUY DEAD!! Press any key to quit!, or 'r' to retry ;)", MSG_BOX_XPOS, MSG_BOX_YPOS);
			while(!API_HasKey()) ;
			if(get_key(&key_data)) {
				if(key_data == 'r') {
					retry_game();
					continue;
				} else
					break;
			}
		}

		/* move cursor */
		API_PrintTextXY(" ", 76, 0);

		/* delay */
		API_Delay(TIME_DELAY);
	}

$exit:
	API_FlushKbdBuf();
	API_ClearScreen();
	API_PrintText("Bye~! ;) \r\n");
	return 0;
}

static void retry_game(void)
{
	int i;

	memset(&m_ApplePos, 0, sizeof(m_ApplePos));
	memset(m_SnakeCell, 0, sizeof(m_SnakeCell));

	m_Score = 0;
	m_RemainedLength = 0;
	m_CurDirection = DIRECTION_UP;

	m_ApplePos.x = BORDER_LEFT_POS+5;
	m_ApplePos.y = BORDER_TOP_POS+5;

	m_SnakeCell[0].draw = TRUE;
	m_SnakeCell[0].x = (BORDER_RIGHT_POS+BORDER_LEFT_POS)/2;
	m_SnakeCell[0].y = (BORDER_BOTTOM_POS+BORDER_TOP_POS)/2;
	for(i=1; i<5; i++) {
		m_SnakeCell[i].draw = TRUE;
		m_SnakeCell[i].x = (BORDER_RIGHT_POS+BORDER_LEFT_POS)/2;
		m_SnakeCell[i].y = m_SnakeCell[i-1].y+1;
	}

	/* draw border */
	draw_border();

	/* game gogogo! */
	API_FlushKbdBuf();
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

static void print_score(void)
{
#define SCORE_X_POS		0
#define SCORE_Y_POS		0

	BYTE score_str[100];

	sprintf(score_str, "You got : %d points! ;)", m_Score);
	API_PrintTextXY(score_str, SCORE_X_POS, SCORE_Y_POS);
}

static void draw_apple(void)
{
	API_PrintTextXY(apple, m_ApplePos.x, m_ApplePos.y);
}

static BOOL move_snake(void)
{
	int i;
	BYTE new_direction = m_CurDirection, temp;

	/* draw blank */
	draw_snake_with_this(blank);

	/* move pos */
	for(i=0; i<SNAKE_TOTAL_CELL; i++) {
		if(m_SnakeCell[i].draw == FALSE)
			break;

		temp = m_SnakeCell[i].direction;
		m_SnakeCell[i].direction = new_direction;
		move_snake_cell(&m_SnakeCell[i]);
		new_direction = temp;
	}

	/* lengthen */
	if(m_RemainedLength) {
		if(!lengthen_snake(i))
			m_RemainedLength = 0;
		else 
			m_RemainedLength--;
	}

	if(is_snake_corrupt())
		return FALSE;

	/* is out of border? */
	if(m_SnakeCell[0].x <= BORDER_LEFT_POS || m_SnakeCell[0].x >= BORDER_RIGHT_POS
		|| m_SnakeCell[0].y <= BORDER_TOP_POS || m_SnakeCell[0].y >= BORDER_BOTTOM_POS)
		return FALSE;

	/* got apple? */
	if(m_SnakeCell[0].x == m_ApplePos.x && m_SnakeCell[0].y == m_ApplePos.y) {
		got_apple();
		m_RemainedLength+=APPLE_PLUS_CELL;
	}

	/* redraw */
	draw_snake_with_this(body);

	return TRUE;
}

static BOOL lengthen_snake(BYTE new_pos)
{
	int x, y;

	if(new_pos == SNAKE_TOTAL_CELL) {
		return FALSE;
	}

	m_SnakeCell[new_pos].draw = TRUE;
	m_SnakeCell[new_pos].direction = m_SnakeCell[new_pos-1].direction;
	x = m_SnakeCell[new_pos-1].x;
	y = m_SnakeCell[new_pos-1].y;
	switch(m_SnakeCell[new_pos-1].direction) {
		case DIRECTION_UP:
			y++;
			break;
		case DIRECTION_DOWN:
			y--;
			break;
		case DIRECTION_LEFT:
			x++;
			break;
		case DIRECTION_RIGHT:
			x--;
			break;
	}
	m_SnakeCell[new_pos].x = x;
	m_SnakeCell[new_pos].y = y;

	return TRUE;
}

static BOOL is_snake_corrupt(void)
{
	int i, j;

	for(i=0; i<SNAKE_TOTAL_CELL-1; i++) {
		if(m_SnakeCell[i].draw == FALSE)
			break;

		for(j=i+1; j<SNAKE_TOTAL_CELL; j++) {
			if(m_SnakeCell[j].draw == FALSE)
				break;

			if(m_SnakeCell[i].x == m_SnakeCell[j].x && m_SnakeCell[i].y == m_SnakeCell[j].y)
				return TRUE;
		}
	}

	return FALSE;
}

static void got_apple(void)
{
	DWORD tick;
	BYTE new_xpos, new_ypos;

	m_Score += 100;
	print_score();

	while(1) {
		tick = API_GetTickCount();
		new_xpos = (BYTE)(tick%80);
		tick = API_GetTickCount();
		new_ypos = (BYTE)(tick%24);

		if(new_xpos <= BORDER_LEFT_POS || new_xpos >= BORDER_RIGHT_POS
			|| new_ypos <= BORDER_TOP_POS || new_ypos >= BORDER_BOTTOM_POS)
			continue;

		for(tick=0; tick<SNAKE_TOTAL_CELL; tick++) {
			if(m_SnakeCell[tick].draw == FALSE)
				break;

			if(m_SnakeCell[tick].x == new_xpos && m_SnakeCell[tick].y == new_ypos)
				goto $go_ahead;
		}

		m_ApplePos.x = new_xpos;
		m_ApplePos.y = new_ypos;
		break;
$go_ahead:
		;
	}

	draw_apple();
}

static void move_snake_cell(SNAKE_CELL *pCell)
{
	switch(pCell->direction)
	{
	case DIRECTION_LEFT:
		pCell->x--;
		break;
	case DIRECTION_UP:
		pCell->y--;
		break;
	case DIRECTION_RIGHT:
		pCell->x++;
		break;
	case DIRECTION_DOWN:
		pCell->y++;
		break;
	}
}

static void draw_snake_with_this(BYTE *pThis)
{
	int i;

	for(i=0; i<SNAKE_TOTAL_CELL; i++) {
		if(m_SnakeCell[i].draw)
			API_PrintTextXY(pThis, m_SnakeCell[i].x, m_SnakeCell[i].y);
	}
}

static void draw_border(void)
{
	BYTE i;

	/* clear screen */
	API_ClearScreen();

	/* print help */
	API_PrintTextXY("==> d'you need any help? LEFT=j, RIGHT=l, UP=i, DOWN=k, QUIT:q", 0, 1);

	/* draw border */
	for(i=BORDER_LEFT_POS; i<=BORDER_RIGHT_POS; i++) {
		API_PrintTextXY(block, i, BORDER_TOP_POS);
		API_PrintTextXY(block, i, BORDER_BOTTOM_POS);
	}
	for(i=BORDER_TOP_POS; i<=BORDER_BOTTOM_POS; i++) {
		API_PrintTextXY(block, BORDER_LEFT_POS, i);
		API_PrintTextXY(block, BORDER_RIGHT_POS, i);
	}
}