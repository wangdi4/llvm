// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __PROVISIONAL_MALLOC_H_
#define  __PROVISIONAL_MALLOC_H_


/**
 * This is a utility to register all dynamically allocated objects,
 * and free/keep them (in reverse order) using a single command.
 *
 * It can be used instead of goto: statements, or ugly/buggy cleanup sequences on exit.
 *
 * For C malloc, it simply frees it (note that aligned malloc is treated specially).
 * For C++ there is a special mechanism that allows for special cleanup,
 * besides just calling delete. It is used, for instance, on CL objects.
 */

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
    ProvisionalNewBase() {}
    virtual ~ProvisionalNewBase() {}

    virtual void deleteObject() = 0;
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
	ProvisionalNew(DataP data) :
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
	ProvisionalArrNew(DataP data) :
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

typedef struct _PROVISONAL_SingleData_ {
	bool         isCPP;
	void        *data;
	const char  *str;
	size_t       line;
	bool         aligned;
#ifdef __cplusplus
	ProvisionalNewBase    *cppData;
#else
	void                  *cppData;
#endif
} _PROVISONAL_SingleData_t;

typedef struct _PROVISONAL_MallocArray_ {
	size_t  maxSize;
	size_t  mallocated_pos;
	_PROVISONAL_SingleData_t mallocArr[PROVISIONAL_MALLOC_SIZE];
} _PROVISONAL_MallocArray_t;


#define PROV_INIT_MALLOCARRAY_T(arr, sz) \
    arr.maxSize = sz; \
    arr.mallocated_pos = -1; \
    memset(arr.mallocArr, 0, sizeof(arr.mallocArr));

	

static void IGNORE_GCC_UNUSED *mallocAndRegister(_PROVISONAL_MallocArray_t *base, size_t sz,
		const char *strCreation, const size_t lineNum)
{
	assert(base && "You must provide a malloc base!");
	if (base->mallocated_pos + 1 >= base->maxSize)
	{
		assert(false && "You are trying to malloc too many memory objects!");
		return nullptr;
	}
	_PROVISONAL_SingleData_t m = base->mallocArr[++base->mallocated_pos];
	m.isCPP   = false;
	m.data    = malloc(sz);
	m.aligned = false;
	m.str     = strCreation;
	m.line    = lineNum;
	m.cppData = nullptr;
    PROV_DEBUG_PRINT("Allocating and registring ( %s ) in line %u at position %lu\n", m.str, (cl_uint)m.line, base->mallocated_pos);
	return m.data;
}

static void IGNORE_GCC_UNUSED *alignedMallocAndRegister(_PROVISONAL_MallocArray_t *base, size_t sz, size_t align,
		const char *strCreation, const size_t lineNum)
{
	assert(base && "You must provide a malloc base!");
	if (base->mallocated_pos + 1 >= base->maxSize)
	{
		assert(false && "You are trying to malloc too many memory objects!");
		return nullptr;
	}

	_PROVISONAL_SingleData_t m = base->mallocArr[++base->mallocated_pos];
	m.isCPP   = true;
	m.data    = malloc(sz);
	m.aligned = true;
	m.str     = strCreation;
	m.line    = lineNum;
	m.cppData = nullptr;
    PROV_DEBUG_PRINT("Allocating Aligned and registering ( %s ) in line %u at position %lu\n", m.str, (cl_uint)m.line, base->mallocated_pos);
	return m.data;
}

#ifdef __cplusplus

template <typename DataP>
DataP registerObject(_PROVISONAL_MallocArray_t *base, DataP obj, const char *creationStr, const size_t line)
{
	assert(base && "You must provide a malloc base!");
	if (!obj) return obj;

	ProvisionalNewBase *wrapper = new ProvisionalNew<DataP>(obj);
    if (base->mallocated_pos + 1 >= base->maxSize)
	{
		assert(false && "You are trying to construct too many objects!");
		wrapper->deleteObject();
		return nullptr;
	}

	_PROVISONAL_SingleData_t m = base->mallocArr[++base->mallocated_pos];
	m.isCPP   = true;
	m.data    = nullptr;
	m.aligned = true;
	m.str     = creationStr;
	m.line    = line;
	m.cppData = wrapper;
    PROV_DEBUG_PRINT("registerObject < %s > from line %u at position %lu\n", m.str, (cl_uint)m.line, base->mallocated_pos);
	return obj;
}

template <typename DataP>
DataP registerNewArr(_PROVISONAL_MallocArray_t *base, DataP arr, const char *creationStr, const size_t line)
{
	assert(base && "You must provide a malloc base!");
	if (!arr) return arr;

	ProvisionalNewBase *wrapper = new ProvisionalArrNew<DataP>(arr);
	if (base->mallocated_pos + 1 >= base->maxSize)
	{
		assert(false && "You are trying to construct too many objects!");
		wrapper->deleteObject();
		return nullptr;
	}

	_PROVISONAL_SingleData_t m = base->mallocArr[++base->mallocated_pos];
	m.isCPP   = true;
	m.data    = nullptr;
	m.aligned = false;
	m.str     = creationStr;
	m.line    = line;
	m.cppData = wrapper;
	PROV_DEBUG_PRINT("registerNewArr < %s > from line %u at position %lu\n", m.str, (cl_uint)m.line, base->mallocated_pos);
	return arr;
}

