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

#include <iostream>
#include "CL/cl_platform.h"
#include "CL/cl.h"
#include "test_utils.h"
#include "TestsHelpClasses.h"

#define IMAGE_ELEM_SIZE     4
// no real meaning for these numbers, I just need IMAGE_WIDTH * IMAGE_ELEM_SIZE to be a multiple of CL_DEVICE_IMAGE_PITCH_ALIGNMENT
#define IMAGE_WIDTH         608
#define IMAGE_HEIGHT        757
#define IMAGE_DEPTH         4
#define IMAGE_ARRAY_SIZE    10
#define PITCH_RATIO         2

extern cl_device_type gDeviceType;

static void FillData(char* pData, size_t szSize)
{
    for (size_t i = 0; i < szSize; i++)
    {
        pData[i] = (char)i;
    }
}

static void TestInvalidFlags(cl_context context, const cl_image_format& clImgFormat, const cl_image_desc& clImgDesc)
{
    const cl_mem_flags invalidFlags =
        ~(CL_MEM_READ_WRITE |
          CL_MEM_WRITE_ONLY |
          CL_MEM_READ_ONLY |
          CL_MEM_USE_HOST_PTR |
          CL_MEM_ALLOC_HOST_PTR |
          CL_MEM_COPY_HOST_PTR |
          CL_MEM_HOST_WRITE_ONLY |
          CL_MEM_HOST_READ_ONLY |
          CL_MEM_HOST_NO_ACCESS);
    cl_int iRet;

    clCreateBuffer(context, invalidFlags, 1, NULL, &iRet);
    CheckException("clCreateBuffer", CL_INVALID_VALUE, iRet);
    
    clMemWrapper buf = clCreateBuffer(context, 0, 1000, NULL, &iRet);
    CheckException("clCreateBuffer", CL_SUCCESS, iRet);
    clCreateSubBuffer(buf, invalidFlags, CL_BUFFER_CREATE_TYPE_REGION, NULL, &iRet);
    CheckException("clCreateSubBuffer", CL_INVALID_VALUE, iRet);

    clCreateImage(context, invalidFlags, &clImgFormat, &clImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_VALUE, iRet);
}

static void TestWriteReadImgArray(cl_command_queue queue, cl_mem clImgArr, cl_mem_object_type type, const cl_image_desc& clImgDesc)
{
    bool bResult = true;
    size_t origin[3], region[3];

    origin[0] = region[0] = clImgDesc.image_width / 3;
    if (CL_MEM_OBJECT_IMAGE1D_ARRAY == type)
    {
        origin[1] = region[1] = clImgDesc.image_array_size / 3;        
        origin[2] = 0;
        region[2] = 1;
    }
    else
    {
        origin[1] = region[1] = clImgDesc.image_height / 3;
        origin[2] = region[2] = clImgDesc.image_array_size / 3;
    }

    const size_t szDataSize = region[0] * region[1] * region[2] * IMAGE_ELEM_SIZE;
    char* const pSrcData = new char[szDataSize], * const pDstData = new char[szDataSize];

    FillData(pSrcData, szDataSize);
    try
    {
        cl_int iRet = clEnqueueWriteImage(queue, clImgArr, CL_TRUE, origin, region, 0, 0, pSrcData, 0, NULL, NULL);
        CheckException("clEnqueueWriteImage", CL_SUCCESS, iRet);
        iRet = clEnqueueReadImage(queue, clImgArr, CL_TRUE, origin, region, 0, 0, pDstData, 0, NULL, NULL);
        CheckException("clEnqueueReadImage", CL_SUCCESS, iRet);
        bResult = memcmp(pSrcData, pDstData, szDataSize) == 0;
        if (!bResult)
        {
            std::cout << "source data differs from destination data" << std::endl;
        }
    }
    catch (const std::exception&)
    {
        bResult = false;
    }
    delete[] pSrcData;
    delete[] pDstData;
    if (!bResult)
    {
        throw std::exception();
    }
}

static void TestImageFromMemObject(cl_command_queue queue, cl_mem clMemObj, cl_mem clImg, const cl_image_desc& clImgDesc, bool bIsFromBuffer)
{
    bool bResult = true;
	const size_t szHeight = CL_MEM_OBJECT_IMAGE1D_BUFFER == clImgDesc.image_type ? 1 : clImgDesc.image_height;
	const size_t szDataSize = clImgDesc.image_width * szHeight * IMAGE_ELEM_SIZE;
    char* const pSrcData = new char[szDataSize], *const pDstData = new char[szDataSize];
	const size_t origin[3] = {0}, region[3] = {clImgDesc.image_width, szHeight, 1};

    FillData(pSrcData, szDataSize);
    try
    {
		cl_int iRet;
		if (bIsFromBuffer)
		{
			iRet = clEnqueueWriteBuffer(queue, clMemObj, CL_TRUE, 0, szDataSize, pSrcData, 0, NULL, NULL);
		}
		else
		{
			iRet = clEnqueueWriteImage(queue, clMemObj, CL_TRUE, origin, region, 0, 0, pSrcData, 0, NULL, NULL);
		}
        CheckException("clEnqueueWriteBuffer", CL_SUCCESS, iRet);
        iRet = clEnqueueReadImage(queue, clImg, CL_TRUE, origin, region, 0, 0, pDstData, 0, NULL, NULL);
        CheckException("clEnqueueReadImage", CL_SUCCESS, iRet);
        bResult = memcmp(pSrcData, pDstData, szDataSize) == 0;
        if (!bResult)
        {
            std::cout << "source data differs from destination data" << std::endl;
        }

		if (bIsFromBuffer)
		{
			cl_mem clBufInfo;
			size_t clBufInfoSize;
			iRet = clGetImageInfo(clImg, CL_IMAGE_BUFFER, sizeof(clBufInfo), &clBufInfo, &clBufInfoSize);
			CheckException("clGetImageInfo", CL_SUCCESS, iRet);
			CheckException("CL_IMAGE_BUFFER", clMemObj, clBufInfo);
			CheckException("clBufInfoSize", sizeof(clBufInfo), clBufInfoSize);
		}
    }
    catch (const std::exception&)
    {
        bResult = false;
    }
    delete[] pSrcData;
    delete[] pDstData;
    if (!bResult)
    {
        throw std::exception();
    }
}

