/////////////////////////////////////////////////////////////////////////
// oclNotifier.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel�s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel�s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "oclNotifier.h"
#include "oclInternalFunctions.h"

//TODO: Ofir, refactor the cl files code
#include <fstream>
#include <stdexcept>

#define CHECK_FOR_NULL(ptr) if (!ptr) return

//used in order to make the supermarket principal work
#define IF_EMPTY_RETURN if (notifiers.empty()) return


//a macro that calls funcName in all the registered notifiers.
#define NOTIFY(funcName, ...) for ( set<oclNotifier*>::iterator it = notifiers.begin(); it != notifiers.end() ; it++){	\
	(*it)->funcName(__VA_ARGS__);	\
} 

//the callback attached to the event of a command
static void CL_CALLBACK commandCallBack(cl_event event, cl_int event_command_exec_status, void *user_data){
	CHECK_FOR_NULL(event);
	CHECK_FOR_NULL(user_data);
	NotifierCollection* notifiers = NotifierCollection::Instance();
	CommandData* commandData = (CommandData*) user_data;
	notifiers->CommandCallBack(event,event_command_exec_status,commandData); 
	//delete objects
	if (--commandData->refCounter == 0){
		NotifierCollection::releaseCommandData(commandData);
	}
}


NotifierCollection* NotifierCollection::Instance() 
{ 
	//TODO: maybe add auto mutex in here...
	static NotifierCollection* instance = NULL;
	if (NULL == instance)
	{
		instance = new NotifierCollection();
	}
	return instance; 
}
void NotifierCollection::Delete(NotifierCollection* &notifiers){
	CHECK_FOR_NULL(notifiers);
	delete notifiers;
	notifiers = NULL;
}
bool NotifierCollection::isActive()
{
	return !notifiers.empty();
}
void NotifierCollection::registerNotifier( oclNotifier *notifier )
{
	CHECK_FOR_NULL(notifier);
	notifiers.insert(notifier);
}
void NotifierCollection::unregisterNotifier( oclNotifier *notifier )
{
	CHECK_FOR_NULL(notifier);
	notifiers.erase(notifier);
	delete notifier;
}
NotifierCollection::~NotifierCollection() {
	for ( set<oclNotifier*>::iterator it = notifiers.begin(); it != notifiers.end() ; it++){	
		delete (*it);
	}
}


void NotifierCollection::PlatformCreate( cl_platform_id platform ){
	CHECK_FOR_NULL(platform);
	NOTIFY(PlatformCreate, platform);
}
void NotifierCollection::PlatformFree( cl_platform_id platform ){
	CHECK_FOR_NULL(platform);
	NOTIFY(PlatformFree, platform);
}

void NotifierCollection::DeviceInit( cl_device_id device, cl_platform_id platform ){
	CHECK_FOR_NULL(device);
	CHECK_FOR_NULL(platform);
	NOTIFY(DeviceInit, device, platform);
}
void NotifierCollection::SubDeviceCreate( cl_device_id parent_device, cl_device_id sub_device){
	CHECK_FOR_NULL(parent_device);
	CHECK_FOR_NULL(sub_device);
	NOTIFY(SubDeviceCreate, parent_device, sub_device);
}
void NotifierCollection::DeviceFree( cl_device_id device )
{
	CHECK_FOR_NULL(device);
	NOTIFY(DeviceFree, device);	
}

void NotifierCollection::ContextCreate( cl_context context)
{
	CHECK_FOR_NULL(context);
	NOTIFY(ContextCreate, context);	
}
void NotifierCollection::ContextFree( cl_context context )
{
	CHECK_FOR_NULL(context);
	NOTIFY(ContextFree, context);	
}

void NotifierCollection::CommandQueueCreate( cl_command_queue queue )
{
	CHECK_FOR_NULL(queue);
	NOTIFY(CommandQueueCreate, queue );	
}
void NotifierCollection::CommandQueueFree( cl_command_queue queue )
{
	CHECK_FOR_NULL(queue);
	NOTIFY(CommandQueueFree, queue);	
}

void NotifierCollection::EventCreate (cl_event event, bool internalEvent, string* cmdName)
{
	CHECK_FOR_NULL(event);
	NOTIFY(EventCreate, event, internalEvent, cmdName);
}
void NotifierCollection::EventFree (cl_event event)
{
	CHECK_FOR_NULL(event);
	NOTIFY(EventFree, event);
}
void NotifierCollection::EventStatusChanged(cl_event event)
{
	CHECK_FOR_NULL(event);
	NOTIFY(EventStatusChanged, event);
}

