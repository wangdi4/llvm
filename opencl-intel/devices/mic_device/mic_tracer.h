// Copyright (c) 2006-2008 Intel Corporation
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

/*
*
* File mic_tracer.h
*
*/
#pragma once

#include <cstring>
#include <string.h>
#include <sstream>
#include <vector>
#include <stdio.h>
#include "mic_config.h"
#include "hw_utils.h"
#include "cl_sys_info.h"

using namespace std;

namespace Intel { namespace OpenCL { namespace MICDevice { 

#define MIC_TRACER_MAX_COUNTER_NAME_SIZE 32

#define MIC_TRACER_MAX_CHAR_POINTER_SIZE 32

#ifdef ENABLE_MIC_TRACER

#include <pthread.h>

#define TRACE_COMMAND_GENERAL(TYPE, NAME, NUM_OF_COUNTERS, INDEX) private: \
														static const unsigned int next_to_use_##NAME = INDEX + NUM_OF_COUNTERS; \
														void init_##NAME() \
                                                        { \
														    strncpy( m_data->m_valuesArr[INDEX].name, #NAME, MIC_TRACER_MAX_COUNTER_NAME_SIZE-1); \
															m_data->m_valuesArr[INDEX].name[MIC_TRACER_MAX_COUNTER_NAME_SIZE-1] = '\0'; \
															memset(&m_data->m_valuesArr[INDEX].value, 0, sizeof(VALUES_TYPE)); \
															m_data->m_valuesArr[INDEX].type = TRACE_unneeded; \
															if (NUM_OF_COUNTERS > 1) \
															{ \
															    for (unsigned int i = 1; i < NUM_OF_COUNTERS; i++) \
																{ \
																	m_data->m_valuesArr[INDEX + i].name[0] = '\0'; \
																	memset(&m_data->m_valuesArr[INDEX + i].value, 0, sizeof(VALUES_TYPE)); \
																	m_data->m_valuesArr[INDEX + i].type = TRACE_unneeded; \
																} \
															} \
                                                        }; \
														void set_valid_##NAME(unsigned int idx) \
														{ \
														   m_data->m_valuesArr[INDEX+idx].type = TRACE_##TYPE; \
														}

#define TRACE_COMMAND_SIMPLE1(TYPE, NAME, NUM_OF_COUNTERS, INDEX) TRACE_COMMAND_GENERAL(TYPE, NAME, NUM_OF_COUNTERS, INDEX); \
															public: \
																	void set_##NAME( const TYPE& in, unsigned int idx = 0) \
																	{ \
																	    set_valid_##NAME(idx); \
																		m_data->m_valuesArr[INDEX+idx].value.value_##TYPE = in; \
																	} 
#define TRACE_COMMAND_SIMPLE(TYPE, NAME, NUM_OF_COUNTERS, PREV_NAME) TRACE_COMMAND_SIMPLE1(TYPE, NAME, NUM_OF_COUNTERS, next_to_use_##PREV_NAME)

#define TRACE_COMMAND_STRING1(TYPE, NAME, NUM_OF_COUNTERS, INDEX) TRACE_COMMAND_GENERAL(TYPE, NAME, NUM_OF_COUNTERS, INDEX); \
															public: \
																	void set_##NAME( const TYPE& in, unsigned int idx = 0) \
																	{ \
																	    set_valid_##NAME(idx); \
																		strncpy(m_data->m_valuesArr[INDEX+idx].value.value_##TYPE, in, MIC_TRACER_MAX_CHAR_POINTER_SIZE - 1); \
																		m_data->m_valuesArr[INDEX+idx].value.value_##TYPE[MIC_TRACER_MAX_CHAR_POINTER_SIZE - 1] = '\0'; \
																	} 
#define TRACE_COMMAND_STRING(TYPE, NAME, NUM_OF_COUNTERS, PREV_NAME) TRACE_COMMAND_STRING1(TYPE, NAME, NUM_OF_COUNTERS, next_to_use_##PREV_NAME)

#define TRACE_COMMAND_COUNTER1(TYPE, NAME, NUM_OF_COUNTERS, INDEX) TRACE_COMMAND_GENERAL(TYPE, NAME, NUM_OF_COUNTERS, INDEX); \
												 public: \
														 void increment_##NAME(unsigned int idx = 0) \
														 { \
															 set_valid_##NAME(idx); \
														     m_data->m_valuesArr[INDEX+idx].value.value_##TYPE ++; \
														 } \
														 void add_delta_##NAME(TYPE& in, unsigned int idx = 0) \
                                                         { \
														     set_valid_##NAME(idx); \
														     m_data->m_valuesArr[INDEX+idx].value.value_##TYPE += in; \
														 }
#define TRACE_COMMAND_COUNTER(TYPE, NAME, NUM_OF_COUNTERS, PREV_NAME) TRACE_COMMAND_COUNTER1(TYPE, NAME, NUM_OF_COUNTERS, next_to_use_##PREV_NAME)

#define TRACE_COMMAND_TIMER1(TYPE, NAME, NUM_OF_COUNTERS, INDEX) TRACE_COMMAND_GENERAL(TYPE, NAME, NUM_OF_COUNTERS, INDEX); \
												 public: \
														 void set_current_time_##NAME(unsigned int idx = 0) \
														 { \
														     set_valid_##NAME(idx); \
														     m_data->m_valuesArr[INDEX+idx].value.value_##TYPE = _RDTSC(); \
														 }; \
														 void add_delta_time_##NAME(TYPE& in, unsigned int idx = 0) \
                                                         { \
														     set_valid_##NAME(idx); \
														     m_data->m_valuesArr[INDEX+idx].value.value_##TYPE += in; \
														 }
#define TRACE_COMMAND_TIMER(TYPE, NAME, NUM_OF_COUNTERS, PREV_NAME) TRACE_COMMAND_TIMER1(TYPE, NAME, NUM_OF_COUNTERS, next_to_use_##PREV_NAME)

#define TRACE_COMMAND_LAST( PREV_NAME ) private: static const unsigned int m_last = next_to_use_##PREV_NAME - 1

class CommandTracer
{
	friend class Tracer;
	friend class HostTracer;
	friend class DeviceTracer;
private:
	static const unsigned int next_to_use_initial = 0;
public:

	CommandTracer();

	~CommandTracer();

#include "mic_tracer_counters.h"

private:

	union VALUES_TYPE
	{
		unsigned_int			value_unsigned_int;
		unsigned_long_long		value_unsigned_long_long;
		char					value_char_pointer[MIC_TRACER_MAX_CHAR_POINTER_SIZE];
	};

	enum TRACE_TYPE
	{
		TRACE_unneeded = 0,
		TRACE_unsigned_int,
		TRACE_unsigned_long_long,
		TRACE_char_pointer
	};

	struct MEMBER
	{
		char name[MIC_TRACER_MAX_COUNTER_NAME_SIZE];
		VALUES_TYPE value;
		TRACE_TYPE  type;
	};

	struct COMMAND_DATA
	{
		MEMBER		m_valuesArr[m_last + 1];
	};

	// Do NOT delete it on destruction.
	COMMAND_DATA* m_data;

public:
    
    static inline unsigned long long _RDTSC()
    {
#ifdef DEVICE_NATIVE
        return Intel::OpenCL::Utils::RDTSC();
#else
        return Intel::OpenCL::Utils::HostTime();
#endif
    }
    

};

class Tracer
{
public:

	Tracer();

	~Tracer();

	void insert(CommandTracer& cmdTracer);

	static Tracer* getTracerInstace() 
	{ 
		return (m_active) ? m_active : createTracerInstance(new Tracer()); 
	};

protected:

	static Tracer* m_active;

	// Delete all elements when destruct
	vector<CommandTracer::COMMAND_DATA*>	m_cmdTracerDataArr;

	pthread_mutex_t			m_mutex;

	static Tracer* createTracerInstance(Tracer* pNewTracer);
};

class HostTracer : public Tracer
{
public:

	static HostTracer* getHostTracerInstace() 
	{ 
		return (m_active) ? (HostTracer*)m_active : (HostTracer*)createTracerInstance(new HostTracer()); 
	};

	void draw_host_to_file(MICDeviceConfig* config);

	void draw_device_to_file(void* cmdTracers, size_t sizeInBytes, unsigned long long freq);

private:

	void draw_to_file(CommandTracer::COMMAND_DATA** cmdTracers, size_t size, stringstream& headerStrStream, const char* fileName);

	void prepare_header_host(stringstream& headerStrStream, MICDeviceConfig* config);

	void prepare_header_device(stringstream& headerStrStream, unsigned long long freq);
};

class DeviceTracer : public Tracer
{
public:

	DeviceTracer() : Tracer()
	{
		m_active = this;
	}

	// Return trace size in bytes
	size_t get_trace_size();

	// get a copy of the trace data
	void getTracerCopy(void* out, size_t outSize);
};

#else // ENABLE_MIC_TRACER

#define TRACE_COMMAND_GENERAL(TYPE, NAME, NUM_OF_COUNTERS, INDEX) private: void init_##NAME() {}
#define TRACE_COMMAND_SIMPLE(TYPE, NAME, NUM_OF_COUNTERS, INDEX)  TRACE_COMMAND_GENERAL(TYPE, NAME, NUM_OF_COUNTERS, INDEX) public: void set_##NAME( const TYPE& in, unsigned int idx = 0) {}
#define TRACE_COMMAND_STRING(TYPE, NAME, NUM_OF_COUNTERS, INDEX)  TRACE_COMMAND_GENERAL(TYPE, NAME, NUM_OF_COUNTERS, INDEX) public: void set_##NAME( const TYPE& in, unsigned int idx = 0) {}
#define TRACE_COMMAND_COUNTER(TYPE, NAME, NUM_OF_COUNTERS, INDEX) TRACE_COMMAND_GENERAL(TYPE, NAME, NUM_OF_COUNTERS, INDEX) public: void increment_##NAME(unsigned int idx = 0) {}; void add_delta_##NAME(TYPE& in, unsigned int idx = 0) {}
#define TRACE_COMMAND_TIMER(TYPE, NAME, NUM_OF_COUNTERS, INDEX)   TRACE_COMMAND_GENERAL(TYPE, NAME, NUM_OF_COUNTERS, INDEX) public: void set_current_time_##NAME(unsigned int idx = 0) {}; void add_delta_time_##NAME(TYPE& in, unsigned int idx = 0) {}
#define TRACE_COMMAND_LAST( PREV_NAME ) static unsigned int PREV_NAME

class CommandTracer
{
public:
#include "mic_tracer_counters.h"
};

class Tracer
{
public:

	void insert(CommandTracer& cmdTracer) {};

	void draw_host_to_file() {};

	void draw_device_to_file(void* cmdTracers, size_t sizeInBytes, unsigned long long freq) {};
};

class HostTracer : public Tracer
{
public:

	static HostTracer* getHostTracerInstace() 
	{ 
		return nullptr;
	};
};

class DeviceTracer : public Tracer
{
};


#endif // ENABLE_MIC_TRACER

}}}
