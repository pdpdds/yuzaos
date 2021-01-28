#include <minwindef.h>
#include <excpt.h>
#include "install_my_handler.h"
#include <memory.h>

//C++'s default exception handler
extern "C" 
EXCEPTION_DISPOSITION  __CxxFrameHandler(
     struct _EXCEPTION_RECORD *ExceptionRecord,
     void * EstablisherFrame,
     struct _CONTEXT *ContextRecord,
     void * DispatcherContext
     );

namespace
{
    char cpp_handler_instructions[5];
    bool saved_handler_instructions = false;
}

namespace my_handler
{
    //Exception handler that replaces C++'s default handler.
    EXCEPTION_DISPOSITION my_exc_handler(
         struct _EXCEPTION_RECORD *ExceptionRecord,
         void * EstablisherFrame,
         struct _CONTEXT *ContextRecord,
         void * DispatcherContext
         ) throw();

#pragma pack(1)
    struct jmp_instr
    {
        unsigned char jmp;
        DWORD         offset;
    };
#pragma pack()
    
    bool WriteMemory(void * loc, void * buffer, int size)
    {
        memcpy(loc, buffer, size);
        return true;
        /*HANDLE hProcess = GetCurrentProcess();
        
        //change the protection of pages containing range of memory 
        //[loc, loc+size] to READ WRITE
        DWORD old_protection;
        
        BOOL ret;
        ret = VirtualProtectEx(hProcess, loc, size, 
                         PAGE_READWRITE, &old_protection);
        if(ret == FALSE)
            return false;

        ret = WriteProcessMemory(hProcess, loc, buffer, size, NULL);
       
        //restore old protection
        DWORD o2;
        VirtualProtectEx(hProcess, loc, size, old_protection, &o2);*/

		//return (ret == TRUE);
    }

    bool ReadMemory(void *loc, void *buffer, DWORD size)
    {
        memcpy(buffer, loc, size);
        return true;
        /*HANDLE hProcess = GetCurrentProcess();
        DWORD bytes_read = 0;
        BOOL ret;
        ret = ReadProcessMemory(hProcess, loc, buffer, size, &bytes_read);
        return (ret == TRUE && bytes_read == size);*/
    }

    bool install_my_handler()
    {
        void * my_hdlr = my_exc_handler;
        void * cpp_hdlr = __CxxFrameHandler;

        jmp_instr jmp_my_hdlr; 
        jmp_my_hdlr.jmp = 0xE9;
        //We actually calculate the offset from __CxxFrameHandler+5
        //as the jmp instruction is 5 byte length.
        jmp_my_hdlr.offset = reinterpret_cast<char*>(my_hdlr) - 
                    (reinterpret_cast<char*>(cpp_hdlr) + 5);
        
        if(!saved_handler_instructions)
        {
            if(!ReadMemory(cpp_hdlr, cpp_handler_instructions,
                        sizeof(cpp_handler_instructions)))
                return false;
            saved_handler_instructions = true;
        }

        return WriteMemory(cpp_hdlr, &jmp_my_hdlr, sizeof(jmp_my_hdlr));
    }

    bool restore_cpp_handler()
    {
        if(!saved_handler_instructions)
            return false;
        else
        {
            void *loc = __CxxFrameHandler;
            return WriteMemory(loc, cpp_handler_instructions, 
                           sizeof(cpp_handler_instructions));
        }
    }
}
