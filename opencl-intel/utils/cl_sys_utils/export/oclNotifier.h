#pragma once

#include "CL/cl.h"
#include <string>
#include <set>
#include <list>
#include "cl_synch_objects.h"
using namespace std;


//a struct to help send commands in a more elegant way
typedef struct CommandData {
	list<void*> objects; //object related to the command
	list< pair< string,string > > data; //all kind of other useful data
	bool ownEvent;	//a flag to know if it is our responsibility to free the event monitoring this command
	long key;	// a unique key that represent this command
	Intel::OpenCL::Utils::AtomicCounter refCounter; //holds the number of callbacks that will use this struct
	cl_event *pEvent;	// pointer to the event that we create if user hasn't provided one
	bool needRelease;	// determines if we need to release the CL resources of the event
	const char* funcName;	// useful for functions that do not appear in CL_EVENT_COMMAND_TYPE
} CommandData;

//a struct to help send function parameters in a more elegant way 
typedef struct OclParameters {
	list< pair< string,string > > parameters; //name and value
} OclParameters;


#define ADD_PARAMETER(params, par) params.parameters.push_back(make_pair(#par, stringify(par)))
// an abstract class that one can implement and register in order to get callbacks 
class oclNotifier
{
public:
	
	/* Platform Callbacks */
	virtual void PlatformCreate(cl_platform_id /* platform */)=0;
	virtual void PlatformFree(cl_platform_id /* platform */)=0;

	/* Device Callbacks */
	virtual void DeviceInit(cl_device_id /* device */, cl_platform_id /* platform */)=0;
	virtual void SubDeviceCreate(cl_device_id /* parent device */, cl_device_id /* sub device */)=0;
	virtual void DeviceFree(cl_device_id /* device */)=0;

	/* Context Callbacks */
	virtual void ContextCreate (cl_context /* context */)=0;
	virtual void ContextFree (cl_context /* context */)=0;

	/* Command Queue Callbacks */
	virtual void CommandQueueCreate (cl_command_queue /* queue */)=0;
	virtual void CommandQueueFree (cl_command_queue /* queue */)=0;

	/* Event Callbacks */
	virtual void EventCreate (cl_event, bool, string* cmdName = NULL)=0;
	virtual void EventFree (cl_event event)=0;
	virtual void EventStatusChanged(cl_event event)=0;

	/* Memory Object Callbacks */
	virtual void BufferCreate (cl_mem /* memobj */, cl_context /* context */, size_t, void*, bool)=0;
	virtual void BufferMap (cl_mem /* memobj */, cl_map_flags)=0;
	virtual void BufferUnmap (cl_mem, cl_command_queue, cl_event*)=0;
	virtual void BufferEnqueue (cl_command_queue, cl_event*, cl_mem)=0;
	virtual void SubBufferCreate (cl_mem /* parent buffer */, cl_mem /* sub buffer */,
								  cl_buffer_create_type /* buffer create type */,
								  const void* /* buffer create info */,
								  cl_context)=0;
	virtual void ImageCreate (cl_mem /* memobj */, cl_context /* context */,
							  const cl_image_desc*, void*, bool /* from external object */ )=0;
	virtual void ImageMap(cl_mem, cl_map_flags)=0;
	virtual void ImageUnmap(cl_mem, cl_command_queue, cl_event*)=0;
	virtual void ImageEnqueue(cl_command_queue,			// clEnqueue<CopyImage/WriteImage/CopyBufferToImage/FillImage>
							  cl_event*,	
							  cl_mem)=0;
	//virtual void ImageChangedCallBack(cl_event, cl_int, ImageInfo*)=0;
	virtual void MemObjectFree (cl_mem /* memobj */)=0;
	virtual void MemObjectReleased (cl_mem)=0;	// called when kernel no longer exists in Profiler & RT

	/* Sampler Callbacks */
	virtual void SamplerCreate (cl_sampler /* sampler */, cl_context /* context */)=0;
	virtual void SamplerFree (cl_sampler /* sampler */)=0;

	/* Program Callbacks */
	virtual void ProgramCreate (cl_program /* program */, cl_context, bool, bool)=0;
	virtual void ProgramFree (cl_program /* program */)=0;
	virtual void ProgramBuild (cl_program /* program */, const cl_device_id* devices, cl_uint numDevices)=0;

	/* Kernel Callbacks */
	virtual void KernelCreate (cl_kernel /* kernel */, cl_program /* program */)=0;
	virtual void KernelFree (cl_kernel /* kernel */)=0;	// clReleaseKernel
	virtual void KernelSetArg (cl_kernel /* kernel */, cl_uint /* arg_index */, size_t /* argSize */,const void* /* arg_value */ )=0;
	virtual void KernelEnqueue (cl_kernel, cl_command_queue, cl_event*)=0;
	virtual void KernelReleased (cl_kernel)=0;	// called when kernel no longer exists in Profiler & RT

	/* Command Callbacks */
	virtual void CommandCallBack(cl_event event, cl_int event_command_exec_status, CommandData *data)=0;

	/* generic Callbacks */
	virtual void ObjectInfo(const void* /* obj */, const pair<string,string> data[],const int dataLength)=0;
	virtual void ObjectRetain( const void* obj)=0;
	virtual void TraceCall( const char* call, cl_int errcode_ret, OclParameters* parameters)=0;

