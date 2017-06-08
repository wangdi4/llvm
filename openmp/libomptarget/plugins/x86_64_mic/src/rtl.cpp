//=== plugins/xeon-phi-coi/rtl.cpp - Target RTLs implementation -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains implementation of a plugin RTL which targets
/// Intel(R) Xeon Phi(TM) devices.
///
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdio>
#include <cstring>
#include <elf.h>
#include <map>
#include <mutex>
#include <pthread.h>
#include <vector>
#include <sstream>

#include <intel-coi/source/COIEngine_source.h>
#include <intel-coi/source/COIProcess_source.h>
#include <intel-coi/source/COIPipeline_source.h>
#include <intel-coi/source/COIBuffer_source.h>

#include "omptargetplugin.h"
#include "utils.h"

#ifdef OMPTARGET_DEBUG
#define DP(...)                                                            \
  {                                                                        \
    fprintf(stderr, "x86_64_mic (HOST) --> ");                             \
    fprintf(stderr, __VA_ARGS__);                                          \
    fflush(nullptr);                                                       \
  }
#else
#define DP(...)                                                            \
  {}
#endif

namespace {

// Name of ELF section where compiler puts entry table
const char * const EntryTableSectionName = ".omp_offloading.entries";
  
// target_main.dump is generated at build time from the target_main executable
const uint8_t TargetBinary[] = {
#include "target_main.dump"
};

// Utility routines for writing data as bytes to a vector of characters.
template <typename T>
void emit(std::vector<char> &Buf, T Data)
{
  auto Bytes = reinterpret_cast<const char*>(&Data);
  std::copy_n(Bytes, sizeof(T), std::back_inserter(Buf));
}

template <typename T>
void emit(std::vector<char> &Buf, const T *Data, size_t Len)
{
  auto Bytes = reinterpret_cast<const char*>(Data);
  std::copy_n(Bytes, sizeof(T) * Len, std::back_inserter(Buf));
}

/// Class containing all the device information.
class DeviceInfoTy {
  // COI runtime library wrapper
  class COITy {
  public:
    // Pointers to functions from COI library which are used by the plugin.
    // Will add more function on demand.

    COIRESULT (*EngineGetCount)(
        COI_DEVICE_TYPE     in_DeviceType,
        uint32_t           *out_pNumEngines);

    COIRESULT (*EngineGetHandle)(
        COI_DEVICE_TYPE     in_DeviceType,
        uint32_t            in_EngineIndex,
        COIENGINE          *out_pEngineHandle);

    COIRESULT (*ProcessCreateFromMemory)(
        COIENGINE           in_Engine,
        const char         *in_pBinaryName,
        const void         *in_pBinaryBuffer,
        uint64_t            in_BinaryBufferLength,
        int                 in_Argc,
        const char        **in_ppArgv,
        uint8_t             in_DupEnv,
        const char        **in_ppAdditionalEnv,
        uint8_t             in_ProxyActive,
        const char         *in_Reserved,
        uint64_t            in_InitialBufferSpace,
        const char         *in_LibrarySearchPath,
        const char         *in_FileOfOrigin,
        uint64_t            in_FileOfOriginOffset,
        COIPROCESS         *out_pProcess);

    COIRESULT (*ProcessDestroy)(
        COIPROCESS          in_Process,
        int32_t             in_WaitForMainTimeout,
        uint8_t             in_ForceDestroy,
        int8_t             *out_pProcessReturn,
        uint32_t           *out_pTerminationCode);

    COIRESULT (*ProcessGetFunctionHandles)(
        COIPROCESS          in_Process,
        uint32_t            in_NumFunctions,
        const char        **in_ppFunctionNameArray,
        COIFUNCTION        *out_pFunctionHandleArray);

    COIRESULT (*ProcessLoadLibraryFromMemory)(
        COIPROCESS          in_Process,
        const void         *in_pLibraryBuffer,
        uint64_t            in_LibraryBufferLength,
        const char         *in_pLibraryName,
        const char         *in_LibrarySearchPath,
        const char         *in_FileOfOrigin,
        uint64_t            in_FileOfOriginOffset,
        uint32_t            in_Flags,
        COILIBRARY         *out_pLibrary);

    COIRESULT (*ProcessUnloadLibrary)(
        COIPROCESS          in_Process,
        COILIBRARY          in_Library);

    COIRESULT (*PipelineCreate)(
        COIPROCESS          in_Process,
        COI_CPU_MASK        in_Mask,
        uint32_t            in_StackSize,
        COIPIPELINE        *out_pPipeline);