static void CalcImageDimsInBytes(size_t szRowPitch, size_t szSlicePitch, const size_t* region, size_t* pDimsInBytes)
{
    // reorganize the actual pitches: we need this, since the pitch of each single 1D image in a 1D image array is given by the slice image
    const size_t szPitches[] = {
        szRowPitch != 0 ? szRowPitch : (szSlicePitch != 0 ? szSlicePitch : 0),
        szRowPitch == 0 && szSlicePitch != 0 ? 0 : szSlicePitch
    };

    if (szPitches[0] > 0)
    {
        pDimsInBytes[0] = szPitches[0];
    }
    else
    {
        pDimsInBytes[0] = IMAGE_ELEM_SIZE * region[0];
    }
    if (szPitches[1] > 0)
    {
        pDimsInBytes[1] = szPitches[1];
    }
    else
    {
        pDimsInBytes[1] = pDimsInBytes[0] * region[1];
    }
}

static void TestImageMapping(cl_command_queue queue, cl_mem clImg, const size_t* origin, const size_t* region, size_t szExpectedRowPitch, size_t szExpectedSlicePitch,
                             size_t szSrcRowPitch, size_t szSrcSlicePitch, cl_event clCopyEvent, void* pSrcData, size_t szDataSize)
{
    size_t szRowPitch, szSlicePitch;
    cl_int iRet;

    clEnqueueMapImage(queue, clImg, CL_TRUE, 0, origin, region, &szRowPitch, &szSlicePitch, 1, NULL, NULL, &iRet);
    CheckException("clEnqueueMapImage", CL_INVALID_EVENT_WAIT_LIST, iRet);

    void* const pMappedData = clEnqueueMapImage(queue, clImg, CL_TRUE, 0, origin, region, &szRowPitch, &szSlicePitch, 1, &clCopyEvent, NULL, &iRet);
    CheckException("clEnqueueMapImage", CL_SUCCESS, iRet);
    CheckException("szExpectedRowPitch", szExpectedRowPitch, szRowPitch);
    CheckException("szExpectedSlicePitch", szExpectedSlicePitch, szSlicePitch);
    if (!pMappedData)
    {
        std::cout << "FAIL: clEnqueueMapImage returned NU" << std::endl;
        throw std::exception();
    }

    size_t szSrcDimsInBytes[2], szMappedDimsInBytes[2];
    CalcImageDimsInBytes(szSrcRowPitch, szSrcSlicePitch, region, szSrcDimsInBytes);
    CalcImageDimsInBytes(szRowPitch, szSlicePitch, region, szMappedDimsInBytes);

    // we assume that origin and region cover the whole image
    char* pSrcPtr = (char*)pSrcData, *pMappedPtr = (char*)pMappedData;
    for (size_t szSlice = 0; szSlice < region[2]; szSlice++)
    {
        for (size_t szRow = 0; szRow < region[1]; szRow++)
        {
            if (memcmp(pSrcPtr, pMappedPtr, region[0] * IMAGE_ELEM_SIZE) != 0)
            {
                std::cout << "mapped data differs from source at slice " << szSlice << " row " << szRow << std::endl;
                throw std::exception();
            }
            pSrcPtr += szSrcDimsInBytes[0];
            pMappedPtr += szMappedDimsInBytes[0];
        }
        pSrcPtr += szSrcDimsInBytes[1] - szSrcDimsInBytes[0] * region[1];
        pMappedPtr += szMappedDimsInBytes[1] - szMappedDimsInBytes[0] * region[1];
    }

    iRet = clEnqueueUnmapMemObject(queue, clImg, pMappedData, 0, NULL, NULL);
    CheckException("clEnqueueUnmapMemObject", CL_SUCCESS, iRet);
}

static void TestHostPtr(cl_context context, cl_command_queue queue, cl_mem_object_type clMemObjType, const cl_image_desc& clImgDesc, const cl_image_format& format)

