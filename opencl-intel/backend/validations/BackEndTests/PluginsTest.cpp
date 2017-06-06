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
#include "plugin_manager.h"

#include <gtest/gtest.h>
#include <assert.h>
#include <stdio.h>
#include <string>

#ifdef WIN32
#include <direct.h>
#include <Windows.h>
#define GetCurrentDir _getcwd    // used for setting the environment variable
#else
#include <stdlib.h>
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

TEST_F(BackEndTests_Plugins, PluginLoadSuccess)
{
    // define the environment variable that will contain the path to the plugin dll
    char currentPath[FILENAME_MAX];
    ASSERT_TRUE(GetCurrentDir(currentPath, sizeof(currentPath)));
    std::string envString("");
#ifdef WIN32
    // envString = "currentpath\PLUGIN_DLL_NAME"
    envString = envString + currentPath + "\\" + PLUGIN_DLL_NAME;
    ASSERT_TRUE(SetEnvironmentVariableA(PLUGIN_ENVIRONMENT_VAR, &(envString[0])));
#else
    // envString = "environmentname=currentpath/PLUGIN_DLL_NAME"
    envString = envString + PLUGIN_ENVIRONMENT_VAR + "=" + currentPath + "/" + PLUGIN_DLL_NAME;
    ASSERT_EQ(putenv(&(envString[0])), 0);
#endif

    // load the plugin dll and get the exported function
    try
    {
        m_dll.Load(PLUGIN_DLL_NAME);
    }catch(Exceptions::DynamicLibException& )
    {
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

    // now, plugin's method OnCreateProgram should have changed the 'pluginWorked' flag
    // check if the flag really changed - should be true
    ASSERT_TRUE(getFlag());
}

TEST_F(BackEndTests_Plugins, PluginLoadWrongPath)
{
    // define the environment variable that will contain the WRONG path to the plugin dll
    char currentPath[FILENAME_MAX];
    ASSERT_TRUE(GetCurrentDir(currentPath, sizeof(currentPath)));

    std::string envString("");
#ifdef WIN32
    // envString = "fakepath\PLUGIN_DLL_NAME"
    envString = envString + currentPath + "\\fakepathblabla\\" + PLUGIN_DLL_NAME;
    ASSERT_TRUE(SetEnvironmentVariableA(PLUGIN_ENVIRONMENT_VAR, &(envString[0])));
#else
    // envString = "environmentname=fakepath/PLUGIN_DLL_NAME"
    envString = envString + PLUGIN_ENVIRONMENT_VAR + "=" + currentPath + "/fakepathblabla/" + PLUGIN_DLL_NAME;
    ASSERT_EQ(putenv(&(envString[0])), 0);
#endif

    try{
      // PluginManager is initialized in lazy fashion - so need to generate even to trigger the initialization
      Intel::OpenCL::PluginManager manager;
      CreateTestEvent(&manager);
    } catch (Intel::OpenCL::PluginManagerException e){
      return;
    }
    FAIL() << "exception was expected";
}

TEST_F(BackEndTests_Plugins, PluginLoadEmptyPath)
{
    // define the environment variable and leave it EMPTY
    std::string envString("");
#if defined(_WIN32)
    // WORKAROUND: set environment variable to ' ' and then to empty string ( maybe this is unsafe, remove??)
    // envString = "environmentname= "
    envString = envString + PLUGIN_ENVIRONMENT_VAR + "= ";

    ASSERT_EQ(putenv(&(envString[0])), 0);
    char* env = getenv(PLUGIN_ENVIRONMENT_VAR);
    ASSERT_TRUE(env);
    ASSERT_TRUE(STRING_EQ(" ", env));
    // setting the environment variable to empty string
    env[0]='\0';
    // reflecting also env change at OS level
    ASSERT_TRUE(SetEnvironmentVariableA(PLUGIN_ENVIRONMENT_VAR, env));
#else
    envString = envString + PLUGIN_ENVIRONMENT_VAR + "=";
    ASSERT_EQ(putenv(&(envString[0])), 0);
#endif
    // make sure the environment variable is defined but empty
    char* env2 = getenv(PLUGIN_ENVIRONMENT_VAR);
    ASSERT_TRUE(env2);
    ASSERT_TRUE(STRING_EQ("", env2));

    // init the backend - should success
    Intel::OpenCL::PluginManager manager;
    CreateTestEvent(&manager);
}

TEST_F(BackEndTests_Plugins, PluginLoadSuccess2)
{
    // ensure the environment variable is not defined
#if defined(_WIN32)
    std::string envString("");
    // envString = "environmentname="
    envString = envString + PLUGIN_ENVIRONMENT_VAR + "=";
    ASSERT_EQ(putenv(&(envString[0])), 0);
    // reflecting also env change at OS level
    ASSERT_TRUE(SetEnvironmentVariableA(PLUGIN_ENVIRONMENT_VAR, NULL));
#else
    unsetenv(PLUGIN_ENVIRONMENT_VAR);
#endif

    char* env = getenv(PLUGIN_ENVIRONMENT_VAR);
    ASSERT_FALSE(env);

    // init the backend - should success
    Intel::OpenCL::PluginManager manager;
    CreateTestEvent(&manager);
}