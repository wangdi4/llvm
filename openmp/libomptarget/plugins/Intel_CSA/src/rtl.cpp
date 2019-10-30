//===--- plugins/Intel_CSA/src/rtl.cpp - CSA RTLs Implementation - C++ -*--===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// RTL for CSA UMR.
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <dlfcn.h>
#include <forward_list>
#include <fstream>
#include <iomanip>
#include <link.h>
#include <list>
#include <math.h>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

#include "elf.h"
#include "omptargetplugin.h"
#include "csa/umr.h"

#ifdef OMPTARGET_DEBUG
static int DebugLevel = 0;

#define DP(Level, ...)                                                         \
  do {                                                                         \
    if (DebugLevel >= Level) {                                                 \
      fprintf(stderr, "CSA  (HOST)  --> ");                                    \
      fprintf(stderr, __VA_ARGS__);                                            \
      fflush(nullptr);                                                         \
    }                                                                          \
  } while (false)
#else
#define DP(Level, ...)                                                         \
  {}
#endif

#define NUMBER_OF_DEVICES 1
#define OFFLOADSECTIONNAME "omp_offloading_entries"
#define CSA_ASM_SECTION ".csa.code"
#define CSA_CODE_SECTION ".csa"

// ENVIRONMENT VARIABLES

// If defined, suppresses use of assembly embedded in the binary and specifies
// the file to use instead
#define ENV_ASSEMBLY_FILE "CSA_ASSEMBLY_FILE"

// Variable value has the following format
//   CSA_ASSEMBLY_FILE=<file>[:<entry list>][;<file>[:<entry list>]]
//
// where
//   <file>       A path to CSA aseembly file.
//   <entry list> Comma-separated list of entries defined in the assembly file.
//                For these entries plugin will use assembly from the file
//                instead of compiler generated assembly.
//
// If there is no entry list, assembly file is supposed to define all entries
// which program will execute on CSA.

static std::string AsmFile;
using String2StringMap = std::unordered_map<std::string, std::string>;
static std::unique_ptr<String2StringMap> Entry2AsmFile;

// Specifies that the tool should display the compilation command
// being generated
#define ENV_VERBOSE "CSA_VERBOSE"
static bool Verbosity;

// If defined, dumps the simulator statistics
#define ENV_DUMP_STATS "CSA_DUMP_STATS"
// Enum for indicating when to dump the stats
enum DumpStatLocation {
  DumpStatDisabled = 0,
  DumpStatEndOfProgram = 1,
  DumpStatAfterEachOffload = 2
};
static DumpStatLocation DumpStats = DumpStatDisabled;

// If defined, leave the temporary files on disk in the user's directory
#define ENV_SAVE_TEMPS "CSA_SAVE_TEMPS"
static bool SaveTemps;

// If defined, specifies temporary file prefix. If not defined, defaults
// to process name with "-csa" appended. No effect if CSA_SAVE_TEMPS is
// not defined
#define ENV_TEMP_PREFIX "CSA_TEMP_PREFIX"
static std::string TempPrefix;

// Run function counter.
static std::atomic<unsigned> RunCount;

// Create temporary file. Returns file name if successfull or empty string
// otherwise.
static std::string makeTempFile() {
  char Template[] = "/tmp/tmpfile_XXXXXX";
  int FD = mkstemp(Template);
  if (FD < 0) {
    DP(1, "Error creating temporary file: %s\n", strerror(errno));
    return "";
  }
  close(FD);
  return Template;
}

namespace {

// Represents a dynamic library which is loaded for this target.
class DynLibTy {
  std::string FileName;
  void *Handle = nullptr;

public:
  explicit DynLibTy(const char *Data, size_t Size) {
    // Create temporary file for the dynamic library
    FileName = makeTempFile();
    if (FileName.empty())
      return;

    // Write library contents to the file.
    std::ofstream OFS(FileName, std::ofstream::binary | std::ofstream::trunc);
    if (!OFS || !OFS.write(Data, Size)) {
      DP(1, "Error while writing to a temporary file %s\n", FileName.c_str());
      return;
    }
    OFS.close();

    // And finally load the library.
    Handle = dlopen(FileName.c_str(), RTLD_LAZY);
  }

