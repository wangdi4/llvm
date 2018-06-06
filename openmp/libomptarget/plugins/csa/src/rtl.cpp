//===------ plugins/csa/src/rtl.cpp - CSA RTLs Implementation - C++ -*-----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// RTL for CSA simulator
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <dlfcn.h>
#include <forward_list>
#include <fstream>
#include <iomanip>
#include <list>
#include <memory>
#include <mutex>
#include <sstream>
#include <link.h>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

#include "omptargetplugin.h"
#include "csa_invoke.h"
#include "elf.h"

#ifdef OMPTARGET_DEBUG
static int DebugLevel = 0;

#define DP(Level, ...)                                                     \
  do {                                                                     \
    if (DebugLevel >= Level) {                                             \
      fprintf(stderr, "CSA  (HOST)  --> ");                                \
      fprintf(stderr, __VA_ARGS__);                                        \
      fflush(nullptr);                                                     \
    }                                                                      \
  } while (false)
#else
#define DP(Level, ...)                                                     \
  {}
#endif

#define NUMBER_OF_DEVICES 4
#define OFFLOADSECTIONNAME ".omp_offloading.entries"
#define CSA_BITCODE_BOUNDS_SECTION ".csa.bc.bounds"
#define CSA_BITCODE_DATA_SECTION ".csa.bc.data"
#define CSA_CODE_SECTION ".csa.code"

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

// If defined, points to the copy of csa-clang to used. If not defined,
// the tool will attempt to find csa-clang on the user's PATH
#define ENV_CLANG "CSA_CLANG"

// Specifies options to add to the csa-clang command. The command will be
// csa-clang -S -o <tempfile>.s <csa-compile-options> <tempfile>.bc
#define ENV_COMPILE_OPTIONS "CSA_COMPILE_OPTIONS"

// Specifies the path to the LLVMgold.so plugin. If not define
// LD_LIBRARY_PATH will be searched for an executable copy.
#define ENV_GOLD_PLUGIN "CSA_GOLD_PLUGIN"

// Specifies options to add to the ld command
#define ENV_LINK_OPTIONS "CSA_LINK_OPTIONS"

// Specifies the path to the ld.gold linker. If not defined, PATH will be
// searched for the gold linker
#define ENV_LD_GOLD "CSA_LD_GOLD"

// Specifies that the tool should display the compilation command
// being generated
#define ENV_VERBOSE "CSA_VERBOSE"
static bool Verbosity;

// If defined, dumps the simulator statistics after each offloaded procedure
// is run
#define ENV_DUMP_STATS "CSA_DUMP_STATS"
static bool DumpStats;

// If defined all stats for a thread are run in a single CSA instance and
// dumped in a single .stat file (if CSA_DUMP_STATS is defined)
#define ENV_MERGE_STATS "CSA_MERGE_STATS"
static bool MergeStats;

// If defined, leave the temporary files on disk in the user's directory
#define ENV_SAVE_TEMPS "CSA_SAVE_TEMPS"
static bool SaveTemps;
static std::list<std::string> filesToDelete;

// If defined, specifies temporary file prefix. If not defined, defaults
// to process name with "-csa" appended. No effect if CSA_SAVE_TEMPS is
// not defined
#define ENV_TEMP_PREFIX "CSA_TEMP_PREFIX"
static std::string TempPrefix;

// If defined specifies the value to initialize the floating point flags
// to before an offloaded function, and that the floating point flags be
// reported after the offloaded funciton returns
#define ENV_FP_FLAGS "CSA_FP_FLAGS"
static int FPFlags = -1;

// Environment variable the FP flags will be saved in
#define ENV_RUN_FP_FLAGS "CSA_RUN_FP_FLAGS"

namespace {

// Bitcode bounds struct built by the compiler. WARNING! This struct MUST
// match the struct written to the .csa.bc.bounds section in CSAAsmPrinter.cpp!
struct BitcodeBounds {
  uint64_t start;
  uint64_t end;

  uint64_t length() const {
    return end - start;
  }
};

} // anonymous namespace

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
  void *Handle;

