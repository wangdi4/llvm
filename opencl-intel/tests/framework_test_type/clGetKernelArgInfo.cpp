#include "CL/cl.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "cl_types.h"
#include "common_utils.h"
#include <stdio.h>

/*******************************************************************************
 * clGetKernelArgInfoTest
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create programs with source
 * (4) build programs
 * (5) query and test kernel arg info
 *******************************************************************************/

extern cl_device_type gDeviceType;

bool clGetKernelArgInfoTest() {
  bool bResult = true;

  // kernel 3 is used to check valid address qualifier
  const char *ocl_test_program[] = {
      "__kernel void test_kernel1(__global char16 pBuff0[], __global char*"
      " pBuff1, __global const char* pBuff2, image2d_t __read_only test_image,"
      "image2d_t __read_write test_image1, image2d_t __write_only test_image2)"
      " __attribute__((vec_type_hint(uint8)))"
      " __attribute__((reqd_work_group_size(8,8,8)))\n"
      "{\n"
      "    size_t id = get_global_id(0);\n"
      "    pBuff0[id] = pBuff1[id] ? pBuff0[id] : pBuff2[id];\n"
      "}\n"
      "\n"
      "__kernel void test_kernel2(__global int4* pBuff0, __global int*"
      " pBuff1, __global const volatile int* pBuff2)\n"
      "{\n"
      "    size_t id = get_global_id(0);\n"
      "    pBuff0[id] = pBuff1[id] ? pBuff0[id] : pBuff2[id];\n"
      "}\n"
      "\n"
      "__kernel void test_kernel3(int n, __local int* m, __constant int* l,"
      " __global int* k, __private int i, __global int4* pBuff0)\n"
      "{\n"
      "    size_t id = get_global_id(0);\n"
      "    pBuff0[id] = n + m[id] + l[id] + k[id] + i;\n"
      "}\n"
      "\n"};

  // The programs are used to check invalid address qualifier
#define INVALID_PROGRAM_NUM 5
  const char *ocl_test_invalid_program1[] = {
      "__kernel void test_invalid_kernel1(int* n, __global int4* pBuff0)\n"
      "{\n"
      "    size_t id = get_global_id(0);\n"
      "    pBuff0[id] = n[id];\n"
      "}\n"
      "\n"};

  const char *ocl_test_invalid_program2[] = {
      "__kernel void test_invalid_kernel2(__private int* n, __global int4*"
      " pBuff0)\n"
      "{\n"
      "    size_t id = get_global_id(0);\n"
      "    pBuff0[id] = n[id];\n"
      "}\n"
      "\n"};
  const char *ocl_test_invalid_program3[] = {
      "__kernel void test_invalid_kernel3(__local int n, __global int4*"
      " pBuff0)\n"
      "{\n"
      "    size_t id = get_global_id(0);\n"
      "    pBuff0[id] = n;\n"
      "}\n"
      "\n"};
  const char *ocl_test_invalid_program4[] = {
      "__kernel void test_invalid_kernel4(__global int n, __global int4*"
      " pBuff0)\n"
      "{\n"
      "    size_t id = get_global_id(0);\n"
      "    pBuff0[id] = n;\n"
      "}\n"
      "\n"};
  const char *ocl_test_invalid_program5[] = {
      "__kernel void test_invalid_kernel5(__constant int n, __global int4*"
      " pBuff0)\n"
      "{\n"
      "    size_t id = get_global_id(0);\n"
      "    pBuff0[id] = n;\n"
      "}\n"
      "\n"};

  printf("clGetKernelArgInfoTest\n");
  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  cl_uint uiNumDevices = 0;
  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  std::vector<cl_device_id> devices(uiNumDevices);
  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, &devices[0], NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // check if all devices support images
  cl_bool isImagesSupported = CL_TRUE;
  for (unsigned int i = 0; i < uiNumDevices; ++i) {
    iRet = clGetDeviceInfo(devices[i], CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool),
                           &isImagesSupported, NULL);
    bResult =
        Check("clGetDeviceInfo(CL_DEVICE_IMAGE_SUPPORT)", CL_SUCCESS, iRet);
    if (!bResult) {
      return bResult;
    }
    // We build program on all the devices, so build is expected to fail
    // if at least one of them doesn't support images.
    if (isImagesSupported == CL_FALSE)
      break;
  }

  // create context
  cl_context context =
      clCreateContext(prop, uiNumDevices, &devices[0], NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateContext = %s\n", ClErrTxt(iRet));
    return false;
  }
  printf("context = %p\n", (void *)context);

  cl_program clProg;
  bResult &=
      BuildProgramSynch(context, 1, (const char **)&ocl_test_program, NULL,
                        "-cl-std=CL2.0 -cl-kernel-arg-info", &clProg);
  if (!bResult) {
    clReleaseContext(context);
    return bResult || isImagesSupported == CL_FALSE;
  }

  cl_kernel clKernel1 = clCreateKernel(clProg, "test_kernel1", &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateKernel = %s\n", ClErrTxt(iRet));

    clReleaseContext(context);
    clReleaseProgram(clProg);

    return false;
  }

  cl_kernel clKernel2 = clCreateKernel(clProg, "test_kernel2", &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateKernel = %s\n", ClErrTxt(iRet));

    clReleaseContext(context);
    clReleaseProgram(clProg);
    clReleaseKernel(clKernel1);

    return false;
  }

  cl_kernel clKernel3 = clCreateKernel(clProg, "test_kernel3", &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateKernel = %s\n", ClErrTxt(iRet));

    clReleaseContext(context);
    clReleaseProgram(clProg);
    clReleaseKernel(clKernel1);
    clReleaseKernel(clKernel2);

    return false;
  }

  cl_kernel_arg_address_qualifier addressQualifier;
  cl_kernel_arg_access_qualifier accessQualifier;
  cl_kernel_arg_type_qualifier typeQualifier;

  iRet |= clGetKernelArgInfo(clKernel1, 0, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                             sizeof(cl_kernel_arg_address_qualifier),
                             &addressQualifier, NULL);

  if (CL_KERNEL_ARG_ADDRESS_GLOBAL != addressQualifier) {
    printf("Incorrect address qualifier: 0x%X(expected: 0x%X)\n",
           addressQualifier, CL_KERNEL_ARG_ADDRESS_GLOBAL);
    iRet = -1;
  }

  // kernel argument without address qualifier
  iRet |= clGetKernelArgInfo(clKernel3, 0, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                             sizeof(cl_kernel_arg_address_qualifier),
                             &addressQualifier, NULL);

  // according to spec, the address qualifier is CL_KERNEL_ARG_ADDRESS_PRIVATE
  // by default. That is, if kernel argument is not specified with address
  // qualifier, it's set to CL_KERNEL_ARG_ADDRESS_PRIVATE.
  if (CL_KERNEL_ARG_ADDRESS_PRIVATE != addressQualifier) {
    printf("Incorrect address qualifier: 0x%X(expected: 0x%X)\n",
           addressQualifier, CL_KERNEL_ARG_ADDRESS_PRIVATE);
    iRet = -1;
  }

  // kernel argument with __local address qualifier
  iRet |= clGetKernelArgInfo(clKernel3, 1, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                             sizeof(cl_kernel_arg_address_qualifier),
                             &addressQualifier, NULL);

  if (CL_KERNEL_ARG_ADDRESS_LOCAL != addressQualifier) {
    printf("Incorrect address qualifier: 0x%X(expected: 0x%X)\n",
           addressQualifier, CL_KERNEL_ARG_ADDRESS_LOCAL);
    iRet = -1;
  }

  // kernel argument with __constant address qualifier
  iRet |= clGetKernelArgInfo(clKernel3, 2, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                             sizeof(cl_kernel_arg_address_qualifier),
                             &addressQualifier, NULL);

  if (CL_KERNEL_ARG_ADDRESS_CONSTANT != addressQualifier) {
    printf("Incorrect address qualifier: 0x%X(expected: 0x%X)\n",
           addressQualifier, CL_KERNEL_ARG_ADDRESS_CONSTANT);
    iRet = -1;
  }

  // kernel argument with __global address qualifier
  iRet |= clGetKernelArgInfo(clKernel3, 3, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                             sizeof(cl_kernel_arg_address_qualifier),
                             &addressQualifier, NULL);

  if (CL_KERNEL_ARG_ADDRESS_GLOBAL != addressQualifier) {
    printf("Incorrect address qualifier: 0x%X(expected: 0x%X)\n",
           addressQualifier, CL_KERNEL_ARG_ADDRESS_GLOBAL);
    iRet = -1;
  }

  // kernel argument with __private address qualifier
  iRet |= clGetKernelArgInfo(clKernel3, 4, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                             sizeof(cl_kernel_arg_address_qualifier),
                             &addressQualifier, NULL);

  if (CL_KERNEL_ARG_ADDRESS_PRIVATE != addressQualifier) {
    printf("Incorrect address qualifier: 0x%X(expected: 0x%X)\n",
           addressQualifier, CL_KERNEL_ARG_ADDRESS_PRIVATE);
    iRet = -1;
  }

  // Ensure clGetKernelArgInfo returns successfully when param_value is set to
  // null.
  iRet |= clGetKernelArgInfo(clKernel3, 4, CL_KERNEL_ARG_ADDRESS_QUALIFIER, 0,
                             NULL, NULL);

  const char **invalid_prog_ptr[] = {
      ocl_test_invalid_program1, ocl_test_invalid_program2,
      ocl_test_invalid_program3, ocl_test_invalid_program4,
      ocl_test_invalid_program5};

  for (int i = 0; i < INVALID_PROGRAM_NUM; i++) {
    bool bResultInvalid = true;
    bResultInvalid =
        BuildProgramSynch(context, 1, (const char **)invalid_prog_ptr[i], NULL,
                          "-cl-kernel-arg-info", &clProg);
    if (bResultInvalid) {
      printf("Program is built unexpectedly for invalid kernel %d\n", i);
      iRet = -1;
    }
  }

  char szTypeName[255];
  iRet |= clGetKernelArgInfo(clKernel1, 0, CL_KERNEL_ARG_TYPE_NAME,
                             sizeof(szTypeName), szTypeName, NULL);

  if (0 != strcmp(szTypeName, "char16*")) {
    iRet = -1;
  }

  // Test access qualifier.
  int arg_ids[4] = {1, 3, 4, 5};
  cl_kernel_arg_access_qualifier expected_results[4] = {
      CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_ACCESS_READ_ONLY,
      CL_KERNEL_ARG_ACCESS_READ_WRITE, CL_KERNEL_ARG_ACCESS_WRITE_ONLY};
  for (int i = 0; i < 4; ++i) {
    iRet |= clGetKernelArgInfo(
        clKernel1, arg_ids[i], CL_KERNEL_ARG_ACCESS_QUALIFIER,
        sizeof(cl_kernel_arg_address_qualifier), &accessQualifier, NULL);

    if (expected_results[i] != accessQualifier) {
      iRet = -1;
    }
  }

  iRet |= clGetKernelArgInfo(clKernel1, 2, CL_KERNEL_ARG_TYPE_QUALIFIER,
                             sizeof(cl_kernel_arg_type_qualifier),
                             &typeQualifier, NULL);

  if (CL_KERNEL_ARG_TYPE_CONST != typeQualifier) {
    iRet = -1;
  }

  iRet |= clGetKernelArgInfo(clKernel2, 0, CL_KERNEL_ARG_TYPE_NAME,
                             sizeof(szTypeName), szTypeName, NULL);

  if (0 != strcmp(szTypeName, "int4*")) {
    iRet = -1;
  }

  char szName[255];
  iRet |= clGetKernelArgInfo(clKernel2, 1, CL_KERNEL_ARG_NAME, sizeof(szName),
                             szName, NULL);

  if (0 != strcmp(szName, "pBuff1")) {
    iRet = -1;
  }

  iRet |= clGetKernelArgInfo(clKernel2, 2, CL_KERNEL_ARG_TYPE_QUALIFIER,
                             sizeof(cl_kernel_arg_type_qualifier),
                             &typeQualifier, NULL);

  if ((CL_KERNEL_ARG_TYPE_CONST | CL_KERNEL_ARG_TYPE_VOLATILE) !=
      typeQualifier) {
    iRet = -1;
  }

  if (CL_SUCCESS != iRet) {
    printf("clGetKernelArgInfo = %s\n", ClErrTxt(iRet));

    clReleaseContext(context);
    clReleaseProgram(clProg);
    clReleaseKernel(clKernel1);
    clReleaseKernel(clKernel2);

    return false;
  }

  iRet |= clGetKernelInfo(clKernel1, CL_KERNEL_ATTRIBUTES, sizeof(szName),
                          szName, NULL);
  if (0 == strstr(szName, "reqd_work_group_size(8,8,8)")) {
    iRet = -1;
  }

  if (0 == strstr(szName, "vec_type_hint(uint8)")) {
    iRet = -1;
  }

  // Release objects
  clReleaseContext(context);
  clReleaseProgram(clProg);
  clReleaseKernel(clKernel1);
  clReleaseKernel(clKernel2);

  return bResult;
}