  DynLibTy(DynLibTy &&That) {
    std::swap(FileName, That.FileName);
    std::swap(Handle, That.Handle);
  }

  ~DynLibTy() {
    if (Handle) {
      dlclose(Handle);
      Handle = nullptr;
    }
    if (!SaveTemps && !FileName.empty()) {
      remove(FileName.c_str());
      FileName.clear();
    }
  }

  DynLibTy &operator=(DynLibTy &&That) {
    if (FileName == That.FileName)
      FileName.clear();
    if (Handle == That.Handle)
      Handle = nullptr;
    std::swap(FileName, That.FileName);
    std::swap(Handle, That.Handle);
    return *this;
  }

  operator bool() const { return Handle != nullptr; }

  const char *getError() const { return dlerror(); }

  const std::string &getName() const { return FileName; }

  Elf64_Addr getBase() const {
    assert(Handle && "invalid handle");
    return reinterpret_cast<struct link_map *>(Handle)->l_addr;
  }

  DynLibTy(const DynLibTy &) = delete;
  DynLibTy &operator=(const DynLibTy &) = delete;
};

// Elf template specialization for CSA (so far it fully matches x86_64).
using CSAElf =
    Elf<EM_X86_64, Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr, Elf64_Rela, Elf64_Sym>;

} // anonymous namespace

#ifdef _MSC_BUILD
static std::string get_process_name() {
  char buf[MAX_PATH];
  char name[_MAX_FNAME];
  GetModuleFileName(NULL, buf, MAX_PATH);
  _splitpath_s(buf, NULL, 0, NULL, 0, name, _MAX_FNAME, NULL, 0);
  return name;
}
#else
static std::string get_process_name() {
  char buf[2048];

  int ret = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
  if (-1 == ret) {
    fprintf(stderr, "Failed to get image name");
    return "unknown-process";
  }

  buf[ret] = '\0';
  char *name = strrchr(buf, '/');
  if (NULL == name) {
    return buf;
  } else {
    return name + 1;
  }
}
#endif

static std::string get_process_name_with_rank() {
  auto name = get_process_name();
  // Append MPI rank to the name if the process is running under MPI.
  if (const auto *Rank = getenv("PMI_RANK"))
    name = name + "-mpi" + Rank;
  return name;
}

static const char *get_offload_name(const char *name) {
  // offload name is embedded as follows
  // __omp_offloading_xxx_xxx_offloadname
  auto *fname = name;
  auto count = 0;
  auto slength = strlen(name);
  while (slength) {
    if (*fname == '_')
      count++;
    if (count == 6)
      break;
    slength--;
    fname++;
  }
  return strlen(fname) ? fname : name;
}

#ifdef OMPTARGET_DEBUG
// Return string describing UMR error.
static const char *getUmrErrorStr(CsaUmrErrors E) {
  switch (E) {
  case kCsaUmrOK:
    return "no error";
  case kCsaUmrErrorContextBusy:
    return "UMR context is being used by another thread";
  case kCsaUmrErrorContextGroupLimit:
    return "too many UMR contexts in a group";
  case kCsaUmrErrorNotContextGroup:
    return "call to UMR contexts from different groups";
  default:
    break;
  }
  return "unknown UMR error";
}
#endif // OMPTARGET_DEBUG

// Check if given address that represents an offload entry points to a vISA
// graph entry. We can determine this by checking the first byte's value. In
// case of vISA graph entry it is guaranteed to be <9 according to the vISA
// binary encoding specification. Otherwise it is expected be an printable
// character.
static bool isVisaGraphEntry(const char *Entry) {
  if (*Entry < 9)
    return true;
  assert((std::isprint(*Entry) || std::isspace(*Entry)) &&
         "must be a printable character");
  return false;
}

namespace {

/// Class containing all the device information.
class RTLDeviceInfoTy {
  // For function entries target address in the offload entry table points to
  // this object. It is a pair of pointers where the first field is an address
  // of a string containing the offload entry name, and the second field can be
  // either a vISA graph entry if we are dealing with the binary vISA graph, or
  // an ascii string with the name of the file containing entry's assembly if
  // entry is encoded as assembly string.
  // TODO: We can get rid of the EntryAddr objects once we completely switch to
  // the binary encoding of CSA graphs.
  using EntryAddr = std::pair<const char *, const char *>;

