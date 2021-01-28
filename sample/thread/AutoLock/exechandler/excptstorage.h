#ifndef _INCLUDED_EXCEPTION_STORAGE_H_
#define _INCLUDED_EXCEPTION_STORAGE_H_

namespace my_handler
{
    struct excpt_info;
    
    class exception_storage
    {
    public:
        exception_storage() throw()
        {
            set(0, 0);
        }

        void set(void *pexc, const excpt_info *pexc_info) throw()
        {
            m_pexc = pexc;
            m_pinfo = pexc_info;
        }

        const excpt_info * get_exception_info() throw()
        {
            return m_pinfo;
        }

        void * get_exception() throw()
        {
            return m_pexc;
        }
    
    private:
        const excpt_info *m_pinfo;
        void *m_pexc;
    };
}

#endif