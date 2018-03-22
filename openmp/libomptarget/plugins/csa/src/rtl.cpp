//===-RTLs/generic-64bit/src/rtl.cpp - Target RTLs Implementation - C++ -*-===//
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
#include <assert.h>
#include <cstdio>
#include <dlfcn.h>
#include <elf.h>
//#include <ffi.h>
#include <gelf.h>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <link.h>
#include <unistd.h>
#include <map>
#include <iomanip>
#include <cmath>
#include <pthread.h>
#include <mutex>

#include "omptargetplugin.h"
#include "csa_invoke.h"

#ifndef TARGET_NAME
#define TARGET_NAME csa
#endif

// Using x86 ID for now
#ifndef TARGET_ELF_ID
#define TARGET_ELF_ID 62
#endif

#define GETNAME2(name) #name
#define GETNAME(name) GETNAME2(name)

#ifdef OMPTARGET_DEBUG
static int DebugLevel = 0;

#define DP(...)                                                            \
  do {                                                                     \
    if (DebugLevel > 0) {                                                  \
      fprintf(stderr, "CSA  (HOST)  --> ");                                \
      fprintf(stderr, __VA_ARGS__);                                        \
      fflush(nullptr);                                                     \
    }                                                                      \
  } while (false)
#else
#define DP(...)                                                            \
  {}
#endif

#define NUMBER_OF_DEVICES 4
#define OFFLOADSECTIONNAME ".omp_offloading.entries"
#define CSA_BITCODE_BOUNDS_SECTION ".csa.bc.bounds"
#define CSA_BITCODE_DATA_SECTION ".csa.bc.data"

// ENVIRONMENT VARIABLES

// If defined, suppresses the generation of assembly from the Bitcode, and
// specifies the file to use instead
#define ENV_ASSEMBLY_FILE "CSA_ASSEMBLY_FILE"

// If defined, points to the copy of csa-clang to used. If not defined,
// the tool will attempt to find csa-clang on the user's PATH
#define ENV_CLANG "CSA_CLANG"

// Specifies options to add to the csa-clang command. The command will be
// csa-clang -S -o <tempfile>.s <csa-compile-options> <tempfile>.bc
#define ENV_COMPILE_OPTIONS "CSA_COMPILE_OPTIONS"

// Specifies whether the application should be aborted if an error occurs.
// If not, then OpenMP will automagically run the native code
#define ENV_EXIT_ON_ERROR "CSA_EXIT_ON_ERROR"

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

// If defined, dumps the simulator statistics after each offloaded procedure
// is run
#define ENV_DUMP_STATS "CSA_DUMP_STATS"

// If defined all stats for a thread are run in a single CSA instance and
// dumped in a single .stat file (if CSA_DUMP_STATS is defined)
#define ENV_MERGE_STATS "CSA_MERGE_STATS"

// If defined, leave the temporary files on disk in the user's directory
#define ENV_SAVE_TEMPS "CSA_SAVE_TEMPS"

// If defined, specifies temporary file prefix. If not defined, defaults
// to process name with "-csa" appended. No effect if CSA_SAVE_TEMPS is
// not defined
#define ENV_TEMP_PREFIX "CSA_TEMP_PREFIX"

// If defined specifies the value to initialize the floating point flags
// to before an offloaded function, and that the floating point flags be
// reported after the offloaded funciton returns
#define ENV_FP_FLAGS "CSA_FP_FLAGS"

// Environment variable the FP flags will be saved in
#define ENV_RUN_FP_FLAGS "CSA_RUN_FP_FLAGS"

// Bitcode bounds struct built by the compiler. WARNING! This struct MUST
// match the struct written to the .csa.bc.bounds section in CSAAsmPrinter.cpp!
struct BitcodeBounds {
  uint64_t start;
  uint64_t end;

  uint64_t length() {
    return end - start;
  }
};

/// Array of Dynamic libraries loaded for this target.
struct DynLibTy {
  std::string FileName;
  void *Handle;
};

