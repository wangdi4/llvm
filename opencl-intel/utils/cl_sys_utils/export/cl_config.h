// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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

#pragma once

#include "PipeCommon.h"
#include "cl_env.h"
#include "cl_types.h"
#include "cpu_dev_limits.h"

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

using std::string;
using std::vector;

namespace Intel {
namespace OpenCL {
namespace Utils {

/*******************************************************************************
 * Class name: ConfigFile
 *
 * Description: represents an ConfigFile object
 ******************************************************************************/
class ConfigFile {
protected:
  std::map<string, string> m_mapContents; // extracted keys and values

  typedef std::map<string, string>::iterator mapi;
  typedef std::map<string, string>::const_iterator mapci;

  string m_sDelimiter; // separator between key and value
  string m_sComment;   // separator between value and comments
  string m_sSentry;    // optional string to signal end of file

public:
  /*****************************************************************************
   * Function: ConfigFile
   * Description: The ConfigFile class constructor
   * Arguments: strFileName [in] local configuration file name
   *            strDelimiter  [in] the configuration parameter's delimiter
   *                             - separator between key and value
   *            strComment  [in] configuration parameter's comments identifier
   *                             - separator between value and comments
   *            strSentry   [in] represents end of file - optional string to
   *                             signal end of file
   ****************************************************************************/
  ConfigFile(const string &strFileName, string strDelimiter = "=",
             string strComment = "#", string strSentry = "EndConfigFile");

  /*****************************************************************************
   * Function:  ConfigFile
   * Description:  The ConfigFile class constructor
   * Arguments:
   ****************************************************************************/
  ConfigFile();

  /*****************************************************************************
   * Function: Read
   * Description: Search for key and read value or optional default value call
   *              as Read<T>(key, default_value)
   * Arguments: strKey [in]  reference to the string that represnts the key
   *            value  [in]  reference to the default value
   *            loadEnvFirst define whether read the value from environment
   *                         variables first or not
   * Return value: the value which assign to the key
   ****************************************************************************/
  template <class T>
  T Read(const string &strKey, const T &value, bool loadEnvFirst = true) const;

  /*****************************************************************************
   * Function: ReadInto
   * Description: Search for key and read value into variable call as
   *              ReadInto<T>(var, key)
   * Arguments: var [in] reference to the variable to which the value will be
   *                     assigned to
   *            key [in] reference to the string that represnts the key
   * Return value: True - if the value exists and was assign successfully to the
   *                      variable
   *               False - the value doesn't exists or can't assign value to the
   *                       variable
   ****************************************************************************/
  template <class T> bool ReadInto(T &var, const string &key) const;

  /*****************************************************************************
   * Function: ReadInto
   * Description: Search for key and read value or optional default value into
   *              variable call as ReadInto<T>(var, key, default_value)
   * Arguments: var - refernce to the variable to which the value will be
   *                  assigned
   *            key [in] - reference to the string that represents the key
   *            value [in] - reference to the default value
   * Return value: True - if the value exists and was assign successfully to the
   *                      variable
   *               False - the value doesn't exists or can't assign value to the
   *                       variable
   ****************************************************************************/
  template <class T>
  bool ReadInto(T &var, const string &key, const T &value) const;

  /*****************************************************************************
   * Function: Add
   * Description: Modify keys and values or add new configuration item call as
   *              Add<T>(key, value)
   * Arguments: key [in] - represents the key of the configuration item
   *            value [in] - reference to the value of the configuration item
   * Return value:
   ****************************************************************************/
  template <class T> void Add(string key, const T &value);

  /*****************************************************************************
   * Function: Remove
   * Description: remove configuration item call as Remove(key)
   * Arguments: key [in] - represents the key of the configuration item
   * Return value:
   ****************************************************************************/
  void Remove(const string &key);

  /*****************************************************************************
   * Function: KeyExists
   * Description: Check whether key exists in configuration call as
   *              KeyExists(key)
   * Arguments: key [in] - represents the key of the configuration item
   * Return value: True - the key exists in configuration
   *               False - the key doesn't exist in configuration
   ****************************************************************************/
  bool KeyExists(const string &key) const;

