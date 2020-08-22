#ifndef PORTABLE_MSG_H_
#define PORTABLE_MSG_H_


typedef struct {
  DWORD id;
  const char *name;
  const char *data;
} cmdmsg_t;

extern const cmdmsg_t g_cmdmsg[];

DWORD WINAPI FormatMessage(
  _In_     DWORD   dwFlags,
  _In_opt_ LPCVOID lpSource,
  _In_     DWORD   dwMessageId,
  _In_     DWORD   dwLanguageId,
  _Out_    LPTSTR  lpBuffer,
  _In_     DWORD   nSize,
  _In_opt_ va_list *Arguments
);

#endif
