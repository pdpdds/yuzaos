#pragma once
extern"C" {  
#include <yuzaos.h>
};  
typedef const type_info TypeDescriptor;  
struct PMD  
{  
    ptrdiff_t mdisp; //vftable offset
    ptrdiff_t pdisp; //vftable offset
    ptrdiff_t vdisp; //vftable offset(for virtual base class)
};  
typedef const struct _s_RTTIBaseClassDescriptor    
{  
    TypeDescriptor                  *pTypeDescriptor;  
    DWORD                           numContainedBases;  
    PMD                             where;  
    DWORD                           attributes;  
} _RTTIBaseClassDescriptor;  
typedef const struct  _s_RTTIBaseClassArray     
{  
    _RTTIBaseClassDescriptor* arrayOfBaseClassDescriptors[3];  
}_RTTIBaseClassArray;  
typedef const struct _s_RTTIClassHierarchyDescriptor   
{  
    DWORD                           signature;  
    DWORD                           attributes;  
    DWORD                           numBaseClasses;  
    _RTTIBaseClassArray             *pBaseClassArray;  
}_RTTIClassHierarchyDescriptor;  
typedef const struct _s_RTTICompleteObjectLocator      
{  
    DWORD                           signature;  
    DWORD                           offset;          //Offset of vftbl relative to this
    DWORD                           cdOffset;        //constructor displacement 
    TypeDescriptor                  *pTypeDescriptor;  
    _RTTIClassHierarchyDescriptor   *pClassDescriptor;  
}_RTTICompleteObjectLocator;  
#define BCD_NOTVISIBLE              0x00000001
#define BCD_AMBIGUOUS               0x00000002
#define BCD_PRIVORPROTINCOMPOBJ     0x00000004
#define BCD_PRIVORPROTBASE          0x00000008
#define BCD_VBOFCONTOBJ             0x00000010
#define BCD_NONPOLYMORPHIC          0x00000020
#define BCD_PTD(bcd)                ((bcd).pTypeDescriptor)
#define BCD_NUMCONTBASES(bcd)       ((bcd).numContainedBases)
#define BCD_WHERE(bcd)              ((bcd).where)
#define BCD_ATTRIBUTES(bcd)         ((bcd).attributes)
#define CHD_MULTINH 0x00000001//Multiple inheritance
#define CHD_VIRTINH 0x00000002//Virtual inheritance
#define CHD_AMBIGUOUS 0x00000004//Multiple inheritance with repeated base classes
#define CHD_SIGNATURE(chd)          ((chd).signature)
#define CHD_ATTRIBUTES(chd)         ((chd).attributes)
#define CHD_NUMBASES(chd)           ((chd).numBaseClasses)
#define CHD_PBCA(chd)               ((chd).pBaseClassArray)
#define COL_SIGNATURE(col)          ((col).signature)
#define COL_OFFSET(col)             ((col).offset)
#define COL_CDOFFSET(col)           ((col).cdOffset)
#define COL_PTD(col)                ((col).pTypeDescriptor)
#define COL_PCHD(col)               ((col).pClassDescriptor)
//extern"C" PVOID __cdecl __RTDynamicCast (PVOID, LONG, PVOID, PVOID, BOOL);  
//extern"C" PVOID __cdecl __RTtypeid (PVOID);     // ptr to vfptr
#define TYPEIDS_EQ(pID1, pID2)  ((pID1 == pID2) || !strcmp(pID1->name(), pID2->name()))
