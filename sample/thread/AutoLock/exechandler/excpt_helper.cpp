#include "excpt_helper.h"
#include "sehhelper.h"
#include "funcinfo.h"

namespace my_handler
{
    //etype_info for catch(...)
    const etype_info g_catch_all_ti = {0,0,0,0,0,0,0};
    
    void exception_helper::get_all_etypeinfos(const excpt_info* pei,
            std::vector<const etype_info*>* ptiv
            ) throw()
	{
		return;
		/*if(0 == pei)
            return;
        etypeinfo_table *t = pei->ptt;
        ptiv->assign(t->arr, t->arr + t->count);*/
	}

	void exception_helper::destroy(
			  void *pexc,
			  const excpt_info* pei
			  ) throw()
	{
		if(pexc != 0 && pei != 0)
		{
			INSTALL_TERMINATION_HANDLER
            DWORD destructor = pei->destructor;
            if(destructor != 0)
            {                
			    _asm
			    {
				    push ecx
				    mov ecx, pexc  // this pointer
				    call destructor
				    pop ecx
			    }
            }
			REMOVE_TERMINATION_HANDLER
		}
	}

    //---------------------------------------------------------------
	//pass exception to the catchblock by value or reference depending 
    //upon the information contained in catch_block structure. 
    //  pexc  - pointer to the exception to pass to the catch block
    //  ti    - contains exception's copy constructor or size
    //  establisher_frame - stack frame where exception should be 
    //          copied. It is the stack frame of the function in 
    //          which catch block resides.
    //  pcatchblock - tells (offset field) where to copy the exception
    //          in the frame
	//---------------------------------------------------------------
    
	void exception_helper::pass_exc_to_catch_block(
            void                   *pexc,           
            const etype_info       *ti,
            EXCEPTION_REGISTRATION *establisher_frame,
            const catchblock       *pcb
            ) throw()
	{
        if(pexc && pcb->offset)
        {
            INSTALL_TERMINATION_HANDLER
            void *ploc = reinterpret_cast<void*>(
                            reinterpret_cast<DWORD>(&establisher_frame->ebp) +
                                            pcb->offset);
            if(BY_VALUE == pcb->val_or_ref)
                copy_exc(pexc, ploc, ti);
            else if(BY_REFERENCE == pcb->val_or_ref)
                copy_reference(pexc, ploc);
    		REMOVE_TERMINATION_HANDLER
        }
	}

	void exception_helper::copy_exc(void *pexc, void *ploc,
                                        const etype_info* ti) throw()
	{
		if(ti->copy_constructor)
		{
			DWORD copy_cnstr = ti->copy_constructor;
            _asm
			{
				push ecx
				mov  ecx, ploc   //this pointer
				push pexc        //copy constructor argument
				call copy_cnstr
				pop ecx
			}
		}
		else if(ti->size)
			memcpy(ploc, pexc, ti->size);
	}
    
    void exception_helper::copy_reference(void *pexc, void *ploc) throw()
	{
		_asm
		{
			push eax   //save
            push ebx   //save

			//*ploc = pexc;
            mov eax, ploc
            mov ebx, pexc
			mov [eax], ebx

            pop ebx
			pop eax
		}
	}
} 
