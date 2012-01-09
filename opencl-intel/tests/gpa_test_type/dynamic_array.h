// Copyright (c) 2006-2007 Intel Corporation
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

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <malloc.h>
#include <gtest/gtest.h>
#include <CL/cl.h>

// DynamicArray - encapsulates dynamic memory management
// Some tests might throw - keeping dynamic memory in this class automatically takes care of dynamic memory deallocation
template <typename T>
class DynamicArray{

private:
	//	canBeFreed - will deallocate memory in destructor iff is true
	bool canBeFreed;
	
	// allocateAlignedArray - returns aligned array
	void* allocateAlignedArray(size_t size);

	//	freeAlignedArray - frees aligned inputArray
	void freeAlignedArray(void* inputArray);

	//	initializeArray - initializes inputArray's elements with values [0,...,arraySize-1]
	void initializeArray(T* input_array, int arraySize)
	{
		T initValue = (T)0;
		for(int i=0; i<arraySize; ++i)
		{
			input_array[i] = initValue++; 
		}
	}

	//	initializeArray - initializes inputArray'selements with value
	void initializeArray(T* inputArray, int arraySize, T value);

public: 
	T* dynamic_array;
	int dynamic_array_size;
	
	DynamicArray(DynamicArray& rhs)
	{
		dynamic_array = NULL;
		dynamic_array_size = rhs.dynamic_array_size;
		canBeFreed = true;
		// returns aligned array
		dynamic_array = (T*)allocateAlignedArray(sizeof(T)*dynamic_array_size);
		// copy elements
		for(int i=0; i<dynamic_array_size; ++i){
			dynamic_array[i]=rhs.dynamic_array[i];
		}
	}

	DynamicArray(int arraySize)
	{
		dynamic_array = NULL;
		dynamic_array_size = arraySize;
		canBeFreed = true;
		// returns aligned array
		dynamic_array = (T*)allocateAlignedArray(sizeof(T)*arraySize);
		initializeArray(dynamic_array, arraySize);
	}

	DynamicArray(int arraySize, T value)
	{
		dynamic_array = NULL;
		dynamic_array_size = arraySize;
		canBeFreed = true;
		// returns aligned array
		dynamic_array = (T*)allocateAlignedArray(sizeof(T)*arraySize);
		initializeArray(dynamic_array, arraySize, value);
	}

	// disableDestructor - will disable destructor
	void disableDestructor(){
		canBeFreed = false;
	}

	~DynamicArray()
	{
		if(!canBeFreed)
		{
			// do not free memory here
			return;
		}
		freeMemoryManually();
		//std::cout << "DTOR" << std::endl;
	}

	void freeMemoryManually()
	{
		if(NULL!=dynamic_array)
		{
			freeAlignedArray(dynamic_array);
			dynamic_array = NULL;
		}
	}

	//	printArrayContent - prints inputArray's content
	void printArrayContent()
	{
		for(int i=0; i<dynamic_array_size; ++i){
			std::cout << "[" << i << "]=" << dynamic_array[i] << std::endl;
		}
	}

	//	compareArray - compares this array to rhsArray (element-wise)
	//	rhsArray must contain at least dynamic_array_size elements
	void compareArray(DynamicArray<T>& rhsArray)
	{
		if(NULL==rhsArray.dynamic_array)
		{
			ASSERT_TRUE(false) << "Null argument provided";
		}
		if(dynamic_array_size!=rhsArray.dynamic_array_size)
		{
			ASSERT_TRUE(false) << "compareArray failed, different sizes";
		}
		for(int i=0; i<dynamic_array_size; ++i){
			if(false == compare(rhsArray.dynamic_array[i],dynamic_array[i])){
				EXPECT_TRUE(false) << "compareArray failed for index " << i;
				return;
			}
		}
	}
	
	//	compareArray - compares this array to rhsArray (element-wise)
	//	rhsArray must contain at least dynamic_array_size elements
	void compareArray(T* rhsArray, size_t arraySize)
	{
		if(NULL==rhsArray)
		{
			ASSERT_TRUE(false) << "Null argument provided";
		}
		arraySize = arraySize<dynamic_array_size?arraySize:dynamic_array_size;
		for(int i=0; i<arraySize; ++i){
			if(false == compare(rhsArray[i],dynamic_array[i])){
				ASSERT_TRUE(false) << "compareArray failed for index " << i;
				return;
			}
		}
	}

	//	compareArray - compares all dynamic_array's elements to expectedValue
	void compareArray(T expectedValue)
	{
		for(int i=0; i<dynamic_array_size; ++i){
			if(expectedValue != dynamic_array[i]){
				//	the following will always result in an error but will give informative output and will break from the loop	
				EXPECT_EQ(expectedValue, dynamic_array[i]) << "compareArray failed for index " << i;
				return;
			}
		}
	}

	//	compareArray - compares array's elements' sum to expectedSum
	void compareArraySum(float expectedSum)
	{
		float actualSum = sumElements();
		if(fabs(fabs(actualSum)-fabs(expectedSum))>1.0f){
			EXPECT_EQ(actualSum, expectedSum) << "compareArraySum failed (content is not equal to expectedSum)";
		}				
	}

	//	compareArrayNotSum - compares array's elements' sum to expectedSum and succeeds iff they are different
	void compareArrayNotSum(float expectedSum)
	{
		float actualSum = sumElements();
		if(fabs(fabs(actualSum)-fabs(expectedSum))<=1.0f){
			EXPECT_EQ(actualSum, expectedSum) << "compareArrayNotSum failed (content is equal to expectedSum)";
		}				
	}

	//	sumElements - returns sum of all elements in dynamic_array
	float sumElements()
	{
		float sum = 0.0f;
		for(int i=0; i<dynamic_array_size; ++i){
			sum+=(float)dynamic_array[i];
		}
		return sum;
	}

	//	sumElements - returns sum of all elements in dynamic_array
	float seriesSum()
	{
		float sum = 0.0f;
		for(int i=0; i<dynamic_array_size; ++i){
			sum+=i;
		}
		return sum;
	}

	// multBy - multiplies each element of dynamic_array by value.
	// If saturate is true will saturate
	void multBy(int value, bool saturate=false)
	{
		for(int i=0; i<dynamic_array_size; ++i){
			dynamic_array[i]*=value;
		}
	}

};

// allocateAlignedArray - returns aligned array
template<typename T>
void* DynamicArray<T>::allocateAlignedArray(size_t size)
{
#ifdef _WIN32
	return _aligned_malloc(size, 128);
#else
	return memalign(128, arraySize * sizeof(int));
#endif
}

//	freeAlignedArray - frees aligned inputArray
template<typename T>
void DynamicArray<T>::freeAlignedArray(void* inputArray)
{
	if(NULL==inputArray){
		return;
	}
#ifdef _WIN32
	_aligned_free(inputArray);
#else
	free(inputArray);
#endif
	inputArray = NULL;
}

template<>
void DynamicArray<cl_int4>::initializeArray(cl_int4* input_array, int arraySize)
{
	for(int i=0; i<arraySize; ++i)
	{
		for(int k=0; k<4; ++k)
		{
			input_array[i].s[k] = (cl_int)4*i+k; 
		}
	}
}

#endif /* DYNAMIC_ARRAY_ */