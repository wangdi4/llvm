//===- plugins/xeon-phi-coi/target_main.cpp - Device executable -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains implementation of the Intel(R) Xeon Phi(TM) target
/// executable which services offload requests from Intel(R) Xeon Phi(TM)
/// libomptarget plugin on the device side.
///
//===----------------------------------------------------------------------===//

#include "omptarget.h"

#include <intel-coi/common/COIEngine_common.h>
#include <intel-coi/sink/COIProcess_sink.h>
#include <intel-coi/sink/COIPipeline_sink.h>

#include <algorithm>
#include <vector>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <link.h>
#ifdef USE_LIBFFI
#include <ffi.h>
#endif // USE_LIBFFI

// Debugging messages. Note that it uses DeviceInfo.ID
#ifdef OMPTARGET_DEBUG
#define DP(...)                                                            \
  {                                                                        \
    fprintf(stderr, "x86_64_mic (MIC%d) --> ", DeviceInfo.ID);             \
    fprintf(stderr, __VA_ARGS__);                                          \
    fflush(nullptr);                                                       \
  }
#else // OMPTARGET_DEBUG
#define DP(...)                                                            \
  {}
#endif // OMPTARGET_DEBUG

namespace {

void fatalError(const char *Fmt, ...) {
  va_list Args;
  va_start(Args, Fmt);
  fprintf(stderr, "device fatal: ");
  vfprintf(stderr, Fmt, Args);
  va_end(Args);
  exit(1);
}

struct DeviceInfoTy {
  // Device index.
  uint32_t ID = -1;

  DeviceInfoTy() {
    COI_ISA_TYPE IsaType;
    COIRESULT R = COIEngineGetIndex(&IsaType, &ID);
    if (R != COI_SUCCESS) {
      fatalError("Failed to get engine ISA and index, error code %d\n", R);
    }
  }
};

DeviceInfoTy DeviceInfo;

template <typename T>
size_t fetch(const char *Buf, size_t Pos, T *Data, size_t Len = 1)
{
  auto Bytes = reinterpret_cast<char*>(Data);
  std::copy_n(Buf + Pos, sizeof(T) * Len, Bytes);
  return Pos + sizeof(T) * Len;
}

} // end anonymous namespace

// Given a list of symbol names returns symbol addresses to the host. Symbol
// names are passed in MiscBuf as string table concatenated by an empty string.
// Addresses are returned through RetData buffer. Buffers are not used so far.
COINATIVELIBEXPORT
void __tgt_device_get_table(
  uint32_t  BufferCount,
  void**    Buffers,
  uint64_t* BuffersLen,
  void*     MiscBuf,
  uint16_t  MiscBufLen,
  void*     RetData,
  uint16_t  RetDataLen
)
{
  // Read inputs from MiscBuf
  auto MiscData = static_cast<const char*>(MiscBuf);
  size_t MiscPos = 0;
  size_t    	NEntries;
  Elf64_Addr	VAddr;

  MiscPos = fetch(MiscData, MiscPos, &NEntries);
  MiscPos = fetch(MiscData, MiscPos, &VAddr);
  const char* LibName = MiscData + MiscPos;

  DP("Reading table for %s, addr %lx, size %lu\n", LibName, VAddr, NEntries);

  // Callback data for the dl_iterate_phdr call below
  struct CallBackDataTy {
    // [IN]  Library name
    const char *Name;
    // [OUT] Base address, where the library was relocated.
    const char *Base;
  } CBD { LibName, nullptr };

  // Walk over process program headers and find where the requested target
  // library landed in the target process.
  dl_iterate_phdr([](struct dl_phdr_info *Info, size_t, void *Data) {
    auto &CBD = *reinterpret_cast<CallBackDataTy*>(Data);
    if (!Info->dlpi_name || strstr(Info->dlpi_name, CBD.Name) == nullptr) {
      return 0;
    }

    // This is the library we are looking for.
    CBD.Base = reinterpret_cast<const char*>(Info->dlpi_addr);
    return 1;
  }, &CBD);
  if (CBD.Base == nullptr) {
    DP("Failed to find a library in the target process\n");
    return;
  }

  // Fill in the return buffer
  auto ETab = reinterpret_cast<const __tgt_offload_entry*>(CBD.Base + VAddr);
  auto TargetAddrs = static_cast<void**>(RetData);
  assert(RetDataLen >= NEntries * sizeof(void*) &&
         "Not enough space in return buffer");
  for (size_t II = 0; II < NEntries; ++II) {
    const auto &E = ETab[II];
    DP("Entry[%lu] addr=%p\tsize=%lu\tname=%s\n", II, E.addr, E.size, E.name);
    TargetAddrs[II] = E.addr;
  }
}