#endif // __cplusplus

static void finalizeMalloc(_PROVISONAL_MallocArray_t *base, const bool keep)
{
	for (int i=base->mallocated_pos ; i >= 0 ; --i)
	{
    	_PROVISONAL_SingleData_t m = base->mallocArr[i];
        if (!m.isCPP)
        {
            if (!keep)
            {

            	PROV_DEBUG_PRINT("finalizeMalloc freeing ( %s ) from line %u at position %d\n",
            			m.str, (cl_uint)m.line, i);
            	if (m.aligned)
            	{
            		ALIGNED_FREE(m.data);
            		m.aligned = false;
            	} else {
            		free(m.data);
            	}
            	m.data  = nullptr;
            	m.isCPP = false;
            }
        }
        #ifdef __cplusplus
        else 
        {
        	ProvisionalNewBase *mcpp = m.cppData;
        	assert(mcpp);
            if (!keep) {
            	PROV_DEBUG_PRINT("finalizeMalloc deleting ( %s ) from line %u at position %d\n",
            			m.str, (cl_uint)m.line, i);
            	mcpp->deleteObject();
	        }
	        delete mcpp;
	        m.isCPP = false;
	        m.cppData = nullptr;
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

#ifndef GTEST_INCLUDE_GTEST_GTEST_H_
/**
 * Call this macro in the function body, before starting dynamic allocation.
 * You can set the value for PROVISIONAL_MALLOC_SIZE !!before!! including this header, to
 * increase the dynamic memory allocation (from the default 20).
 * DO NOT CALL THIS MACRO IF YOU USE THE GTEST FIXTURE.
 */
#define PROV_INIT \
	_PROVISONAL_MallocArray_t PROV_ARRAY_NAME; \
	PROV_INIT_MALLOCARRAY_T(PROV_ARRAY_NAME, PROVISIONAL_MALLOC_SIZE)

#endif // !GTEST_INCLUDE_GTEST_GTEST_H_

/**
 * Call this macro instead of malloc.
 * @example
 * char *buf = (char *)PROV_MALLOC(sizeof(char) * 1024);
 */
#define PROV_MALLOC(sz) \
    mallocAndRegister(&PROV_ARRAY_NAME, sz, #sz, __LINE__)

/**
 * Call this macro instead of aligned malloc.
 * @example
 * char *buf = (char *)PROV_ALIGNED_MALLOC(sizeof(char) * 1024, 128);
 */
#define PROV_ALIGNED_MALLOC(sz, align) \
    alignedMallocAndRegister(&PROV_ARRAY_NAME, sz, align, #sz" aligned at: "#align, __LINE__)

#ifndef GTEST_INCLUDE_GTEST_GTEST_H_
/**
 * Call this macro when exiting your function, if all is OK, and
 * the dynamically allocated data should be kept.
 * DO NOT CALL THIS MACRO IF YOU USE THE GTEST FIXTURE.
 */
#define PROV_RETURN_AND_KEEP(retval) \
    finalizeMalloc(&PROV_ARRAY_NAME, true); \
    return retval;

/**
 * Call this macro when exiting your functionon error, and
 * the dynamically allocated data should be deleted/freed.
 * DO NOT CALL THIS MACRO IF YOU USE THE GTEST FIXTURE.
 */
#define PROV_RETURN_AND_ABANDON(retval) \
    finalizeMalloc(&PROV_ARRAY_NAME, false); \
    return retval;

#endif // !GTEST_INCLUDE_GTEST_GTEST_H_

#ifdef __cplusplus

/**
 * Call this macro to make an (new) allocated object provisional. Use it for objects
 * allocated somewhere else, or for objects with a special new operator.
 * CMyClass *c = PROV_OBJ( new(CMyClass::special_new) CMyClass(1, 2, "abc") );
 */
#define PROV_OBJ(obj_declaration_with_allocation) \
	registerObject(&PROV_ARRAY_NAME, obj_declaration_with_allocation, #obj_declaration_with_allocation, __LINE__)

/**
 * Call this macro to make a (new) allocated array provisional. Use it for arrays
 * allocated somewhere else, or for arrays with a special new operator.
 * CMyClass arr[] = PROV_ARR( new(CMyClass::special_new) CMyClass[5] );
 */
#define PROV_ARR(objarr_declaration_with_allocation) \
		registerNewArr(&PROV_ARRAY_NAME, objarr_declaration_with_allocation, #objarr_declaration_with_allocation, __LINE__)

#endif // __cplusplus


#ifdef GTEST_INCLUDE_GTEST_GTEST_H_

#ifdef PROV_ARRAY_NAME
#undef PROV_ARRAY_NAME
#endif
#define PROV_ARRAY_NAME m_provisionalArray
/**
 * This is GTEST specific fixture that seamlessly harnesses the PROVISIONAL API.
 */
class BaseProvisionalTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        PROV_INIT_MALLOCARRAY_T(PROV_ARRAY_NAME, PROVISIONAL_MALLOC_SIZE);
    }

    virtual void TearDown()
    {
        finalizeMalloc(&PROV_ARRAY_NAME, false);
    }

	_PROVISONAL_MallocArray_t PROV_ARRAY_NAME;
};

#endif // GTEST_INCLUDE_GTEST_GTEST_H_

#endif // __PROVISIONAL_MALLOC_H_