  // Structure which represents an offload entry table for CSA binary.
  struct EntryTable : public __tgt_target_table {
    static EntryTable *create(const __tgt_offload_entry *Entries, size_t Size) {
      std::unique_ptr<EntryTable> Table(new EntryTable());
      if (!Table->construct(Entries, Size))
        return nullptr;
      return Table.release();
    }

    ~EntryTable() {
      if (!SaveTemps)
        for (const auto &File : Addr2AsmFile)
          remove(File.second.c_str());
    }

  private:
    bool construct(const __tgt_offload_entry *Table, size_t Size) {
      Entries.resize(Size);
      for (size_t I = 0u; I < Size; ++I) {
        Entries[I] = Table[I];
        if (!Entries[I].size) {
          // Function entry. Create an EntryAddr instance for it and assign
          // its address the the entry address.
          DP(2, "Entry[%lu]: name %s\n", I, Entries[I].name);

          // Check if we are dealing with assembly string or vISA graph entry.
          // If it is an assembly, then save it to a temporary file and use
          // file's name as the entry address.
          auto *Addr = static_cast<const char *>(Entries[I].addr);
          if (!isVisaGraphEntry(Addr)) {
            Addr = getEntryFile(Entries[I]);
            if (!Addr)
              return false;
          }

          Addresses.emplace_front(Entries[I].name, Addr);
          Entries[I].addr = &Addresses.front();
        } else
          // It is a data entry. Keep entry address as is. It is supposed to be
          // the same as host's address, but if not, we can always propagate it
          // from the host table.
          DP(2, "Entry[%lu]: name %s, address %p, size %lu\n", I,
             Entries[I].name, Entries[I].addr, Entries[I].size);
      }
      EntriesBegin = Entries.data();
      EntriesEnd = Entries.data() + Size;
      return true;
    }

    const char *getEntryFile(const __tgt_offload_entry &Entry) {
      // There are three possible options for getting assembly for an entry.
      // (1) If we have a single user-defined assembly file, then we use it.
      if (!AsmFile.empty())
        return AsmFile.c_str();

      // (2) Otherwise if there is an entry -> assembly map, try to find
      // assembly file for the given entry.
      if (Entry2AsmFile) {
        auto It = Entry2AsmFile->find(Entry.name);
        if (It != Entry2AsmFile->end())
          return It->second.c_str();
      }

      // (3) Otherwise save the embedded assembly to a file.
      // Check if we have already saved this asm string earlier.
      auto It = Addr2AsmFile.find(Entry.addr);
      if (It != Addr2AsmFile.end())
        return It->second.c_str();

      // We have not seen this entry yet.
      std::string FileName;
      if (SaveTemps) {
        static std::atomic<unsigned int> AsmCount;
        std::stringstream SS;
        SS << TempPrefix << AsmCount++ << ".s";
        FileName = SS.str();

        if (Verbosity)
          fprintf(stderr, "Saving CSA assembly to \"%s\"\n", FileName.c_str());
      } else {
        FileName = makeTempFile();
        if (FileName.empty())
          return nullptr;
      }

      // Save assembly.
      DP(3, "Saving CSA assembly to \"%s\"\n", FileName.c_str());
      std::ofstream OFS(FileName, std::ofstream::trunc);
      if (!OFS || !(OFS << static_cast<const char *>(Entry.addr))) {
        DP(1, "Error while saving assembly to a file %s\n", FileName.c_str());
        return nullptr;
      }
      OFS.close();

      // And update asm files map.
      auto Res = Addr2AsmFile.emplace(Entry.addr, std::move(FileName));
      assert(Res.second && "unexpected entry in the map");
      return Res.first->second.c_str();
    }

  private:
    std::vector<__tgt_offload_entry> Entries;
    std::forward_list<EntryAddr> Addresses;
    std::unordered_map<void *, std::string> Addr2AsmFile;
  };

  // An object which contains all data for a single CSA binary - dynamic library
  // object and the entry table for this binary.
  using CSAImage = std::pair<DynLibTy, std::unique_ptr<EntryTable>>;

