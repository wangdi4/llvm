#include <CL/sycl/detail/pi.h>
#include <cassert>
#include <iostream>

#include <level_zero/ze_api.h>

// Define the types that are opaque in pi.h in a manner suitabale for L0 plugin

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

  // L0 command list for this queue.
  ze_command_list_handle_t L0CommandList;

  // Keeps the PI context to which this queue belongs.
  pi_context Context;

  // L0 doesn't do the reference counting, so we have to do.
  pi_uint32 RefCount;

  // Methods for working with the queue's command list.
  ze_command_list_handle_t getCommandList();
  void executeCommandList();
};

struct _pi_event {
  // L0 event handle.
  ze_event_handle_t L0Event;

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

  // TODO: add ref-count.
};

struct _pi_kernel {
  // L0 function handle.
  ze_kernel_handle_t L0Kernel;

  // Keep the program of the kernel.
  pi_program Program;
};

template<class To, class From>
To pi_cast(From value) {
  // TODO: see if more sanity checks are possible.
  assert(sizeof(From) == sizeof(To));
  return (To)(value);
}

[[noreturn]] void pi_throw(const char *message) {
  std::cerr << "pi_throw: " << message << std::endl;
  throw message;
  //std::terminate();
}

void pi_assert(bool cond) { assert(cond); }

