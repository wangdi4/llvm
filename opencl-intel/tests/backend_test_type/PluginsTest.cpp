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

File Name:  PluginsTest.cpp

\*****************************************************************************/

#define _POSIX_SOURCE

#include "BackendWrapper.h"
#include "cl_env.h"
#include "common_utils.h"
#include "plugin_manager.h"
#include "gtest_wrapper.h"
#include <assert.h>
#include <stdio.h>
#include <string>

TEST_F(BackEndTests_Plugins, PluginLoadSuccess) {
  // define the environment variable that will contain the path
  // to the plugin dll
  const char *pluginPath = PLUGIN_DLL_NAME;
  ASSERT_TRUE(SETENV(PLUGIN_ENVIRONMENT_VAR, pluginPath));

  // load the plugin dll and get the exported function
  try {
    m_dll.Load(pluginPath);
  } catch (Exceptions::DynamicLibException &) {
    FAIL() << "Cannot load the plugin dll file.\n";
  }
  getFlag = (PLUGIN_EXPORT_F)(intptr_t)m_dll.GetFuncPtr("getTheFlag");
  ASSERT_TRUE(getFlag);

  //-----------------------------------------------------------------
  // check the 'pluginWorked' flag - should be false
  ASSERT_FALSE(getFlag());

  // To detect that the plugin is loaded, we can create an event that will
  // make the plugin "do something", the event CreateProgram, will lead to
  // the plugin's method OnCreateProgram which will change the flag
  Intel::OpenCL::PluginManager manager;
  CreateTestEvent(&manager);

  // now, plugin's method OnCreateProgram should have changed the 'pluginWorked'
  // flag check if the flag really changed - should be true
  ASSERT_TRUE(getFlag());
}

TEST_F(BackEndTests_Plugins, PluginLoadWrongPath) {
  // define the environment variable that will contain the WRONG path to the
  // plugin dll
  std::string envString("");
#ifdef _WIN32
  // envString = "fakepath\PLUGIN_DLL_NAME"
  envString = get_exe_dir() + "fakepathblabla\\" + PLUGIN_DLL_NAME;
  ASSERT_TRUE(SetEnvironmentVariableA(PLUGIN_ENVIRONMENT_VAR, &(envString[0])));
#else
  // envString = "environmentname=fakepath/PLUGIN_DLL_NAME"
  envString = envString + PLUGIN_ENVIRONMENT_VAR + "=" + get_exe_dir() +
              "fakepathblabla/" + PLUGIN_DLL_NAME;
  ASSERT_EQ(putenv(&(envString[0])), 0);
#endif

  try {
    // PluginManager is initialized in lazy fashion - so need to generate even
    // to trigger the initialization
    Intel::OpenCL::PluginManager manager;
    CreateTestEvent(&manager);
  } catch (Intel::OpenCL::PluginManagerException e) {
    return;
  }
  FAIL() << "exception was expected";
}

TEST_F(BackEndTests_Plugins, PluginLoadEmptyPath) {
  // define the environment variable and leave it EMPTY
  ASSERT_TRUE(SETENV(PLUGIN_ENVIRONMENT_VAR, ""));
  // make sure the environment variable is defined but empty
  std::string Env;
  ASSERT_TRUE(Intel::OpenCL::Utils::getEnvVar(Env, PLUGIN_ENVIRONMENT_VAR));
  ASSERT_TRUE(STRING_EQ("", Env));

  // init the backend - should success
  Intel::OpenCL::PluginManager manager;
  CreateTestEvent(&manager);
}

TEST_F(BackEndTests_Plugins, PluginLoadSuccess2) {
  // ensure the environment variable is not defined
  ASSERT_TRUE(UNSETENV(PLUGIN_ENVIRONMENT_VAR));

  std::string Env;
  ASSERT_FALSE(Intel::OpenCL::Utils::getEnvVar(Env, PLUGIN_ENVIRONMENT_VAR));

  // init the backend - should success
  Intel::OpenCL::PluginManager manager;
  CreateTestEvent(&manager);
}