public:
  explicit DynLibTy(const char *Data, size_t Size) : Handle(nullptr) {
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

  DynLibTy& operator=(DynLibTy &&That) {
    if (FileName == That.FileName)
      FileName.clear();
    if (Handle == That.Handle)
      Handle = nullptr;
    std::swap(FileName, That.FileName);
    std::swap(Handle, That.Handle);
    return *this;
  }

  operator bool() const {
    return Handle != nullptr;
  }

  const char* getError() const {
    return dlerror();
  }

  const std::string& getName() const {
    return FileName;
  }

  Elf64_Addr getBase() const {
    assert(Handle && "invalid handle");
    return reinterpret_cast<struct link_map*>(Handle)->l_addr;
  }

  DynLibTy(const DynLibTy &) = delete;
  DynLibTy& operator=(const DynLibTy &) = delete;
};

// Elf template specialization for CSA (so far it fully matches x86_64).
using CSAElf =
  Elf<EM_X86_64, Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr, Elf64_Rela, Elf64_Sym>;

} // anonymous namespace

#ifdef _MSC_BUILD
static
std::string get_process_name() {
  char buf[MAX_PATH];
  char name[_MAX_FNAME];
  GetModuleFileName(NULL, buf, MAX_PATH);
  _splitpath_s(buf, NULL, 0, NULL, 0, name, _MAX_FNAME, NULL, 0);
  return name;
#else
static
std::string get_process_name() {
  char buf[2048];

  int ret = readlink("/proc/self/exe", buf, sizeof(buf)-1);
  if (-1 == ret) {
    fprintf(stderr, "Failed to get image name");
    return "unknown-process";
  }

  buf[ret] = '\0';
  char* name = strrchr(buf, '/');
  if (NULL == name) {
    return buf;
  } else {
    return name+1;
  }
}
#endif

static
bool build_csa_assembly(const std::string &tmp_name,
                        const CSAElf::Section *bitcode_bounds,
                        const CSAElf::Section *bitcode_data,
                        std::string &csaAsmFile);

namespace {

/// Class containing all the device information.
class RTLDeviceInfoTy {
  // For function entries target address in the offload entry table for CSA
  // will point to this object. It is a pair of two null-termminated strings
  // where the first string is the offload entry name, and the second is the
  // name of file which contains entry's assembly.
  using EntryAddr = std::pair<const char*, const char*>;

  // Structure which represents an offload entry table for CSA binary.
  struct EntryTable : public __tgt_target_table {
    static EntryTable* create(const __tgt_offload_entry *Entries, size_t Size) {
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

          const auto *FileName = getEntryFile(Entries[I]);
          if (!FileName)
            return false;

          Addresses.emplace_front(Entries[I].name, FileName);
          Entries[I].addr = &Addresses.front();
        }
        else
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
      }
      else {
        FileName = makeTempFile();
        if (FileName.empty())
          return nullptr;
      }

      // Save assembly.
      DP(3, "Saving CSA assembly to \"%s\"\n", FileName.c_str());
      std::ofstream OFS(FileName, std::ofstream::trunc);
      if (!OFS || !(OFS << static_cast<const char*>(Entry.addr))) {
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
    std::unordered_map<void*, std::string> Addr2AsmFile;
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
  __tgt_target_table* loadImage(const __tgt_device_image *Image) {
    if (!Image)
      return nullptr;

    // Image start and size.
    const char *Start = static_cast<const char*>(Image->ImageStart);
    size_t Size = static_cast<const char*>(Image->ImageEnd) - Start;

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

    // Check if target binary contains bitcode sections. If yes, then we need
    // to create assembly first unless user has provided his own assembly file.
    // TODO: remove this code once we stop embedding IR to the target binary.
    const auto *BCBoundsSec = Elf.findSection(CSA_BITCODE_BOUNDS_SECTION);
    const auto *BCDataSec = Elf.findSection(CSA_BITCODE_DATA_SECTION);
    if (BCBoundsSec && BCDataSec && AsmFile.empty()) {
      DP(1, "Offset of bitcode bounds section is (%016lx).\n",
        BCBoundsSec->getAddr());
      DP(1, "Offset of bitcode data section is (%016lx).\n",
        BCDataSec->getAddr());

      if (!build_csa_assembly(DL.getName(), BCBoundsSec, BCDataSec, AsmFile)) {
        DP(1, "Error while compiling bitcode to assembly\n");
        return nullptr;
      }

      // Need to remove assembly file at cleanup. This is a generated file.
      filesToDelete.push_back(AsmFile);
    }

    // Entry table address in the loaded library.
    auto *Tab = reinterpret_cast<__tgt_offload_entry*>(DL.getBase() +
                                                       EntriesAddr);

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
    class : private std::unordered_set<void*> {
      std::mutex Mutex;

    public:
      void* alloc(size_t Size) {
        void *Ptr = malloc(Size);
        if (!Ptr)
          return nullptr;
        std::lock_guard<std::mutex> Lock(Mutex);
        auto Res = insert(Ptr);
        assert(Res.second && "allocated memory is in the set");
        (void) Res;
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
    void* alloc(size_t Size) {
      return MemoryMap.alloc(Size);
    }

    void free(void *Ptr) {
      MemoryMap.free(Ptr);
    }

  public:
    // Simulator data which is associated with offload entry - CSA processor,
    // module and entry.
    using CSAEntry = std::tuple<csa_processor*, csa_module*, csa_entry*>;

    // Maps offload entry to a CSA entry for a thread. No synchronization is
    // necessary for this object because it is accessed and/or modified by one
    // thread only.
    class CSAEntryMap : public std::unordered_map<const EntryAddr*, CSAEntry> {
      csa_processor *SingleProc = nullptr;

    public:
      csa_processor* getSingleProc() const {
        return SingleProc;
      }

    private:
      // Allocates CSA processor. The way how we do it depends on the
      // MergeStats setting. If MergeStats is on then we are using single CSA
      // processor for all entries. Otherwise each entry gets its own processor
      // instance.
      csa_processor* allocProc() {
        if (!MergeStats)
          return csa_alloc("autounit");

        // When MergeStats is on thread is supposed to run all entries in
        // a single CSA processor instance.
        if (!SingleProc)
          SingleProc = csa_alloc("autounit");
        return SingleProc;
      }

    public:
      CSAEntry* getEntry(const EntryAddr *Addr) {
        auto It = find(Addr);
        if (It != end())
          return &It->second;

        auto *Proc = allocProc();
        if (!Proc) {
          DP(1, "Failed to allocate CSA processor\n");
          return nullptr;
        }

        DP(5, "Using assembly from \"%s\" for entry \"%s\"\n",
           Addr->second, Addr->first);
        auto *Mod = csa_assemble(Addr->second, Proc);
        if (!Mod) {
          DP(1, "Failed to assemble entry\n");
          return nullptr;
        }

        auto *Entry = csa_lookup(Mod, Addr->first);
        if (!Entry) {
          DP(1, "Failed to find \"%s\"\n", Addr->first);
          return nullptr;
        }

        auto Res = this->emplace(Addr, std::make_tuple(Proc, Mod, Entry));
        assert(Res.second && "entry is already in the map");
        return &Res.first->second;
      }
    };

    // CSA simulator is not thread safe so we need to keep own CSAEntry map for
    // each thread.
    class ThreadEntryMap :
        public std::unordered_map<std::thread::id, CSAEntryMap> {
      std::mutex Mutex;

    public:
      CSAEntryMap& getEntries() {
        std::lock_guard<std::mutex> Lock(Mutex);
        return (*this)[std::this_thread::get_id()];
      }
    };

  private:
    ThreadEntryMap ThreadEntries;

  public:
    ThreadEntryMap& getThreadEntries() {
      return ThreadEntries;
    }

    bool runFunction(void *Ptr, std::vector<void*> &Args) {
      auto *Addr = static_cast<EntryAddr*>(Ptr);
      auto *Info = ThreadEntries.getEntries().getEntry(Addr);
      if (!Info) {
        DP(1, "Error while creating CSA entry\n");
        return false;
      }

      // Run function counter.
      static std::atomic<unsigned int> RunCount;
      unsigned int RunNumber = RunCount++;

      auto *Name = Addr->first;
      auto *Proc = std::get<0>(*Info);
      auto *Entry = std::get<2>(*Info);

      DP(2, "Running function %s with %lu argument(s)\n", Name, Args.size());
      for (size_t I = 0u; I < Args.size(); ++I)
        DP(2, "\tArg[%lu] = %p\n", I, Args[I]);

      if (FPFlags >= 0)
        csa_set_fp_flags(Proc, FPFlags);

      if (Verbosity)
        fprintf(stderr, "\nRun %u: Running %s on the CSA simulator..\n",
                RunNumber, Name);

      auto Start = csa_cycle_counter(Proc);
      csa_call(Entry, Args.size(), reinterpret_cast<csa_arg*>(Args.data()));
      auto Cycles = csa_cycle_counter(Proc) - Start;

      if (Verbosity)
        fprintf(stderr,
                "\nRun %u: %s ran on the CSA simulator in %llu cycles\n\n",
                RunNumber, Name, Cycles);

      if (FPFlags >= 0) {
        auto Flags = csa_get_fp_flags(Proc);
        fprintf(stderr, "FP Flags at completion of %s : 0x%02x\n", Name, Flags);
        std::stringstream SS;
        SS << Flags;
        setenv(ENV_RUN_FP_FLAGS, SS.str().c_str(), true);
      }

      return true;
    }
  };

private:
  std::unique_ptr<Device[]> Devices;
  int NumDevices = 0;

public:
  int getNumDevices() const {
    return NumDevices;
  }

  Device& getDevice(int ID) {
    assert(ID >= 0 && ID < getNumDevices() && "bad device ID");
    return Devices[ID];
  }

  const Device& getDevice(int ID) const {
    assert(ID >= 0 && ID < getNumDevices() && "bad device ID");
    return Devices[ID];
  }

public:
  RTLDeviceInfoTy() {
    NumDevices = NUMBER_OF_DEVICES;
    Devices.reset(new Device[NumDevices]);
  }

  ~RTLDeviceInfoTy() {
    std::unordered_map<std::thread::id, int> TID2Num;
    std::string ProcessName;
    int Width = 0;

    if (DumpStats) {
      // Build a map of thread IDs to simple numbers
      int ThreadNum = 0;
      for (int I = 0; I < getNumDevices(); ++I)
        for (const auto &Thr : getDevice(I).getThreadEntries())
          if (TID2Num.find(Thr.first) == TID2Num.end())
            TID2Num[Thr.first] = ThreadNum++;
      Width = ceil(log10(ThreadNum));
      ProcessName = get_process_name();
    }

    // Finish up - Dump the stats, release the CSA instances
    for (int I = 0; I < getNumDevices(); ++I)
      for (auto &Thr : getDevice(I).getThreadEntries()) {
        auto cleanup = [&](csa_processor *Proc, const std::string &Entry) {
          if (DumpStats) {
            // Compose a file name using the following template
            // <exe name>-<entry name>-dev<device num>-thr<thread num>
            std::stringstream SS;
            SS << ProcessName << "-" << Entry << "-dev" << I << "-thd"
               << std::setfill('0') << std::setw(Width) << TID2Num[Thr.first];
            csa_dump_statistics(Proc, SS.str().c_str());
          }
          csa_free(Proc);
        };

        if (auto *SingleProc = Thr.second.getSingleProc())
          cleanup(SingleProc, "*");
        else
          for (auto &Entry : Thr.second)
            cleanup(std::get<0>(Entry.second), Entry.first->first);
      }

    // Delete any temporary files we've created
    if (!SaveTemps)
      while (!filesToDelete.empty()) {
        remove(filesToDelete.front().c_str());
        filesToDelete.pop_front();
      }
  }
};

} // anonymous namespace

static RTLDeviceInfoTy& getDeviceInfo() {
  static RTLDeviceInfoTy DeviceInfo;
  static std::once_flag InitFlag;

  std::call_once(InitFlag, [&]() {
    // One time initialization
#ifdef OMPTARGET_DEBUG
    if (const char *Str = getenv("LIBOMPTARGET_DEBUG"))
      DebugLevel = std::stoi(Str);
#endif // OMPTARGET_DEBUG
    Verbosity = getenv(ENV_VERBOSE);
    DumpStats = getenv(ENV_DUMP_STATS);
    MergeStats = getenv(ENV_MERGE_STATS);
    SaveTemps = getenv(ENV_SAVE_TEMPS);
    if (SaveTemps) {
      // Temp prefix is in effect only if save temps is set.
      if (const char *Str = getenv(ENV_TEMP_PREFIX))
        TempPrefix = Str;
      else
        TempPrefix = get_process_name();
    }
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
            Entry2AsmFile->insert({ Entry, File });
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
    if (const auto *Str = getenv(ENV_FP_FLAGS)) {
      int Flags = std::stoi(Str);
      if (Flags < 0)
        fprintf(stderr, "invalid initial FP flags value %d ignored\n", Flags);
      else
        FPFlags = Flags;
    }
  });
  return DeviceInfo;
}

static
std::string find_gold_plugin() {

  // If the user defined an environment variable to point to the gold
  // plugin use it
  const char *gold_plugin_env = getenv(ENV_GOLD_PLUGIN);
  if (NULL != gold_plugin_env) {
    return gold_plugin_env;
  }

  std::string gold_plugin_path;

  // Get the LD_LIBRARY_PATH environment variable and make a copy I can muck
  // with.
  const char *ld_library_path = getenv("LD_LIBRARY_PATH");
  char *myPath = strdup(ld_library_path);
  bool found_gold_plugin = false;

  // Scan LD_LIBRARY_PATH looking for a directory that contains an executable
  // copy of LLVMgold.so
  for (const char *dir = strtok(myPath, ":"); dir; dir = strtok(NULL, ":")) {
    gold_plugin_path = dir;
    gold_plugin_path += "/LLVMgold.so";
    if (0 == access(gold_plugin_path.c_str(), X_OK)) {
      found_gold_plugin = true;
      break;
    }
  }

  // Free the allocated memory before I forget.
  free(myPath);

  // If we found a copy of LLVMgold.so return it. Otherwise return an empty
  // string
  if (found_gold_plugin) {
    return gold_plugin_path;
  } else {
    return "";
  }
}

static
std::string find_fixup_pass() {
#if 0
  // If the user defined an envirionment variable to point to the gold
  // plugin use it
  const char *gold_plugin_env = getenv(ENV_GOLD_PLUGIN);
  if (NULL != gold_plugin_env) {
    return gold_plugin_env;
  }
#endif

  std::string fixup_pass_path;

  // Get the LD_LIBRARY_PATH environment variable and make a copy I can muck
  // with.
  const char *ld_library_path = getenv("LD_LIBRARY_PATH");
  char *myPath = strdup(ld_library_path);
  bool found_fixup_pass = false;

  // Scan LD_LIBRARY_PATH looking for a directory that contains an executable
  // copy of LLVMCSAFixupOmpEntries.so
  for (const char *dir = strtok(myPath, ":"); dir; dir = strtok(NULL, ":")) {
    fixup_pass_path = dir;
    fixup_pass_path += "/LLVMCSAFixupOmpEntries.so";
    if (0 == access(fixup_pass_path.c_str(), X_OK)) {
      found_fixup_pass = true;
      break;
    }
  }

  // Free the allocated memory before I forget.
  free(myPath);

  // If we found a copy of LLVMgold.so return it. Otherwise return an empty
  // string
  if (found_fixup_pass) {
    return fixup_pass_path;
  } else {
    return "";
  }
}

static
bool is_ld_gold(const char *ldPath) {
  std::string command(ldPath);
  command += " --version";

  // Create the process to execute the command
  FILE *fCommandOut = popen(command.c_str(), "r");
  if (NULL == fCommandOut) {
    return false;
  }

  // Gather any output
  std::stringstream commandOutStream;
  char buff[1024];
  while (fgets(buff, sizeof(buff), fCommandOut) != NULL) {
    commandOutStream << buff;
  }

  pclose(fCommandOut);

  // If this is ld.gold, the version string will return something
  // like "GNU gold (GNU Binutils 2.27) 1.12"
  #define LD_GOLD_PREFIX "GNU gold"
  return (0 == strncmp (LD_GOLD_PREFIX, commandOutStream.str().c_str(), sizeof(LD_GOLD_PREFIX)-1));
}

static
std::string find_opt(const std::string &csa_clang) {
  // Find the root of the path for csa-clang
  std::string::size_type pos = csa_clang.find_last_of('/');
  if (std::string::npos == pos) {
    return "";
  }

  // Assume that we can find csa-opt in the same directory that
  // has csa-clang
  std::string opt = csa_clang.substr(0, pos);
  opt += "/csa-opt";

  // Trust, but verify
  if (0 == access(opt.c_str(), X_OK)) {
    return opt;
  } else {
    return "";
  }
}

static
std::string find_ld_gold() {

  // If the user defined an envirionment variable to point to ld.gold
  // or llvm-ld, use it
  const char *csa_ld_gold_env = getenv(ENV_LD_GOLD);
  if (NULL != csa_ld_gold_env) {
    return csa_ld_gold_env;
  }

  std::string csa_ld_gold_path;

  // Get the PATH environment variable and make a copy I can muck with.
  const char *path = getenv("PATH");
  char *myPath = strdup(path);
  bool found_csa_ld_gold = false;

  // Scan PATH looking for a directory that contains an executable
  // copy of csa-clang
  for (const char *dir = strtok(myPath, ":"); dir; dir = strtok(NULL, ":")) {
    csa_ld_gold_path = dir;
    csa_ld_gold_path += "/ld.gold";
    if (0 == access(csa_ld_gold_path.c_str(), X_OK)) {
      found_csa_ld_gold = true;
      break;
    }
    csa_ld_gold_path = dir;
    csa_ld_gold_path += "/ld";
    if (0 == access(csa_ld_gold_path.c_str(), X_OK)) {
      if (is_ld_gold(csa_ld_gold_path.c_str())) {
        found_csa_ld_gold = true;
        break;
      }
    }
  }

  // Free the allocated memory before I forget.
  free(myPath);

  // If we found a copy of ld.gold we can execute return it.
  // Otherwise return an empty string
  if (found_csa_ld_gold) {
    return csa_ld_gold_path;
  } else {
    return "";
  }
}

static
int execute_command(const char *command) {

  if (Verbosity) {
    fprintf(stderr, "Executing command: %s\n", command);
  }

  // Create the process to execute the command
  FILE *fCommandOut = popen(command, "r");
  if (NULL == fCommandOut) {
    DP(1, "Failed to create process to execute command\n");
    return 1;
  }

  // Gather any output
  std::stringstream commandOutStream;
  char buff[1024];
  while (fgets(buff, sizeof(buff), fCommandOut) != NULL) {
    commandOutStream << buff;
  }

  int exitCode = pclose(fCommandOut);
  if ((0 != exitCode) && Verbosity) {
    fprintf(stderr, "Command failed:\n");
      fprintf(stderr, "%s\n", commandOutStream.str().c_str());
  }

  return exitCode;
}

static
bool extract_bc_file(const char *tmp_name,
                     const std::string &bcFile,
                     const std::string &csa_clang,
                     const CSAElf::Section *bitcode_bounds,
                     const CSAElf::Section *bitcode_data) {

  // If there's only one file just extract it
  unsigned bitcodeCount = bitcode_bounds->getSize()/sizeof(BitcodeBounds);
  const char *data = bitcode_data->getBits();

  if (1 == bitcodeCount) {
    FILE *fBC = fopen(bcFile.c_str(), "wb");
    if (! fBC) {
      return false;
    }
    fwrite(data, bitcode_data->getSize(), 1, fBC);
    fclose(fBC);
    filesToDelete.push_back(bcFile);
    return true;
  }


  std::string llvmLinkCommand = "llvm-link ";
  llvmLinkCommand += " -o ";
  llvmLinkCommand += bcFile;

  filesToDelete.push_back(bcFile);

  // Save each of the the bitcode files to disk so we can concatenate them
  auto *bounds = (const BitcodeBounds *)bitcode_bounds->getBits();

  for (unsigned i = 0; i < bitcodeCount; i++) {
    std::string tmpBcFile(tmp_name);
    tmpBcFile += "_";
    char buf[16];
    sprintf(buf, "%u", i);
    tmpBcFile += buf;
    tmpBcFile += ".bc";

    uint64_t len = bounds[i].length();

    FILE *fBC = fopen(tmpBcFile.c_str(), "wb");
    if (! fBC) {
      return false;
    }
    fwrite(data, len, 1, fBC);
    fclose(fBC);

    filesToDelete.push_back(tmpBcFile);

    data += len;

    llvmLinkCommand += " ";
    llvmLinkCommand += tmpBcFile;
  }

  // Verify that we dealt with all of the bitcode data
  assert (data == (const char *)bitcode_data->getBits() + bitcode_data->getSize());

  // Now execute the llvm-link command to concatenate the bitcode files
  int result = execute_command(llvmLinkCommand.c_str());
  
  if (0 == result) {
    return true;
  }

  fprintf(stderr,
          "Failed to concatenate bitcode files extracted from the image\n");
  return false;
}

static
bool link_bc_file(const char *tmp_name,
                  std::string &bcFile,
                  const std::string &ldGold,
                  const std::string &goldPlugin,
                  const std::string &goldOptions) {

  // Link the bitcode file against one or more archives using the ld.gold
  // linker
  std::string linkResult(tmp_name);
  linkResult += "-linked.bc";

  // Start building the command string
  std::string linkCommand(ldGold);

  // Tell ld.gold to use the LLVM gold plugin since it understands how to
  // handle bitcode (.bc) files
  linkCommand += " --plugin ";
  linkCommand += goldPlugin;

  // The the LLVM gold plugin to emit a bitcode file. Options understood by
  // the gold plugin are listed in process_plugin_option() in
  // llvm/tools/gold/gold-plugin.cpp. Options that the plugin doesn't
  // "cherry pick" will be passed to llvm
  linkCommand += " -plugin-opt emit-llvm";

  // Specify the output file
  linkCommand += " -o ";
  linkCommand += linkResult;

  // Specify the input bitcode file
  linkCommand += " ";
  linkCommand += bcFile;

  // And the linker options given us by the environment variable. This is
  // expected to specify libraries, but can be any linker option
  linkCommand += " ";
  linkCommand += goldOptions;

  // Now execute the linker command
  int result = execute_command(linkCommand.c_str());
  if (0 != result) {
    fprintf(stderr, "Failed to create process to link Bitcode file\n");
    return false;
  }

  // Remove the original bitcode file and replace it with what we just
  // created
  bcFile = linkResult;
  filesToDelete.push_back(linkResult);

  return true;
}

static
bool fixup_bc_file(const char* tmp_name,
                   std::string& bcFile,
                   std::string& csa_clang) {

  // The gold plugin is making the .omp_offloading.entry globals into internal
  // symbols. We need to convert them back to global symbols so they're
  // not thrown away by clang as dead code. Use opt and a special pass
  // to fix this. opt should be in the same directory as csa-clang
  std::string optCommand = find_opt(csa_clang);
  if (optCommand.empty()) {
    fprintf(stderr, "Failed to find csa-opt.\n");
    return false;
  }

  std::string fixupPass = find_fixup_pass();
  if (fixupPass.empty()) {
    fprintf(stderr, "Failed to find LLVMCSAFixupOmpEntries.so.\n");
    return false;
  }

  optCommand += " -load=";
  optCommand += fixupPass;
  optCommand += " -csa-fixup-omp-entries ";
  optCommand += bcFile;
  optCommand += " -o ";

  bcFile = tmp_name;
  bcFile += "-fixed.bc";

  optCommand += bcFile;

  filesToDelete.push_back(bcFile);

  // Now execute the opt command to execute the pass that will fixup the
  // OpenMP entries so they won't get deleted by the dead code elimination
  // pass. It would be nice if we could get this into the compiler *really*
  // early, but since we can use opt to do it for us, it's not worth fighting
  // with llvm over.
  int result = execute_command(optCommand.c_str());
  if (0 != result) {
    fprintf(stderr,
            "Pass to fixup OpenMP entries failed\n");
    return false;
  }

  return true;
}

static
bool build_csa_assembly(const std::string &tmp_name,
                        const CSAElf::Section *bitcode_bounds,
                        const CSAElf::Section *bitcode_data,
                        std::string &csaAsmFile) {

  // If the user wants to save temporaries, name them after the process name
  std::string tmp_prefix;
  if (SaveTemps)
    tmp_prefix = TempPrefix;
  else
    tmp_prefix = tmp_name;

  // Find csa-clang. Do this before we create anything that might need
  // to be cleaned up
  std::string csa_clang = "icx -target csa ";
  if (csa_clang.empty()) {
    fprintf(stderr, "Failed to find icx.\n");
    return false;
  }

  // If the user gave us linker options, find a copy of ld.gold.
  // Note that the user may have a copy of ld.gold named ld.
  std::string ldGold, goldPlugin;
  const char *goldOptions = getenv(ENV_LINK_OPTIONS);
  if (goldOptions) {
    ldGold = find_ld_gold();
    if (ldGold.empty()) {
      fprintf(stderr, "Failed to find ld.gold.\n");
      return false;
    }

    goldPlugin = find_gold_plugin();
    if (goldPlugin.empty()) {
      fprintf(stderr, "Failed to find LLVMgold.so.\n");
      return false;
    }
  }

  std::string bcFile = tmp_prefix + ".bc";

  // Extract the bitcode from the image. If there are multiple bitcode
  // files in the image, use llvm-link to contcatenate them into a
  // single bitcode file
  if (! extract_bc_file(tmp_prefix.c_str(), bcFile, csa_clang,
                        bitcode_bounds, bitcode_data)) {
    return false;
  }

  // If the user gave us ld options, use ld.gold to build the final
  // .bc file
  if (! ldGold.empty()) {
    if (! link_bc_file(tmp_prefix.c_str(), bcFile, ldGold, goldPlugin, goldOptions)) {
      return false;
    }

    if (! fixup_bc_file(tmp_prefix.c_str(), bcFile, csa_clang)) {
      return false;
    }
  }

  // Generate a name for the target file
  csaAsmFile = tmp_prefix;
  csaAsmFile += ".s";

  // Build the csa-clang command to convert the bitcode to an CSA assembly file
  std::string clangCommand(csa_clang);
  clangCommand += " -S -o ";
  clangCommand += csaAsmFile;
  clangCommand += " ";

  // Add any compilation options
  const char *options = getenv(ENV_COMPILE_OPTIONS);
  if (NULL != options) {
    clangCommand += options;
    clangCommand += " ";
  }

  // Finish off the command with the input file name
  clangCommand += bcFile;

  // Execute the clang command
  if (0 == execute_command(clangCommand.c_str())) {
    return true;
  }
  else {
    return false;
  }
}

// Plugin API implementation.

int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image) {
  const char *Start = static_cast<char*>(Image->ImageStart);
  size_t Size = static_cast<char*>(Image->ImageEnd) - Start;

  CSAElf Elf;
  if (!Elf.readFromMemory(Start, Size)) {
    DP(1, "Unable to read ELF!\n");
    return false;
  }

  // So far CSA binary is indistinguishable from x86_64 by looking at ELF
  // machine only. We can slightly enhance this test by checking if given
  // binary contains CSA code section.
  const auto *CSACodeSec = Elf.findSection(CSA_CODE_SECTION);
  const auto *BCBoundsSec = Elf.findSection(CSA_BITCODE_BOUNDS_SECTION);
  const auto *BCDataSec = Elf.findSection(CSA_BITCODE_DATA_SECTION);
  if (!CSACodeSec && !(BCBoundsSec && BCDataSec)) {
    DP(1, "No CSA code or bitcode sections in the binary\n");
    return false;
  }

  return true;
}

int32_t __tgt_rtl_number_of_devices() {
  return getDeviceInfo().getNumDevices();
}

int32_t __tgt_rtl_init_device(int32_t ID) {
  return OFFLOAD_SUCCESS;
}

__tgt_target_table *__tgt_rtl_load_binary(int32_t ID, __tgt_device_image *Ptr) {
  return getDeviceInfo().loadImage(Ptr);
}

void *__tgt_rtl_data_alloc(int32_t ID, int64_t Size, void *HPtr) {
  if (HPtr)
    return HPtr;
  return getDeviceInfo().getDevice(ID).alloc(Size);
}

int32_t __tgt_rtl_data_submit(int32_t ID, void *TPtr, void *HPtr, int64_t Size) {
  if (TPtr != HPtr)
    memcpy(TPtr, HPtr, Size);
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_retrieve(int32_t ID, void *HPtr, void *TPtr, int64_t Size) {
  if (HPtr != TPtr)
    memcpy(HPtr, TPtr, Size);
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_delete(int32_t ID, void *TPtr) {
  getDeviceInfo().getDevice(ID).free(TPtr);
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_team_region(int32_t ID, void *Entry,
    void **Bases, ptrdiff_t *Offsets, int32_t NumArgs, int32_t TeamNum,
    int32_t ThreadLimit, uint64_t LoopTripCount) {
  std::vector<void*> Args(NumArgs);
  for (int32_t I = 0; I < NumArgs; ++I)
    Args[I] = static_cast<char*>(Bases[I]) + Offsets[I];

  if (!getDeviceInfo().getDevice(ID).runFunction(Entry, Args))
    return OFFLOAD_FAIL;
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_region(int32_t device_id, void *tgt_entry_ptr,
                                    void **tgt_args, ptrdiff_t *tgt_offsets, int32_t arg_num) {
  // use one team and one thread.
  return __tgt_rtl_run_target_team_region(device_id, tgt_entry_ptr, tgt_args,
                                          tgt_offsets, arg_num, 1, 1, 0);
}
