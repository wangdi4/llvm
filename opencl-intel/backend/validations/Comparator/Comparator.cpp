/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  Comparator.cpp

\*****************************************************************************/
#include "llvm/Support/raw_ostream.h"
#include "IComparisonResults.h"
#include "Comparator.h"
#include "NEATValue.h"
#include "Buffer.h"
#include "Image.h"
#include <limits>
#include "math.h"
#include "dxfloat.h"
#include "Exception.h"
#include <cmath>
#include <memory>
#include "FloatOperations.h"

using namespace Validation;
using namespace Intel::OpenCL::DeviceBackend;

// helper class to correctly process indexes within {} scope
// automatically pushes value to stack in constructor
// in destructor pops element
class IndexStackHelper
{
public:
    IndexStackHelper(IComparisonResults::Index& obj, const uint32_t& val):
      m_obj(obj)
    {  obj.pushIdx(val); }
    ~IndexStackHelper() {m_obj.popIdx();}
private:
    IComparisonResults::Index& m_obj;
};

Validation::COMP_RESULT Comparator::Compare(IComparisonResults& res,
                                            const std::vector<bool>* pIgnore,
                                            const IBufferContainerList& pActual,
                                            const IBufferContainerList* pReference,
                                            const IBufferContainerList* pNEAT )
{
    // set object local pointer to IComparisonResluts used by ReportMismatch function
    m_pComparisonResults = &res;
    // clear mismatches index stack
    m_indexStack = IComparisonResults::Index();
    // result of comparator
    Validation::COMP_RESULT CompRes = PASSED;
    // set flags if we have reference and NEAT
    m_haveReference = ( pReference != NULL );
    m_haveNEAT = ( pNEAT != NULL );

    // check number of buffer containers in list are equal with reference
    if(m_haveReference)
    {
        if(pActual.GetBufferContainerCount() != pReference->GetBufferContainerCount())
            return NOT_PASSED;
    }

    // check number of buffer containers in list are equal with NEAT
    if(m_haveNEAT)
    {
        if(pActual.GetBufferContainerCount() != pNEAT->GetBufferContainerCount())
            return NOT_PASSED;
    }

    /// loop for each BufferContainer
    for(uint32_t BCIndex = 0; BCIndex < pActual.GetBufferContainerCount(); ++BCIndex)
    {
        // push BC index
        IndexStackHelper ih(m_indexStack, BCIndex);
        // get actual BC
        const IBufferContainer* pBCAct = pActual.GetBufferContainer(BCIndex);
        const uint32_t count = (uint32_t)pBCAct->GetMemoryObjectCount();

        // obtain reference and NEAT BC
        const IBufferContainer* pBCRef =
            (m_haveReference) ? pReference->GetBufferContainer(BCIndex) : NULL;
        const IBufferContainer* pBCNEAT =
            (m_haveNEAT) ? pNEAT->GetBufferContainer(BCIndex) : NULL;

        // check ignore list size is equal to number of buffers
        if(pIgnore && !(pIgnore->size() == count))
        {
            throw Exception::InvalidArgument("Comparator::CompareInternal invalid size of ignore list");
        }

        // check number of MemObjects in Container are equal with reference
        if(m_haveReference)
        {
            if(pBCAct->GetMemoryObjectCount() != pBCRef->GetMemoryObjectCount())
                return NOT_PASSED;
        }

        // check number of MemObjects in Container are equal with NEAT
        if(m_haveNEAT)
        {
            if(pBCAct->GetMemoryObjectCount() != pBCNEAT->GetMemoryObjectCount())
                return NOT_PASSED;
        }

        // go through buffers in the BufferContainter
        for(uint32_t bufInd = 0; bufInd<count; bufInd++)
        {
            // skip buffers marked for ignore
            if(pIgnore && pIgnore->at(bufInd))
            {
                continue;
            }

            // push buffer idx
            IndexStackHelper ih(m_indexStack, bufInd);

            // obtain MemoryObjects
            const IMemoryObject* pMemObjAct = pBCAct->GetMemoryObject(bufInd);
            // set current memory object description
            m_pMemObjDescCurrent = pMemObjAct->GetMemoryObjectDesc();
            // obtain Reference MemoryObject
            const IMemoryObject* pMemObjRef = (m_haveReference) ?
                pBCRef->GetMemoryObject(bufInd) : NULL;
            // set flag if current memobject supports NEAT format
            // and NEAT should be used for this memobject
            m_IsNEATSupportedMemObj = (m_haveNEAT) ?
                pBCNEAT->GetMemoryObject(bufInd)->GetMemoryObjectDesc()->IsNEAT() :
                false;

            // obtain NEAT MemoryObjects
            const IMemoryObject* pMemObjNEAT = (m_haveNEAT && m_IsNEATSupportedMemObj) ?
                pBCNEAT->GetMemoryObject(bufInd) : NULL;

            if(Image::GetImageName() == pMemObjAct->GetName())
            {   // Image object
                // obtain Image
                const Image* pImageAct = static_cast<const Image*>(pMemObjAct);
                // obtain Reference Image
                const Image* pImageRef = (m_haveReference) ?
                    static_cast<const Image*>(pMemObjRef) : NULL;
                // obtain NEAT Image
                const Image* pImageNEAT = (m_IsNEATSupportedMemObj) ?
                    static_cast<const Image*>(pMemObjNEAT) : NULL;

                // check if Reference object is image
                if(m_haveReference && (pImageRef == NULL))
                    return NOT_PASSED;

                // check if NEAT object is image
                if(m_IsNEATSupportedMemObj && (pImageNEAT == NULL))
                    return NOT_PASSED;

                // call image comparator
                if(NOT_PASSED == CompareImages(pImageAct, pImageRef, pImageNEAT))
                    CompRes = NOT_PASSED;
            }
            else if (Buffer::GetBufferName() == pMemObjAct->GetName())
            {   // Buffer object
                // obtain actual buffer
                const Buffer * pBufAct = static_cast<const Buffer*>(pMemObjAct);
                // obtain Reference buffer
                const Buffer* pBufRef = (m_haveReference) ?
                    static_cast<const Buffer*>(pMemObjRef) : NULL;
                // obtain NEAT Image
                const Buffer* pBufNEAT = (m_IsNEATSupportedMemObj) ?
                    static_cast<const Buffer*>(pMemObjNEAT) : NULL;

                // check if Reference object is buffer
                if(m_haveReference && (pBufRef == NULL))
                    return NOT_PASSED;

                // check if NEAT object is buffer
                if(m_IsNEATSupportedMemObj && (pBufNEAT == NULL))
                    return NOT_PASSED;

                // call buffer comparison
                if(NOT_PASSED == CompareBuffers(pBufAct, pBufRef, pBufNEAT))
                    CompRes = NOT_PASSED;
            }
            else
                // throw exception if unknown MemObj
                throw Exception::InvalidArgument("Comparator::CompareInternal Unknown type of MemObj");
        }
        // check index stack
        assert(m_indexStack.m_stack.size() == 1);
    }
    return CompRes;
}

