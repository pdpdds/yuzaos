#include "cmd.h"


//char * str_append(char *a, char *b);

#define MAX_INSERTS 200 
#define UNICODE_NULL 0
 
// http://read.pudn.com/downloads74/sourcecode/os/272231/msutils/efilib/efisrc/efistrutil.cxx__.htm
NTSTATUS 
RtlFormatMessage( 
    IN LPCSTR MessageFormat, 
    IN ULONG MaximumWidth OPTIONAL, 
    IN BOOLEAN IgnoreInserts, 
    IN BOOLEAN ArgumentsAreAnsi, 
    IN BOOLEAN ArgumentsAreAnArray, 
    IN va_list *Arguments, 
    OUT LPSTR Buffer, 
    IN ULONG Length, 
    OUT PULONG ReturnLength OPTIONAL 
    ) 
{ 
    ULONG Column; 
    int cchRemaining, cchWritten; 
    PULONG_PTR ArgumentsArray = (PULONG_PTR)Arguments; 
    ULONG_PTR rgInserts[ MAX_INSERTS ]; 
    ULONG cSpaces; 
    ULONG MaxInsert, CurInsert; 
    ULONG PrintParameterCount; 
    ULONG_PTR PrintParameter1; 
    ULONG_PTR PrintParameter2; 
    CHAR PrintFormatString[ 32 ]; 
    BOOLEAN DefaultedFormatString; 
    CHAR c; 
    LPSTR s, s1; 
    LPSTR lpDst, lpDstBeg, lpDstLastSpace; 
 
    cchRemaining = Length / sizeof( CHAR ); 
    lpDst = Buffer; 
    MaxInsert = 0; 
    lpDstLastSpace = NULL; 
    Column = 0; 
    s = MessageFormat; 
    while (*s != UNICODE_NULL) { 
        if (*s == '%') { 
            s++; 
            lpDstBeg = lpDst; 
            if (*s >= '1' && *s <= '9') { 
                CurInsert = *s++ - '0'; 
                if (*s >= '0' && *s <= '9') { 
                    CurInsert = (CurInsert * 10) + (*s++ - '0'); 
                    if (*s >= '0' && *s <= '9') { 
                        CurInsert = (CurInsert * 10) + (*s++ - '0'); 
                        if (*s >= '0' && *s <= '9') { 
                            return( STATUS_INVALID_PARAMETER ); 
                            } 
                        } 
                    } 
                CurInsert -= 1; 
 
                PrintParameterCount = 0; 
                if (*s == '!') { 
                    DefaultedFormatString = FALSE; 
                    s1 = PrintFormatString; 
                    *s1++ = '%'; 
                    s++; 
                    while (*s != '!') { 
                        if (*s != UNICODE_NULL) { 
                            if (s1 >= &PrintFormatString[ 31 ]) { 
                                return( STATUS_INVALID_PARAMETER ); 
                                } 
 
                            if (*s == '*') { 
                                if (PrintParameterCount++ > 1) { 
                                    return( STATUS_INVALID_PARAMETER ); 
                                    } 
                                } 
 
                            *s1++ = *s++; 
                            } 
                        else { 
                            return( STATUS_INVALID_PARAMETER ); 
                            } 
                        } 
 
                    s++; 
                    *s1 = UNICODE_NULL; 
                    } 
                else { 
                    DefaultedFormatString = TRUE; 
                    strcpy( PrintFormatString, TEXT("%s") ); 
                    s1 = PrintFormatString + strlen( PrintFormatString ); 
                    } 
 
                if (IgnoreInserts) { 
                    if (!strcmp( PrintFormatString, TEXT("%s") )) { 
                        cchWritten = (int)snprintf( lpDst, 
                                                 cchRemaining, 
                                                 TEXT("%%%u"), 
                                                 CurInsert+1 
                                               ); 
                        } 
                    else { 
                        cchWritten = (int)snprintf( lpDst, 
                                                 cchRemaining, 
                                                 TEXT("%%%u!%s!"), 
                                                 CurInsert+1, 
                                                 &PrintFormatString[ 1 ] 
                                               ); 
                        } 
 
                    if (cchWritten == -1) { 
                        return(STATUS_BUFFER_OVERFLOW); 
                        } 
                    } 
                else 
                if (ARGUMENT_PRESENT( Arguments )) { 
                    if ((CurInsert+PrintParameterCount) >= MAX_INSERTS) { 
                        return( STATUS_INVALID_PARAMETER ); 
                        } 
 
                    if (ArgumentsAreAnsi) { 
                        if (s1[ -1 ] == 'c' && s1[ -2 ] != 'h' 
                          && s1[ -2 ] != 'w' && s1[ -2 ] != 'l') { 
                            strcpy( &s1[ -1 ], TEXT("hc") ); 
                            } 
                        else 
                        if (s1[ -1 ] == 's' && s1[ -2 ] != 'h' 
                          && s1[ -2 ] != 'w' && s1[ -2 ] != 'l') { 
                            strcpy( &s1[ -1 ], TEXT("hs") ); 
                            } 
                        else if (s1[ -1 ] == 'S') { 
                            s1[ -1 ] = 's'; 
                            } 
                        else if (s1[ -1 ] == 'C') { 
                            s1[ -1 ] = 'c'; 
                            } 
                        } 
 
                    while (CurInsert >= MaxInsert) { 
                        if (ArgumentsAreAnArray) { 
                                PULONG_PTR aaa; 
                                aaa = (PULONG_PTR)Arguments++; 
                                PrintParameter2 = rgInserts[ MaxInsert++ ] = *(aaa); 
                            } 
                        else { 
                            rgInserts[ MaxInsert++ ] = va_arg(*Arguments, ULONG_PTR); 
                            } 
                        } 
 
                    s1 = (LPSTR)rgInserts[ CurInsert ]; 
                    PrintParameter1 = 0; 
                    PrintParameter2 = 0; 
                    if (PrintParameterCount > 0) { 
                        if (ArgumentsAreAnArray) { 
                                PULONG_PTR aaa; 
                                aaa = (PULONG_PTR)Arguments; 
                                PrintParameter2 = rgInserts[ MaxInsert++ ] = *(aaa)++; 
                            } 
                        else { 
                            PrintParameter1 = rgInserts[ MaxInsert++ ] = va_arg( *Arguments, ULONG_PTR ); 
                            } 
 
                        if (PrintParameterCount > 1) { 
                            if (ArgumentsAreAnArray) { 
                                PULONG_PTR aaa; 
                                aaa = (PULONG_PTR)Arguments; 
                                PrintParameter2 = rgInserts[ MaxInsert++ ] = *(aaa)++; 
                                } 
                            else { 
                                PrintParameter2 = rgInserts[ MaxInsert++ ] = va_arg( *Arguments, ULONG_PTR ); 
                                } 
                            } 
                        } 
 
                    cchWritten = (int)snprintf( lpDst, 
                                             cchRemaining, 
                                             PrintFormatString, 
                                             s1, 
                                             PrintParameter1, 
                                             PrintParameter2 
                                           ); 
 
                    if (cchWritten == -1) { 
                        return(STATUS_BUFFER_OVERFLOW); 
                        } 
                    } 
                else { 
                    return( STATUS_INVALID_PARAMETER ); 
                    } 
 
                if ((cchRemaining -= cchWritten) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                lpDst += cchWritten; 
                } 
            else 
            if (*s == '0') { 
                if ((cchRemaining -= 1) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                *lpDst++ = '\0'; 
 
                break; 
                } 
            else 
            if (!*s) { 
                return( STATUS_INVALID_PARAMETER ); 
                } 
            else 
            if (*s == 'r') { 
                if ((cchRemaining -= 1) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                *lpDst++ = '\r'; 
                s++; 
                lpDstBeg = NULL; 
                } 
            else 
            if (*s == 'n') { 
                if ((cchRemaining -= 2) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                *lpDst++ = '\r'; 
                *lpDst++ = '\n'; 
                s++; 
                lpDstBeg = NULL; 
                } 
            else 
            if (*s == 't') { 
                if ((cchRemaining -= 1) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                if (Column % 8) { 
                    Column = (Column + 7) & ~7; 
                    } 
                else { 
                    Column += 8; 
                    } 
 
                lpDstLastSpace = lpDst; 
                *lpDst++ = '\t'; 
                s++; 
                } 
            else 
            if (*s == 'b') { 
                if ((cchRemaining -= 1) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                lpDstLastSpace = lpDst; 
                *lpDst++ = ' '; 
                s++; 
                } 
            else 
            if (IgnoreInserts) { 
                if ((cchRemaining -= 2) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                *lpDst++ = '%'; 
                *lpDst++ = *s++; 
                } 
            else { 
                if ((cchRemaining -= 1) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                *lpDst++ = *s++; 
                } 
 
            if (lpDstBeg == NULL) { 
                lpDstLastSpace = NULL; 
                Column = 0; 
                } 
            else { 
                Column += (ULONG)(lpDst - lpDstBeg); 
                } 
            } 
        else { 
            c = *s++; 
            if (c == '\r' || c == '\n') { 
                if ((c == '\n' && *s == '\r') || 
                    (c == '\r' && *s == '\n') 
                   ) { 
                    s++; 
                    } 
 
                if (MaximumWidth != 0) { 
                    lpDstLastSpace = lpDst; 
                    c = ' '; 
                    } 
                else { 
                    c = '\n'; 
                    } 
                } 
 
 
            if (c == '\n') { 
                if ((cchRemaining -= 2) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                *lpDst++ = '\r'; 
                *lpDst++ = '\n'; 
                lpDstLastSpace = NULL; 
                Column = 0; 
                } 
            else { 
                if ((cchRemaining -= 1) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                if (c == ' ') { 
                    lpDstLastSpace = lpDst; 
                    } 
 
                *lpDst++ = c; 
                Column += 1; 
                } 
            } 
 
        if (MaximumWidth != 0 && 
            MaximumWidth != 0xFFFFFFFF && 
            Column >= MaximumWidth 
           ) { 
            if (lpDstLastSpace != NULL) { 
                lpDstBeg = lpDstLastSpace; 
                while (*lpDstBeg == ' ' || *lpDstBeg == '\t') { 
                    lpDstBeg += 1; 
                    if (lpDstBeg == lpDst) { 
                        break; 
                        } 
                    } 
                while (lpDstLastSpace > Buffer) { 
                    if (lpDstLastSpace[ -1 ] == ' ' || lpDstLastSpace[ -1 ] == '\t') { 
                        lpDstLastSpace -= 1; 
                        } 
                    else { 
                        break; 
                        } 
                    } 
 
                cSpaces = (ULONG)(lpDstBeg - lpDstLastSpace); 
                if (cSpaces == 1) { 
                    if ((cchRemaining -= 1) <= 0) { 
                        return STATUS_BUFFER_OVERFLOW; 
                        } 
                    } 
                else 
                if (cSpaces > 2) { 
                    cchRemaining += (cSpaces - 2); 
                    } 
 
                memmove( lpDstLastSpace + 2, 
                         lpDstBeg, 
                         (ULONG) ((lpDst - lpDstBeg) * sizeof( CHAR )) 
                       ); 
                *lpDstLastSpace++ = '\r'; 
                *lpDstLastSpace++ = '\n'; 
                Column = (ULONG)(lpDst - lpDstBeg); 
                lpDst = lpDstLastSpace + Column; 
                lpDstLastSpace = NULL; 
                } 
            else { 
                if ((cchRemaining -= 2) <= 0) { 
                    return STATUS_BUFFER_OVERFLOW; 
                    } 
 
                *lpDst++ = '\r'; 
                *lpDst++ = '\n'; 
                lpDstLastSpace = NULL; 
                Column = 0; 
                } 
            } 
        } 
 
    if ((cchRemaining -= 2) <= 0) { 
        return STATUS_BUFFER_OVERFLOW; 
        } 
 
    *lpDst++ = '\r'; 
    *lpDst++ = '\n'; 
 
    if ((cchRemaining -= 1) <= 0) { 
        return STATUS_BUFFER_OVERFLOW; 
        } 
 
    *lpDst++ = '\0'; 
    if ( ARGUMENT_PRESENT(ReturnLength) ) { 
        *ReturnLength = (ULONG)(lpDst - Buffer) * sizeof( CHAR ); 
        } 
 
    return( STATUS_SUCCESS ); 
} 

#define  FORMAT_MESSAGE_MAX_WIDTH_MASK  0x000000FF


// https://msdn.microsoft.com/en-us/library/windows/desktop/ms679351(v=vs.85).aspx
// https://github.com/NVIDIA/winex_lgpl/blob/9dfd0bf40ee99420b0c99c88ff9d9e570b08cd30/winex/dlls/kernel/format_msg.c
DWORD WINAPI FormatMessage(
  _In_     DWORD   dwFlags,
  _In_opt_ LPCVOID lpSource,
  _In_     DWORD   dwMessageId,
  _In_     DWORD   dwLanguageId,
  _Out_    LPTSTR  lpBuffer,
  _In_     DWORD   nSize,
  _In_opt_ va_list *Arguments
) {
   LPSTR lpAllocedBuffer;

   if (lpBuffer == NULL) {
      SetLastError( STATUS_INVALID_PARAMETER );
      return 0;
    }

  // FindMessage...
  int N = 0;
  const cmdmsg_t *msg = NULL;
  while( TRUE ) {
    msg = &g_cmdmsg[N];
    if( msg->id == 0xFFFFFFFF ) {      
      msg = NULL;
      break;
    }
    if( msg->id == dwMessageId ) {
      break;
    }
    N++;
  }

  if( msg == NULL ) {
    snprintf(lpBuffer, nSize, "Message: %d (%x)\n", dwMessageId, dwMessageId);
  }
  else {
    BOOLEAN IgnoreInserts = dwFlags & FORMAT_MESSAGE_IGNORE_INSERTS;
    BOOLEAN ArgumentsAreAnArray = dwFlags & FORMAT_MESSAGE_ARGUMENT_ARRAY;
    DWORD MaximumWidth = dwFlags & FORMAT_MESSAGE_MAX_WIDTH_MASK;
    if (MaximumWidth == FORMAT_MESSAGE_MAX_WIDTH_MASK) {
      MaximumWidth = 0xFFFFFFFF;
    }

    DWORD LengthNeeded = 0;
    char *buf = malloc(nSize + 1);
    NTSTATUS Status = RtlFormatMessage( msg->data,
                                   MaximumWidth,
                                   IgnoreInserts,
                                   TRUE, // ArgumentsAreAnsi,
                                   ArgumentsAreAnArray,
                                   Arguments,
                                   buf,
                                   nSize,
                                   &LengthNeeded
                                 );
    if( Status != STATUS_SUCCESS ) {
      free(buf);
    }
    //snprintf(lpBuffer, nSize, "%s", msg->data); 
    strncpy(lpBuffer, buf, LengthNeeded);
  }
  return strlen(lpBuffer);
}
