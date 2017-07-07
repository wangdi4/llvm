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
#define DP(...) DEBUGP("Target " GETNAME(TARGET_NAME) " RTL", __VA_ARGS__)

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
  char *FileName;
  void *Handle;
};

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

/// Keep entries table per device.
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::string csaAsmFile;
  AllocatedMemoryMap ExplicitlyAllocatedMemory;
};

/// Class containing all the device information.
class RTLDeviceInfoTy {
  std::vector<FuncOrGblEntryTy> FuncGblEntries;

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
  const char *findOffloadEntry(int32_t device_id, void *addr) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    for (__tgt_offload_entry *i = E.Table.EntriesBegin, *e = E.Table.EntriesEnd;
         i < e; ++i) {
      if (i->addr == addr)
        return E.csaAsmFile.c_str();
    }

    return NULL;
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
  
  RTLDeviceInfoTy(int32_t num_devices) { FuncGblEntries.resize(num_devices); }

  ~RTLDeviceInfoTy() {
    // Close dynamic libraries
    for (std::list<DynLibTy>::iterator ii = DynLibs.begin(),
                                       ie = DynLibs.begin();
         ii != ie; ++ii)
      if (ii->Handle) {
        dlclose(ii->Handle);
        remove(ii->FileName);

        // Cleanup CSA files
        std::string bcFile(ii->FileName);  bcFile += ".bc";
        remove(bcFile.c_str());

        std::string asmFile(ii->FileName); asmFile += ".s";
        remove(asmFile.c_str());
      }
  }
};

static RTLDeviceInfoTy DeviceInfo(NUMBER_OF_DEVICES);

// How noisy we should be
static int32_t verbosity = 0;

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

  // Check if ELF is the right kind
  if (elf_kind(e) != ELF_K_ELF) {
    DP("Unexpected ELF type!\n");
    return 0;
  }
  Elf64_Ehdr *eh64 = elf64_getehdr(e);
  Elf32_Ehdr *eh32 = elf32_getehdr(e);

  if (!eh64 && !eh32) {
    DP("Unable to get machine ID from ELF file!\n");
    elf_end(e);
    return 0;
  }

  uint16_t MachineID;
  if (eh64 && !eh32)
    MachineID = eh64->e_machine;
  else if (eh32 && !eh64)
    MachineID = eh32->e_machine;
  else {
    DP("Ambiguous ELF header!\n");
    elf_end(e);
    return 0;
  }
  elf_end(e);

  return MachineID == TARGET_ELF_ID;
#endif
}

int32_t __tgt_rtl_number_of_devices() { return NUMBER_OF_DEVICES; }

int32_t __tgt_rtl_init_device(int32_t device_id) { return OFFLOAD_SUCCESS; }

static
int32_t checkForExitOnError(int32_t error) {
  if (NULL == getenv(ENV_EXIT_ON_ERROR)) {
    return error;
  }

  exit(error);
}

static
std::string find_csa_clang() {

  // If the user defined an envirionment variable to point to csa-clang,
  // use it
  const char *csa_clang_env = getenv(ENV_CLANG);
  if (NULL != csa_clang_env) {
    return csa_clang_env;
  }

  std::string csa_clang_path;

  // Get the PATH environment variable and make a copy I can muck with.
  const char *path = getenv("PATH");
  char *myPath = strdup(path);
  bool found_csa_clang = false;

  // Scan PATH looking for a directory that contains an executable
  // copy of csa-clang
  for (const char *dir = strtok(myPath, ":"); dir; dir = strtok(NULL, ":")) {
    csa_clang_path = dir;
    csa_clang_path += "/csa-clang";
    if (0 == access(csa_clang_path.c_str(), X_OK)) {
      found_csa_clang = true;
      break;
    }
  }

  // Free the allocated memory before I forget.
  free(myPath);

  // If we found a copy of csa-clang we can execute return it. Otherwise
  // return an empty string
  if (found_csa_clang) {
    return csa_clang_path;
  } else {
    return "";
  }
}