    COIRESULT (*PipelineDestroy)(
        COIPIPELINE         in_Pipeline);

    COIRESULT (*PipelineRunFunction)(
        COIPIPELINE         in_Pipeline,
        COIFUNCTION         in_Function,
        uint32_t            in_NumBuffers,
        const COIBUFFER    *in_pBuffers,
        const COI_ACCESS_FLAGS *in_pBufferAccessFlags,
        uint32_t            in_NumDependencies,
        const COIEVENT     *in_pDependencies,
        const void         *in_pMiscData,
        uint16_t            in_MiscDataLen,
        void               *out_pAsyncReturnValue,
        uint16_t            in_AsyncReturnValueLen,
        COIEVENT           *out_pCompletion);

    COIRESULT (*BufferCreate)(
        uint64_t            in_Size,
        COI_BUFFER_TYPE     in_Type,
        uint32_t            in_Flags,
        const void         *in_pInitData,
        uint32_t            in_NumProcesses,
        const COIPROCESS   *in_pProcesses,
        COIBUFFER          *out_pBuffer);

    COIRESULT (*BufferCreateFromMemory)(
        uint64_t            in_Size,
        COI_BUFFER_TYPE     in_Type,
        uint32_t            in_Flags,
        void               *in_Memory,
        uint32_t            in_NumProcesses,
        const COIPROCESS   *in_pProcesses,
        COIBUFFER          *out_pBuffer);

    COIRESULT (*BufferDestroy)(
        COIBUFFER           in_Buffer);

    COIRESULT (*BufferGetSinkAddressEx)(
        COIPROCESS          in_Process,
        COIBUFFER           in_Buffer,
        uint64_t           *out_pAddress);

    COIRESULT (*BufferWrite)(
        COIBUFFER           in_DestBuffer,
        uint64_t            in_Offset,
        const void         *in_pSourceData,
        uint64_t            in_Length,
        COI_COPY_TYPE       in_Type,
        uint32_t            in_NumDependencies,
        const COIEVENT     *in_pDependencies,
        COIEVENT           *out_pCompletion);

    COIRESULT (*BufferRead)(
        COIBUFFER           in_SourceBuffer,
        uint64_t            in_Offset,
        void               *in_pDestData,
        uint64_t            in_Length,
        COI_COPY_TYPE       in_Type,
        uint32_t            in_NumDependencies,
        const COIEVENT     *in_pDependencies,
        COIEVENT           *out_pCompletion);

  public:
    bool init() {
      const char* LibName = "libcoi_host.so.0";

      DP("Loading COI library %s ...\n", LibName);

      if (!(LibHandle = DynLibTy(LibName, RTLD_NOW))) {
        DP("Failed to load the library\n");
        return false;
      }

      if (!getFunc(&EngineGetCount,
                   "COIEngineGetCount")                          ||
          !getFunc(&EngineGetHandle,
                   "COIEngineGetHandle")                         ||
          !getFunc(&ProcessCreateFromMemory,
                   "COIProcessCreateFromMemory")                 ||
          !getFunc(&ProcessDestroy,
                   "COIProcessDestroy")                          ||
          !getFunc(&ProcessGetFunctionHandles,
                   "COIProcessGetFunctionHandles")               ||
          !getFunc(&ProcessLoadLibraryFromMemory,
                   "COIProcessLoadLibraryFromMemory", "COI_2.0") ||
          !getFunc(&ProcessUnloadLibrary,
                   "COIProcessUnloadLibrary")                    ||
          !getFunc(&PipelineCreate, 
                   "COIPipelineCreate")                          ||
          !getFunc(&PipelineDestroy,
                   "COIPipelineDestroy")                         ||
          !getFunc(&PipelineRunFunction,
                   "COIPipelineRunFunction")                     ||
          !getFunc(&BufferCreate,
                   "COIBufferCreate")                            ||
          !getFunc(&BufferCreateFromMemory,
                   "COIBufferCreateFromMemory")                  ||
          !getFunc(&BufferDestroy,
                   "COIBufferDestroy")                           ||
          !getFunc(&BufferGetSinkAddressEx,
                   "COIBufferGetSinkAddressEx")                  ||
          !getFunc(&BufferWrite,
                   "COIBufferWrite")                             ||
          !getFunc(&BufferRead,
                   "COIBufferRead")) {
        fini();
        return false;
      }

      DP("COI library loaded successfully\n");
      return true;
    }

    void fini() {
      LibHandle.close();
    }

  private:
    template <typename Ret, typename... Args>
    using FuncTy = Ret(Args... args);
    