  // Keep a list of loaded CSA binaries. This list is always accessed from a
  // single thread so, there is no need to do any synchronization while
  // accessing/modifying it.
  std::forward_list<CSAImage> CSAImages;

public:
  // Loads given CSA image and returns the image's entry table.
  __tgt_target_table *loadImage(const __tgt_device_image *Image) {
    if (!Image)
      return nullptr;

    // Image start and size.
    const char *Start = static_cast<const char *>(Image->ImageStart);
    size_t Size = static_cast<const char *>(Image->ImageEnd) - Start;

    DP(1, "Reading target ELF %p...\n", Start);
    CSAElf Elf;
    if (!Elf.readFromMemory(Start, Size)) {
      DP(1, "Error while parsing target ELF\n");
      return nullptr;
    }

    // Find section with offload entry table.
    const auto *EntriesSec = Elf.findSection(OFFLOADSECTIONNAME);
    if (!EntriesSec) {
      DP(1, "Entries Section Not Found\n");
      return nullptr;
    }
    auto EntriesAddr = EntriesSec->getAddr();
    auto EntriesSize = EntriesSec->getSize();
    DP(1, "Entries Section: address %lx, size %lu\n", EntriesAddr, EntriesSize);

    // Entry table size is expected to match on the host and target sides.
    auto TabSize = EntriesSize / sizeof(__tgt_offload_entry);
    assert(TabSize == size_t(Image->EntriesEnd - Image->EntriesBegin) &&
           "table size mismatch");

    // Create temp file with library contents and load the library.
    DynLibTy DL{Start, Size};
    if (!DL) {
      DP(1, "Error while loading %s - %s\n", DL.getName().c_str(),
         DL.getError());
      return nullptr;
    }
    DP(1, "Saved device binary to %s\n", DL.getName().c_str());

    // Entry table address in the loaded library.
    auto *Tab =
        reinterpret_cast<__tgt_offload_entry *>(DL.getBase() + EntriesAddr);

    // Construct entry table.
    auto *Table = EntryTable::create(Tab, TabSize);
    if (!Table) {
      DP(1, "Error while creating entry table\n");
      return nullptr;
    }

    // Construct new CSA image and insert it into the list.
    CSAImages.emplace_front(std::move(DL), std::unique_ptr<EntryTable>(Table));
    return CSAImages.front().second.get();
  }

public:
  // An object which represents a single OpenMP offload device.
  class Device {
    // Set which keeps addresses for allocated memory.
    class : private std::unordered_set<void *> {
      std::mutex Mutex;

    public:
      void *alloc(size_t Size) {
        void *Ptr = malloc(Size);
        if (!Ptr)
          return nullptr;
        std::lock_guard<std::mutex> Lock(Mutex);
        auto Res = insert(Ptr);
        assert(Res.second && "allocated memory is in the set");
        (void)Res;
        return Ptr;
      }

      void free(void *Ptr) {
        if (!Ptr)
          return;
        {
          std::lock_guard<std::mutex> Lock(Mutex);
          auto It = find(Ptr);
          if (It == end())
            return;
          erase(It);
        }
        free(Ptr);
      }
    } MemoryMap;

  public:
    void *alloc(size_t Size) { return MemoryMap.alloc(Size); }

    void free(void *Ptr) { MemoryMap.free(Ptr); }

  private:
    // Object that contains data associated with each host thread which performs
    // offloading. It includes thread's CSA UMR context (which is created on the
    // first offload) and a map holding bound UMR graphs. No synchronization is
    // necessary for this object because it is accessed and/or modified by one
    // thread only.
    class ThreadContext {
      // Thread's UMR context.
      CsaUmrContext *Context = nullptr;

      // Graphs that have already been bound.
      std::unordered_map<const EntryAddr *, CsaUmrBoundGraph *> Graphs;

    public:
      ThreadContext() {}
      ThreadContext(const ThreadContext &) = delete;
      ThreadContext(ThreadContext &&Other) { swap(Other); }

      ~ThreadContext() {
        if (Context) {
          CsaUmrDeleteContext(Context);
          Context = nullptr;
        }
      }