void NotifierCollection::BufferCreate(cl_mem memobj, cl_context context,
                                      size_t size, void* hostPtr,
                                      bool fromGL,
                                      unsigned int cookie)
{
	CHECK_FOR_NULL(memobj);
	CHECK_FOR_NULL(context);
	NOTIFY(BufferCreate, memobj, context, size,
           hostPtr, fromGL, cookie);
}
void NotifierCollection::BufferMap(cl_mem memobj, cl_map_flags mapFlags,
                                   unsigned int cookie)
{
	CHECK_FOR_NULL(memobj);
	NOTIFY(BufferMap, memobj, mapFlags, cookie);
}
void NotifierCollection::BufferUnmap(cl_mem memobj, cl_command_queue queue, cl_event* event)
{
	CHECK_FOR_NULL(memobj);
	NOTIFY(BufferUnmap, memobj, queue, event);
}
void NotifierCollection::BufferEnqueue (cl_command_queue queue, cl_event* event,
                                        cl_mem memobj, unsigned int cookie)
{
	CHECK_FOR_NULL(memobj);
	NOTIFY(BufferEnqueue, queue, event, memobj, cookie);
}
void NotifierCollection::SubBufferCreate(cl_mem parentBuffer,
										 cl_mem subBuffer,
										 cl_buffer_create_type bufferCreateType,
										 const void* bufferCreateInfo,
										 cl_context context,
                                         unsigned int cookie)
{
	CHECK_FOR_NULL(parentBuffer);
	CHECK_FOR_NULL(subBuffer);
	NOTIFY(SubBufferCreate, parentBuffer, subBuffer,
		   bufferCreateType, bufferCreateInfo, context,
           cookie);
}
void NotifierCollection::ImageCreate(cl_mem memobj, cl_context context, 
									 const cl_image_desc* imageDesc,
									 void* hostPtr, bool fromGL,
                                     unsigned int cookie)
{
	CHECK_FOR_NULL(memobj);
	CHECK_FOR_NULL(context);
	NOTIFY(ImageCreate, memobj, context, imageDesc,
           hostPtr, fromGL, cookie);
}
void NotifierCollection::ImageMap(cl_mem memobj, cl_map_flags mapFlags,
                                  unsigned int cookie)
{
	CHECK_FOR_NULL(memobj);
	NOTIFY(ImageMap, memobj, mapFlags, cookie);
}
void NotifierCollection::ImageUnmap(cl_mem memobj, cl_command_queue queue, cl_event* event)
{
	NOTIFY(ImageUnmap, memobj, queue, event);
}
void NotifierCollection::ImageEnqueue(cl_command_queue queue,
									  cl_event* event,
									  cl_mem memobj,
                                      unsigned int cookie)
{
	NOTIFY(ImageEnqueue, queue, event, memobj, cookie);
}
void NotifierCollection::PipeCreate(
    cl_mem pipe,
    cl_context context,
    cl_mem_flags memFlags,
    cl_uint packetSize,
    cl_uint maxPackets,
    const cl_pipe_properties *props,
    unsigned int traceCookie)
{
    CHECK_FOR_NULL(pipe);
    NOTIFY(PipeCreate, pipe, context, memFlags,
        packetSize, maxPackets, props, traceCookie);
}

void NotifierCollection::MemObjectFree( cl_mem memobj )
{
	CHECK_FOR_NULL(memobj);
	NOTIFY(MemObjectFree, memobj);	
}
void NotifierCollection::MemObjectReleased(cl_mem memobj)
{
	NOTIFY(MemObjectReleased, memobj);
}

void NotifierCollection::SamplerCreate( cl_sampler sampler, cl_context context )
{
	CHECK_FOR_NULL(sampler);
	CHECK_FOR_NULL(context);
	NOTIFY(SamplerCreate, sampler, context);	
}
void NotifierCollection::SamplerFree( cl_sampler sampler )
{
	CHECK_FOR_NULL(sampler);
	NOTIFY(SamplerFree, sampler);	
}

void NotifierCollection::ProgramCreate( cl_program program, cl_context context, bool withBinary, bool withSource=true )
{
	CHECK_FOR_NULL(program);
	CHECK_FOR_NULL(context);
	NOTIFY(ProgramCreate, program, context, withBinary, withSource);	
}
void NotifierCollection::ProgramFree( cl_program program )
{
	CHECK_FOR_NULL(program);
	NOTIFY(ProgramFree, program);	
}
void NotifierCollection::ProgramBuild( cl_program program, const cl_device_id* devices, cl_uint numDevices )
{
	CHECK_FOR_NULL(program);
	CHECK_FOR_NULL(devices);
	NOTIFY(ProgramBuild, program, devices, numDevices);	
}

