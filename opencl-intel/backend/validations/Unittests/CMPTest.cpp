/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CMPTest.cpp

\*****************************************************************************/
#include "Comparator.h"
#include "Buffer.h"
#include "IComparisonResults.h"
#include "ComparisonResults.h"
#include <iostream>
#include <limits>
#include <list>
#include <typeinfo>
#include "BufferContainer.h"
#include "dxfloat.h"
#include "NEATValue.h"
#include "ALUTest.h"

#include <gtest/gtest.h>

using namespace Validation;

template<typename T>
COMP_RESULT CompareSingleInterval(T bufOut, T minVal, T maxVal, DataTypeVal valType)
{
    IsFloatType<T> _notUsed;
    UNUSED_ARGUMENT(_notUsed);
    Comparator cmp;
    /// Fill equal data
    BufferContainerList list;
    IBufferContainer* bc = list.CreateBufferContainer();
    BufferDesc desc = BufferDesc(1, V1, valType);
    IMemoryObject* buf = bc->CreateBuffer(desc);
    BufferAccessor<T> outBA(*buf);
    outBA.SetElem(0, 0, bufOut);
    BufferContainerList Neat;
    IBufferContainer* bcNeat = Neat.CreateBufferContainer();
    BufferDesc NeatDesc = BufferDesc(1, V1, valType, true);
    IMemoryObject* NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    BufferAccessor<NEATValue> bAccessor(*NeatBuf);
    NEATValue vd1((T)minVal, (T)maxVal);
    bAccessor.SetElem(0, 0, vd1);
    ComparisonResults res;
    COMP_RESULT res1 = cmp.Compare(res, NULL, list, NULL, &Neat);
    return res1;
}

template <>
COMP_RESULT CompareSingleInterval<CFloat16>(CFloat16 bufOut, CFloat16 minVal, CFloat16 maxVal, DataTypeVal valType)
{
    float bufOutFloat = (float)bufOut;
    float minFloat = (float)minVal;
    float maxFloat = (float)maxVal;
    return CompareSingleInterval<float>(bufOutFloat, minFloat, maxFloat, F32);
}

template<typename T>
COMP_RESULT CompareSingleValueRef(T bufOut, T refOut, DataTypeVal valType)
{
    Comparator cmp;
    /// Fill equal data
    BufferContainerList list;
    IBufferContainer* bc = list.CreateBufferContainer();
    BufferDesc desc = BufferDesc(1, V1, valType);
    IMemoryObject* buf = bc->CreateBuffer(desc);
    BufferAccessor<T> outBA(*buf);
    outBA.SetElem(0, 0, bufOut);
    BufferContainerList ref;
    bc = ref.CreateBufferContainer();
    BufferDesc refDesc = BufferDesc(1, V1, valType);
    IMemoryObject* pRefBuf = bc->CreateBuffer(refDesc);
    BufferAccessor<T> bAccessor(*pRefBuf);
    bAccessor.SetElem(0, 0, refOut);
    ComparisonResults res;
    COMP_RESULT toReturn = cmp.Compare(res, NULL, list, &ref, NULL);
    return toReturn;
}

template<typename T>
COMP_RESULT CompareSingleValueULP(T bufOut, T refOut, const double ULP, DataTypeVal valType)
{
    IsFloatType<T> _un;
    UNUSED_ARGUMENT(_un);
    Comparator cmp;
    cmp.SetULPTolerance(ULP);
    /// Fill equal data
    BufferContainerList list;
    IBufferContainer* bc = list.CreateBufferContainer();
    BufferDesc desc = BufferDesc(1, V1, valType);
    IMemoryObject* buf = bc->CreateBuffer(desc);
    BufferAccessor<T> outBA(*buf);
    outBA.SetElem(0, 0, bufOut);
    BufferContainerList ref;
    bc = ref.CreateBufferContainer();
    BufferDesc refDesc = BufferDesc(1, V1, valType);
    IMemoryObject* pRefBuf = bc->CreateBuffer(refDesc);
    BufferAccessor<T> bAccessor(*pRefBuf);
    bAccessor.SetElem(0, 0, refOut);
    ComparisonResults res;
    COMP_RESULT toReturn = cmp.Compare(res, NULL, list, &ref, NULL);
    return toReturn;
}


template <>
COMP_RESULT CompareSingleValueRef<CFloat16>(CFloat16 bufOut, CFloat16 refOut, DataTypeVal valType)
{
    float bufOutFloat = (float)bufOut;
    float refOutFloat = (float)refOut;
    return CompareSingleValueRef(bufOutFloat, refOutFloat, F32);
}

template<typename T>
COMP_RESULT CompareSingleValueNeat(T bufOut, T refOut, DataTypeVal valType)
{
    IsFloatType<T> _notUsed;
    UNUSED_ARGUMENT(_notUsed);
    Comparator cmp;
    /// Fill equal data
    BufferContainerList list;
    IBufferContainer* bc = list.CreateBufferContainer();
    BufferDesc desc = BufferDesc(1, V1, valType);
    IMemoryObject* buf = bc->CreateBuffer(desc);
    BufferAccessor<T> outBA(*buf);
    outBA.SetElem(0, 0, bufOut);
    BufferContainerList Neat;
    IBufferContainer* bcNeat = Neat.CreateBufferContainer();
    BufferDesc NeatDesc = BufferDesc(1, V1, valType, true);
    IMemoryObject* NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    BufferAccessor<NEATValue> bAccessor(*NeatBuf);
    NEATValue vd1((T)refOut);
    bAccessor.SetElem(0, 0, vd1);
    ComparisonResults res;
    return cmp.Compare(res, NULL, list, NULL, &Neat);
}

template <>
COMP_RESULT CompareSingleValueNeat<CFloat16>(CFloat16 bufOut, CFloat16 refOut, DataTypeVal valType)
{
    float bufOutFloat = (float)bufOut;
    float refOutFloat = (float)refOut;
    return CompareSingleValueNeat(bufOutFloat, refOutFloat, F32);
}

