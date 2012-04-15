#include <gtest/gtest.h>
#include <plugin_manager.h>
#include <compile_data.h>
#include <link_data.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <cstdlib>
#endif


using namespace Intel::OpenCL;
using namespace Intel::OpenCL::Frontend;

struct TestData : CompileData, LinkData{
};

TEST(FEPluginTest, sanity){
#if defined(_WIN32)
  SetEnvironmentVariable("OCLBACKEND_PLUGINS", "FePluginMock.dll");
#else
  setenv("OCLBACKEND_PLUGINS", "libFePluginMock.so", 1);
#endif
  PluginManager::Init();
  PluginManager& manager = PluginManager::Instance();
  TestData* data = new TestData();
  manager.OnLink(data);
  manager.OnCompile(data);
  //the pluging should set the following fields:
  const SourceFile& file = data->sourceFile();
  ASSERT_STREQ("my name is", file.getName().c_str());
  ASSERT_STREQ("slim shady", file.getContents().c_str());
  delete data;
  PluginManager::Terminate();
}

