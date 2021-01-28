#include "sehhelper.h"
#include "funcinfo.h"
#include "excpt_helper.h"
#include "excptstorage.h"

void terminate()
{
    
}

namespace my_handler
{
    __declspec (naked) 
    EXCEPTION_DISPOSITION  my_exc_handler(
        _EXCEPTION_RECORD *ExceptionRecord,
        void * EstablisherFrame,
        struct _CONTEXT *ContextRecord,
        void * DispatcherContext
	 ) throw()
    {
        _asm
        {
            push EBP         //prolog
            mov EBP, ESP

            push DispatcherContext
            push ContextRecord
            push EstablisherFrame
            push ExceptionRecord
            push EAX
            call _my_exc_handler

            mov ESP, EBP     //epilog
            pop EBP
            ret
        }
    }
    
    EXCEPTION_DISPOSITION  _my_exc_handler(
         const funcinfo * pfuncinfo,
		 _EXCEPTION_RECORD *ExceptionRecord,
		 void * EstablisherFrame,
		 struct _CONTEXT *ContextRecord,
		 void * DispatcherContext
		 ) throw()
    {
        EXCEPTION_REGISTRATION *pFrame;
        pFrame = reinterpret_cast<EXCEPTION_REGISTRATION*>(EstablisherFrame);
        
        if(!(ExceptionRecord->ExceptionFlags & (  
              _EXCEPTION_UNWINDING | _EXCEPTION_EXIT_UNWIND)))
        {
            const tryblock   *ptryblock = 0;
            const catchblock *pcatchblock = 0;
            const etype_info   *ptypeinfo = 0;  //exception typeinfo
            if(any_catch_block(pfuncinfo, ExceptionRecord, pFrame->id, 
                                &ptryblock, &pcatchblock, &ptypeinfo))
            {
                void *pexc = 0;
                get_exception(ExceptionRecord, &pexc);

                //catch block uses the stack frame of the function in which
                //it resides. copy the exception in that stack frame
                exception_helper::pass_exc_to_catch_block(pexc, 
                                      ptypeinfo, pFrame, pcatchblock);
                //perform stack unwinding
                global_unwind(pFrame, ExceptionRecord);
                do_local_unwind(pfuncinfo, pFrame, ptryblock);

                DWORD addr = call_catch_block(pcatchblock, ExceptionRecord,
                                                pFrame);
                jump_to_continuation(addr, pFrame);  //never returns
            }
        }
        else //stack is unwinding, clean up the frame
        {
            using namespace std;
            const unwind *punwind_table;
            int count;
            if(get_unwind_table(pfuncinfo, &punwind_table, &count))
            {
                if(pFrame->id >= count) //stack corruption
                    terminate();
                local_unwind(punwind_table, pFrame, -1, -1);
            }
        }
        return ExceptionContinueSearch;
    }

    //--------------------------------------------------------------------
    // Finds if there is any catch block that is interested in handling 
    // the exception.
    //--------------------------------------------------------------------
    bool any_catch_block(
          const funcinfo *pfuncinfo,
          const _EXCEPTION_RECORD *ExceptionRecord,
          const int   id, 
          const tryblock **pptryblock,
          const catchblock **ppcatchblock,
          const etype_info   **pptypeinfo
        ) throw()
    {
        INSTALL_TERMINATION_HANDLER
        bool found = false;
        if(id_in_any_try_block(pfuncinfo, id))
        {
            TYPEINFOVEC *exc_types = new TYPEINFOVEC();
            exc_types->reserve(5);
            const excpt_info *pexcpt_info;
            if(get_excpt_info(ExceptionRecord, &pexcpt_info))
                exception_helper::get_all_etypeinfos(pexcpt_info, exc_types);
            exc_types->push_back(&g_catch_all_ti);
            TYPEINFOVEC::iterator iter;
            found = find_catch_block(pfuncinfo, *exc_types, id, 
                                           pptryblock, ppcatchblock, &iter);
            if(found)
                *pptypeinfo = *iter;
            delete exc_types;
        }
        REMOVE_TERMINATION_HANDLER
        return found;
    }

