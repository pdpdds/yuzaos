#include <stdio.h>
#include "typeinfo"
#include "rtti.h"
#pragma warning(disable:4297)
static PVOID __cdecl FindCompleteObject(PVOID*);
static _RTTIBaseClassDescriptor* __cdecl FindSITargetTypeInstance(PVOID, _RTTICompleteObjectLocator*, TypeDescriptor*, int, TypeDescriptor*);
static _RTTIBaseClassDescriptor* __cdecl FindMITargetTypeInstance(PVOID, _RTTICompleteObjectLocator*, TypeDescriptor*, int, TypeDescriptor*);
static _RTTIBaseClassDescriptor* __cdecl FindVITargetTypeInstance(PVOID, _RTTICompleteObjectLocator*, TypeDescriptor*, int, TypeDescriptor*);
static ptrdiff_t __cdecl PMDtoOffset(PVOID pThis, const PMD& pmd);
extern"C" PVOID __cdecl __RTtypeid(PVOID inptr) throw(...)
{
    if (!inptr) {
       // throw std::bad_typeid("Attempted a typeid of NULL pointer!");
        return NULL;
    }
  //  __try {
        // Ptr to CompleteObjectLocator should be stored at vfptr[-1]
        _RTTICompleteObjectLocator* pCompleteLocator = (_RTTICompleteObjectLocator*)((*((void***)inptr))[-1]);
        return (PVOID)pCompleteLocator->pTypeDescriptor;
   // }
   // __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
   // {
    //    throw std::__non_rtti_object("Access violation - no RTTI data!");
   // }
}