    template <typename Ret, typename... Args>
    bool getFunc(FuncTy<Ret, Args...> ** Func, const char *Name,
                 const char *Ver = "COI_1.0") {
      if (!(*Func = LibHandle.getSymbol<FuncTy<Ret, Args...>>(Name, Ver))) {
        DP("Failed to find %s in COI library\n", Name);
        return false;
      }
      return true;
    }

    // COI Library handle
    DynLibTy LibHandle;
  };
    
  // Plugin settings.
  struct SettingsTy {
    uint64_t    InitialBufferSpace;
    uint32_t    TargetStackSize;
    std::string TargetLibraryPath;
    bool        TargetDoProxyIO;

    void init() {
      // Initialize settings. TODO: using default values so far, but in
      // future we should provide a way to override these defaults.
      InitialBufferSpace = 0;
      TargetStackSize = 12 * 1024 * 1024;
      TargetDoProxyIO = true;
    
      if (const char *Env = getenv("LD_LIBRARY_PATH")) {
        TargetLibraryPath = Env;
      }
    }
  };

  // Data which is associated with host threads
  struct ThrDataTy {
    ThrDataTy() {
      for (auto & Pipeline : Pipelines) {
        Pipeline = nullptr;
      }
    }

    ~ThrDataTy() {
      for (auto & Pipeline : Pipelines) {
        if (Pipeline) {
          DP("Destroying pipeline\n");
          auto R = COI.PipelineDestroy(Pipeline);
          if (R != COI_SUCCESS) {
            DP("Error destroying pipeline, error code %d\n", R);
          }
          Pipeline = nullptr;
        }
      }
    }

    // Pipelines
    COIPIPELINE Pipelines[COI_MAX_ISA_MIC_DEVICES];
  };

public:
  // The data associated with each device
  class DeviceTy {
  public:
    bool start(int DeviceID) {
      assert(Process == nullptr && "process has already been started");

      // Set ID for the device.
      ID = DeviceID;

      // Create execution context in the specified device
      DP("Getting device %d handle\n", ID);
      COIRESULT R = COI.EngineGetHandle(COI_DEVICE_MIC, ID, &Engine);
      if (R != COI_SUCCESS) {
        DP("Error geting device %d handle, error code %d\n", ID, R);
        return false;
      }

      DP("Loading target binary to device %d\n", ID);
      R = COI.ProcessCreateFromMemory(
        Engine,                                 // in_Engine
        ".omp.offloading.main",                 // in_pBinaryName
        TargetBinary,                           // in_pBinaryBuffer
        sizeof(TargetBinary),                   // in_BinaryBufferLength
        0,                                      // in_Argc
        nullptr,                                // in_ppArgv
        false,                                  // in_DupEnv
        nullptr,                                // in_ppAdditionalEnv
        Settings.TargetDoProxyIO,               // in_ProxyActive
        nullptr,                                // in_Reserved
        Settings.InitialBufferSpace,            // in_InitialBufferSpace
        Settings.TargetLibraryPath.c_str(),     // in_LibrarySearchPath
        nullptr,                                // in_FileOfOrigin
        0,                                      // in_FileOfOriginOffset
        &Process                                // out_pProcess
      );
      if (R != COI_SUCCESS) {
        DP("Cannot start process on device %d, error code %d\n", ID, R);
        return false;
      }

      // get function handles
      R = COI.ProcessGetFunctionHandles(Process, NumTargetFuncs,
                                        TargetFuncNames, TargetFuncs);
      if (R != COI_SUCCESS) {
        DP("Cannot get function handles on device %d, error code %d\n", ID, R);
        return false;
      }
      return true;
    }

    void stop() {
      if (Process) {
        uint32_t Sig;
        int8_t Ret;
        COIRESULT R = COI.ProcessDestroy(Process, -1, 0, &Ret, &Sig);
        if (R != COI_SUCCESS) {
          DP("Failed to stop process on device %d, error code %d\n", ID, R);
          return;
        }
        DP("Device %d process: signal %d, exit code %d\n", ID, Sig, Ret);
        Process = nullptr;
      }
    }