      ThreadContext &operator=(const ThreadContext &) = delete;
      ThreadContext &operator=(ThreadContext &&Other) {
        swap(Other);
        return *this;
      }

      void swap(ThreadContext &Other) {
        std::swap(Context, Other.Context);
        std::swap(Graphs, Other.Graphs);
      }

      CsaUmrContext *getContext() const { return Context; }

      bool offloadEntry(const EntryAddr *Addr,
                        const std::vector<void *> &Args) {
        auto *Graph = bindGraph(Addr);
        if (!Graph)
          return false;

        auto *Name = Addr->first;

        DP(2, "Running function %s with %lu argument(s)\n", Name, Args.size());
        for (size_t I = 0u; I < Args.size(); ++I)
          DP(2, "\tArg[%lu] = %p\n", I, Args[I]);

        unsigned RunNumber;
        int64_t StartCycles;
        if (Verbosity) {
          RunNumber = RunCount++;
          StartCycles = CsaUmrSimulatorGetCycles(Context);

          fprintf(stderr, "\nRun %u: Running %s on the CSA ..\n", RunNumber,
                  Name);
        }

        CsaUmrCallInfo CI = {0};
        CI.graph = Graph;
        CI.num_inputs = Args.size();
        CI.inputs = reinterpret_cast<const CsaArchValue64 *>(Args.data());
        if (isVisaGraphEntry(Addr->second)) {
          // TODO: remove const_cast<>() once CsaUmrCallInfo::entry is changed
          // to a 'const' pointer.
          CI.entry = const_cast<CsaArchVisaEntry *>(
              reinterpret_cast<const CsaArchVisaEntry *>(Addr->second));
        } else {
          CI.flags = kCsaUmrCallEntryByName;
          CI.entry_name = Name;
        }

        auto E = CsaUmrCall(&CI, 0);
        if (E) {
          DP(1, "Error calling CSA graph - %s\n", getUmrErrorStr(E));
          return false;
        }

        if (DumpStats == DumpStatAfterEachOffload) {
          auto ProcessName = get_process_name_with_rank();
          auto FuncName = get_offload_name(Name);
          std::stringstream SS;
          SS << ProcessName << "-run" << RunNumber << FuncName;
          CsaUmrSimulatorDumpStatistics(Context, SS.str().c_str());
        }

        if (Verbosity) {
          auto Cycles = CsaUmrSimulatorGetCycles(Context) - StartCycles;
          fprintf(stderr, "\nRun %u: %s ran on the CSA in %ld cycles\n\n",
                  RunNumber, Name, Cycles);
        }
        return true;
      }

    private:
      CsaUmrBoundGraph *bindGraph(const EntryAddr *Addr) {
        auto It = Graphs.find(Addr);
        if (It != Graphs.end())
          return It->second;

        if (!Context) {
          auto E = CsaUmrCreateContext(nullptr, nullptr, &Context);
          if (E) {
            DP(1, "Error creating UMR context - %s\n", getUmrErrorStr(E));
            return nullptr;
          }
        }

        CsaUmrBoundGraph *BoundGraph = nullptr;
        if (isVisaGraphEntry(Addr->second)) {
          auto *VisaGraph = CsaArchVisaGraphFromEntry(
              reinterpret_cast<const CsaArchVisaEntry *>(Addr->second));
          DP(1, "Binding vISA graph %p to context %p for entry \"%s\"\n",
             reinterpret_cast<void *>(VisaGraph),
             reinterpret_cast<void *>(Context), Addr->first);
          auto E = CsaUmrBindGraph(Context, VisaGraph, &BoundGraph);
          if (E) {
            DP(1, "Failed to bind CSA graph - %s\n", getUmrErrorStr(E));
            return nullptr;
          }
        } else {
          auto *AsmFile = reinterpret_cast<const char *>(Addr->second);
          DP(5, "Using assembly from \"%s\" for entry \"%s\"\n", AsmFile,
             Addr->first);
          auto E = CsaUmrBindGraphFromFile(Context, AsmFile, &BoundGraph);
          if (E) {
            DP(1, "Failed to bind CSA graph from file - %s\n",
               getUmrErrorStr(E));
            return nullptr;
          }
        }

        Graphs[Addr] = BoundGraph;
        return BoundGraph;
      }
    };