static std::list<std::string> filesToDelete;
static bool SaveTemps = false;

/// Account the memory allocated per device.
struct AllocMemEntryTy {
  int64_t TotalSize;
  std::vector<std::pair<void *, int64_t>> Ptrs;

  AllocMemEntryTy() : TotalSize(0) {}
};

// Map for tracking explicitly allocated memory which
// we'll need to copy or release
class AllocatedMemoryMap: public std::map<void *, bool> {
};

struct CsaEntryInfo {
  csa_processor *processor;
  csa_module *module;
  csa_entry *entry;
  const char *entry_name;
};

typedef class std::map<std::pair<pthread_t,const void*>, CsaEntryInfo> CsaEntryMap_t;

/// Keep entries table per device.
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::string csaAsmFile;
  CsaEntryMap_t csaTidToEntryMap;
  AllocatedMemoryMap ExplicitlyAllocatedMemory;
};

static std::string tmp_prefix;


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


/// Class containing all the device information.
class RTLDeviceInfoTy {
  std::vector<FuncOrGblEntryTy> FuncGblEntries;
  std::mutex FuncGblEntriesMutex;

public:
  std::list<DynLibTy> DynLibs;

  // Record entry point associated with device.
  void createOffloadTable(int32_t device_id, __tgt_offload_entry *begin,
                          __tgt_offload_entry *end, std::string csaAsmFile) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    E.Table.EntriesBegin = begin;
    E.Table.EntriesEnd = end;
    E.csaAsmFile = csaAsmFile;
  }

  // Return true if the entry is associated with device.
  const char *findOffloadEntry(int32_t device_id, const void *addr,
                               csa_processor **proc, csa_module **mod, csa_entry **entry) {

    // Clean out the pointers
    *proc = NULL;
    *mod = NULL;
    *entry = NULL;

    // Take out the lock. Automatically released when "lock" goes out of scope
    std::lock_guard<std::mutex> lock(FuncGblEntriesMutex);

    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    for (__tgt_offload_entry *i = E.Table.EntriesBegin, *e = E.Table.EntriesEnd; i < e; ++i) {
      if (i->addr == addr) {
        pthread_t tid = pthread_self();
        CsaEntryMap_t::iterator i = E.csaTidToEntryMap.find(std::pair<pthread_t,const void*>(tid, addr));
        if (E.csaTidToEntryMap.end() != i) {
          *proc = i->second.processor;
          *mod = i->second.module;
          *entry = i->second.entry;
        }
        return E.csaAsmFile.c_str();
      }
    }

    return NULL;
  }

  bool setCsaOffloadEntryInfo(int32_t device_id, void *addr,csa_processor *proc, csa_module *mod, csa_entry *entry) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");

    // Take out the lock. Automatically released when "lock" goes out of scope
    std::lock_guard<std::mutex> lock(FuncGblEntriesMutex);

    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    CsaEntryInfo mapEntry;
    mapEntry.processor = proc;
    mapEntry.module = mod;
    mapEntry.entry = entry;
    mapEntry.entry_name = (const char*)addr;

    for (__tgt_offload_entry *i = E.Table.EntriesBegin, *e = E.Table.EntriesEnd;
         i < e; ++i) {
      if (i->addr == addr) {
        pthread_t tid = pthread_self();
        
        E.csaTidToEntryMap[std::pair<pthread_t,const void*>(tid, addr)] = mapEntry;
        return true;
      }
    }
    return false;
  }

  // Return the pointer to the target entries table.
  __tgt_target_table *getOffloadEntriesTable(int32_t device_id) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    return &E.Table;
  }

  void RecordMemoryAlloc(int32_t device_id, void *addr) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];
    E.ExplicitlyAllocatedMemory[addr] = true;
  }

  bool IsExplicitlyAllocatedMemory(int32_t device_id, void *addr) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];
    return (E.ExplicitlyAllocatedMemory.end() !=
            E.ExplicitlyAllocatedMemory.find(addr));
  }

  void RecordMemoryFree(int32_t device_id, void *addr) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];
    AllocatedMemoryMap::iterator iter = E.ExplicitlyAllocatedMemory.find(addr);
    assert(E.ExplicitlyAllocatedMemory.end() != iter);
    E.ExplicitlyAllocatedMemory.erase(iter);
  }

  RTLDeviceInfoTy(int32_t num_devices) {
#ifdef OMPTARGET_DEBUG
    if (char *envStr = getenv("LIBOMPTARGET_DEBUG")) {
      DebugLevel = std::stoi(envStr);
    }
#endif // OMPTARGET_DEBUG
    if (auto *Str = getenv(ENV_SAVE_TEMPS)) {
      SaveTemps = std::atoi(Str);
    }
    FuncGblEntries.resize(num_devices);
  }

  ~RTLDeviceInfoTy() {
    // Close dynamic libraries
    for (const auto &Lib : DynLibs) {
      if (Lib.Handle)
        dlclose(Lib.Handle);

      // Cleanup CSA files
      if (!SaveTemps) {
        remove(Lib.FileName.c_str());

        //std::string bcFile(Lib.FileName);  bcFile += ".bc";
        //remove(bcFile.c_str());

        std::string asmFile(Lib.FileName); asmFile += ".s";
        remove(asmFile.c_str());
      }
    }

    // Are we dumping the stats files?
    bool dumpStats = (NULL != getenv(ENV_DUMP_STATS));

    // Build a map of thread IDs to simple numbers
    typedef std::map<pthread_t, int> ThreadNumMap_t;
    ThreadNumMap_t threadNumMap;
    int threadNum;
    int width = 0;

    if (dumpStats) {
      for (size_t i = 0; i < FuncGblEntries.size(); i++) {
        FuncOrGblEntryTy &E = FuncGblEntries[i];
        for (CsaEntryMap_t::iterator i = E.csaTidToEntryMap.begin(); i != E.csaTidToEntryMap.end(); ++i) {
          std::pair<pthread_t, const void*>key = i->first;
          
          ThreadNumMap_t::iterator iter = threadNumMap.find(key.first);
          if (threadNumMap.end() == iter) {
            threadNumMap[key.first] = threadNum++;
          }
        }
      }
      width = ceil(log10(threadNum));
    }

    // Finish up - Dump the stats, release the CSA isntances
    for (size_t i = 0; i < FuncGblEntries.size(); i++) {
      FuncOrGblEntryTy &E = FuncGblEntries[i];
      for (CsaEntryMap_t::iterator i = E.csaTidToEntryMap.begin(); i != E.csaTidToEntryMap.end(); ++i) {
        if (dumpStats) {
          std::pair<pthread_t, const void*>key = i->first;
          std::string entry_name = i->second.entry_name;

          // Get the thread number that maps to this thread ID
          int num = threadNumMap[key.first];

          std::stringstream ss;
          ss << get_process_name();

          // Do we need to strip "__omp_offloading" from the entryoint name?
          std::string omp_prefix("__omp_offloading");
          if (0 == entry_name.compare(0, omp_prefix.length(), omp_prefix)) {
            // If the entrypoint starts with "__omp_offloading"
            ss << entry_name.substr(omp_prefix.length());
          } else {
            // Just use the process name
            ss << entry_name;
          }

          // Add the thread number
          ss << "-thd" << std::setfill('0') << std::setw(width) << num;

          csa_dump_statistics(i->second.processor, ss.str().c_str());
        }

        // Release csa_processor
        csa_free(i->second.processor);
      }
    }
  }
};
static RTLDeviceInfoTy DeviceInfo(NUMBER_OF_DEVICES);

