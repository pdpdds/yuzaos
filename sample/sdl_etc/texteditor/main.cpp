#include "widget.hpp"
#include "application.hpp"
#include "main_window.hpp"

void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

int main(int argc, char **argv)
{
  Application a(argc, argv);
  MainWindow mainWindow;
  return a.exec();
}