template<typename T>
COMP_RESULT BaseComparisonUnEq(DataTypeVal valType)
{
    Comparator cmp;
    /// Fill equal data
    BufferContainerList list;
    IBufferContainer* bc = list.CreateBufferContainer();
    BufferDesc desc = BufferDesc(1, V2, valType);
    IMemoryObject* buf = bc->CreateBuffer(desc);
    BufferAccessor<T> outBA(*buf);
    outBA.SetElem(0, 0, 1);
    outBA.SetElem(0, 1, 1);
    BufferContainerList ref;
    IBufferContainer* bcRef = ref.CreateBufferContainer();
    BufferDesc refDesc = BufferDesc(1, V2, valType);
    IMemoryObject* refBuf = bcRef->CreateBuffer(desc);
    BufferAccessor<T> refBA(*refBuf);
    refBA.SetElem(0, 0, 1);
    refBA.SetElem(0, 1, 2);
    BufferContainerList NEAT;
    IBufferContainer* bcNEAT = NEAT.CreateBufferContainer();
    BufferDesc NEATdesc = BufferDesc(1, V2, valType, true);
    IMemoryObject* NEATbuf = bcNEAT->CreateBuffer(NEATdesc);
    BufferAccessor<NEATValue> bAccessor(*NEATbuf);
    NEATValue vd1((T)1, (T)0, (T)3, NEATValue::ACCURATE);
    bAccessor.SetElem(0, 0, vd1);
    NEATValue vd2((T)2, (T)0, (T)3, NEATValue::ACCURATE);
    bAccessor.SetElem(0, 1, vd2);
    ComparisonResults res;
    COMP_RESULT res1 = cmp.Compare(res, NULL, list, &ref, &NEAT);
    return res1;
}

template<typename T>
COMP_RESULT NeatInternalAccurate(DataTypeVal valType, bool isEqual)
{
    Comparator cmp;
    /// Fill equal data. For testing purposes one buffer container is used.
    /// It contains one buffer with one vector of two element
    /// First element is 1, the second is 2 for each type
    BufferContainerList list;
    /// Create and fill first buffer container for actual output
    IBufferContainer* bc = list.CreateBufferContainer();
    BufferDesc desc = BufferDesc(2, V2, valType);
    IMemoryObject* buf = bc->CreateBuffer(desc);
    BufferAccessor<T> BufAcc1(*buf);
    BufAcc1.SetElem(0, 0, 1.0f);
    BufAcc1.SetElem(0, 1, 2.0f);
    BufAcc1.SetElem(1, 0, 3.0f);
    BufAcc1.SetElem(1, 1, 4.0f);
    buf = bc->CreateBuffer(desc);
    BufferAccessor<T> BufAcc2(*buf);
    BufAcc2.SetElem(0, 0, 5.0f);
    BufAcc2.SetElem(0, 1, 6.0f);
    BufAcc2.SetElem(1, 0, 7.0f);
    BufAcc2.SetElem(1, 1, 8.0f);

    /// Create and fill second buffer container for actual output
    bc = list.CreateBufferContainer();
    buf = bc->CreateBuffer(desc);
    BufferAccessor<T> BufAcc3(*buf);
    BufAcc3.SetElem(0, 0, 9.0f);
    BufAcc3.SetElem(0, 1, 10.0f);
    if(isEqual)
        BufAcc3.SetElem(1, 0, 11.0f);
    else
        BufAcc3.SetElem(1, 0, 12.0f);
    BufAcc3.SetElem(1, 1, 12.0f);
    buf = bc->CreateBuffer(desc);
    BufferAccessor<T> BufAcc4(*buf);
    BufAcc4.SetElem(0, 0, 13.0f);
    BufAcc4.SetElem(0, 1, 14.0f);
    BufAcc4.SetElem(1, 0, 15.0f);
    BufAcc4.SetElem(1, 1, 16.0f);
    
    BufferContainerList Neat;

    /// Create and fill first buffer container for NEAT output
    IBufferContainer* bcNeat = Neat.CreateBufferContainer();
    BufferDesc NeatDesc = BufferDesc(2, V2, valType, true);
    IMemoryObject* NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    BufferAccessor<NEATValue> NewBufAcc1(*NeatBuf);
    NewBufAcc1.SetElem(0, 0, NEATValue((T)1.0f));
    NewBufAcc1.SetElem(0, 1, NEATValue((T)2.0f));
    NewBufAcc1.SetElem(1, 0, NEATValue((T)3.0f));
    NewBufAcc1.SetElem(1, 1, NEATValue((T)4.0f));
    NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    BufferAccessor<NEATValue> NewBufAcc2(*NeatBuf);
    NewBufAcc2.SetElem(0, 0, NEATValue((T)5.0f));
    NewBufAcc2.SetElem(0, 1, NEATValue((T)6.0f));
    NewBufAcc2.SetElem(1, 0, NEATValue((T)7.0f));
    NewBufAcc2.SetElem(1, 1, NEATValue((T)8.0f));
    bcNeat = Neat.CreateBufferContainer();
    NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    BufferAccessor<NEATValue> NewBufAcc3(*NeatBuf);
    NewBufAcc3.SetElem(0, 0, NEATValue((T)9.0f));
    NewBufAcc3.SetElem(0, 1, NEATValue((T)10.0f));
    NewBufAcc3.SetElem(1, 0, NEATValue((T)11.0f));
    NewBufAcc3.SetElem(1, 1, NEATValue((T)12.0f));
    NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    BufferAccessor<NEATValue> NewBufAcc4(*NeatBuf);
    NewBufAcc4.SetElem(0, 0, NEATValue((T)13.0f));
    NewBufAcc4.SetElem(0, 1, NEATValue((T)14.0f));
    NewBufAcc4.SetElem(1, 0, NEATValue((T)15.0f));
    NewBufAcc4.SetElem(1, 1, NEATValue((T)16.0f));
    ComparisonResults res;
    COMP_RESULT res1 = cmp.Compare(res, NULL, list, NULL, &Neat);
    return res1;
}

template<typename T>
COMP_RESULT EqualNeatAccurate(DataTypeVal valType)
{
    return NeatInternalAccurate<T>(valType, true);
}

