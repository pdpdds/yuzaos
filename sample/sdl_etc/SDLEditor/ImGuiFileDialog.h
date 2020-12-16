#ifndef __IMGUI_FILE_DIALOG_H_
#define __IMGUI_FILE_DIALOG_H_

#include <eastl/vector.h>
#include <eastl/string.h>

#define MAX_FILE_DIALOG_NAME_BUFFER 1024

struct FileInfoStruct
{
	char type;
	eastl::string filePath;
	eastl::string fileName;
	eastl::string ext;
};

class ImGuiFileDialog
{
private:
	eastl::vector<FileInfoStruct> m_FileList;
	eastl::string m_SelectedFileName;
	eastl::string m_CurrentPath;
	eastl::vector<eastl::string> m_CurrentPath_Decomposition;
	eastl::string m_CurrentFilterExt;

public:
	static char FileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
	static int FilterIndex;
	bool IsOk;

public:
	static ImGuiFileDialog* Instance()
	{
		static ImGuiFileDialog *_instance = new ImGuiFileDialog();
		return _instance;
	}

public:
	ImGuiFileDialog();
	~ImGuiFileDialog();

	bool FileDialog(const char* vName, const char* vFilters = 0, eastl::string vPath = ".", eastl::string vDefaultFileName = "");
	eastl::string GetFilepathName();
	eastl::string GetCurrentPath();
	eastl::string GetCurrentFileName();
	eastl::string GetCurrentFilter();

private:
	void ScanDir(eastl::string vPath);
	void SetCurrentDir(eastl::string vPath);
	void ComposeNewPath(eastl::vector<eastl::string>::iterator vIter);
};


#endif // __IMGUI_FILE_DIALOG_H_