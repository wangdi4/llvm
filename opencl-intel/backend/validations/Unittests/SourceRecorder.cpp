// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "Exception.h"
#include "compile_data.h"
#include "gtest_wrapper.h"
#include "ocl_source_recorder.h"
#include "plugin_manager.h"
#include "source_file.h"
#include <cassert>
#include <exception>
#include <link_data.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <cstdlib>
#endif

#if defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_GETSPECIFIC)
#include <pthread.h>
#endif // defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_GETSPECIFIC)

using namespace Intel::OpenCL;
using namespace Intel::OpenCL::Frontend;
using namespace Validation;

struct TestData : CompileData, LinkData {};

//
// CompileDataFactory
//
class CompileDataFactory {
public:
  CompileDataFactory &withContent(const std::string &contents) {
    m_contents = contents;
    return *this;
  }

  CompileDataFactory &withFileName(const std::string &fileName) {
    m_fileName = fileName;
    return *this;
  }

  CompileDataFactory &withBuffer(const void *buffer, size_t bufferSize) {
    m_buffer = buffer;
    m_bufferSize = bufferSize;
    return *this;
  }

  static CompileDataFactory *init() { return new CompileDataFactory(); }

  static void free(CompileDataFactory *factory) {
    assert("NULL factory" && factory);
    delete factory;
  }

  CompileData *create() const {
    if (m_contents.empty())
      throw Exception::OperationFailed("contents cannot be empty");
    if (m_fileName.empty())
      throw Exception::OperationFailed("file name cannot be empty");
    if (NULL == m_buffer)
      throw Exception::OperationFailed("binary buffer cannot be NULL");
    if (0 == m_bufferSize)
      throw Exception::OperationFailed("buffer size cannot be zero");
    SourceFile sourceFile(m_fileName, m_contents, "");
    BinaryBuffer binaryBuffer;
    binaryBuffer.binary = m_buffer;
    binaryBuffer.size = m_bufferSize;
    sourceFile.setBinaryBuffer(binaryBuffer);
    CompileData *pCompileData = new CompileData();
    pCompileData->sourceFile(sourceFile);
    return pCompileData;
  }

private:
  CompileDataFactory() : m_buffer(NULL), m_bufferSize(0) {}

  std::string m_contents;
  std::string m_fileName;
  const void *m_buffer;
  size_t m_bufferSize;
};

#ifdef OCL_DEV_BACKEND_PLUGINS
//
// Sanity test for the plugin mechanism
//
TEST(FEPluginTest, DISABLED_sanity) {
#if defined(_WIN32)
  SetEnvironmentVariableA("OCLBACKEND_PLUGINS", "FePluginMock.dll");
#else
  setenv("OCLBACKEND_PLUGINS", "libFePluginMock.so", 1);
#endif
  PluginManager pluginManager;
  TestData *data = new TestData();
  pluginManager.OnLink(data);
  pluginManager.OnCompile(data);
  // the pluging should set the following fields:
  const SourceFile &file = data->sourceFile();
  ASSERT_STREQ("my name is", file.getName().c_str());
  ASSERT_STREQ("slim shady", file.getContents().c_str());
  delete data;
}

//
// A basic test for the OclourceRecorder (1.1 capabilities only), which tests
// that a compiled module is 'connected' to itself.
//
TEST(OCLSourceRecorder1_1, DISABLED_sorce_recorder_basic) {
  const char *kernelName = "k.cl";
  ;
  const char *kernelContents =
      "__kernel void a(__global char* a, __global char* b){"
      " int gid = get_global_id(0); b[gid] = a[gid];}";
  unsigned char buffer[] = {0xde, 0xad, 0xbe, 0xef};
  CompileDataFactory *pFactory = CompileDataFactory::init();
  CompileData *pCompileData = pFactory->withFileName(kernelName)
                                  .withContent(kernelContents)
                                  .withBuffer(buffer, sizeof(buffer))
                                  .create();
  MD5 md5(buffer, 4);
  MD5Code md5Code = md5.digest();
  OclSourceRecorder oclSourceRecorder;
  oclSourceRecorder.OnCompile(pCompileData);
  FileIter fileIter = oclSourceRecorder.begin(md5Code);
  ASSERT_STREQ(kernelName, (*fileIter).getName().c_str());
  ASSERT_STREQ(kernelContents, (*fileIter).getContents().c_str());
  delete pCompileData;
  CompileDataFactory::free(pFactory);
}

