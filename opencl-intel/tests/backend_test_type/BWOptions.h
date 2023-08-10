/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BWOptions.h

\*****************************************************************************/
#ifndef BW_OPTIONS_H
#define BW_OPTIONS_H
/** @brief the implemented Options classes used in BackEnd tests
 *
 */

#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include <assert.h>

#define PAGE_SIZE 4096

// FIXME 'using namespace' shouldn't be used in header file.
using namespace Intel::OpenCL::DeviceBackend;

/** @brief CompilationServiceOptions: used when calling GetCompilationService
 *        the options consist of:
 *        - the CPU architecture selection
 *        - the CPU features to enable/disable
 *        - the transpose size
 *        - force enable/disable the use of VTune
 */
class CompilationServiceOptions : public ICLDevBackendOptions {
public:
  /// @brief initiate the options from a given set
  void InitFromTestConfiguration(const DeviceMode device,
                                 const std::string subdevice,
                                 const std::string subdeviceFeatures,
                                 const ETransposeSize transposeSize,
                                 const bool useVTune) {
    m_device = device;
    m_subdevice = subdevice;
    m_subdeviceFeatures = subdeviceFeatures;
    m_transposeSize = transposeSize;
    m_useVTune = useVTune;
  }

  /// @brief implementing the interface's method, added the VTune option
  virtual bool GetBooleanValue(int optionId, bool defaultValue) const {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_USE_VTUNE:
      return m_useVTune;
    default:
      return defaultValue;
    }
  }

  /// @brief implementing the interface's method, added the TransposeSize option
  virtual int GetIntValue(int optionId, int defaultValue) const {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_DEVICE:
      return m_device;
    case CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE:
      return (int)m_transposeSize;
    default:
      return defaultValue;
    }
  }

  /// @brief implementing the interface's method, added the CPU selection & CPU
  /// features option
  virtual const char *GetStringValue(int optionId,
                                     const char *defaultValue) const {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_SUBDEVICE:
      return m_subdevice.c_str();
    case CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES:
      return m_subdeviceFeatures.c_str();
    default:
      return defaultValue;
    }
  }

  /// @brief implementing the interface's method, didn't add an option to return
  virtual bool GetValue(int optionId, void *Value, size_t *pSize) const {
    return false;
  }

private:
  DeviceMode m_device;
  std::string m_subdevice;
  std::string m_subdeviceFeatures;
  ETransposeSize m_transposeSize;
  bool m_useVTune;
};

/** @brief ExecutionServiceOptions: used when calling GetExecutionService
 *        the options consist of:
 *        - the CPU architecture selection
 */
class ExecutionServiceOptions : public ICLDevBackendOptions {
public:
  /// @brief initiate the options from a given set
  void InitFromTestConfiguration(const DeviceMode device,
                                 const std::string subdevice) {
    m_device = device;
    m_subdevice = subdevice;
  }

  /// @brief implementing the interface's method, didn't add an option to return
  bool GetBooleanValue(int optionId, bool defaultValue) const {
    return defaultValue;
  }

  /// @brief implementing the interface's method, didn't add an option to return
  virtual int GetIntValue(int optionId, int defaultValue) const {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_DEVICE:
      return m_device;
    default:
      return defaultValue;
    }
  }

  /// @brief implementing the interface's method, added the CPU selection option
  virtual const char *GetStringValue(int optionId,
                                     const char *defaultValue) const {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_SUBDEVICE:
      return m_subdevice.c_str();
    default:
      return defaultValue;
    }
  }

  /// @brief implementing the interface's method, didn't add an option to return
  virtual bool GetValue(int optionId, void *Value, size_t *pSize) const {
    return false;
  }

private:
  DeviceMode m_device;
  std::string m_subdevice;
};

/** @brief JITAllocator: implementing the jit allocator interface,
 *        this class is not directly relevant to the backend tests,
 *        as its only used in SerializationService option class
 */
class JITAllocator : public ICLDevBackendJITAllocator {
private:
  JITAllocator(){};
  ~JITAllocator(){};

public:
  /// @brief initiate the instance
  static void Init();
  /// @brief terminate the instance
  static void Terminate();

  static JITAllocator *GetInstance();

  void *AllocateExecutable(size_t size, size_t alignment);
  void FreeExecutable(void *ptr);

private:
  static JITAllocator *s_pInstance;
};

/** @brief SerializationServiceOptions: used when calling
 * GetSerializationService the options consist of:
 *        - the CPU architecture selection
 *        - the JIT Allocator pointer
 */
class SerializationServiceOptions : public ICLDevBackendOptions {
public:
  /// @brief initiate the options from a given set
  void InitFromTestConfiguration(const DeviceMode device,
                                 const std::string &subdevice,
                                 JITAllocator *jitAllocator) {
    m_device = device;
    m_subdevice = subdevice;
    m_JITAllocator = jitAllocator;
  }

  /// @brief implementing the interface's method, didn't add an option to return
  bool GetBooleanValue(int optionId, bool defaultValue) const {
    return defaultValue;
  }

  /// @brief implementing the interface's method, didn't add an option to return
  virtual int GetIntValue(int optionId, int defaultValue) const {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_DEVICE:
      return m_device;
    default:
      return defaultValue;
    }
  }

  /// @brief implementing the interface's method, added the CPU selection option
  virtual const char *GetStringValue(int optionId,
                                     const char *defaultValue) const {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_SUBDEVICE:
      return m_subdevice.c_str();
    default:
      return defaultValue;
    }
  }

  /// @brief implementing the interface's method, added the JIT Allocator option
  virtual bool GetValue(int optionId, void *Value, size_t *pSize) const {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_JIT_ALLOCATOR:
      assert(Value);
      *(static_cast<JITAllocator **>(Value)) = m_JITAllocator;
      return true;
    default:
      assert(false && "Unknown option");
      return false;
    }
  }

private:
  DeviceMode m_device;
  std::string m_subdevice;
  JITAllocator *m_JITAllocator;
};

/** @brief ProgramBuilderBuildOptions: used when calling BuildProgram
 *        the options consist of:
 *        - the CPU architecture selection
 */
class ProgramBuilderBuildOptions : public ICLDevBackendOptions {
public:
  /// @brief initiate the options from a given set
  void InitFromTestConfiguration(const std::string cpu) { m_subdevice = cpu; }

  /// @brief implementing the interface's method, didn't add an option to return
  bool GetBooleanValue(int optionId, bool defaultValue) const {
    return defaultValue;
  }

  /// @brief implementing the interface's method, didn't add an option to return
  virtual int GetIntValue(int optionId, int defaultValue) const {
    return defaultValue;
  }

  /// @brief implementing the interface's method, added the CPU selection option
  virtual const char *GetStringValue(int optionId,
                                     const char *defaultValue) const {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_SUBDEVICE:
      return m_subdevice.c_str();
    default:
      return defaultValue;
    }
  }

  /// @brief implementing the interface's method, didn't add an option to return
  virtual bool GetValue(int optionId, void *Value, size_t *pSize) const {
    return false;
  }

private:
  std::string m_subdevice;
};

#endif // BW_OPTIONS_H
