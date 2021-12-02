// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -fexceptions -fcxx-exceptions -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s -check-prefixes=CHECK,LIN
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-pc -fexceptions -fcxx-exceptions -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s -check-prefixes=CHECK,WIN

// Validate that 'runtime functions' get the proper DTRans metadata generated
// for them, so we don't pessimize in optimization passes.

// LIN: @IP = external global i32*, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
// WIN: @"?IP@@3PEAHEA" = external dso_local global i32*, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
extern int *IP;

// LIN: @_ZZ11static_initE1i = {{.+}}!intel_dtrans_type ![[CHAR_PTR:[0-9]+]]
// WIN: @"?i@?1??static_init@@9@4PEADEA" = {{.+}}!intel_dtrans_type ![[CHAR_PTR:[0-9]+]]

// CHECK: define dso_local "intel_dtrans_func_index"="1" i32* @callee(){{.+}}!intel.dtrans.func.type ![[CALLEE:[0-9]+]]
extern "C" __attribute__((noinline)) int *callee() {
  if (IP)
    throw 0;
  return IP;
}

// LIN: declare !intel.dtrans.func.type ![[ALLOC_EXCEPT:[0-9]+]] "intel_dtrans_func_index"="1" i8* @__cxa_allocate_exception(i64)
// LIN: declare !intel.dtrans.func.type ![[THROW:[0-9]+]] void @__cxa_throw(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i8* "intel_dtrans_func_index"="3")
// WIN: declare !intel.dtrans.func.type ![[THROW:[0-9]+]] dso_local void @_CxxThrowException(i8* "intel_dtrans_func_index"="1", %eh.ThrowInfo* "intel_dtrans_func_index"="2")

// CHECK: define dso_local "intel_dtrans_func_index"="1" i32* @caller(){{.+}}!intel.dtrans.func.type ![[CALLER:[0-9]+]]
extern "C" int *caller(void) {
  try {
    int *p = callee();
    return p;
  } catch(int i) {
    throw;
  } catch (...) {

    return IP;
  }
}

// Windows uses the catchpad stuff, and just calls _CxxThrowException for rethrow.
// LIN: declare !intel.dtrans.func.type ![[BEGIN_CATCH:[0-9]+]] "intel_dtrans_func_index"="1" i8* @__cxa_begin_catch(i8* "intel_dtrans_func_index"="2")
// LIN: declare void @__cxa_end_catch()
// LIN: declare void @__cxa_rethrow()

struct A {
  virtual void foo();
};
struct B : A {};

// CHECK: define dso_local "intel_dtrans_func_index"="1" %{{\"?}}struct.{{.+}}.B{{\"?}}* @calls_dyn_cast(%{{\"?}}struct.{{.+}}.A{{\"?}}* "intel_dtrans_func_index"="2" %{{.+}}){{.+}}!intel.dtrans.func.type ![[CALLS_DYN_CAST:[0-9]+]]

extern "C" B* calls_dyn_cast(A* a) {
  return dynamic_cast<B*>(a);
}
// LIN: declare !intel.dtrans.func.type ![[DYN_CAST:[0-9]+]] "intel_dtrans_func_index"="1" i8* @__dynamic_cast(i8* "intel_dtrans_func_index"="2", i8* "intel_dtrans_func_index"="3", i8* "intel_dtrans_func_index"="4", i64)
// WIN: declare !intel.dtrans.func.type ![[DYN_CAST:[0-9]+]] dso_local "intel_dtrans_func_index"="1" i8* @__RTDynamicCast(i8* "intel_dtrans_func_index"="2", i32, i8* "intel_dtrans_func_index"="3", i8* "intel_dtrans_func_index"="4", i32)

extern "C" char *get_i();

// CHECK: define dso_local void @static_init()
extern "C" void static_init() {
  static char *i = get_i();
}

// LIN: declare !intel.dtrans.func.type ![[GUARD_ACQ:[0-9]+]] i32 @__cxa_guard_acquire(i64* "intel_dtrans_func_index"="1")
// WIN: declare !intel.dtrans.func.type ![[GUARD_ACQ:[0-9]+]] dso_local void @_Init_thread_header(i32* "intel_dtrans_func_index"="1")

// CHECK: declare !intel.dtrans.func.type ![[GET_IV:[0-9]+]] {{.*}}"intel_dtrans_func_index"="1" i8* @get_i()

