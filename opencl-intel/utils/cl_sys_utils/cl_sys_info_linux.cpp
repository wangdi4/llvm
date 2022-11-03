// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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

#include "cl_sys_info.h"

#include <sstream>

using namespace Intel::OpenCL::Utils;
#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <time.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/sysinfo.h>

#include <sys/syscall.h>

#ifdef USE_NUMA
#include <numa.h>
#endif // USE_NUMA

#include "cl_secure_string_linux.h"
#include "hw_utils.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/Valgrind.h"

using namespace llvm;
using namespace llvm::sys;

unsigned long long Intel::OpenCL::Utils::TotalVirtualSize() {

  static unsigned long long vsize = 0;
  if (0 == vsize) {
    rlimit tLimitStruct;
    if (getrlimit(RLIMIT_AS, &tLimitStruct) != 0) {
      return 0;
    }
    unsigned long long totalVirtual = tLimitStruct.rlim_cur;

    struct sysinfo tSysInfoStruct;
    if (sysinfo(&tSysInfoStruct) != 0) {
      return 0;
    }
    unsigned long long totalPhys =
        tSysInfoStruct.totalram * tSysInfoStruct.mem_unit;

    vsize = min(totalPhys, totalVirtual);
  }
  return vsize;
}

unsigned long long Intel::OpenCL::Utils::TotalPhysicalSize() {
  static unsigned long long totalPhys = 0;
  if (0 == totalPhys) {
    struct sysinfo tSysInfoStruct;
    if (sysinfo(&tSysInfoStruct) != 0) {
      return 0;
    }
    totalPhys = tSysInfoStruct.totalram * tSysInfoStruct.mem_unit;
  }

  return totalPhys;
}

unsigned long long Intel::OpenCL::Utils::MaxClockFrequency() {
  static unsigned long long freq = 0;
  unsigned int cpuInfo[4] = {0 - 1u};
  char buffer[sizeof(cpuInfo) * 3 + 1];
  char *pBuffer = buffer;

  if (freq) {
    return freq;
  }

  memset(buffer, 0, sizeof(cpuInfo) * 3 + 1);
  for (unsigned int i = 0x80000002; i <= 0x80000004; i++) {
    cpuid(cpuInfo, i);
    memcpy(pBuffer, cpuInfo, sizeof(cpuInfo));
    pBuffer = pBuffer + sizeof(cpuInfo);
  }

  int buffLen = strlen(buffer);
  long long mul = 0;
  double freqDouble = 0;
  if ((buffer[buffLen - 1] == 'z') && (buffer[buffLen - 2] == 'H') &&
      ((buffer[buffLen - 3] == 'M') || (buffer[buffLen - 3] == 'G') ||
       (buffer[buffLen - 3] == 'T'))) {
    switch (buffer[buffLen - 3]) {
    case 'M':
      mul = 1;
      break;
    case 'G':
      mul = 1000;
      break;
    case 'T':
      mul = 1000000;
      break;
    }

    int i = buffLen - 1;
    while (i >= 0) {
      if (buffer[i] == ' ') {
        freqDouble = strtod(&(buffer[i]), nullptr);
        break;
      }
      i--;
    }
  }
  // We return ClockFreq in MHz
  freq = (unsigned long long)(freqDouble * mul);
  return freq;
}

unsigned long long Intel::OpenCL::Utils::ProfilingTimerResolution() {
  /* sys_clock_getres returns resolution (ns interval between ticks) and not the
   * frequency. */
  struct timespec tp;

  clock_getres(CLOCK_MONOTONIC, &tp);
  return tp.tv_nsec;
}

/////////////////////////////////////////////////////////////////////////////////////////
// HostTime - Return host time in nano second
/////////////////////////////////////////////////////////////////////////////////////////
unsigned long long Intel::OpenCL::Utils::AccurateHostTime() {
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return (unsigned long long)(tp.tv_sec) * 1000000000 + tp.tv_nsec;
}

unsigned long long Intel::OpenCL::Utils::HostTime() {
  return AccurateHostTime();
}

