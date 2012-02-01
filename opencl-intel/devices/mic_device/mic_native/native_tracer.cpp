#include "mic_tracer.h"

#include <sink/COIPipeline_sink.h>

#include <assert.h>

using namespace Intel::OpenCL::MICDevice;

// Global instance of tracer
DeviceTracer gTracer;

#ifdef ENABLE_MIC_TRACER

// Return the size in bytes of the trace (in in_pReturnValue as uint64_t)
COINATIVELIBEXPORT
void get_trace_size(uint32_t         in_BufferCount,
			     void**           in_ppBufferPointers,
				 uint64_t*        in_pBufferLengths,
				 void*            in_pMiscData,
				 uint16_t         in_MiscDataLength,
				 void*            in_pReturnValue,
				 uint16_t         in_ReturnValueLength)
{
	assert(in_pReturnValue);
	assert(in_ReturnValueLength == sizeof(uint64_t));
	uint64_t* size = (uint64_t*)in_pReturnValue;
	*size = gTracer.get_trace_size();
}

// Return a copy of the tracer data
COINATIVELIBEXPORT
void get_trace(uint32_t         in_BufferCount,
			     void**           in_ppBufferPointers,
				 uint64_t*        in_pBufferLengths,
				 void*            in_pMiscData,
				 uint16_t         in_MiscDataLength,
				 void*            in_pReturnValue,
				 uint16_t         in_ReturnValueLength)
{
	assert(in_BufferCount == 1);
	assert(in_ppBufferPointers);
	assert(in_pBufferLengths);
	gTracer.getTracerCopy(in_ppBufferPointers[0], in_pBufferLengths[0]);
}



size_t DeviceTracer::get_trace_size()
{
	pthread_mutex_lock(&m_mutex);
	size_t size = m_cmdTracerDataArr.size() * sizeof(CommandTracer::COMMAND_DATA);
	pthread_mutex_unlock(&m_mutex);
	return size;
}

void DeviceTracer::getTracerCopy(void* out, size_t size)
{
	assert(out);
	assert(size == get_trace_size());
	CommandTracer::COMMAND_DATA* cmdTracersArr = (CommandTracer::COMMAND_DATA*)out;
	assert(cmdTracersArr);
	pthread_mutex_lock(&m_mutex);
	for (unsigned int i = 0; i < m_cmdTracerDataArr.size(); i++)
	{
		cmdTracersArr[i] = *(m_cmdTracerDataArr[i]);
	}
	pthread_mutex_unlock(&m_mutex);
}

#endif