void NotifierCollection::KernelCreate( cl_kernel kernel, cl_program program )
{
	CHECK_FOR_NULL(kernel);
	CHECK_FOR_NULL(program);
	NOTIFY(KernelCreate, kernel, program);
}
void NotifierCollection::KernelFree( cl_kernel kernel )
{
	CHECK_FOR_NULL(kernel);
	NOTIFY(KernelFree, kernel);	
}
void NotifierCollection::KernelSetArg (cl_kernel kernel, cl_uint arg_index, size_t arg_size,const void* arg_value )
{
	CHECK_FOR_NULL(kernel);
	NOTIFY(KernelSetArg, kernel, arg_index, arg_size, arg_value);
}
void NotifierCollection::KernelEnqueue (cl_kernel kernel,
                                        cl_command_queue queue,
                                        cl_event* event,
                                        unsigned int cookie)
{
	NOTIFY(KernelEnqueue, kernel, queue, event, cookie);
}
void NotifierCollection::KernelReleased (cl_kernel kernel)
{
	NOTIFY(KernelReleased, kernel);
}

void NotifierCollection::CommandCallBack( cl_event event, cl_int event_command_exec_status, CommandData *data )
{
	NOTIFY(CommandCallBack, event, event_command_exec_status, data);
}

void NotifierCollection::ObjectInfo( const void* obj, const pair<string,string> data[],const int dataLength )
{
	CHECK_FOR_NULL(obj);
	NOTIFY(ObjectInfo, obj, data, dataLength);	
}
void NotifierCollection::ObjectRetain( const void* obj){
	CHECK_FOR_NULL(obj);
	NOTIFY(ObjectRetain, obj);	
}
void NotifierCollection::TraceCall(const char* call, cl_int errcode_ret,
                                   OclParameters* parameters,
                                   unsigned int *cookie){
	CHECK_FOR_NULL(call);
	NOTIFY(TraceCall, call, errcode_ret, parameters, cookie);
}


void NotifierCollection::commandQueueProfiling( cl_command_queue_properties &properties )
{
	IF_EMPTY_RETURN; //we don't want to change anything if we don't have notifiers...
	//CHECK_FOR_NULL(properties); this is not a pointer, no need.
	properties |= CL_QUEUE_PROFILING_ENABLE;
}
vector<cl_queue_properties> NotifierCollection::commandQueueProfiling(const cl_queue_properties* properties)
{
    vector<cl_queue_properties> propsWithProfiling;
    const cl_queue_properties* pCurProperty = NULL;
    cl_command_queue_properties cmdQueuePropValue = 0;

    bool addedProfilingProp = false;
    for (pCurProperty = properties; pCurProperty && *pCurProperty; pCurProperty+= 2)
    {
        if (CL_QUEUE_PROPERTIES == *pCurProperty) {
            cmdQueuePropValue = *(pCurProperty + 1);
            commandQueueProfiling(cmdQueuePropValue);
            addQueueProperty(propsWithProfiling,
                CL_QUEUE_PROPERTIES, cmdQueuePropValue);
            addedProfilingProp = true;
        } else {
            addQueueProperty(propsWithProfiling,
                *pCurProperty, *(pCurProperty + 1));
        }
    }

    if (!addedProfilingProp) {
        addQueueProperty(propsWithProfiling,
            CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE);
    }

    propsWithProfiling.push_back(0);
    return propsWithProfiling;
}
CommandData* NotifierCollection::commandEventProfiling( cl_event **pEvent )
{
	IF_EMPTY_RETURN NULL; //we don't want to change anything if we don't have notifiers...
	CHECK_FOR_NULL(pEvent) NULL;
	CommandData* commandData = new CommandData; //dynamic allocation - should be released in the CommandCallBack
	commandData->ownEvent = true;
	commandData->pEvent = NULL;
	commandData->needRelease = false;	// will change to true if following enqueue operation succeeded
	commandData->funcName = NULL;
	if (*pEvent != NULL){ 
		//user has the event, just retain it so we can release it when we want to (retain is only after event is created)
		commandData->ownEvent = false;
	} else {
		*pEvent = new cl_event; //dynamic allocation - we will need to release it in CommandCallBack (event member)
		**pEvent = NULL;
		commandData->pEvent = *pEvent;	// keep track of allocated event to be released in CommandCallBack or releaseCommandData
	}
	return commandData;
}

