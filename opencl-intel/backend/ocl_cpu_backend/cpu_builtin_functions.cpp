// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "ICLDevBackendServiceFactory.h"
#include "SystemInfo.h"
#include "cl_utils.h"
#include "cpu_dev_limits.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/DynamicLibrary.h"

#include <assert.h>
#include <cstdint>
#ifdef _WIN32
#include <smmintrin.h>
#else
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string>

#ifndef LLVM_BACKEND_NOINLINE_PRE
#if defined(_WIN32)
#define LLVM_BACKEND_NOINLINE_PRE __declspec(noinline)
#else
#define LLVM_BACKEND_NOINLINE_PRE
#endif
#endif
#ifndef LLVM_BACKEND_NOINLINE_POST
#if defined(_WIN32)
#define LLVM_BACKEND_NOINLINE_POST
#else
#define LLVM_BACKEND_NOINLINE_POST __attribute__((__noinline__))
#endif
#endif

using namespace Intel::OpenCL::DeviceBackend;

#ifndef OclCpuBackEnd_EXPORTS
#define OclCpuBackEnd_EXPORTS
#endif

/*******************************************************************************
 *    Synchronization functions (Section 6.11.9)
 ******************************************************************************/
extern "C" LLVM_BACKEND_API void __cpu_dbg_print(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);

  vprintf(fmt, va);
  va_end(va);
}

extern "C" LLVM_BACKEND_API void
__cpu_lprefetch(const char *ptr, size_t numElements, size_t elmSize) {
  size_t totalLines = ((numElements * elmSize) + CPU_DEV_DCU_LINE_SIZE - 1) /
                      CPU_DEV_DCU_LINE_SIZE;

  for (size_t i = 0; i < totalLines; ++i) {
    _mm_prefetch(ptr, _MM_HINT_T0);
    ptr += CPU_DEV_DCU_LINE_SIZE;
  }
}

extern "C" LLVM_BACKEND_API unsigned long long __cpu_get_time_counter() {
  return Intel::OpenCL::DeviceBackend::Utils::SystemInfo::HostTime();
}

// floating-point extend/truncate builtins
extern "C" LLVM_BACKEND_API uint16_t __gnu_f2h_ieee(float a);
extern "C" LLVM_BACKEND_API float __gnu_h2f_ieee(uint16_t a);
extern "C" LLVM_BACKEND_API float __extendhfsf2(uint16_t a);
extern "C" LLVM_BACKEND_API long double __extendhftf2(uint16_t a);
extern "C" LLVM_BACKEND_API long double __extendsftf2(float a);
extern "C" LLVM_BACKEND_API long double __extenddftf2(double a);
extern "C" LLVM_BACKEND_API uint16_t __truncsfhf2(float a);
extern "C" LLVM_BACKEND_API uint16_t __truncdfhf2(double a);
extern "C" LLVM_BACKEND_API uint16_t __trunctfhf2(long double a);
extern "C" LLVM_BACKEND_API float __trunctfsf2(long double a);
extern "C" LLVM_BACKEND_API double __trunctfdf2(long double a);

#if defined(_WIN64)
#if defined(__clang__)
// division for int128
typedef int ti_int __attribute__((mode(TI)));
typedef unsigned tu_int __attribute__((mode(TI)));
extern "C" LLVM_BACKEND_API tu_int __udivmodti4(tu_int, tu_int, tu_int *);
extern "C" LLVM_BACKEND_API tu_int __udivti3(tu_int, tu_int);
extern "C" LLVM_BACKEND_API ti_int __divti3(ti_int, ti_int);
#else // defined(__clang__)
#pragma message(                                                               \
    "warning: 128-bit int division support is disabled for non-clang-like compiler.")
#endif // defined(__clang__)
#endif // defined(_WIN64)

// _chkstk routine used by Cygwin/MingW environments
#ifdef _WIN32
#ifdef _WIN64
extern "C" LLVM_BACKEND_API void ___chkstk_ms();
#else
extern "C" LLVM_BACKEND_API void *___chkstk(size_t);
#endif
#endif

// usage of the function forward declaration prior to the function definition is
// because "__noinline__" attribute cannot appear with definition
extern "C" LLVM_BACKEND_API int
__opencl_printf(const char *format, char *args,
                ICLDevBackendDeviceAgentCallback *pCallback, void *pHandle);
