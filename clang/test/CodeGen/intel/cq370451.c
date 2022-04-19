// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm -opaque-pointers -o - %s | FileCheck %s
#include <stdarg.h>

void test(void *f, const char *format, ... ) {
  va_list args;
  va_start(args, format);
  int a, n;
  char *s;
  __builtin_printf(format, a);
  // CHECK: call i32 (ptr, ...) @printf(ptr noundef {{.*}}, i32 noundef %{{.*}})
  __builtin_fprintf(f, format, a);
  // CHECK: call i32 (ptr, ptr, ...) @fprintf(ptr noundef %{{.*}}, ptr noundef %{{.*}}, i32 noundef %{{.*}})
  __builtin_sprintf(s, format, a);
  // CHECK: call i32 (ptr, ptr, ...) @sprintf(ptr noundef %{{.*}}, ptr noundef %{{.*}}, i32 noundef %{{.*}})
  __builtin_snprintf(s, n, format, a);
  // CHECK: call i32 (ptr, i64, ptr, ...) @snprintf(ptr noundef %{{.*}}, i64 noundef %{{.*}}, ptr noundef %{{.*}}, i32 noundef %{{.*}})
  __builtin_vprintf(format, args);
  // CHECK: call i32 @vprintf(ptr noundef %{{.*}}, ptr noundef %{{.*}})
  __builtin_vfprintf(f, format, args);
  // CHECK: call i32 @vfprintf(ptr noundef %{{.*}}, ptr noundef %{{.*}}, ptr noundef %{{.*}})
  __builtin_vsprintf(s, format, args);
  // CHECK: call i32 @vsprintf(ptr noundef %{{.*}}, ptr noundef %{{.*}}, ptr noundef %{{.*}})
  __builtin_vsnprintf(s, n, format, args);
  // CHECK: call i32 @vsnprintf(ptr noundef %{{.*}}, i64 noundef %{{.*}} ptr noundef %{{.*}}, ptr noundef %{{.*}})
  __builtin_scanf(format, &a);
  // CHECK: call i32 (ptr, ...) @scanf(ptr noundef {{.*}}, ptr noundef %{{.*}})
  __builtin_fscanf(f, format, &a);
  // CHECK: call i32 (ptr, ptr, ...) @fscanf(ptr noundef %{{.*}}, ptr noundef %{{.*}}, ptr noundef %{{.*}})
  __builtin_sscanf(s, format, &a);
  // CHECK: call i32 (ptr, ptr, ...) @sscanf(ptr noundef %{{.*}}, ptr noundef %{{.*}}, ptr noundef %{{.*}})
  __builtin_vscanf(format, args);
  // CHECK: call i32 @vscanf(ptr noundef %{{.*}}, ptr noundef %{{.*}})
  __builtin_vfscanf(f, format, args);
  // CHECK: call i32 @vfscanf(ptr noundef %{{.*}}, ptr noundef %{{.*}}, ptr noundef %{{.*}})
  __builtin_vsscanf(s, format, args);
  // CHECK: call i32 @vsscanf(ptr noundef %{{.*}}, ptr noundef %{{.*}}, ptr noundef %{{.*}})
  va_end(args);
}
