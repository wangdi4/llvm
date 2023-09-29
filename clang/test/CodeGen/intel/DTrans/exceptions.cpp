// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -fwhole-program-vtables -fexceptions -fcxx-exceptions -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
  class bad_alloc
  {
  public:
    bad_alloc() throw() { }
    bad_alloc(const bad_alloc&) = default;
    bad_alloc& operator=(const bad_alloc&) = default;
    virtual ~bad_alloc() throw();
    virtual const char* what() const throw();
  };

class XalanDummyMemoryManager
{
public:
 virtual
 ~XalanDummyMemoryManager()
 {
 }
 virtual
   void
 allocate( )
 {
  throw bad_alloc();
 }
 virtual void throw_int() {
   throw 5;
 }
};
static XalanDummyMemoryManager s_dummyMemMgr;

// CHECK: @_ZTVN10__cxxabiv117__class_type_infoE = external global [0 x ptr], !intel_dtrans_type ![[ZERO_PTR:[0-9]+]]
// CHECK: @_ZTI9bad_alloc = external constant ptr, !intel_dtrans_type ![[CHAR_PTR:[0-9]+]]
// CHECK: @_ZTV9bad_alloc = {{.+}}{ [5 x ptr] }{{.+}} !intel_dtrans_type ![[BAD_ALLOC_ARRAY:[0-9]+]]
// CHECK: @_ZTIi = external constant ptr, !intel_dtrans_type ![[CHAR_PTR]]
// CHECK: @llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @{{.+}}, ptr null }], !intel_dtrans_type ![[GLOBAL_CTORS_TYPE:[0-9]+]]
// CHECK: @llvm.compiler.used = appending global [1 x ptr] [ptr @_ZTV9bad_alloc], section "llvm.metadata", !intel_dtrans_type ![[USED_TYPE:[0-9]+]]
// CHECK: declare !intel.dtrans.func.type ![[BAD_ALLOC_D1:[0-9]+]] void @_ZN9bad_allocD1Ev(ptr {{.+}} "intel_dtrans_func_index"="1")
// CHECK: declare !intel.dtrans.func.type ![[BAD_ALLOC_D0:[0-9]+]] void @_ZN9bad_allocD0Ev(ptr {{.+}} "intel_dtrans_func_index"="1")
// CHECK: declare !intel.dtrans.func.type ![[BAD_ALLOC_WHAT:[0-9]+]] noundef "intel_dtrans_func_index"="1" ptr @_ZNK9bad_alloc4whatEv(ptr {{.+}} "intel_dtrans_func_index"="2")


// CHECK-DAG: ![[ZERO_PTR]] = !{!"A", i32 0, ![[CHAR_PTR]]}
// CHECK-DAG: ![[CHAR_PTR]] = !{i8 0, i32 1}
// CHECK-DAG: ![[BAD_ALLOC_ARRAY]] = !{!"L", i32 1, ![[CHAR_5_ARR:[0-9]+]]}
// CHECK-DAG: ![[CHAR_5_ARR]] = !{!"A", i32 5, ![[CHAR_PTR]]}
// CHECK: ![[GLOBAL_CTORS_TYPE]] = !{!"A", i32 1, ![[GLOBAL_CTORS_LIT_REF:[0-9]+]]}
// CHECK: ![[GLOBAL_CTORS_LIT_REF]] = !{![[GLOBAL_CTORS_LIT:[0-9]+]], i32 0}
// CHECK: ![[GLOBAL_CTORS_LIT]] = !{!"L", i32 3, ![[INT:[0-9]+]], ![[FPTR:[0-9]+]], ![[CHAR_PTR]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[FPTR]] = !{![[FUNC:[0-9]+]], i32 1}
// CHECK: ![[FUNC]] = !{!"F", i1 false, i32 0, ![[VOID:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[USED_TYPE]] = !{!"A", i32 1, ![[CHAR_PTR]]}
// CHECK: ![[BAD_ALLOC_PTR:[0-9]+]] = !{%class._ZTS9bad_alloc.bad_alloc zeroinitializer, i32 1}
// CHECK: ![[BAD_ALLOC_D1]] = distinct !{![[BAD_ALLOC_PTR]]}
// CHECK: ![[BAD_ALLOC_D0]] = distinct !{![[BAD_ALLOC_PTR]]}
// CHECK: ![[BAD_ALLOC_WHAT]] = distinct !{![[CHAR_PTR]], ![[BAD_ALLOC_PTR]]}