// LIN: declare !intel.dtrans.func.type ![[GUARD_ABORT:[0-9]+]] void @__cxa_guard_abort(i64* "intel_dtrans_func_index"="1")
// WIN: declare !intel.dtrans.func.type ![[GUARD_ABORT:[0-9]+]] dso_local void @_Init_thread_abort(i32* "intel_dtrans_func_index"="1")

// LIN: declare !intel.dtrans.func.type ![[GUARD_REL:[0-9]+]] void @__cxa_guard_release(i64* "intel_dtrans_func_index"="1")
// WIN: declare !intel.dtrans.func.type ![[GUARD_REL:[0-9]+]] dso_local void @_Init_thread_footer(i32* "intel_dtrans_func_index"="1")

// LIN: !intel.dtrans.types = !{![[STRUCTB:[0-9]+]], ![[STRUCTA:[0-9]+]]}
// WIN: !intel.dtrans.types = !{![[EH_THROWINFO:[0-9]+]], ![[STRUCTB:[0-9]+]], ![[STRUCTA:[0-9]+]]}

// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
// WIN: ![[EH_THROWINFO]] = !{!"S", %eh.ThrowInfo zeroinitializer, i32 4, ![[INT:[0-9]+]], ![[INT]], ![[INT]], ![[INT]]}
// WIN: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[STRUCTB]] = !{!"S", %{{\"?}}struct.{{.+}}.B{{\"?}} zeroinitializer, i32 1, ![[STRUCTA_REF:[0-9]+]]}
// CHECK: ![[STRUCTA_REF]] = !{%{{\"?}}struct.{{.+}}.A{{\"?}} zeroinitializer, i32 0}
// CHECK: ![[STRUCTA]] = !{!"S", %{{\"?}}struct.{{.+}}.A{{\"?}} zeroinitializer, i32 1, ![[FOO_PTR:[0-9]+]]}
// CHECK: ![[FOO_PTR]] = !{![[FOO:[0-9]+]], i32 2}
// LIN: ![[FOO]] = !{!"F", i1 true, i32 0, ![[INT:[0-9]+]]
// WIN: ![[FOO]] = !{!"F", i1 true, i32 0, ![[INT]]
// LIN: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[CALLEE]] = distinct !{![[INT_PTR]]}
// LIN: ![[ALLOC_EXCEPT]] = distinct !{![[CHAR_PTR]]}
// LIN: ![[THROW]] = distinct !{![[CHAR_PTR]], ![[CHAR_PTR]], ![[CHAR_PTR]]}
// WIN: ![[THROW]] = distinct !{![[CHAR_PTR]], ![[THROW_INFO:[0-9]+]]}
// WIN: ![[THROW_INFO]] = !{%eh.ThrowInfo zeroinitializer, i32 1}
// CHECK: ![[CALLER]] = distinct !{![[INT_PTR]]}
// LIN: ![[BEGIN_CATCH]] = distinct !{![[CHAR_PTR]], ![[CHAR_PTR]]}
// CHECK: ![[CALLS_DYN_CAST]] = distinct !{![[B_PTR:[0-9]+]], ![[A_PTR:[0-9]+]]}
// CHECK: ![[B_PTR]] = !{%{{\"?}}struct.{{.+}}.B{{\"?}} zeroinitializer, i32 1}
// CHECK: ![[A_PTR]] = !{%{{\"?}}struct.{{.+}}.A{{\"?}} zeroinitializer, i32 1}
// CHECK: ![[DYN_CAST]] = distinct !{![[CHAR_PTR]], ![[CHAR_PTR]], ![[CHAR_PTR]], ![[CHAR_PTR]]}
// LIN: ![[GUARD_ACQ]] = distinct !{![[I64_PTR:[0-9]+]]}
// WIN: ![[GUARD_ACQ]] = distinct !{![[INT_PTR]]}
// LIN: ![[I64_PTR]] = !{i64 0, i32 1}
// CHECK: ![[GET_IV]] = distinct !{![[CHAR_PTR]]}
// LIN: ![[GUARD_ABORT]] = distinct !{![[I64_PTR:[0-9]+]]}
// WIN: ![[GUARD_ABORT]] = distinct !{![[INT_PTR]]}
// LIN: ![[GUARD_REL]] = distinct !{![[I64_PTR:[0-9]+]]}
// WIN: ![[GUARD_REL]] = distinct !{![[INT_PTR]]}