    // Container for thread contexts that attempted to do an offload at least
    // once. Each host thread gets its own context.
    class ThreadContextsMap
        : public std::unordered_map<std::thread::id, ThreadContext> {
      std::mutex Mutex;

    public:
      ThreadContext &getThreadContext() {
        std::lock_guard<std::mutex> Lock(Mutex);
        return (*this)[std::this_thread::get_id()];
      }
    };

    ThreadContextsMap ThreadContexts;

    // Making it a friend so it can access thread contexts for dumping simulator
    // statistics. TODO: this can be removed once we switch to a real hardware
    // because there would be no simulator statistics anymore.
    friend RTLDeviceInfoTy;

  public:
    bool runFunction(void *Ptr, const std::vector<void *> &Args) {
      auto *Addr = static_cast<EntryAddr *>(Ptr);

      // Do offload in the context of the calling thread.
      return ThreadContexts.getThreadContext().offloadEntry(Addr, Args);
    }
  };

private:
  std::unique_ptr<Device[]> Devices;
  int NumDevices = 0;

public:
  int getNumDevices() const { return NumDevices; }

  Device &getDevice(int ID) {
    assert(ID >= 0 && ID < getNumDevices() && "bad device ID");
    return Devices[ID];
  }

  const Device &getDevice(int ID) const {
    assert(ID >= 0 && ID < getNumDevices() && "bad device ID");
    return Devices[ID];
  }

public:
  RTLDeviceInfoTy() {
    NumDevices = NUMBER_OF_DEVICES;
    Devices.reset(new Device[NumDevices]);
  }

  ~RTLDeviceInfoTy() {
    if (DumpStats == DumpStatEndOfProgram) {
      std::unordered_map<std::thread::id, int> TID2Num;

      // Build a map of thread IDs to simple numbers
      int ThreadNum = 0;
      for (int I = 0; I < getNumDevices(); ++I)
        for (const auto &Thr : getDevice(I).ThreadContexts)
          if (TID2Num.find(Thr.first) == TID2Num.end())
            TID2Num[Thr.first] = ThreadNum++;
      int Width = ceil(log10(ThreadNum));
      auto ProcessName = get_process_name_with_rank();

      // Finish up - Dump the stats, release the CSA instances
      for (int I = 0; I < getNumDevices(); ++I)
        for (const auto &Thr : getDevice(I).ThreadContexts)
          if (auto *C = Thr.second.getContext()) {
            // Compose a file name using the following template
            // <exe name>-dev<device num>-thr<thread num>
            std::stringstream SS;
            SS << ProcessName << "-dev" << I << "-thd" << std::setfill('0')
               << std::setw(Width) << TID2Num[Thr.first];
            CsaUmrSimulatorDumpStatistics(C, SS.str().c_str());
          }
    }
    Devices = nullptr;
  }
};

} // anonymous namespace

static RTLDeviceInfoTy &getDeviceInfo() {
  static RTLDeviceInfoTy DeviceInfo;
  static std::once_flag InitFlag;

  std::call_once(InitFlag, [&]() {
    // One time initialization
#ifdef OMPTARGET_DEBUG
    if (const char *Str = getenv("LIBOMPTARGET_DEBUG"))
      DebugLevel = std::stoi(Str);
#endif // OMPTARGET_DEBUG
    Verbosity = getenv(ENV_VERBOSE);
    SaveTemps = getenv(ENV_SAVE_TEMPS);
    if (SaveTemps) {
      // Temp prefix is in effect only if save temps is set.
      if (const char *Str = getenv(ENV_TEMP_PREFIX))
        TempPrefix = Str;
      else
        TempPrefix = get_process_name();
    }
    if (const char *Str = getenv(ENV_DUMP_STATS))
      DumpStats = (DumpStatLocation)std::stoi(Str);
    if (const auto *Str = getenv(ENV_ASSEMBLY_FILE)) {
      // Parse string which is expected to have the following format
      //   CSA_ASSEMBLY_FILE=<file>[:<entry list>][;<file>[:<entry list>]]
      std::istringstream SSV(Str);
      std::string Value;
      while (std::getline(SSV, Value, ';')) {
        auto EntriesPos = Value.find(':');
        if (EntriesPos == std::string::npos)
          // If no entry list is given, then asm file overrides all entries.
          AsmFile = Value;
        else {
          // Otherwise we have asm file name with a list of entries.
          auto File = Value.substr(0u, EntriesPos);

          // Split entries and put them into the entry map.
          std::istringstream SSE(Value.substr(EntriesPos + 1u));
          std::string Entry;
          while (std::getline(SSE, Entry, ',')) {
            if (!Entry2AsmFile)
              Entry2AsmFile.reset(new String2StringMap());
            Entry2AsmFile->insert({Entry, File});
          }
        }
      }

      // Check that we do not have AsmFile and Entry2AsmFile both defined.
      if (!AsmFile.empty() && Entry2AsmFile) {
        fprintf(stderr, "ignoring malformed %s setting\n", ENV_ASSEMBLY_FILE);
        AsmFile = "";
        Entry2AsmFile = nullptr;
      }
    }
  });
  return DeviceInfo;
}

