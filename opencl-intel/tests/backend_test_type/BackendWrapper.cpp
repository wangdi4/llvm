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

File Name:  BackendWrapper.cpp

\*****************************************************************************/
#include "BackendWrapper.h"
#include "common_utils.h"
#include "plugin_manager.h"
#include <assert.h>
#include <iterator>

BackendWrapper::BackendWrapper(void)
    : m_funcInit(NULL), m_funcTerminate(NULL), m_funcGetFactory(NULL) {
  backendInitiated = false;
}

BackendWrapper::~BackendWrapper(void) {}

void BackendWrapper::SetUp() {
  if (!(BackendWrapper::Initiate())) {
    FAIL() << "Error initiating BackendWrapper.\n";
  }
}

void BackendWrapper::TearDown() {
  // terminate the jit allocator if it was initiated
  JITAllocator::Terminate();
  // terminate the Backend Wrapper instance
  if (!(BackendWrapper::Terminate())) {
    FAIL() << "Error terminating BackendWrapper.\n";
  }
}

BackendWrapper *BackendWrapper::s_instance = NULL;

bool BackendWrapper::Initiate(bool callBackendInit) {
  if (s_instance) {
    return false;
  }

  s_instance = new BackendWrapper();
  // should we init the device backend here?
  if (callBackendInit) {
    if (s_instance->LoadDll() && CL_DEV_SUCCESS == (s_instance->Init(NULL))) {
      return true;
    }
  } else {
    // dont call the Backend exported function InitDeviceBackend
    if (s_instance->LoadDll()) {
      return true;
    }
  }
  // Initiating failed, return to safe state
  delete s_instance;
  s_instance = NULL;
  return false;
}

BackendWrapper &BackendWrapper::GetInstance() {
  assert(s_instance);
  return *s_instance;
}

bool BackendWrapper::LoadDll() {
  // load the dll and get the exported functions
  try {
    m_dll.Load(CPUBACKEND_DLL_NAME);

  } catch (Exceptions::DynamicLibException &err) {
    std::cerr << "Cannot load the backend DLL. " << err.what() << "\n";
    return false;
  }

  m_funcInit =
      (BACKEND_INIT_FUNCPTR)(intptr_t)m_dll.GetFuncPtr("InitDeviceBackend");
  m_funcTerminate = (BACKEND_TERMINATE_FUNCPTR)(intptr_t)m_dll.GetFuncPtr(
      "TerminateDeviceBackend");
  m_funcGetFactory = (BACKEND_GETFACTORY_FUNCPTR)(intptr_t)m_dll.GetFuncPtr(
      "GetDeviceBackendFactory");
  return true;
}

bool BackendWrapper::Terminate() {
  if (s_instance) {
    s_instance->Release();
    delete s_instance;
    s_instance = NULL;
    return true;
  }
  return false;
}

cl_dev_err_code
BackendWrapper::Init(const ICLDevBackendOptions *pBackendOptions) {
  assert(m_funcInit);
  // call the backend dll exported function Init
  cl_dev_err_code ret = m_funcInit(pBackendOptions);
  backendInitiated = true;
  return ret;
}

void BackendWrapper::Release() {
  assert(m_funcTerminate);
  // call the backend dll exported function Terminate
  if (backendInitiated) {
    m_funcTerminate();
    backendInitiated = false;
  }
}

ICLDevBackendServiceFactory *BackendWrapper::GetBackendServiceFactory() {
  assert(m_funcGetFactory);
  // call the backend dll exported function GetFactory
  return m_funcGetFactory();
}

void BackendWrapper::CreateProgramContainer(const std::string &programFile,
                                            std::vector<char> &buffer) {
  // open the bitcode file
  std::ifstream testFile(programFile.c_str(), std::ios::binary);
  if (!testFile.is_open()) {
    FAIL() << "Could not open program file: " << programFile << "\n";
  }
  buffer.assign(std::istreambuf_iterator<char>(testFile),
                std::istreambuf_iterator<char>());
}

void BackEndTests_Plugins::SetUp() {
  // IMPORTANT! dont init the backend here
  // need to set the environment variable in the test first

  // init BackendWrapper but dont call InitDeviceBackend
  if (!(BackendWrapper::Initiate(false))) {
    FAIL() << "Error initiating BackendWrapper.\n";
  }
  // initiate the exported function pointer
  getFlag = NULL;
}

/** @brief creating the event for testing the plugin flag (see SamplePlugin)
 *        in our case the event is CreateProgram
 *        this method is called in the plugins test body
 */
void BackEndTests_Plugins::CreateTestEvent() {
  ASSERT_TRUE(s_instance);
  // get Backend service factory
  ICLDevBackendServiceFactory *funcGetFactory =
      BackendWrapper::GetInstance().GetBackendServiceFactory();
  ;
  ASSERT_TRUE(funcGetFactory);

  // create a Compilation service
  ICLDevBackendCompileServicePtr spCompileService(NULL);
  ASSERT_TRUE(spCompileService.get() == NULL);
  cl_dev_err_code ret =
      funcGetFactory->GetCompilationService(NULL, spCompileService.getOutPtr());
  EXPECT_EQ(CL_DEV_SUCCESS, ret);
  ICLDevBackendCompilationService *pCompileService = spCompileService.get();
  ASSERT_TRUE(pCompileService);

  // load pre created bitcode buffer in correct format - with kernels
  std::unique_ptr<BackendWrapper> pBackendWrapper(new BackendWrapper());
  ASSERT_TRUE(pBackendWrapper.get());
  std::vector<char> program;
  pBackendWrapper->CreateProgramContainer(get_exe_dir() + FILE_BC_WITH_KERNELS,
                                          program);
  ASSERT_TRUE(program.size() > 0);

  // create the program with valid parameters - should success
  ICLDevBackendProgram_ *pProgram = NULL;
  ret =
      pCompileService->CreateProgram(program.data(), program.size(), &pProgram);
  EXPECT_EQ(CL_DEV_SUCCESS, ret);
}

/** @brief creating the event for testing the plugin flag (see SamplePlugin)
 *        in our case the event is CreateProgram
 *        this method is called in the plugins test body
 */
void BackEndTests_Plugins::CreateTestEvent(
    Intel::OpenCL::PluginManager *pManager) {
  // load pre created bitcode buffer in correct format - with kernels
  std::unique_ptr<BackendWrapper> pBackendWrapper(new BackendWrapper());
  ASSERT_TRUE(pBackendWrapper.get());
  std::vector<char> program;
  pBackendWrapper->CreateProgramContainer(get_exe_dir() + FILE_BC_WITH_KERNELS,
                                          program);
  ASSERT_TRUE(program.size() > 0);

  // create the program with valid parameters - should success
  std::unique_ptr<ICLDevBackendProgram_> pProgram(new OCLProgramMock());
  pManager->OnCreateProgram(program.data(), program.size(), pProgram.get());
}