template<typename T>
COMP_RESULT UnEqualNeatAccurate(DataTypeVal valType)
{
    return NeatInternalAccurate<T>(valType, false);
}

template<typename T>
COMP_RESULT NeatInternalInterval(DataTypeVal valType, bool isEqual)
{
    Comparator cmp;
    /// Fill equal data. For testing purposes two buffer containers are used.
    /// Each contains two buffer with two vector of two element
    BufferContainerList list;
    /// Create and fill first buffer container for actual output
    IBufferContainer* bc = list.CreateBufferContainer();
    BufferDesc desc = BufferDesc(2, V2, valType);
    IMemoryObject* buf = bc->CreateBuffer(desc);
    BufferAccessor<T> BufAcc1(*buf);
    BufAcc1.SetElem(0, 0, 1.0f);
    BufAcc1.SetElem(0, 1, 2.0f);
    BufAcc1.SetElem(1, 0, 3.0f);
    BufAcc1.SetElem(1, 1, 4.0f);
    buf = bc->CreateBuffer(desc);
    BufferAccessor<T> BufAcc2(*buf);
    BufAcc2.SetElem(0, 0, 5.0f);
    BufAcc2.SetElem(0, 1, 6.0f);
    BufAcc2.SetElem(1, 0, 7.0f);
    BufAcc2.SetElem(1, 1, 8.0f);

    /// Create and fill second buffer container for actual output
    bc = list.CreateBufferContainer();
    buf = bc->CreateBuffer(desc);
    BufferAccessor<T> BufAcc3(*buf);
    BufAcc3.SetElem(0, 0, 9.0f);
    BufAcc3.SetElem(0, 1, 10.0f);
    if(isEqual)
        BufAcc3.SetElem(1, 0, 11.0f);
    else
        BufAcc3.SetElem(1, 0, 13.0f);
    BufAcc3.SetElem(1, 1, 12.0f);
    buf = bc->CreateBuffer(desc);
    BufferAccessor<T> BufAcc4(*buf);
    BufAcc4.SetElem(0, 0, 13.0f);
    BufAcc4.SetElem(0, 1, 14.0f);
    BufAcc4.SetElem(1, 0, 15.0f);
    BufAcc4.SetElem(1, 1, 16.0f);
    
    BufferContainerList Neat;

    /// Create and fill first buffer container for NEAT output
    IBufferContainer* bcNeat = Neat.CreateBufferContainer();
    BufferDesc NeatDesc = BufferDesc(2, V2, valType, true);
    IMemoryObject* NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    /// In the first buffer output value is located in the center of interval
    BufferAccessor<NEATValue> NeatBufAcc1(*NeatBuf);
    NeatBufAcc1.SetElem(0, 0, NEATValue((T)0.0f,(T)2.0f));
    NeatBufAcc1.SetElem(0, 1, NEATValue((T)1.0f,(T)3.0f));
    NeatBufAcc1.SetElem(1, 0, NEATValue((T)2.0f,(T)4.0f));
    NeatBufAcc1.SetElem(1, 1, NEATValue((T)3.0f,(T)5.0f));
    NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    /// In the second buffer output value is equal to the lowest value
    BufferAccessor<NEATValue> NeatBufAcc2(*NeatBuf);
    NeatBufAcc2.SetElem(0, 0, NEATValue((T)5.0f,(T)6.0f));
    NeatBufAcc2.SetElem(0, 1, NEATValue((T)6.0f,(T)7.0f));
    NeatBufAcc2.SetElem(1, 0, NEATValue((T)7.0f,(T)8.0f));
    NeatBufAcc2.SetElem(1, 1, NEATValue((T)8.0f,(T)9.0f));
    bcNeat = Neat.CreateBufferContainer();
    NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    /// For the third buffer output value is equal to the highest value
    BufferAccessor<NEATValue> NeatBufAcc3(*NeatBuf);
    NeatBufAcc3.SetElem(0, 0, NEATValue((T)8.0f,(T)9.0f));
    NeatBufAcc3.SetElem(0, 1, NEATValue((T)9.0f,(T)10.0f));
    NeatBufAcc3.SetElem(1, 0, NEATValue((T)10.0f,(T)11.0f));
    NeatBufAcc3.SetElem(1, 1, NEATValue((T)11.0f,(T)12.0f));
    NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    /// The last buffer will contain zero length intervals with output value in it
    BufferAccessor<NEATValue> NeatBufAcc4(*NeatBuf);
    NeatBufAcc4.SetElem(0, 0, NEATValue((T)13.0f,(T)13.0f));
    NeatBufAcc4.SetElem(0, 1, NEATValue((T)14.0f,(T)14.0f));
    NeatBufAcc4.SetElem(1, 0, NEATValue((T)15.0f,(T)15.0f));
    NeatBufAcc4.SetElem(1, 1, NEATValue((T)16.0f,(T)16.0f));
    ComparisonResults res;
    COMP_RESULT res1 = cmp.Compare(res, NULL, list, NULL, &Neat);
    return res1;
}

template<typename T>
COMP_RESULT EqualNeatInterval(DataTypeVal valType)
{
    return NeatInternalInterval<T>(valType, true);
}

template<typename T>
COMP_RESULT UnEqualNeatInterval(DataTypeVal valType)
{
    return NeatInternalInterval<T>(valType, false);
}