// How noisy we should be
static int32_t verbosity = 0;
static int32_t initial_fp_flags = -1;


#ifdef __cplusplus
extern "C" {
#endif

int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *image) {
// If we don't have a valid ELF ID we can just fail.
#if TARGET_ELF_ID < 1
  return 0;
#else
  // Is the library version incompatible with the header file?
  if (elf_version(EV_CURRENT) == EV_NONE) {
    DP("Incompatible ELF library!\n");
    return 0;
  }

  char *img_begin = (char *)image->ImageStart;
  char *img_end = (char *)image->ImageEnd;
  size_t img_size = img_end - img_begin;

  // Obtain elf handler
  Elf *e = elf_memory(img_begin, img_size);
  if (!e) {
    DP("Unable to get ELF handle: %s!\n", elf_errmsg(-1));
    return 0;
  }

  // Utility object for closing Elf on return.
  struct ElfEnd {
    ElfEnd(Elf *E) : E(E) {}
    ~ElfEnd() {
     elf_end(E);
  }
  private:
    Elf *E;
  } ElfEndCaller(e);

  // Check if ELF is the right kind
  if (elf_kind(e) != ELF_K_ELF) {
    DP("Unexpected ELF type!\n");
    return 0;
  }
  Elf64_Ehdr *eh64 = elf64_getehdr(e);
  Elf32_Ehdr *eh32 = elf32_getehdr(e);

  if (!eh64 && !eh32) {
    DP("Unable to get machine ID from ELF file!\n");
    return 0;
  }

  uint16_t MachineID;
  if (eh64 && !eh32)
    MachineID = eh64->e_machine;
  else if (eh32 && !eh64)
    MachineID = eh32->e_machine;
  else {
    DP("Ambiguous ELF header!\n");
    return 0;
  }

  if (MachineID != TARGET_ELF_ID) {
    DP("Unexpected ELF machine\n");
    return 0;
  }

  // So far CSA binary is indistinguishable from x86_64 by looking at ELF
  // machine only. We can slightly enhance this test by checking if given
  // binary contains CSA bitcode section.
  {
    size_t ShStrIdx = 0;
    if (elf_getshdrstrndx(e, &ShStrIdx)) {
      DP("Unable to get ELF strings index!\n");
      return 0;
    }
    Elf_Scn *Sec = nullptr;
    while ((Sec = elf_nextscn(e, Sec))) {
      GElf_Shdr Shdr;
      gelf_getshdr(Sec, &Shdr);

      if (const auto *Name = elf_strptr(e, ShStrIdx, Shdr.sh_name))
        if (strcmp(Name, CSA_BITCODE_DATA_SECTION) == 0)
          return 1;
    }
  }

  DP("No CSA bitcode data section\n");
  return 0;
#endif
}

