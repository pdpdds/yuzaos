#ifndef _INCLUDED_EXCPT_INFO_H_
#define _INCLUDED_EXCPT_INFO_H_

#ifndef _INCLUDED_TYPEINFO_
#define _INCLUDED_TYPEINFO_
#include "typeinfo.h"
#endif

#ifndef _INCLUDED_VECTOR_
#define _INCLUDED_VECTOR_
#include <vector>
#endif

#ifndef _INCLUDED_WINDOWS_H_
#define _INCLUDED_WINDOWS_H_
#include <minwindef.h>
#endif

namespace my_handler
{
    //All 'u..' fields of the structures below may be reserved
    //I am not aware of their intent. They are not used by the 
    //handler.

    struct etype_info;    //extended type_info
    struct etypeinfo_table;

    struct excpt_info
    {
        DWORD   u1;
        DWORD   destructor;
        DWORD   u2;
        etypeinfo_table *ptt;
    };

    struct etypeinfo_table
    {
        DWORD       count; 
        etype_info   *arr[1];
    };

    struct etype_info
    {
        DWORD           u1;
        type_info *ti; 
        DWORD           u2[3];
        DWORD           size;
        DWORD           copy_constructor;

        bool equal(const etype_info* t)
        {
            if(t->ti == 0 || ti == 0)
                return (t->ti == ti) ? true: false;
            //VC++ 6.0 returns type int and not bool
           // return (*t->ti == *ti) ? true : false;
        }
    };

    struct catchblock;
    struct EXCEPTION_REGISTRATION;

    typedef std::vector<const etype_info*> TYPEINFOVEC;
    extern const etype_info g_catch_all_ti; //etype_info for catch(...)

    class exception_helper
    {
    public:
        static void get_all_etypeinfos(
            const excpt_info*, 
            std::vector<const etype_info*>*
            ) throw();

        //destroy the exception 
        static void destroy(
            void *pexc,
            const excpt_info* pexc_info
            ) throw();

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
        static void pass_exc_to_catch_block(
            void             *pexc,           
            const etype_info   *ti,   
            EXCEPTION_REGISTRATION *establisher_frame,
            const catchblock *pcatch_block 
            ) throw();
    
    private:
        static void copy_exc(
            void *pexc, 
            void *ploc,
            const etype_info* ti
            ) throw();
        
        static void copy_reference(
            void *pexc, 
            void *ploc
            ) throw();
    };
}

#endif