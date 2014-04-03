#pragma once

#include "CL/cl.h"
#include <string>
#include <set>
#include <list>
#include "cl_synch_objects.h"
#include "oclEventsMapper.h"
#include "oclRetainers.h"

using Intel::OpenCL::Utils::EventsMapper;
using Intel::OpenCL::Utils::OclMutex;
using Intel::OpenCL::Utils::AtomicCounter;

using namespace std;

// TODO (Ofir): move this to a separate header file

typedef struct CommandData {
    // All objects related to this command.
    list<void*> objects;

    // Used to keep the associated objects alive while
    // we monitor the commands.
    list<OclRetainer*> retainers;
    
    // Key-value pairs of per-command info.
    list< pair<string,string> > data;

    // A unique key that represents a command instance.
    long key;

    // Work-around for commands that do not appear in CL_EVENT_COMMAND_TYPE,
    // (due to a bug in Gen runtime).
	string funcName;

    // Helpful member for determining when to release the cb data.
	AtomicCounter refCounter;

    // Used if user didn't provide an event, so that event
    // status querying is still possible.
    cl_event ourEvent;

    // The event we pass to CommandCallBack function.
    // This is used due to the existence of wrapper events
    // for the original (user) events.
    cl_event callbackEvent;

    // Associated memobj with the event, optional field.
    // It is used for retaining & releasing the user memory object
    // (to ensure it still exists during monitoring).
    cl_mem memobj;

    // True if the user event was successfully retained,
    // false otherwise.
    bool retained;

} CommandData;


//a struct to help send function parameters in a more elegant way 
typedef struct OclParameters {
	list< pair< string,string > > parameters; //name and value
} OclParameters;