template<typename T>
COMP_RESULT NEATComparison(DataTypeVal valType)
{
    Comparator cmp;

    NEATValue::Status st;
    switch(valType)
    {
    case F16:
        st = NEATValue::INTERVAL;
        break;
    case F32:
        st = NEATValue::INTERVAL;
        break;
    case F64:
        st = NEATValue::INTERVAL;
        break;
    default:
        st = NEATValue::ACCURATE;
        break;
    };

    /// Fill equal data
    BufferContainerList list;
    IBufferContainer* bc = list.CreateBufferContainer();
    BufferDesc desc = BufferDesc(1, V2, valType);
    IMemoryObject* buf = bc->CreateBuffer(desc);
    BufferAccessor<T> outBA(*buf);
    outBA.SetElem(0, 0, 1);
    outBA.SetElem(0, 1, 2);
    BufferContainerList ref;
    IBufferContainer* bcRef = ref.CreateBufferContainer();
    BufferDesc refDesc = BufferDesc(1, V2, valType);
    IMemoryObject* refBuf = bcRef->CreateBuffer(desc);
    BufferAccessor<T> refBA(*refBuf);
    refBA.SetElem(0, 0, 1);
    refBA.SetElem(0, 1, 2);
    BufferContainerList NEAT;
    IBufferContainer* bcNEAT = NEAT.CreateBufferContainer();
    BufferDesc NEATdesc = BufferDesc(1, V2, valType, true);
    IMemoryObject* NEATbuf = bcNEAT->CreateBuffer(NEATdesc);
    BufferAccessor<NEATValue> bAccessor(*NEATbuf);
    NEATValue vd1((T)0.1, (T)3.1, st);
    bAccessor.SetElem(0, 0, vd1);
    NEATValue vd2((T)0.1, (T)3.1, st);
    bAccessor.SetElem(0, 1, vd2);
    ComparisonResults res;
    COMP_RESULT res1 = cmp.Compare(res, NULL, list, &ref, &NEAT);
    return res1;
}

/// @brief Tests comparator NEAT functionality.
/// It creates 2 buffer containers inside buffer container list
/// Each buffer container contains 2 buffers with 2 vectors 2 elements each (totally 16 values)
/// To test the case when Comparator have value outside NEAT interval 
/// one value in input buffer doesn't fit within interval boundaries.
/// Then Comparator is called and its output is verified to be NOT_PASSED.
/// Test covers: 
/// Floating point types: F16, F32, F64.
TEST(Comparator, UnEqualNEATInterval)
{
    EXPECT_EQ(NOT_PASSED, UnEqualNeatInterval<CFloat16>(F16));
    EXPECT_EQ(NOT_PASSED, UnEqualNeatInterval<float>(F32));
    EXPECT_EQ(NOT_PASSED, UnEqualNeatInterval<double>(F64));
}

/// @brief Validates comparator NEAT functionality.
/// It creates 2 buffer containers inside buffer container list
/// Each buffer container contains 2 buffers with 2 vectors 2 elements each (totally 16 values)
/// All values in actual input fit within specified NEAT intervals.
/// Then Comparator is called and its output is verified to be PASSED.
/// Test covers: 
/// Floating point types: F16, F32, F64.
TEST(Comparator, EqualNEATInterval)
{
    EXPECT_EQ(PASSED, EqualNeatInterval<CFloat16>(F16));
    EXPECT_EQ(PASSED, EqualNeatInterval<float>(F32));
    EXPECT_EQ(PASSED, EqualNeatInterval<double>(F64));
}

/// @brief Tests comparator precise comparison functionality
/// It creates 2 buffer containers inside buffer container list
/// Each buffer container contains 2 buffers with 2 vectors 2 elements each (totally 16 values)
/// One value in the input differs from accurate NEAT data.
/// Comparator output value is then verified to be NOT_PASSED.
/// Reference data pointer is NULL, but NEAT data has accurate status.
/// Therefore, precise comparison is performed.
/// Test covers:
/// Floating point types: F16, F32, F63
TEST(Comparator, UnEqualNEATAccurate)
{
    /// Test all available data types
    EXPECT_EQ(NOT_PASSED, UnEqualNeatAccurate<CFloat16>(F16));
    EXPECT_EQ(NOT_PASSED, UnEqualNeatAccurate<float>(F32));
    EXPECT_EQ(NOT_PASSED, UnEqualNeatAccurate<double>(F64));
}

/// @brief Tests comparator precise comparison functionality
/// It creates 2 buffer containers inside buffer container list
/// Each buffer container contains 2 buffers with 2 vectors 2 elements each (totally 16 values)
/// All input values are equal to NEAT accurate data.
/// Reference data pointer is NULL, but NEAT data has accurate status.
/// Therefore, precise comparison is performed.
/// Test covers:
/// Floating point types: F16, F32, F63
TEST(Comparator, EqualNEATAccurate)
{
    /// Test all available data type for equality
    EXPECT_EQ(PASSED, EqualNeatAccurate<CFloat16>(F16));
    EXPECT_EQ(PASSED, EqualNeatAccurate<float>(F32));
    EXPECT_EQ(PASSED, EqualNeatAccurate<double>(F64));
}