COMP_RESULT Comparator::CompareBuffers( const Buffer* pAct, const Buffer* pRef, const Buffer* pNEAT )
{
    const BufferDesc * pActDesc = static_cast<const BufferDesc *>(pAct->GetMemoryObjectDesc());

    const BufferDesc * pRefDesc = (m_haveReference) ?
        static_cast<const BufferDesc *>(pRef->GetMemoryObjectDesc()) : NULL;

    const BufferDesc * pNEATDesc = (m_IsNEATSupportedMemObj) ?
        static_cast<const BufferDesc *>(pNEAT->GetMemoryObjectDesc()) : NULL;

    // check descriptors in actual and reference
    if(m_haveReference && ((*pActDesc) != (*pRefDesc)))
        return NOT_PASSED;

    // check descriptors in actual and NEAT
    if(m_IsNEATSupportedMemObj && (*pActDesc != *pNEATDesc))
        return NOT_PASSED;

    // obtain pointer to actual data
    const int8_t* pActData = (int8_t* ) pAct->GetDataPtr();
    // obtain pointer to ref data
    const int8_t* pRefData = (m_haveReference) ?
        (int8_t* ) pRef->GetDataPtr() : NULL;
    // obtain pointer to NEAT data
    const NEATValue* pNEATData = (m_IsNEATSupportedMemObj) ?
        (NEATValue* ) pNEAT->GetDataPtr() : NULL;

    // call comparator for N sequential elements
    return CompareNElements(pActData, pRefData, pNEATData,
        pActDesc->GetElementDescription(), pActDesc->NumOfElements());

}

