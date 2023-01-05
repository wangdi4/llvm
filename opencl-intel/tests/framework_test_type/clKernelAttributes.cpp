#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>
#include <string>
#include <vector>

extern cl_device_type gDeviceType;

/*******************************************************************************
 * clKernelAttributes
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create binary
 * (4) create program with source
 * (5) build program
 ******************************************************************************/

bool clKernelAttributesTest() {
  bool bResult = true;
  const char *sample_attributes_kernel[] = {
      "__kernel __attribute__((reqd_work_group_size(2, 3, 4)))\n"
      "__attribute__((vec_type_hint(float)))\n"
      " void sample_test_reqrd(__global long *result)\n"
      "{\n"
      "result[get_global_id(0)] = 0;\n"
      "}\n"
      "__kernel void sample_test_prefered(__global long *result)\n"
      "{\n"
      "result[get_global_id(0)] = 0;\n"
      "}\n"};

  printf("clKernelAttributesTest\n");
  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  cl_context context;
  cl_program prog;
  cl_kernel kernel;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  if (CL_SUCCESS != iRet) {
    delete[] pDevices;
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= Check("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    return false;
  }

  prog = clCreateProgramWithSource(
      context, 1, (const char **)&sample_attributes_kernel, NULL, &iRet);
  bResult &= Check("clCreateProgramWithSource", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  iRet = clBuildProgram(prog, uiNumDevices, pDevices, NULL, NULL, NULL);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseProgram(prog);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  kernel = clCreateKernel(prog, "sample_test_reqrd", &iRet);
  bResult = Check("clCreateKernel", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseProgram(prog);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  // Get attribute size.
  size_t size = 0;
  iRet = clGetKernelInfo(kernel, CL_KERNEL_ATTRIBUTES, 0, nullptr, &size);
  bResult = Check("clGetKernelInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseProgram(prog);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  // Get attribute string.
  std::vector<char> attributes(size);
  iRet = clGetKernelInfo(kernel, CL_KERNEL_ATTRIBUTES, attributes.size(),
                         attributes.data(), nullptr);
  bResult = Check("clGetKernelInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseProgram(prog);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }
  std::string attrStr(attributes.data());

  bool attrFound =
      attrStr.find("reqd_work_group_size(2,3,4)") != std::string::npos &&
      attrStr.find("vec_type_hint(float)") != std::string::npos;
  bool bRes = Check("CL_KERNEL_ATTRIBUTES", true, attrFound);

  // Query attribute of kernel not created from source.
  std::vector<size_t> binarySizes(uiNumDevices);
  // Get the binary
  iRet =
      clGetProgramInfo(prog, CL_PROGRAM_BINARY_SIZES,
                       sizeof(size_t) * uiNumDevices, binarySizes.data(), NULL);
  bResult = Check("clGetProgramInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseProgram(prog);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  size_t sumBinariesSize = 0;
  unsigned char **pBinaries = new unsigned char *[uiNumDevices];
  for (unsigned int i = 0; i < uiNumDevices; i++) {
    pBinaries[i] = new unsigned char[binarySizes[i]];
    sumBinariesSize += binarySizes[i];
  }
  iRet = clGetProgramInfo(prog, CL_PROGRAM_BINARIES, sumBinariesSize, pBinaries,
                          nullptr);
  bResult = Check("clGetProgramInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    for (unsigned int i = 0; i < uiNumDevices; i++) {
      delete[] pBinaries[i];
    }
    clReleaseProgram(prog);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }
  cl_program binProg = clCreateProgramWithBinary(
      context, uiNumDevices, pDevices, binarySizes.data(),
      const_cast<const unsigned char **>(pBinaries), nullptr, &iRet);
  for (unsigned int i = 0; i < uiNumDevices; i++) {
    delete[] pBinaries[i];
  }
  bResult = Check("clCreateProgramWithBinary", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseProgram(binProg);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  iRet = clBuildProgram(binProg, uiNumDevices, pDevices, nullptr, nullptr,
                        nullptr);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseProgram(binProg);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  cl_kernel binKernel = clCreateKernel(binProg, "sample_test_reqrd", &iRet);
  bResult = Check("clCreateKernel", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseProgram(binProg);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  // Get attribute size.
  iRet = clGetKernelInfo(binKernel, CL_KERNEL_ATTRIBUTES, 0, nullptr, &size);
  bResult = Check("clGetKernelInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseKernel(binKernel);
    clReleaseProgram(binProg);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  // Get attribute string.
  std::vector<char> binAttributes(size);
  iRet = clGetKernelInfo(binKernel, CL_KERNEL_ATTRIBUTES, binAttributes.size(),
                         binAttributes.data(), nullptr);
  bResult = Check("clGetKernelInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseKernel(binKernel);
    clReleaseProgram(binProg);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }
  std::string binAttrStr(binAttributes.data());
  bRes = Check("CL_KERNEL_ATTRIBUTES", true, binAttrStr.empty());

  // Get kernel extended attributes
  size_t wgSizeInfo[3];
  iRet = clGetKernelWorkGroupInfo(kernel, pDevices[0],
                                  CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                                  sizeof(wgSizeInfo), &wgSizeInfo, NULL);
  bResult = Check("clGetKernelWorkGroupInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  bRes &= Check("CL_KERNEL_COMPILE_WORK_GROUP_SIZE", true,
                (wgSizeInfo[0] == 2) && (wgSizeInfo[1] == 3) &&
                    (wgSizeInfo[2] == 4));

  size_t wgMaxSize = 0;
  iRet =
      clGetKernelWorkGroupInfo(kernel, pDevices[0], CL_KERNEL_WORK_GROUP_SIZE,
                               sizeof(size_t), &wgMaxSize, NULL);
  bResult = Check("clGetKernelWorkGroupInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }

  iRet = clGetKernelWorkGroupInfo(kernel, pDevices[0],
                                  CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                  sizeof(size_t), &wgMaxSize, NULL);
  bResult = Check("clGetKernelWorkGroupInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }
  bRes &= Check("CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE", true,
                (wgSizeInfo[0] * wgSizeInfo[1] * wgSizeInfo[2] == wgMaxSize));

  cl_ulong ulPrSize;
  iRet =
      clGetKernelWorkGroupInfo(kernel, pDevices[0], CL_KERNEL_PRIVATE_MEM_SIZE,
                               sizeof(cl_ulong), &ulPrSize, NULL);
  bResult = Check("clGetKernelWorkGroupInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
    clReleaseContext(context);
    delete[] pDevices;
    return false;
  }
  bRes &= Check("CL_KERNEL_PRIVATE_MEM_SIZE", 0, ulPrSize);

  clReleaseKernel(kernel);
  clReleaseProgram(prog);
  clReleaseContext(context);
  delete[] pDevices;

  return bRes;
}
