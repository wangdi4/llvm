// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

void test_MCU_builtins() {
  unsigned size = 23, count = 10;
  void *stdout;
  const char *str = "abacaba";
  void *buf1 = __builtin_malloc(size), *buf2 = __builtin_calloc(size, count);
  // CHECK: call i8* @malloc(i64 %{{.*}})
  // CHECK: call i8* @calloc(i64 %{{.*}}, i64 %{{.*}})
  __builtin_fwrite(buf1, size, count, stdout);
  // CHECK: call i64 @fwrite(i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i8* %{{.*}})
  __builtin_fputc(str[0], stdout);
  // CHECK: call i32 @fputc(i32 %{{.*}}, i8* %{{.*}})
  __builtin_puts(str);
  // CHECK: call i32 @puts(i8* %{{.*}})
  __builtin_fputs(str, stdout);
  // CHECK: call i32 @fputs(i8* %{{.*}}, i8* %{{.*}})
  int is_const = __builtin_constant_p(2 * (8 - 3) + size);
  __builtin_exit(is_const);
  // CHECK: call void @exit(i32 %{{.*}})
}