// Plugin API implementation.

int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image) {
  const char *Start = static_cast<char *>(Image->ImageStart);
  size_t Size = static_cast<char *>(Image->ImageEnd) - Start;

  CSAElf Elf;
  if (!Elf.readFromMemory(Start, Size)) {
    DP(1, "Unable to read ELF!\n");
    return false;
  }

  // Check if device binary contains CSA assembly/code section.
  for (const auto *Sec : Elf.getSections())
    if (Sec->getName() == CSA_ASM_SECTION || Sec->getName() == CSA_CODE_SECTION)
      return true;
  DP(1, "No CSA sections in the binary\n");
  return false;
}

int32_t __tgt_rtl_number_of_devices() {
  return getDeviceInfo().getNumDevices();
}

int32_t __tgt_rtl_init_device(int32_t ID) { return OFFLOAD_SUCCESS; }

__tgt_target_table *__tgt_rtl_load_binary(int32_t ID, __tgt_device_image *Ptr) {
  return getDeviceInfo().loadImage(Ptr);
}

void *__tgt_rtl_data_alloc(int32_t ID, int64_t Size, void *HPtr) {
  if (HPtr)
    return HPtr;
  return getDeviceInfo().getDevice(ID).alloc(Size);
}

int32_t __tgt_rtl_data_submit(int32_t ID, void *TPtr, void *HPtr,
                              int64_t Size) {
  if (TPtr != HPtr)
    memcpy(TPtr, HPtr, Size);
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_retrieve(int32_t ID, void *HPtr, void *TPtr,
                                int64_t Size) {
  if (HPtr != TPtr)
    memcpy(HPtr, TPtr, Size);
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_delete(int32_t ID, void *TPtr) {
  getDeviceInfo().getDevice(ID).free(TPtr);
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_team_region(int32_t ID, void *Entry, void **Bases,
                                         ptrdiff_t *Offsets, int32_t NumArgs,
                                         int32_t TeamNum, int32_t ThreadLimit,
                                         uint64_t LoopTripCount) {
  std::vector<void *> Args(NumArgs);
  for (int32_t I = 0; I < NumArgs; ++I)
    Args[I] = static_cast<char *>(Bases[I]) + Offsets[I];

  if (!getDeviceInfo().getDevice(ID).runFunction(Entry, Args))
    return OFFLOAD_FAIL;
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_region(int32_t device_id, void *tgt_entry_ptr,
                                    void **tgt_args, ptrdiff_t *tgt_offsets,
                                    int32_t arg_num) {
  // use one team and one thread.
  return __tgt_rtl_run_target_team_region(device_id, tgt_entry_ptr, tgt_args,
                                          tgt_offsets, arg_num, 1, 1, 0);
}

int32_t
__tgt_rtl_run_target_team_nd_region(int32_t device_id, void *tgt_entry_ptr,
                                    void **tgt_args, ptrdiff_t *tgt_offsets,
                                    int32_t num_args, int32_t num_teams,
                                    int32_t thread_limit, void *loop_desc) {
  return OFFLOAD_FAIL;
}