	virtual ~oclNotifier() {}
};


//a class that acts as oclNotifier but actually manage all the notifiers in the system 
//and it is the one that actually get called
//you need to register your notifier to this class in order to activate the notifier.
//if no registered notifiers exists it will just won't do anything when the methods are called...
class NotifierCollection : public oclNotifier {
public:

	/******* Notifiers manipulation methods *******/

	//in order to get it, it is a singleton 
	static NotifierCollection* Instance();

	//use it to delete your notifier, at the moment does not keep a count of references
	static void Delete(NotifierCollection* &notifiers);

	//check if there are any active notifiers around
	bool isActive();

	//use it to register a notifier to the system
	void registerNotifier( oclNotifier *notifier );

	//use it if you want to stop receiving notification, 
	//if not it will be released when this class gets deleted
	void unregisterNotifier( oclNotifier *notifier );

	virtual ~NotifierCollection();




	/******* Notifier interface methods *******/


	/* Platform Callbacks */
	virtual void PlatformCreate(cl_platform_id /* platform */);
	virtual void PlatformFree(cl_platform_id /* platform */);

	/* Device Callbacks */
	virtual void DeviceInit(cl_device_id /* device */, cl_platform_id /* platform */);
	virtual void SubDeviceCreate(cl_device_id /* parent device */, cl_device_id /* sub device */);
	virtual void DeviceFree(cl_device_id /* device */);

	/* Context Callbacks */
	virtual void ContextCreate (cl_context /* context */);
	virtual void ContextFree (cl_context /* context */);

	/* Command Queue Callbacks */
	virtual void CommandQueueCreate (cl_command_queue /* queue */);
	virtual void CommandQueueFree (cl_command_queue /* queue */);

	/* Event Callbacks */
	virtual void EventCreate (cl_event, bool, string* cmdName = NULL);
	virtual void EventFree (cl_event event);
	virtual void EventStatusChanged(cl_event event);

	/* Memory Object Callbacks */
	virtual void BufferCreate (cl_mem /* memobj */, cl_context /* context */, size_t, void*, bool);
	virtual void BufferMap (cl_mem /* memobj */, cl_map_flags);
	virtual void BufferUnmap (cl_mem, cl_command_queue, cl_event*);
	virtual void BufferEnqueue (cl_command_queue, cl_event*, cl_mem);
	virtual void SubBufferCreate (cl_mem /* parent buffer */, cl_mem /* sub buffer */,
								  cl_buffer_create_type /* buffer create type */,
								  const void* /* buffer create info */,
								  cl_context);
	virtual void ImageCreate (cl_mem /* memobj */, cl_context /* context */,
							  const cl_image_desc*, void*, bool /* from external object */ );
	virtual void ImageMap(cl_mem, cl_map_flags);
	virtual void ImageUnmap(cl_mem, cl_command_queue, cl_event*);
	virtual void ImageEnqueue(cl_command_queue,			// clEnqueue<CopyImage/WriteImage/CopyBufferToImage/FillImage>
							  cl_event*,	
							  cl_mem);
	virtual void MemObjectFree (cl_mem /* memobj */);
	virtual void MemObjectReleased (cl_mem);	// called when kernel no longer exists in Profiler & RT

	/* Sampler Callbacks */
	virtual void SamplerCreate (cl_sampler /* sampler */, cl_context /* context */);
	virtual void SamplerFree (cl_sampler /* sampler */);

	/* Program Callbacks */
	virtual void ProgramCreate (cl_program /* program */, cl_context, bool, bool );
	virtual void ProgramFree (cl_program /* program */);
	virtual void ProgramBuild (cl_program /* program */, const cl_device_id* devices, cl_uint numDevices);

	/* Kernel Callbacks */
	virtual void KernelCreate (cl_kernel /* kernel */, cl_program /* program */);
	virtual void KernelFree (cl_kernel /* kernel */);
	virtual void KernelSetArg (cl_kernel /* kernel */, cl_uint /* arg_index */, size_t /* argSize */,const void* /* arg_value */ );
	virtual void KernelEnqueue (cl_kernel, cl_command_queue, cl_event*);
	virtual void KernelReleased (cl_kernel);	// called when kernel no longer exists in Profiler & RT

	/* Command Callbacks */
	virtual void CommandCallBack(cl_event event, cl_int event_command_exec_status, CommandData *data);

	/* generic Callbacks */
	virtual void ObjectInfo(const void* /* obj */, const pair<string,string> data[],const int dataLength);
	virtual void ObjectRetain( const void* obj);
	virtual void TraceCall( const char* call, cl_int errcode_ret, OclParameters* parameters);



	/******* Helper methods *******/


	/* functions that make profiling ready to work */
	void commandQueueProfiling(cl_command_queue_properties &properties);
	CommandData* commandEventProfiling(cl_event **pEvent);

	void createCommandEvents(cl_event *event, CommandData *data);
	static void releaseCommandData(CommandData* data);

	const char* enableKernelArgumentInfo(const char* options);


private:
	//singleton
	NotifierCollection() {}
    NotifierCollection(const NotifierCollection&);
    NotifierCollection& operator=(const NotifierCollection&);

	set<oclNotifier*> notifiers; //the container that keeps all the notifiers
	Intel::OpenCL::Utils::AtomicCounter commandsIDs; //gives unique ids to commands
};
