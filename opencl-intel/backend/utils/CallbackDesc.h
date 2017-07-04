#include "ImplicitArgsUtils.h"

namespace intel {
// VarArgsCallbackDesc - describes callsbacks which implement built-in functions
// which are var args. These built-in are not present in the built-in library
// because var-args functions cannot be inlined.
struct VarArgsCallbackDesc {
  unsigned FuncCallType; // key defined by the ICT_* enums
  const char *CallbackName;
  unsigned NonVarArgCount;         // Number of non-variadic arguments
//  bool NeedToCopyOverVariadicArgs; // Do the variadic args need to be passed on
                                   // to the callback?
//  bool NeedRuntimeHandleArg;  // true if need to add the RuntimeHandle
                                   // implicit arg?
  bool operator<(const VarArgsCallbackDesc &Other) const {
    return FuncCallType < Other.FuncCallType;
  }
};

// The array below must be sorted by the key which is FuncCallType!!!
static const VarArgsCallbackDesc VarArgsCallbackLookup[] = {
  { ICT_ENQUEUE_KERNEL_LOCALMEM, "ocl20_enqueue_kernel_localmem", 5 },
  { ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM, "ocl20_enqueue_kernel_events_localmem", 8 },
};
static const VarArgsCallbackDesc *getVarArgsCallbackDesc(unsigned FuncCallType) {
  size_t CallbackLookupCount = sizeof(VarArgsCallbackLookup) / sizeof(VarArgsCallbackLookup[0]);
  // This loop asserts the array is sorted
  for (unsigned I = 0; I < CallbackLookupCount - 1; ++I)
    assert(VarArgsCallbackLookup[I] < VarArgsCallbackLookup[I + 1]);
  // Create a dummy object for searching
  VarArgsCallbackDesc Dummy;
  Dummy.FuncCallType = FuncCallType;
  const VarArgsCallbackDesc *D =
      std::lower_bound(&VarArgsCallbackLookup[0], VarArgsCallbackLookup + CallbackLookupCount, Dummy);
  assert(D != VarArgsCallbackLookup + CallbackLookupCount && "Item not found");
  return D;
}

#if 0
// CallbackDesc - describes the 'ordinary' callbacks which are called by
// built-in functions in the built-in library
struct CallbackDesc {
  const char *CallbackName;
  bool NeedRuntimeHandleArg;  // true if need to add the RuntimeHandle
                              // implicit arg
};
static const CallbackDesc CallbackLookup[] = {
  {"ocl20_capture_event_profiling_info", false},
  {"ocl20_create_user_event", false},
  {"ocl20_enqueue_kernel_basic", true},
  {"ocl20_enqueue_kernel_events", true},
  {"ocl20_enqueue_marker", false},
  {"ocl20_get_default_queue", false},
  {"ocl20_get_kernel_preferred_wg_size_multiple", false},
  {"ocl20_get_kernel_wg_size", false},
  {"ocl20_release_event", false},
  {"ocl20_retain_event", false},
  {"ocl20_set_user_event_status", false},
};
#endif
}