int32_t __tgt_rtl_number_of_devices() { return NUMBER_OF_DEVICES; }

int32_t __tgt_rtl_init_device(int32_t device_id) { return OFFLOAD_SUCCESS; }

static
void deleteTempFiles() {
  // If we've been asked to save the temporaries, don't delete them
  if (getenv(ENV_SAVE_TEMPS)) {
    return;
  }

  // Delete any temporary files we've created
  while (! filesToDelete.empty()) {
    std::string& tmpFile = filesToDelete.front();
    remove (tmpFile.c_str());
    filesToDelete.pop_front();
  }
}

// The compiler is whining about functions declared but not used
#if 0
static
void dumpTempFiles() {
  fprintf(stderr, "%zd files in list:\n", filesToDelete.size());
  for (std::string file: filesToDelete) {
    fprintf (stderr, "   %s\n", file.c_str());
  }
}
#endif

static
int32_t checkForExitOnError(int32_t error, const char* errorText) {
  if (NULL == getenv(ENV_EXIT_ON_ERROR)) {
    return error;
  }
  if (errorText) {
    fprintf(stderr, "%s\n", errorText);
  }
  exit(error);
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

  if (verbosity) {
    fprintf(stderr, "Executing command: %s\n", command);
  }

  // Create the process to execute the command
  FILE *fCommandOut = popen(command, "r");
  if (NULL == fCommandOut) {
    DP("Failed to create process to execute command\n");
    return 1;
  }

  // Gather any output
  std::stringstream commandOutStream;
  char buff[1024];
  while (fgets(buff, sizeof(buff), fCommandOut) != NULL) {
    commandOutStream << buff;
  }

  int exitCode = pclose(fCommandOut);
  if ((0 != exitCode) && verbosity) {
    fprintf(stderr, "Command failed:\n");
      fprintf(stderr, "%s\n", commandOutStream.str().c_str());
  }

  return exitCode;
}