static
std::string find_gold_plugin() {

  // If the user defined an envirionment variable to point to the gold
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
std::string find_llvm_link(const std::string &csa_clang) {
  // Find the root of the path for csa-clang
  std::string::size_type pos = csa_clang.find_last_of('/');
  if (std::string::npos == pos) {
    return "";
  }

  // Assume that we can find csa-llvm-link in the same directory that
  // has csa-clang
  std::string llvm_link = csa_clang.substr(0, pos);
  llvm_link += "/csa-llvm-link";

  // Trust, but verify
  if (0 == access(llvm_link.c_str(), X_OK)) {
    return llvm_link;
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
    return true;
  }

  // There are multiple bitcode files in the image. We'll need to use
  // llvm-link to concatenate them. It (or the version with a csa- prefix)
  // should be in the same directory as csa-clang
  std::string llvmLinkCommand = find_llvm_link(csa_clang);
  if (llvmLinkCommand.empty()) {
    fprintf(stderr, "Failed to find csa-llvm-link.\n");
    return false;
  }

  llvmLinkCommand += " -o ";
  llvmLinkCommand += bcFile;

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

    data += len;

    llvmLinkCommand += " ";
    llvmLinkCommand += tmpBcFile;
  }

  // Verify that we dealt with all of the bitcode data
  assert (data == (const char *)bitcode_data->d_buf + bitcode_data->d_size);

  // Now execute the llvm-link command to concatenate the bitcode files
  int result = execute_command(llvmLinkCommand.c_str());
  
  // Cleanup the temporary files before we forget
  for (unsigned i = 0; i < bitcodeCount; i++) {
    std::string tmpBcFile(tmp_name);
    tmpBcFile += "_";
    char buf[16];
    sprintf(buf, "%u", i);
    tmpBcFile += buf;
    tmpBcFile += ".bc";
    remove(tmpBcFile.c_str());
  }

  if (0 == result) {
    return true;
  }

  DP("Failed to concatenate bitcode files extracted from the image\n");
  return false;
}

static
bool link_bc_file(const char *tmp_name,
                  const std::string &bcFile,
                  const std::string &ldGold,
                  const std::string &goldPlugin,
                  const std::string &goldOptions) {

  // Link the bitcode file against one or more archives using the ld.gold
  // linker
  std::string linkResult(tmp_name);
  linkResult += "-linked.bc";

  // Start building the command string
  std::string linkCommand(ldGold);

  // Make sure that the linker understands that .omp_offloading.entry must be
  // exported. Yeah, this is a hack
  linkCommand += " --entry=.omp_offloading.entry";

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
  if (0 == execute_command(linkCommand.c_str())) {
    // Remove the original bitcode file and replace it with what we just
    // created
    remove(bcFile.c_str());
    rename(linkResult.c_str(), bcFile.c_str());
    return true;
  }

  DP("Failed to create process to link Bitcode file\n");
  remove(linkResult.c_str());
  return false;
}

static
bool build_csa_assembly(const char *tmp_name,
                        const Elf_Data *bitcode_bounds,
                        const Elf_Data *bitcode_data,
                        std::string &csaAsmFile) {

  // If the user is using his own assembly file, don't bother. Do not fill
  // in csaAsmFile since we don't want to delete it when the shared object
  // unloads.
  if (getenv(ENV_ASSEMBLY_FILE)) {
    return true;
  }

  // Find csa-clang. Do this before we create anything that might need
  // to be cleaned up
  std::string csa_clang = find_csa_clang();
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

  std::string bcFile(tmp_name);
  bcFile += ".bc";

  // Extract the bitcode from the image. If there are multiple bitcode
  // files in the image, use llvm-link to contcatenate them into a
  // single bitcode file
  if (! extract_bc_file(tmp_name, bcFile, csa_clang,
                        bitcode_bounds, bitcode_data)) {
    return false;
  }

  // If the user gave us ld options, use ld.gold to build the final
  // .bc file
  if (! ldGold.empty()) {
    if (! link_bc_file(tmp_name, bcFile, ldGold, goldPlugin, goldOptions)) {
      remove(bcFile.c_str());
      return false;
    }
  }

  // Generate a name for the target file
  csaAsmFile = tmp_name;
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
    remove(csaAsmFile.c_str());
    remove(bcFile.c_str());
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
    DP("Incompatible ELF library!\n");
    checkForExitOnError(1);
    return NULL;
  }

  // Obtain elf handler
  Elf *e = elf_memory((char *)image->ImageStart, ImageSize);
  if (!e) {
    DP("Unable to get ELF handle: %s!\n", elf_errmsg(-1));
    checkForExitOnError(1);
    return NULL;
  }

  if (elf_kind(e) != ELF_K_ELF) {
    DP("Invalid Elf kind!\n");
    elf_end(e);
    checkForExitOnError(1);
    return NULL;
  }

  // Find the entries section offset
  Elf_Scn *section = 0;
  Elf_Data *bitcode_bounds = 0;
  Elf_Data *bitcode_data = 0;
  Elf64_Off entries_offset = 0;


  size_t shstrndx;

  if (elf_getshdrstrndx(e, &shstrndx)) {
    DP("Unable to get ELF strings index!\n");
    elf_end(e);
    checkForExitOnError(1);
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
    checkForExitOnError(1);
    return NULL;
  }

  if (!bitcode_bounds) {
    DP("CSA bitcode bounds section Not Found\n");
    elf_end(e);
    checkForExitOnError(1);
    return NULL;
  }

  if (!bitcode_data) {
    DP("CSA bitcode data section Not Found\n");
    elf_end(e);
    checkForExitOnError(1);
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
    checkForExitOnError(1);
    return NULL;
  }

  FILE *ftmp = fdopen(tmp_fd, "wb");

  if (!ftmp) {
    elf_end(e);
    checkForExitOnError(1);
    return NULL;
  }

  fwrite(image->ImageStart, ImageSize, 1, ftmp);
  fclose(ftmp);

  DynLibTy Lib = {tmp_name, dlopen(tmp_name, RTLD_LAZY)};

  if (!Lib.Handle) {
    DP("target library loading error: %s\n", dlerror());
    elf_end(e);
    checkForExitOnError(1);
    return NULL;
  }

  // Build the CSA assembly file
  std::string csaAsmFile;
  if (! build_csa_assembly(tmp_name, bitcode_bounds, bitcode_data, csaAsmFile)) {
    elf_end(e);
    remove(tmp_name);
    checkForExitOnError(1);
    return NULL;
  }

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
    DP("Can't obtain entries begin\n");
    elf_end(e);
    checkForExitOnError(1);
    return NULL;
  }

  DP("Entries table range is (%016lx)->(%016lx)\n", (Elf64_Addr)entries_begin,
     (Elf64_Addr)entries_end)
    DeviceInfo.createOffloadTable(device_id, entries_begin, entries_end, csaAsmFile);

  elf_end(e);

  return DeviceInfo.getOffloadEntriesTable(device_id);
}