#define ADD_PARAMETER(params, par) addParameter(params, #par, par);
inline std::string stripLeadingZeros(std::string &s)
{
    if (s.length() == 0) {
        return s;
    }
	size_t i = 0;
	for (; i < s.length() && s[i] == '0'; i++) {}
	if (i == s.length()) {
		return "0";
	}
	return s.substr(i);
}
template<typename T> inline void addParameter(OclParameters& params, string paramName, const T& x)
{
    params.parameters.push_back(make_pair(paramName, stripLeadingZeros(stringify(x))));
}
inline void addParameter(OclParameters& params, string paramName, const char* c)
{
    params.parameters.push_back(make_pair(paramName, stringify(c)));
}

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
	virtual void EventCreate (cl_event event,
                              bool internalEvent,
                              string cmdName = "")=0;
	virtual void EventFree (cl_event event, bool internalRelease)=0;
	virtual void EventReleased (cl_event event)=0;
    virtual void EventStatusChanged(cl_event event)=0;

	/* Memory Object Callbacks */
	virtual void BufferCreate (cl_mem /* memobj */, cl_context /* context */,
                               size_t, void*, bool, unsigned int traceCookie)=0;
	virtual void BufferMap (cl_mem /* memobj */, cl_map_flags,
                            unsigned int traceCookie)=0;
	virtual void BufferUnmap (cl_mem, cl_command_queue, cl_event*)=0;
	virtual void BufferEnqueue (cl_command_queue, cl_event*, cl_mem,
                                unsigned int traceCookie)=0;
	virtual void SubBufferCreate (cl_mem /* parent buffer */, cl_mem /* sub buffer */,
								  cl_buffer_create_type /* buffer create type */,
								  const void* /* buffer create info */,
								  cl_context,
                                  unsigned int traceCookie)=0;
	virtual void ImageCreate (cl_mem /* memobj */, cl_context /* context */,
							  const cl_image_desc*, void*,
                              bool /* from external object */,
                              unsigned int traceCookie)=0;
	virtual void ImageMap(cl_mem, cl_map_flags, unsigned int traceCookie)=0;
	virtual void ImageUnmap(cl_mem, cl_command_queue, cl_event*)=0;
	virtual void ImageEnqueue(cl_command_queue,			// clEnqueue<CopyImage/WriteImage/CopyBufferToImage/FillImage>
							  cl_event*, cl_mem,
							  unsigned int traceCookie)=0;
	virtual void PipeCreate(cl_mem pipe,
                            cl_context context,
                            cl_mem_flags memFlags,
                            cl_uint packetSize,
                            cl_uint maxPackets,
                            const cl_pipe_properties *props,
                            unsigned int traceCookie)=0;
    //virtual void ImageChangedCallBack(cl_event, cl_int, ImageInfo*)=0;
	virtual void MemObjectFree (cl_mem /* memobj */, bool internalRelease)=0;
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
	virtual void KernelFree (cl_kernel /* kernel */, bool internalRelease)=0;	// clReleaseKernel
	virtual void KernelSetArg (cl_kernel /* kernel */, cl_uint /* arg_index */, size_t /* argSize */,const void* /* arg_value */ )=0;
	virtual void KernelEnqueue (cl_kernel, cl_command_queue, cl_event*, unsigned int traceCookie)=0;
	virtual void KernelReleased (cl_kernel)=0;	// called when kernel no longer exists in Profiler & RT

	/* Command Callbacks */
	virtual void CommandCallBack(cl_event event, cl_int event_command_exec_status, CommandData *data)=0;

	/* generic Callbacks */
	virtual void ObjectInfo(const void* /* obj */, const pair<string,string> data[],const int dataLength)=0;
	virtual void ObjectRetain(const void* obj, bool internalRetain)=0;
	virtual void TraceCall( const char* call, cl_int errcode_ret,
                            OclParameters* parameters, unsigned int* traceCookie = NULL)=0;

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
	virtual void EventCreate (cl_event event,
                              bool internalEvent,
                              string cmdName = "");
	virtual void EventFree (cl_event event, bool internalRelease);
    virtual void EventReleased (cl_event event);
	virtual void EventStatusChanged(cl_event event);

	/* Memory Object Callbacks */
	virtual void BufferCreate (cl_mem /* memobj */, cl_context /* context */,
                               size_t, void*, bool, unsigned int traceCookie);
	virtual void BufferMap (cl_mem /* memobj */, cl_map_flags, unsigned int traceCookie);
	virtual void BufferUnmap (cl_mem, cl_command_queue, cl_event*);
	virtual void BufferEnqueue (cl_command_queue, cl_event*, cl_mem, unsigned int traceCookie);
	virtual void SubBufferCreate (cl_mem /* parent buffer */, cl_mem /* sub buffer */,
								  cl_buffer_create_type /* buffer create type */,
								  const void* /* buffer create info */,
								  cl_context, unsigned int traceCookie);
	virtual void ImageCreate (cl_mem /* memobj */, cl_context /* context */,
							  const cl_image_desc*,
                              void*, bool /* from external object */,
                              unsigned int traceCookie);
	virtual void ImageMap(cl_mem, cl_map_flags, unsigned int traceCookie);
	virtual void ImageUnmap(cl_mem, cl_command_queue, cl_event*);
	virtual void ImageEnqueue(cl_command_queue,			// clEnqueue<CopyImage/WriteImage/CopyBufferToImage/FillImage>
							  cl_event*, cl_mem,
                              unsigned int traceCookie);
	virtual void PipeCreate(cl_mem pipe,
                            cl_context context,
                            cl_mem_flags memFlags,
                            cl_uint packetSize,
                            cl_uint maxPackets,
                            const cl_pipe_properties *props,
                            unsigned int traceCookie);
	virtual void MemObjectFree (cl_mem /* memobj */, bool internalRelease);
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
	virtual void KernelFree (cl_kernel /* kernel */, bool internalRelease);
	virtual void KernelSetArg (cl_kernel /* kernel */, cl_uint /* arg_index */, size_t /* argSize */,const void* /* arg_value */ );
	virtual void KernelEnqueue (cl_kernel, cl_command_queue, cl_event*, unsigned int traceCookie);
	virtual void KernelReleased (cl_kernel);	// called when kernel no longer exists in Profiler & RT

	/* Command Callbacks */
	virtual void CommandCallBack(cl_event event, cl_int event_command_exec_status, CommandData *data);

	/* generic Callbacks */
	virtual void ObjectInfo(const void* /* obj */, const pair<string,string> data[],const int dataLength);
	virtual void ObjectRetain(const void* obj, bool internalRetain);
	virtual void TraceCall( const char* call, cl_int errcode_ret,
                            OclParameters* parameters, unsigned int* traceCookie = NULL);



	/******* Helper methods *******/

	/* functions that make profiling ready to work */
	void commandQueueProfiling(cl_command_queue_properties &properties);
    vector<cl_queue_properties> commandQueueProfiling(const cl_queue_properties* properties);
	
    CommandData* createCommandAndEvent(cl_event** pEvent);
	void profileCommand(cl_event profiledEvent, cl_event callbackEvent, CommandData *data);
	static void releaseCommandData(CommandData* data);

	const char* enableKernelArgumentInfo(const char* options);
    unsigned int getTraceCookie();
    vector<cl_device_id> getProgramDevices(cl_program program);

    void setEventsMapper(EventsMapper* pEventsMapper);
    cl_event getNotifierEvent(cl_event userEvent);
    cl_event getUserEvent(cl_event notifierEvent);

private:
	//singleton
	NotifierCollection(): m_eventsMapper(NULL) {}
    NotifierCollection(const NotifierCollection&);
    NotifierCollection& operator=(const NotifierCollection&);

    inline void addQueueProperty(vector<cl_queue_properties>& queueProps,
                                cl_queue_properties key,
                                cl_queue_properties value);

	set<oclNotifier*> notifiers; //the container that keeps all the notifiers
	AtomicCounter commandsIDs; //gives unique ids to commands
    AtomicCounter traceCookie;
    OclMutex m_lock;
    EventsMapper* m_eventsMapper;
};