template<typename T>
void TestFloatBoundsReference(DataTypeVal valType)
{
    /// Check infinities
    EXPECT_EQ(PASSED, CompareSingleValueRef<T>(numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), valType));
    EXPECT_EQ(PASSED, CompareSingleValueRef<T>(-numeric_limits<T>::infinity(),-numeric_limits<T>::infinity(), valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<T>(numeric_limits<T>::infinity(),-numeric_limits<T>::infinity(), valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<T>(-numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<T>(numeric_limits<T>::infinity(), 0, valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<T>(0, numeric_limits<T>::infinity(), valType));

    /// Check NaNs
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<T>(numeric_limits<T>::quiet_NaN(), 0, valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<T>(0, numeric_limits<T>::quiet_NaN(), valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<T>(numeric_limits<T>::quiet_NaN(), numeric_limits<T>::infinity(), valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<T>(numeric_limits<T>::infinity(), numeric_limits<T>::quiet_NaN(), valType));
    EXPECT_EQ(PASSED, CompareSingleValueRef<T>(numeric_limits<T>::quiet_NaN(), numeric_limits<T>::quiet_NaN(), valType));
    EXPECT_EQ(PASSED, CompareSingleValueRef<T>(numeric_limits<T>::quiet_NaN(), numeric_limits<T>::signaling_NaN(), valType));
    EXPECT_EQ(PASSED, CompareSingleValueRef<T>(numeric_limits<T>::signaling_NaN(), numeric_limits<T>::signaling_NaN(), valType));
}

void TestCFloat16BoundsReference()
{
      /// Check infinities
    EXPECT_EQ(PASSED, CompareSingleValueRef<CFloat16>(CFloat16::GetPInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(PASSED, CompareSingleValueRef<CFloat16>(CFloat16::GetNInf(), CFloat16::GetNInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<CFloat16>(CFloat16::GetPInf(),CFloat16::GetNInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<CFloat16>(CFloat16::GetNInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<CFloat16>(CFloat16::GetPInf(), CFloat16(0.0f), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<CFloat16>(CFloat16(0.0f), CFloat16::GetPInf(), F16));

    /// Check NaNs
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<CFloat16>(CFloat16::GetNaN(), CFloat16(0.0f), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<CFloat16>(CFloat16(0.0f), CFloat16::GetNaN(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<CFloat16>(CFloat16::GetNaN(), CFloat16::GetPInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueRef<CFloat16>(CFloat16::GetPInf(), CFloat16::GetNaN(), F16));
    EXPECT_EQ(PASSED, CompareSingleValueRef<CFloat16>(CFloat16::GetNaN(), CFloat16::GetNaN(), F16));
}

template<typename T>
void TestFloatBoundsNeatAccurate(DataTypeVal valType)
{
    /// Check infinities
    EXPECT_EQ(PASSED, CompareSingleValueNeat<T>(numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), valType));
    EXPECT_EQ(PASSED, CompareSingleValueNeat<T>(-numeric_limits<T>::infinity(),-numeric_limits<T>::infinity(), valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<T>(numeric_limits<T>::infinity(),-numeric_limits<T>::infinity(), valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<T>(-numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<T>(numeric_limits<T>::infinity(), 0, valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<T>(0, numeric_limits<T>::infinity(), valType));

    /// Check NaNs
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<T>(numeric_limits<T>::quiet_NaN(), 0, valType));
    EXPECT_EQ(PASSED, CompareSingleValueNeat<T>(0, numeric_limits<T>::quiet_NaN(), valType));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<T>(numeric_limits<T>::quiet_NaN(), numeric_limits<T>::infinity(), valType));
    EXPECT_EQ(PASSED, CompareSingleValueNeat<T>(numeric_limits<T>::infinity(), numeric_limits<T>::quiet_NaN(), valType));
    EXPECT_EQ(PASSED, CompareSingleValueNeat<T>(numeric_limits<T>::quiet_NaN(), numeric_limits<T>::quiet_NaN(), valType));
    EXPECT_EQ(PASSED, CompareSingleValueNeat<T>(numeric_limits<T>::quiet_NaN(), numeric_limits<T>::signaling_NaN(), valType));
    EXPECT_EQ(PASSED, CompareSingleValueNeat<T>(numeric_limits<T>::signaling_NaN(), numeric_limits<T>::signaling_NaN(), valType));
}

void TestCFloat16BoundsNeatAccurate()
{
      /// Check infinities
    EXPECT_EQ(PASSED, CompareSingleValueNeat<CFloat16>(CFloat16::GetPInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(PASSED, CompareSingleValueNeat<CFloat16>(CFloat16::GetNInf(), CFloat16::GetNInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<CFloat16>(CFloat16::GetPInf(),CFloat16::GetNInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<CFloat16>(CFloat16::GetNInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<CFloat16>(CFloat16::GetPInf(), CFloat16(0.0f), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<CFloat16>(CFloat16(0.0f), CFloat16::GetPInf(), F16));

    /// Check NaNs
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<CFloat16>(CFloat16::GetNaN(), CFloat16(0.0f), F16));
    EXPECT_EQ(PASSED, CompareSingleValueNeat<CFloat16>(CFloat16(0.0f), CFloat16::GetNaN(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleValueNeat<CFloat16>(CFloat16::GetNaN(), CFloat16::GetPInf(), F16));
    EXPECT_EQ(PASSED, CompareSingleValueNeat<CFloat16>(CFloat16::GetPInf(), CFloat16::GetNaN(), F16));
    EXPECT_EQ(PASSED, CompareSingleValueNeat<CFloat16>(CFloat16::GetNaN(), CFloat16::GetNaN(), F16));
}

/// @brief Tests comparator ability to handle floating point boundary cases
/// Performs precise comparison of boundary values including NaNs and infinities.
/// Reference and actual output passed to comparator contain one element.
/// NEAT data pointer is NULL in this case
/// Test covers:
/// Floating point types: F16, F32, F64
TEST(Comparator, BoundaryCasesPrecise)
{
    TestFloatBoundsReference<float>(F32);
    TestFloatBoundsReference<double>(F64);
    TestCFloat16BoundsReference();

    TestFloatBoundsNeatAccurate<float>(F32);
    TestFloatBoundsNeatAccurate<double>(F64);
    TestCFloat16BoundsNeatAccurate();
}


template<typename T>
inline T addUlps(const T& val, const long ULPs)
{
    Utils::FloatParts<T> ap(val);
    ap.AddUlps(ULPs);
    return ap.val();
}

/// @brief Tests comparator ability to treat Accuracy with ULPs
/// Floating point types: F16, F32, F64
template<typename T>
void TestComparatorPreciseWithULPs()
{

    const long ULP_tol = 5;
    T valRef = (T) 3.0;

    // check accurate comparison
    EXPECT_EQ(PASSED, CompareSingleValueULP<T>(valRef, valRef, ULP_tol, F32));
    
    // check tolerance
    EXPECT_EQ(PASSED, 
        CompareSingleValueULP<T>(valRef, addUlps<T>(valRef, ULP_tol - 1), ULP_tol, F32));

    EXPECT_EQ(PASSED, 
        CompareSingleValueULP<T>(valRef, addUlps<T>(valRef, ULP_tol), ULP_tol, F32));

    EXPECT_EQ(NOT_PASSED, 
        CompareSingleValueULP<T>(valRef, addUlps<T>(valRef, ULP_tol + 1), ULP_tol, F32));

    // if ULP tolerance is zero should not pass
    EXPECT_EQ(NOT_PASSED, 
        CompareSingleValueULP<T>(valRef, addUlps<T>(valRef, 1), 0.0, F32));

    // if ULP tolerance is zero should pass
    EXPECT_EQ(PASSED, 
        CompareSingleValueULP<T>(valRef, valRef, 0.0, F32));
}

TEST(Comparator, PreciseWithULPs)
{
    // TODO: enable when Utils::eq_tol supports CFloat16
    //TestComparatorPreciseWithULPs<CFloat16>();
    
    TestComparatorPreciseWithULPs<float>();
    
    // TODO: enable double test when Utils:: eq_tol is working with long doubles
    // Utils::eq_tol is working with long doubles to estimate ULPs
#if 0
    //#if !(defined(_WIN32) && defined(_MSC_VER))
    TestComparatorPreciseWithULPs<double>();
#endif

}
template<typename T>
void TestFloatBoundsNeat(DataTypeVal val)
{
    /// Test infinities
    EXPECT_EQ(PASSED, CompareSingleInterval<T>(numeric_limits<T>::infinity(),numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), val));
    EXPECT_EQ(NOT_PASSED, CompareSingleInterval<T>(numeric_limits<T>::infinity(),-numeric_limits<T>::infinity(),-numeric_limits<T>::infinity(), val));
    EXPECT_EQ(NOT_PASSED, CompareSingleInterval<T>(-numeric_limits<T>::infinity(),numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), val));
    EXPECT_EQ(NOT_PASSED, CompareSingleInterval<T>(0, numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), val));
    EXPECT_EQ(PASSED, CompareSingleInterval<T>(0, -numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), val));
    //EXPECT_FATAL_FAILURE(NOT_PASSED, CompareSingleInterval<T>(0, numeric_limits<T>::infinity(),-numeric_limits<T>::infinity(), val));
    EXPECT_EQ(NOT_PASSED, CompareSingleInterval<T>(-1, 0, numeric_limits<T>::infinity(), val));
    EXPECT_EQ(PASSED, CompareSingleInterval<T>(0, -1, numeric_limits<T>::infinity(), val));
    EXPECT_EQ(NOT_PASSED, CompareSingleInterval<T>(0, -numeric_limits<T>::infinity(), -1, val));
    EXPECT_EQ(PASSED, CompareSingleInterval<T>(0, -numeric_limits<T>::infinity(), 1, val));

    // Test NaNs
    EXPECT_EQ(PASSED,    CompareSingleInterval<T>(numeric_limits<T>::quiet_NaN(),-numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), val));
    EXPECT_EQ(NOT_PASSED,CompareSingleInterval<T>(numeric_limits<T>::quiet_NaN(),numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), val));
    EXPECT_EQ(PASSED, CompareSingleInterval<T>(numeric_limits<T>::infinity(),numeric_limits<T>::quiet_NaN(),numeric_limits<T>::infinity(), val));
    EXPECT_EQ(PASSED, CompareSingleInterval<T>(-numeric_limits<T>::infinity(),-numeric_limits<T>::infinity(),numeric_limits<T>::quiet_NaN(), val));
    EXPECT_EQ(PASSED, CompareSingleInterval<T>(numeric_limits<T>::infinity(),-numeric_limits<T>::infinity(),numeric_limits<T>::quiet_NaN(), val));
    EXPECT_EQ(PASSED,CompareSingleInterval<T>(numeric_limits<T>::infinity(),-numeric_limits<T>::infinity(),numeric_limits<T>::quiet_NaN(), val));
    EXPECT_EQ(PASSED,CompareSingleInterval<T>(numeric_limits<T>::quiet_NaN(),-numeric_limits<T>::infinity(),numeric_limits<T>::infinity(), val));
    EXPECT_EQ(NOT_PASSED,CompareSingleInterval<T>(numeric_limits<T>::quiet_NaN(),-numeric_limits<T>::infinity(), 0, val));
}

void TestCFloat16BoundsNeat()
{
        /// Test infinities
    EXPECT_EQ(PASSED, CompareSingleInterval<CFloat16>(CFloat16::GetPInf(),CFloat16::GetPInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleInterval<CFloat16>(CFloat16::GetPInf(),CFloat16::GetNInf(),CFloat16::GetNInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleInterval<CFloat16>(CFloat16::GetNInf(),CFloat16::GetPInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleInterval<CFloat16>(CFloat16(0.0f), CFloat16::GetPInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(PASSED, CompareSingleInterval<CFloat16>(CFloat16(0.0f), CFloat16::GetNInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleInterval<CFloat16>(CFloat16(-1.0f), CFloat16(0.0f), CFloat16::GetPInf(), F16));
    EXPECT_EQ(PASSED, CompareSingleInterval<CFloat16>(CFloat16(0.0f), CFloat16(-1.0f), CFloat16::GetPInf(), F16));
    EXPECT_EQ(NOT_PASSED, CompareSingleInterval<CFloat16>(CFloat16(0.0f), CFloat16::GetNInf(), CFloat16(-1.0f), F16));
    EXPECT_EQ(PASSED, CompareSingleInterval<CFloat16>(CFloat16(0.0f), CFloat16::GetNInf(), CFloat16(1.0f), F16));

    // Test NaNs
    EXPECT_EQ(PASSED,    CompareSingleInterval<CFloat16>(CFloat16::GetNaN(),CFloat16::GetNInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(NOT_PASSED,CompareSingleInterval<CFloat16>(CFloat16::GetNaN(),CFloat16::GetPInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(PASSED, CompareSingleInterval<CFloat16>(CFloat16::GetPInf(),CFloat16::GetNaN(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(PASSED, CompareSingleInterval<CFloat16>(CFloat16::GetNInf(),CFloat16::GetNInf(),CFloat16::GetNaN(), F16));
    EXPECT_EQ(PASSED, CompareSingleInterval<CFloat16>(CFloat16::GetPInf(),CFloat16::GetNInf(),CFloat16::GetNaN(), F16));
    EXPECT_EQ(PASSED,CompareSingleInterval<CFloat16>(CFloat16::GetPInf(),CFloat16::GetNInf(),CFloat16::GetNaN(), F16));
    EXPECT_EQ(PASSED,CompareSingleInterval<CFloat16>(CFloat16::GetNaN(),CFloat16::GetNInf(),CFloat16::GetPInf(), F16));
    EXPECT_EQ(NOT_PASSED,CompareSingleInterval<CFloat16>(CFloat16::GetNaN(),CFloat16::GetNInf(), CFloat16(0.0f), F16));
}

/// @brief Tests comparator ability to handle floating point boundary cases
/// Performs NEAT comparison of boundary values including NaNs and infinities.
/// Reference and NEAT output passed to comparator contain one element.
/// Reference data pointer is NULL in this case.
/// Test covers:
/// Floating point types: F16, F32, F64
TEST(Comparator, BoundaryCasesNeat)
{
    TestFloatBoundsNeat<float>(F32);
    TestFloatBoundsNeat<double>(F64);
    TestCFloat16BoundsNeat();
}

/// @brief Tests Statistics collector for average, summation and counting modes
TEST(Comparator, StatisticsCollector)
{
    StatisticsCollector coll;
    const std::string SUM_STRING = "SUM";
    const std::string AVG_STRING = "AVG";
    const std::string COUNT_STRING = "COUNT";
    EXPECT_EQ(coll.GetResult(SUM_STRING), 0.0);
    EXPECT_EQ(coll.GetResult(AVG_STRING), 0.0);
    EXPECT_EQ(coll.GetResult(COUNT_STRING), 0.0);
    const int TEST_ITERATIONS = 5;
    const double SUM_VAL = 1.0;
    for(int i = 0; i<TEST_ITERATIONS; i++)
    {
        coll.UpdateStatistics(StatisticsCollector::SUM, SUM_STRING, SUM_VAL);
        coll.UpdateStatistics(StatisticsCollector::AVG, AVG_STRING, SUM_VAL);
        coll.CountStatistics(COUNT_STRING);
    }
    EXPECT_EQ(coll.GetResult(SUM_STRING), TEST_ITERATIONS * SUM_VAL);
    EXPECT_EQ(coll.GetResult(AVG_STRING), SUM_VAL);
    EXPECT_EQ(coll.GetResult(COUNT_STRING), TEST_ITERATIONS);
}



/**
 * @def ALIGN_UP()
 *
 * @brief The macro calculates a round up value for _NotAlignedSize divisible by _AlignFactor.
 *
 * @note The result is k*_AlignFactor where k is a positive integer that leads to: 
 *
 *         (k-1)*_AlignFactor < _NotAlignedSize <= k*_AlignFactor 
 */
#define ALIGN_UP(_NotAlignedSize, _AlignFactor) ((((_NotAlignedSize)+(_AlignFactor)-1)/(_AlignFactor))*(_AlignFactor))

const uint32_t IMAGE_LINE_ALIGNMENT_BYTES = 16;

template<ImageChannelDataTypeVal T1, ImageChannelOrderVal T2>
void TestCompareSingleImage(const uint64_t& width, const uint64_t& height)
{
    Comparator cmp;
    ComparisonResults compRes;
    const size_t& pixSize = ImageDesc::CalcPixelSizeInBytes(T1, T2);
    const uint64_t widthStep = ALIGN_UP(width * pixSize, IMAGE_LINE_ALIGNMENT_BYTES );

    
    const ImageDesc desc = ImageDesc (2,
        ImageSizes(width, height, widthStep),
        T1, 
        T2);

    BufferContainerList listAct;
    IBufferContainer* bcAct1 = listAct.CreateBufferContainer();
    IMemoryObject* imgAct1 = bcAct1->CreateImage(desc);

    BufferContainerList listRef;
    IBufferContainer* bcRef1 = listRef.CreateBufferContainer();
    IMemoryObject* imgRef1 = bcRef1->CreateImage(desc);
    
    // set imgAct1 values to 1
    ::memset(imgAct1->GetDataPtr(), 193, desc.GetImageSizeInBytes());
    // copy to Reference
    ::memcpy(imgRef1->GetDataPtr(), imgAct1->GetDataPtr(), desc.GetImageSizeInBytes());

    // expect they are equal
    EXPECT_EQ(PASSED, cmp.Compare(compRes, NULL, listAct, &listRef, NULL));

    // change first byte in Actual
    ::memset(imgAct1->GetDataPtr(), 0, 1);
    // expect they images are not equal
    EXPECT_EQ(NOT_PASSED, cmp.Compare(compRes, NULL, listAct, &listRef, NULL));
}

/// @brief Tests accurate mode of Comparator on Images
TEST(Comparator, ImagesAccurate)
{
    const uint64_t width = 123;
    const uint64_t height = 243;
    
    TestCompareSingleImage<OpenCL_SNORM_INT8, OpenCL_R>(width, height);
    TestCompareSingleImage<OpenCL_FLOAT, OpenCL_R>(width, height);
    TestCompareSingleImage<OpenCL_FLOAT, OpenCL_RGBA>(width, height);
    TestCompareSingleImage<OpenCL_UNSIGNED_INT32, OpenCL_R>(width, height);
    TestCompareSingleImage<OpenCL_FLOAT, OpenCL_RGB>(width, height);
    TestCompareSingleImage<OpenCL_UNORM_INT_101010, OpenCL_RGB>(width, height);
    TestCompareSingleImage<OpenCL_UNORM_SHORT_565, OpenCL_RGB>(width, height);
}


// class for creating random NEAT image
template <typename T>
class ComparatorImageTestNEAT;

template <>
class ComparatorImageTestNEAT<float> : public ALUTest
{
public:
    
    // Intervals with test values for common functions arguments.
    float * Arg1Min;
    float * Arg1Max;

    BufferContainerList m_listNEAT;
    ImageDesc m_imgDescNEAT;
    
    BufferContainerList m_list;
    ImageDesc m_imgDesc;

    IMemoryObject* m_imgNEAT1;
    IMemoryObject * m_img;

    ComparatorImageTestNEAT()
    {
        const uint64_t ImageWidth = 234;
        const uint64_t ImageHeight = 129;
        const ImageChannelDataTypeVal DataType = OpenCL_FLOAT;
        const ImageChannelOrderVal Order = OpenCL_RGBA;
        // Parameters for random data generator.
        const VectorWidth currWidth = V1;
        const DataTypeVal dataTypeVal = F32; // float

        const std::size_t PixelSizeInBytes = ImageDesc::CalcPixelSizeInBytes(DataType, Order);
        const std::size_t widthStep = ALIGN_UP( ImageWidth * PixelSizeInBytes, IMAGE_LINE_ALIGNMENT_BYTES );
        const ImageSizes imSize(ImageWidth, ImageHeight, widthStep);

        m_imgDesc = ImageDesc(2, imSize, DataType, Order, false);
        m_imgDescNEAT = ImageDesc(2, imSize, DataType, Order, true);
        
        std::size_t imSizeBytes = m_imgDesc.GetImageSizeInBytes();

        Arg1Min =   (float * ) malloc(imSizeBytes);
        Arg1Max =   (float * ) malloc(imSizeBytes);
        
        // how many floats in image (approx)
        std::size_t numFloats = imSizeBytes / sizeof(float);
        assert(!(imSizeBytes % sizeof(float) ));
        
        // Fill up argument values with random data
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Min[0], currWidth, numFloats );
        GenerateRandomVectorsAutoSeed(dataTypeVal, &Arg1Max[0], currWidth, numFloats );

        // Make random data aligned with the names: Arg1Min must be <= Arg1Max
        for (std::size_t i = 0; i < numFloats; ++i)
            if (Arg1Min[i] > Arg1Max[i]) std::swap(Arg1Min[i], Arg1Max[i]);

        // create NEAT
        IBufferContainer* bcNEAT1 = m_listNEAT.CreateBufferContainer();
        m_imgNEAT1 = bcNEAT1->CreateImage(m_imgDescNEAT);

        // create actual image
        IBufferContainer* bc = m_list.CreateBufferContainer();
        m_img = bc->CreateImage(m_imgDesc);

        float * pData = (float*) m_img->GetDataPtr();
        NEATValue* pNEAT1 = (NEATValue* ) m_imgNEAT1->GetDataPtr();
        
        for(std::size_t cnt = 0; 
            cnt < numFloats;
            ++cnt)
        {
            pNEAT1[cnt].SetIntervalVal<float>(Arg1Min[cnt], Arg1Max[cnt]);
            // pixel val in the middle of NEAT interval
            pData[cnt] = (Arg1Min[cnt] + Arg1Max[cnt]) / 2.0f;
        }

    }

    virtual ~ComparatorImageTestNEAT()
    {
        free(Arg1Min);
        free(Arg1Max);
    }
};


typedef ::testing::Types<float> FloatTypes;
TYPED_TEST_CASE(ComparatorImageTestNEAT, FloatTypes);

TYPED_TEST(ComparatorImageTestNEAT, Test1)
{
    Comparator cmp;
    ComparisonResults compRes;
    // check float image fits into NEAT intervals
    EXPECT_EQ(PASSED, cmp.Compare(compRes, NULL, this->m_list, NULL, &this->m_listNEAT));

    //check NEAT NOT PASSED
    NEATValue* pNEAT1 = (NEATValue* ) this->m_imgNEAT1->GetDataPtr();
    float * pData = (float*) this->m_img->GetDataPtr();
    const std::size_t ELEM_NUM = 10;
    // make 10th pixel = NEAT min boundary -1 ULP
    Utils::FloatParts<float> ap(*(pNEAT1[ELEM_NUM].GetMin<float>()));
    ap.AddUlps(-1);
    pData[ELEM_NUM] = ap.val();
    // check float image not fit into NEAT intervals
    EXPECT_EQ(NOT_PASSED, cmp.Compare(compRes, NULL, this->m_list, NULL, &this->m_listNEAT));
}


template<typename T>
COMP_RESULT CompareNEATSpecialStatus(T bufOut, NEATValue::Status status, DataTypeVal valType)
{
    IsFloatType<T> _notUsed;
    UNUSED_ARGUMENT(_notUsed);
    Comparator cmp;
    /// Fill equal data
    BufferContainerList list;
    IBufferContainer* bc = list.CreateBufferContainer();
    BufferDesc desc = BufferDesc(1, V1, valType);
    IMemoryObject* buf = bc->CreateBuffer(desc);
    BufferAccessor<T> outBA(*buf);
    outBA.SetElem(0, 0, bufOut);
    BufferContainerList Neat;
    IBufferContainer* bcNeat = Neat.CreateBufferContainer();
    BufferDesc NeatDesc = BufferDesc(1, V1, valType, true);
    IMemoryObject* NeatBuf = bcNeat->CreateBuffer(NeatDesc);
    BufferAccessor<NEATValue> bAccessor(*NeatBuf);
    NEATValue vd1;
    vd1.SetStatus(status);
    bAccessor.SetElem(0, 0, vd1);
    ComparisonResults res;
    COMP_RESULT res1 = cmp.Compare(res, NULL, list, NULL, &Neat);
    return res1;
}

template<typename T>
void TestFloatSpecialStatusNeat(DataTypeVal val)
{
    EXPECT_EQ(PASSED, CompareNEATSpecialStatus<T>(0.0, NEATValue::ANY, val));
    EXPECT_EQ(PASSED, CompareNEATSpecialStatus<T>(1.0, NEATValue::UNKNOWN, val));
    EXPECT_EQ(PASSED, CompareNEATSpecialStatus<T>(3.0, NEATValue::UNWRITTEN, val));
}

/// @brief Tests comparator ability to handle NEAT special statuses: ANY, UNKNOWN, UNWRITTEN
/// Test covers:
/// Floating point types: F16, F32, F64
TEST(Comparator, NEATSpecialStatus)
{
    TestFloatSpecialStatusNeat<CFloat16>(F16);
    TestFloatSpecialStatusNeat<float>(F32);
    TestFloatSpecialStatusNeat<double>(F64);

}