{
    assert(clImgDesc.image_row_pitch == 0);
    assert(clImgDesc.image_slice_pitch == 0);

    size_t clMemSize, szExpectedSrcRowPitch = 0, szExpectedSrcSlicePitch = 0, szExpectedDstRowPitch = 0, szExpectedDstSlicePitch = 0, szHostDataSize,
        region[3] = { clImgDesc.image_width, 1, 1};   // default values for region, they will be changed according to the image type
    cl_image_desc localClImgDesc = clImgDesc;
    cl_int iRet;
    const size_t origin[3] = {0};

    localClImgDesc.image_type = clMemObjType;
    switch (clMemObjType)
    {
    case CL_MEM_OBJECT_IMAGE1D:
        szHostDataSize = IMAGE_ELEM_SIZE * clImgDesc.image_width;
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        region[1] = clImgDesc.image_height;
        szExpectedSrcRowPitch = (size_t)(IMAGE_ELEM_SIZE * clImgDesc.image_width * PITCH_RATIO);
        szExpectedDstRowPitch = IMAGE_ELEM_SIZE * clImgDesc.image_width;
        szHostDataSize = szExpectedSrcRowPitch * clImgDesc.image_height;
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        region[1] = clImgDesc.image_height;
        region[2] = clImgDesc.image_depth;
        szExpectedSrcRowPitch = (size_t)(IMAGE_ELEM_SIZE * clImgDesc.image_width * PITCH_RATIO);
        szExpectedDstRowPitch = IMAGE_ELEM_SIZE * clImgDesc.image_width;
        szExpectedSrcSlicePitch = szExpectedSrcRowPitch * (size_t)(clImgDesc.image_height * PITCH_RATIO);
        szExpectedDstSlicePitch = szExpectedDstRowPitch * clImgDesc.image_height;
        szHostDataSize = szExpectedSrcSlicePitch * clImgDesc.image_depth;
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        region[1] = clImgDesc.image_array_size;
        szExpectedSrcSlicePitch = szExpectedSrcRowPitch = (size_t)(IMAGE_ELEM_SIZE * clImgDesc.image_width * PITCH_RATIO);
        szExpectedDstSlicePitch = szExpectedDstRowPitch = IMAGE_ELEM_SIZE * clImgDesc.image_width;
        szHostDataSize = szExpectedSrcSlicePitch * clImgDesc.image_array_size;
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        region[1] = clImgDesc.image_height;
        region[2] = clImgDesc.image_array_size;
        szExpectedSrcRowPitch = (size_t)(IMAGE_ELEM_SIZE * clImgDesc.image_width * PITCH_RATIO);
        szExpectedDstRowPitch = IMAGE_ELEM_SIZE * clImgDesc.image_width;
        szExpectedSrcSlicePitch = szExpectedSrcRowPitch * (size_t)(clImgDesc.image_height * PITCH_RATIO);
        szExpectedDstSlicePitch = szExpectedDstRowPitch * clImgDesc.image_height;
        szHostDataSize = szExpectedSrcSlicePitch * clImgDesc.image_array_size;
        break;
    default:
        assert(false);
    }
    localClImgDesc.image_row_pitch = szExpectedSrcRowPitch;
    localClImgDesc.image_slice_pitch = szExpectedSrcSlicePitch;

    bool bResult = true;
    char* const pHostData = new char[szHostDataSize];
    cl_event clCopyEvent = NULL;
    FillData(pHostData, szHostDataSize);
    try
    {
        // create source and destination images
        clMemWrapper clSrcImg = clCreateImage(context, CL_MEM_COPY_HOST_PTR, &format, &localClImgDesc, pHostData, &iRet);
        CheckException("clCreateImage", CL_SUCCESS, iRet);
        localClImgDesc.image_row_pitch = localClImgDesc.image_slice_pitch = 0;
        clMemWrapper clDstImg = clCreateImage(context, 0, &format, &localClImgDesc, NULL, &iRet);
        CheckException("clCreateImage", CL_SUCCESS, iRet);

        // check query of the image's memory size
        iRet = clGetMemObjectInfo(clSrcImg, CL_MEM_SIZE, sizeof(clMemSize), &clMemSize, NULL);
        CheckException("clGetMemObjectInfo", CL_SUCCESS, iRet);
        CheckException("CL_MEM_SIZE", szHostDataSize, clMemSize);

        // copy images        
        iRet = clEnqueueCopyImage(queue, clSrcImg, clDstImg, origin, origin, region, 0, NULL, &clCopyEvent);
        CheckException("clEnqueueCopyImage", CL_SUCCESS, iRet);

        TestImageMapping(queue, clSrcImg, origin, region, szExpectedSrcRowPitch, szExpectedSrcSlicePitch, szExpectedSrcRowPitch, szExpectedSrcSlicePitch, clCopyEvent,
            pHostData, szHostDataSize);
        TestImageMapping(queue, clDstImg, origin, region, szExpectedDstRowPitch, szExpectedDstSlicePitch, szExpectedSrcRowPitch, szExpectedSrcSlicePitch, clCopyEvent,
            pHostData, szHostDataSize);
    }
    catch (const std::exception&)
    {
        bResult = false;
    }
    delete[] pHostData;
    if (clCopyEvent)
    {
        iRet = clReleaseEvent(clCopyEvent);
        CheckException("clReleaseEvent", CL_SUCCESS, iRet);
    }    
    if (!bResult)
    {
        throw std::exception();
    }
}

static void TestBufAnd1DImgBufFlagsMismatch(cl_mem_flags bufFlags, cl_mem_flags imgFlags, cl_context context, const cl_image_desc& clImgDesc, const cl_image_format& clFormat)
{
    cl_int iRet;
    cl_image_desc localImgDesc = clImgDesc;

    clMemWrapper clBuf = clCreateBuffer(context, bufFlags, 1000, NULL, &iRet);
    CheckException("clCreateBuffer", CL_SUCCESS, iRet);
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D_BUFFER;
    localImgDesc.mem_object = clBuf;
    clCreateImage(context, imgFlags, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_VALUE, iRet);
}

static void TestInvalidBounds(cl_mem img, size_t index, cl_command_queue queue)
{
    const size_t goodOrigin[] = {0, 0, 0}, goodRegion[] = {1, 1, 1};
    size_t badOrigin[] = {0, 0, 0}, badRegion[] = {1, 1, 1};
    char buf;
    cl_int iRet;

    badOrigin[index] = 1;
    badRegion[index] = 0;
    iRet = clEnqueueReadImage(queue, img, CL_TRUE, badOrigin, goodRegion, 0, 0, &buf, 0, NULL, NULL);
    CheckException("clEnqueueReadImage", CL_INVALID_VALUE, iRet);
    iRet = clEnqueueReadImage(queue, img, CL_TRUE, goodOrigin, badRegion, 0, 0, &buf, 0, NULL, NULL);
    CheckException("clEnqueueReadImage", CL_INVALID_VALUE, iRet);
}