  /*****************************************************************************
   * Function: GetDelimiter
   * Description: get delimiter of configuration syntax call as delimiter =
   *              GetDelimiter()
   * Arguments:
   * Return value: delimiter of configuration syntax
   ****************************************************************************/
  string GetDelimiter() const { return m_sDelimiter; }

  /*****************************************************************************
   * Function: GetComment
   * Description: get comment of configuration syntax call as comment =
   *              GetComment()
   * Arguments:
   * Return value: comment of configuration syntax
   ****************************************************************************/
  string GetComment() const { return m_sComment; }

  /*****************************************************************************
   * Function: GetSentry
   * Description: Get value of the sentry from the configuration syntax call as
   *              sentry = GetSentry()
   * Arguments:
   * Return value: sentry of configuration syntax
   ****************************************************************************/
  string GetSentry() const { return m_sSentry; }

  /*****************************************************************************
   * Function: SetDelimiter
   * Description: set new delimiter value to the configuration syntax call as
   *              old_delimiter = SetDelimiter(new_delimiter)
   * Arguments: strDelimiter [in] - new delimiter
   * Return value: previous value of delimiter of configuration syntax
   ****************************************************************************/
  string SetDelimiter(const string &strDelimiter) {
    string strPrefDelimiter = m_sDelimiter;
    m_sDelimiter = strDelimiter;
    return strPrefDelimiter;
  }

  /*****************************************************************************
   * Function: SetComment
   * Description: set new comment value to the configuration syntax call as
   *              old_commens = SetComment(new_comment)
   * Arguments: s [in] -  new comment
   * Return value: previous value of comment of configuration syntax
   ****************************************************************************/
  string SetComment(const string &strComment) {
    string strOldComment = m_sComment;
    m_sComment = strComment;
    return strOldComment;
  }

  /*****************************************************************************
   * Function: ReadFile
   * Description: read configuration file into ConfigFile object
   * Arguments: fileName [in] - full path of configuration file
   *            cf [in] - reference to configuration file object
   * Return value: CL_SUCCESS - file was read successfully
   *               CL_ERR_FILE_NOT_EXISTS - file name doesn't exists
   ****************************************************************************/
  static cl_err_code ReadFile(const string &fileName, ConfigFile &cf);

  /*****************************************************************************
   * Function: WriteFile
   * Description: write ConfigFile object into configuration file
   * Arguments: fileName [in] - full path of configuration file
   *            cf [in] - reference to configuration file object
   * Return value: CL_SUCCESS - file was read successfully
   ****************************************************************************/
  static cl_err_code WriteFile(string fileName, ConfigFile &cf);

  /*****************************************************************************
   * Function: tokenize
   * Description: create substrings vector from a tring sentence. using the *';'
   *              or ',' or '|' character as seperators
   * Arguments: sin [in] - input string
   *            tokens [in] -  output substrings vector
   * Return value: number of substrings in vector
   ****************************************************************************/
  static int tokenize(const string &sin, std::vector<string> &tokens);

  /*****************************************************************************
   * Function: ConvertTtypeToString
   * Description: convert class T to string (Type T must support << operator)
   * Arguments: t [in] - input class
   * Return value:  string which represents the class
   ****************************************************************************/
  template <class T> static string ConvertTypeToString(const T &t);

  /*****************************************************************************
   * Function: ConvertStringToType
   * Description: convert string to class T (Type T must support << operator)
   * Arguments: s [in] - input string
   * Return value: class T
   ****************************************************************************/
  template <class T> static T ConvertStringToType(const string &str);

  /// Convert C string to class T.
  template <class T> static T ConvertStringToType(const char *str) {
    if (!str)
      return false;
    std::string strInput(str);
    return ConvertStringToType<T>(strInput);
  }

