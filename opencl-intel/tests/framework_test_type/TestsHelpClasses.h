#pragma once
/*
 * This file holds all of the functions/classes and macros that can help you
 * make a better,easier tests.
 *
 */
#include "CL/cl.h"
#include "FrameworkTest.h"
#include "gtest_wrapper.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

// used to make a macro definition into a std::string simply call XSTR with you
// defined macro "s"
#define XSTR(s) STR(s)
#define STR(s) #s

#define MAX_SOURCE_SIZE 100000 //(0x100000)
#define WORK_SIZE 400

// just to help eliminate some DERY boilerplate code
#define NullCheckAndExecute(check, command)                                    \
  if ((check) != NULL)                                                         \
  command

//================ helper functions ===================================
/*
 * opens a file using fopen like function, works on windows an linux
 * filename - the name of the file
 * optns - options (see fopen)
 * file - the pointer returned with the file
 * return an int with the error type of the problem, 0 means success
 */
int crossPlatformFOpen(char *fileName, const char *optns, FILE *file);

#define FOPEN_OPTIONS                                                          \
  rbD // read,binary,Temporary , remove D so binary file will not be deleted

/*
 * return true if the file exists or false otherwise
 */
inline bool checkFileExistence(char *fileName);
/*
 * check if 2 text files are equals or not, if so google test will fail in this
 *function and will produce the 2 different lines (in case you asked for equal
 *  fileName1,fileName2 - the files names to be checked
 *  isEqual - true if you want to check they are equal and false if you want
 *to check they are different
 */
void validateEqualityOfFiles(std::string fileName1, std::string fileName2,
                             bool isEqual, int linesToSkip);

void validateSubstringInFile(std::string fileName, std::string subString,
                             bool doesExist);
/*
 * just delete all the files you give him, if they exists
 * files[] - all the file names to be removed
 * num - the length of the array.
 */
void removeFiles(std::string files[], int num);

/*
 * google test base class for OpenCL framework tests, just inherit from him and
 * use TEST_F this class does all the boilerplate code of openCL include
 * creating all of the base object
 */
class baseOcl : public testing::Test {
public:
  baseOcl();
  ~baseOcl();
  /*
   * default SetUpTestCase Procedure, Override for other purposes
   * the default one initialize the basic objects, and they are shared for all
   * the test suite for time saving
   */
  static void SetUpTestCase();
  static void TearDownTestCase();
  void TearDown();

  /*
   * initialize the Platform object, used by the default set up
   */
  static void initPlatform();
  /*
   * initialize the Device object, used by the default set up
   */
  static void initDevices();
  /*
   * initialize the Context object, used by the default set up
   */
  static void initContext();
  /*
   * initialize the Command Queue object, used by the default set up
   */
  static void initCommandQueue();
  // static field because they are shared for all the test suite
  static cl_platform_id platform;
  static cl_device_id device;
  static cl_context context;
  static cl_command_queue cmd_queue;

protected:
  // Create a program from the kernelCode
  void simpleProgramCreation(const char **kernelCode);
  // just build that program with all possibles devices
  void simpleProgramBuild();
  // Create a kernel from the program, assumes there is only one __kernel
  void simpleKernelCreation();
  cl_kernel kernel;
  cl_program program;
};