Validation::COMP_RESULT Comparator::CompareNElements( const int8_t* pAct, const int8_t* pRef,
                                                      const NEATValue* pNEAT,
                                                      const TypeDesc& Type,
                                                      const std::size_t& NumElems )
{
// helper macro to call scalar comparator
#define CASE_TYPE(__TYPEDESC,__TYPE) \
        case __TYPEDESC: {\
            COMP_RESULT localres = CompareNScalarElements<__TYPE>(\
                (const __TYPE *) pAct, (const __TYPE *) pRef, pNEAT, NumElems);\
            res = (localres == NOT_PASSED) ? NOT_PASSED : res;\
            break;}

    COMP_RESULT res = PASSED;
    switch(Type.GetType())
    {
        // integer
        CASE_TYPE(TCHAR, int8_t)
        CASE_TYPE(TSHORT, int16_t)
        CASE_TYPE(TINT, int32_t)
        CASE_TYPE(TLONG, int64_t)
        CASE_TYPE(TUCHAR, uint8_t)
        CASE_TYPE(TUSHORT, uint16_t)
        CASE_TYPE(TUINT, uint32_t)
        CASE_TYPE(TULONG, uint64_t)
        // floating point
        CASE_TYPE(THALF, CFloat16)
        CASE_TYPE(TFLOAT, float)
        CASE_TYPE(TDOUBLE, double)

    case TPOINTER:
        // TODO: what to do with TPOINTER?
        throw Exception::NotImplemented("Not implemented TPOINTER");
    case TARRAY:
    case TVECTOR:
        {
            // obtain typeDesc of Array/Vector element
            const TypeDesc VecElemType = Type.GetSubTypeDesc(0);
            // size of element in bytes
            const std::size_t BufElemSize = Type.GetSizeInBytes();
            // number of elements in vector/array
            const uint64_t VecElemsNum = Type.GetNumberOfElements();
            // size of NEAT buffer element in bytes
            std::size_t BufElemNEATSize = 0;
            if(m_IsNEATSupportedMemObj){
                TypeDesc TypeNEAT(Type);
                TypeNEAT.SetNeat(true);
                BufElemNEATSize = TypeNEAT.GetSizeInBytes();
            }
            // loop over elements in Buffer
            for(std::size_t cntBufferElem = 0; cntBufferElem < NumElems; ++cntBufferElem)
            {
                // push buffer element index
                IndexStackHelper ih(m_indexStack, cntBufferElem);
                // obtain pointer to actual data
                const int8_t* pActData = pAct + cntBufferElem * BufElemSize;
                // obtain pointer to ref data
                const int8_t* pRefData = (m_haveReference) ?
                    pRef + cntBufferElem * BufElemSize : NULL;
                // obtain pointer to NEAT data
                const NEATValue* pNEATData = (m_IsNEATSupportedMemObj) ?
                    (NEATValue*)((uint8_t*)pNEAT + cntBufferElem * BufElemNEATSize) : NULL; // NEATValue *

                // compare elements in vector thru recursion
                if( NOT_PASSED == CompareNElements(pActData, pRefData, pNEATData,
                    VecElemType, VecElemsNum))
                    res = NOT_PASSED;
            }
            break;
        }
    case TSTRUCT:
        {
            // size of Buffer element in bytes
            const std::size_t BufElemSize = Type.GetSizeInBytes();
            // create NEAT description for element
            // and obtain size of NEAT element in buffer of structures
            TypeDesc TypeNEAT;
            std::size_t BufElemNEATSize = 0;
            const int8_t * pNeatInt8 = NULL; // pointer to NEATValue in int8_t* type
            if(m_IsNEATSupportedMemObj){
                TypeNEAT = Type; TypeNEAT.SetNeat(true);
                BufElemNEATSize = TypeNEAT.GetSizeInBytes();
                pNeatInt8 = (const int8_t *) pNEAT;
            }
            // loop over elements in Buffer
            for(std::size_t cntBufferElem = 0; cntBufferElem < NumElems; ++cntBufferElem)
            {
                // push buffer element index
                IndexStackHelper ih(m_indexStack, cntBufferElem);
                // obtain pointer to actual data
                const int8_t* pActData = pAct + cntBufferElem * BufElemSize;
                // obtain pointer to ref data
                const int8_t* pRefData = (m_haveReference) ?
                    pRef + cntBufferElem * BufElemSize : NULL;
                // obtain pointer to NEAT data as int8_t *
                const int8_t* pNEATDataInt8 = (m_IsNEATSupportedMemObj) ?
                    pNeatInt8 + cntBufferElem * sizeof(BufElemNEATSize) : NULL;

                // loop over struct members
                for(uint64_t cntStructElem = 0;
                    cntStructElem < Type.GetNumOfSubTypes();
                    ++cntStructElem)
                {
                    // push struct element index to stack
                    IndexStackHelper ih(m_indexStack, cntStructElem);
                    const TypeDesc subType = Type.GetSubTypeDesc(cntStructElem);
                    const int8_t* pActS =
                        pActData + subType.GetOffsetInStruct();
                    const int8_t* pRefS = (m_haveReference) ?
                        pRefData + subType.GetOffsetInStruct() : NULL;
                    const NEATValue * pNEATS = (m_IsNEATSupportedMemObj) ?
                        (const NEATValue *) (pNEATDataInt8 +
                        TypeNEAT.GetSubTypeDesc(cntStructElem).GetOffsetInStruct())
                        : NULL;

                    // compare one struct element via recursion
                    if( NOT_PASSED == CompareNElements(pActS, pRefS, pNEATS, subType, 1))
                        res = NOT_PASSED;
                }
            } // for(std::size_t cntBufferElem = 0; cntBufferElem < NumElems; ++cntBufferElem)
            break;
        } // case TSTRUCT:
    default:
        throw Exception::InvalidArgument("Comparator can't process elements of the following data type: " + Type.TypeToString());
    }

    return res;

#undef CASE_TYPE
}