static size_t GetMaxValue(cl_device_id device, cl_device_info clDevInfo)
{
    size_t val;

    const cl_int iRet = clGetDeviceInfo(device, clDevInfo, sizeof(val), &val, NULL);
    CheckException("clGetDeviceInfo", CL_SUCCESS, iRet);
    return val;
}

// we assume that szDim is a reference to a field in clImgDesc
static void TestInvalidDim(cl_context context, const cl_image_format& clImgFormat, const cl_image_desc& clImgDesc, size_t szMaxVal, size_t& szDim)
{
    cl_int iRet;
    const size_t szOrigDim = szDim;

    szDim = 0;
    clCreateImage(context, 0, &clImgFormat, &clImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);
    if (szMaxVal > 0)
    {
        szDim = szMaxVal + 1;
        clCreateImage(context, 0, &clImgFormat, &clImgDesc, NULL, &iRet);
        CheckException("clCreateImage", CL_INVALID_IMAGE_SIZE, iRet);
        szDim = szOrigDim;
    }
}

static void TestNegative(const cl_image_format& clFormat, const cl_image_desc& clImageDesc, cl_context context, cl_device_id device, cl_command_queue queue)
{
    cl_context badContext = (cl_context)-1;
    cl_int iRet;
    cl_image_desc localImgDesc = clImageDesc;

    // some good images
    localImgDesc = clImageDesc;
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
    clMemWrapper img3d = clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_SUCCESS, iRet);
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
    clMemWrapper img2dArr = clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_SUCCESS, iRet);
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    clMemWrapper img2d = clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_SUCCESS, iRet);    
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
    clMemWrapper img1d = clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_SUCCESS, iRet);    
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
    clMemWrapper img1dArr = clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_SUCCESS, iRet);
    const size_t szBufSize = 1000;
    localImgDesc.image_width = szBufSize;
    clMemWrapper clBuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, szBufSize, NULL, &iRet);
    CheckException("clCreateBuffer", CL_SUCCESS, iRet);
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D_BUFFER;
    localImgDesc.mem_object = clBuf;
    localImgDesc.image_width = szBufSize / IMAGE_ELEM_SIZE;
    clMemWrapper img1dBuf = clCreateImage(context, CL_MEM_WRITE_ONLY, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_SUCCESS, iRet);

    // get maximum values
    const size_t
        szImgMaxArrSize = GetMaxValue(device, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE),
        szImg2dMaxWidth = GetMaxValue(device, CL_DEVICE_IMAGE2D_MAX_WIDTH),
        szImg2dMaxHeight = GetMaxValue(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT),
        szImg3dMaxWidth = GetMaxValue(device, CL_DEVICE_IMAGE3D_MAX_WIDTH),
        szImg3dMaxHeight = GetMaxValue(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT),
        szImg3dMaxDepth = GetMaxValue(device, CL_DEVICE_IMAGE3D_MAX_DEPTH),
        szImgMaxBufSize = GetMaxValue(device, CL_DEVICE_IMAGE_MAX_BUFFER_SIZE);

    // bad context
    localImgDesc = clImageDesc;
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
    clCreateImage(badContext, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_CONTEXT, iRet);
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
    clCreateImage(badContext, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_CONTEXT, iRet);    

    // host_ptr isn't NULL, buf flags is 0
    char buf;
    clCreateImage(context, 0, &clFormat, &localImgDesc, &buf, &iRet);
    CheckException("clCreateImage", CL_INVALID_HOST_PTR, iRet);

    // trying to create an image array with invalid image descriptor    
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
    localImgDesc.image_array_size = 0;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);
    localImgDesc.image_array_size = szImgMaxArrSize + 1;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_SIZE, iRet);
    localImgDesc.image_array_size = clImageDesc.image_array_size;
    localImgDesc.image_width = 0;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);
    localImgDesc.image_width = szImg2dMaxWidth + 1;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_SIZE, iRet);
    localImgDesc.image_width = clImageDesc.image_width;
    localImgDesc.image_height = 0;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);
    localImgDesc.image_height = szImg2dMaxHeight + 1;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_SIZE, iRet);

    // calling clGetSupportedImageFormats for object type CL_MEM_OBJECT_BUFFER
    cl_uint uiNumImageFormats;
    iRet = clGetSupportedImageFormats(context, 0, CL_MEM_OBJECT_BUFFER, 0, NULL, &uiNumImageFormats);
    CheckException("clGetSupportedImageFormats", CL_INVALID_VALUE, iRet);

    // trying to create a 1D image buffer with localImgDesc.mem_object NULL
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D_BUFFER;
    localImgDesc.mem_object = NULL;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

    // mismatch in flags between buffer and its 1D image buffer
    TestBufAnd1DImgBufFlagsMismatch(CL_MEM_WRITE_ONLY, CL_MEM_READ_WRITE, context, clImageDesc, clFormat);
    TestBufAnd1DImgBufFlagsMismatch(CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY, context, clImageDesc, clFormat);
    TestBufAnd1DImgBufFlagsMismatch(CL_MEM_READ_ONLY, CL_MEM_READ_WRITE, context, clImageDesc, clFormat);
    TestBufAnd1DImgBufFlagsMismatch(CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY, context, clImageDesc, clFormat);
    TestBufAnd1DImgBufFlagsMismatch(0, CL_MEM_USE_HOST_PTR, context, clImageDesc, clFormat);
    TestBufAnd1DImgBufFlagsMismatch(0, CL_MEM_ALLOC_HOST_PTR, context, clImageDesc, clFormat);
    TestBufAnd1DImgBufFlagsMismatch(0, CL_MEM_COPY_HOST_PTR, context, clImageDesc, clFormat);
    TestBufAnd1DImgBufFlagsMismatch(CL_MEM_HOST_WRITE_ONLY, CL_MEM_HOST_READ_ONLY, context, clImageDesc, clFormat);
    TestBufAnd1DImgBufFlagsMismatch(CL_MEM_HOST_READ_ONLY, CL_MEM_HOST_WRITE_ONLY, context, clImageDesc, clFormat);
    TestBufAnd1DImgBufFlagsMismatch(CL_MEM_HOST_NO_ACCESS, CL_MEM_HOST_READ_ONLY, context, clImageDesc, clFormat);
    TestBufAnd1DImgBufFlagsMismatch(CL_MEM_HOST_NO_ACCESS, CL_MEM_HOST_WRITE_ONLY, context, clImageDesc, clFormat);

    // trying to create a 1D image buffer that is larger than its buffer    
    localImgDesc.mem_object = clBuf;
    clCreateImage(context, CL_MEM_WRITE_ONLY, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

    // trying to create a 1D image buffer whose width is larger than CL_DEVICE_IMAGE_MAX_BUFFER_SIZE
    cl_ulong szMaxMemAllocSize;
    iRet = clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(szMaxMemAllocSize), &szMaxMemAllocSize, NULL);
    CheckException("clGetDeviceInfo", CL_SUCCESS, iRet);
    if (szMaxMemAllocSize > szImgMaxBufSize * IMAGE_ELEM_SIZE)
    {
        clMemWrapper clBigBuf = clCreateBuffer(context, 0, szMaxMemAllocSize <= (size_t)-1 ? (size_t)szMaxMemAllocSize : (size_t)-1, NULL, &iRet);
        if (CL_OUT_OF_HOST_MEMORY != iRet)
        {
            CheckException("clCreateBuffer", CL_SUCCESS, iRet);
            localImgDesc.mem_object = clBigBuf;
            localImgDesc.image_width = szImgMaxBufSize + 1;
            clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
            CheckException("clCreateImage", CL_INVALID_IMAGE_SIZE, iRet);
        }
    }

    // invalid bounds

    TestInvalidBounds(img2d, 2, queue);
    TestInvalidBounds(img1dArr, 2, queue);
    TestInvalidBounds(img1d, 1, queue);
    TestInvalidBounds(img1d, 2, queue);
    TestInvalidBounds(img1dBuf, 1, queue);
    TestInvalidBounds(img1dBuf, 2, queue);

    // negative testing for clGetImageInfo of 1D image buffer    
    iRet = clGetImageInfo(img1dBuf, CL_IMAGE_BUFFER, 0, NULL, NULL);
    CheckException("clGetImageInfo", CL_INVALID_VALUE, iRet);
    cl_mem clBufInfo;
    iRet = clGetImageInfo(img1dBuf, CL_IMAGE_BUFFER, 0, &clBufInfo, NULL);
    CheckException("clGetImageInfo", CL_INVALID_VALUE, iRet);

    // invalid origin and region for image array    
    const size_t overflowedOrigin[] = {0, localImgDesc.image_array_size, 0}, goodRegion[] = {1, 1, 1};
    iRet = clEnqueueReadImage(queue, img1dArr, CL_TRUE, overflowedOrigin, goodRegion, 0, 0, &buf, 0, NULL, NULL);
    CheckException("clEnqueueReadImage", CL_INVALID_VALUE, iRet);
    const size_t goodOrigin[] = {0, 0, 0}, badRegion[] = {1, 1, 0};
    iRet = clEnqueueReadImage(queue, img1dArr, CL_TRUE, goodOrigin, badRegion, 0, 0, &buf, 0, NULL, NULL);
    CheckException("clEnqueueReadImage", CL_INVALID_VALUE, iRet);

    // check memory overlap
    iRet = clEnqueueCopyImage(queue, img1dArr, img1dArr, goodOrigin, goodOrigin, goodRegion, 0, NULL, NULL);
    CheckException("clEnqueueCopyImage", CL_MEM_COPY_OVERLAP, iRet);
    iRet = clEnqueueCopyImage(queue, img1dBuf, img1dBuf, goodOrigin, goodOrigin, goodRegion, 0, NULL, NULL);
    CheckException("clEnqueueCopyImage", CL_MEM_COPY_OVERLAP, iRet);

    // try to read from host write-only image and vice versa
    clMemWrapper writeOnlyImg = clCreateImage(context, CL_MEM_HOST_WRITE_ONLY, &clFormat, &clImageDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_SUCCESS, iRet);
    iRet = clEnqueueReadImage(queue, writeOnlyImg, CL_TRUE, goodOrigin, goodRegion, 0, 0, &buf, 0, NULL, NULL);
    CheckException("clEnqueueReadImage", CL_INVALID_OPERATION, iRet);
    clMemWrapper readOnlyImg = clCreateImage(context, CL_MEM_HOST_READ_ONLY, &clFormat, &clImageDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_SUCCESS, iRet);
    iRet = clEnqueueWriteImage(queue, readOnlyImg, CL_TRUE, goodOrigin, goodRegion, 0, 0, &buf, 0, NULL, NULL);
    CheckException("clEnqueueWriteImage", CL_INVALID_OPERATION, iRet);

    // try to copy a 1D image buffer to its own buffer and vice versa
    iRet = clEnqueueCopyImageToBuffer(queue, img1dBuf, clBuf, goodOrigin, goodRegion, 0, 0, NULL, NULL);
    CheckException("clEnqueueCopyImageToBuffer", CL_INVALID_MEM_OBJECT, iRet);
    iRet = clEnqueueCopyBufferToImage(queue, clBuf, img1dBuf, 0, goodOrigin, goodRegion, 0, NULL, NULL);
    CheckException("clEnqueueCopyBufferToImage", CL_INVALID_MEM_OBJECT, iRet);

    // invalid image parameters
    localImgDesc = clImageDesc;
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
    clCreateImage(context, CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_VALUE, iRet);

    // invalid image descriptor
    clCreateImage(context, 0, &clFormat, NULL, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);
    localImgDesc = clImageDesc;
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
    localImgDesc.num_mip_levels = 1;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);
    localImgDesc.num_mip_levels = 0;
    localImgDesc.num_samples = 1;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);
    localImgDesc.num_samples = 0;
    localImgDesc.mem_object = clBuf;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

    // invalid slice pitches
    localImgDesc = clImageDesc;
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
    localImgDesc.image_row_pitch = IMAGE_ELEM_SIZE * localImgDesc.image_width;
    localImgDesc.image_slice_pitch = (IMAGE_ELEM_SIZE * localImgDesc.image_width * localImgDesc.image_height) - 1;  // slice pitch smaller than row pitch * height
    clCreateImage(context, CL_MEM_USE_HOST_PTR, &clFormat, &localImgDesc, &buf, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);
    localImgDesc.image_slice_pitch = (IMAGE_ELEM_SIZE * localImgDesc.image_width * localImgDesc.image_height) + 1;  // slice pitch not multiple of row pitch
    clCreateImage(context, CL_MEM_USE_HOST_PTR, &clFormat, &localImgDesc, &buf, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

    // NULL pitch pointers
    size_t szImgRowPitch;
    clEnqueueMapImage(queue, img1d, CL_TRUE, 0, goodOrigin, goodRegion, NULL, NULL, 0, NULL, NULL, &iRet);
    CheckException("clEnqueueMapImage", CL_INVALID_VALUE, iRet);    
    clEnqueueMapImage(queue, img3d, CL_TRUE, 0, goodOrigin, goodRegion, &szImgRowPitch, NULL, 0, NULL, NULL, &iRet);
    CheckException("clEnqueueMapImage", CL_INVALID_VALUE, iRet);
    clEnqueueMapImage(queue, img1dArr, CL_TRUE, 0, goodOrigin, goodRegion, &szImgRowPitch, NULL, 0, NULL, NULL, &iRet);
    CheckException("clEnqueueMapImage", CL_INVALID_VALUE, iRet);
    clEnqueueMapImage(queue, img2dArr, CL_TRUE, 0, goodOrigin, goodRegion, &szImgRowPitch, NULL, 0, NULL, NULL, &iRet);
    CheckException("clEnqueueMapImage", CL_INVALID_VALUE, iRet);

    // test invalid dimensions
    localImgDesc = clImageDesc;
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
    TestInvalidDim(context, clFormat, localImgDesc, szImg2dMaxWidth, localImgDesc.image_width);
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    TestInvalidDim(context, clFormat, localImgDesc, szImg2dMaxWidth, localImgDesc.image_width);
    TestInvalidDim(context, clFormat, localImgDesc, szImg2dMaxHeight, localImgDesc.image_height);
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
    TestInvalidDim(context, clFormat, localImgDesc, szImg3dMaxWidth, localImgDesc.image_width);
    TestInvalidDim(context, clFormat, localImgDesc, szImg3dMaxHeight, localImgDesc.image_height);
    TestInvalidDim(context, clFormat, localImgDesc, szImg3dMaxDepth, localImgDesc.image_depth);  
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE1D_BUFFER;
    localImgDesc.mem_object = clBuf;
    TestInvalidDim(context, clFormat, localImgDesc, 0, localImgDesc.image_width);

    // pitch different than 0 when hostptr is NULL
    localImgDesc = clImageDesc;
    localImgDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    localImgDesc.image_row_pitch = 1;
    clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

    // row pitch smaller than minimum
    clCreateImage(context, CL_MEM_USE_HOST_PTR, &clFormat, &localImgDesc, &buf, &iRet);
    CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

	// a 2D image from a 1D image	
	cl_image_desc desc;
	memset(&desc, 0, sizeof(cl_image_desc));
	cl_image_format format;
	desc.image_type = CL_MEM_OBJECT_IMAGE1D;
	desc.image_width = IMAGE_WIDTH;
	format.image_channel_order = CL_RGBA;
    format.image_channel_data_type = CL_UNSIGNED_INT8;
	
	img1d = clCreateImage(context, 0, &format, &desc, NULL, &iRet);
	CheckException("clCreateImage", CL_SUCCESS, iRet);

	desc.image_type = CL_MEM_OBJECT_IMAGE2D;
	desc.mem_object = img1d;
	clCreateImage(context, 0, &format, &desc, NULL, &iRet);
	CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

	iRet = clReleaseMemObject(img1d);
	CheckException("clReleaseMemObject", CL_SUCCESS, iRet);

	// 2D image created from buffer with pitch not a multiple of the maximum of the CL_DEVICE_IMAGE_PITCH_ALIGNMENT value of the device
	localImgDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
	localImgDesc.image_width = 5;
	localImgDesc.image_height = IMAGE_HEIGHT;
	cl_mem memBuf = clCreateBuffer(context, 0, IMAGE_ELEM_SIZE * localImgDesc.image_width * localImgDesc.image_height, NULL, &iRet);
	CheckException("clCreateBuffer", CL_SUCCESS, iRet);
	localImgDesc.mem_object = memBuf;

	img2d = clCreateImage(context, 0, &clFormat, &localImgDesc, NULL, &iRet);
	CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

	iRet = clReleaseMemObject(memBuf);
	CheckException("clReleaseMemObject", CL_SUCCESS, iRet);	
}

