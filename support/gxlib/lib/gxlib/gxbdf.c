/*******************************************************************************
프로젝트 : gxLib
모듈내용 : BDF 폰트 출력
변경일자 : 2008-10-22
작성자   : 푸른수염(ssseo88@chol.com)
수정자   : 장길석( jwjw, jwjwmx@gmail.com)
저작권   : 주석 내용을 변경하지 않는 한 무료 제공
수정내용 :
            - 2010-05-03
                폰트를 여러개 사용할 수 있도록 수정
홈페이지 : http://forum.falinux.com
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>                                                              // malloc srand
#include <string.h>                                                              // abs

#include <unistd.h>                                                              // open/close
#include <fcntl.h>                                                               // O_RDWR

#include <gx.h>
#include <gxbdf.h>
#include <fontinfo.h>
#include <ksc5601.h>

#define MAX_STRING_LENGTH     1024

//jwjw bdfFontCache   gx_fontcache;
//jwjw FILE*          gx_fontfile    = NULL;

static void  _text_out_( dc_t *dc, int org_x , int org_y, unsigned short* string)
//-------------------------------------------------------------------------------
{
    int             k , i, j ,m;
    int             xPixel, yPixel;
    int             save_x;
    unsigned int    nCode;
    bdfFont         Font;
    pbdfFont        pFont;
    
    for( k = 0 ; string[k] != 0x0000 ; k+=1 )
    {
        nCode = string[k];//(0xff00 & ( ((unsigned char ) str[strIndex][k]) << 8 )) | ( ( (unsigned char) str[strIndex][k+1]) & 0x00ff );
        
        if ( ReadFontOfCode( &dc->font->fontcache, dc->font->fontfile, dc->font->fndex, nCode ,&Font ))
            pFont = &Font;
        else
            pFont = NULL;
        
        if ( pFont != NULL )
        {
            save_x = xPixel = org_x + pFont->bbxoff;
            yPixel = org_y - pFont->bbyoff;
            
            for( i = pFont->bbh -1 ; i >=0   ; --i )
            {
                for ( j=0 ; j < (pFont->bbw+8)/8 ; ++ j)
                {
                    for( m = 0 ; m < 8 ; ++m )
                    {
                        if ( ( j*8 + m + 1) > pFont->bbw ) break;
                        if ((( pFont->data[(pFont->bbw+7)/8*i+j] & (1 << (8-m-1)) )>>(8-m-1)) == 1 )
                        {
                            gx_set_pixel( dc ,xPixel ,yPixel, dc->font_color);
                        }
                        else
                        {
                            //HBRUSH hOld = (HBRUSH)::SelectObject( pDC->GetSafeHdc() ,::GetStockObject(WHITE_BRUSH) );
                            //pDC->Rectangle(CRect(x,y,x+1 , y+1 ));
                            //::SelectObject( pDC->GetSafeHdc() , hOld );
                        }
                        xPixel++;
                    }
                }
                yPixel--;
                xPixel = save_x;
            }
            org_x+= pFont->dwidth;
        }
    }
}

int gx_text_out( dc_t *dc, int coor_x, int coor_y, char *text)
/**
    @brief  DC로 문자열을 출력한다.\n
    주의: coo_y는 문자열 출력의 하단 좌표. 즉, coor_x=좌측 좌표, coor_y=하단 좌표
    @param  dc : Device Context
            coor_x : x 좌표
            coor_y : 하단 y 좌표 
            text : 출력할 문자열
*/
{
    static unsigned short   uniString[MAX_STRING_LENGTH+1];
    int                     szText;
                     
    if  ( NULL == dc->font)                                             // 지정된 폰트가 없다면 복귀
    {          
        printf( "gx_text_out() : no font assigned.\n");
        return GXERR_NO_ASSIGNED_FONT;
    }
    szText   = strlen( text);
    if ( MAX_STRING_LENGTH < szText)
    {
       text[MAX_STRING_LENGTH] = '\0';
       szText                  = MAX_STRING_LENGTH;
    }
    
    convertMultibyteToUnicodeString( (unsigned char*)text, szText, uniString, MAX_STRING_LENGTH);
    _text_out_( dc, coor_x ,coor_y, uniString);
    
    return GXERR_NONE;
}

void gx_set_font( dc_t *dc, font_t *font)
/**
    @brief  인수로 받은 DC에 폰트를 지정한다.\n
    이 함수를 사용하지 않고 DC의 font 속성을 직접 지정해도 된다.
    @param  dc : Device Context
            font : 글꼴 포인터
    @return\n
        폰트 사용을 위한 포인터
        NULL : 폰트 열기에 실패
*/
{
    dc->font    = font;
}

font_t *gx_open_font( char *font_filename)
/**
    @brief  폰트를 사용하기 위한 글꼴 파일을 열기를 한다.\n
    gxLib는 폰트를 사용하기 위한 gx_open_font()만 제공한다. \n
    임베디드 시스템에서는 한 번 열기를 한 폰트는 프로그램 종료까지 계속 사용되기도 하지만, \n
    폰트 내부 처리 함수에서 malloc() 함수를 호출하는 루틴에 대한 분석이 미약하기 때문이다. \n
    gx_close_font()를 생성하려면 fontinfo.c 에서 호출하는 malloc()에 대한 free()를 해주어야 한다.
    @param  font_filename : 글꼴 파일 이름
    @return\n
        폰트 사용을 위한 포인터
        NULL : 폰트 열기에 실패
*/
{
    font_t  *font;
    
    font = malloc( sizeof( font_t));                                               // dc_t 구조체 메모리 할당
    if ( NULL != font)                                                             // 메모리 구하기에 성공함녀
    {
        font->fontfile = fopen( font_filename, "r");                               // 폰트 파일 열기
        if ( NULL == font->fontfile)
        {
            free( font);
            printf( "gx_open_font() : no file->%s\n", font_filename);
            return NULL;
        }
        FontCacheInit( &font->fontcache);
        FontChanged(    font->fontfile,  &font->fontcache, font->fndex);
    }
    else
    {
        printf( "gx_open_font() : out of memory.\n");    
    }
    return font;
}
