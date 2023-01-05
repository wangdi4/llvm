#include "CL/cl.h"
#include "FrameworkTest.h"
#include "FrameworkTestThreads.h"
#include "llvm/Support/Compiler.h" // LLVM_FALLTHROUGH

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define REOPEN_STDOUT 0
#define RELEASE_KERNEL 1
#define RELEASE_PROGRAM 2
#define RELEASE_QUEUE 3
#define RELEASE_CONTEXT 4
#define RELEASE_END 5
#define FINAL_ROUTINE 6
#define RELEASE_MEM 7

#define WORK_SIZE                                                              \
  1 // all test are checking one work-item run, so will surely fail if changed
#define N 0
#define MAX_SOURCE_SIZE 100000 //(0x100000)

#define XSTR(s) STR(s)
#define STR(s) #s

#ifdef _WIN32
#define FILENO _fileno
#define FLUSHALL() _flushall()
#else
#define FILENO fileno
#define FLUSHALL() fflush(NULL)
#endif

extern cl_device_type gDeviceType;

bool restore_stdout(FILE *outfile, int old_stdout) {
  if (outfile == NULL) {
    return false;
  }
  fflush(stdout);
  fclose(outfile);
  DUP2(old_stdout, 1);
  // TODO AdirD - Ask Eli about the flushall of input streams
  FLUSHALL();
  return true;
}
// OPEN THE FILE, THE CALLER NEED TO CLOSE IT
bool redirect_stdout(FILE **outfile, const char *outfile_name, const char *mode,
                     int *old_stdout) {
  fflush(stdout);
  if (outfile_name == NULL || old_stdout == NULL) {
    return false;
  }
  (*old_stdout) = DUP(1);
  if ((*old_stdout) == -1) {
    return false;
  }
#ifdef _WIN32
  if (0 != fopen_s(outfile, outfile_name, mode)) {
    return false;
  }
#else
  *outfile = fopen(outfile_name, mode);
  if (NULL == *outfile) {
    return false;
  }
#endif
  if (DUP2(FILENO((*outfile)), 1) == -1) {
    restore_stdout(*outfile, (*old_stdout));
    return false;
  }

  return true;
}
/*
/ compare the two files and if they are the same also deletes them....
*/
bool FilesCompareNdDelete(const char *f_name1, const char *f_name2) {
  if (f_name1 == NULL || f_name2 == NULL) {
    return false;
  }
  FILE *fp1;
#ifdef _WIN32
  if (0 != fopen_s(&fp1, f_name1, "r")) {
    return false;
  }
#else
  fp1 = fopen(f_name1, "r");
  if (NULL == fp1) {
    return false;
  }
#endif
  FILE *fp2;
#ifdef _WIN32
  if (0 != fopen_s(&fp2, f_name2, "r")) {
    fclose(fp1);
    return false;
  }
#else
  fp2 = fopen(f_name2, "r");
  if (NULL == fp2) {
    fclose(fp1);
    return false;
  }
#endif
  char c1, c2;
  int place = 0;
  bool res = true;
  // comparing....
  while (!feof(fp1)) {
    c1 = fgetc(fp1);
    if (ferror(fp1)) {
      printf("ERROR: file %s couldn't read or smaller \n", f_name1);
      res = false;
      break;
    }
    c2 = fgetc(fp2);
    if (ferror(fp2)) {
      printf("ERROR: file %s couldn't read or smaller \n", f_name2);
      res = false;
      break;
    }
    if (c1 != c2) {
      printf("ERROR: files %s & %s are different at char %d \n", f_name1,
             f_name2, place);
      res = false;
      break;
    }
    place++;
  }

  fclose(fp1);
  fclose(fp2);
  if (res) {
    remove(f_name1);
    remove(f_name2);
  }
  return res;
}

