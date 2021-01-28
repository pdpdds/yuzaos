#ifndef _INCLUDED_INTALL_MY_HANDLER_H_
#define _INCLUDED_INTALL_MY_HANDLER_H_

namespace my_handler
{
    __declspec(dllexport) bool install_my_handler();
    __declspec(dllexport) bool restore_cpp_handler();
}

#endif