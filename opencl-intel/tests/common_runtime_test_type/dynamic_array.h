// Copyright (c) 2006-2012 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
//
// dynamic_array.h

#ifndef DYNAMIC_ARRAY_
#define DYNAMIC_ARRAY_

#include "general_purpose_struct.h"
#include "gtest_wrapper.h"
#include "ieeehalfprecision.h"
#include "vector_comparator.h"
#include <CL/cl.h>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <malloc.h>
#include <stdlib.h>
#include <string>

template <typename T> inline T getNextValue(T element) {
  ++element;
  if (element < (T)0) {
    return (T)0;
  }
  return element;
}

// DynamicArray - encapsulates dynamic memory management
// Some tests might throw - keeping dynamic memory in this class automatically
// takes care of dynamic memory deallocation
template <typename T> class DynamicArray : public VectorComparator<T> {

private:
  // isHalfArray - true iff contains half array
  bool isHalfArray;

  //  canBeFreed - will deallocate memory in destructor iff is true
  bool canBeFreed;

  // allocateAlignedArray - returns aligned array
  inline void *allocateAlignedArray(size_t size);

  //  freeAlignedArray - frees aligned inputArray
  inline void freeAlignedArray(void *inputArray);

  inline void initializeArrayHalf(T *input_array, int arraySize) {
    // will not get here
  }

  //  initializeArray - initializes inputArray's elements with values
  //[0,...,arraySize-1]
  inline void initializeArray(T *input_array, int arraySize, bool isHalf) {
    if (isHalf) {
      initializeArrayHalf(input_array, arraySize);
      isHalfArray = true;
      return;
    }
    isHalfArray = false;
    initializeDynamicArray(input_array, arraySize);
  }

  inline void initializeDynamicArray(T *input_array, int arraySize) {
    T initValue = (T)0;
    for (int i = 0; i < arraySize; ++i) {
      input_array[i] = initValue = getNextValue(initValue);
    }
  }

  //  initializeArray - initializes inputArray'selements with value
  inline void initializeDynamicArray(T *inputArray, int arraySize, T value);

public:
  T *dynamic_array;
  int dynamic_array_size;

  DynamicArray(DynamicArray &rhs) {
    dynamic_array = NULL;
    dynamic_array_size = rhs.dynamic_array_size;
    canBeFreed = true;
    // returns aligned array
    dynamic_array = (T *)allocateAlignedArray(sizeof(T) * dynamic_array_size);
    // copy elements
    for (int i = 0; i < dynamic_array_size; ++i) {
      dynamic_array[i] = rhs.dynamic_array[i];
    }
  }

  DynamicArray(int arraySize, bool isHalf = false) {
    dynamic_array = NULL;
    dynamic_array_size = arraySize;
    canBeFreed = true;
    // returns aligned array
    dynamic_array = (T *)allocateAlignedArray(sizeof(T) * arraySize);
    initializeArray(dynamic_array, arraySize, isHalf);
  }

  DynamicArray(int arraySize, T value, bool isHalf = false) {
    dynamic_array = NULL;
    dynamic_array_size = arraySize;
    canBeFreed = true;
    // returns aligned array
    dynamic_array = (T *)allocateAlignedArray(sizeof(T) * arraySize);
    initializeDynamicArray(dynamic_array, arraySize, value);
  }

  // disableDestructor - will disable destructor
  inline void disableDestructor() { canBeFreed = false; }

  ~DynamicArray() {
    if (!canBeFreed) {
      // do not free memory here
      return;
    }
    freeMemoryManually();
  }

  inline void freeMemoryManually() {
    if (NULL != dynamic_array) {
      freeAlignedArray(dynamic_array);
      dynamic_array = NULL;
    }
  }

  //  printArrayContent - prints inputArray's content
  inline void printArrayContent() {
    for (int i = 0; i < dynamic_array_size; ++i) {
      std::cout << "[" << i << "]=" << dynamic_array[i] << std::endl;
    }
  }

  //  compareArray - compares this array to rhsArray (element-wise)
  //  rhsArray must contain at least dynamic_array_size elements
  inline void compareArray(DynamicArray<T> &rhsArray) {
    if (NULL == rhsArray.dynamic_array) {
      ASSERT_TRUE(false) << "Null argument provided";
    }
    if (dynamic_array_size != rhsArray.dynamic_array_size) {
      ASSERT_TRUE(false) << "compareArray failed, different sizes";
    }
    for (int i = 0; i < dynamic_array_size; ++i) {
      if (false == VectorComparator<T>::compare(rhsArray.dynamic_array[i],
                                                dynamic_array[i])) {
        EXPECT_TRUE(false) << "compareArray failed for index " << i;
        return;
      }
    }
  }

  //  compareArray - compares this array to rhsArray (element-wise)
  //  rhsArray must contain at least dynamic_array_size elements
  inline void compareArray(T *rhsArray, size_t arraySize) {
    if (NULL == rhsArray) {
      ASSERT_TRUE(false) << "Null argument provided";
    }
    std::vector<cl_float> float_array(dynamic_array_size);

    // copy current half array to float array
    halfp2singles(&float_array[0], dynamic_array, dynamic_array_size);

    std::vector<cl_float> float_array2(dynamic_array_size);

    // copy current half array to float array
    halfp2singles(&float_array2[0], rhsArray, dynamic_array_size);

    arraySize = arraySize < (cl_uint)dynamic_array_size ? arraySize
                                                        : dynamic_array_size;
    for (cl_uint i = 0; i < arraySize; ++i) {
      if (false ==
          VectorComparator<T>::compare(rhsArray[i], dynamic_array[i])) {
        ASSERT_TRUE(false) << "compareArray failed for index " << i;
        return;
      }
    }
  }

  //  compareArray - compares all dynamic_array's elements to expectedValue
  inline void compareArray(T expectedValue) {
    for (int i = 0; i < dynamic_array_size; ++i) {
      if (expectedValue != dynamic_array[i]) {
        //  the following will always result in an error but will give
        // informative output and will break from the loop
        EXPECT_EQ(expectedValue, dynamic_array[i])
            << "compareArray failed for index " << i;
        return;
      }
    }
  }

  inline void compareArraySumHalf(float expectedSum) {
    // will not get here
  }

  //  compareArray - compares array's elements' sum to expectedSum
  inline void compareArraySum(float expectedSum) {
    if (isHalfArray) {
      compareArraySumHalf(expectedSum);
      return;
    }
    float actualSum = sumElements();
    if (fabs(fabs(actualSum) - fabs(expectedSum)) > 1.0f) {
      EXPECT_EQ(actualSum, expectedSum)
          << "compareArraySum failed (content is not equal to expectedSum)";
    }
  }

  //  sumElements - returns sum of all elements in dynamic_array
  inline float sumElements() {
    float sum = 0.0f;
    for (int i = 0; i < dynamic_array_size; ++i) {
      sum += (float)dynamic_array[i];
    }
    return sum;
  }

  //  sumElements - returns sum of all elements in dynamic_array
  inline float seriesSum() {
    float sum = 0.0f;
    for (int i = 0; i < dynamic_array_size; ++i) {
      sum += i;
    }
    return sum;
  }

  // multBy - multiplies each element of dynamic_array by value.
  // If saturate is true will saturate
  inline void multBy(int value, bool saturate = false) {
    for (int i = 0; i < dynamic_array_size; ++i) {
      dynamic_array[i] *= value;
    }
  }
};

