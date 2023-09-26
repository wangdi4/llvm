; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -debug-only=irmover-dtrans-types -S %s %S/Inputs/intel-merge-mangled01-a.ll %S/Inputs/intel-merge-mangled01-b.ll 2>&1 | FileCheck %s

; This is a very simple test case that checks if the class TestClass was merged
; correctly when linking intel-merge-mangled01-a.ll with
; intel-merge-mangled01-b.ll. It is the same test case as
; intel-merge-mangled01.ll but it checks the debug information. The example was
; created from this test case:

; simple.cpp:
;   #include "simple.h"
;
;   void foo(TestClass *T, int I);
;
;   int bar() {
;     TestClass T;
;     foo(&T, 10);
;     return T.getVal();
;   }


; simple.h:
;   #ifndef __TESTCLASS_H__
;   #define __TESTCLASS_H__
;
;   class TestClass {
;   public:
;     TestClass() : val(0) { }
;     void setVal(int I) { val = I; }
;     int getVal() { return val; }
;
;   private:
;     int val;
;   };
;
;   #endif // __TESTCLASS_H__


; foo.cpp:
;   #include "simple.h"
;
;   void foo(TestClass *T, int I) {
;     T->setVal(I);
;   }


; CHECK: Merging types from source module:
; CHECK-SAME: Inputs/intel-merge-mangled01-a.ll
; CHECK:   Source type: %class._ZTS9TestClass.TestClass = type { i32 }
; CHECK:     Destination type: None
; CHECK: Running destination module verifier
; CHECK: Destination module passed verification
; CHECK: -------------------------------------------------------

; CHECK: Merging types from source module:
; CHECK-SAME: Inputs/intel-merge-mangled01-b.ll
; CHECK:   Source type: %class._ZTS9TestClass.TestClass.0 = type { i32 }
; CHECK:     Destination type: %class._ZTS9TestClass.TestClass = type { i32 }
; CHECK: Running destination module verifier
; CHECK: Destination module passed verification
; CHECK: -------------------------------------------------------


; CHECK: %class._ZTS9TestClass.TestClass = type { i32 }
; CHECK-NOT: .TestClass.{{[0-9]+}} =

; CHECK: !intel.dtrans.types = !{!0, !0}
; CHECK: !0 = !{!"S", %class._ZTS9TestClass.TestClass zeroinitializer, i32 1, !1}

;end INTEL_FEATURE_SW_DTRANS
