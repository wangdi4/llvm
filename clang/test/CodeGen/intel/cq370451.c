// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s
#include <stdarg.h>

void test(void *f, const char *format, ... ) {
  va_list args;
  va_start(args, format);
  int a, n;
  char *s;
  __builtin_printf(format, a);
  // CHECK: call i32 (i8*, ...) @printf(i8* {{.*}}, i32 %{{.*}})
  __builtin_fprintf(f, format, a);
  // CHECK: call i32 (i8*, i8*, ...) @fprintf(i8* %{{.*}}, i8* %{{.*}}, i32 %{{.*}})
  __builtin_sprintf(s, format, a);
  // CHECK: call i32 (i8*, i8*, ...) @sprintf(i8* %{{.*}}, i8* %{{.*}}, i32 %{{.*}})
  __builtin_snprintf(s, n, format, a);
  // CHECK: call i32 (i8*, i64, i8*, ...) @snprintf(i8* %{{.*}}, i64 %{{.*}}, i8* %{{.*}}, i32 %{{.*}})
  __builtin_vprintf(format, args);
  // CHECK: call i32 @vprintf(i8* %{{.*}}, %struct.__va_list_tag* %{{.*}})
  __builtin_vfprintf(f, format, args);
  // CHECK: call i32 @vfprintf(i8* %{{.*}}, i8* %{{.*}}, %struct.__va_list_tag* %{{.*}})
  __builtin_vsprintf(s, format, args);
  // CHECK: call i32 @vsprintf(i8* %{{.*}}, i8* %{{.*}}, %struct.__va_list_tag* %{{.*}})
  __builtin_vsnprintf(s, n, format, args);
  // CHECK: call i32 @vsnprintf(i8* %{{.*}}, i64 %{{.*}} i8* %{{.*}}, %struct.__va_list_tag* %{{.*}})
  __builtin_scanf(format, &a);
  // CHECK: call i32 (i8*, ...) @scanf(i8* {{.*}}, i32* %{{.*}})
  __builtin_fscanf(f, format, &a);
  // CHECK: call i32 (i8*, i8*, ...) @fscanf(i8* %{{.*}}, i8* %{{.*}}, i32* %{{.*}})
  __builtin_sscanf(s, format, &a);
  // CHECK: call i32 (i8*, i8*, ...) @sscanf(i8* %{{.*}}, i8* %{{.*}}, i32* %{{.*}})
  __builtin_vscanf(format, args);
  // CHECK: call i32 @vscanf(i8* %{{.*}}, %struct.__va_list_tag* %{{.*}})
  __builtin_vfscanf(f, format, args);
  // CHECK: call i32 @vfscanf(i8* %{{.*}}, i8* %{{.*}}, %struct.__va_list_tag* %{{.*}})
  __builtin_vsscanf(s, format, args);
  // CHECK: call i32 @vsscanf(i8* %{{.*}}, i8* %{{.*}}, %struct.__va_list_tag* %{{.*}})
  va_end(args);
}