    //---------------------------------------------------------------------
    // Call the catch block. The return type is the address where control
    // should be transferred after catch block returns. After calling the 
    // catch block, it destroys the exception.
    //---------------------------------------------------------------------
    DWORD call_catch_block(
        const catchblock *pcatch_block,
        _EXCEPTION_RECORD *pexc_rec,
        EXCEPTION_REGISTRATION  *preg_frame
        ) throw()
    {
        void *pexc = 0;
        get_exception(pexc_rec, &pexc);
        const excpt_info *pexc_info = 0;
        get_excpt_info(pexc_rec, &pexc_info);

        DWORD address = pcatch_block->catchblock_addr;
        DWORD retaddr;
        DWORD catch_ebp = reinterpret_cast<DWORD>(   //EBP of the stack frame 
                                  &preg_frame->ebp); //that catch block uses.
        
        //get thread specific exception_storage
        exception_storage *p = get_exception_storage();
        p->set(pexc, pexc_info);  //save current exception info.

        INSTALL_CATCH_PROTECTOR
        _asm
        {
            mov EAX, address
            pushad       //save all registers. with optimizations on, 
                         //catch block does not preserve esi, edi registers.
            push EBP
            mov EBP, catch_ebp
            call EAX     //The return type of calling the catch block is 
                         //the address where control should be transferred
            pop EBP
            mov retaddr, EAX 
            
            popad
        }

        REMOVE_CATCH_PROTECTOR
        exception_helper::destroy(pexc, pexc_info);
        return retaddr;
    }

    EXCEPTION_DISPOSITION  terminate_handler(
		 _EXCEPTION_RECORD *ExceptionRecord,
		 void * EstablisherFrame,
		 struct _CONTEXT *ContextRecord,
		 void * DispatcherContext
		 ) throw()
    {
        using namespace std;
        terminate(); //in VC++ 6.0, this function is not in std.
        return ExceptionContinueSearch;
    }

    //-------------------------------------------------------------------
    // If this handler is calles, exception was (re)thrown from catch 
    // block. The  exception  handler  (my_handler)  registers this 
    // handler before calling the catch block. Its job is to determine
    // if the  catch block  threw  new  exception or did a rethrow. If 
    // catch block threw a  new  exception, then it should destroy the 
    // previous exception object that was passed to the catch block. If 
    // the catch block did a rethrow, then this handler should retrieve
    // the original exception and save in ExceptionRecord for the 
    // exception handlers to use it.
    //-------------------------------------------------------------------
    EXCEPTION_DISPOSITION  catch_block_protector(
		 _EXCEPTION_RECORD *ExceptionRecord,
		 void * EstablisherFrame,
		 struct _CONTEXT *ContextRecord,
		 void * DispatcherContext
		 ) throw()
    {
        EXCEPTION_REGISTRATION *pFrame;
        pFrame = reinterpret_cast<EXCEPTION_REGISTRATION*>(EstablisherFrame);
        
        if(!(ExceptionRecord->ExceptionFlags & (  
              _EXCEPTION_UNWINDING | _EXCEPTION_EXIT_UNWIND)))
        {
            void *pcur_exc = 0, *pprev_exc = 0;
            const excpt_info *pexc_info = 0, *pprev_excinfo = 0;
            exception_storage *p = get_exception_storage();
            pprev_exc = p->get_exception();
            pprev_excinfo = p->get_exception_info();
            p->set(0, 0);
            bool cpp_exc = ExceptionRecord->ExceptionCode == MS_CPP_EXC;
            get_exception(ExceptionRecord, &pcur_exc);
            get_excpt_info(ExceptionRecord, &pexc_info);
            if(cpp_exc && 0 == pcur_exc && 0 == pexc_info)  //rethrow
            {
                ExceptionRecord->ExceptionInformation[1] = 
                    reinterpret_cast<DWORD>(pprev_exc);
                ExceptionRecord->ExceptionInformation[2] = 
                    reinterpret_cast<DWORD>(pprev_excinfo);
            }
            else
            {
                exception_helper::destroy(pprev_exc, pprev_excinfo);
            }
        }
        return ExceptionContinueSearch;
    }