// allocateAlignedArray - returns aligned array
template <typename T>
inline void *DynamicArray<T>::allocateAlignedArray(size_t size) {
#ifdef _WIN32
  return _aligned_malloc(size, 128);
#else
  return memalign(128, size * sizeof(int));
#endif
}

//  freeAlignedArray - frees aligned inputArray
template <typename T>
inline void DynamicArray<T>::freeAlignedArray(void *inputArray) {
  if (NULL == inputArray) {
    return;
  }
#ifdef _WIN32
  _aligned_free(inputArray);
#else
  free(inputArray);
#endif
  inputArray = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  initialization
////////////////////////////////////////////////////////////////////////////////////////////////////
template <>
inline void DynamicArray<cl_half>::initializeArrayHalf(cl_half *input_array,
                                                       int arraySize) {
  cl_float *float_array = new cl_float[arraySize];
  for (int i = 0; i < arraySize; ++i) {
    float_array[i] = (cl_float)i;
  }
  singles2halfp(input_array, float_array, arraySize);
  delete[] float_array;
}

template <>
inline void DynamicArray<HalfWrapper>::initializeArray(HalfWrapper *input_array,
                                                       int arraySize,
                                                       bool isHalf) {
  isHalfArray = true;
  cl_float *float_array = new cl_float[arraySize];
  for (int i = 0; i < arraySize; ++i) {
    float_array[i] = (cl_float)i;
  }
  singles2halfp(input_array, float_array, arraySize);
  delete[] float_array;
}

template <>
inline void DynamicArray<UserDefinedStructure>::initializeDynamicArray(
    UserDefinedStructure *input_array, int arraySize) {
  for (int i = 0; i < arraySize; ++i) {
    input_array[i].x = 3 * i;
    input_array[i].y = (float)3 * i + 1;
    input_array[i].z = 3 * i + 2;
  }
}

template <>
inline void
DynamicArray<cl_char2>::initializeDynamicArray(cl_char2 *input_array,
                                               int arraySize) {
  cl_char initValue = (cl_char)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 2; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_char4>::initializeDynamicArray(cl_char4 *input_array,
                                               int arraySize) {
  cl_char initValue = (cl_char)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 4; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_char8>::initializeDynamicArray(cl_char8 *input_array,
                                               int arraySize) {
  cl_char initValue = (cl_char)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 8; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_char16>::initializeDynamicArray(cl_char16 *input_array,
                                                int arraySize) {
  cl_char initValue = (cl_char)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 16; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_uchar2>::initializeDynamicArray(cl_uchar2 *input_array,
                                                int arraySize) {
  cl_uchar initValue = (cl_uchar)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 2; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_uchar4>::initializeDynamicArray(cl_uchar4 *input_array,
                                                int arraySize) {
  cl_uchar initValue = (cl_uchar)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 4; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_uchar8>::initializeDynamicArray(cl_uchar8 *input_array,
                                                int arraySize) {
  cl_uchar initValue = (cl_uchar)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 8; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_uchar16>::initializeDynamicArray(cl_uchar16 *input_array,
                                                 int arraySize) {
  cl_uchar initValue = (cl_uchar)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 16; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_short2>::initializeDynamicArray(cl_short2 *input_array,
                                                int arraySize) {
  cl_short initValue = (cl_short)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 2; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_short4>::initializeDynamicArray(cl_short4 *input_array,
                                                int arraySize) {
  cl_short initValue = (cl_short)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 4; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_short8>::initializeDynamicArray(cl_short8 *input_array,
                                                int arraySize) {
  cl_short initValue = (cl_short)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 8; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_short16>::initializeDynamicArray(cl_short16 *input_array,
                                                 int arraySize) {
  cl_short initValue = (cl_short)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 16; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_ushort2>::initializeDynamicArray(cl_ushort2 *input_array,
                                                 int arraySize) {
  cl_ushort initValue = (cl_ushort)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 2; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_ushort4>::initializeDynamicArray(cl_ushort4 *input_array,
                                                 int arraySize) {
  cl_ushort initValue = (cl_ushort)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 4; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_ushort8>::initializeDynamicArray(cl_ushort8 *input_array,
                                                 int arraySize) {
  cl_ushort initValue = (cl_ushort)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 8; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_ushort16>::initializeDynamicArray(cl_ushort16 *input_array,
                                                  int arraySize) {
  cl_ushort initValue = (cl_ushort)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 16; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void DynamicArray<cl_int2>::initializeDynamicArray(cl_int2 *input_array,
                                                          int arraySize) {
  cl_int initValue = (cl_int)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 2; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void DynamicArray<cl_int4>::initializeDynamicArray(cl_int4 *input_array,
                                                          int arraySize) {
  cl_int initValue = (cl_int)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 4; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void DynamicArray<cl_int8>::initializeDynamicArray(cl_int8 *input_array,
                                                          int arraySize) {
  cl_int initValue = (cl_int)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 8; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_int16>::initializeDynamicArray(cl_int16 *input_array,
                                               int arraySize) {
  cl_int initValue = (cl_int)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 16; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_uint2>::initializeDynamicArray(cl_uint2 *input_array,
                                               int arraySize) {
  cl_uint initValue = (cl_uint)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 2; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_uint4>::initializeDynamicArray(cl_uint4 *input_array,
                                               int arraySize) {
  cl_uint initValue = (cl_uint)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 4; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_uint8>::initializeDynamicArray(cl_uint8 *input_array,
                                               int arraySize) {
  cl_uint initValue = (cl_uint)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 8; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_uint16>::initializeDynamicArray(cl_uint16 *input_array,
                                                int arraySize) {
  cl_uint initValue = (cl_uint)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 16; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_long2>::initializeDynamicArray(cl_long2 *input_array,
                                               int arraySize) {
  cl_long initValue = (cl_long)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 2; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_long4>::initializeDynamicArray(cl_long4 *input_array,
                                               int arraySize) {
  cl_long initValue = (cl_long)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 4; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_long8>::initializeDynamicArray(cl_long8 *input_array,
                                               int arraySize) {
  cl_long initValue = (cl_long)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 8; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_long16>::initializeDynamicArray(cl_long16 *input_array,
                                                int arraySize) {
  cl_long initValue = (cl_long)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 16; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_ulong2>::initializeDynamicArray(cl_ulong2 *input_array,
                                                int arraySize) {
  cl_ulong initValue = (cl_ulong)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 2; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_ulong4>::initializeDynamicArray(cl_ulong4 *input_array,
                                                int arraySize) {
  cl_ulong initValue = (cl_ulong)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 4; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_ulong8>::initializeDynamicArray(cl_ulong8 *input_array,
                                                int arraySize) {
  cl_ulong initValue = (cl_ulong)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 8; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_ulong16>::initializeDynamicArray(cl_ulong16 *input_array,
                                                 int arraySize) {
  cl_ulong initValue = (cl_ulong)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 16; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_float2>::initializeDynamicArray(cl_float2 *input_array,
                                                int arraySize) {
  cl_float initValue = (cl_float)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 2; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_float4>::initializeDynamicArray(cl_float4 *input_array,
                                                int arraySize) {
  cl_float initValue = (cl_float)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 4; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_float8>::initializeDynamicArray(cl_float8 *input_array,
                                                int arraySize) {
  cl_float initValue = (cl_float)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 8; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

template <>
inline void
DynamicArray<cl_float16>::initializeDynamicArray(cl_float16 *input_array,
                                                 int arraySize) {
  cl_float initValue = (cl_float)0;
  for (int i = 0; i < arraySize; ++i) {
    for (int k = 0; k < 16; ++k) {
      input_array[i].s[k] = initValue = getNextValue(initValue);
    }
  }
}

//  initializeArray - initializes inputArray's elements with value
template <typename T>
inline void DynamicArray<T>::initializeDynamicArray(T *inputArray,
                                                    int arraySize, T value) {
  for (int i = 0; i < arraySize; ++i) {
    inputArray[i] = value;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  printArrayContent
////////////////////////////////////////////////////////////////////////////////////////////////////

//  printArrayContent - prints inputArray's content
template <> inline void DynamicArray<cl_float2>::printArrayContent() {
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 2; ++k) {
      std::cout << "[" << i << "].[" << k << "]=" << dynamic_array[i].s[k]
                << std::endl;
    }
  }
}

template <> inline void DynamicArray<cl_float4>::printArrayContent() {
  for (int i = 0; i < dynamic_array_size; ++i) {
    std::cout << i << std::endl;
    for (int k = 0; k < 4; ++k) {
      std::cout << "[" << i << "].[" << k << "]=" << dynamic_array[i].s[k]
                << std::endl;
    }
  }
}

template <> inline void DynamicArray<cl_uchar4>::printArrayContent() {
  for (int i = 0; i < dynamic_array_size; ++i) {
    std::cout << i << std::endl;
    for (int k = 0; k < 4; ++k) {
      std::cout << "[" << i << "].[" << k << "]=" << (int)dynamic_array[i].s[k]
                << std::endl;
    }
  }
}

template <> inline void DynamicArray<cl_char4>::printArrayContent() {
  for (int i = 0; i < dynamic_array_size; ++i) {
    std::cout << i << std::endl;
    for (int k = 0; k < 4; ++k) {
      std::cout << "[" << i << "].[" << k << "]=" << (int)dynamic_array[i].s[k]
                << std::endl;
    }
  }
}

template <> inline void DynamicArray<cl_int4>::printArrayContent() {
  for (int i = 0; i < dynamic_array_size; ++i) {
    std::cout << i << std::endl;
    for (int k = 0; k < 4; ++k) {
      std::cout << "[" << i << "].[" << k << "]=" << (int)dynamic_array[i].s[k]
                << std::endl;
    }
  }
}

template <>
inline void DynamicArray<UserDefinedStructure>::printArrayContent() {
  for (int i = 0; i < dynamic_array_size; ++i) {
    std::cout << i << std::endl;
    std::cout << "[" << i << "].x=" << dynamic_array[i].x << std::endl;
    std::cout << "[" << i << "].y=" << dynamic_array[i].y << std::endl;
    std::cout << "[" << i << "].z=" << (int)dynamic_array[i].z << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  sumElements
////////////////////////////////////////////////////////////////////////////////////////////////////
template <> inline float DynamicArray<cl_float4>::sumElements() {
  float sum = 0.0f;
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      sum += (float)dynamic_array[i].s[k];
    }
  }
  return sum;
}

template <> inline float DynamicArray<cl_char4>::sumElements() {
  float sum = 0.0f;
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      sum += (float)dynamic_array[i].s[k];
    }
  }
  return sum;
}

template <> inline float DynamicArray<cl_int4>::sumElements() {
  float sum = 0.0f;
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      sum += (float)dynamic_array[i].s[k];
    }
  }
  return sum;
}

template <> inline float DynamicArray<cl_uint4>::sumElements() {
  float sum = 0.0f;
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      sum += (float)dynamic_array[i].s[k];
    }
  }
  return sum;
}

template <> inline float DynamicArray<cl_short4>::sumElements() {
  float sum = 0.0f;
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      sum += (float)dynamic_array[i].s[k];
    }
  }
  return sum;
}

template <> inline float DynamicArray<cl_ushort4>::sumElements() {
  float sum = 0.0f;
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      sum += (float)dynamic_array[i].s[k];
    }
  }
  return sum;
}

template <> inline float DynamicArray<cl_uchar4>::sumElements() {
  float sum = 0.0f;
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      sum += (float)dynamic_array[i].s[k];
    }
  }
  return sum;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  multBy
////////////////////////////////////////////////////////////////////////////////////////////////////
template <>
inline void DynamicArray<cl_half>::multBy(int value, bool saturate) {
  cl_float *float_array = new cl_float[dynamic_array_size];

  // copy current half array to float array
  halfp2singles(float_array, dynamic_array, dynamic_array_size);

  // multiply elements of float array
  for (int i = 0; i < dynamic_array_size; ++i) {
    float_array[i] *= value;
  }

  // copy float array back to singles array
  singles2halfp(dynamic_array, float_array, dynamic_array_size);
  delete[] float_array;
}

template <>
inline void DynamicArray<cl_float4>::multBy(int value, bool saturate) {
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      dynamic_array[i].s[k] *= value;
    }
  }
}

template <>
inline void DynamicArray<cl_char4>::multBy(int value, bool saturate) {
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      if (saturate) {
        int tmpElement = dynamic_array[i].s[k];
        tmpElement *= value;
        if (CHAR_MAX <= tmpElement) {
          dynamic_array[i].s[k] = CHAR_MAX;
        } else {
          dynamic_array[i].s[k] = tmpElement;
        }
      } else {
        dynamic_array[i].s[k] *= value;
      }
    }
  }
}

template <>
inline void DynamicArray<cl_short4>::multBy(int value, bool saturate) {
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      if (saturate) {
        int tmpElement = dynamic_array[i].s[k];
        tmpElement *= value;
        if (SHRT_MAX <= tmpElement) {
          dynamic_array[i].s[k] = SHRT_MAX;
        } else {
          dynamic_array[i].s[k] = tmpElement;
        }
      } else {
        dynamic_array[i].s[k] *= value;
      }
    }
  }
}

template <>
inline void DynamicArray<cl_int4>::multBy(int value, bool saturate) {
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      dynamic_array[i].s[k] *= value;
    }
  }
}

template <>
inline void DynamicArray<cl_uchar4>::multBy(int value, bool saturate) {
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      if (saturate) {
        int tmpElement = dynamic_array[i].s[k];
        tmpElement *= value;
        if (UCHAR_MAX <= tmpElement) {
          dynamic_array[i].s[k] = UCHAR_MAX;
        } else {
          dynamic_array[i].s[k] = tmpElement;
        }
      } else {
        dynamic_array[i].s[k] *= value;
      }
    }
  }
}

template <>
inline void DynamicArray<cl_ushort4>::multBy(int value, bool saturate) {
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      if (saturate) {
        int tmpElement = dynamic_array[i].s[k];
        tmpElement *= value;
        if (USHRT_MAX <= tmpElement) {
          dynamic_array[i].s[k] = USHRT_MAX;
        } else {
          dynamic_array[i].s[k] = tmpElement;
        }
      } else {
        dynamic_array[i].s[k] *= value;
      }
    }
  }
}

template <>
inline void DynamicArray<cl_uint4>::multBy(int value, bool saturate) {
  for (int i = 0; i < dynamic_array_size; ++i) {
    for (int k = 0; k < 4; ++k) {
      dynamic_array[i].s[k] *= value;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  compareArray
////////////////////////////////////////////////////////////////////////////////////////////////////
//  compareArray - compares array's elements' sum to expectedSum
template <>
inline void DynamicArray<cl_half>::compareArraySumHalf(float expectedSum) {
  cl_float *float_array = new cl_float[dynamic_array_size];
  halfp2singles(float_array, dynamic_array, dynamic_array_size);
  float actualSum = 0.0f;
  for (int i = 0; i < dynamic_array_size; ++i) {
    actualSum += float_array[i];
  }
  delete[] float_array;
  if (fabs(fabs(actualSum) - fabs(expectedSum)) > 1.0f) {
    EXPECT_EQ(actualSum, expectedSum)
        << "compareArraySum failed (content is not equal to expectedSum)";
  }
}

#endif /* DYNAMIC_ARRAY_ */