extern "C" LLVM_BACKEND_API int
__opencl_snprintf(char *outstr, size_t size, const char *format, char *args,
                  ICLDevBackendDeviceAgentCallback *pCallback, void *pHandle);

extern "C" LLVM_BACKEND_API int
__devicelib_assert_fail(const char *expr, const char *file, int32_t line,
                        const char *func, uint64_t gid0, uint64_t gid1,
                        uint64_t gid2, uint64_t lid0, uint64_t lid1,
                        uint64_t lid2);
extern "C" LLVM_BACKEND_API void __devicelib_assert_read(void *);

extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
__opencl_dbg_declare_local(void *addr, uint64_t var_metadata_addr,
                           uint64_t expr_metadata_addr, uint64_t gid0,
                           uint64_t gid1,
                           uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
__opencl_dbg_declare_global(void *addr, uint64_t var_metadata_addr,
                            uint64_t gid0, uint64_t gid1,
                            uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
__opencl_dbg_stoppoint(uint64_t metadata_addr, uint64_t gid0, uint64_t gid1,
                       uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
__opencl_dbg_enter_function(uint64_t metadata_addr, uint64_t gid0,
                            uint64_t gid1,
                            uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
__opencl_dbg_exit_function(uint64_t metadata_addr, uint64_t gid0, uint64_t gid1,
                           uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;

// Thread-local storage emulation built-in
struct __emutls_control;
extern "C" LLVM_BACKEND_API void *
__opencl_emutls_get_address(__emutls_control *control);

// IHC support for FPGA
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void *
_ihc_mutex_create() LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
_ihc_mutex_delete(void *) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
_ihc_mutex_lock(void *) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
_ihc_mutex_unlock(void *) LLVM_BACKEND_NOINLINE_POST;

extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void *
_ihc_cond_create() LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
_ihc_cond_delete(void *cv) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
_ihc_cond_notify_one(void *) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
_ihc_cond_wait(void *, void *) LLVM_BACKEND_NOINLINE_POST;

extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void *
_ihc_pthread_create(void *(*)(void *), void *) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
_ihc_pthread_join(void *handle) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
_ihc_pthread_detach(void *handle) LLVM_BACKEND_NOINLINE_POST;

#ifdef _WIN32
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void *
_Znwy(unsigned long long) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
_ZdlPvy(void *, unsigned long long) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
_ZSt14_Xlength_errorPKc(char const *) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
_ZdlPv(void *) LLVM_BACKEND_NOINLINE_POST;
#endif

extern "C" LLVM_BACKEND_API unsigned __opencl_get_cpu_node_id() {
  return getCpuNodeId();
}
extern "C" LLVM_BACKEND_API unsigned __opencl_get_hw_thread_id() {
  return getHWThreadId();
}

extern "C" LLVM_BACKEND_API int __opencl_atexit(void (*function)(void)) {
  return 0;
}

// OpenCL20. Extended execution
class IDeviceCommandManager;
#include "Intel_opencl_task_sequence.h" // INTEL
#include "opencl20_ext_execution.h"

static std::map<std::string, std::string> initSortBuiltinNames() {
  std::map<std::string, std::string> BuiltinNames;
  const std::string TypeSuffix[11] = {"c", "h", "s",  "t", "i", "j",
                                      "l", "m", "Dh", "f", "d"};
  for (auto &KeySuffix : TypeSuffix) {
    std::string KeyOnlyBIName = "_Z10__ocl_sortPU3AS4" + KeySuffix + "jb";
    BuiltinNames.insert(std::make_pair(KeySuffix, KeyOnlyBIName));
    for (auto &ValueSuffix : TypeSuffix) {
      std::string NewValueSuffix =
          (KeySuffix == ValueSuffix) ? "S0_" : "PU3AS4" + ValueSuffix;
      std::string KeyValueBIName =
          "_Z10__ocl_sortPU3AS4" + KeySuffix + NewValueSuffix + "jb";
      BuiltinNames.insert(
          std::make_pair(KeySuffix + "_" + ValueSuffix, KeyValueBIName));
    }
  }
  return BuiltinNames;
}

#define REGISTER_KEY_VALUE_SORT_HELPER(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX)   \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, char, "c")  \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, uint8_t,    \
                             "h")                                              \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, short, "s") \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, uint16_t,   \
                             "t")                                              \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, int, "i")   \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, uint32_t,   \
                             "j")                                              \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, long, "l")  \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, uint64_t,   \
                             "m")                                              \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, half, "Dh") \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, float, "f") \
  REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYTYPE, KEYSUFFIX, double, "d")