    __tgt_target_table* loadLibrary(__tgt_device_image *Image) {
      auto Start = static_cast<const char*>(Image->ImageStart);
      auto End = static_cast<const char*>(Image->ImageEnd);

      DP("Loading library on device %d: start %p, end %p\n", ID, Start, End);

      // Target libraries do not have SONAME (currently), so we need to provide
      // some name to COI. Lets compose some fake unique name 
      static int Counter = 0;
      std::ostringstream SS;
      SS << ".omp.offloading.library." << ++Counter;

      // Load library to the device
      COILIBRARY Lib;
      COIRESULT R = COI.ProcessLoadLibraryFromMemory(
        Process,                               // in_Process
        Start,                                 // in_pLibraryBuffer
        End - Start,                           // in_LibraryBufferLength
        SS.str().c_str(),                      // in_pLibraryName
        Settings.TargetLibraryPath.c_str(),    // in_LibrarySearchPath
        nullptr,                               // in_FileOfOrigin
        0,                                     // in_FileOfOriginOffset
        COI_LOADLIBRARY_V1_FLAGS,              // in_Flags
        &Lib                                   // out_pLibrary
      );
      if (R != COI_SUCCESS) {
        DP("Error loading library to the device %d, error code %d\n", ID, R);
        return nullptr;
      }
      return getTargetEntryTable(Image, SS.str());
    }

    void* allocBuffer(uint64_t Size) {
      COIBUFFER Buf;
      COIRESULT R = COI.BufferCreate(
        Size,                   // in_Size
        COI_BUFFER_NORMAL,      // in_Type
        0,                      // in_Flags
        0,                      // in_pInitData
        1,                      // in_NumProcesses
        &Process,               // in_pProcesses
        &Buf                    // out_pBuffer
      );
      if (R != COI_SUCCESS) {
        DP("Failed to create buffer for device %d, error code %d\n", ID, R);
        return nullptr;
      }

      uint64_t SinkAddr = 0;
      R = COI.BufferGetSinkAddressEx(Process, Buf, &SinkAddr);
      if (R != COI_SUCCESS) {
        DP("Cannot get buffer address on device %d, error code %d\n", ID, R);
        COI.BufferDestroy(Buf);
        return 0;
      }

      auto TargetPtr = reinterpret_cast<void*>(SinkAddr);

      auto P = BufferMap.insert({ MemBufferTy(TargetPtr, Size), Buf });
      assert(P.second && "New buffer overlaps with existing buffer\n");

      return TargetPtr;
    }

    bool freeBuffer(void *TargetPtr) {
      auto It = BufferMap.find(MemBufferTy(TargetPtr, 0));
      if (It == BufferMap.end()) {
        // No buffer for given address
        return false;
      }

      COIRESULT R = COI.BufferDestroy(It->second);
      if (R != COI_SUCCESS) {
        DP("Failed to delete buffer for device %d, error code %d\n", ID, R);
        return false;
      }

      BufferMap.erase(It);
      return true;
    }

    bool writeBuffer(void *TargetPtr, const void *Ptr, size_t Size) const {
      COIBUFFER Buffer;
      ptrdiff_t Offset;
      BufferDestroyerTy BD;

      auto It = BufferMap.find(MemBufferTy(TargetPtr, 0));
      if (It != BufferMap.end()) {
        Buffer = It->second;
        Offset = static_cast<char*>(TargetPtr) -
                 static_cast<char*>(It->first.getStart());
      }
      else {
        // We do not have buffer for the given target address which means
        // that it was created outside of OpenMP. Try creating a temporary
        // buffer around the given target address range.
        DP("Creating temporary buffer for %p\n", TargetPtr);
        if (!(BD.Buffer = createBufferFromMemory(TargetPtr, Size, true))) {
          return false;
        }
        Buffer = BD.Buffer;
        Offset = 0;
      }

      COIRESULT R = COI.BufferWrite(
        Buffer,                   // in_DestBuffer
        Offset,                   // in_Offset
        Ptr,                      // in_pSourceData
        Size,                     // in_Length
        COI_COPY_UNSPECIFIED,     // in_Type
        0,                        // in_NumDependencies
        nullptr,                  // in_pDependencies
        nullptr                   // out_pCompletion
      );
      if (R != COI_SUCCESS) {
        DP("Error writing buffer data for device %d, error code %d\n", ID, R);
        return false;
      }
      return true;
    }

    bool readBuffer(void *TargetPtr, void *Ptr, size_t Size) {
      COIBUFFER Buffer;
      ptrdiff_t Offset;
      BufferDestroyerTy BD;

      auto It = BufferMap.find(MemBufferTy(TargetPtr, 0));
      if (It != BufferMap.end()) {
        Buffer = It->second;
        Offset = static_cast<char*>(TargetPtr) -
                 static_cast<char*>(It->first.getStart());
      }
      else {
        // Create temporary buffer around the given target address range.
        DP("Creating temporary buffer for %p\n", TargetPtr);
        if (!(BD.Buffer = createBufferFromMemory(TargetPtr, Size, true))) {
          return false;
        }
        Buffer = BD.Buffer;
        Offset = 0;
      }

      COIRESULT R = COI.BufferRead(
        Buffer,                   // in_SourceBuffer
        Offset,                   // in_Offset
        Ptr,                      // in_pDestData
        Size,                     // in_Length
        COI_COPY_UNSPECIFIED,     // in_Type
        0,                        // in_NumDependencies
        nullptr,                  // in_pDependencies
        nullptr                   // out_pCompletion
      );
      if (R != COI_SUCCESS) {
        DP("Error reading buffer data on device %d, error code %d\n", ID, R);
        return false;
      }
      return true;
    }

