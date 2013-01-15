/////////////////////////////////////////////////////////////////////////
// oclNotifierCWrapper.cpp:
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

//TODO: deprecated for now, remove it in the future
//NotifierCollection* NotifierCollection::instance = NULL;

/*
 #define NOTIFY(funcName, ...) for ( notifier = notifiersIterator(); notifier != NULL ; notifier = notifiersIterator()){	\
 	funcName(__VA_ARGS__)	\
 }
*/ 

#define CHECK_FOR_NULL(ptr) if (!ptr) return


//a macro that calls funcName in all the registered notifiers.
#define NOTIFY(funcName, ...) for ( set<oclNotifier*>::iterator it = notifiers.begin(); it != notifiers.end() ; it++){	\
	(*it)->funcName(__VA_ARGS__);	\
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

//TODO: deprecated for now, remove it in the future
//oclNotifier* NotifierCollection::notifiersIterator(){
//		static set<oclNotifier*>::iterator it = notifiers.begin();
//		oclNotifier* ret;
//		if ( it == notifiers.end() ){
//			ret =  NULL;
//			it = notifiers.begin();
//		} else {
//			ret = *it;
//			it++;
//		}
//		return ret;
//}

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

void NotifierCollection::Delete(NotifierCollection* &notifiers){
	CHECK_FOR_NULL(notifiers);
	delete notifiers;
	notifiers = NULL;
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

void NotifierCollection::EventCreate (cl_event event, bool internalEvent)
{
	CHECK_FOR_NULL(event);
	NOTIFY(EventCreate, event, internalEvent);
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

void NotifierCollection::BufferCreate( cl_mem memobj, cl_context context )
{
	CHECK_FOR_NULL(memobj);
	CHECK_FOR_NULL(context);
	NOTIFY(BufferCreate, memobj, context);	
}

void NotifierCollection::ImageCreate( cl_mem memobj, cl_context context )
{
	CHECK_FOR_NULL(memobj);
	CHECK_FOR_NULL(context);
	NOTIFY(ImageCreate, memobj, context);	
}

void NotifierCollection::MemObjectFree( cl_mem memobj )
{
	CHECK_FOR_NULL(memobj);
	NOTIFY(MemObjectFree, memobj);	
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

void NotifierCollection::ProgramCreate( cl_program program, cl_context context )
{
	CHECK_FOR_NULL(program);
	CHECK_FOR_NULL(context);
	NOTIFY(ProgramCreate, program, context);	
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

//TODO: deprecated for now, remove it in the future
// void NotifierCollection::CommandEnqueue( cl_command_queue queue , void* pCommand,cl_command_type commandType, const char* commandName, void** oclObjectArray, cl_uint numObjects )
// {
// 	NOTIFY(CommandEnqueue, queue, pCommand, commandType, commandName, oclObjectArray, numObjects);	
// }
// 
// void NotifierCollection::CommandRunning( void* pCommand )
// {
// 	NOTIFY(CommandRunning, pCommand);	
// }
// 
// void NotifierCollection::CommandComplete( void* pCommand, cl_int CompletionResult )
// {
// 	NOTIFY(CommandComplete, pCommand, CompletionResult);	
// }

void NotifierCollection::CommandCallBack( cl_event event, cl_int event_command_exec_status, CommandData *data )
{
	NOTIFY(CommandCallBack, event, event_command_exec_status, data);
}

void NotifierCollection::releaseCommandData(cl_event *event, CommandData* data){
	CHECK_FOR_NULL(data);
	bool ownEvent = data->ownEvent;
	releaseCommandData(*event,data);
	if (ownEvent){
		delete event;
	}
}
void NotifierCollection::releaseCommandData( cl_event event, CommandData* data ){
	CHECK_FOR_NULL(data);
	delete data;
	CHECK_FOR_NULL(event);
	clReleaseEvent(event);
}

//the callback attached to the event of a command
void CL_CALLBACK commandCallBack(cl_event event, cl_int event_command_exec_status, void *user_data){
	CHECK_FOR_NULL(event);
	CHECK_FOR_NULL(user_data);
	NotifierCollection* notifiers = NotifierCollection::Instance();
	CommandData* commandData = (CommandData*) user_data;
	notifiers->CommandCallBack(event,event_command_exec_status,commandData); 
	//delete objects
	if (--commandData->refCounter == 0){
		NotifierCollection::releaseCommandData(event, commandData);
	}
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

void NotifierCollection::TraceCall( const char* call, cl_int errcode_ret, OclParameters* parameters){
	CHECK_FOR_NULL(call);
	NOTIFY(TraceCall, call, errcode_ret, parameters);
}

//used in order to make the supermarket principal work
#define IF_EMPTY_RETURN if (notifiers.empty()) return


void NotifierCollection::commandQueueProfiling( cl_command_queue_properties &properties )
{
	IF_EMPTY_RETURN; //we don't want to change anything if we don't have notifiers...
	//CHECK_FOR_NULL(properties); this is not a pointer, no need.
	properties |= CL_QUEUE_PROFILING_ENABLE;
}
CommandData* NotifierCollection::commandEventProfiling( cl_event **pEvent )
{
	IF_EMPTY_RETURN NULL; //we don't want to change anything if we don't have notifiers...
	CHECK_FOR_NULL(pEvent) NULL;
	CommandData* commandData = new CommandData; //dynamic allocation - should be released in the CommandCallBack
	commandData->ownEvent = true;
	if (*pEvent != NULL){ 
		//user has the event, just retain it so we can release it when we want to (retain is only after event is created)
		commandData->ownEvent = false;
	} else {
	*pEvent = new cl_event; //dynamic allocation - we will need to release it in CommandCallBack
	**pEvent = NULL;
	}
	return commandData;
}

void NotifierCollection::createCommandEvents(cl_event* event, CommandData *commandData){
	IF_EMPTY_RETURN; //we don't want to change anything if we don't have notifiers...
	CHECK_FOR_NULL(event);
	CHECK_FOR_NULL(commandData);
	bool isOwnEvent = commandData->ownEvent;
	EventCreate(*event, isOwnEvent); //TODO: consider moving it...
	if (isOwnEvent == false){
		clRetainEvent( *event); //we need to retain it so it won't be released before we have completed with it.
	}
	//take id
	commandData->key = (long) commandsIDs; //TODO: think about guids
	++commandsIDs;
	//set callbacks for all types
	commandData->refCounter = 3;
	cl_uint err = -1;
	err = clSetEventCallback(*event, CL_SUBMITTED, &commandCallBack, (void*) commandData);
	if ( err != CL_SUCCESS){
		//log
		commandData->refCounter--;
	}

	err = clSetEventCallback(*event, CL_RUNNING, &commandCallBack, (void*) commandData);
	if ( err != CL_SUCCESS){
		//log
		//TODO: think about sending an error notification massage 
		commandData->refCounter--;
	}

	err = clSetEventCallback(*event, CL_COMPLETE, &commandCallBack, (void*) commandData);
	if ( err != CL_SUCCESS){
		//log 
		if (--commandData->refCounter == 0){
		releaseCommandData(*event, commandData);
		}
	}
	//from here commandData considered invalid because all the eventCallbacks might have happened

	if ( isOwnEvent){
		delete event; //only release the pointer to the event and not the actual object.
	}
}
