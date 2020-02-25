#include <CL/sycl/detail/pi.h>
#include <cassert>
#include <iostream>
#include <map>

#include <level_zero/ze_api.h>

// Define the types that are opaque in pi.h in a manner suitabale for L0 plugin

struct _pi_device {
  // L0 device handle.
  ze_device_handle_t L0Device;

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
  pi_uint32 RefCount;

  // Create a new command list for executing on this device.
  // It's caller's responsibility to remember and destroy the created
  // command list when no longer needed.
  //
  ze_command_list_handle_t createCommandList();
};

struct _pi_context {
  // L0 does not have notion of contexts.
  // Keep the device here (must be exactly one) to return it when PI context
  // is queried for devices.
  //
  pi_device Device;

  // L0 doesn't do the reference counting, so we have to do.
  pi_uint32 RefCount;
};

struct _pi_queue {
  // L0 command queue handle.
  ze_command_queue_handle_t L0CommandQueue;

  // Keeps the PI context to which this queue belongs.
  pi_context Context;

  // L0 doesn't do the reference counting, so we have to do.
  pi_uint32 RefCount;

  // Attach a command list to this queue, close, and execute it.
  // Note that this command list cannot be appended to after this.
  // The "is_blocking" tells if the wait for completion is requested.
  //
  void executeCommandList(ze_command_list_handle_t L0CommandList,
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
  pi_uint32 RefCount;

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

  // Opaque data to hold any data needed for CommandType.
  void * CommandData;

  // L0 doesn't do the reference counting, so we have to do.
  pi_uint32 RefCount;

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
  pi_uint32 RefCount;
};

struct _pi_kernel {
  // L0 function handle.
  ze_kernel_handle_t L0Kernel;

  // Keep the program of the kernel.
  pi_program Program;

  // L0 doesn't do the reference counting, so we have to do.
  pi_uint32 RefCount;
};

struct _pi_sampler {
  // L0 sampler handle.
  ze_sampler_handle_t L0Sampler;

  // L0 doesn't do the reference counting, so we have to do.
  pi_uint32 RefCount;
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

[[noreturn]] void pi_throw(const char *message) {
  std::cerr << "pi_throw: " << message << std::endl;
  throw message;
  //std::terminate();
}

void pi_assert(bool cond) { assert(cond); }

