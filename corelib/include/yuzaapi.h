#pragma once
#include <windef.h>
#include <string>

#if  defined(DLL_YUZA_API_EXPORT)
#define YUZA_API __declspec(dllexport) 
#else
#define YUZA_API __declspec(dllimport)
#endif

#define LONGTYPE(a) a##i64
#define snprintf _snprintf
#define vsnprintf vsnprintf


#define CROSS_LEN 512						/* Maximum filename size */


#if defined (WIN32) || defined (OS2)				/* Win 32 & OS/2*/
#define CROSS_FILENAME(blah) 
#define CROSS_FILESPLIT '\\'
#define F_OK 0
#else
#define	CROSS_FILENAME(blah) strreplace(blah,'\\','/')
#define CROSS_FILESPLIT '/'
#endif

#define CROSS_NONE	0
#define CROSS_FILE	1
#define CROSS_DIR	2
#if defined (WIN32)
#define ftruncate(blah,blah2) chsize(blah,blah2)
#endif

//Solaris maybe others
#if defined (DB_HAVE_NO_POWF)
#include <math.h>
static inline float powf (float x, float y) { return (float) pow (x,y); }
#endif

class Cross {
public:
	static void GetPlatformConfigDir(std::string& in);
	static void GetPlatformConfigName(std::string& in);
	static void CreatePlatformConfigDir(std::string& in);
	static void ResolveHomedir(std::string & temp_line);
	static void CreateDir(std::string const& temp);
	static bool IsPathAbsolute(std::string const& in);
};


#if defined (WIN32)

#define WIN32_LEAN_AND_MEAN        // Exclude rarely-used stuff from 
#include <windows.h>

typedef struct dir_struct {
	HANDLE          handle;
	char            base_path[MAX_PATH+4];
	WIN32_FIND_DATA search_data;
} dir_information;

#else

//#include <sys/types.h> //Included above
#include <dirent.h>

typedef struct dir_struct { 
	DIR*  dir;
	char base_path[CROSS_LEN];
} dir_information;

#endif

#ifdef __cplusplus
extern "C" {
#endif
	YUZA_API dir_information* open_directory(const char* dirname);
	YUZA_API bool read_directory_first(dir_information* dirp, char* entry_name, bool& is_directory);
	YUZA_API bool read_directory_next(dir_information* dirp, char* entry_name, bool& is_directory);
	YUZA_API void close_directory(dir_information* dirp);

	YUZA_API void SendSerialLog(const char* fmt, ...);
	YUZA_API void exit_process(int errorcode);
	YUZA_API int GetCommandFromKeyboard(char* buffer, int length);

#ifdef __cplusplus
}
#endif