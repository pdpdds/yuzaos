#ifndef _INCLUDED_FUNCINFO_H_
#define _INCLUDED_FUNCINFO_H_

#ifndef _INCLUDED_EXCPT_HELPER_H_
#define _INCLUDED_EXCPT_HELPER_H_
#include "excpt_helper.h"
#endif

#ifndef _INCLUDED_WINDOWS_H_
#define _INCLUDED_WINDOWS_H_
#include <windows.h>
#endif

#ifndef _INCLUDED_ALGORITHM_
#define _INCLUDED_ALGORITHM_
#include <algorithm>
#endif

#ifndef _INCLUDED_FUNCTIONAL_
#define _INCLUDED_FUNCTIONAL_
#include <functional>
#endif

namespace my_handler
{
    typedef void (*CLEANUP_FUNC)();

    struct unwind
    {
        DWORD        prev;
        CLEANUP_FUNC cf;
    };

    struct tryblock
    {
        DWORD		start_id;
        DWORD		end_id;
        DWORD		u1;
        DWORD		catchblock_count;
        catchblock *pcatchblock_table;
    };

	enum VAL_OR_REF  {BY_VALUE = 0, BY_REFERENCE = 8};
	
    struct catchblock
    {
        DWORD      val_or_ref; //Whether the exception is passed
                               //by val or by reference
        type_info *ti;         
        int        offset;     //offset from stack frame pointer
                               //(EBP) where exception should be
                               //copied.
        DWORD      catchblock_addr;
    };

    struct funcinfo
    {
        DWORD	  sig;
        DWORD	  unwind_count;   //number of entries in unwindtable
        unwind   *punwindtable;
        DWORD     tryblock_count; //number of entries in tryblocktable
        tryblock *ptryblock_table;
    };


    inline bool get_tryblock_table(
        const funcinfo *pfuncinfo,
        const tryblock **pptryblock,
        int       *pcount  //number of entries in the tryblock table
        ) throw()
    {
        *pptryblock = pfuncinfo->ptryblock_table;
        *pcount = pfuncinfo->tryblock_count;
        return (*pcount!=0);
    }

    inline bool get_unwind_table(
        const funcinfo *pfuncinfo,
        const unwind **ppunwindtable,
        int         *pcount  //number of entries in the unwind table
        ) throw()
    {
        *ppunwindtable = pfuncinfo->punwindtable;
        *pcount = pfuncinfo->unwind_count;
        return (*pcount!=0);
    }

    //catchblock predicate. tells if the type of the catch block matches
    //the type of the exception or any of its public base classes.
    class catchblock_eq : public std::unary_function<const catchblock&, bool>
    {
    public:
        catchblock_eq(TYPEINFOVEC &et, const catchblock **ppc, 
            TYPEINFOVEC::iterator *piter) throw() : 
        m_ppcatchblock(ppc), m_exc_types(et), m_piter(piter)
        {
            memset(&m_catch_ti, 0, sizeof(etype_info));
        }
        
        bool operator()(const catchblock& cb) throw()
        {
            m_catch_ti.ti = cb.ti;
            *m_ppcatchblock = &cb;

            //VC++ 5.0 header <functional> has a bug (might have been fixed 
            //in service packs), which will give compilation error. There could
            //be two workarounds:
            // a) add 'const' qualfier to mem_fun1_t::operator() in <functional>
            // b) replace your functional header by the one provided by VC++6.0

            *m_piter = std::find_if(m_exc_types.begin(), m_exc_types.end(),
                std::bind1st(std::mem_fun(&etype_info::equal), &m_catch_ti));
            return (*m_piter!=m_exc_types.end());
        }

    private:
        const catchblock **m_ppcatchblock;
        TYPEINFOVEC & m_exc_types;        
        TYPEINFOVEC::iterator *m_piter;
        etype_info m_catch_ti;
    };

    //find a catch block from its list of catch blocks
    class tryblock_find : public std::unary_function<const tryblock*, bool>
    {
    public:
        tryblock_find(TYPEINFOVEC & et, const catchblock **ppc, 
            TYPEINFOVEC::iterator *piter) throw() : 
        m_ppcatchblock(ppc), m_exc_types(et), m_piter(piter) 
        {
        }

        bool operator()(const tryblock* tb) throw()
        {
            const catchblock *beg = tb->pcatchblock_table;
            const int count = tb->catchblock_count;
            const catchblock *p, *end = beg + count;
            p = std::find_if(beg, end, catchblock_eq(m_exc_types, 
                m_ppcatchblock, m_piter));
            return (p != end);
        }

    private:
        const catchblock **m_ppcatchblock;
        TYPEINFOVEC & m_exc_types;        
        TYPEINFOVEC::iterator *m_piter;
    };

    //--------------------------------------------------------------------
    // Finds if there is any catch block in the function that is interested 
    // in handling the exception.
    //---------------------------------------------------------------------
    bool find_catch_block(
        const funcinfo *pfuncinfo,
        TYPEINFOVEC& exc_types,
        const int   id,
        const tryblock **pptryblock,
        const catchblock **ppcatchblock,
        TYPEINFOVEC::iterator *piter
    ) throw();

    //---------------------------------------------------------------------
    // Get pointer to all the try blocks to which the current id belongs.
    // In case of nested try blocks, the id may be in multiple try blocks.
    //---------------------------------------------------------------------
    void get_try_block(
        const funcinfo *pfuncinfo,
        int id,
        /*out*/std::vector<const tryblock*>& vec
        ) throw();

    bool id_in_any_try_block(const funcinfo *pfuncinfo, int id) throw();

    void _destroy_local_objects(
        const DWORD _ebp,
        const unwind *punwind_table, 
        const int cur_id, 
        const int last_id
        ) throw();

    void destroy_local_objects(
        DWORD _ebp,
        const unwind *punwind_table, 
        const int cur_id, 
        const int last_id
        ) throw();

    void local_unwind(
        const unwind *puwind_table,
        EXCEPTION_REGISTRATION *pFrame,
        const int last_id,
        const int new_id
        ) throw();
}

#endif