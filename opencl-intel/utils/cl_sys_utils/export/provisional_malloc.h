
#ifndef __PROVISIONAL_MALLOC_H_
#define  __PROVISIONAL_MALLOC_H_

#ifdef WIN32
	#define IGNORE_GCC_UNUSED
#else
	#define IGNORE_GCC_UNUSED __attribute__((unused))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "cl_sys_defines.h"

//#define PROV_DEBUG_PRINT(...) printf(__VA_ARGS__)
#define PROV_DEBUG_PRINT(...)

#ifndef PROVISIONAL_MALLOC_SIZE
    #define PROVISIONAL_MALLOC_SIZE 20
#endif


#ifdef __cplusplus
/**
 * Base class for provisional malloc templates.
 */
class ProvisionalNewBase
{
public:
    ProvisionalNewBase(const char *strCreation) : m_strObjCreation(strCreation)
    {}

	virtual void deleteObject() = 0;

    const char *m_strObjCreation;
};

template <typename DataP>
void provisionalDeleteObjectByType(DataP data)
{
    delete data;
}

template <typename DataP>
class ProvisionalNew : public ProvisionalNewBase
{
public:
	ProvisionalNew(DataP data, const char *strCreation) : 
      ProvisionalNewBase(strCreation),
      m_data(data)
	{
        //PROV_DEBUG_PRINT("ProvisionalNew::CTOR (%s)\n", m_strObjCreation);
    }
	
	virtual void deleteObject()
	{
		//PROV_DEBUG_PRINT("ProvisionalNew::deleteObject (%s)\n", m_strObjCreation);
		provisionalDeleteObjectByType(m_data);
	}
protected:
	DataP m_data;
};


template <typename DataP>
void provisionalDeleteArrByType(DataP arr)
{
    delete[] arr;
}

template <typename DataP>
class ProvisionalArrNew : public ProvisionalNewBase
{
public:
	ProvisionalArrNew(DataP data, const char *strCreation) : 
      ProvisionalNewBase(strCreation),
      m_data(data)
	{
		//PROV_DEBUG_PRINT("ProvisionalArrNew::CTOR (%s)\n", m_strObjCreation);
    }

	virtual void deleteObject()
	{
		//PROV_DEBUG_PRINT("ProvisionalArrNew::deleteObject [] (%s)\n", m_strObjCreation);
		provisionalDeleteArrByType(m_data);
	}
protected:
	DataP m_data;
};

#endif // __cplusplus

typedef struct _PROVISONAL_MallocArray_ {
	size_t  maxSize;
	size_t  mallocated_pos;
	void   *mallocated[PROVISIONAL_MALLOC_SIZE];
    bool    aligned[PROVISIONAL_MALLOC_SIZE];
    const char  *mallocated_str[PROVISIONAL_MALLOC_SIZE];
#ifdef __cplusplus
	ProvisionalNewBase    *newlocated[PROVISIONAL_MALLOC_SIZE];
#endif
} _PROVISONAL_MallocArray_t;


#define PROV_INIT_MALLOCARRAY_T(arr, sz) \
    arr.maxSize = sz; \
    arr.mallocated_pos = -1; \
    memset(arr.mallocated, 0, sizeof(void*)*sz); \
	memset(arr.aligned, 0, sizeof(void*)*sz);

	

static void IGNORE_GCC_UNUSED *mallocAndRegister(_PROVISONAL_MallocArray_t *mallocArr, size_t sz, const char *strCreation)
{
	assert(mallocArr && "You must provide a malloc array!");
	if (mallocArr->mallocated_pos + 1 >= mallocArr->maxSize)
	{
		assert(false && "You are trying to malloc too many memory objects!");
		return NULL;
	}
	mallocArr->mallocated[++mallocArr->mallocated_pos] = malloc(sz);
	mallocArr->aligned[mallocArr->mallocated_pos] = false;
    mallocArr->mallocated_str[mallocArr->mallocated_pos] = strCreation;
    //PROV_DEBUG_PRINT("Allocating and registring ( %s ) at position %lu\n", mallocArr->mallocated_str[mallocArr->mallocated_pos],
        // mallocArr->mallocated_pos);
	return mallocArr->mallocated[mallocArr->mallocated_pos];
}

static void IGNORE_GCC_UNUSED *alignedMallocAndRegister(_PROVISONAL_MallocArray_t *mallocArr, size_t sz, size_t align, const char *strCreation)
{
	assert(mallocArr && "You must provide a malloc array!");
	if (mallocArr->mallocated_pos + 1 >= mallocArr->maxSize)
	{
		assert(false && "You are trying to malloc too many memory objects!");
		return NULL;
	}
	mallocArr->mallocated[++mallocArr->mallocated_pos] = ALIGNED_MALLOC(sz, align);
	mallocArr->aligned[mallocArr->mallocated_pos] = true;
    mallocArr->mallocated_str[mallocArr->mallocated_pos] = strCreation;
	return mallocArr->mallocated[mallocArr->mallocated_pos];
}

#ifdef __cplusplus