static void Test2DImageFromImage(cl_context context, cl_command_queue queue)
{
    cl_image_desc desc;
    memset(&desc, 0, sizeof(desc));
    cl_image_format format;
    cl_int iRet;

    desc.image_width = IMAGE_WIDTH;
    desc.image_height = IMAGE_HEIGHT;
    desc.image_type = CL_MEM_OBJECT_IMAGE2D;
    format.image_channel_data_type = CL_UNORM_INT8;
    format.image_channel_order = CL_sRGBA;

    cl_mem img = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc, NULL, &iRet);
    CheckException("clCreateImage", CL_SUCCESS, iRet);

    format.image_channel_order = CL_RGBA;
    desc.mem_object = img;
    cl_mem newImg = clCreateImage(context, 0, &format, &desc, NULL, &iRet);
    CheckException("clCreateImage", CL_SUCCESS, iRet);

    TestImageFromMemObject(queue, img, newImg, desc, false); 

    iRet = clReleaseMemObject(img);
    CheckException("clReleaseMemObject", CL_SUCCESS, iRet);
    iRet = clReleaseMemObject(newImg);
    CheckException("clReleaseMemObject", CL_SUCCESS, iRet);
}

bool clCreateImageTest()
{
    cl_int iRet = CL_SUCCESS;
    cl_platform_id platform = 0;
    bool bResult = true;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_image_format clFormat;
    memset(&clFormat, 0, sizeof(clFormat));
    cl_image_desc clImageDesc;
    memset(&clImageDesc, 0, sizeof(clImageDesc));
    clMemWrapper clImg1D, clBuffer, clImg1DBuffer, clImg2D, clImg2DOld, clImg3D, clImg3DOld, clImg1DArr, clImg2DArr, clImg2DBuffer;
    const cl_mem_object_type clImgTypes[] = {
        CL_MEM_OBJECT_IMAGE1D,
        CL_MEM_OBJECT_IMAGE2D,
        CL_MEM_OBJECT_IMAGE3D,
        CL_MEM_OBJECT_IMAGE1D_ARRAY,
        CL_MEM_OBJECT_IMAGE2D_ARRAY
    };

    std::cout << "=============================================================" << std::endl;
    std::cout << "clCreateImageTest" << std::endl;
    std::cout << "=============================================================" << std::endl;

    try
    {
        iRet = clGetPlatformIDs(1, &platform, NULL);
        CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);        
        iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
        CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);
        size_t szSize;

        const cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };    
        context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);    
        CheckException("clCreateContextFromType", CL_SUCCESS, iRet);    
        queue = clCreateCommandQueue(context, device, 0, &iRet);
        CheckException("clCreateCommandQueue", CL_SUCCESS, iRet);
        clFormat.image_channel_order = CL_RGBA;
        clFormat.image_channel_data_type = CL_UNSIGNED_INT8;
        clImageDesc.image_width = IMAGE_WIDTH;
        clImageDesc.image_height = IMAGE_HEIGHT;
        clImageDesc.image_depth = IMAGE_DEPTH;

        // FPGA emulator doesn't support images
        if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR)
        {
            // 1D image
            clImageDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
            clImg1D = clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
            CheckException("clCreateImage", CL_INVALID_OPERATION, iRet);

            // 2D image, old API
            clImageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
            clImg2DOld = clCreateImage2D(context, 0, &clFormat, clImageDesc.image_width,
                                         clImageDesc.image_height,
                                         clImageDesc.image_row_pitch, NULL, &iRet);
            CheckException("clCreateImage2D", CL_INVALID_OPERATION, iRet);

            // 3D image, old API
            clImageDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
            clImg3DOld = clCreateImage3D(context, 0, &clFormat, clImageDesc.image_width,
                                         clImageDesc.image_height, clImageDesc.image_depth,
                                         clImageDesc.image_row_pitch,
                                         clImageDesc.image_slice_pitch, NULL, &iRet);
            CheckException("clCreateImage3D", CL_INVALID_OPERATION, iRet);
        }
        else
        {
            // 1D image
            clImageDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
            clImg1D = clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
            CheckException("clCreateImage", CL_SUCCESS, iRet);

            // 1D image buffer

            clBuffer = clCreateBuffer(context, 0, clImageDesc.image_width * IMAGE_ELEM_SIZE, NULL, &iRet);
            CheckException("clCreateBuffer", CL_SUCCESS, iRet);

            clImageDesc.image_type = CL_MEM_OBJECT_IMAGE1D_BUFFER;
            clImageDesc.mem_object = clBuffer;
            clImg1DBuffer = clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
            CheckException("clCreateImage", CL_SUCCESS, iRet);
            clImageDesc.mem_object = NULL;

            TestImageFromMemObject(queue, clBuffer, clImg1DBuffer, clImageDesc, true);

            // 2D image
            clImageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
            clImg2D = clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
            CheckException("clCreateImage", CL_SUCCESS, iRet);

            // 2D image, old API
            clImg2DOld = clCreateImage2D(context, 0, &clFormat, clImageDesc.image_width, clImageDesc.image_height, clImageDesc.image_row_pitch, NULL, &iRet);
            CheckException("clCreateImage2D", CL_SUCCESS, iRet);

            // 2D image buffer
            clBuffer = clCreateBuffer(context, 0, clImageDesc.image_width * clImageDesc.image_height * IMAGE_ELEM_SIZE, NULL, &iRet);
            CheckException("clCreateBuffer", CL_SUCCESS, iRet);

            clImageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
            clImageDesc.mem_object = clBuffer;
            clImg2DBuffer = clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
            CheckException("clCreateImage", CL_SUCCESS, iRet);
            clImageDesc.mem_object = NULL;

            TestImageFromMemObject(queue, clBuffer, clImg2DBuffer, clImageDesc, true);

            // 2D image from another 2D image
            Test2DImageFromImage(context, queue);

            // 3D image
            clImageDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
            clImg3D = clCreateImage(context, CL_MEM_READ_WRITE, &clFormat, &clImageDesc, NULL, &iRet);
            CheckException("clCreateImage", CL_SUCCESS, iRet);

            // 3D image, old API
            clImg3DOld = clCreateImage3D(context, 0, &clFormat, clImageDesc.image_width, clImageDesc.image_height, clImageDesc.image_depth, clImageDesc.image_row_pitch,
                clImageDesc.image_slice_pitch, NULL, &iRet);
            CheckException("clCreateImage3D", CL_SUCCESS, iRet);

            // unsupported image type
            clImageDesc.image_type = 0xffff;
            clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
            CheckException("clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

            // 1D image array
            clImageDesc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
            clImageDesc.image_array_size = IMAGE_ARRAY_SIZE;
            clImg1DArr = clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
            CheckException("clCreateImage", CL_SUCCESS, iRet);

            size_t szImgArrSize;
            iRet = clGetImageInfo(clImg1DArr, CL_IMAGE_ARRAY_SIZE, sizeof(szImgArrSize), &szImgArrSize, &szSize);
            CheckException("clGetMemObjectInfo", CL_SUCCESS, iRet);
            CheckException("CL_IMAGE_ARRAY_SIZE", clImageDesc.image_array_size, szImgArrSize);
            CheckException("szSize", sizeof(szImgArrSize), szSize);

            // CL_IMAGE_ARRAY_SIZE for non image array objects should return 0 without an error
            iRet = clGetImageInfo(clImg3D, CL_IMAGE_ARRAY_SIZE, sizeof(szImgArrSize), &szImgArrSize, &szSize);
            CheckException("clGetMemObjectInfo", CL_SUCCESS, iRet);
            CheckException("CL_IMAGE_ARRAY_SIZE", (size_t)0, szImgArrSize);
            CheckException("szSize", sizeof(szImgArrSize), szSize);

            TestWriteReadImgArray(queue, clImg1DArr, CL_MEM_OBJECT_IMAGE1D_ARRAY, clImageDesc);

            // 2D image array
            clImageDesc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
            clImg2DArr = clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
            CheckException("clCreateImage", CL_SUCCESS, iRet);

            iRet = clGetImageInfo(clImg2DArr, CL_IMAGE_ARRAY_SIZE, sizeof(szImgArrSize), &szImgArrSize, &szSize);
            CheckException("clGetMemObjectInfo", CL_SUCCESS, iRet);
            CheckException("CL_IMAGE_ARRAY_SIZE", clImageDesc.image_array_size, szImgArrSize);
            CheckException("szSize", sizeof(szImgArrSize), szSize);

            // CL_IMAGE_NUM_MIP_LEVELS and CL_IMAGE_NUM_SAMPLES should be 0
            cl_uint uiNumMipLevels, uiNumSamples;
            iRet = clGetImageInfo(clImg2DArr, CL_IMAGE_NUM_MIP_LEVELS, sizeof(uiNumMipLevels), &uiNumMipLevels, &szSize);
            CheckException("clGetImageInfo", CL_SUCCESS, iRet);
            CheckException("CL_IMAGE_NUM_MIP_LEVELS", (cl_uint)0, uiNumMipLevels);
            CheckException("szSize", sizeof(uiNumMipLevels), szSize);
            iRet = clGetImageInfo(clImg2DArr, CL_IMAGE_NUM_SAMPLES, sizeof(uiNumSamples), &uiNumSamples, &szSize);
            CheckException("clGetImageInfo", CL_SUCCESS, iRet);
            CheckException("CL_IMAGE_NUM_SAMPLES", (cl_uint)0, uiNumSamples);
            CheckException("szSize", sizeof(uiNumSamples), szSize);

            TestWriteReadImgArray(queue, clImg2DArr, CL_MEM_OBJECT_IMAGE2D_ARRAY, clImageDesc);

            // host pointer
            for (size_t i = 0; i < sizeof(clImgTypes) / sizeof(clImgTypes[0]); i++)
            {
                TestHostPtr(context, queue, clImgTypes[i], clImageDesc, clFormat);
            }

            TestNegative(clFormat, clImageDesc, context, device, queue);

            TestInvalidFlags(context, clFormat, clImageDesc);
        }
    }
    catch (const std::exception&)
    {
        bResult = false;
    }
    if (context)
    {
        clReleaseContext(context);
    }
    if (queue)
    {
        clReleaseCommandQueue(queue);
    }
    return bResult;
}