void *__tgt_rtl_data_alloc(int32_t device_id, int64_t size, void *hst_ptr) {
  return hst_ptr;
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
  // ignore team num and thread limit.
  int32_t status = OFFLOAD_SUCCESS;

  // Normally this would be the code, but since we're loading assembly into
  // the simulator, the compilation gave us the address of the function name.
  const char *functionName = (const char *)tgt_entry_ptr;

  // If we've got an environment variable specifying the assembly file, 
  // use it's value
  const char *assemblyFile = getenv(ENV_ASSEMBLY_FILE);
  if (NULL == assemblyFile) {
    // Find the function in the target table, which will give us the path for
    // the compiled code so we can load it into the assembler
    assemblyFile = DeviceInfo.findOffloadEntry(device_id, tgt_entry_ptr);
  }

  if (NULL == assemblyFile) {
    DP("Failed to find assembly file\n");
    return checkForExitOnError(OFFLOAD_FAIL);
  }

  // Allocate an CSA
  csa_processor *processor = csa_alloc("autounit");
  if (NULL == processor) {
    DP("Failed to allocate CSA processor\n");
    status = checkForExitOnError(OFFLOAD_FAIL);
  } else {

    // Load the assembly file into the simulator
    csa_module *mod = csa_assemble(assemblyFile, processor);
    if (NULL == mod) {
      DP("Failed to load module\n");
      status = checkForExitOnError(OFFLOAD_FAIL);
    } else {

      // Find the function
      csa_entry *entry = csa_lookup(mod, functionName);
      if (NULL == entry) {
        DP("Failed to find entrypoint \"%s\"\n", functionName);
        status = checkForExitOnError(OFFLOAD_FAIL);
      } else {
        if (verbosity) {
          fprintf(stderr, "\nRunning %s on the CSA simulator..\n", functionName);
        }

        std::vector<void *> ptrs(arg_num);
        for (int32_t i=0; i<arg_num; ++i) {
          ptrs[i] = (void*)((intptr_t)tgt_args[i] + tgt_offsets[i]);
        }

        // Execute the CSA code. target() is adding an "omp handle" to the
        // arguments array which we don't care about. So ignore it
        assert(sizeof(csa_arg) == sizeof(ptrs[0]));
        csa_call(entry, arg_num-1, (csa_arg *)&ptrs[0]);

        // Dump the statistics, if requested
        if (getenv (ENV_DUMP_STATS)) {
          csa_dump_statistics(processor);
        }

        if (verbosity) {
          fprintf(stderr, "\n%s ran on the CSA simulator\n\n", functionName);
        }
      }
    }
  }
  csa_free(processor);

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