  /*****************************************************************************
   * Function: GetRegistryOrEtcValue
   * Description: Get a value from registry on Windows (in key
   *              SOFTWARE\Intel\OpenCL) or from a file in
   *              /etc/OpenCL/vendors/Intel/ folder.
   * Arguments: name [in] - the name of the registry value or etc file
   *            defaultVal - default value that is returned in any case of error
   * Return value: the value stored in the registry
   ****************************************************************************/
  template <class T>
  static T GetRegistryOrEtcValue(const string &name, const T &defaultVal);

protected:
  /*****************************************************************************
   * Function: trim
   * Description: trim operation - remove unnecessary empty strings
   * Arguments: s [in] - reference to input string
   * Return value:
   ****************************************************************************/
  static void trim(string &str);
};

// Convert Type T to a string
template <class T> string ConfigFile::ConvertTypeToString(const T &t) {
  std::ostringstream ostOutput;
  ostOutput << t;
  return ostOutput.str();
}

// Convert string argument into a specific Type
template <class T> T ConfigFile::ConvertStringToType(const string &str) {
  T returned_type;
  std::istringstream istInput(str);
  istInput.unsetf(std::ios::basefield);
  istInput >> returned_type;
  return returned_type;
}

// Convert string argument to a string type
template <>
inline string ConfigFile::ConvertStringToType<string>(const string &str) {
  return str;
}

// Convert string argument to a bool type
// False considered as one of the followings: empty string or
// {"0", "false", "F", "no", "n"}
// True considered as one of the followings: {"1", "True", "true", "T", "yes",
// "y", "-1", all others}
template <>
inline bool ConfigFile::ConvertStringToType<bool>(const string &str) {
  if (str.empty())
    return false;
  string strInput = str;
  string::iterator it = strInput.begin();
  while (it != strInput.end()) {
    // convert all string chars to upper case
    *it = toupper(*it);
    ++it;
  }
  if (strInput == string("0") || strInput == string("FALSE") ||
      strInput == string("NO") || strInput == string("F") ||
      strInput == string("N") || strInput == string("NONE")) {
    return false;
  }
  return true;
}

template <class T>
T ConfigFile::Read(const string &key, const T &value, bool loadEnvFirst) const {
  // search first for environment variable
  string strEnv;
  // For some key, we only load value from config file.
  if (loadEnvFirst && Intel::OpenCL::Utils::getEnvVar(strEnv, key))
    return ConvertStringToType<T>(strEnv);

  // Return the value corresponding to key or given default value
  // if key is not found
  mapci p = m_mapContents.find(key);
  if (p == m_mapContents.end()) {
    return value;
  }
  return ConvertStringToType<T>(p->second);
}

template <class T>
bool ConfigFile::ReadInto(T &returnedVar, const string &strKey) const {
  // search first for environment variable
  bool bFound = true;
  string strEnv;
  if (Intel::OpenCL::Utils::getEnvVar(strEnv, strKey)) {
    returnedVar = ConvertStringToType<T>(strEnv);
    return bFound;
  }

  // if the environment value doesn't exists get the value corresponding to the
  // input key and store it in var
  std::map<string, string>::const_iterator it = m_mapContents.find(strKey);
  bFound = (it != m_mapContents.end());
  if (bFound) {
    returnedVar = ConvertStringToType<T>(it->second);
  }
  return bFound;
}

template <class T>
bool ConfigFile::ReadInto(T &returnVar, const string &strKey,
                          const T &defultValue) const {
  // search first for environment variable
  bool bFound = true;
  string strEnv;
  if (Intel::OpenCL::Utils::getEnvVar(strEnv, strKey)) {
    returnVar = ConvertStringToType<T>(strEnv);
    return bFound;
  }

  // if the environment value doesn't exists get the value corresponding to the
  // input key and store it in var
  std::map<string, string>::const_iterator it = m_mapContents.find(strKey);
  bFound = (it != m_mapContents.end());
  if (bFound) {
    returnVar = ConvertStringToType<T>(it->second);
  } else {
    returnVar = defultValue;
  }
  return bFound;
}

template <class T> void ConfigFile::Add(string strKey, const T &value) {
  // Add a key with given value
  string strValue = ConvertTypeToString(value);
  trim(strKey);
  trim(strValue);
  m_mapContents[strKey] = strValue;
  return;
}

#ifdef _WIN32

template <typename T>
T GetRegistryValue(HKEY key, const string &valName, const T &defaultVal) {
  T regVal;
  DWORD regValSize = sizeof(regVal);
  LONG res = RegQueryValueExA(key, valName.c_str(), nullptr, nullptr,
                              (BYTE *)&regVal, &regValSize);
  if (ERROR_SUCCESS != res) {
    return defaultVal;
  }
  return regVal;
}

template <typename T>
T GetRegistryKeyValue(const string &keyName, const string &valName,
                      T defaultVal) {
  HKEY key = nullptr;
  LONG res = RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyName.c_str(), 0,
                           KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key);

