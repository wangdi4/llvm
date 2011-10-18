#include <assert.h>
using namespace Intel::OpenCL::MICDevice;

extern bool gSafeReleaseOfCoiObjects;

template <class T>
cl_dev_err_code RunFuncDataUsingMisc<T>::readData(T& data, void* pOutMiscData, const size_t size)
{
	assert(pOutMiscData);
	memcpy(pOutMiscData, data.pDataBuffer, size);
	return CL_DEV_SUCCESS;
}

template <class T>
cl_dev_err_code RunFuncDataUsingBuffer<T>::init(T& data, const size_t size, COIPROCESS* pCoiProcess)
{
	//TODO - Consider using pool of misc_data COIBUFFERs instead of creating new one for each kernel invokation
	// Create coi buffer for misc_data
	COIRESULT coi_err = COIBufferCreateFromMemory(
							size,				                             // The number of bytes to allocate for the buffer.
							COI_BUFFER_NORMAL,                               // The type of the buffer to create
							0,                                               // A bitmask of attributes for the newly created buffer.
							data.pDataBuffer,                                 // If non-NULL the buffer will be initialized with the data pointed
							1, pCoiProcess,                                  // The number of processes with which this buffer might be used, and The process
							&(data.coiBuffer));	   						     // Pointer to a buffer handle
	
	// Is the COIBufferCreate succeeded?
	if (COI_SUCCESS != coi_err)
	{
		return CL_DEV_OBJECT_ALLOC_FAIL;
	}

	return CL_DEV_SUCCESS;
}

template <class T>
void RunFuncDataUsingBuffer<T>::registerData(T& data, const COI_ACCESS_FLAGS& accessFlag, vector<COIBUFFER>& coiBuffsArr, vector<COI_ACCESS_FLAGS>& coiBuffsAccessFlag)
{
	assert(data.coiBuffer);
	coiBuffsArr.push_back(data.coiBuffer);
	coiBuffsAccessFlag.push_back(accessFlag);
	assert(coiBuffsArr.size() == coiBuffsAccessFlag.size());
}

template <class T>
cl_dev_err_code RunFuncDataUsingBuffer<T>::readData(T& data, void* pOutMiscData, const size_t size)
{
	assert(data.coiBuffer);
	// read m_miscBuffer in order to get kernel execution result and profiling data (synchronous)
	COIRESULT coi_err = COIBufferRead ( data.coiBuffer,
										0,
										pOutMiscData,
										size,
										COI_COPY_USE_CPU,
										0,
										NULL,
										NULL );

	// Is the COIBufferRead succeeded?
	if (COI_SUCCESS != coi_err)
	{
		return CL_DEV_ERROR_FAIL;
	}

	return CL_DEV_SUCCESS;
}

template <class T>
void RunFuncDataUsingBuffer<T>::release(T& data)
{
	if ((data.coiBuffer) && (gSafeReleaseOfCoiObjects))
	{
		COIBufferDestroy(data.coiBuffer);
	}
}