#define REGISTER_SORT_BI(BUILTIN_NAME_MAP, ARGTYPE, SUFFIX)                    \
  REGISTER_BI_FUNCTION(BUILTIN_NAME_MAP[SUFFIX], __ocl_sort_##ARGTYPE)

#define REGISTER_KEY_VALUE_SORT_BI(BUILTIN_NAME_MAP, KEYARGTYPE, KEYSUFFIX,    \
                                   VALUEARGTYPE, VALUESUFFIX)                  \
  REGISTER_BI_FUNCTION(BUILTIN_NAME_MAP[std::string(KEYSUFFIX) + "_" +         \
                                        std::string(VALUESUFFIX)],             \
                       __ocl_sort_##KEYARGTYPE##_##VALUEARGTYPE)

#define SORT_BI_REGISTER(BUILTIN_NAME_MAP, TYPE, SUFFIX)                       \
  REGISTER_SORT_BI(BUILTIN_NAME_MAP, TYPE, SUFFIX)                             \
  REGISTER_KEY_VALUE_SORT_HELPER(BUILTIN_NAME_MAP, TYPE, SUFFIX)

#define SORT_KEY_ONLY_BI_DECLARATION(ARGTYPE)                                  \
  extern "C" LLVM_BACKEND_API void __ocl_sort_##ARGTYPE(ARGTYPE *, uint32_t,   \
                                                        bool);

#define SORT_KEY_VALUE_BI_DECLARATION(KeyType, ValueType)                      \
  extern "C" LLVM_BACKEND_API void __ocl_sort_##KeyType##_##ValueType(         \
      KeyType *, ValueType *, uint32_t, bool);

#define SORT_KEY_VALUE_BI_DECLARATION_HELPER(KeyType)                          \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, char)                                 \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, uint8_t)                              \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, short)                                \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, uint16_t)                             \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, int)                                  \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, uint32_t)                             \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, long)                                 \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, uint64_t)                             \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, half)                                 \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, float)                                \
  SORT_KEY_VALUE_BI_DECLARATION(KeyType, double)

#define SORT_BI_DECLARATION(TYPE)                                              \
  SORT_KEY_ONLY_BI_DECLARATION(TYPE)                                           \
  SORT_KEY_VALUE_BI_DECLARATION_HELPER(TYPE)

SORT_BI_DECLARATION(char)
SORT_BI_DECLARATION(uint8_t)
SORT_BI_DECLARATION(short)
SORT_BI_DECLARATION(uint16_t)
SORT_BI_DECLARATION(int)
SORT_BI_DECLARATION(uint32_t)
SORT_BI_DECLARATION(long)
SORT_BI_DECLARATION(uint64_t)
SORT_BI_DECLARATION(half)
SORT_BI_DECLARATION(float)
SORT_BI_DECLARATION(double)

extern "C" LLVM_BACKEND_API void intel_device_barrier(unsigned int);
extern "C" LLVM_BACKEND_API bool intel_is_device_barrier_valid();

// Register BI functions defined above to JIT.
//   MCJIT: use llvm::sys::DynamicLibrary::AddSymbol for each function.
//   LLJIT: use defineAbsolute for each function.
#define REGISTER_BI_FUNCTION(name, ptr)                                        \
  if (LLJIT) {                                                                 \
    if (auto Err = LLJIT->getMainJITDylib().define(llvm::orc::absoluteSymbols( \
            {{LLJIT->mangleAndIntern(name),                                    \
              {llvm::orc::ExecutorAddr::fromPtr(&ptr), flag}}})))              \
      return Err;                                                              \
  } else {                                                                     \
    llvm::sys::DynamicLibrary::AddSymbol(llvm::StringRef(name),                \
                                         (void *)(intptr_t)ptr);               \
  }
