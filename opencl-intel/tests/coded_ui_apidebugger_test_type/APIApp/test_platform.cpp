/******************************************************************
 //
 //  OpenCL Conformance Tests
 // 
 //  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/
#include "testBase.h"

#include <string.h>

#define EXTENSION_NAME_BUF_SIZE 4096

#define PRINT_EXTENSION_INFO 0

int test_get_platform_ids(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements) {
  cl_platform_id platforms[16];
  cl_uint num_platforms;
  char *string_returned;
  
  string_returned = (char*)malloc(8192);

  int total_errors = 0;
  int err = CL_SUCCESS;
  
  err = clGetPlatformIDs(0, platforms, &num_platforms);
  err = clGetPlatformIDs(16, platforms, &num_platforms);
  test_error(err, "clGetPlatformIDs failed");
    
  if (num_platforms <= 16) {
    // Try with NULL
    err = clGetPlatformIDs(num_platforms, platforms, NULL);
    test_error(err, "clGetPlatformIDs failed with NULL for return size");
  }
  
  if (num_platforms < 1) {
    log_error("Found 0 platforms.\n");
    return -1;
  }
  log_info("Found %d platforms.\n", num_platforms);
  
  
  for (int p=0; p<1/*(int)num_platforms*/; p++) {
    cl_device_id *devices;
    cl_uint num_devices;
    size_t size;
    
    
    log_info("Platform %d (%p):\n", p, platforms[p]);
    
    memset(string_returned, 0, 8192);
    err = clGetPlatformInfo(platforms[p], CL_PLATFORM_PROFILE, 0, string_returned, &size);
    err = clGetPlatformInfo(platforms[p], CL_PLATFORM_PROFILE, 8192, string_returned, &size);
    test_error(err, "clGetPlatformInfo for CL_PLATFORM_PROFILE failed");
    log_info("\tCL_PLATFORM_PROFILE: %s\n", string_returned);
    if (strlen(string_returned)+1 != size) {
      log_error("Returned string length %ld does not equal reported one %ld.\n", strlen(string_returned)+1, size);
      total_errors++;
    }
    
    memset(string_returned, 0, 8192);
    err = clGetPlatformInfo(platforms[p], CL_PLATFORM_VERSION, 8192, string_returned, &size);
    test_error(err, "clGetPlatformInfo for CL_PLATFORM_VERSION failed");
    log_info("\tCL_PLATFORM_VERSION: %s\n", string_returned);
    if (strlen(string_returned)+1 != size) {
      log_error("Returned string length %ld does not equal reported one %ld.\n", strlen(string_returned)+1, size);
      total_errors++;
    }

    memset(string_returned, 0, 8192);
    err = clGetPlatformInfo(platforms[p], CL_PLATFORM_NAME, 8192, string_returned, &size);
    test_error(err, "clGetPlatformInfo for CL_PLATFORM_NAME failed");
    log_info("\tCL_PLATFORM_NAME: %s\n", string_returned);
    if (strlen(string_returned)+1 != size) {
      log_error("Returned string length %ld does not equal reported one %ld.\n", strlen(string_returned)+1, size);
      total_errors++;
    }

    memset(string_returned, 0, 8192);
    err = clGetPlatformInfo(platforms[p], CL_PLATFORM_VENDOR, 8192, string_returned, &size);
    test_error(err, "clGetPlatformInfo for CL_PLATFORM_VENDOR failed");
    log_info("\tCL_PLATFORM_VENDOR: %s\n", string_returned);
    if (strlen(string_returned)+1 != size) {
      log_error("Returned string length %ld does not equal reported one %ld.\n", strlen(string_returned)+1, size);
      total_errors++;
    }
    
    memset(string_returned, 0, 8192);
    err = clGetPlatformInfo(platforms[p], CL_PLATFORM_EXTENSIONS, 8192, string_returned, &size);
    test_error(err, "clGetPlatformInfo for CL_PLATFORM_EXTENSIONS failed");
    log_info("\tCL_PLATFORM_EXTENSIONS: %s\n", string_returned);
    if (strlen(string_returned)+1 != size) {
      log_error("Returned string length %ld does not equal reported one %ld.\n", strlen(string_returned)+1, size);
      total_errors++;
    }
    
    err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    err = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    test_error(err, "clGetDeviceIDs size failed.\n");
    devices = (cl_device_id *)malloc(num_devices*sizeof(cl_device_id));
    memset(devices, 0, sizeof(cl_device_id)*num_devices);
    err = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
    test_error(err, "clGetDeviceIDs failed.\n");
    
    log_info("\tPlatform has %d devices.\n", (int)num_devices);
    for (int d=0; d<1/*(int)num_devices*/; d++) {
      size_t returned_size;
      cl_platform_id returned_platform;
      cl_context context;
      cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[p], 0 };

      err = clGetDeviceInfo(devices[d], CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &returned_platform, &returned_size);
      test_error(err, "clGetDeviceInfo failed for CL_DEVICE_PLATFORM\n");
      if (returned_size != sizeof(cl_platform_id)) {
        log_error("Reported return size (%ld) does not match expected size (%ld).\n", returned_size, sizeof(cl_platform_id));
        total_errors++;
      }
      
      memset(string_returned, 0, 8192);
      err = clGetDeviceInfo(devices[d], CL_DEVICE_NAME, 8192, string_returned, NULL);
      test_error(err, "clGetDeviceInfo failed for CL_DEVICE_NAME\n");
      
      log_info("\t\tPlatform for device %d (%s) is %p.\n", d, string_returned, returned_platform);
      
      log_info("\t\t\tTesting clCreateContext for the platform/device...\n");
      // Try creating a context for the platform
      context = clCreateContext(properties, 1, &devices[d], NULL, NULL, &err);
      test_error(err, "\t\tclCreateContext failed for device with platform properties\n");
      
      memset(properties, 0, sizeof(cl_context_properties)*3);
      
      err = clGetContextInfo(context, CL_CONTEXT_PROPERTIES, NULL, properties, &returned_size);
      err = clGetContextInfo(context, CL_CONTEXT_PROPERTIES, sizeof(cl_context_properties)*3, properties, &returned_size);
      test_error(err, "clGetContextInfo for CL_CONTEXT_PROPERTIES failed");
      if (returned_size != sizeof(cl_context_properties)*3) {
        log_error("Invalid size returned from clGetContextInfo for CL_CONTEXT_PROPERTIES. Got %ld, expected %ld.\n", 
                  returned_size, sizeof(cl_context_properties)*3);
        total_errors++;
      }
      
      if (properties[0] != (cl_context_properties)CL_CONTEXT_PLATFORM || properties[1] != (cl_context_properties)platforms[p]) {
        log_error("Wrong properties returned. Expected: [%p %p], got [%p %p]\n",
                  (void*)CL_CONTEXT_PLATFORM, platforms[p], (void*)properties[0], (void*)properties[1]);
        total_errors++;
      }
      
      err = clRetainContext(NULL);
      err = clRetainContext(context);

      err = clReleaseContext(NULL);
      err = clReleaseContext(context);

      err = clReleaseContext(context);
      test_error(err, "clReleaseContext failed");
    }
    free(devices);
  }
  
  free(string_returned);
             
  return total_errors;
}