//================ Google test helpers ===================================
// googleTest specialized assertions
#define EXPECT_OCL_EQ(actual, expected, function_name)                         \
  EXPECT_EQ(oclErr(actual), oclErr(expected)) << ERR_FUNCTION(#function_name)
#define ASSERT_OCL_EQ(actual, expected, function_name)                         \
  ASSERT_EQ(oclErr(actual), oclErr(expected)) << ERR_FUNCTION(#function_name)
#define EXPECT_OCL_SUCCESS(actual, function_name)                              \
  EXPECT_OCL_EQ(actual, CL_SUCCESS, function_name)
#define ASSERT_OCL_SUCCESS(actual, function_name)                              \
  ASSERT_OCL_EQ(actual, CL_SUCCESS, function_name)
/*#define ASSERT_EQ(rowPitch , getRowPitch()) <<
 * ERR_FUNCTION("clEnqueueMapImage") << ERR_IN_LOOP(index);*/

#define ERROR_RESET 1
// for better error messages
#define ERR_FUNCTION(message) "while executing: " << (message)
#define ERR_IN_LOOP(i) " in loop with index: " << (i);
/*
 * this class help produce better error messages, output the acutal names of the
 * error instead of thier serial number
 */
class oclErr {
public:
  oclErr(cl_err_code errCode);

  std::string gerErrString() const;
  bool operator==(const oclErr &other) const;
  bool operator!=(const oclErr &other) const { return !operator==(other); }

private:
  cl_err_code errCode;
  std::string err;
};

::std::ostream &operator<<(::std::ostream &os, const oclErr &OclErr);

class Ocl2DImage {
public:
  Ocl2DImage(size_t size_, bool Random = false);
  Ocl2DImage(const Ocl2DImage &copy);
  // just wrap the data
  Ocl2DImage(cl_uchar *pImage_, size_t Size_) : pImage(pImage_), Size(Size_) {}

  Ocl2DImage(int numOfImages, const Ocl2DImage &copy);

  void init(size_t size_, bool Random);
  ~Ocl2DImage(void);
  Ocl2DImage &operator=(const Ocl2DImage &other);
  bool operator==(const Ocl2DImage &other) const;
  bool operator!=(const Ocl2DImage &other) const { return !(*this == other); }
  inline size_t size() const { return Size; }

  operator cl_uchar *() const { return pImage; }
  operator void *() const { return pImage; }

private:
  cl_uchar *pImage;
  size_t Size;
};

//================ C++ wrappers for all clObjects
//===================================
// this are wrappers taken from typeWrappers.h in conformance ,
// they are copied because I don't won't a dependency to this files.
// so this wrappers might defer slightly from the original ones
namespace wrapper_details {

// clRetain*() and clRelease*() functions share the same type.
template <typename T> // T should be cl_context, cl_program, ...
using RetainReleaseType = cl_int CL_API_CALL(T);

// A generic wrapper class that follows OpenCL retain/release semantics.
//
// This Wrapper class implement copy and move semantics, which makes it
// compatible with standard containers for example.
//
// Template parameters:
//  - T is the cl_* type (e.g. cl_context, cl_program, ...)
//  - Retain is the clRetain* function (e.g. clRetainContext, ...)
//  - Release is the clRelease* function (e.g. clReleaseContext, ...)
template <typename T, RetainReleaseType<T> Retain, RetainReleaseType<T> Release>
class Wrapper {
  static_assert(std::is_pointer<T>::value, "T should be a pointer type.");
  T object = nullptr;

  void retain() {
    if (!object)
      return;

    auto err = Retain(object);
    ASSERT_EQ(err, CL_SUCCESS) << "clRetain*() failed";
  }

  void release() {
    if (!object)
      return;

    auto err = Release(object);
    ASSERT_EQ(err, CL_SUCCESS) << "clRelease*() failed";
  }

public:
  Wrapper() = default;

  // On initialisation, assume the object has a refcount of one.
  Wrapper(T object) : object(object) {}

  // On assignment, assume the object has a refcount of one.
  Wrapper &operator=(T rhs) {
    reset(rhs);
    return *this;
  }

  // Copy semantics, increase retain count.
  Wrapper(Wrapper const &w) { *this = w; }
  Wrapper &operator=(Wrapper const &w) {
    reset(w.object);
    retain();
    return *this;
  }

  // Move semantics, directly take ownership.
  Wrapper(Wrapper &&w) { *this = std::move(w); }
  Wrapper &operator=(Wrapper &&w) {
    reset(w.object);
    w.object = nullptr;
    return *this;
  }

  ~Wrapper() { reset(); }

  // Release the existing object, if any, and own the new one, if any.
  void reset(T new_object = nullptr) {
    release();
    object = new_object;
  }

  operator T() const { return object; }

  // Ideally this function should not exist as it breaks encapsulation by
  // allowing external mutation of the Wrapper internal state. However, too
  // much code currently relies on this. For example, instead of using T* as
  // output parameters, existing code can be updated to use Wrapper& instead.
  T *operator&() { return &object; }
};

} // namespace wrapper_details

using clContextWrapper =
    wrapper_details::Wrapper<cl_context, clRetainContext, clReleaseContext>;

using clProgramWrapper =
    wrapper_details::Wrapper<cl_program, clRetainProgram, clReleaseProgram>;

using clKernelWrapper =
    wrapper_details::Wrapper<cl_kernel, clRetainKernel, clReleaseKernel>;

using clMemWrapper =
    wrapper_details::Wrapper<cl_mem, clRetainMemObject, clReleaseMemObject>;

using clCommandQueueWrapper =
    wrapper_details::Wrapper<cl_command_queue, clRetainCommandQueue,
                             clReleaseCommandQueue>;

using clSamplerWrapper =
    wrapper_details::Wrapper<cl_sampler, clRetainSampler, clReleaseSampler>;

using clEventWrapper =
    wrapper_details::Wrapper<cl_event, clRetainEvent, clReleaseEvent>;