#include <CL/sycl/detail/pi.h>
#include <cassert>
#include <iostream>
#include <map>
#include <atomic>

#include <level_zero/ze_api.h>

// Define the types that are opaque in pi.h in a manner suitabale for L0 plugin

struct _pi_platform {
  // L0 lacks the notion of a platform, but thert is a driver, which is a
  // pretty good fit to keep here.
  //
  ze_driver_handle_t L0Driver;
};

struct _pi_device {
  // L0 device handle.
  ze_device_handle_t L0Device;

  // PI platform to which this device belongs.
  pi_platform Platform;

  // Immediate L0 command list for this device, to be used for initializations.
  // To be created as:
  // - Immediate command list: So any command appended to it is immediately
  //   offloaded to the device.
  // - Synchronous: So implicit synchronization is made inside the level-zero
  //   driver.
  ze_command_list_handle_t L0CommandListInit;

  // Indicates if this is a root-device or a sub-device.
  // Technically this information can be queried from a device handle, but it
  // seems better to just keep it here.
  //
  bool IsSubDevice;

  // L0 doesn't do the reference counting, so we have to do.
  // Must be atomic to prevent data race when incrementing/decrementing.
  std::atomic<pi_uint32> RefCount;

  // Create a new command list for executing on this device.
  // It's caller's responsibility to remember and destroy the created
  // command list when no longer needed.
  //
  pi_result createCommandList(ze_command_list_handle_t *ze_command_list);

  // Cache of the immutable device properties.
  ze_device_properties_t L0DeviceProperties;
  ze_device_compute_properties_t L0DeviceComputeProperties;
};

struct _pi_context {
  // L0 does not have notion of contexts.
  // Keep the device here (must be exactly one) to return it when PI context
  // is queried for devices.
  //
  pi_device Device;

  // L0 doesn't do the reference counting, so we have to do.
  // Must be atomic to prevent data race when incrementing/decrementing.
  std::atomic<pi_uint32> RefCount;

  // Following member variables are used to manage assignment of events
  // to event pools.
  // TODO: These variables may be moved to pi_device and pi_platform
  // if appropriate
  // Event pool to which events are being added to
  ze_event_pool_handle_t L0EventPool;
  // This map will be used to determine if a pool is full or not
  // by storing number of empty slots available in the pool
  std::map<ze_event_pool_handle_t, pi_uint32> NumEventsAvailableInEventPool;
  // This map will be used to determine number of live events in the pool
  // We use separate maps for number of event slots available in the pool
  // number of events live in the pool live
  // This will help when we try to make the code thread-safe
  std::map<ze_event_pool_handle_t, pi_uint32> NumEventsLiveInEventPool;
};

struct _pi_queue {
  // L0 command queue handle.
  ze_command_queue_handle_t L0CommandQueue;

  // Keeps the PI context to which this queue belongs.
  pi_context Context;

  // L0 doesn't do the reference counting, so we have to do.
  // Must be atomic to prevent data race when incrementing/decrementing.
  std::atomic<pi_uint32> RefCount;

  // Attach a command list to this queue, close, and execute it.
  // Note that this command list cannot be appended to after this.
  // The "is_blocking" tells if the wait for completion is requested.
  //
  pi_result executeCommandList(ze_command_list_handle_t L0CommandList,
                                   bool is_blocking = false);
};

struct _pi_mem {
  // PI memory is either buffer or image.
  union {
    // L0 memory handle is really just a naked pointer.
    // It is just convenient to have it char * to simplify offset arithmetics.
    //
    char *L0Mem;

    // L0 image handle.
    ze_image_handle_t L0Image;
  };

  // Keeps the PI platform of this memory handle.
  // NOTE: it is coming *after* the native handle because the code in
  // piextKernelSetArgMemObj needs it to be so.
  pi_platform Platform;

  // TODO: as this only affects buffers and not images reorganize to
  // not waste memory. Even for buffers this should better be a pointer
  // (null for normal buffers) than statically allocates structure.
  //
  struct {
    _pi_mem * Parent;
    size_t Origin; // only valid if Parent != nullptr
    size_t Size;   // only valid if Parent != nullptr
  } SubBuffer;

