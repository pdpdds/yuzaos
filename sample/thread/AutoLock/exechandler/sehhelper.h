#pragma once
#include "excpt.h"


#define INSTALL_EXC_HANDLER(handler)  _asm { \
	_asm push handler     \
	_asm push FS:[0]      \
	_asm mov  FS:[0], ESP \
}

//side effect:  contents of EAX register are over written
#define REMOVE_EXC_HANDLER  _asm { \
	_asm mov EAX, [ESP]  \
    _asm mov FS:[0], EAX \
	_asm add ESP, 8      \
}

#define INSTALL_TERMINATION_HANDLER { \
    void *handler = my_handler::terminate_handler; \
    INSTALL_EXC_HANDLER(handler)  \
   }

#define REMOVE_TERMINATION_HANDLER  REMOVE_EXC_HANDLER

#define INSTALL_CATCH_PROTECTOR { \
    void *handler = my_handler::catch_block_protector; \
    INSTALL_EXC_HANDLER(handler)  \
   }

#define REMOVE_CATCH_PROTECTOR REMOVE_EXC_HANDLER

namespace my_handler
{
    const DWORD MS_CPP_EXC = 0xe06d7363;
    
    struct EXCEPTION_REGISTRATION 
    { 
        EXCEPTION_REGISTRATION *prev; 
        DWORD                   handler; 
        DWORD                   id; 
        DWORD                   ebp; 
    };

    inline EXCEPTION_REGISTRATION * get_registration_head() throw()
    {
        EXCEPTION_REGISTRATION *p;
        _asm
        {
            mov EAX, FS:[0]
            mov p, EAX
        }
        return p;
    }

    inline void set_registration_head(const EXCEPTION_REGISTRATION *p) throw()
    {
        _asm
        {
            mov EAX, p
            mov FS:[0], EAX
        }
    }

    inline void get_stack_limits(void **pbase, void **ptop) throw()
    {
        void *base, *top;
        _asm
        {
            mov EAX, FS:[4]
            mov top, EAX
            mov EAX, FS:[8]
            mov base, EAX
        }
        *pbase = base;
        *ptop = top;
    }

    struct excpt_info;
    struct funcinfo;
    struct tryblock;
    struct catchblock;
    struct etype_info;

    EXCEPTION_DISPOSITION my_exc_handler(
		 _EXCEPTION_RECORD *ExceptionRecord,
		 void * EstablisherFrame,
		 struct _CONTEXT *ContextRecord,
		 void * DispatcherContext
		 ) throw();

    EXCEPTION_DISPOSITION _my_exc_handler(
         const funcinfo * pfuncinfo,
	     _EXCEPTION_RECORD *ExceptionRecord,
	     void * EstablisherFrame,
	     struct _CONTEXT *ContextRecord,
	     void * DispatcherContext
	 ) throw();

    EXCEPTION_DISPOSITION terminate_handler(
		 _EXCEPTION_RECORD *ExceptionRecord,
		 void * EstablisherFrame,
		 struct _CONTEXT *ContextRecord,
		 void * DispatcherContext
		 ) throw();

    EXCEPTION_DISPOSITION catch_block_protector(
		 _EXCEPTION_RECORD *ExceptionRecord,
		 void * EstablisherFrame,
		 struct _CONTEXT *ContextRecord,
		 void * DispatcherContext
		 ) throw();

    inline bool get_exception(
        const _EXCEPTION_RECORD *pRec,
        void **ppexc
        ) throw()
    {
        *ppexc = 0;
        if(pRec->ExceptionCode==MS_CPP_EXC && pRec->NumberParameters==3)
            *ppexc = reinterpret_cast<void*>(pRec->ExceptionInformation[1]);
        return (*ppexc != 0);
    }

    inline bool get_excpt_info(
        const _EXCEPTION_RECORD *pRec,
        const excpt_info **ppexcpt_info
        ) throw()
    {
        *ppexcpt_info = 0;
        if(pRec->ExceptionCode==MS_CPP_EXC && pRec->NumberParameters==3)
            *ppexcpt_info = reinterpret_cast<excpt_info*>(
                                    pRec->ExceptionInformation[2]);
        return (*ppexcpt_info != 0);
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
          const etype_info **pptypeinfo
        ) throw();

    //---------------------------------------------------------------------
    // Perform stack unwind. 
    //---------------------------------------------------------------------
    void global_unwind(const EXCEPTION_REGISTRATION *pFrame,
                       const _EXCEPTION_RECORD *ExceptionRecord) throw();

    //-----------------------------------------------------------------------
    // Destroy the local objects of the try block.
    //-----------------------------------------------------------------------
    void do_local_unwind(
        const funcinfo *pfuncinfo, 
        EXCEPTION_REGISTRATION *pFrame, 
        const tryblock *ptryblock) throw();

    
    //---------------------------------------------------------------------
    // Call the catch block. The return type is the address where control
    // should be transferred after catch block returns. After calling the 
    // catch block, it destroys the exception.
    //---------------------------------------------------------------------
    DWORD call_catch_block(
        const catchblock *pcatch_block,
        _EXCEPTION_RECORD *pexc_rec,
        EXCEPTION_REGISTRATION  *preg_frame
        ) throw();

    //----------------------------------------------------------------------
    // This function is called after calling the catch block to transfer 
    // control to resume normal execution. It never returns.
    //----------------------------------------------------------------------
    void jump_to_continuation(
        DWORD addr,
        EXCEPTION_REGISTRATION *pexc_reg
        ) throw();

    class exception_storage;
    exception_storage* get_exception_storage() throw();

    const int _EXCEPTION_UNWINDING   = 2;
    const int _EXCEPTION_EXIT_UNWIND = 4;

    //Not of concern to us really
    inline void capture_context(_CONTEXT* p)
    {
    }
}