void clGetKernelArgInfoNotAvailableTest() {
  printf("---------------------------------------\n");
  printf("clGetKernelArgInfoNotAvailable\n");
  printf("---------------------------------------\n");
  cl_device_id Device = NULL;
  cl_int BinaryStatus;
  cl_context Ctx;
  cl_platform_id Platform = 0;
  cl_program ProgramB = 0;
  cl_program ProgramIL = 0;

  cl_int Err = clGetPlatformIDs(1, &Platform, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clGetPlatformIDs");

  cl_context_properties Props[3] = {CL_CONTEXT_PLATFORM,
                                    (cl_context_properties)Platform, 0};
  Err = clGetDeviceIDs(Platform, gDeviceType, 1, &Device, NULL);
  ASSERT_OCL_SUCCESS(Err, "clGetDeviceIDs");

  // create context
  Ctx = clCreateContext(Props, 1, &Device, NULL, NULL, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateContext");

  // read bitcode
  std::ifstream BitcodeFile(get_exe_dir() + "test.bc", std::fstream::binary);
  std::vector<char> Bitcode(std::istreambuf_iterator<char>(BitcodeFile), {});
  size_t FileSize = Bitcode.size();
  unsigned char *Binaries[] = {
      reinterpret_cast<unsigned char *>(Bitcode.data())};

  // create program with binary
  ProgramB = clCreateProgramWithBinary(
      Ctx, 1, &Device, &FileSize, const_cast<const unsigned char **>(Binaries),
      &BinaryStatus, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateProgramWithBinary");

  // build program
  Err = clBuildProgram(ProgramB, 1, &Device, NULL, NULL, NULL);
  ASSERT_OCL_SUCCESS(Err, "clBuildProgram");
  // create kernel
  cl_kernel KernelB = clCreateKernel(ProgramB, "test_hostptr", &Err);
  ASSERT_OCL_SUCCESS(Err, "clBuildProgram");

  // Arg Type Qualifier
  cl_kernel_arg_type_qualifier ArgTypeQualifierB = 0;
  Err = clGetKernelArgInfo(KernelB, 0, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                           sizeof(cl_kernel_arg_address_qualifier),
                           &ArgTypeQualifierB, nullptr);
  ASSERT_OCL_EQ(Err, CL_KERNEL_ARG_INFO_NOT_AVAILABLE, "clGetKernelArgInfo");

  // read spirv
  std::ifstream SpirvFile(get_exe_dir() + "test.spv", std::fstream::in |
                                                          std::fstream::binary |
                                                          std::fstream::ate);
  std::vector<char> Spirv;
  size_t Length = SpirvFile.tellg();
  SpirvFile.seekg(0, SpirvFile.beg);
  Spirv.resize(Length, 0);
  SpirvFile.read(&Spirv[0], Length);

  // create program with spirv
  ProgramIL = clCreateProgramWithIL(Ctx, Spirv.data(), Spirv.size(), &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateProgramWithIL");

  // build program
  Err = clBuildProgram(ProgramIL, 1, &Device, NULL, NULL, NULL);
  ASSERT_OCL_SUCCESS(Err, "clBuildProgram");

  // create kernel
  cl_kernel KernelIL = clCreateKernel(ProgramB, "test_hostptr", &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateKernel");

  // Arg Type Qualifier
  cl_kernel_arg_type_qualifier ArgTypeQualifierIL = 0;
  Err = clGetKernelArgInfo(KernelIL, 0, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                           sizeof(cl_kernel_arg_address_qualifier),
                           &ArgTypeQualifierIL, nullptr);
  ASSERT_OCL_EQ(Err, CL_KERNEL_ARG_INFO_NOT_AVAILABLE, "clGetKernelArgInfo");

  Err = clReleaseKernel(KernelB);
  ASSERT_OCL_SUCCESS(Err, "clReleaseKernel");
  Err = clReleaseProgram(ProgramB);
  ASSERT_OCL_SUCCESS(Err, "clReleaseProgram");
  Err = clReleaseKernel(KernelIL);
  ASSERT_OCL_SUCCESS(Err, "clReleaseKernel");
  Err = clReleaseProgram(ProgramIL);
  ASSERT_OCL_SUCCESS(Err, "clReleaseProgram");
  Err = clReleaseContext(Ctx);
  ASSERT_OCL_SUCCESS(Err, "clReleaseContext");
}

void clGetKernelArgInfoAvailableWithBinaryTest() {
  printf("---------------------------------------\n");
  printf("clGetKernelArgInfoAvailableWithBinary\n");
  printf("---------------------------------------\n");
  cl_device_id Device = NULL;
  cl_int BinaryStatus;
  cl_context Ctx;
  cl_platform_id Platform = 0;
  cl_program ProgramB = 0;

  cl_int Err = clGetPlatformIDs(1, &Platform, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clGetPlatformIDs");

  cl_context_properties Props[3] = {CL_CONTEXT_PLATFORM,
                                    (cl_context_properties)Platform, 0};
  Err = clGetDeviceIDs(Platform, gDeviceType, 1, &Device, NULL);
  ASSERT_OCL_SUCCESS(Err, "clGetDeviceIDs");

  // create context
  Ctx = clCreateContext(Props, 1, &Device, NULL, NULL, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateContext");

  // read bitcode
  std::ifstream BitcodeFile(get_exe_dir() + "test.bc", std::fstream::binary);
  std::vector<char> Bitcode(std::istreambuf_iterator<char>(BitcodeFile), {});
  size_t FileSize = Bitcode.size();
  unsigned char *Binaries[] = {
      reinterpret_cast<unsigned char *>(Bitcode.data())};

  // create program with binary
  ProgramB = clCreateProgramWithBinary(
      Ctx, 1, &Device, &FileSize, const_cast<const unsigned char **>(Binaries),
      &BinaryStatus, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateProgramWithBinary");

  // build program
  Err = clBuildProgram(ProgramB, 1, &Device, "-x spir", NULL, NULL);
  ASSERT_OCL_SUCCESS(Err, "clBuildProgram");

  // create kernel
  cl_kernel KernelB = clCreateKernel(ProgramB, "test_hostptr", &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateKernel");

  // Arg Type Qualifier
  cl_kernel_arg_type_qualifier ArgTypeQualifierB = 0;
  Err = clGetKernelArgInfo(KernelB, 0, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                           sizeof(cl_kernel_arg_address_qualifier),
                           &ArgTypeQualifierB, nullptr);
  ASSERT_OCL_EQ(Err, CL_SUCCESS, "clGetKernelArgInfo");

  Err = clReleaseKernel(KernelB);
  ASSERT_OCL_SUCCESS(Err, "clReleaseKernel");
  Err = clReleaseProgram(ProgramB);
  ASSERT_OCL_SUCCESS(Err, "clReleaseProgram");
  Err = clReleaseContext(Ctx);
  ASSERT_OCL_SUCCESS(Err, "clReleaseContext");
}

void clGetKernelArgInfoAfterLinkTest() {
  printf("---------------------------------------\n");
  printf("clGetKernelArgInfAfterLinkTest\n");
  printf("---------------------------------------\n");

  const char *KernelCode = "\
    __kernel void test_kernel(__global int* pBuff)\n\
    {\n\
        size_t id = get_global_id(0);\n\
        pBuff[id] = id * 2;\n\
    }\n\
    ";

  cl_device_id Device = NULL;
  cl_int BinaryStatus;
  cl_context Ctx;
  cl_platform_id Platform = 0;
  cl_program ProgramBinary = 0;
  cl_program ProgramSource = 0;

  cl_int Err = clGetPlatformIDs(1, &Platform, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clGetPlatformIDs");

  cl_context_properties Props[3] = {CL_CONTEXT_PLATFORM,
                                    (cl_context_properties)Platform, 0};
  Err = clGetDeviceIDs(Platform, gDeviceType, 1, &Device, NULL);
  ASSERT_OCL_SUCCESS(Err, "clGetDeviceIDs");

  // create context
  Ctx = clCreateContext(Props, 1, &Device, NULL, NULL, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateContext");

  // create program with source
  ProgramSource = clCreateProgramWithSource(Ctx, 1, &KernelCode, nullptr, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateProgramWithSource");

  // compiler program built from source
  Err = clCompileProgram(ProgramSource, 1, &Device, "-cl-std=CL2.0", 0, NULL,
                         NULL, NULL, NULL);
  ASSERT_OCL_SUCCESS(Err, "clCompileProgram");

  // read bitcode
  std::ifstream BitcodeFile(get_exe_dir() + "test.bc", std::fstream::binary);
  std::vector<char> Bitcode(std::istreambuf_iterator<char>(BitcodeFile), {});
  size_t FileSize = Bitcode.size();
  unsigned char *Binaries[] = {
      reinterpret_cast<unsigned char *>(Bitcode.data())};

  // create program with binary
  ProgramBinary = clCreateProgramWithBinary(
      Ctx, 1, &Device, &FileSize, const_cast<const unsigned char **>(Binaries),
      &BinaryStatus, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateProgramWithBinary");

  // compiler program built from binary
  Err = clCompileProgram(ProgramBinary, 1, &Device, "-cl-std=CL2.0 -x spir", 0,
                         NULL, NULL, NULL, NULL);
  ASSERT_OCL_SUCCESS(Err, "clCompileProgram");

  const cl_program ToLink[] = {ProgramSource, ProgramBinary};

  cl_program ProgramLinked =
      clLinkProgram(Ctx, 1, &Device, NULL, 2, ToLink, NULL, NULL, &Err);
  ASSERT_OCL_SUCCESS(Err, "clLinkProgram");

  // create kernel of ProgramBinary
  cl_kernel KernelBinary = clCreateKernel(ProgramLinked, "test_hostptr", &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateKernel");

  // create kernel of ProgramSource
  cl_kernel KernelSource = clCreateKernel(ProgramLinked, "test_kernel", &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateKernel");

  // Arg Type Qualifier
  cl_kernel_arg_type_qualifier ArgTypeQualifierBinary = 0;
  Err = clGetKernelArgInfo(KernelBinary, 0, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                           sizeof(cl_kernel_arg_address_qualifier),
                           &ArgTypeQualifierBinary, nullptr);
  ASSERT_OCL_EQ(Err, CL_SUCCESS, "clGetKernelArgInfo");

  cl_kernel_arg_type_qualifier ArgTypeQualifierSource = 0;
  Err = clGetKernelArgInfo(KernelSource, 0, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                           sizeof(cl_kernel_arg_address_qualifier),
                           &ArgTypeQualifierSource, nullptr);
  ASSERT_OCL_EQ(Err, CL_SUCCESS, "clGetKernelArgInfo");

  Err = clReleaseKernel(KernelBinary);
  ASSERT_OCL_SUCCESS(Err, "clReleaseKernel");
  Err = clReleaseProgram(ProgramBinary);
  ASSERT_OCL_SUCCESS(Err, "clReleaseProgram");
  Err = clReleaseKernel(KernelSource);
  ASSERT_OCL_SUCCESS(Err, "clReleaseKernel");
  Err = clReleaseProgram(ProgramSource);
  ASSERT_OCL_SUCCESS(Err, "clReleaseProgram");
  Err = clReleaseProgram(ProgramLinked);
  ASSERT_OCL_SUCCESS(Err, "clReleaseProgram");
  Err = clReleaseContext(Ctx);
  ASSERT_OCL_SUCCESS(Err, "clReleaseContext");
}
