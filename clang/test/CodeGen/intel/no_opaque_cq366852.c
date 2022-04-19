// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s
// expected-no-diagnostics

void test_MCU_builtins() {
  unsigned size = 23, count = 10;
  void *stdout;
  const char *str = "abacaba";
  void *buf1 = __builtin_malloc(size), *buf2 = __builtin_calloc(size, count);
  // CHECK: call i8* @malloc(i64 noundef %{{.*}})
  // CHECK: call i8* @calloc(i64 noundef %{{.*}}, i64 noundef %{{.*}})
  __builtin_fwrite(buf1, size, count, stdout);
  // CHECK: call i64 @fwrite(i8* noundef %{{.*}}, i64 noundef %{{.*}}, i64 noundef %{{.*}}, i8* noundef %{{.*}})
  __builtin_fwrite_unlocked(buf1, size, count, stdout);
  // CHECK: call i64 @fwrite_unlocked(i8* noundef %{{.*}}, i64 noundef %{{.*}}, i64 noundef %{{.*}}, i8* noundef %{{.*}})
  __builtin_fputc(str[0], stdout);
  // CHECK: call i32 @fputc(i32 noundef %{{.*}}, i8* noundef %{{.*}})
  __builtin_fputc_unlocked(str[0], stdout);
  // CHECK: call i32 @fputc_unlocked(i32 noundef %{{.*}}, i8* noundef %{{.*}})
  __builtin_puts(str);
  // CHECK: call i32 @puts(i8* noundef %{{.*}})
  __builtin_fputs(str, stdout);
  // CHECK: call i32 @fputs(i8* noundef %{{.*}}, i8* noundef %{{.*}})
  __builtin_fputs_unlocked(str, stdout);
  // CHECK: call i32 @fputs_unlocked(i8* noundef %{{.*}}, i8* noundef %{{.*}})
  __builtin_puts_unlocked(str);
  // CHECK: call i32 @puts_unlocked(i8* noundef %{{.*}})
  int is_const = __builtin_constant_p(2 * (8 - 3) + size);
  __builtin_exit(is_const);
  // CHECK: call void @exit(i32 noundef %{{.*}})
}