/////////////////////////////////////////////////////////////////////////////////////////
// CurrentProcessName
/////////////////////////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::GetProcessName(char *pProcName, size_t strLen) {
  const int readChars = readlink("/proc/self/exe", pProcName, strLen);
  if (-1 == readChars) {
    pProcName[0] = 0;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// CurrentProcessId
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int Intel::OpenCL::Utils::GetProcessId() { return getpid(); }

/////////////////////////////////////////////////////////////////////////////////////////
// Current Module Directory Name
/////////////////////////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::GetModuleDirectoryImp(const void *addr,
                                                 char *szModuleDir,
                                                 size_t strLen) {
  const int readChars = GetModulePathName(addr, szModuleDir, strLen - 1);
  if (readChars > 0) {
    char *pLastDelimiter = strrchr(szModuleDir, '/');
    if (nullptr != pLastDelimiter) {
      *(pLastDelimiter + 1) = 0;
    } else {
      szModuleDir[0] = 0;
    }
  } else {
    szModuleDir[0] = 0;
  }
}

std::string Intel::OpenCL::Utils::GetClangRuntimePath() {
  char ModuleName[MAX_PATH];
  GetModuleDirectory(ModuleName, MAX_PATH);
  std::string BaseLibDir =
      std::string(path::parent_path(path::parent_path(ModuleName)));

  SmallString<128> P(BaseLibDir);

  path::append(P, "clang", CLANG_VERSION_STRING, "lib",
               llvm::sys::getDefaultTargetTriple());

  return std::string(P.str());
}