//
// File iterator test
//
TEST(OCLSourceRecorder1_1, DISABLED_iterator_test) {
  const char *kernelName = "k.cl";
  ;
  const char *kernelContents =
      "__kernel void a(__global char* a, __global char* b){"
      " int gid = get_global_id(0); b[gid] = a[gid];}";
  unsigned char buffer[] = {0xde, 0xad, 0xbe, 0xef};
  CompileDataFactory *pFactory = CompileDataFactory::init();
  CompileData *pCompileData = pFactory->withFileName(kernelName)
                                  .withContent(kernelContents)
                                  .withBuffer(buffer, sizeof(buffer))
                                  .create();
  MD5 md5(buffer, 4);
  MD5Code md5Code = md5.digest();
  OclSourceRecorder oclSourceRecorder;
  oclSourceRecorder.OnCompile(pCompileData);
  FileIter fileIter = oclSourceRecorder.begin(md5Code);
  FileIter endIter1 = oclSourceRecorder.end();
  FileIter endIter2 = oclSourceRecorder.end();
  ASSERT_TRUE(fileIter == fileIter);
  ASSERT_TRUE(fileIter != endIter1);
  ASSERT_TRUE(endIter1 != fileIter);
  ASSERT_TRUE(endIter1 == endIter1);
  ASSERT_TRUE(endIter1 == endIter2);
  FileIter copyIter = fileIter;
  ASSERT_TRUE(copyIter == fileIter);
  fileIter++;
  ASSERT_TRUE(fileIter == endIter1);
}

//
// A test for the thread safety of the ocl recorder
//
#ifndef _WIN32

#define UNUSED(x) x = x

// Event class (ManualResetEvent equivalent)
class Event {
public:
  // params:
  //   signaled: true to set the initial state of the event as signaled.
  //   false otherwise.
  Event(bool signaled = true) : m_signal(signaled) {
    pthread_mutex_init(&m_mutex, 0);
    pthread_cond_init(&m_condition, 0);
  }

  ~Event() {
    pthread_cond_destroy(&m_condition);
    pthread_mutex_destroy(&m_mutex);
  }

  // hangs until some thread calls the 'signal'
  void wait() {
    pthread_mutex_lock(&m_mutex);
    while (!m_signal)
      pthread_cond_wait(&m_condition, &m_mutex);
    pthread_mutex_unlock(&m_mutex);
  }

  // sets the state of the event to unsignaled
  void reset() {
    pthread_mutex_lock(&m_mutex);
    m_signal = false;
    pthread_mutex_unlock(&m_mutex);
  }

  // sets the states of the event to signled
  void signal() {
    pthread_mutex_lock(&m_mutex);
    m_signal = true;
    pthread_cond_signal(&m_condition);
    pthread_mutex_unlock(&m_mutex);
  }

private:
  bool m_signal;
  pthread_mutex_t m_mutex;
  pthread_cond_t m_condition;
};

struct ThreadArg {
  OclSourceRecorder *pRecorder;
  CompileData *pCompileData;
  Event *pEvent;
};

static void *addCompile(void *args) {
  ThreadArg *parg = (ThreadArg *)args;
  parg->pEvent->wait();
  parg->pRecorder->OnCompile(parg->pCompileData);
  return NULL;
}