namespace {

#ifndef USE_LIBFFI
// Workaround for libffi issue in the latest MPSS. Helper struct for
// implementing call to an outlined entry. N is a number of entry's arguments.
template<int N, int ... Idxs>
struct EntryCaller : EntryCaller<N-1, N-1, Idxs...>
{};

template<int ... Idxs>
struct EntryCaller<0, Idxs...>
{
  static inline void call(void(*Entry)(...), const std::vector<void*> &Args) {
    Entry(Args[Idxs]...);
  }
};
#endif // USE_LIBFFI

// Calls entry with given arguments in the following way
//   Entry(Args[0], Args[1], .. , Args[N-1])
// where N is the number of arguments in the Args vector.
template<typename F>
void callEntry(F Entry, std::vector<void*> &Args)
{
  auto NumArgs = Args.size();

#ifdef USE_LIBFFI
  // Use libffi to launch execution. All args are references.
  std::vector<ffi_type*> FFIArgsTypes(NumArgs, &ffi_type_pointer);
  std::vector<void*> FFIArgs(NumArgs);
  for(int32_t II = 0; II < NumArgs; ++II) {
    FFIArgs[II] = &Args[II];
  }

  ffi_cif CIF;
  auto R = ffi_prep_cif(&CIF, FFI_DEFAULT_ABI, NumArgs, &ffi_type_void,
                        FFIArgsTypes.data());
  if (R != FFI_OK) {
    fatalError("Error preparing arguments for device entry call\n");
  }

  // Call native entry
  ffi_call(&CIF, Entry, nullptr, FFIArgs.data());
#else // USE_LIBFFI
  // Workaround for libffi issue in the latest MPSS. I have limited the number
  // of entry arguments by 100, hopefully that will be enough for the majority
  // of OpenMP programs.
  switch (NumArgs) {
    case 0: EntryCaller<0>::call(Entry, Args); break;
    case 1: EntryCaller<1>::call(Entry, Args); break;
    case 2: EntryCaller<2>::call(Entry, Args); break;
    case 3: EntryCaller<3>::call(Entry, Args); break;
    case 4: EntryCaller<4>::call(Entry, Args); break;
    case 5: EntryCaller<5>::call(Entry, Args); break;
    case 6: EntryCaller<6>::call(Entry, Args); break;
    case 7: EntryCaller<7>::call(Entry, Args); break;
    case 8: EntryCaller<8>::call(Entry, Args); break;
    case 9: EntryCaller<9>::call(Entry, Args); break;
    case 10: EntryCaller<10>::call(Entry, Args); break;
    case 11: EntryCaller<11>::call(Entry, Args); break;
    case 12: EntryCaller<12>::call(Entry, Args); break;
    case 13: EntryCaller<13>::call(Entry, Args); break;
    case 14: EntryCaller<14>::call(Entry, Args); break;
    case 15: EntryCaller<15>::call(Entry, Args); break;
    case 16: EntryCaller<16>::call(Entry, Args); break;
    case 17: EntryCaller<17>::call(Entry, Args); break;
    case 18: EntryCaller<18>::call(Entry, Args); break;
    case 19: EntryCaller<19>::call(Entry, Args); break;
    case 20: EntryCaller<20>::call(Entry, Args); break;
    case 21: EntryCaller<21>::call(Entry, Args); break;
    case 22: EntryCaller<22>::call(Entry, Args); break;
    case 23: EntryCaller<23>::call(Entry, Args); break;
    case 24: EntryCaller<24>::call(Entry, Args); break;
    case 25: EntryCaller<25>::call(Entry, Args); break;
    case 26: EntryCaller<26>::call(Entry, Args); break;
    case 27: EntryCaller<27>::call(Entry, Args); break;
    case 28: EntryCaller<28>::call(Entry, Args); break;
    case 29: EntryCaller<29>::call(Entry, Args); break;
    case 30: EntryCaller<30>::call(Entry, Args); break;
    case 41: EntryCaller<41>::call(Entry, Args); break;
    case 42: EntryCaller<42>::call(Entry, Args); break;
    case 43: EntryCaller<43>::call(Entry, Args); break;
    case 44: EntryCaller<44>::call(Entry, Args); break;
    case 45: EntryCaller<45>::call(Entry, Args); break;
    case 46: EntryCaller<46>::call(Entry, Args); break;
    case 47: EntryCaller<47>::call(Entry, Args); break;
    case 48: EntryCaller<48>::call(Entry, Args); break;
    case 49: EntryCaller<49>::call(Entry, Args); break;
    case 50: EntryCaller<50>::call(Entry, Args); break;
    case 51: EntryCaller<51>::call(Entry, Args); break;
    case 52: EntryCaller<52>::call(Entry, Args); break;
    case 53: EntryCaller<53>::call(Entry, Args); break;
    case 54: EntryCaller<54>::call(Entry, Args); break;
    case 55: EntryCaller<55>::call(Entry, Args); break;
    case 56: EntryCaller<56>::call(Entry, Args); break;
    case 57: EntryCaller<57>::call(Entry, Args); break;
    case 58: EntryCaller<58>::call(Entry, Args); break;
    case 59: EntryCaller<59>::call(Entry, Args); break;
    case 60: EntryCaller<60>::call(Entry, Args); break;
    case 61: EntryCaller<61>::call(Entry, Args); break;
    case 62: EntryCaller<62>::call(Entry, Args); break;
    case 63: EntryCaller<63>::call(Entry, Args); break;
    case 64: EntryCaller<64>::call(Entry, Args); break;
    case 65: EntryCaller<65>::call(Entry, Args); break;
    case 66: EntryCaller<66>::call(Entry, Args); break;
    case 67: EntryCaller<67>::call(Entry, Args); break;
    case 68: EntryCaller<68>::call(Entry, Args); break;
    case 69: EntryCaller<69>::call(Entry, Args); break;
    case 70: EntryCaller<70>::call(Entry, Args); break;
    case 71: EntryCaller<71>::call(Entry, Args); break;
    case 72: EntryCaller<72>::call(Entry, Args); break;
    case 73: EntryCaller<73>::call(Entry, Args); break;
    case 74: EntryCaller<74>::call(Entry, Args); break;
    case 75: EntryCaller<75>::call(Entry, Args); break;
    case 76: EntryCaller<76>::call(Entry, Args); break;
    case 77: EntryCaller<77>::call(Entry, Args); break;
    case 78: EntryCaller<78>::call(Entry, Args); break;
    case 79: EntryCaller<79>::call(Entry, Args); break;
    case 80: EntryCaller<80>::call(Entry, Args); break;
    case 81: EntryCaller<81>::call(Entry, Args); break;
    case 82: EntryCaller<82>::call(Entry, Args); break;
    case 83: EntryCaller<83>::call(Entry, Args); break;
    case 84: EntryCaller<84>::call(Entry, Args); break;
    case 85: EntryCaller<85>::call(Entry, Args); break;
    case 86: EntryCaller<86>::call(Entry, Args); break;
    case 87: EntryCaller<87>::call(Entry, Args); break;
    case 88: EntryCaller<88>::call(Entry, Args); break;
    case 89: EntryCaller<89>::call(Entry, Args); break;
    case 90: EntryCaller<90>::call(Entry, Args); break;
    case 91: EntryCaller<91>::call(Entry, Args); break;
    case 92: EntryCaller<92>::call(Entry, Args); break;
    case 93: EntryCaller<93>::call(Entry, Args); break;
    case 94: EntryCaller<94>::call(Entry, Args); break;
    case 95: EntryCaller<95>::call(Entry, Args); break;
    case 96: EntryCaller<96>::call(Entry, Args); break;
    case 97: EntryCaller<97>::call(Entry, Args); break;
    case 98: EntryCaller<98>::call(Entry, Args); break;
    case 99: EntryCaller<99>::call(Entry, Args); break;
    default:
      fatalError("Too many arguments for outlined entry %d\n", NumArgs);
  }
#endif // USE_LIBFFI
}

} // end anonymous namespace