int CharToHexDigit(char c) {

  if ((c >= '0') && (c <= '9')) {
    return c - '0';
  }
  if ((c >= 'a') && (c <= 'f')) {
    return c - 'a' + 10;
  }
  if ((c >= 'A') && (c <= 'F')) {
    return c - 'A' + 10;
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Specific module full path name intended for loaded library full path.
// On Win32 - it asks for the module handle then calls GetModuleFileNameA
// method.  (modulePtr must be address of method belongs to the loaded library)
// On Linux - it investigates the loaded library path from /proc/self/maps.
// (modulePtr must be address of method belongs to the loaded library)
////////////////////////////////////////////////////////////////////
int Intel::OpenCL::Utils::GetModulePathName(const void *modulePtr,
                                            char *fileName, size_t strLen) {
  if ((fileName == nullptr) || (strLen <= 0)) {
    return 0;
  }
  ifstream ifs("/proc/self/maps", ifstream::in);
  if (!ifs.good()) {
    fileName[0] = 0;
    return 0;
  }
  string address, perms, offset, dev, inode, pathName;

  char buff[MAX_PATH + 1024];
  while (ifs.getline(buff, MAX_PATH + 1024)) {
    istringstream strStream(buff);
    address = "\0";
    pathName = "\0";
    strStream >> address >> perms >> offset >> dev >> inode >> pathName;
    if ((address != "\0") && (pathName != "\0")) {
      string::size_type pos = address.find("-");
      if (pos != string::npos) {
        size_t from = 0;
        size_t to = 0;
        bool legalAddress = true;
        for (unsigned int i = 0; ((i < pos) && (legalAddress)); i++) {
          int digit = CharToHexDigit(address.at(i));
          if (digit >= 0) {
            from = (from << 4) + digit;
          } else {
            legalAddress = false;
          }
        }
        if (!legalAddress) {
          continue;
        }
        int len = address.length();
        for (int i = pos + 1; ((i < len) && (legalAddress)); i++) {
          int digit = CharToHexDigit(address.at(i));
          if (digit >= 0) {
            to = (to << 4) + digit;
          } else {
            legalAddress = false;
          }
        }
        if (!legalAddress) {
          continue;
        }
        if (((size_t)modulePtr >= from) && ((size_t)modulePtr <= to)) {
          assert(strLen >= pathName.size() &&
                 "safeStrCpy() produces not null-terminated string in case of"
                 "nubmer of copied chars is less than length of src string");
          if (0 != safeStrCpy(fileName, strLen, pathName.c_str())) {
            int counter = 0;
            for (unsigned int i = 0;
                 ((i < strLen - 1) && (i < pathName.length())); i++) {
              fileName[i] = pathName.at(i);
              counter++;
            }
            fileName[counter] = 0;
            return counter;
          }
          return pathName.length();
        }
      }
    }
  }
  fileName[0] = 0;
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the number of processors configured.
// It is also possible to ge the number of processors currently online
// (Available), by changing the function to sysconf(_SC_NPROCESSORS_ONLN)
////////////////////////////////////////////////////////////////////
unsigned long Intel::OpenCL::Utils::GetNumberOfProcessors() {
  static unsigned long numProcessors = 0;
  if (0 == numProcessors) {
    affinityMask_t mask;
    threadid_t mainThreadTID = (threadid_t)GetProcessId();
    clGetThreadAffinityMask(&mask, mainThreadTID);
    numProcessors = CPU_COUNT(&mask);
  }
  return numProcessors;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the number of physical cpus (sockets) configured.
////////////////////////////////////////////////////////////////////
unsigned int Intel::OpenCL::Utils::GetNumberOfCpuSockets() {
  static unsigned int numCpuSockets = 0;
  if (0 == numCpuSockets) {
    // TODO use hwloc to detect number of sockets

    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
      char *line = nullptr;
      size_t len;
      const char *s = "physical id";
      while (getline(&line, &len, fp) != -1) {
        if (strncmp(line, s, strlen(s)) != 0)
          continue;

        char *p = strchr(line, ':');
        if (!p)
          continue;
        unsigned int physicalID = 0;
        sscanf(p + 1, "%u\n", &physicalID);
        physicalID++;
        if (numCpuSockets < physicalID)
          numCpuSockets = physicalID;
      }
      if (line != nullptr)
        free(line);
      fclose(fp);
    } else
      assert(false && "Failed to open /proc/cpuinfo");
  }
  assert(numCpuSockets != 0 && "Number of sockets should not be 0");
  return numCpuSockets;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return whether cpu is using hyper-threading
////////////////////////////////////////////////////////////////////
bool Intel::OpenCL::Utils::IsHyperThreadingEnabled() {
  static int hyperThreadingEnabled = -1;
  if (-1 == hyperThreadingEnabled) {
    // TODO use hwloc to detect hyper-threading

    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
      char *line = nullptr;
      size_t len;
      unsigned int siblings = 0;
      unsigned int cpuCores = 0;
      const char *s0 = "siblings";
      const char *s1 = "cpu cores";
      while (getline(&line, &len, fp) != -1 &&
             (0 == siblings || 0 == cpuCores)) {
        char *p = strchr(line, ':');
        if (!p)
          continue;
        if (0 == siblings && strncmp(line, s0, strlen(s0)) == 0)
          sscanf(p + 1, "%u\n", &siblings);
        if (0 == cpuCores && strncmp(line, s1, strlen(s1)) == 0)
          sscanf(p + 1, "%u\n", &cpuCores);
      }
      if (line != nullptr)
        free(line);
      fclose(fp);
      if (0 != siblings && 0 != cpuCores)
        hyperThreadingEnabled = (siblings == (2 * cpuCores)) ? 1 : 0;
    } else
      assert(false && "Failed to open /proc/cpuinfo");
  }
  return 1 == hyperThreadingEnabled;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the number of NUMA nodes on the system
////////////////////////////////////////////////////////////////////
unsigned long Intel::OpenCL::Utils::GetMaxNumaNode() {
  // TODO use hwloc to get the number numa nodes
  const char *sysNodePath = "/sys/devices/system/node";
  int numNodes = 0;
  DIR *dir;
  struct dirent *entry;
  dir = opendir(sysNodePath);

  if (dir == nullptr) {
    assert(dir && "failed to open node dir");
    return 0;
  }

  while ((entry = readdir(dir))) {
    if (entry->d_type == DT_DIR && strncmp("node", entry->d_name, 4) == 0)
      numNodes++;
  }

  closedir(dir);
  assert(numNodes > 0 && "Failed to get number of NUMA nodes");
  return numNodes;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return an index representing the processors in a given NUMA node
////////////////////////////////////////////////////////////////////
bool Intel::OpenCL::Utils::GetProcessorIndexFromNumaNode(
    unsigned long node, std::vector<cl_uint> &index) {
  // TODO use hwloc to get the cpu index of numa node
  index.clear();
  const char *prefix = "/sys/devices/system/node/node";
  std::string path(prefix);
  path.append(std::to_string(node) + "/cpumap");
  FILE *cpumap = fopen(path.c_str(), "r");

  if (cpumap == nullptr) {
    assert(cpumap && "failed to open cpumap");
    return false;
  }

  char cpumask[128];
  if (fgets(cpumask, 128, cpumap)) {
    int len = strlen(cpumask);
    if (len < 1)
      return false;
    const char *maskEnd = cpumask + len - 1;
    uint16_t core = 0;
    while (maskEnd >= cpumask) {
      if (*maskEnd == ',' || *maskEnd == '\n') {
        maskEnd--;
        continue;
      }
      int cpu = CharToHexDigit(*maskEnd);
      if (cpu & 1)
        index.push_back(core);
      if (cpu & 2)
        index.push_back(core + 1);
      if (cpu & 4)
        index.push_back(core + 2);
      if (cpu & 8)
        index.push_back(core + 3);
      maskEnd--;
      core += 4;
    }
  }
  fclose(cpumap);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return a bitmask representing the processors in a given NUMA node
////////////////////////////////////////////////////////////////////
bool Intel::OpenCL::Utils::GetProcessorMaskFromNumaNode(
    unsigned long node, affinityMask_t *pMask, unsigned int *nodeSize) {
#ifdef USE_NUMA
  struct bitmask b;
  unsigned long long CPUs;
  b.size = 8 * sizeof(unsigned long long);
  b.maskp = (unsigned long *)(&CPUs);
  int ret = numa_node_to_cpus((int)node, &b);
  if (0 != ret) {
    return false;
  }
  CPU_ZERO(pMask);
  int cpu = 0;
  unsigned int node_size = 0;
  while (0 != CPUs) {
    if (CPUs & 0x1) {
      CPU_SET(cpu, pMask);
      ++node_size;
    }
    CPUs >>= 1;
    ++cpu;
  }
  if (nullptr != nodeSize) {
    *nodeSize = node_size;
  }
  return true;
#else
  // The parameters are used if USE_NUMA macro is defined
  (void)node;
  (void)pMask;
  (void)nodeSize;
  return false;
#endif // USE_NUMA
}

std::map<int, int> Intel::OpenCL::Utils::GetProcessorToSocketMap() {
  // TODO use hwloc.
  const unsigned numSockets = Intel::OpenCL::Utils::GetNumberOfCpuSockets();
  (void)numSockets;
  std::map<int, int> CoreIdToPhysicalId;

  std::string SysCpuDir = "/sys/devices/system/cpu";
  assert(fs::is_directory(SysCpuDir) &&
         "/sys/devices/system/cpu isn't a directory");

  Regex r("cpu[0-9]+");
  std::error_code EC;
  for (fs::directory_iterator I(SysCpuDir, EC), E; I != E && !EC;
       I.increment(EC)) {
    const std::string Path = I->path();
    StringRef Filename = path::filename(Path);
    if (!r.match(Filename))
      continue;
    std::string PhysicalIdFile = Path + "/topology/physical_package_id";
    std::string Buf(4, ' ');
    Expected<fs::file_t> FD = fs::openNativeFileForRead(PhysicalIdFile);
    if (!FD)
      assert(false && toString(FD.takeError()).c_str());
    if (Expected<size_t> BytesRead = fs::readNativeFile(
            *FD, makeMutableArrayRef(&*Buf.begin(), Buf.size())))
      Buf = Buf.substr(0, *BytesRead);
    else
      assert(false && toString(BytesRead.takeError()).c_str());
    EC = fs::closeFile(*FD);
    assert(!EC && "failed to close file");
    int PhysicalId = std::stoi(Buf);
    assert((unsigned)PhysicalId < numSockets && "physical id is out of range");
    CoreIdToPhysicalId[std::stoi(Filename.substr(3).str())] = PhysicalId;
  }

  return CoreIdToPhysicalId;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the ID of the CPU the current thread is running on
////////////////////////////////////////////////////////////////////
unsigned int Intel::OpenCL::Utils::GetCpuId() {
  int id;

  if (llvm::sys::RunningOnValgrind()) {
    id = 0;
  } else {
    id = sched_getcpu();
  }
  assert(id >= 0);
  return (unsigned int)id;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the full path to the module that is about to be loaded
// In linux returns only the modules name as modules are loaded according to
// LD_LIBRARY_PATH
////////////////////////////////////////////////////////////////////
const char *
Intel::OpenCL::Utils::GetFullModuleNameForLoad(const char *moduleName) {
  return moduleName;
}

unsigned int Intel::OpenCL::Utils::GetThreadId() {
  return (unsigned int)syscall(SYS_gettid);
}
