; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-quick-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-mangled02-a.ll %S/Inputs/intel-merge-mangled02-b.ll %S/Inputs/intel-merge-mangled02-c.ll %S/Inputs/intel-merge-mangled02-d.ll | FileCheck %s

; This test case checks that the types were correctly merged when templates
; are used. It is the same test case as intel-merge-mangled02.ll (even uses
; the same input files), but it checks that the simple verifier works
; correctly. It was generated from the following example:

; simple.cpp:
;   #include "simple.h"
;
;   void foo(TestClass<int> *T, int I);
;   void bas(TestClass<double> *T, double I);
;
;   int bar() {
;     TestClass<int> T;
;     foo(&T, 10);
;     return T.getVal();
;   }
;
;   double caz() {
;     TestClass<double> T;
;     bas(&T, 10.0);
;     return T.getVal();
;   }

; simple2.cpp
;   #include "simple.h"
;
;   void foo(TestClass<int> *T, int I);
;   void bas(TestClass<double> *T, double I);
;
;   int barUpdate() {
;     TestClass<int> T;
;     foo(&T, 10);
;     return T.getVal();
;   }
;
;   double cazUpdate() {
;     TestClass<double> T;
;     bas(&T, 10.0);
;     return T.getVal();
;   }

; simple.h:
;   #ifndef __TESTCLASS_H__
;   #define __TESTCLASS_H__
;
;   template <class T>
;   class TestClass {
;   public:
;     TestClass() { }
;     void setVal(T I) { val = I; }
;     T getVal() { return val; }
;
;   private:
;     T val;
;   };
;
;   #endif // __TESTCLASS_H__


; foo.cpp:
;   #include "simple.h"
;
;   void foo(TestClass<int> *T, int I) {
;     T->setVal(I);
;   }

; bas.cpp:
;   #include "simple.h"
;
;   void bas(TestClass<double> *T, double I) {
;     T->setVal(I);
;   }


; CHECK: %class._ZTS9TestClassIiE.TestClass = type { i32 }
; CHECK: %class._ZTS9TestClassIdE.TestClass = type { double }

; CHECK-NOT: .TestClass.{{[0-9]+}} =

; CHECK: !intel.dtrans.types = !{!0, !2, !0, !2, !0, !2}

; CHECK: !0 = !{!"S", %class._ZTS9TestClassIiE.TestClass zeroinitializer, i32 1, !1}
; CHECK: !1 = !{i32 0, i32 0}
; CHECK: !2 = !{!"S", %class._ZTS9TestClassIdE.TestClass zeroinitializer, i32 1, !3}
; CHECK: !3 = !{double 0.000000e+00, i32 0}


;end INTEL_FEATURE_SW_DTRANS