#define PRINTF_SPECIAL_CHARS                                                   \
  __kernel void printf_special_chars(__global int *count, int num_of_chars) {  \
    int len = printf("special characters check: \n");                          \
    len += printf("Percent: %% \n");                                           \
    len += printf("Single quote: \'\n");                                       \
    len += printf("Double quote: \"\n");                                       \
    len += printf("Backslash: \\\n");                                          \
    len += printf("Audible bell: \a\n");                                       \
    len += printf("Backspace: \b\n");                                          \
    len += printf("Newline: \n\n");                                            \
    len += printf("\t:Horizontal tab\n");                                      \
    len += printf("Octal number (035): \035\n");                               \
    len +=                                                                     \
        printf("Null character (really just the octal number zero): \0   \n"); \
    len += printf("Formfeed: \f\n");                                           \
    len += printf("Carriage return: \r\n");                                    \
    len += printf("Vertical tab: \v\n");                                       \
    len += printf("Hexadecimal number (0F3)): \x0F3\n");                       \
    if (len == num_of_chars) {                                                 \
      atom_inc(count); /* conformance issue should be: atomic_inc(count); */   \
    }                                                                          \
  }

int printf_special_chars() {
  int len = printf("special characters check: \n");
  len += printf("Percent: %% \n");
  len += printf("Single quote: \'\n");
  len += printf("Double quote: \"\n");
  len += printf("Backslash: \\\n");
  len += printf("Audible bell: \a\n");
  len += printf("Backspace: \b\n");
  len += printf("Newline: \n\n");
  len += printf("\t:Horizontal tab\n");
  len += printf("Octal number (035): \035\n");
  len += printf("Null character (really just the octal number zero): %c   \n",
                '\0');
  len += printf("Formfeed: \f\n");
  len += printf("Carriage return: \r\n");
  len += printf("Vertical tab: \v\n");
  len += printf("Hexadecimal number (0F3)): \x0F3\n");
  return len;
}