  if (ERROR_SUCCESS != res) {
    return defaultVal;
  } else {
    T regVal = GetRegistryValue(key, valName, defaultVal);
    RegCloseKey(key);
    return regVal;
  }
}
#endif

#ifndef DEVICE_NATIVE
template <class T>
T ConfigFile::GetRegistryOrEtcValue(const string &name, const T &defaultVal) {
#ifdef _WIN32
  return GetRegistryKeyValue("SOFTWARE\\Intel\\OpenCL", name, defaultVal);
#else
  T regVal;
  std::ifstream ifs(("/etc/OpenCL/vendors/Intel/" + name).c_str());
  if (!ifs.good()) {
    return defaultVal;
  }
  ifs >> regVal;
  if (!ifs.good()) {
    return defaultVal;
  }
  ifs.close();
  return regVal;
#endif
}
#endif

enum OPENCL_VERSION {
  OPENCL_VERSION_UNKNOWN = 0,
  OPENCL_VERSION_1_0 = 1,
  OPENCL_VERSION_1_1 = 2,
  OPENCL_VERSION_1_2 = 3,
  OPENCL_VERSION_2_0 = 4,
  OPENCL_VERSION_2_1 = 5,
  OPENCL_VERSION_2_2 = 6,
  OPENCL_VERSION_3_0 = 7
};

OPENCL_VERSION GetOpenclVerByCpuModel();

/**
 * This is the base class to all config wrappers.
 */
class BasicCLConfigWrapper {
public:
  BasicCLConfigWrapper() : m_pConfigFile(nullptr) {}

  virtual ~BasicCLConfigWrapper() { Release(); }

  BasicCLConfigWrapper(const BasicCLConfigWrapper &) = delete;
  BasicCLConfigWrapper &operator=(const BasicCLConfigWrapper &) = delete;

  virtual cl_err_code Initialize(std::string filename) {
    m_pConfigFile = new ConfigFile(filename);
    return CL_SUCCESS;
  }

  bool IsInitialized() const { return nullptr != m_pConfigFile; }

  void Release() {
    if (nullptr != m_pConfigFile) {
      delete m_pConfigFile;
      m_pConfigFile = nullptr;
    }
  }

  /**
   * @return the dynamically detected OpenCL version (according to registry in
   * Windows and /etc/ in Linux)
   */
  OPENCL_VERSION GetOpenCLVersion() const;
  bool DisableStackDump() const {
#ifndef NDEBUG
    return m_pConfigFile->Read<bool>("CL_DISABLE_STACK_TRACE", false);
#else
    return false;
#endif
  }
  bool UseRelaxedMath() const {
    return m_pConfigFile->Read<bool>("CL_CONFIG_USE_FAST_RELAXED_MATH", false);
  }
  int RTLoopUnrollFactor() const {
    return m_pConfigFile->Read<int>("CL_CONFIG_CPU_RT_LOOP_UNROLL_FACTOR", 1);
  }

  /**
   * @returns using name of device mode set from/in config
   * file
   */
  std::string GetDeviceModeName() const {
    return m_pConfigFile->Read<string>("CL_CONFIG_DEVICES", "", false);
  }

  /**
   * @returns using device mode set from/in config file
   */
  DeviceMode GetDeviceMode() const {
    std::string strDeviceMode = GetDeviceModeName();
    if ("fpga-emu" == strDeviceMode) {
      return FPGA_EMU_DEVICE;
    }
    // TODO: strDeviceMode may be empty. But currently we do not define
    // invalid device. We return CPU device by default. It's better to fix
    // it.
    return CPU_DEVICE;
  }

  cl_ulong GetStackDefaultSize() const {
    std::string strStackDefaultSize;
    if (!m_pConfigFile->ReadInto(strStackDefaultSize,
                                 "CL_CONFIG_STACK_DEFAULT_SIZE")) {
      return CPU_DEV_STACK_DEFAULT_SIZE;
    }
    return ParseStringToSize(strStackDefaultSize);
  }

  cl_ulong GetStackExtraSize() const {
    std::string strStackExtraSize;
    if (!m_pConfigFile->ReadInto(strStackExtraSize,
                                 "CL_CONFIG_STACK_EXTRA_SIZE")) {
      return CPU_DEV_STACK_EXTRA_SIZE;
    }
    return ParseStringToSize(strStackExtraSize);
  }