    bool runTargetRegion(void *Entry, void **Args, ptrdiff_t *Offsets,
                         int32_t NumArgs) const {
      assert(NumArgs >= 0 && "Illegal number of arguments");

      DP("Offloading %p to device %d\n", Entry, ID);

      // Prepare Buffers and Misc data for invocation on device.
      // Misc data has the following format:
      //
      // Number of arguments
      //    int32_t NumArgs;
      //
      // Followed by NumArgs addresses
      //    void* Args[NumArgs];
      //
      // Followed by a target entry address.
      //    void*

      std::vector<char> MiscData;
      std::vector<COIBUFFER> Buffers;

      emit(MiscData, NumArgs);
      for (int32_t II = 0; II < NumArgs; ++II) {
        auto It = BufferMap.find(MemBufferTy(Args[II], 0));
        if (It != BufferMap.end()) {
          Buffers.push_back(It->second);
        }
        emit(MiscData, static_cast<char*>(Args[II]) + Offsets[II]);
      }

      // Append Entry address
      emit(MiscData, Entry);

      // Run function on device
      if (!callTargetEntry(TargetFuncCompute, &Buffers, &MiscData)) {
        return false;
      }
      return true;
    }

    int32_t getID() const {
      return ID;
    }

  private:
    // Returns COI pipeline which is associated with the calling thread.
    COIPIPELINE getPipeline() const {
      auto Data = static_cast<ThrDataTy*>(pthread_getspecific(ThreadKey));
      if (!Data) {
        Data = new ThrDataTy();
        if (pthread_setspecific(ThreadKey, Data)) {
          DP("Error setting thread local data\n");
          delete Data;
          return nullptr;
        }
      }

      auto & Pipeline = Data->Pipelines[ID];
      if (!Pipeline) {
        DP("Creating pipeline on device %d\n", ID);
        auto R = COI.PipelineCreate(
          Process,                                // in_Process
          nullptr,                                // in_Mask
          Settings.TargetStackSize,               // in_StackSize
          &Pipeline                               // out_pPipeline
        );
        if (R != COI_SUCCESS) {
          DP("Error creating pipeline on device %d, error code %d\n", ID, R);
          return nullptr;
        }
      }
      return Pipeline;
    }

    template <typename I, typename O = char>
    bool callTargetEntry(int FuncID,
                         const std::vector<COIBUFFER> *Buffers,
                         const std::vector<I> *In,
                         std::vector<O> *Out = nullptr) const {
      assert(FuncID >= 0 && FuncID < NumTargetFuncs && "Unexpected func ID");

      // Get pipeline for the calling thread
      auto Pipeline = getPipeline();
      if (!Pipeline) {
        return false;
      }

      auto NumBufs = Buffers ? Buffers->size() : 0;

      // Access flags for buffers. Assume the worst case - all buffers are
      // modified by the target.
      std::vector<COI_ACCESS_FLAGS> Temp(NumBufs, COI_SINK_WRITE);

      // Args massaging for COI
      auto Bufs = NumBufs ? Buffers->data() : nullptr;
      auto Flags = NumBufs ? Temp.data() : nullptr;
      auto MiscDataSize = In ? In->size() * sizeof(I) : 0;
      auto MiscData = MiscDataSize ? In->data() : nullptr;
      auto RetDataSize = Out ? Out->size() * sizeof(O) : 0;
      auto RetData = RetDataSize ? Out->data() : nullptr;

      auto R = COI.PipelineRunFunction(
        Pipeline,                              // in_Pipeline
        TargetFuncs[FuncID],                   // in_Function
        NumBufs,                               // in_NumBuffers
        Bufs,                                  // in_pBuffers
        Flags,                                 // in_pBufferAccessFlags
        0,                                     // in_NumDependencies
        nullptr,                               // in_pDependencies
        MiscData,                              // in_pMiscData
        MiscDataSize,                          // in_MiscDataLen
        RetData,                               // out_pAsyncReturnValue
        RetDataSize,                           // in_AsyncReturnValueLen
        nullptr                                // out_pCompletion
      );
      if (R != COI_SUCCESS) {
        DP("Failed to run function on device %d, error code %d\n", ID, R);
        return false;
      }
      return true;
    }

