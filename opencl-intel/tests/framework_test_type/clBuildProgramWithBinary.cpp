#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

/*******************************************************************************
 * clBuildProgram
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create binary
 * (4) create program with binary
 * (5) build program
 ******************************************************************************/

cl_program g_clProgram = 0;

char g_pwsUserData[256] = "BLA BLA BLA";

bool g_bFinished = false;

bool g_bResult = false;

bool clBuildProgramWithBinaryTest(openBcFunc pFunc) {
  bool bResult = true;

  printf("---------------------------------------\n");
  printf("clBuildProgramWithBinaryTest\n");
  printf("---------------------------------------\n");
  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;
  cl_int *pBinaryStatus;
  cl_context context;
  unsigned char **ppContainers;
  //  cl_program program;

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
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];
  ppContainers = new unsigned char *[uiNumDevices];

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  if (CL_SUCCESS != iRet) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;

    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateContext = %s\n", ClErrTxt(iRet));
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    return false;
  }
  printf("context = %p\n", (void *)context);

  // create binary container
  unsigned int uiContSize = 0;
  FILE *pIRfile = NULL;
  pFunc(pIRfile);
  fpos_t fileSize;
  SET_FPOS_T(fileSize, 0);
  if (NULL == pIRfile) {
    printf("Failed open file.\n");
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    return false;
  }
  fseek(pIRfile, 0, SEEK_END);
  fgetpos(pIRfile, &fileSize);
  uiContSize += (unsigned int)GET_FPOS_T(fileSize);
  fseek(pIRfile, 0, SEEK_SET);

  unsigned char *pCont = (unsigned char *)malloc(uiContSize);
  if (NULL == pCont) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    return false;
  }
  // Construct program container
  size_t ret =
      fread(((unsigned char *)pCont), 1, (size_t)GET_FPOS_T(fileSize), pIRfile);
  if (ret != (size_t)GET_FPOS_T(fileSize)) {
    printf("Failed read file.\n");
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    return false;
  }
  fclose(pIRfile);

  for (unsigned int i = 0; i < uiNumDevices; i++) {
    pBinarySizes[i] = uiContSize;
    ppContainers[i] = pCont;
  }

  // create program with binary
  g_clProgram = clCreateProgramWithBinary(
      context, uiNumDevices, pDevices, pBinarySizes,
      const_cast<const unsigned char **>(ppContainers), pBinaryStatus, &iRet);
  bResult &= Check("clCreateProgramWithBinary", CL_SUCCESS, iRet);

  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    return bResult;
  }

  iRet = clBuildProgram(g_clProgram, uiNumDevices, pDevices, NULL, NULL, NULL);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);

  //  while (!g_bFinished)
  //  {
  //    Sleep(1);
  //  }

  //  bResult &= g_bResult;

  cl_build_status clBuildStatus = CL_BUILD_NONE;
  for (cl_uint ui = 0; ui < uiNumDevices; ++ui) {
    iRet = clGetProgramBuildInfo(g_clProgram, pDevices[ui],
                                 CL_PROGRAM_BUILD_STATUS,
                                 sizeof(cl_build_status), &clBuildStatus, NULL);
    bResult &= Check("clGetProgramBuildInfo(CL_PROGRAM_BUILD_STATUS)",
                     CL_SUCCESS, iRet);
    bResult &= CheckInt("check status", (int)clBuildStatus, CL_BUILD_SUCCESS);
  }

  // a test for garbage binary file
  // TODO: uncomment and check
  //   struct stat IRstatus;
  //   stat("test.bc",&IRstatus);
  //   iRet=fopen_s(&pIRfile,"test.bc", "rb");  //without all the headers so
  // garbage   bResult&=SilentCheck("open file pIRfile",CL_SUCCESS,iRet);
  // if
  // (!bResult){     goto finish_line;
  //   }
  //   cl_int binary_status=1;
  //   size_t binary_length=IRstatus.st_size;
  //   program=clCreateProgramWithBinary(context,1,pDevices,&(binary_length),(const
  // unsigned char**)(&pIRfile),&binary_status,&iRet);
  //   bResult&=SilentCheck("clCreateCommandQueue",CL_SUCCESS,iRet);
  //   fclose(pIRfile);
  //   if (!bResult){
  //     goto finish_line;
  //   }
  //
  //   //Build Program
  //   iRet=clBuildProgram(program,0,NULL,NULL,NULL,NULL);  //should fail here
  //   switch (iRet)
  //   {
  //   case CL_SUCCESS:
  //     printf ("ERROR: got CL_SUCCESS and should have got an error");
  //     goto finish_line;
  //   case CL_INVALID_CONTEXT:
  //     printf ("SUCCESS: clBuildProgram got the following error: ");
  //     printf("CL_INVALID_CONTEXT \n");
  //     break;
  //   case CL_INVALID_VALUE:
  //     printf ("SUCCESS: clBuildProgram got the following error: ");
  //     printf("CL_INVALID_VALUE \n");
  //     break;
  //   case CL_INVALID_DEVICE:
  //     printf ("SUCCESS: clBuildProgram got the following error: ");
  //     printf("CL_INVALID_DEVICE \n");
  //     break;
  //   case CL_INVALID_BINARY:
  //     printf ("SUCCESS: clBuildProgram got the following error: ");
  //     printf("CL_INVALID_BINARY \n");
  //     break;
  //   case CL_OUT_OF_RESOURCES:
  //     printf ("SUCCESS: clBuildProgram got the following error: ");
  //     printf("CL_OUT_OF_RESOURCES\n");
  //     break;
  //   case CL_OUT_OF_HOST_MEMORY:
  //     printf ("SUCCESS: clBuildProgram got the following error: ");
  //     printf("CL_OUT_OF_HOST_MEMORY \n");
  //     break;
  //   default:
  //     printf ("ERROR: clBuildProgram got the following error %d ,
  // which is not a legal error \n",iRet);     goto finish_line;
  //   }
  //   cl_build_status build_status=1;
  //   iRet=clGetProgramBuildInfo(program,pDevices[0],CL_PROGRAM_BUILD_STATUS,sizeof(cl_build_status),&build_status,NULL);
  //   bResult&=SilentCheck("clGetProgramBuildInfo",CL_SUCCESS,iRet);
  //   if(CL_BUILD_ERROR!=build_status){
  //     printf("ERROR: build status is not CL_BUILD_ERROR , it is
  // %d",build_status);     goto finish_line;
  //   }
  //
  //   finish_line:
  // end test for garbage binary file

  delete[] pDevices;
  delete[] pBinarySizes;
  delete[] pBinaryStatus;
  delete[] ppContainers;

  free(pCont);

  clReleaseProgram(g_clProgram);
  clReleaseContext(context);
  return bResult;
}