llvm::Error RegisterCPUBIFunctions(bool isFPGAEmuDev, llvm::orc::LLJIT *LLJIT) {
  llvm::JITSymbolFlags flag;

  // FIXME: Now LLJIT already defined atexit symbol and will call the
  // registered function when it's destroyed. But we don't know why it
  // can't work fine with clang profile library. We have a workaround
  // here until we figure out the reason.
  if (LLJIT) {
    if (auto Err =
            LLJIT->getMainJITDylib().remove({LLJIT->mangleAndIntern("atexit")}))
      return Err;
  }

  REGISTER_BI_FUNCTION("atexit", __opencl_atexit)
  REGISTER_BI_FUNCTION("__dbg_print", __cpu_dbg_print)
  REGISTER_BI_FUNCTION("__lprefetch", __cpu_lprefetch)
  REGISTER_BI_FUNCTION("__get_time_counter", __cpu_get_time_counter)
  REGISTER_BI_FUNCTION("__opencl_printf", __opencl_printf)
  REGISTER_BI_FUNCTION("__opencl_snprintf", __opencl_snprintf)
  REGISTER_BI_FUNCTION("__devicelib_assert_fail", __devicelib_assert_fail)
  REGISTER_BI_FUNCTION("__devicelib_assert_read", __devicelib_assert_read)
  REGISTER_BI_FUNCTION("__opencl_dbg_declare_local", __opencl_dbg_declare_local)
  REGISTER_BI_FUNCTION("__opencl_dbg_enter_function",
                       __opencl_dbg_enter_function)
  REGISTER_BI_FUNCTION("__opencl_dbg_exit_function", __opencl_dbg_exit_function)
  REGISTER_BI_FUNCTION("__opencl_dbg_declare_global",
                       __opencl_dbg_declare_global)
  REGISTER_BI_FUNCTION("__opencl_dbg_stoppoint", __opencl_dbg_stoppoint)
  REGISTER_BI_FUNCTION("__opencl_get_cpu_node_id", __opencl_get_cpu_node_id)
  REGISTER_BI_FUNCTION("__opencl_get_hw_thread_id", __opencl_get_hw_thread_id)
  REGISTER_BI_FUNCTION("__ocl20_get_default_queue", __ocl20_get_default_queue)
  REGISTER_BI_FUNCTION("__ocl20_enqueue_kernel_basic",
                       __ocl20_enqueue_kernel_basic)
  REGISTER_BI_FUNCTION("__ocl20_enqueue_kernel_localmem",
                       __ocl20_enqueue_kernel_localmem)
  REGISTER_BI_FUNCTION("__ocl20_enqueue_kernel_events",
                       __ocl20_enqueue_kernel_events)
  REGISTER_BI_FUNCTION("__ocl20_enqueue_kernel_events_localmem",
                       __ocl20_enqueue_kernel_events_localmem)
  REGISTER_BI_FUNCTION("__ocl20_enqueue_marker", __ocl20_enqueue_marker)
  REGISTER_BI_FUNCTION("__ocl20_retain_event", __ocl20_retain_event)
  REGISTER_BI_FUNCTION("__ocl20_release_event", __ocl20_release_event)
  REGISTER_BI_FUNCTION("__ocl20_create_user_event", __ocl20_create_user_event)
  REGISTER_BI_FUNCTION("__ocl20_set_user_event_status",
                       __ocl20_set_user_event_status)
  REGISTER_BI_FUNCTION("__ocl20_capture_event_profiling_info",
                       __ocl20_capture_event_profiling_info)
  REGISTER_BI_FUNCTION("__ocl20_get_kernel_wg_size", __ocl20_get_kernel_wg_size)
  REGISTER_BI_FUNCTION("__ocl20_get_kernel_preferred_wg_size_multiple",
                       __ocl20_get_kernel_preferred_wg_size_multiple)
  REGISTER_BI_FUNCTION("__ocl20_is_valid_event", __ocl20_is_valid_event)
#if INTEL_CUSTOMIZATION
  REGISTER_BI_FUNCTION("__ocl_task_sequence_create", __ocl_task_sequence_create)
  REGISTER_BI_FUNCTION("__ocl_task_sequence_async", __ocl_task_sequence_async)
  REGISTER_BI_FUNCTION("__ocl_task_sequence_get", __ocl_task_sequence_get)
  REGISTER_BI_FUNCTION("__ocl_task_sequence_release",
                       __ocl_task_sequence_release)
#endif // INTEL_CUSTOMIZATION
  REGISTER_BI_FUNCTION("__emutls_get_address", __opencl_emutls_get_address)
  // Floating-point extend/truncate builtins
  REGISTER_BI_FUNCTION("__gnu_f2h_ieee", __gnu_f2h_ieee)
  REGISTER_BI_FUNCTION("__gnu_h2f_ieee", __gnu_h2f_ieee)
  REGISTER_BI_FUNCTION("__truncdfhf2", __truncdfhf2)
  REGISTER_BI_FUNCTION("__truncsfhf2", __truncsfhf2)
  REGISTER_BI_FUNCTION("__extendhfsf2", __extendhfsf2)
#ifndef _WIN32
  REGISTER_BI_FUNCTION("__trunctfhf2", __trunctfhf2)
  REGISTER_BI_FUNCTION("__trunctfsf2", __trunctfsf2)
  REGISTER_BI_FUNCTION("__trunctfdf2", __trunctfdf2)
  REGISTER_BI_FUNCTION("__extendsftf2", __extendsftf2)
  REGISTER_BI_FUNCTION("__extenddftf2", __extenddftf2)
  REGISTER_BI_FUNCTION("__extendhftf2", __extendhftf2)
#endif
#if defined(_WIN64) && defined(__clang__)
  REGISTER_BI_FUNCTION("__udivmodti4", __udivmodti4)
  REGISTER_BI_FUNCTION("__udivti3", __udivti3)
  REGISTER_BI_FUNCTION("__divti3", __divti3)
#endif
  // IHS support
  REGISTER_BI_FUNCTION("_ihc_mutex_create", _ihc_mutex_create)
  REGISTER_BI_FUNCTION("_ihc_mutex_delete", _ihc_mutex_delete)
  REGISTER_BI_FUNCTION("_ihc_mutex_lock", _ihc_mutex_lock)
  REGISTER_BI_FUNCTION("_ihc_mutex_unlock", _ihc_mutex_unlock)
  REGISTER_BI_FUNCTION("_ihc_cond_create", _ihc_cond_create)
  REGISTER_BI_FUNCTION("_ihc_cond_delete", _ihc_cond_delete)
  REGISTER_BI_FUNCTION("_ihc_cond_notify_one", _ihc_cond_notify_one)
  REGISTER_BI_FUNCTION("_ihc_cond_wait", _ihc_cond_wait)
  REGISTER_BI_FUNCTION("_ihc_pthread_create", _ihc_pthread_create)
  REGISTER_BI_FUNCTION("_ihc_pthread_join", _ihc_pthread_join)
  REGISTER_BI_FUNCTION("_ihc_pthread_detach", _ihc_pthread_detach)
  REGISTER_BI_FUNCTION("_Z20intel_device_barrierj", intel_device_barrier)
  REGISTER_BI_FUNCTION("_Z29intel_is_device_barrier_validv",
                       intel_is_device_barrier_valid)
#ifdef _WIN32
  if (isFPGAEmuDev) {
    REGISTER_BI_FUNCTION("_Znwy", _Znwy)
    REGISTER_BI_FUNCTION("_ZdlPvy", _ZdlPvy)
    REGISTER_BI_FUNCTION("_ZSt14_Xlength_errorPKc", _ZSt14_Xlength_errorPKc)
    REGISTER_BI_FUNCTION("_ZdlPv", _ZdlPv)
  }
#ifdef _WIN64
  REGISTER_BI_FUNCTION("___chkstk_ms", ___chkstk_ms)
#else
  // _alloca and ___chkstk are the same function
  REGISTER_BI_FUNCTION("_alloca", ___chkstk)
#endif
#endif

  std::map<std::string, std::string> BuiltinNames = initSortBuiltinNames();

  SORT_BI_REGISTER(BuiltinNames, char, "c")
  SORT_BI_REGISTER(BuiltinNames, uint8_t, "h")
  SORT_BI_REGISTER(BuiltinNames, short, "s")
  SORT_BI_REGISTER(BuiltinNames, uint16_t, "t")
  SORT_BI_REGISTER(BuiltinNames, int, "i")
  SORT_BI_REGISTER(BuiltinNames, uint32_t, "j")
  SORT_BI_REGISTER(BuiltinNames, long, "l")
  SORT_BI_REGISTER(BuiltinNames, uint64_t, "m")
  SORT_BI_REGISTER(BuiltinNames, half, "Dh")
  SORT_BI_REGISTER(BuiltinNames, float, "f")
  SORT_BI_REGISTER(BuiltinNames, double, "d")

  return llvm::Error::success();
}
