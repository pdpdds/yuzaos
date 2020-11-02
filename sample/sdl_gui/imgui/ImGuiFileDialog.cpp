#include "ImGuiFileDialog.h"
#include "imgui.h"
//#include "dirent/dirent.h"
#include <minwindef.h>
#include <dirent.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <algorithm>
#include <FileService.h>

inline bool ReplaceString(std::string& str, const std::string& oldStr, const std::string& newStr)
{
	bool Finded = false;
	size_t pos = 0;
	while ((pos = str.find(oldStr, pos)) != std::string::npos) {
		Finded = true;
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
	return Finded;
}

inline std::vector<std::string> splitStringVector(const std::string& text, char delimiter)
{
	std::vector<std::string> arr;
	std::string::size_type start = 0;
	std::string::size_type end = text.find(delimiter, start);
	while (end != std::string::npos)
	{
		std::string token = text.substr(start, end - start);
		if (token.compare("") != 0)
			arr.push_back(token);
		start = end + 1;
		end = text.find(delimiter, start);
	}
	arr.push_back(text.substr(start));
	return arr;
}

inline void AppendToBuffer(char* vBuffer, int vBufferLen, std::string vStr)
{
	std::string st = vStr;
	if (st.compare("") != 0 && st.compare("\n") != 0)
		ReplaceString(st, "\n", "");
	int slen = strlen(vBuffer);
	vBuffer[slen] = '\0';
	std::string str = std::string(vBuffer);
	if (str.size() > 0) str += "\n";
	str += vStr;
	int len = vBufferLen - 1;
	if (len > str.size()) len = str.size();
#ifdef MINGW32
	strncpy_s(vBuffer, vBufferLen, str.c_str(), len);
#else
	strncpy(vBuffer, str.c_str(), len);
#endif
	vBuffer[len] = '\0';
}

inline void ResetBuffer(char* vBuffer)
{
	vBuffer[0] = '\0';
}

char ImGuiFileDialog::FileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
int ImGuiFileDialog::FilterIndex = 0;

ImGuiFileDialog::ImGuiFileDialog()
{

}

ImGuiFileDialog::~ImGuiFileDialog()
{

}

/* Alphabetical sorting */
static int alphaSort(
	const void* a, const void* b)
{
	return strcoll(((dirent*)a)->d_name, ((dirent*)b)->d_name);
}

static bool stringComparator(FileInfoStruct a, FileInfoStruct b)
{
	bool res;
	if (a.type != b.type) res = (a.type < b.type);
	else res = (a.fileName < b.fileName);
	return res;
}
size_t get_dir_item_count(const char* dirname, int (*sdfilter)(dirent*))
{
	DIR* dirp;
	int count = 0;
	int result = 0;
	dirent* info;
	if ((dirp = opendir(dirname)) == 0)
		return 0;

	while ((info = readdir(dirp)) != 0)
	{
		// 디렉터리에서 엔트리 하나를 읽음
		/*result = readdir(&dirp, &info);
		// 더 이상 파일이 없으면 나감
		if (result != 0)
		{
			break;
		}*/

		if (sdfilter != NULL && !(*sdfilter)(info))
			continue;       /* just selected names */

		count++;
	}

	closedir(dirp);

	return count;
}


int scandir(const char* dirname, dirent*** namelist, int (*sdfilter)(dirent*), int (*dcomp)(const void*, const void*))
{
	dirent* p = NULL, ** names = NULL;
	dirent* info;
	size_t nitems = 0;

	size_t arraysz = get_dir_item_count(dirname, sdfilter);

	if (arraysz == 0)
		return 0;

	DIR* dirp;
	int result = 0;

	if ((dirp = opendir(dirname)) == 0)
		goto error_out;

	names = (dirent**)malloc(arraysz * sizeof(dirent*));
	if (names == NULL)
		goto error_out;

	while ((info = readdir(dirp)) != 0)
	{
		
		if (sdfilter != NULL && !(*sdfilter)(info))
			continue;       /* just selected names */

		p = (dirent*)malloc(sizeof(dirent));
		if (p == NULL)
			goto error_out;

		memcpy(p, info, sizeof(dirent));

		names[nitems++] = p;

		p = NULL;
	}

	closedir(dirp);

	if (nitems && dcomp != NULL)
		qsort(names, nitems, sizeof(dirent*), dcomp);

	*namelist = names;

	return nitems;

error_out:
	if (names)
	{
		int i;
		for (i = 0; i < nitems; i++)
			free(names[i]);
		free(names);
	}

	closedir(dirp);
	return -1;
}


void ImGuiFileDialog::ScanDir(std::string vPath)
{
	dirent** files = 0;
	int i = 0;
	int n = 0;

	m_FileList.clear();

	if (m_CurrentPath_Decomposition.size() == 0)
	{
		// get currentPath
		DIR* currentDir = opendir(vPath.c_str());
		if (currentDir == 0) // path not existing
		{
			vPath = "."; // current  app path
			currentDir = opendir(vPath.c_str());
		}
		if (currentDir != 0)
		{
			std::string str;
			str = (char*)currentDir->name;
			m_CurrentPath = std::string(str.begin(), str.end());
			ReplaceString(m_CurrentPath, "\\*", "");
			m_CurrentPath = vPath;
			closedir(currentDir);
			m_CurrentPath_Decomposition = splitStringVector(m_CurrentPath, '\\');
		}
		else
		{
			return;
		}
	}

	/* Scan files in directory */
	n = scandir(vPath.c_str(), &files, NULL, alphaSort);
	if (n >= 0)
	{
		for (i = 0; i < n; i++)
		{
			dirent* ent = files[i];

			FileInfoStruct infos;

			infos.fileName = ent->d_name;

			if (infos.fileName.compare(".") != 0)
			{
				if (ent->dwAttribute == 0)
				{
					infos.type = 'd';
				}
				else
				{
					infos.type = 'f';
				}

				if (infos.type == 'f')
				{
					size_t lpt = infos.fileName.find_last_of(".");
					if (lpt != std::string::npos)
						infos.ext = infos.fileName.substr(lpt);
				}


				m_FileList.push_back(infos);
			}
		}

		for (i = 0; i < n; i++)
		{
			free(files[i]);
		}
		free(files);
	}

	std::sort(m_FileList.begin(), m_FileList.end(), stringComparator);
}

void ImGuiFileDialog::SetCurrentDir(std::string vPath)
{
	DIR* dir = opendir(vPath.c_str());
	if (dir == 0)
	{
		vPath = ".";
		dir = opendir(vPath.c_str());
	}
	if (dir != 0)
	{
		std::string str;
		str = dir->name;
		m_CurrentPath = std::string(str.begin(), str.end());
		ReplaceString(m_CurrentPath, "\\*", "");
		closedir(dir);
		m_CurrentPath_Decomposition = splitStringVector(m_CurrentPath, '\\');
	}
}

void ImGuiFileDialog::ComposeNewPath(std::vector<std::string>::iterator vIter)
{
	m_CurrentPath = "";
	while (vIter != m_CurrentPath_Decomposition.begin())
	{
		if (m_CurrentPath.size() > 0)
			m_CurrentPath = *vIter + "\\" + m_CurrentPath;
		else
			m_CurrentPath = *vIter;
		vIter--;
	}

	if (m_CurrentPath.size() > 0)
		m_CurrentPath = *vIter + "\\" + m_CurrentPath;
	else
		m_CurrentPath = *vIter + "\\";
}

bool ImGuiFileDialog::FileDialog(const char* vName, const char* vFilters, std::string vPath, std::string vDefaultFileName)
{
	bool res = false;

	IsOk = false;

	ImGui::Begin(vName);

	if (vPath.size() == 0) vPath = ".";

	if (m_FileList.size() == 0)
	{
		if (vDefaultFileName.size() > 0)
		{
			ResetBuffer(FileNameBuffer);
			AppendToBuffer(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, vDefaultFileName);
		}

		ScanDir(vPath);
	}

	// show current path
	bool pathClick = false;
	for (std::vector<std::string>::iterator itPathDecomp = m_CurrentPath_Decomposition.begin();
		itPathDecomp != m_CurrentPath_Decomposition.end(); ++itPathDecomp)
	{
		if (itPathDecomp != m_CurrentPath_Decomposition.begin())
			ImGui::SameLine();
		if (ImGui::Button((*itPathDecomp).c_str()))
		{
			ComposeNewPath(itPathDecomp);
			pathClick = true;
			break;
		}
	}

	//ImVec2 size = ImGui::GetContentRegionMax() - ImVec2(0.0f, 120.0f);
	ImVec2 size = { 300, 400 };

	ImGui::BeginChild("##FileDialog_FileList", size);

	for (std::vector<FileInfoStruct>::iterator it = m_FileList.begin(); it != m_FileList.end(); ++it)
	{
		FileInfoStruct infos = *it;

		bool show = true;

		std::string str;
		if (infos.type == 'd') str = "[Dir] " + infos.fileName;
		if (infos.type == 'l') str = "[Link] " + infos.fileName;
		if (infos.type == 'f') str = "[File] " + infos.fileName;
		if (infos.type == 'f' && m_CurrentFilterExt.size() > 0 && infos.ext != m_CurrentFilterExt)
		{
			show = false;
		}
		if (show == true)
		{
			if (ImGui::Selectable(str.c_str(), (infos.fileName == m_SelectedFileName)))
			{
				if (infos.type == 'd')
				{
					if (infos.fileName == "..")
					{
						if (m_CurrentPath_Decomposition.size() > 1)
						{
							std::vector<std::string>::iterator itPathDecomp = m_CurrentPath_Decomposition.end() - 2;
							ComposeNewPath(itPathDecomp);
						}
					}
					else
					{
						m_CurrentPath += "\\" + infos.fileName;
					}
					pathClick = true;
				}
				else
				{
					m_SelectedFileName = infos.fileName;
					ResetBuffer(FileNameBuffer);
					AppendToBuffer(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, m_SelectedFileName);
				}
				break;
			}
		}
	}

	// changement de repertoire
	if (pathClick == true)
	{
		ScanDir(m_CurrentPath);
		m_CurrentPath_Decomposition = splitStringVector(m_CurrentPath, '\\');
		if (m_CurrentPath_Decomposition.size() == 2)
			if (m_CurrentPath_Decomposition[1] == "")
				m_CurrentPath_Decomposition.erase(m_CurrentPath_Decomposition.end() - 1);
	}

	ImGui::EndChild();

	ImGui::Text("File Name : ");

	ImGui::SameLine();

	float width = ImGui::GetContentRegionAvailWidth();
	if (vFilters != 0) width -= 120.0f;
	ImGui::PushItemWidth(width);
	ImGui::InputText("##FileName", FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
	ImGui::PopItemWidth();

	if (vFilters != 0)
	{
		ImGui::SameLine();

		ImGui::PushItemWidth(100.0f);
		bool comboClick = ImGui::Combo("##Filters", &FilterIndex, vFilters) || m_CurrentFilterExt.size() == 0;
		ImGui::PopItemWidth();
		if (comboClick == true)
		{
			int itemIdx = 0;
			const char* p = vFilters;
			while (*p)
			{
				if (FilterIndex == itemIdx)
				{
					m_CurrentFilterExt = std::string(p);
					break;
				}
				p += strlen(p) + 1;
				itemIdx++;
			}
		}
	}

	if (ImGui::Button("Cancel"))
	{
		IsOk = false;
		res = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Ok"))
	{
		IsOk = true;
		res = true;
	}

	ImGui::End();

	if (res == true)
	{
		m_FileList.clear();
	}

	return res;
}

std::string ImGuiFileDialog::GetFilepathName()
{
	return GetCurrentPath() + "\\" + GetCurrentFileName();
}

std::string ImGuiFileDialog::GetCurrentPath()
{
	return m_CurrentPath;
}

std::string ImGuiFileDialog::GetCurrentFileName()
{
	return std::string(FileNameBuffer);
}

std::string ImGuiFileDialog::GetCurrentFilter()
{
	return m_CurrentFilterExt;
}
