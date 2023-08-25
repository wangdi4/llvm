// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

struct Foo {
  int i = 0; int j = 0;
};

struct StringRef {
  Foo f={};
  const char * Data;
  unsigned long len;
  int a= 0;

  constexpr StringRef(const char* data, unsigned long len) :
    Data(data), len(len){}
};

struct StringLiteral : StringRef {

  template<unsigned long N>
    constexpr StringLiteral(const char (&Str)[N]) : StringRef(Str, N - 1){}

  int j = 0;
};

struct PassThrough : StringLiteral {
  constexpr PassThrough() :StringLiteral("qwer"){}
};

StringLiteral L[] = {"asdf"};
// CHECK: @L = global [1 x { %struct._ZTS3Foo.Foo, ptr, i64, i32, i32 }]{{.*}} !intel_dtrans_type ![[L_TY:[0-9]+]]

StringRef R[] = {{"asdf", 4}};
// CHECK: @R = global [1 x { %struct._ZTS3Foo.Foo, ptr, i64, i32 }]{{.*}} !intel_dtrans_type ![[R_TY:[0-9]+]]

PassThrough P[]= {{}};
// CHECK: @P = global [1 x { %struct._ZTS3Foo.Foo, ptr, i64, i32, i32 }]{{.*}} !intel_dtrans_type ![[L_TY:[0-9]+]]

// CHECK: ![[L_TY]] = !{!"A", i32 1, ![[LIT_WRAP:[0-9]+]]
// CHECK: ![[LIT_WRAP]] = !{![[LIT:[0-9]+]], i32 0}
// CHECK: ![[LIT]] = !{!"L", i32 5, ![[FOO_INIT:[0-9]+]], ![[CHAR_PTR:[0-9]+]], ![[I64:[0-9]+]], ![[I32:[0-9]+]], ![[I32]]}
// CHECK: ![[FOO_INIT]] = !{%struct._ZTS3Foo.Foo zeroinitializer, i32 0}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
// CHECK: ![[I64]] = !{i64 0, i32 0}
// CHECK: ![[I32]] = !{i32 0, i32 0}
// CHECK: ![[R_TY]] = !{!"A", i32 1, ![[LIT_WRAP2:[0-9]+]]
// CHECK: ![[LIT_WRAP2]] = !{![[LIT2:[0-9]+]], i32 0}
// CHECK: ![[LIT2]] = !{!"L", i32 4, ![[FOO_INIT]], ![[CHAR_PTR]], ![[I64]], ![[I32]]}