void run_kernel_printf_special_chars(cl_context &context, cl_device_id &device,
                                     cl_command_queue &cmd_queue,
                                     bool &bResult) {
  cl_int err;
  cl_kernel kernel = nullptr;
  cl_program program = nullptr;
  cl_mem count = nullptr;
  int success_count = 0;
  FILE *fp = NULL;
  int old_stdout;
  try {
    const char *ocl_test_program[] = {XSTR(PRINTF_SPECIAL_CHARS)};
    program = clCreateProgramWithSource(
        context, 1, (const char **)&ocl_test_program, NULL, &err);
    bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_QUEUE;

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    cl_build_status build_status;

    err |= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
                                 MAX_SOURCE_SIZE, &build_status, NULL);
    if (err != CL_SUCCESS || build_status == CL_BUILD_ERROR) {
      printf("\n build status is: %d \n", build_status);
      char err_str[MAX_SOURCE_SIZE]; // instead of dynamic allocation
      char *err_str_ptr = err_str;
      err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  MAX_SOURCE_SIZE, err_str_ptr, NULL);
      if (err != CL_SUCCESS)
        printf("Build Info error: %d \n", err);
      printf("%s \n", err_str_ptr);
      throw RELEASE_QUEUE;
    }
    bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_QUEUE;

    kernel = clCreateKernel(program, "printf_special_chars", &err);
    bResult &= SilentCheck("clCreateKernel", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_PROGRAM;

    bResult &=
        redirect_stdout(&fp, "printf_sepcial_chars.txt", "w", &old_stdout);
    if (!bResult) {
      printf("redirect stdout failed \n");
      throw RELEASE_KERNEL;
    }

    int num_of_chars = printf_special_chars();

    restore_stdout(fp, old_stdout);

    bResult &= redirect_stdout(&fp, "kernel_printf_sepcial_chars.txt", "w",
                               &old_stdout);
    if (!bResult) {
      printf("redirect stdout failed \n");
      throw RELEASE_KERNEL;
    }

    count = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
    bResult &= SilentCheck("clCreateBuffer", CL_SUCCESS, err);
    if (!bResult)
      throw REOPEN_STDOUT;
    err = clEnqueueWriteBuffer(cmd_queue, count, CL_TRUE, 0, sizeof(int),
                               &success_count, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueWriteBuffer", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_MEM;

    err = clSetKernelArg(kernel, 0, sizeof(int), &count);
    err |= clSetKernelArg(kernel, 1, sizeof(int), &num_of_chars);
    bResult &= SilentCheck("clSetKernelArg", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_MEM;

    size_t global_work_size[1];
    global_work_size[0] = WORK_SIZE;

    err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, global_work_size,
                                 NULL, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_MEM;

  } catch (int error) {
    bResult = false;
    switch (error) {
    case RELEASE_MEM:
      clReleaseMemObject(count);
      LLVM_FALLTHROUGH;
    case REOPEN_STDOUT:
      restore_stdout(fp, old_stdout);
      LLVM_FALLTHROUGH;
    case RELEASE_KERNEL:
      clReleaseKernel(kernel);
      LLVM_FALLTHROUGH;
    case RELEASE_PROGRAM:
      clReleaseProgram(program);
      throw RELEASE_QUEUE;
    default:
      throw error;
    }
  }
  clFinish(cmd_queue);
  restore_stdout(fp, old_stdout);
  err = clEnqueueReadBuffer(cmd_queue, count, CL_TRUE, 0, sizeof(int),
                            &success_count, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, err);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  if (success_count != WORK_SIZE) {
    // didn't printf as it should....
    printf("printf_special_chars test failed... \n");
    printf("number of successful work item executed: %d \n", success_count);
  }
  clReleaseMemObject(count);
  if (FilesCompareNdDelete("printf_sepcial_chars.txt",
                           "kernel_printf_sepcial_chars.txt")) {
    printf("SUCCESS: printf_special_chars \n");
  }
}

#define PRINTF_SIMPLE                                                          \
  __kernel void printf_simple() { printf("Hello\n"); }

void run_kernel_printf_simple(cl_context &context, cl_device_id &device,
                              cl_command_queue &cmd_queue, bool &bResult) {
  cl_int err;
  cl_kernel kernel = nullptr;
  cl_program program = nullptr;
  try {
    const char *ocl_test_program[] = {XSTR(PRINTF_SIMPLE)};

    program = clCreateProgramWithSource(
        context, 1, (const char **)&ocl_test_program, NULL, &err);
    bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_QUEUE;

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    cl_build_status build_status;
    err |= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
                                 MAX_SOURCE_SIZE, &build_status, NULL);
    if (err != CL_SUCCESS || build_status == CL_BUILD_ERROR) {
      printf("\n build status is: %d \n", build_status);
      char err_str[MAX_SOURCE_SIZE]; // instead of dynamic allocation
      char *err_str_ptr = err_str;
      err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  MAX_SOURCE_SIZE, err_str_ptr, NULL);
      if (err != CL_SUCCESS)
        printf("Build Info error: %d \n", err);
      printf("%s \n", err_str_ptr);
      throw RELEASE_QUEUE;
    }
    bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_QUEUE;

    kernel = clCreateKernel(program, "printf_simple", &err);
    bResult &= SilentCheck("clCreateKernel", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_PROGRAM;

    size_t global_work_size[1];
    global_work_size[0] = WORK_SIZE;

    err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, global_work_size,
                                 NULL, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_KERNEL;
  } catch (int error) {
    bResult = false;
    switch (error) {
      //        case RELEASE_MEM:
      //          clReleaseMemObject(count);
      //        case REOPEN_STDOUT:
      //          restore_stdout(fp,old_stdout);
    case RELEASE_KERNEL:
      clReleaseKernel(kernel);
      LLVM_FALLTHROUGH;
    case RELEASE_PROGRAM:
      clReleaseProgram(program);
      throw RELEASE_QUEUE;
    default:
      throw error;
    }
  }

  clFinish(cmd_queue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
}

#define PRINTF_INT                                                             \
  __kernel void printf_int(int n) {                                            \
    int tid = get_global_id(0);                                                \
    int printf_return;                                                         \
    printf_return = printf("printf_int , tid: %d \n", tid);                    \
    for (int i = 0; i < n; i++) {                                              \
      printf("%d", i);                                                         \
    }                                                                          \
    printf("\n done \n");                                                      \
  }

void run_kernel_printf_int(cl_context &context, cl_device_id &device,
                           cl_command_queue &cmd_queue, bool &bResult) {
  cl_int err;
  cl_kernel kernel = nullptr;
  cl_program program = nullptr;

  try {
    const char *ocl_test_program[] = {XSTR(PRINTF_INT)};
    program = clCreateProgramWithSource(
        context, 1, (const char **)&ocl_test_program, NULL, &err);
    bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_QUEUE;

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    cl_build_status build_status;

    err |= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
                                 MAX_SOURCE_SIZE, &build_status, NULL);
    if (err != CL_SUCCESS || build_status == CL_BUILD_ERROR) {
      printf("\n build status is: %d \n", build_status);
      char err_str[MAX_SOURCE_SIZE]; // instead of dynamic allocation
      char *err_str_ptr = err_str;
      err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  MAX_SOURCE_SIZE, err_str_ptr, NULL);
      if (err != CL_SUCCESS)
        printf("Build Info error: %d \n", err);
      printf("%s \n", err_str_ptr);
      throw RELEASE_PROGRAM;
    }
    bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_PROGRAM;

    kernel = clCreateKernel(program, "printf_int", &err);
    bResult &= SilentCheck("clCreateKernel", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_PROGRAM;

    int n = N;

    err = clSetKernelArg(kernel, 0, sizeof(int), &n);
    bResult &= SilentCheck("clSetKernelArg", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_KERNEL;

    size_t global_work_size[1];
    global_work_size[0] = WORK_SIZE;

    err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, global_work_size,
                                 NULL, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
    if (!bResult)
      throw REOPEN_STDOUT;
  } catch (int error) {
    bResult = false;
    switch (error) {
    case RELEASE_KERNEL:
      clReleaseKernel(kernel);
      LLVM_FALLTHROUGH;
    case RELEASE_PROGRAM:
      clReleaseProgram(program);
      throw REOPEN_STDOUT;
    default:
      throw error;
    }
  }

  clFinish(cmd_queue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
}

#define PRINTF_FORMAT                                                          \
  __kernel void printf_format(__global int *count, int num_of_chars) {         \
    int num = 1023456789;                                                      \
    int width = 30;                                                            \
    int precision = 30;                                                        \
    int len = printf("format check: \n");                                      \
    len += printf("regular int: %d \n", num);                                  \
    len += printf("left-justify: %-d \n", num);                                \
    len += printf("sign: %+d \n", num);                                        \
    len += printf("width : %30d \n", num);                                     \
    len += printf("zeros-width : %030d \n", num);                              \
    len += printf("external width : %0*d \n", width, num);                     \
    len += printf("short : %hd \n", (short)num);                               \
    len += printf("long : %ld \n", num);                                       \
    len += printf("all : %-+0*d \n", width, num);                              \
    float f_num = -3.1415926535897932384626433832795;                          \
    len += printf("regular float: %f \n", f_num);                              \
    len += printf("left-justify: %-f \n", f_num);                              \
    len += printf("sign: %+f \n", f_num);                                      \
    len += printf("width : %30f \n", f_num);                                   \
    len += printf("zeros-width : %030f \n", f_num);                            \
    len += printf("external width : %0*f \n", width, f_num);                   \
    len += printf("precision : %.30f \n", f_num);                              \
    len += printf("external precision : %.*f \n", precision, f_num);           \
    if (len == num_of_chars) {                                                 \
      atom_inc(count); /* conformance issue should be: atomic_inc(count); */   \
    }                                                                          \
  }

int printf_format() {
  int num = 1023456789;
  int width = 30;
  int precision = 30;
  int len = printf("format check: \n");
  len += printf("regular int: %d \n", num);
  len += printf("left-justify: %-d \n", num);
  len += printf("sign: %+d \n", num);
  len += printf("width : %30d \n", num);
  len += printf("zeros-width : %030d \n", num);
  len += printf("external width : %0*d \n", width, num);
  len += printf("short : %hd \n", (short)num);
  len += printf("long : %d \n", num);
  len += printf("all : %-+*d \n", width, num);
  float f_num = -3.1415926535897932384626433832795f;
  len += printf("regular float: %f \n", f_num);
  len += printf("left-justify: %-f \n", f_num);
  len += printf("sign: %+f \n", f_num);
  len += printf("width : %30f \n", f_num);
  len += printf("zeros-width : %030f \n", f_num);
  len += printf("external width : %0*f \n", width, f_num);
  len += printf("precision : %.30f \n", f_num);
  len += printf("external precision : %.*f \n", precision, f_num);

  return len;
}

void run_kernel_printf_format(cl_context &context, cl_device_id &device,
                              cl_command_queue &cmd_queue, bool &bResult) {
  cl_int err;
  cl_kernel kernel = nullptr;
  cl_program program = nullptr;
  cl_mem count = nullptr;
  int success_count = 0;
  FILE *fp = NULL;
  int old_stdout;

  try {
    const char *ocl_test_program[] = {XSTR(PRINTF_FORMAT)};
    program = clCreateProgramWithSource(
        context, 1, (const char **)&ocl_test_program, NULL, &err);
    bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_QUEUE;

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    cl_build_status build_status;

    err |= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
                                 MAX_SOURCE_SIZE, &build_status, NULL);
    if (err != CL_SUCCESS || build_status == CL_BUILD_ERROR) {
      printf("\n build status is: %d \n", build_status);
      char err_str[MAX_SOURCE_SIZE]; // instead of dynamic allocation
      char *err_str_ptr = err_str;
      err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  MAX_SOURCE_SIZE, err_str_ptr, NULL);
      if (err != CL_SUCCESS)
        printf("Build Info error: %d \n", err);
      printf("%s \n", err_str_ptr);
      throw RELEASE_QUEUE;
    }
    bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_QUEUE;

    kernel = clCreateKernel(program, "printf_format", &err);
    bResult &= SilentCheck("clCreateKernel", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_PROGRAM;

    bResult &= redirect_stdout(&fp, "printf_format.txt", "w", &old_stdout);
    if (!bResult) {
      printf("redirect stdout failed \n");
      throw RELEASE_KERNEL;
    }

    int num_of_chars = printf_format();

    restore_stdout(fp, old_stdout);

    bResult &=
        redirect_stdout(&fp, "kernel_printf_format.txt", "w", &old_stdout);
    if (!bResult) {
      printf("redirect stdout failed \n");
      throw RELEASE_KERNEL;
    }

    count = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
    bResult &= SilentCheck("clCreateBuffer", CL_SUCCESS, err);
    if (!bResult)
      throw REOPEN_STDOUT;
    err = clEnqueueWriteBuffer(cmd_queue, count, CL_TRUE, 0, sizeof(int),
                               &success_count, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueWriteBuffer", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_MEM;

    err = clSetKernelArg(kernel, 0, sizeof(int), &count);
    err |= clSetKernelArg(kernel, 1, sizeof(int), &num_of_chars);
    bResult &= SilentCheck("clSetKernelArg", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_MEM;

    size_t global_work_size[1];
    global_work_size[0] = WORK_SIZE;

    err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, global_work_size,
                                 NULL, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_MEM;

  } catch (int error) {
    bResult = false;
    switch (error) {
    case RELEASE_MEM:
      clReleaseMemObject(count);
      LLVM_FALLTHROUGH;
    case REOPEN_STDOUT:
      restore_stdout(fp, old_stdout);
      LLVM_FALLTHROUGH;
    case RELEASE_KERNEL:
      clReleaseKernel(kernel);
      LLVM_FALLTHROUGH;
    case RELEASE_PROGRAM:
      clReleaseProgram(program);
      throw RELEASE_QUEUE;
    default:
      throw error;
    }
  }
  clFinish(cmd_queue);
  restore_stdout(fp, old_stdout);
  err = clEnqueueReadBuffer(cmd_queue, count, CL_TRUE, 0, sizeof(int),
                            &success_count, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, err);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  if (success_count != WORK_SIZE) {
    // didn't printf as it should....
    printf("printf_special_chars test failed... \n");
    printf("number of successful work item executed: %d \n", success_count);
  }
  clReleaseMemObject(count);
  if (FilesCompareNdDelete("printf_format.txt", "kernel_printf_format.txt")) {
    printf("SUCCESS: printf_format \n");
  }
}

#define PRINTF_CHARS                                                           \
  __kernel void printf_chars(__global int *count, int num_of_chars) {          \
    int len = printf("char check: \n");                                        \
    char c = 0;                                                                \
    do {                                                                       \
      len += printf("%c \n", c);                                               \
      c++;                                                                     \
    } while (c != 0);                                                          \
    if (len == num_of_chars) {                                                 \
      atom_inc(count); /* conformance issue should be: atomic_inc(count); */   \
    }                                                                          \
  }
int printf_chars() {
  int len = printf("char check: \n");
  char c = 0;
  do {
    len += printf("%c \n", c);
    c++;
  } while (c != 0);

  return len;
}
void run_kernel_printf_chars(cl_context &context, cl_device_id &device,
                             cl_command_queue &cmd_queue, bool &bResult) {
  cl_int err;
  cl_kernel kernel = nullptr;
  cl_program program = nullptr;
  cl_mem count = nullptr;
  int success_count = 0;
  FILE *fp = NULL;
  int old_stdout;
  try {
    const char *ocl_test_program[] = {XSTR(PRINTF_CHARS)};
    program = clCreateProgramWithSource(
        context, 1, (const char **)&ocl_test_program, NULL, &err);
    bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_QUEUE;

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    cl_build_status build_status;

    err |= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
                                 MAX_SOURCE_SIZE, &build_status, NULL);
    if (err != CL_SUCCESS || build_status == CL_BUILD_ERROR) {
      printf("\n build status is: %d \n", build_status);
      char err_str[MAX_SOURCE_SIZE]; // instead of dynamic allocation
      char *err_str_ptr = err_str;
      err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  MAX_SOURCE_SIZE, err_str_ptr, NULL);
      if (err != CL_SUCCESS)
        printf("Build Info error: %d \n", err);
      printf("%s \n", err_str_ptr);
      throw RELEASE_QUEUE;
    }
    bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_QUEUE;

    kernel = clCreateKernel(program, "printf_chars", &err);
    bResult &= SilentCheck("clCreateKernel", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_PROGRAM;

    bResult &= redirect_stdout(&fp, "printf_chars.txt", "w", &old_stdout);
    if (!bResult) {
      printf("redirect stdout failed \n");
      throw RELEASE_KERNEL;
    }

    int num_of_chars = printf_chars();

    restore_stdout(fp, old_stdout);

    bResult &=
        redirect_stdout(&fp, "kernel_printf_chars.txt", "w", &old_stdout);
    if (!bResult) {
      printf("redirect stdout failed \n");
      throw RELEASE_KERNEL;
    }

    count = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
    bResult &= SilentCheck("clCreateBuffer", CL_SUCCESS, err);
    if (!bResult)
      throw REOPEN_STDOUT;
    err = clEnqueueWriteBuffer(cmd_queue, count, CL_TRUE, 0, sizeof(int),
                               &success_count, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueWriteBuffer", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_MEM;

    err = clSetKernelArg(kernel, 0, sizeof(int), &count);
    err |= clSetKernelArg(kernel, 1, sizeof(int), &num_of_chars);
    bResult &= SilentCheck("clSetKernelArg", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_MEM;

    size_t global_work_size[1];
    global_work_size[0] = WORK_SIZE;

    err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, global_work_size,
                                 NULL, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
    if (!bResult)
      throw RELEASE_MEM;

  } catch (int error) {
    bResult = false;
    switch (error) {
    case RELEASE_MEM:
      clReleaseMemObject(count);
      LLVM_FALLTHROUGH;
    case REOPEN_STDOUT:
      restore_stdout(fp, old_stdout);
      LLVM_FALLTHROUGH;
    case RELEASE_KERNEL:
      clReleaseKernel(kernel);
      LLVM_FALLTHROUGH;
    case RELEASE_PROGRAM:
      clReleaseProgram(program);
      throw RELEASE_QUEUE;
    default:
      throw error;
    }
  }
  clFinish(cmd_queue);
  restore_stdout(fp, old_stdout);
  err = clEnqueueReadBuffer(cmd_queue, count, CL_TRUE, 0, sizeof(int),
                            &success_count, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, err);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  if (success_count != WORK_SIZE) {
    // didn't printf as it should....
    printf("printf_special_chars test failed... \n");
    printf("number of successful work item executed: %d \n", success_count);
  }
  clReleaseMemObject(count);
  if (FilesCompareNdDelete("printf_chars.txt", "kernel_printf_chars.txt")) {
    printf("SUCCESS: printf_chars \n");
  }
}

// multi thread simple test , used only for debugging
class PrintfThread : public SynchronizedThread {
public:
  PrintfThread(cl_context context, cl_command_queue queue, cl_program program)
      : m_context(context), m_queue(queue), m_program(program) {}
  virtual ~PrintfThread() {}

protected:
  virtual void ThreadRoutine();

  cl_context m_context;
  cl_command_queue m_queue;
  cl_program m_program;
};

// multi thread simple test , used only for debugging
void PrintfThread::ThreadRoutine() {
  printf("start of ThreadRoutine \n");

  cl_int err;
  const size_t globalSize = 1;

  cl_kernel kernel = clCreateKernel(m_program, "printf_simple", &err);
  SilentCheck("clCreateKernel", CL_SUCCESS, err);

  err = clEnqueueNDRangeKernel(m_queue, kernel, 1, NULL, &globalSize, NULL, 0,
                               NULL, NULL);
  SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);

  clReleaseKernel(kernel);

  printf("end of ThreadRoutine \n");
}

// multi thread simple test , used only for debugging
bool MultithreadedPrintf() {
  bool bResult = true;
  cl_int iRet = CL_SUCCESS;

  const size_t numThreads = getMaxNumExternalThreads();

  cl_platform_id platform = 0;
  cl_device_id deviceId = 0;

  cl_context context;
  cl_command_queue queue;
  cl_program program;

  printf("changing stdout now.... \n");
  int old_stdout;
  FILE *fp;
  redirect_stdout(&fp, "threads_stdout.txt", "w", &old_stdout);
  printf("file: threads_stdout.txt: \n");

  const char *programSource = XSTR(PRINTF_SIMPLE);
  printf("Begin multi threaded printf test\n");

  iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // Creation phase
  context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
  bResult &=
      SilentCheck("Create Context from type (gDeviceType)", CL_SUCCESS, iRet);

  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &deviceId, NULL);
  bResult &= SilentCheck("Get device ID (gDeviceType)", CL_SUCCESS, iRet);

  queue = clCreateCommandQueueWithProperties(context, deviceId, NULL, &iRet);
  bResult &= SilentCheck("Create command queue", CL_SUCCESS, iRet);

  program = clCreateProgramWithSource(context, 1, &programSource, NULL, &iRet);
  bResult &= SilentCheck("Create program with source", CL_SUCCESS, iRet);

  iRet = clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);
  bResult &= SilentCheck("Build program", CL_SUCCESS, iRet);

  if (!bResult) {
    return false;
  }

  std::vector<SynchronizedThread *> threads(numThreads);
  for (size_t i = 0; i < numThreads; ++i) {
    threads[i] = new PrintfThread(context, queue, program);
  }

  SynchronizedThreadPool pool;
  pool.Init(&threads[0], numThreads);
  pool.StartAll();
  pool.WaitAll();

  for (size_t i = 0; i < numThreads; ++i) {
    delete threads[i];
  }

  clFinish(queue);
  printf("changing to stdout back.... \n");
  restore_stdout(fp, old_stdout);
  printf("back to stdout \n");
  fflush(stdout);
  clReleaseProgram(program);
  clReleaseCommandQueue(queue);
  clReleaseContext(context);

  return true;
}

bool printf_test() {
  printf("---------------------------------------\n");
  printf("printf_test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
  cl_command_queue cmd_queue = NULL;
  cl_device_id device = NULL;
  cl_int err;
  cl_platform_id platform = NULL;
  try {
    // init platform
    err = clGetPlatformIDs(1, &platform, NULL);
    bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, err);
    if (!bResult) {
      throw RELEASE_END;
    }

    // init Devices (only one CPU...)
    err = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
    bResult = SilentCheck("clGetDeviceIDs", CL_SUCCESS, err);
    if (!bResult) {
      throw RELEASE_END;
    }

    // init context
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    bResult &= SilentCheck("clCreateContext", CL_SUCCESS, err);
    if (!bResult) {
      throw RELEASE_END;
    }

    // init Command Queue
    cmd_queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
    bResult &=
        SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, err);
    if (!bResult) {
      throw RELEASE_CONTEXT;
    }

    // run_kernel_printf_simple(context,device,cmd_queue,bResult);

    run_kernel_printf_special_chars(context, device, cmd_queue, bResult);

    run_kernel_printf_format(context, device, cmd_queue, bResult);

    run_kernel_printf_chars(context, device, cmd_queue, bResult);

    throw FINAL_ROUTINE;
  } catch (int error) {
    switch (error) {
    case FINAL_ROUTINE:
    case RELEASE_QUEUE:
      clFinish(cmd_queue);
      fflush(stdout);
      clReleaseCommandQueue(cmd_queue);
      LLVM_FALLTHROUGH;
    case RELEASE_CONTEXT:
      clReleaseContext(context);
      LLVM_FALLTHROUGH;
    case RELEASE_END:
      return bResult;
    default:
      printf("no such error \n");
      return false;
    }
  }
}