    // Creates COI buffer for existing memory (either host or target).
    COIBUFFER createBufferFromMemory(void *Ptr, uint64_t Size,
                                     bool IsTargetPtr) const {
      COIBUFFER Buffer = nullptr;
      COIRESULT R = COI.BufferCreateFromMemory(
        Size,                              // in_Size
        COI_BUFFER_NORMAL,                 // in_Type
        IsTargetPtr ? COI_SINK_MEMORY : 0, // in_Flags
        Ptr,                               // in_Memory
        1,                                 // in_NumProcesses
        &Process,                          // in_pProcesses
        &Buffer                            // out_pBuffer
      );
      if (R != COI_SUCCESS) {
        DP("Cannot create buffer from sink memory on device %d, "
           "error code %d\n", ID, R);
        return nullptr;
      }
      return Buffer;
    }

    // Returns entry table section header for the given image if it present in
    // the image or nullptr otherwise.
    const Elf64_Shdr* findTableSection(const __tgt_device_image *Image) const {
      const auto *Base = static_cast<const char*>(Image->ImageStart);
        
      // This is supposed to be an ELF image with at least one section
      auto EH = reinterpret_cast<const Elf64_Ehdr*>(Base);
      if (memcmp(EH->e_ident, ELFMAG, SELFMAG) != 0 || EH->e_shnum <= 0) {
        return nullptr;
      }

      // Section headers
      auto SH = reinterpret_cast<const Elf64_Shdr*>(Base + EH->e_shoff);

      // Find section header string table
      auto StrIdx = EH->e_shstrndx;
      if (StrIdx == SHN_XINDEX) {
        StrIdx = SH[0].sh_link;
      }
      assert(StrIdx < EH->e_shnum && "String section index is out of range");
      auto StrTab = Base + SH[StrIdx].sh_offset;

      // And finally find the section where the entry table resides
      for (size_t I = 0; I < EH->e_shnum; ++I) {
        if (strcmp(StrTab + SH[I].sh_name, EntryTableSectionName) == 0) {
          assert((SH[I].sh_flags & SHF_ALLOC) &&
                 "Entry table section does not have ALLOC flags");
          assert(SH[I].sh_addralign <= 1 &&
                 "Bad entry table section data alignment");
          return &SH[I];
        }
      }
      return nullptr;
    }

    // Build target entry table for the given target image.
    __tgt_target_table* getTargetEntryTable(const __tgt_device_image *Image,
                                            const std::string &LibName) {
      // Return existing table if we have built it earlier.
      auto It = Image2Entries.find(Image->ImageStart);
      if (It != Image2Entries.end()) {
        return &It->second;
      }

      // Otherwise go to device and fetch addresses from the target process.
      DP("Reading entry table for image %p for device %d\n",
         static_cast<const void*>(Image), ID);

      // Find entry table section in device image, need its virtual address
      // and size.
      auto SH = findTableSection(Image);
      if (SH == nullptr) {
        DP("Cannot find entry table section in image %p\n", Image->ImageStart);
        return nullptr;
      }
      DP("Entry table section addr %lx, size %lu\n", SH->sh_addr, SH->sh_size);

      // Number of entries in the table. Host and target entry tables should
      // have the same number of entries.
      size_t NEntries = Image->EntriesEnd - Image->EntriesBegin;
      if (NEntries != SH->sh_size / sizeof(__tgt_offload_entry)) {
        DP("Host and target entry table size mismatch\n");
        return nullptr;
      }

      // Now we need to go to device, find entry table on the target and return
      // target addresses to the host. MiscData has the following format
      // 
      // Number of entries in the table
      //    size_t  NEntries
      //
      // Entry table address (virtual, before relocation)
      //    intptr_t TabAddr;
      //
      // Library name
      //    char Name[]
      //
      std::vector<char> MiscData;
      emit(MiscData, NEntries);
      emit(MiscData, SH->sh_addr);
      emit(MiscData, LibName.c_str(), LibName.size() + 1);

      // Device is expected to return address of each entry in return data.
      std::vector<void*> Addrs(NEntries);
      if (!callTargetEntry(TargetFuncGetTable, nullptr, &MiscData, &Addrs)) {
        return nullptr;
      }

      // Now we have everything to build the target entry table.
      TargetEntriesTy Entries(NEntries);
      for (size_t II = 0; II < NEntries; ++II) {
        auto &E = Entries.EntriesBegin[II];

        // Replicate entry from the host table
        E = Image->EntriesBegin[II];

        // and patch target address
        E.addr = Addrs[II];

        // Static data entries should be added to buffer map.
        // TODO: should delay creating buffers for these entries until data
        // is updated via OpenMP constructs, i.e. create buffers on demand.
        if (E.size > 0) {
          auto Res = BufferMap.insert({ MemBufferTy(E.addr, E.size), nullptr });
          if (Res.second) {
            Res.first->second = createBufferFromMemory(E.addr, E.size, true);
          }
        }
      }

      // Finally update table map and return table address
      auto Res = Image2Entries.insert({ Image->ImageStart, Entries });
      assert(Res.second && "Unexpected entry in the table map");

      Res.first->second.fixupTable();
      return &Res.first->second;
    }

