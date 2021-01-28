#include "funcinfo.h"
#include "sehhelper.h"

namespace my_handler
{
    //---------------------------------------------------------------
    // Destroy local objects of a block or a function. 
    //---------------------------------------------------------------
    void local_unwind(
        const unwind *punwind_table,
        EXCEPTION_REGISTRATION *pFrame,
        const int last_id,
        const int new_id
        ) throw()
    {
        //EBP of the function whose stack is being unwound
        DWORD _ebp = reinterpret_cast<DWORD>(&pFrame->ebp);
        destroy_local_objects(_ebp, punwind_table, pFrame->id, last_id);
        pFrame->id = new_id;
    }

    void destroy_local_objects(
        const DWORD _ebp,
        const unwind *punwind_table,
        const int cur_id, 
        const int last_id
        ) throw()
    {
        INSTALL_TERMINATION_HANDLER
        _destroy_local_objects(_ebp, punwind_table, cur_id, last_id);
        REMOVE_TERMINATION_HANDLER
    }

    //--------------------------------------------------------------------
    // _ebp    - Contains the value of the stack frame pointer (EBP) for 
    //           the function whose local objects are being destroyed.
    // cur_id  - The current id on the stack frame
    // last_id - When the prev field of the unwind linked list equals 
    //           last_id, it should not inspect the prev unwind structure
    //           for destroying the object and should return. 
    //--------------------------------------------------------------------
    void _destroy_local_objects(
        const DWORD _ebp,
        const unwind *punwind_table, 
        const int cur_id, 
        const int last_id
        ) throw()
    {
        int id = cur_id;
        while(id != last_id && id>=0)
        {
            CLEANUP_FUNC cf = punwind_table[id].cf;
            if(cf != 0) 
            {
                _asm
                {
                    mov EAX, cf
                    push EBP       //save current EBP
                    mov EBP, _ebp  //the EBP of the unwinding function
                    call EAX
                    pop EBP
                }
            }
            id = punwind_table[id].prev;
        }
    }
}