void NotifierCollection::createCommandEvents(cl_event* event, CommandData *commandData){
	IF_EMPTY_RETURN; //we don't want to change anything if we don't have notifiers...
	CHECK_FOR_NULL(event);
	CHECK_FOR_NULL(commandData);
	// retain is at least 1 since the event can be either the user event
	// and then we don't want to lose it so we retain it, or it's our event
	// and the preceding enqueue succeeded so retain is set to 1
	commandData->needRelease = true;
	bool isOwnEvent = commandData->ownEvent;
	string *cmdName = NULL;
	if (NULL != commandData->funcName) {
		cmdName = new string(commandData->funcName);
	}
	EventCreate(*event, isOwnEvent, cmdName); //TODO: consider moving it...
	delete cmdName;
	if (isOwnEvent == false){
		_clRetainEventINTERNAL( *event); //we need to retain it so it won't be released before we have completed with it.
		commandData->pEvent = event;	// we'll need it for clReleaseEvent later
	}

	//take id
	commandData->key = (long) commandsIDs; //TODO: think about guids
	++commandsIDs;
	//set callbacks for all types
	commandData->refCounter = 3;
	cl_int err;
	err = _clSetEventCallbackINTERNAL(*event, CL_SUBMITTED, &commandCallBack, (void*) commandData);
	if ( err != CL_SUCCESS){
		//log
		commandData->refCounter--;
	}

	err = _clSetEventCallbackINTERNAL(*event, CL_RUNNING, &commandCallBack, (void*) commandData);
	if ( err != CL_SUCCESS){
		//log
		//TODO: think about sending an error notification massage 
		commandData->refCounter--;
	}

	err = _clSetEventCallbackINTERNAL(*event, CL_COMPLETE, &commandCallBack, (void*) commandData);
	if ( err != CL_SUCCESS){
		//log 
		if (--commandData->refCounter == 0){
			releaseCommandData(commandData);
		}
	}
	//from here commandData considered invalid because all the eventCallbacks might have happened

}
void NotifierCollection::releaseCommandData(CommandData* data)
{
	CHECK_FOR_NULL(data);
	cl_event* pEvent = data->pEvent;
	if (data->needRelease)
	{
		_clReleaseEventINTERNAL(*pEvent);
	}
	if (data->ownEvent)
	{
		delete pEvent;
	}
	delete data;
}

const char* NotifierCollection::enableKernelArgumentInfo(const char* options){
	size_t options_length = strlen(options);
	const char* CL_KERNEL_ARG_INFO_OPTION = "-cl-kernel-arg-info";
	if (strstr(options, CL_KERNEL_ARG_INFO_OPTION))
	{
		char* newOptions = new char[options_length + 1]; //Dynamic allocation - will be freed in buildProgram
		STRCPY_S(newOptions, options_length + 1, options);
		return newOptions; //TODO: fix, you don't need dynamic allocation here...
	}	
	size_t addon_length = strlen(CL_KERNEL_ARG_INFO_OPTION);
	size_t dstSize = options_length + addon_length + 2;	// space and null terminator
	char* newOptions = new char[dstSize]; //Dynamic allocation - will be freed in buildProgram
	STRCPY_S(newOptions, dstSize, options);
	STRCAT_S(newOptions, dstSize, " ");
	STRCAT_S(newOptions, dstSize, CL_KERNEL_ARG_INFO_OPTION); 
	return newOptions;
}
unsigned int NotifierCollection::getTraceCookie()
{
    return static_cast<unsigned int>(traceCookie++);
}
vector<cl_device_id> NotifierCollection::getProgramDevices(cl_program program)
{
    cl_int err = CL_INVALID_PROGRAM;
    size_t deviceSizeInBytes = 0;
    size_t numDevices = 0;
    err = _clGetProgramInfoINTERNAL(program, CL_PROGRAM_DEVICES, 0,
                                    NULL, &deviceSizeInBytes);
    if (CL_SUCCESS != err) {
        stringstream ss;
        ss << "Unable to get no. of devices of program. " 
           << "err = " << err;
        throw runtime_error(ss.str());
    }
	numDevices = deviceSizeInBytes / sizeof(cl_device_id);
    vector<cl_device_id> vec(numDevices, NULL);
	err = _clGetProgramInfoINTERNAL(program,
                                    CL_PROGRAM_DEVICES,
                                    deviceSizeInBytes,
									(void *)&vec[0], NULL);
    if (CL_SUCCESS != err) {
        stringstream ss;
        ss << "Unable to get program devices. " 
           << "err = " << err;
        throw runtime_error(ss.str());
    }

    return vec;
}
inline void NotifierCollection::addQueueProperty(
    vector<cl_queue_properties>& queueProps,
    cl_queue_properties key,
    cl_queue_properties value)
{
    queueProps.push_back(key);
    queueProps.push_back(value);
}