    // Helper class for destroying temporary COI buffers
    struct BufferDestroyerTy {
      COIBUFFER Buffer = nullptr;
      ~BufferDestroyerTy() {
        if (Buffer) {
          COIRESULT R = COI.BufferDestroy(Buffer);
          if (R != COI_SUCCESS) {
            DP("Error destroying buffer, error code %d\n", R);
          }
          Buffer = nullptr;
        }
      }
    };

  private:
    // We need to maintain a map between buffer's target address range and
    // COIBUFFER handle for translating target address that is passed to
    // plugin from libomptarget to COIBUFFER.
    struct MemBufferTy {
      MemBufferTy(void *Start, size_t Len) : Addr(Start), Size(Len) {}

      // Returns starting and ending addresses of the buffer.
      void* getStart() const { return Addr; }
      void* getEnd() const { return static_cast<char*>(Addr) + Size; }

      // Returns buffer size
      size_t getSize() const { return Size; }

      // Compare operation for two buffers. Buffers are ordered by the start
      // address unless they overlap.
      bool operator<(const MemBufferTy &Other) const {
        return getStart() < Other.getStart() && !overlaps(Other);
      }

      // Returns true if buffer overlaps with another buffer.
      bool overlaps(const MemBufferTy &Other) const {
        return getStart() < Other.getEnd() && getEnd() > Other.getStart();
      }

    private:
      void*  Addr;
      size_t Size;
    };

    std::map<MemBufferTy, COIBUFFER> BufferMap;

    // Predefined target entries.
    enum {
      TargetFuncGetTable = 0,
      TargetFuncCompute,
      NumTargetFuncs
    };

    int32_t ID = -1;
    COIENGINE Engine = nullptr;
    COIPROCESS Process = nullptr;
    COIFUNCTION TargetFuncs[NumTargetFuncs];

    // Predefined entry names
    static const char* TargetFuncNames[NumTargetFuncs];

    // Wrapper over the target entry table struct. Adds storage for entries.
    struct TargetEntriesTy : public __tgt_target_table {
      explicit TargetEntriesTy(size_t Size) : Entries(Size) {
        fixupTable();
      }

      void fixupTable() {
        EntriesBegin = Entries.data();
        EntriesEnd = EntriesBegin + Entries.size();
      }

      std::vector<__tgt_offload_entry> Entries;
    };

    // Map between target image address and target entry table
    std::map<void*, TargetEntriesTy> Image2Entries;
  };

private:
  // Device data - one element for each MIC device in the system
  std::vector<DeviceTy> Devices;

public:
  // Returns the number of MIC devices available for offloading
  int32_t getNumberOfDevices() const {
    return Devices.size();
  }

  // Returns device data for specified device ID.
  DeviceTy& getDevice(int32_t ID) {
    assert(ID >= 0 && ID < getNumberOfDevices() && "Unexpected device id!");
    return Devices[ID];
  }
  const DeviceTy& getDevice(int32_t ID) const {
    assert(ID >= 0 && ID < getNumberOfDevices() && "Unexpected device id!");
    return Devices[ID];
  }

private:
  DeviceInfoTy() {}

  bool init() {
    DP("Start initializing plugin\n");

    if (!COI.init()) {
      DP("COI runtime is not available.\n");
      return false;
    }

    // Get the number of available MIC devices.
    uint32_t NumDevices = 0;
    COIRESULT R = COI.EngineGetCount(COI_DEVICE_MIC, &NumDevices);
    if (R != COI_SUCCESS) {
      DP("Error getting engines count, error code %d\n", R);
      return false;
    }
    if (NumDevices == 0) {
      DP("There are no MIC devices available for offloading.\n");
      return false;
    }

    DP("There are %d MIC devices available for offloading.\n", NumDevices);

    // Create key for thread local data
    if (pthread_key_create(&ThreadKey, [](void *Ptr) {
      delete static_cast<ThrDataTy*>(Ptr);
    })) {
      DP("Error creating thread local key.\n");
      return false;
    }

    // Initialize plugin settings.
    Settings.init();

    // Initialization is done
    Devices.resize(NumDevices);
    return true;
  }