  // TODO: see if there a better way to tag buffer vs. image.
  bool IsMemImage;

  // Keeps the host pointer where the buffer will be mapped to,
  // if created with PI_MEM_FLAGS_HOST_PTR_USE (see
  // piEnqueueMemBufferMap for details).
  //
  char *MapHostPtr;

#ifndef NDEBUG
  // Keep the descriptor of the image (for debugging purposes)
  ze_image_desc_t L0ImageDesc;
#endif // !NDEBUG

  // L0 doesn't do the reference counting, so we have to do.
  // Must be atomic to prevent data race when incrementing/decrementing.
  std::atomic<pi_uint32> RefCount;

  // Supplementary data to keep track of the mappings of this memory
  // created with piEnqueueMemBufferMap and piEnqueueMemImageMap.
  //
  struct mapping {
    // The offset in the buffer giving the start of the mapped region.
    size_t offset;
    // The size of the mapped region.
    size_t size;
  };
  // The key is the host pointer representing an active mapping.
  // The value is the information needed to maintain/undo the mapping.
  // TODO: make this thread-safe.
  //
  std::map<void *, mapping> Mappings;
};

struct _pi_event {
  // L0 event handle.
  ze_event_handle_t L0Event;
  // L0 event pool handle.
  ze_event_pool_handle_t L0EventPool;

  // L0 command list where the command signaling this event was appended to.
  // This is currently used to remember/destroy the command list after
  // all commands in it are completed, i.e. this event signaled.
  //
  ze_command_list_handle_t L0CommandList;

  // Keeps the command-queue and command associated with the event.
  // These are NULL for the user events.
  pi_queue Queue;
  pi_command_type CommandType;
  // Provide direct access to Context, instead of going via queue
  // Not every PI event has a queue, and we need a handle to Context
  // to get to event pool related information
  pi_context Context;

  // Opaque data to hold any data needed for CommandType.
  void * CommandData;

  // L0 doesn't do the reference counting, so we have to do.
  // Must be atomic to prevent data race when incrementing/decrementing.
  std::atomic<pi_uint32> RefCount;

  // Methods for translating PI events list into L0 events list
  static ze_event_handle_t * createL0EventList(pi_uint32, const pi_event *);
  static void deleteL0EventList(ze_event_handle_t *);

};

struct _pi_program {
  // L0 module handle.
  ze_module_handle_t L0Module;

  // Keep the context of the program.
  pi_context Context;

  // L0 doesn't do the reference counting, so we have to do.
  // Must be atomic to prevent data race when incrementing/decrementing.
  std::atomic<pi_uint32> RefCount;
};

struct _pi_kernel {
  // L0 function handle.
  ze_kernel_handle_t L0Kernel;

  // Keep the program of the kernel.
  pi_program Program;

  // L0 doesn't do the reference counting, so we have to do.
  // Must be atomic to prevent data race when incrementing/decrementing.
  std::atomic<pi_uint32> RefCount;
};

struct _pi_sampler {
  // L0 sampler handle.
  ze_sampler_handle_t L0Sampler;

  // L0 doesn't do the reference counting, so we have to do.
  // Must be atomic to prevent data race when incrementing/decrementing.
  std::atomic<pi_uint32> RefCount;
};

template<class To, class From>
To pi_cast(From value) {
  // TODO: see if more sanity checks are possible.
  assert(sizeof(From) == sizeof(To));
  return (To)(value);
}

template<>
uint32_t pi_cast(uint64_t value) {
  // Cast value and check that we don't lose any information.
  uint32_t casted_value = (uint32_t)(value);
  assert((uint64_t)casted_value == value);
  return casted_value;
}

// TODO: Currently die is defined in each plugin. Probably some
// common header file with utilities should be created. Resolve after open
// sourcing.
[[noreturn]] void die(const char *Message) {
  std::cerr << "die: " << Message << std::endl;
  std::terminate();
}