template <typename DataP>
DataP registerObject(_PROVISONAL_MallocArray_t *mallocArr, DataP obj, const char *creationStr)
{
	assert(mallocArr && "You must provide a malloc array!");
	if (!obj) return obj;

	ProvisionalNewBase *wrapper = new ProvisionalNew<DataP>(obj, creationStr);
    if (mallocArr->mallocated_pos + 1 >= mallocArr->maxSize)
	{
		assert(false && "You are trying to construct too many objects!");
		wrapper->deleteObject();
		return NULL;
	}
    mallocArr->newlocated[++mallocArr->mallocated_pos] = wrapper;
    //PROV_DEBUG_PRINT("registerObject ( %s ) at position %lu\n", creationStr, mallocArr->mallocated_pos);
	return obj;
}

template <typename DataP>
DataP registerNewArr(_PROVISONAL_MallocArray_t *mallocArr, DataP arr, const char *creationStr)
{
	assert(mallocArr && "You must provide a malloc array!");
	if (!arr) return arr;

	ProvisionalNewBase *wrapper = new ProvisionalArrNew<DataP>(arr, creationStr);
	if (mallocArr->mallocated_pos + 1 >= mallocArr->maxSize)
	{
		assert(false && "You are trying to construct too many objects!");
		wrapper->deleteObject();
		return NULL;
	}
	mallocArr->newlocated[++mallocArr->mallocated_pos] = wrapper;
	//PROV_DEBUG_PRINT("registerNewArr ( %s ) at position %lu\n", creationStr, mallocArr->mallocated_pos);
	return arr;
}

#endif // __cplusplus

static void finalizeMalloc(_PROVISONAL_MallocArray_t *arr, const bool keep)
{
	for (int i=arr->mallocated_pos ; i >= 0 ; --i)
	{
        if (arr->mallocated[i])
        {
            if (!keep)
            {
            	PROV_DEBUG_PRINT("finalizeMalloc freeing ( %s ) at position %d\n", arr->mallocated_str[i], i);
            	if (arr->aligned[i])
            	{
            		ALIGNED_FREE(arr->mallocated[i]);
            		arr->aligned[i] = false;
            	} else {
            		free(arr->mallocated[i]);
            	}
            }
        }
        #ifdef __cplusplus
        else 
        {
            if (!keep) {
            	PROV_DEBUG_PRINT("finalizeMalloc deleting ( %s ) at position %d\n", arr->newlocated[i]->m_strObjCreation, i);
                arr->newlocated[i]->deleteObject();
	        }
	        delete arr->newlocated[i];
        }
        #endif // __cplusplus
    }
}


/**
 ***********************************************************
 * User macros.
 ***********************************************************
 */

#define PROV_ARRAY_NAME _mallocArr_

/**
 * Call this macro in the function body, before starting dynamic allocation.
 * You can set the value for PROVISIONAL_MALLOC_SIZE !!before!! including this header, to
 * increase the dynamic memory allocation (from the default 20).
 */
#define PROV_INIT \
	_PROVISONAL_MallocArray_t PROV_ARRAY_NAME; \
	PROV_INIT_MALLOCARRAY_T(PROV_ARRAY_NAME, PROVISIONAL_MALLOC_SIZE)

/**
 * Call this macro instead of malloc.
 * @example
 * char *buf = (char *)PROV_MALLOC(sizeof(char) * 1024);
 */
#define PROV_MALLOC(sz) \
    mallocAndRegister(&PROV_ARRAY_NAME, sz, #sz)

/**
 * Call this macro instead of aligned malloc.
 * @example
 * char *buf = (char *)PROV_ALIGNED_MALLOC(sizeof(char) * 1024, 128);
 */
#define PROV_ALIGNED_MALLOC(sz, align) \
    alignedMallocAndRegister(&PROV_ARRAY_NAME, sz, align, #sz" aligned at: "#align)

/**
 * Call this macro when exiting your function, if all is OK, and
 * the dynamically allocated data should be kept.
 */
#define PROV_RETURN_AND_KEEP(retval) \
    finalizeMalloc(&PROV_ARRAY_NAME, TRUE); \
    return retval;

/**
 * Call this macro when exiting your functionon error, and
 * the dynamically allocated data should be deleted/freed.
 */
#define PROV_RETURN_AND_ABANDON(retval) \
    finalizeMalloc(&PROV_ARRAY_NAME, FALSE); \
    return retval;

#ifdef __cplusplus

/**
 * Call this macro to make an (new) allocated object provisional. Use it for objects
 * allocated somewhere else, or for objects with a special new operator.
 * CMyClass *c = MAKE_OBJ_PROVISIONAL( new(CMyClass::special_new) CMyClass(1, 2, "abc") );
 */
#define PROV_OBJ(obj_declaration_with_allocation) \
	registerObject(&PROV_ARRAY_NAME, obj_declaration_with_allocation, #obj_declaration_with_allocation)

/**
 * Call this macro to make a (new) allocated array provisional. Use it for arrays
 * allocated somewhere else, or for arrays with a special new operator.
 * CMyClass arr[] = MAKE_OBJARR_PROVISIONAL( new(CMyClass::special_new) CMyClass[5] );
 */
#define PROV_ARR(objarr_declaration_with_allocation) \
		registerNewArr(&PROV_ARRAY_NAME, objarr_declaration_with_allocation, #objarr_declaration_with_allocation)

#endif // __cplusplus
	
#endif // __PROVISIONAL_MALLOC_H_