  void fini() {
    // Destroy per thread data (does destroy pipelines)
    pthread_key_delete(ThreadKey);

    // Stop process on all target devices
    for (auto &Device : Devices) {
      DP("Stopping device %d\n", Device.getID());
      Device.stop();
    }

    // And cleanup COI
    COI.fini();
  }

private:
  static COITy COI;
  static SettingsTy Settings;
  static pthread_key_t ThreadKey;

  friend DeviceInfoTy& getDeviceInfo();
};

DeviceInfoTy::COITy DeviceInfoTy::COI;
DeviceInfoTy::SettingsTy DeviceInfoTy::Settings;
pthread_key_t DeviceInfoTy::ThreadKey;

const char* DeviceInfoTy::DeviceTy::TargetFuncNames[NumTargetFuncs] = {
  "__tgt_device_get_table",
  "__tgt_device_compute"
};

DeviceInfoTy& getDeviceInfo() {
  static DeviceInfoTy DeviceInfo;
  static std::once_flag InitFlag;

  std::call_once(InitFlag, [&]() {
    if (DeviceInfo.init()) {
      atexit([&]() {
        DeviceInfo.fini();
      });
    }
  });
  return DeviceInfo;
}

} // end anonymous namespace

// Plugin API implementation

int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image) {
  auto Ehdr = reinterpret_cast<Elf64_Ehdr*>(Image->ImageStart);

  // Make sure that we are dealing with ELF
  if (memcmp(Ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
    DP("Image %p is not a valid ELF image\n", Image->ImageStart);
    return 0;
  }

  // It should be a dynamic library for x86_64 architecture
  if (Ehdr->e_type != ET_DYN) {
    DP("Image %p is not a shared libray image\n", Image->ImageStart);
    return 0;
  }
  if (Ehdr->e_machine != EM_X86_64) {
    DP("Image %p architecture is not compatible with MIC\n",
       Image->ImageStart);
    return 0;
  }

  // Do we need more checks here?
  return 1;
}

int32_t __tgt_rtl_number_of_devices() {
  return getDeviceInfo().getNumberOfDevices();
}

int32_t __tgt_rtl_init_device(int32_t ID) {
  if (!getDeviceInfo().getDevice(ID).start(ID)) {
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

__tgt_target_table *__tgt_rtl_load_binary(
  int32_t ID,
  __tgt_device_image *Image
) {
  if (!__tgt_rtl_is_valid_binary(Image)) {
    return nullptr;
  }
  return getDeviceInfo().getDevice(ID).loadLibrary(Image);
}

void* __tgt_rtl_data_alloc(int32_t ID, int64_t Size, void *HostPtr) {
  return getDeviceInfo().getDevice(ID).allocBuffer(Size);
}

int32_t __tgt_rtl_data_submit(
  int32_t ID,
  void *TargetPtr,
  void *HostPtr,
  int64_t Size
) {
  if (!getDeviceInfo().getDevice(ID).writeBuffer(TargetPtr, HostPtr, Size)) {
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_retrieve(
  int32_t ID,
  void *HostPtr,
  void *TargetPtr,
  int64_t Size
) {
  if (!getDeviceInfo().getDevice(ID).readBuffer(TargetPtr, HostPtr, Size)) {
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_delete(int32_t ID, void *TargetPtr) {
  if (!getDeviceInfo().getDevice(ID).freeBuffer(TargetPtr)) {
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_region(
  int32_t ID,
  void *Entry,
  void **Args,
  ptrdiff_t *Offsets,
  int32_t NumArgs
) {
  // libomptarget unconditionally adds an extra nullptr to the list of arguments
  // for invocation on the target.
  if (NumArgs > 0) {
    assert(Args[NumArgs-1] == nullptr && "unexpected last argument");
    if (--NumArgs == 0) {
      Args = nullptr;
      Offsets = nullptr;
    }
  }
  if (!getDeviceInfo().getDevice(ID).runTargetRegion(Entry, Args, Offsets, NumArgs)) {
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_team_region(
  int32_t ID,
  void *Entry,
  void **Args,
  ptrdiff_t *Offsets,
  int32_t NumArgs,
  int32_t NumTeams,
  int32_t ThreadLimit,
  uint64_t LoopTripcount
) {
  return __tgt_rtl_run_target_region(ID, Entry, Args, Offsets, NumArgs);
}