    void global_unwind(const EXCEPTION_REGISTRATION *pFrame, 
        const _EXCEPTION_RECORD *ExceptionRecord) throw()
    {
        using namespace std;
        void *stack_base = 0, *stack_top = 0;
        get_stack_limits(&stack_base, &stack_top);

        _EXCEPTION_RECORD excpt_rec, *pexcpt_rec;
        pexcpt_rec = &excpt_rec;
        excpt_rec.ExceptionCode = ExceptionRecord->ExceptionCode;
        excpt_rec.ExceptionFlags = _EXCEPTION_UNWINDING;
        excpt_rec.ExceptionAddress = 0;
        excpt_rec.ExceptionRecord = 0;
        excpt_rec.NumberParameters = ExceptionRecord->NumberParameters;

        for(int i = 0; i<ExceptionRecord->NumberParameters; ++i)
        {
            excpt_rec.ExceptionInformation[i] = 
                    ExceptionRecord->ExceptionInformation[i];
        }
        _CONTEXT context;
        _CONTEXT *pcontext = &context;
        capture_context(pcontext);

        EXCEPTION_REGISTRATION *pcur_frame = get_registration_head();
        
        INSTALL_TERMINATION_HANDLER
        int saved_esp;
        _asm mov saved_esp, ESP

        while (pcur_frame != pFrame)
        {
            //check for possible stack corruption
            if(pcur_frame < stack_base || 
               pcur_frame + sizeof(pcur_frame) > stack_top)
                    terminate(); //not in std in VC++ 6.0
            
            //prev node of the EXCEPTION_REGISTRATION list is always
            //higher on stack (as stack grows downwards)
            if(pcur_frame > pFrame || (
                pcur_frame->prev != reinterpret_cast<void*>(-1) //end of list
                && pcur_frame > pcur_frame->prev))
                    terminate();
            
            DWORD handler = pcur_frame->handler;
            _asm
            {
                push 0x00000000
                push pcontext
                push pcur_frame
                push pexcpt_rec
                //Note that after "call handler" below, there is no
                //instruction "add ESP, 10h" even though we have
                //declared the handler function to have __cdecl
                //calling convention. This is due to the fact that
                //the system installs its own handlers that
                //have __stdcall calling convention. So we set
                //the ESP right after exiting from this loop.
                call handler
            }
            pcur_frame = pcur_frame->prev;
        }
        _asm mov ESP, saved_esp
        REMOVE_TERMINATION_HANDLER
        set_registration_head(pFrame);        
    }

    //---------------------------------------------------------------------
    // Destroy the local objects of the try block.
    //---------------------------------------------------------------------
    void do_local_unwind(
        const funcinfo *pfuncinfo, 
        EXCEPTION_REGISTRATION *pFrame, 
        const tryblock *ptryblock) throw()
    {
        using namespace std;
        const unwind *punwind_table;
        int count;
        if(get_unwind_table(pfuncinfo, &punwind_table, &count))
        {
            int last_id = punwind_table[ptryblock->start_id].prev;
            if(pFrame->id >= count) //stack corruption
                terminate();
            local_unwind(punwind_table, pFrame, last_id, ptryblock->end_id+1);
        }
    }

    //----------------------------------------------------------------------
    // This function is called after calling the catch block to transfer 
    // control to resume normal execution. It never returns.
    //----------------------------------------------------------------------
    void jump_to_continuation(
        DWORD addr,
        EXCEPTION_REGISTRATION *pexc_reg
        ) throw()
    {
        DWORD _ebp = reinterpret_cast<DWORD>(&pexc_reg->ebp);
        _asm
        {
            mov EAX, addr
            mov EBP, _ebp
            mov ESP, [EBP-10h]
            jmp EAX
        }
    }
    exception_storage* get_exception_storage() throw()
    {
        static exception_storage* p = 0;

        if (p == 0)
            p = new exception_storage();

        return p;
    }
}