template<typename T>
COMP_RESULT Comparator::CompareNScalarElements( const T* pAct,
                                                    const T* pRef,
                                                    const NEATValue* pNEAT,
                                                    const  std::size_t& NumElems )
{
    // check T is scalar
    IsScalarType<T> _notUsed;
    UNUSED_ARGUMENT(_notUsed);

    COMP_RESULT res = PASSED;
    for(std::size_t cntBufferElem = 0; cntBufferElem < NumElems; ++cntBufferElem)
    {
        // push element index
        IndexStackHelper ih(m_indexStack, cntBufferElem);
        const T * pActElem = pAct + cntBufferElem;
        const T * pRefElem = (m_haveReference) ? pRef + cntBufferElem : NULL;
        const NEATValue *
            pNEATElem = (m_IsNEATSupportedMemObj) ? pNEAT + cntBufferElem : NULL;

        COMP_RESULT localres = NOT_PASSED;
        if(m_IsNEATSupportedMemObj)
        {
            localres = CompareScalarNEAT<T>(*pActElem, *pNEATElem);

            // checks if Reference fits into NEAT interval
            // todo: place under some NEAT debug condition
            if(pRef &&
                (NOT_PASSED == CompareScalarNEAT<T>(*pRefElem, *pNEATElem)))
            {
                localres = NOT_PASSED;
            }
        }
        else
        {
            localres = CompareScalarAccurate<T>(*pActElem, *pRefElem);
        }

        if(NOT_PASSED == localres)
        {
            ReportMismatch(pActElem, pRefElem, pNEATElem);
            res = NOT_PASSED;
        }
        res = (NOT_PASSED == localres) ? NOT_PASSED : res;
    }
    return res;
}


