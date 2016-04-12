// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

#ifdef CPP
extern "C++" {
#endif
const char *func;
const char *function;
const char *pretty_function;

void foo()
{
  func = &__func__[0];
  function = &__FUNCTION__[0];
  pretty_function = &__PRETTY_FUNCTION__[0];
}

#ifdef CPP
}
#endif
// CHECK: [[__func__:@.+]] = {{.+}} [4 x i8] c"foo\00"
// CHECK: [[func:@.+]] = common global i8* null
// CHECK: [[function:@.+]] = common global i8* null
// CHECK: [[pretty_function:@.+]] = common global i8* null
// CHECK: store i8* getelementptr inbounds ([4 x i8], [4 x i8]* [[__func__]], i64 0, i64 0), i8** [[func]]
// CHECK: store i8* getelementptr inbounds ([4 x i8], [4 x i8]* [[__func__]], i64 0, i64 0), i8** [[function]]
// CHECK: store i8* getelementptr inbounds ([4 x i8], [4 x i8]* [[__func__]], i64 0, i64 0), i8** [[pretty_function]]