static
bool extract_bc_file(const char *tmp_name,
                     const std::string &bcFile,
                     const std::string &csa_clang,
                     const Elf_Data *bitcode_bounds,
                     const Elf_Data *bitcode_data) {

  // If there's only one file just extract it
  unsigned bitcodeCount = bitcode_bounds->d_size/sizeof(BitcodeBounds);
  const char *data = (const char *)bitcode_data->d_buf;

  if (1 == bitcodeCount) {
    FILE *fBC = fopen(bcFile.c_str(), "wb");
    if (! fBC) {
      return false;
    }
    fwrite(data, bitcode_data->d_size, 1, fBC);
    fclose(fBC);
    filesToDelete.push_back(bcFile);
    return true;
  }


  std::string llvmLinkCommand = "llvm-link ";
  llvmLinkCommand += " -o ";
  llvmLinkCommand += bcFile;

  filesToDelete.push_back(bcFile);

  // Save each of the the bitcode files to disk so we can concatenate them
  BitcodeBounds *bounds = (BitcodeBounds *)bitcode_bounds->d_buf;

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
  assert (data == (const char *)bitcode_data->d_buf + bitcode_data->d_size);

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
bool build_csa_assembly(const char *tmp_name,
                        const Elf_Data *bitcode_bounds,
                        const Elf_Data *bitcode_data,
                        std::string &csaAsmFile) {

  // If the user wants to save temporaries, name them after the process name
  //
  // Note that tmp_prefix is a global variable. We'll use it for naming
  // the stats file when we run
  if (getenv(ENV_SAVE_TEMPS)) {
    const char* prefix = getenv(ENV_TEMP_PREFIX);
    if (prefix) {
      tmp_prefix = prefix;
    } else {
      tmp_prefix = get_process_name();
    }
  } else {
    tmp_prefix = tmp_name;
  }

  // If the user is using his own assembly file, don't bother. Do not fill
  // in csaAsmFile since we don't want to delete it when the shared object
  // unloads.
  if (getenv(ENV_ASSEMBLY_FILE)) {
    return true;
  }

  // Find csa-clang. Do this before we create anything that might need
  // to be cleaned up
  std::string csa_clang = "icx -target csa ";
  if (csa_clang.empty()) {
    fprintf(stderr, "Failed to find csa-clang.\n");
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

__tgt_target_table *__tgt_rtl_load_binary(int32_t device_id,
                                          __tgt_device_image *image) {

  const char *verbose_env = getenv(ENV_VERBOSE);
  if (NULL != verbose_env) {
    verbosity = 1;
  }

  DP("Dev %d: load binary from 0x%llx image\n", device_id,
     (long long)image->ImageStart);

  assert(device_id >= 0 && device_id < NUMBER_OF_DEVICES && "bad dev id");

  size_t ImageSize = (size_t)image->ImageEnd - (size_t)image->ImageStart;
  size_t NumEntries = (size_t)(image->EntriesEnd - image->EntriesBegin);
  DP("Expecting to have %ld entries defined.\n", (long)NumEntries);

  // Is the library version incompatible with the header file?
  if (elf_version(EV_CURRENT) == EV_NONE) {
    checkForExitOnError(1, "Incompatible ELF library!");
    return NULL;
  }

  // Obtain elf handler
  Elf *e = elf_memory((char *)image->ImageStart, ImageSize);
  if (!e) {
    DP("Unable to get ELF handle: %s!\n", elf_errmsg(-1));
    checkForExitOnError(1, NULL);
    return NULL;
  }

  if (elf_kind(e) != ELF_K_ELF) {
    elf_end(e);
    checkForExitOnError(1, "Invalid Elf kind!");
    return NULL;
  }

  // Find the entries section offset
  Elf_Scn *section = 0;
  Elf_Data *bitcode_bounds = 0;
  Elf_Data *bitcode_data = 0;
  Elf64_Off entries_offset = 0;


  size_t shstrndx;

  if (elf_getshdrstrndx(e, &shstrndx)) {
    elf_end(e);
    checkForExitOnError(1, "Unable to get ELF strings index!");
    return NULL;
  }

  while ((section = elf_nextscn(e, section))) {
    GElf_Shdr hdr;
    gelf_getshdr(section, &hdr);

    const char *sectionName = elf_strptr(e, shstrndx, hdr.sh_name);

    if (!strcmp(sectionName, OFFLOADSECTIONNAME)) {
      entries_offset = hdr.sh_addr;
      DP("Offset of entries section is (%016lx).\n", entries_offset);
    }

    if (!strcmp(sectionName, CSA_BITCODE_BOUNDS_SECTION)) {
      DP("Offset of bitcode bounds section is (%016lx).\n", hdr.sh_addr);
      bitcode_bounds = elf_getdata(section, NULL);
    }

    if (!strcmp(sectionName, CSA_BITCODE_DATA_SECTION)) {
      DP("Offset of bitcode data section is (%016lx).\n", hdr.sh_addr);
      bitcode_data = elf_getdata(section, NULL);
    }
  }

  if (!entries_offset) {
    DP("Entries Section Offset Not Found\n");
    elf_end(e);
    checkForExitOnError(1, NULL);
    return NULL;
  }

  if (!bitcode_bounds) {
    elf_end(e);
    checkForExitOnError(1, "CSA bitcode bounds section Not Found");
    return NULL;
  }

  if (!bitcode_data) {
    elf_end(e);
    checkForExitOnError(1, "CSA bitcode data section Not Found");
    return NULL;
  }

  // load dynamic library and get the entry points. We use the dl library
  // to do the loading of the library, but we could do it directly to avoid the
  // dump to the temporary file.
  //
  // 1) Create tmp file with the library contents.
  // 2) Use dlopen to load the file and dlsym to retrieve the symbols.
  char tmp_name[] = "/tmp/tmpfile_XXXXXX";
  int tmp_fd = mkstemp(tmp_name);

  if (tmp_fd == -1) {
    elf_end(e);
    checkForExitOnError(1, NULL);
    return NULL;
  }

  FILE *ftmp = fdopen(tmp_fd, "wb");

  if (!ftmp) {
    elf_end(e);
    checkForExitOnError(1, NULL);
    return NULL;
  }

  fwrite(image->ImageStart, ImageSize, 1, ftmp);
  fclose(ftmp);

  DynLibTy Lib = {tmp_name, dlopen(tmp_name, RTLD_LAZY)};

  if (!Lib.Handle) {
    std::string err("target library loading error: ");
    err += dlerror();
    elf_end(e);
    checkForExitOnError(1, err.c_str());
    return NULL;
  }

  DeviceInfo.DynLibs.push_back(Lib);

  // Build the CSA assembly file
  std::string csaAsmFile;
  if (! build_csa_assembly(tmp_name, bitcode_bounds, bitcode_data, csaAsmFile)) {
    elf_end(e);
    deleteTempFiles();
    remove(tmp_name);
    checkForExitOnError(1, NULL);
    return NULL;
  }

  // Delete the temp files create to build the .s file
  deleteTempFiles();

  struct link_map *libInfo = (struct link_map *)Lib.Handle;

  // The place where the entries info is loaded is the library base address
  // plus the offset determined from the ELF file.
  Elf64_Addr entries_addr = libInfo->l_addr + entries_offset;

  DP("Pointer to first entry to be loaded is (%016lx).\n", entries_addr);

  // Table of pointers to all the entries in the target.
  __tgt_offload_entry *entries_table = (__tgt_offload_entry *)entries_addr;

  __tgt_offload_entry *entries_begin = &entries_table[0];
  __tgt_offload_entry *entries_end = entries_begin + NumEntries;

  if (!entries_begin) {
    elf_end(e);
    checkForExitOnError(1, "Can't obtain entries begin");
    return NULL;
  }

  DP("Entries table range is (%016lx)->(%016lx)\n", (Elf64_Addr)entries_begin,
     (Elf64_Addr)entries_end);
  DeviceInfo.createOffloadTable(device_id, entries_begin, entries_end, csaAsmFile);

  elf_end(e);

  return DeviceInfo.getOffloadEntriesTable(device_id);
}

void *__tgt_rtl_data_alloc(int32_t device_id, int64_t size, void *hst_ptr) {
  if (hst_ptr)
    return hst_ptr;

  if (verbosity)
    fprintf(stderr, "Note: allocating %ld bytes for this region.\n", size);

  void *ptr = malloc(size);
  DeviceInfo.RecordMemoryAlloc(device_id, ptr);
  return ptr;
}

int32_t __tgt_rtl_data_submit(int32_t device_id, void *tgt_ptr, void *hst_ptr,
                              int64_t size) {
  if (tgt_ptr != hst_ptr) {
    memcpy(tgt_ptr, hst_ptr, size);
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_retrieve(int32_t device_id, void *hst_ptr, void *tgt_ptr,
                                int64_t size) {
  if (hst_ptr != tgt_ptr) {
    memcpy(hst_ptr, tgt_ptr, size);
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_delete(int32_t device_id, void *tgt_ptr) {
  if (DeviceInfo.IsExplicitlyAllocatedMemory(device_id, tgt_ptr)) {
    free(tgt_ptr);
    DeviceInfo.RecordMemoryFree(device_id, tgt_ptr);
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_team_region(int32_t device_id, void *tgt_entry_ptr,
    void **tgt_args, ptrdiff_t *tgt_offsets, int32_t arg_num, int32_t team_num,
    int32_t thread_limit, uint64_t loop_tripcount /*not used*/) {

  typedef std::pair<csa_processor*, csa_module*> InstanceInfo_t;
  typedef std::map<pthread_t, InstanceInfo_t> MapThreadToProcessor_t;

  static MapThreadToProcessor_t mapThreadToProcessor;
  static std::mutex s_threadToProcessorMapMutex;

  // ignore team num and thread limit.
  int32_t status = OFFLOAD_SUCCESS;
  static uint32_t run_count = 0;

  // If not checked yet, see if the CSA_FL_FLAGS environment variable is defined
  if (-1 == initial_fp_flags) {
    const char* fpFlags = getenv(ENV_FP_FLAGS);
    if (NULL == fpFlags) {
      initial_fp_flags = -2;
    } else {
      initial_fp_flags = atoi(fpFlags);
      if (initial_fp_flags < 0) {
        fprintf(stderr, "invalid initial FP flags value %d ignored\n", initial_fp_flags);
        initial_fp_flags = -2;
      }
    }
  }

  csa_processor* proc = NULL;
  csa_module* mod = NULL;
  csa_entry* entry = NULL;

  // Normally this would be the code, but since we're loading assembly into
  // the simulator, the compilation gave us the address of the function name.
  const char *functionName = (const char *)tgt_entry_ptr;

  // If we've got an environment variable specifying the assembly file, 
  // use it's value
  const char *assemblyFile = getenv(ENV_ASSEMBLY_FILE);
  if (NULL == assemblyFile) {
    // Find the function in the target table, which will give us the path for
    // the compiled code so we can load it into the assembler
    assemblyFile = DeviceInfo.findOffloadEntry(device_id, tgt_entry_ptr, &proc, &mod, &entry);
  }

  if (NULL == assemblyFile) {
    return checkForExitOnError(OFFLOAD_FAIL, "Failed to find assembly file");
  }

  // If we've already got a CSA instance, module & entry, use them. Otherwise
  // create them now.
  if (NULL == proc) {
    bool mergeStats = (NULL != getenv(ENV_MERGE_STATS));
    bool newInstance = false;
    pthread_t tid = pthread_self();

    if (mergeStats) {
      {
        std::lock_guard<std::mutex> lock(s_threadToProcessorMapMutex);

        // Do we already have an instance for this thread?
        MapThreadToProcessor_t::iterator iter = mapThreadToProcessor.find(tid);
        if (mapThreadToProcessor.end() != iter) {
          InstanceInfo_t instance = iter->second;
          proc = instance.first;
          mod = instance.second;
          entry = csa_lookup(mod, functionName);
        }
      }
    }

    // If we still didn't find it, create a new instance
    if (NULL == entry) {
      // Allocate an CSA
      proc = csa_alloc("autounit");
      if (NULL == proc) {
        return checkForExitOnError(OFFLOAD_FAIL,
                                   "Failed to allocate CSA processor");
      }
      newInstance = true;

      mod = csa_assemble(assemblyFile, proc);
      if (NULL == mod) {
        return checkForExitOnError(OFFLOAD_FAIL, "Failed to load module");
      }

      entry = csa_lookup(mod, functionName);

      if (mergeStats) {
        std::lock_guard<std::mutex> lock(s_threadToProcessorMapMutex);

        mapThreadToProcessor[tid] = InstanceInfo_t(proc,mod);
      }
    }

    if (newInstance) {
      bool success = DeviceInfo.setCsaOffloadEntryInfo(device_id, tgt_entry_ptr,
                                                       proc, mod, entry);
      assert(success && "saving CSA offload entry information failed!");
    }
  }

#ifdef RAVI  //need new csasim header which defines csa_set_fp_flags
  if (initial_fp_flags >= 0) {
    csa_set_fp_flags(proc, (unsigned int)initial_fp_flags);
  }
  if (verbosity) {
    fprintf(stderr, "\nRun %u: Running %s on the CSA simulator..\n",
            run_count, functionName);
  }
#endif

  std::vector<void *> ptrs(arg_num);
  for (int32_t i=0; i<arg_num; ++i) {
    ptrs[i] = (void*)((intptr_t)tgt_args[i] + tgt_offsets[i]);
  }

  // Execute the CSA code. target() is adding an "omp handle" to the
  // arguments array which we don't care about. So ignore it
  assert(sizeof(csa_arg) == sizeof(ptrs[0]));
  unsigned long long start = csa_cycle_counter(proc);
  csa_call(entry, arg_num, (csa_arg *)&ptrs[0]);
  unsigned long long cycles = csa_cycle_counter(proc) - start;

#if 0
  // Dump the statistics, if requested
  if (getenv (ENV_DUMP_STATS)) {
    std::stringstream ss;
    ss << tmp_prefix << "-" << run_count;
    csa_dump_statistics(proc, ss.str().c_str());
  }
#endif

  if (verbosity) {
    fprintf(stderr, "\nRun %u: %s ran on the CSA simulator in %llu cycles\n\n",
            run_count, functionName, cycles);
  }
#ifdef RAVI  //need new csasim header which defines csa_get_fp_flags
  if (initial_fp_flags >= 0) {
    unsigned int flags = csa_get_fp_flags(proc);
    fprintf(stderr, "FP Flags at completion of %s : 0x%02x\n",
            functionName, flags);
    char buf[32];
    sprintf(buf, "%u", flags);
    setenv(ENV_RUN_FP_FLAGS, buf, 1);
  }
#endif

  run_count++;

  return status;
}

int32_t __tgt_rtl_run_target_region(int32_t device_id, void *tgt_entry_ptr,
                                    void **tgt_args, ptrdiff_t *tgt_offsets, int32_t arg_num) {
  // use one team and one thread.
  return __tgt_rtl_run_target_team_region(device_id, tgt_entry_ptr, tgt_args,
                                          tgt_offsets, arg_num, 1, 1, 0);
}

#ifdef __cplusplus
}
#endif
