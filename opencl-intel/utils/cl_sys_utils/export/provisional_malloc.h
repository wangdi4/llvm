
#ifndef __PROVISIONAL_MALLOC_H_
#define  __PROVISIONAL_MALLOC_H_

#ifdef WIN32
#define NULL 0
#define TRUE true
#define FALSE false
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
    const char  *mallocated_str[PROVISIONAL_MALLOC_SIZE];
#ifdef __cplusplus
	ProvisionalNewBase    *newlocated[PROVISIONAL_MALLOC_SIZE];
#endif
} _PROVISONAL_MallocArray_t;


#define PROV_INIT_MALLOCARRAY_T(arr, sz) \
    arr.maxSize = sz; \
    arr.mallocated_pos = -1; \
    memset(arr.mallocated, 0, sizeof(void*)*sz);

	

static void *mallocAndRegister(_PROVISONAL_MallocArray_t *mallocArr, size_t sz, const char *strCreation)
{
	assert(mallocArr && "You must provide a malloc array!");
	if (mallocArr->mallocated_pos + 1 >= mallocArr->maxSize)
	{
		assert(false && "You are trying to malloc too many memory objects!");
		return NULL;
	}
	mallocArr->mallocated[++mallocArr->mallocated_pos] = malloc(sz);
    mallocArr->mallocated_str[mallocArr->mallocated_pos] = strCreation;
    //PROV_DEBUG_PRINT("Allocating and registring ( %s ) at position %lu\n", mallocArr->mallocated_str[mallocArr->mallocated_pos],
        // mallocArr->mallocated_pos);
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
		        free(arr->mallocated[i]);
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

/**
 * Call this macro in the function body, before starting dymaic allocation.
 * You can set the value for PROVISIONAL_MALLOC_SIZE !!before!! including this header, to
 * increase the dynamic memory allocation (from the default 20).
 */
#define PROV_INIT \
	_PROVISONAL_MallocArray_t _mallocArr_; \
	PROV_INIT_MALLOCARRAY_T(_mallocArr_, PROVISIONAL_MALLOC_SIZE)

/**
 * Call this macro instead of malloc.
 * @example
 * char *buf = (char *)PROVISIONAL_MALLOC(sizeof(char) * 1024);
 */
#define PROV_MALLOC(sz) \
    mallocAndRegister(&_mallocArr_, sz, #sz)

/**
 * Call this macro when exiting your function, if all is OK, and
 * the dynamically allocated data should be kept.
 */
#define PROV_RETURN_AND_KEEP(retval) \
    finalizeMalloc(&_mallocArr_, TRUE); \
    return retval;

/**
 * Call this macro when exiting your functionon error, and
 * the dynamically allocated data should be deleted/freed.
 */
#define PROV_RETURN_AND_ABANDON(retval) \
    finalizeMalloc(&_mallocArr_, FALSE); \
    return retval;

#ifdef __cplusplus

/**
 * Call this macro to make an (new) allocated object provisional. Use it for objects
 * allocated somewhere else, or for objects with a special new operator.
 * CMyClass *c = MAKE_OBJ_PROVISIONAL( new(CMyClass::special_new) CMyClass(1, 2, "abc") );
 */
#define PROV_OBJ(obj_declaration_with_allocation) \
	registerObject(&_mallocArr_, obj_declaration_with_allocation, #obj_declaration_with_allocation)

/**
 * Call this macro to make a (new) allocated array provisional. Use it for arrays
 * allocated somewhere else, or for arrays with a special new operator.
 * CMyClass arr[] = MAKE_OBJARR_PROVISIONAL( new(CMyClass::special_new) CMyClass[5] );
 */
#define PROV_ARR(objarr_declaration_with_allocation) \
		registerNewArr(&_mallocArr_, objarr_declaration_with_allocation, #objarr_declaration_with_allocation)

#endif // __cplusplus
	
#endif // __PROVISIONAL_MALLOC_H_

