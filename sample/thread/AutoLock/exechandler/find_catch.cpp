#include "funcinfo.h"

namespace my_handler
{
    //--------------------------------------------------------------------
    // Finds if there is any catch block in the function that is interested 
    // in handling the exception.
    //---------------------------------------------------------------------
    bool find_catch_block(
            const funcinfo *pfuncinfo,
            TYPEINFOVEC& exc_types,
            int   id,
            const tryblock **pptryblock,
            const catchblock **ppcatchblock,
            TYPEINFOVEC::iterator *piter
        ) throw()
    {
        typedef std::vector<const tryblock*> TRYVEC;
        TRYVEC vec;
        vec.reserve(5);
        get_try_block(pfuncinfo, id, vec);
        TRYVEC::iterator p;
        TRYVEC::iterator end = vec.end();
        p = std::find_if(vec.begin(), end, tryblock_find(
                              exc_types, ppcatchblock, piter));
        if(p != end)
            *pptryblock = *p;
        return (p != end);
    }

    //---------------------------------------------------------------------
    // Get pointer to all the try blocks to which the current id belongs.
    // In case of nested try blocks, the id may be in multiple try blocks.
    //---------------------------------------------------------------------
    void get_try_block(
        const funcinfo *pfuncinfo,
        int id,
        /*out*/std::vector<const tryblock*>& vec
        ) throw()
    {
        int count;
        const tryblock *ptryblock = 0;
        get_tryblock_table(pfuncinfo, &ptryblock, &count);
        for(int i = 0; i<count; ++i)
        {
            if(id>=ptryblock[i].start_id && id<=ptryblock[i].end_id)
                vec.push_back(&ptryblock[i]);
        }
    }
    
    //-----------------------------------------------------------------
    //does id have enclosing try block?
    //-----------------------------------------------------------------
    bool id_in_any_try_block(const funcinfo *pfuncinfo, int id) throw()
    {
        int count;
        const tryblock *ptryblock = 0;
        get_tryblock_table(pfuncinfo, &ptryblock, &count);
        for(int i = 0; i<count; ++i)
        {
            if(id>=ptryblock[i].start_id && id<=ptryblock[i].end_id)
                return true;
        }
        return false;
    }
}