TEST(OCLSourceRecorder1_1, DISABLED_source_recoder_thread_safety) {
  const int THREAD_NUM = 2;
  CompileData *compileData[THREAD_NUM];
  unsigned char b1[16] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                          0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
  unsigned char b2[16] = {0x0, 0x3, 0x5, 0x13, 0x3,  0x3, 0x8,  0x0,
                          0x4, 0xc, 0xb, 0x9,  0x12, 0x9, 0x12, 0xc};
  // compileData initialization
  //
  // 1st buffer
  CompileDataFactory *pFactory = CompileDataFactory::init();
  compileData[0] = pFactory->withFileName("abc")
                       .withContent("the brown fox")
                       .withBuffer(b1, sizeof(b1))
                       .create();
  //
  // 2nd buffer
  compileData[1] = pFactory->withFileName("def")
                       .withContent("jumped over the wall")
                       .withBuffer(b2, sizeof(b2))
                       .create();
  // thread initialization
  pthread_t thread[THREAD_NUM];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  OclSourceRecorder oclSourceRecorder;
  Event event(false); // sets the initial state to false
  ThreadArg threadArgs[THREAD_NUM];
  for (int i = 0; i < THREAD_NUM; i++) {
    threadArgs[i].pRecorder = &oclSourceRecorder;
    threadArgs[i].pCompileData = compileData[i];
    threadArgs[i].pEvent = &event;
    int rc = pthread_create(&thread[i], &attr, addCompile, &threadArgs[i]);
#ifdef NDEBUG
    UNUSED(rc);
#else
    assert("thread creation error?!" && 0 == rc);
#endif
  }
  // go go threads!
  event.signal();
  // waiting for threads to end
  void *status;
  pthread_attr_destroy(&attr);
  for (int i = 0; i < THREAD_NUM; i++) {
    int rc = pthread_join(thread[i], &status);
#ifdef NDEBUG
    UNUSED(rc);
#else
    assert(0 == rc && "thread join failed?!");
#endif
  }
  // checking the result
  MD5 md5(b1, sizeof(b1));
  MD5Code md5Code = md5.digest();
  FileIter iter = oclSourceRecorder.begin(md5Code);
  ASSERT_STREQ("abc", (*iter).getName().c_str());
  ASSERT_STREQ("the brown fox", (*iter).getContents().c_str());
  // cleanup
  for (int i = 0; i < THREAD_NUM; i++)
    delete compileData[i];
  delete pFactory;
}
#endif // #if _WIN32
//
// Checks the dependency between a source file and its headers
//
TEST(OCLSourceRecorder1_1, DISABLED_header_source_connection) {
  const char *contents = "veni vidi vichi";
  const char *h1 = "h1";
  const char *h2 = "h2";
  const char *s = "s";
  unsigned char b1[2] = {0x0, 0x0};
  unsigned char b2[2] = {0x1, 0x2};
  unsigned char b3[2] = {0x3, 0x4};
  CompileData *psource;
  SourceFile ph1, ph2;
  // Calculating MD5 code of b3 (which belongs to source)
  MD5 md5(b3, sizeof(b3));
  MD5Code code = md5.digest();
  CompileDataFactory *pFactory = CompileDataFactory::init();
  //
  // header 1:
  //
  ph1 = SourceFile(h1, contents, "");
  ph1.setBinaryBuffer(BinaryBuffer((const void *)b1, sizeof(b1)));
  //
  // header 2:
  //
  ph2 = SourceFile(h2, contents, "");
  ph2.setBinaryBuffer(BinaryBuffer((const void *)b2, sizeof(b2)));
  //
  // source: assigning file name and binary buffer
  //
  psource = pFactory->withFileName(s)
                .withContent(contents)
                .withBuffer(b3, sizeof(b3))
                .create();
  // include headers
  psource->addIncludeFile(ph1);
  psource->addIncludeFile(ph2);
  //
  // checking the expected dependency:  source -> h1 and source ->h2
  //
  OclSourceRecorder oclSourceRecorder;
  oclSourceRecorder.OnCompile(psource);
  FileIter iter = oclSourceRecorder.begin(code);
  FileIter endIter = oclSourceRecorder.end();
  std::map<std::string, bool> hitmap;
  hitmap[h1] = false;
  hitmap[h2] = false;
  hitmap[s] = false;
  while (iter != endIter) {
    std::string sname = (*iter).getName();
    ASSERT_FALSE(hitmap[sname]);
    hitmap[sname] = true;
    iter++;
  }
  std::map<std::string, bool>::const_iterator mapiter = hitmap.begin(),
                                              end = hitmap.end();
  while (mapiter != end) {
    ASSERT_TRUE(mapiter->second);
    mapiter++;
  }
  // cleanup
  delete psource;

  CompileDataFactory::free(pFactory);
}

TEST(OCLSourceRecorder1_1, DISABLED_referenceContension) {
#if defined(_WIN32)
  SetEnvironmentVariableA("OCLBACKEND_PLUGINS", "OclRecorder.dll");
#else
  setenv("OCLBACKEND_PLUGINS", "libOclRecorder.so", 1);
#endif
  PluginManager pluginManager;
  // compile data
  unsigned char b[2] = {0x0, 0x0};
  CompileDataFactory *pFactory = CompileDataFactory::init();
  CompileData *pCompileData =
      pFactory->withFileName("test123.cl")
          .withContent(
              "__kernel void(__global int* a){ printf(\"hello\\n\"); }")
          .withBuffer(b, sizeof(b))
          .create();
  {
    // wrapped in a new scope, we make sure the destruction of this object does
    // not interfere with the other one
    PluginManager manager2;
    manager2.OnCompile(pCompileData);
  }
  pluginManager.OnCompile(pCompileData);
  // cleanup
  delete pCompileData;
  CompileDataFactory::free(pFactory);
}

#endif // OCL_DEV_BACKEND_PLUGINS