COINATIVELIBEXPORT
void __tgt_device_compute(
  uint32_t  BufferCount,
  void**    Buffers,
  uint64_t* BuffersLen,
  void*     MiscBuf,
  uint16_t  MiscBufLen,
  void*     ReturnData,
  uint16_t  ReturnDataLen
)
{
  auto MiscData = static_cast<const char*>(MiscBuf);
  size_t MiscPos = 0;

  // Prepare arguments for the outlined entry
  int32_t NumArgs = 0;
  MiscPos = fetch(MiscData, MiscPos, &NumArgs);
  assert(NumArgs >= 0 && "Illegal number of arguments");

  std::vector<void*> EntryArgs(NumArgs);
  for (int32_t II = 0; II < NumArgs; ++II) {
    MiscPos = fetch(MiscData, MiscPos, &EntryArgs[II]);
  }

  // Get entry address
  union {
#ifdef USE_LIBFFI
    void(*Entry)();
#else // USE_LIBFFI
    void(*Entry)(...);
#endif // USE_LIBFFI
    void* Addr;
  };
  MiscPos = fetch(MiscData, MiscPos, &Addr);
  assert(MiscPos <= MiscBufLen && "Exceeded misc buffer length");

  DP("Executing target entry %p ...\n", Addr);

  callEntry(Entry, EntryArgs);

  DP("... Done\n");
}

int main(int argc, char **argv)
{
  DP("Entered main, starting pipeline.\n");

  COIRESULT R = COIPipelineStartExecutingRunFunctions();
  if (R != COI_SUCCESS) {
    DP("Failed to start pipeline, error code %d\n", R);
    return 1;
  }

  DP("Waiting for shutdown\n");
  R = COIProcessWaitForShutdown();
  if (R != COI_SUCCESS) {
    DP("Error waiting for shutdown, error code %d\n", R);
    return 1;
  }

  DP("Exiting main\n");

  return 0;
}