  bool UseAutoMemory() const {
    // Auto memory is disabled by default with x86 Windows build (32bit).
    // There is no request to enable this mechanism w/x86 Windows build.
    // And it may lead to "no enough memory resources" error on processors
    // with many cores (like icelake).
    return m_pConfigFile->Read<bool>("CL_CONFIG_AUTO_MEMORY",
#if defined(_WIN32) && !defined(_WIN64)
                                     false
#else
                                     true
#endif
    );
  }

  cl_ulong GetForcedLocalMemSize() const {
    std::string strForcedSize;
    if (!m_pConfigFile->ReadInto(strForcedSize,
                                 "CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE")) {
      // Let GetForcedPrivateMemSize and GetForcedLocalMemSize also
      // work when auto memory allocation mode is enabled. They should
      // be set as the max allocable memory size because they are used
      // as checking whether the execution of kernel will be out of resource
      // and are also needed to provide the memory info such as
      // CL_DEVICE_LOCAL_MEM_SIZE.
      return (GetDeviceMode() == FPGA_EMU_DEVICE) ? 0 : CPU_DEV_LCL_MEM_SIZE;
    }

    return ParseStringToSize(strForcedSize);
  }

  cl_ulong GetForcedPrivateMemSize() const {
    std::string strForcedSize;
    if (!m_pConfigFile->ReadInto(strForcedSize,
                                 "CL_CONFIG_CPU_FORCE_PRIVATE_MEM_SIZE")) {
      return (GetDeviceMode() == FPGA_EMU_DEVICE) ? 0
                                                  : CPU_DEV_MAX_WG_PRIVATE_SIZE;
    }
    return ParseStringToSize(strForcedSize);
  }

  int GetChannelDepthEmulationMode() const {
    std::string strDepthEmulationMode;
    if (!m_pConfigFile->ReadInto(strDepthEmulationMode,
                                 "CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE")) {
      return CHANNEL_DEPTH_MODE_STRICT;
    }

    if (strDepthEmulationMode == "ignore-depth")
      return CHANNEL_DEPTH_MODE_IGNORE_DEPTH;
    if (strDepthEmulationMode == "default")
      return CHANNEL_DEPTH_MODE_DEFAULT;
    if (strDepthEmulationMode == "strict")
      return CHANNEL_DEPTH_MODE_STRICT;

    assert(0 && "Unknown channel depth emulation mode!");
    return CHANNEL_DEPTH_MODE_STRICT;
  }

  /**
   * @returns maximum work-group size for cpu or fpga emulator device.
   */
  size_t GetDeviceMaxWGSize(bool IsFPGAEmulator = false) const;

  std::string GetForcedWGSize() const {
    std::string WGSize;
    (void)m_pConfigFile->ReadInto(WGSize,
                                  "CL_CONFIG_CPU_FORCE_WORK_GROUP_SIZE");
    return WGSize;
  }

  /**
   * @returns the TBB version number
   */
  std::string GetTBBVersion() const {
    return m_pConfigFile->Read<string>("CL_CONFIG_TBB_VERSION", "", false);
  }

  /**
   * @returns the TBB DLL PATH
   */
  std::string GetTBBDLLPath() const {
    return m_pConfigFile->Read<string>("CL_CONFIG_TBB_DLL_PATH", "", false);
  }
  /**
   * @returns the number of TBB workers.
   */
  unsigned GetNumTBBWorkers() const;

  /**
   * @returns use streaming always or not
   */
  bool GetStreamingAlways() const {
    return m_pConfigFile->Read<bool>("CL_CONFIG_CPU_STREAMING_ALWAYS", false);
  }

  unsigned GetExpensiveMemOpts() const {
    return m_pConfigFile->Read<unsigned>("CL_CONFIG_CPU_EXPENSIVE_MEM_OPT", 0);
  }

  int GetSubGroupConstructionMode() const {
    return m_pConfigFile->Read<int>("CL_CONFIG_CPU_SUB_GROUP_CONSTRUCTION", 0);
  }

protected:
  ConfigFile *m_pConfigFile;

  cl_ulong ParseStringToSize(const std::string &userStr) const;
};

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
