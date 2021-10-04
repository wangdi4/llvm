; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-by-mangled-names -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-mangled05-a.ll %S/Inputs/intel-merge-mangled05-b.ll %S/Inputs/intel-merge-mangled05-c.ll %S/Inputs/intel-merge-mangled05-d.ll 2>&1 | FileCheck %s

; This test case checks that the types were correctly merged when there is an
; anonymous class inside the class. The goal is to check that the
; anonymous class wasn't merged. It was generated from the following
; example:

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
;     class {
;       public:
;         T valInner;
;         int valInt;
;         double valDouble;
;     };
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


; CHECK: %class._ZTS9TestClassIiE.TestClass = type { i32, %class._ZTSN9TestClassIiEUt_E.anon }
; CHECK: %class._ZTSN9TestClassIiEUt_E.anon = type { i32, i32, double }
; CHECK: %class._ZTS9TestClassIdE.TestClass = type { double, %class._ZTSN9TestClassIdEUt_E.anon }
; CHECK: %class._ZTSN9TestClassIdEUt_E.anon = type { double, i32, double }

; CHECK-NOT: .TestClass.{{[0-9]+}} =
; CHECK-NOT: .anon.{{[0-9]+}} =

; CHECK:!intel.dtrans.types = !{!0, !3, !5, !7, !0, !3, !5, !7, !0, !3, !5, !7}

; CHECK:!0 = !{!"S", %class._ZTS9TestClassIiE.TestClass zeroinitializer, i32 2, !1, !2}
; CHECK:!1 = !{i32 0, i32 0}
; CHECK:!2 = !{%class._ZTSN9TestClassIiEUt_E.anon zeroinitializer, i32 0}
; CHECK:!3 = !{!"S", %class._ZTSN9TestClassIiEUt_E.anon zeroinitializer, i32 3, !1, !1, !4}
; CHECK:!4 = !{double 0.000000e+00, i32 0}
; CHECK:!5 = !{!"S", %class._ZTS9TestClassIdE.TestClass zeroinitializer, i32 2, !4, !6}
; CHECK:!6 = !{%class._ZTSN9TestClassIdEUt_E.anon zeroinitializer, i32 0}
; CHECK:!7 = !{!"S", %class._ZTSN9TestClassIdEUt_E.anon zeroinitializer, i32 3, !4, !1, !4}


;end INTEL_FEATURE_SW_DTRANS