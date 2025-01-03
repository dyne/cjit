/*!
 *  @file libxutils/src/data/array.h
 *
 *  This source is part of "libxutils" project
 *  2015-2020  Sun Dro (s.kalatoz@gmail.com)
 * 
 * @brief Dynamically allocated data holder 
 * with some sorting and search algorithms.
 */

#ifndef __XUTILS_XARRAY_H__
#define __XUTILS_XARRAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#define XARRAY_SUCCESS          0
#define XARRAY_FAILURE          -1

#define XARRAY_INITIAL_SIZE     8
#define XARRAY_SORTBY_SIZE      1
#define XARRAY_SORTBY_KEY       0

typedef enum {
    XARRAY_STATUS_OK = (uint8_t)0,
    XARRAY_STATUS_EMPTY,
    XARRAY_STATUS_NO_MEMORY
} xarray_status_t;

typedef struct XArrayData {
    void* pData;
    size_t nSize;
    uint32_t nKey;
} xarray_data_t;

typedef int(*xarray_comparator_t)(const void*, const void*, void*);
typedef void(*xarray_clear_cb_t)(xarray_data_t *pArrData);

typedef struct XArray_ {
    xarray_data_t** pData;
    xarray_clear_cb_t clearCb;
    xarray_status_t eStatus;
    uint8_t nFixed;
    uint8_t nAlloc;
    size_t nSize;
    size_t nUsed;
} xarray_t;

xarray_data_t *XArray_NewData(void *pData, size_t nSize, uint32_t nKey);
void XArray_FreeData(xarray_data_t *pArrData);
void XArray_ClearData(xarray_t *pArr, xarray_data_t *pArrData);

xarray_t* XArray_New(size_t nSize, uint8_t nFixed);
void* XArray_Init(xarray_t *pArr, size_t nSize, uint8_t nFixed);
size_t XArray_Realloc(xarray_t *pArr);
void XArray_Destroy(xarray_t *pArr);
void XArray_Clear(xarray_t *pArr);
void XArray_Free(xarray_t **ppArr);

int XArray_Add(xarray_t *pArr, xarray_data_t *pNewData);
int XArray_AddData(xarray_t *pArr, void* pData, size_t nSize);
int XArray_PushData(xarray_t *pArr, void *pData, size_t nSize);
int XArray_AddDataKey(xarray_t *pArr, void* pData, size_t nSize, uint32_t nKey);
void* XArray_GetDataOr(xarray_t *pArr, size_t nIndex, void *pRet);
void* XArray_GetData(xarray_t *pArr, size_t nIndex);
size_t XArray_GetSize(xarray_t *pArr, size_t nIndex);
uint32_t XArray_GetKey(xarray_t *pArr, size_t nIndex);
uint8_t XArray_Contains(xarray_t *pArr, size_t nIndex);

xarray_data_t* XArray_Remove(xarray_t *pArr, size_t nIndex);
void XArray_Delete(xarray_t *pArr, size_t nIndex);
void XArray_Swap(xarray_t *pArr, size_t nIndex1, size_t nIndex2);

void XArray_Sort(xarray_t *pArr, xarray_comparator_t compare, void *pCtx);
void XArray_BubbleSort(xarray_t *pArr, xarray_comparator_t compare, void *pCtx);
void XArray_QuickSort(xarray_t *pArr, xarray_comparator_t compare, void *pCtx, int nStart, int nFinish);
void XArray_SortBy(xarray_t *pArr, int nSortBy);

int XArray_SentinelSearch(xarray_t *pArr, uint32_t nKey);
int XArray_LinearSearch(xarray_t *pArr, uint32_t nKey);
int XArray_DoubleSearch(xarray_t *pArr, uint32_t nKey);
int XArray_BinarySearch(xarray_t *pArr, uint32_t nKey);

xarray_data_t* XArray_Get(xarray_t *pArr, size_t nIndex);
xarray_data_t* XArray_Set(xarray_t *pArr, size_t nIndex, xarray_data_t *pNewData);
xarray_data_t* XArray_Insert(xarray_t *pArr, size_t nIndex, xarray_data_t *pData);
xarray_data_t* XArray_SetData(xarray_t *pArr, size_t nIndex, void *pData, size_t nSize);
xarray_data_t* XArray_InsertData(xarray_t *pArr, size_t nIndex, void *pData, size_t nSize);

size_t XArray_Used(xarray_t *pArr);
size_t XArray_Size(xarray_t *pArr);

#ifdef __cplusplus
}
#endif

#endif /* __XUTILS_XARRAY_H__ */