// helper macro to call scalar comparator
// this macro obtains corresponding C type to pixel type
// and calls CompareNScalarElements specialized for the C type obtained
#define CASE_TYPE_IMAGES(__TYPEDESC) \
        case __TYPEDESC:{\
        typedef ImageChannelDataTypeValToCType<__TYPEDESC>::type CType;\
        res = CompareNScalarElements<CType>(\
        (const CType *) ptrAct, (const CType *) ptrRef, ptrNEAT, channelCount);\
        break;}

Validation::COMP_RESULT Comparator::CompareImages( const Image* pImgAct, const Image* pImgRef, const Image* pImgNEAT )
{
    COMP_RESULT toReturn = PASSED;

    const ImageDesc * pActDesc = static_cast<const ImageDesc *>(
        pImgAct->GetMemoryObjectDesc());

    const ImageDesc * pRefDesc = (m_haveReference) ?
        static_cast<const ImageDesc *>(pImgRef->GetMemoryObjectDesc()) : NULL;

    const ImageDesc * pNEATDesc = (m_IsNEATSupportedMemObj) ?
        static_cast<const ImageDesc *>(pImgNEAT->GetMemoryObjectDesc()) : NULL;

    // check descriptors in actual and reference
    if(m_haveReference && (*pActDesc != *pRefDesc))
        return NOT_PASSED;

    // check descriptors in actual and NEAT
    if(m_IsNEATSupportedMemObj)
    {
        std::auto_ptr<ImageDesc> pNeatModifiedForSizeComparison( (ImageDesc*) pNEATDesc->Clone());
        pNeatModifiedForSizeComparison->SetNeat(false);
        if(m_IsNEATSupportedMemObj && (*pActDesc != *pNeatModifiedForSizeComparison))
            return NOT_PASSED;
    }

    // is image 3D
    const bool Is3D = (pActDesc->GetNumOfDimensions() == 3);

    // check NEAT is 2D image
    if(m_IsNEATSupportedMemObj && Is3D)
        throw Validation::Exception::InvalidArgument("CompareImages::3D NEAT images are not supported");


    const int8_t *ptrBaseAct = (const int8_t *)pImgAct-> GetDataPtr();
    const int8_t *ptrBaseRef = (m_haveReference) ?
        (const int8_t *)pImgRef-> GetDataPtr() : NULL;
    const int8_t *ptrBaseNEAT = (m_IsNEATSupportedMemObj) ?
        (const int8_t *)pImgNEAT-> GetDataPtr() : NULL;

    const size_t pixelSize = pActDesc->GetElementSize();
    const uint64_t Depth = Is3D ? pActDesc->GetSizes().depth : 1;
    const uint64_t slicePitch = Is3D ? pActDesc->GetSizes().slice : 0;
    const uint64_t Height = pActDesc->GetSizes().height;
    const uint64_t rowPitch = pActDesc->GetSizes().row;
    const uint64_t Width = pActDesc->GetSizes().width;
    const size_t channelCount = GetChannelCount(pActDesc->GetImageChannelOrder());
    const ImageChannelDataTypeVal ImageChannelDataType = pActDesc->GetImageChannelDataType();
    const size_t pixelSizeNEAT = (m_IsNEATSupportedMemObj) ? pNEATDesc->GetElementSize() : 0;
    const uint64_t rowPitchNEAT = (m_IsNEATSupportedMemObj) ? pNEATDesc->GetSizes().row : 0;
    const uint64_t slicePitchNEAT = (m_IsNEATSupportedMemObj) ? 0: 0; // do not support 3D at the moment

    // loop over pixels in image
    for(uint64_t z = 0; z < Depth; ++z)
    {
        if(Is3D) m_indexStack.pushIdx(z);

        for(uint64_t y = 0; y < Height; ++y)
        {
            IndexStackHelper ih(m_indexStack, y);
            for(uint64_t x = 0; x < Width; ++x)
            {
                COMP_RESULT res = PASSED;
                IndexStackHelper ih(m_indexStack, x);
                const int8_t * ptrAct = ptrBaseAct + z * slicePitch + y * rowPitch + x * pixelSize;
                const int8_t * ptrRef = (m_haveReference) ?
                    ptrBaseRef + z * slicePitch + y * rowPitch + x * pixelSize : NULL;
                const NEATValue* ptrNEAT = (m_IsNEATSupportedMemObj) ?
                    (const NEATValue*)(ptrBaseNEAT + z * slicePitchNEAT + y * rowPitchNEAT + x * pixelSizeNEAT) : NULL;

                switch(ImageChannelDataType)
                {
                    CASE_TYPE_IMAGES(OpenCL_SNORM_INT8)
                    CASE_TYPE_IMAGES(OpenCL_SNORM_INT16)
                    CASE_TYPE_IMAGES(OpenCL_UNORM_INT8)
                    CASE_TYPE_IMAGES(OpenCL_UNORM_INT16)
                    CASE_TYPE_IMAGES(OpenCL_SIGNED_INT8)
                    CASE_TYPE_IMAGES(OpenCL_SIGNED_INT16)
                    CASE_TYPE_IMAGES(OpenCL_SIGNED_INT32)
                    CASE_TYPE_IMAGES(OpenCL_UNSIGNED_INT8)
                    CASE_TYPE_IMAGES(OpenCL_UNSIGNED_INT16)
                    CASE_TYPE_IMAGES(OpenCL_UNSIGNED_INT32)
                    CASE_TYPE_IMAGES(OpenCL_FLOAT)
                    CASE_TYPE_IMAGES(OpenCL_HALF_FLOAT)
                    case OpenCL_UNORM_SHORT_555 :
                    {   // special case. treat OpenCL_UNORM_SHORT_555 as
                        // one element. all channels are packed into one element
                        typedef ImageChannelDataTypeValToCType<OpenCL_UNORM_SHORT_555>::type CType;
                        res = CompareNScalarElements<CType>(
                           (const CType *) ptrAct, (const CType *) ptrRef, ptrNEAT,
                            1 // one channel
                            );
                            break;
                    }
                    case OpenCL_UNORM_SHORT_565 :
                    {   // special case. treat OpenCL_UNORM_SHORT_565 as
                        // one element. all channels are packed into one element
                        typedef ImageChannelDataTypeValToCType<OpenCL_UNORM_SHORT_565>::type CType;
                        res = CompareNScalarElements<CType>(
                            (const CType *) ptrAct, (const CType *) ptrRef, ptrNEAT,
                            1 // one channel
                            );
                        break;
                    }
                    case OpenCL_UNORM_INT_101010 :
                    {   // special case. treat OpenCL_UNORM_INT_101010 as
                        // one element. all channels are packed into one element
                        typedef ImageChannelDataTypeValToCType<OpenCL_UNORM_INT_101010>::type CType;
                        res = CompareNScalarElements<CType>(
                            (const CType *) ptrAct, (const CType *) ptrRef, ptrNEAT,
                            1 // one channel
                            );
                        break;
                    }
                    // TODO: enable CFloat16 when Validation::Utils function support CFloat16
                    //CASE_TYPE_IMAGES(OpenCL_HALF_FLOAT, CFloat16)
                    default:
                        throw Exception::InvalidArgument("Unknown ImageChannelDataType");
                }
                // update function return value
                toReturn = (res == NOT_PASSED) ? NOT_PASSED : toReturn;
            } // for(uint64_t x = 0; x < Width; ++x)
        } // for(uint64_t y = 0; y < Height; ++y){
        if(Is3D) m_indexStack.popIdx(); // z
    } // for(uint64_t z = 0; z < Depth; ++z)
    return toReturn;
}
#undef CASE_TYPE_IMAGES