extern "C" PVOID __RTDynamicCast(
    PVOID inptr,
    LONG VfDelta,
    PVOID SrcType,
    PVOID TargetType,
    BOOL isReference
) throw(...)
{
    PVOID pResult;
    _RTTIBaseClassDescriptor* pBaseClass;
    if (inptr == NULL)
        return NULL;
    //__try {
        PVOID pCompleteObject = FindCompleteObject((PVOID*)inptr);
        _RTTICompleteObjectLocator* pCompleteLocator = (_RTTICompleteObjectLocator*)((*((void***)inptr))[-1]);
        // Adjust by vfptr displacement, if any
        inptr = (PVOID*)((char*)inptr - VfDelta);
        // Calculate offset of source object in complete object
        int inptr_delta = (char*)inptr - (char*)pCompleteObject;
        if (!(CHD_ATTRIBUTES(*COL_PCHD(*pCompleteLocator)) & CHD_MULTINH)) {             // if not multiple inheritance
            pBaseClass = FindSITargetTypeInstance(pCompleteObject,
                pCompleteLocator,
                (TypeDescriptor*)SrcType,
                inptr_delta,
                (TypeDescriptor*)TargetType);
        } else if(!(CHD_ATTRIBUTES(*COL_PCHD(*pCompleteLocator)) & CHD_VIRTINH)) { // if multiple, but not virtual, inheritance
            pBaseClass = FindMITargetTypeInstance(pCompleteObject,
                pCompleteLocator,
                (TypeDescriptor*)SrcType,
                inptr_delta,
                (TypeDescriptor*)TargetType);
        }
        else {                                                                   // if virtual inheritance
            pBaseClass = FindVITargetTypeInstance(pCompleteObject,
                pCompleteLocator,
                (TypeDescriptor*)SrcType,
                inptr_delta,
                (TypeDescriptor*)TargetType);
        }
        if (pBaseClass != NULL) {
            // Calculate ptr to result base class from pBaseClass->where
            pResult = ((char*)pCompleteObject) + PMDtoOffset(pCompleteObject, pBaseClass->where);
        }
        else {
            pResult = NULL;
           // if (isReference) {
           //     throw std::bad_cast("Bad dynamic_cast!");
           // }
        }
    //}
    //__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        //pResult = NULL;
        //throw std::__non_rtti_object("Access violation - no RTTI data!");
    //}
    return pResult;
}
/////////////////////////////////////////////////////////////////////////////
//
// FindCompleteObject - Calculate member offset from PMD & this
//
// Output: pointer to the complete object containing class *inptr
//
// Side-effects: NONE.
//
static PVOID __cdecl FindCompleteObject(PVOID* inptr)          // Pointer to polymorphic object
{
    // Ptr to CompleteObjectLocator should be stored at vfptr[-1]
    _RTTICompleteObjectLocator* pCompleteLocator = (_RTTICompleteObjectLocator*)((*((void***)inptr))[-1]);
    char* pCompleteObject = (char*)inptr - pCompleteLocator->offset;
    // Adjust by construction displacement, if any
    if (pCompleteLocator->cdOffset)
        pCompleteObject += *(ptrdiff_t*)((char*)inptr - pCompleteLocator->cdOffset);
    return (PVOID)pCompleteObject;
}
static _RTTIBaseClassDescriptor* __cdecl FindSITargetTypeInstance(
    PVOID pCompleteObject,                          // pointer to complete object
    _RTTICompleteObjectLocator* pCOLocator, // pointer to Locator of complete object
    TypeDescriptor* pSrcTypeID,        // pointer to TypeDescriptor of source object
    int SrcOffset,                                          // offset of source object in complete object
    TypeDescriptor* pTargetTypeID)     // pointer to TypeDescriptor of result of cast
{
    _RTTIBaseClassDescriptor* pBase;
    _RTTIBaseClassDescriptor* const* pBasePtr;
    DWORD i;
    for (i = 0, pBasePtr = pCOLocator->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors;
        i < pCOLocator->pClassDescriptor->numBaseClasses;
        i++, pBasePtr++) {
        // Test type of selected base class
        pBase = *pBasePtr;
        if (TYPEIDS_EQ(pBase->pTypeDescriptor, pTargetTypeID) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_NOTVISIBLE)) {
            return pBase;
        }
    }
    return NULL;
}
static _RTTIBaseClassDescriptor* __cdecl FindMITargetTypeInstance(
    PVOID pCompleteObject,                          // pointer to complete object
    _RTTICompleteObjectLocator* pCOLocator, // pointer to Locator of complete object
    TypeDescriptor* pSrcTypeID,        // pointer to TypeDescriptor of source object
    int SrcOffset,                                          // offset of source object in complete object
    TypeDescriptor* pTargetTypeID)     // pointer to TypeDescriptor of result of cast
{
    _RTTIBaseClassDescriptor* pBase, * pSubBase;
    _RTTIBaseClassDescriptor* const* pBasePtr, * const* pSubBasePtr;
    DWORD i, j;
    // First, try down-casts
    for (i = 0, pBasePtr = pCOLocator->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors;
        i < pCOLocator->pClassDescriptor->numBaseClasses;
        i++, pBasePtr++) {
        pBase = *pBasePtr;
        // Test type of selected base class
        if (TYPEIDS_EQ(pBase->pTypeDescriptor, pTargetTypeID)) {
            // If base class is proper type, see if it contains our instance of source class
            for (j = 0, pSubBasePtr = pBasePtr + 1;
                j < pBase->numContainedBases;
                j++, pSubBasePtr++) {
                pSubBase = *pSubBasePtr;
                if (TYPEIDS_EQ(pSubBase->pTypeDescriptor, pSrcTypeID) &&
                    (PMDtoOffset(pCompleteObject, pSubBase->where) == SrcOffset)) {
                    // Yes, this is the proper instance of source class
                    return pBase;
                }
            }
        }
    }
    // Down-cast failed, try cross-cast
    for (i = 0, pBasePtr = pCOLocator->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors;
        i < pCOLocator->pClassDescriptor->numBaseClasses;
        i++, pBasePtr++) {
        pBase = *pBasePtr;
        // Check if base class has proper type, is accessible & is unambiguous
        if (TYPEIDS_EQ(pBase->pTypeDescriptor, pTargetTypeID) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_NOTVISIBLE) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_AMBIGUOUS)) {
            return pBase;
        }
    }
    return NULL;
}
static _RTTIBaseClassDescriptor* __cdecl FindVITargetTypeInstance(
    PVOID pCompleteObject,                          // pointer to complete object
    _RTTICompleteObjectLocator* pCOLocator, // pointer to Locator of complete object
    TypeDescriptor* pSrcTypeID,        // pointer to TypeDescriptor of source object
    int SrcOffset,                                          // offset of source object in complete object
    TypeDescriptor* pTargetTypeID)     // pointer to TypeDescriptor of result of cast
{
    _RTTIBaseClassDescriptor* pBase, * pSubBase;
    _RTTIBaseClassDescriptor* const* pBasePtr, * const* pSubBasePtr;
    _RTTIBaseClassDescriptor* pResult = NULL;
    DWORD i, j;
    // First, try down-casts
    for (i = 0, pBasePtr = pCOLocator->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors;
        i < pCOLocator->pClassDescriptor->numBaseClasses;
        i++, pBasePtr++) {
        pBase = *pBasePtr;
        // Test type of selected base class
        if (TYPEIDS_EQ(pBase->pTypeDescriptor, pTargetTypeID)) {
            // If base class is proper type, see if it contains our instance of source class
            for (j = 0, pSubBasePtr = pBasePtr + 1;
                j < pBase->numContainedBases;
                j++, pSubBasePtr++) {
                pSubBase = *pSubBasePtr;
                if (TYPEIDS_EQ(pSubBase->pTypeDescriptor, pSrcTypeID) &&
                    (PMDtoOffset(pCompleteObject, pSubBase->where) == SrcOffset)) {
                    // Yes, this is the proper instance of source class - make sure it is unambiguous
                    // Ambiguity now determined by inequality of offsets of source class within complete object, not pointer inequality
                    if ((pResult != NULL) && (PMDtoOffset(pCompleteObject, pResult->where) != PMDtoOffset(pCompleteObject, pBase->where))) {
                        // We already found an earlier instance, hence ambiguity
                        return NULL;
                    }
                    else {
                        // Unambiguous
                        pResult = pBase;
                    }
                }
            }
        }
    }
    if (pResult != NULL)
        return pResult;
    // Down-cast failed, try cross-cast
    for (i = 0, pBasePtr = pCOLocator->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors;
        i < pCOLocator->pClassDescriptor->numBaseClasses;
        i++, pBasePtr++) {
        pBase = *pBasePtr;
        // Check if base class has proper type, is accessible & is unambiguous
        if (TYPEIDS_EQ(pBase->pTypeDescriptor, pTargetTypeID) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_NOTVISIBLE) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_AMBIGUOUS)) {
            return pBase;
        }
    }
    return NULL;
}
static ptrdiff_t __cdecl PMDtoOffset(
    PVOID pThis,                    // ptr to complete object
    const PMD& pmd)                 // pointer-to-member-data structure
{
    ptrdiff_t RetOff = 0;
    if (pmd.pdisp >= 0) {                       // if base is in the virtual part of class
        RetOff = pmd.pdisp;
        RetOff += *(ptrdiff_t*)((char*)*(ptrdiff_t*)((char*)pThis + RetOff) + pmd.vdisp);
    }
    RetOff += pmd.mdisp;
    return RetOff;
}