#pragma once
#include "minwindef.h"
#include "skyoswindow.h"

inline void kInternalDrawPixel( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, int iX, int iY, COLOR stColor );
void kInternalDrawLine( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, int iX1, int iY1, int iX2, int iY2, COLOR stColor );
void kInternalDrawRect( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, int iX1, int iY1, int iX2, int iY2, COLOR stColor, bool bFill );
void kInternalDrawCircle( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, int iX, int iY, int iRadius, COLOR stColor, bool bFill );
int kInternalDrawText( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, const char* pcString, int iLength );
int kInternalDrawEnglishText( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, const char* pcString, int iLength );
int kInternalDrawHangulText( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, const char* pcString, int iLength );

inline bool kIsInRectangle( const RECT* pstArea, int iX, int iY );
inline int kGetRectangleWidth( const RECT* pstArea );
inline int kGetRectangleHeight( const RECT* pstArea );
inline void kSetRectangleData( int iX1, int iY1, int iX2, int iY2, RECT* pstRect );
inline bool kGetOverlappedRectangle( const RECT* pstArea1, const RECT* pstArea2, RECT* pstIntersection  );
inline bool kIsRectangleOverlapped( const RECT* pstArea1, const RECT* pstArea2 );
