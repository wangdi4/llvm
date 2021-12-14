; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -irmover-enable-full-dtrans-types-check -debug-only=irmover-dtrans-types -S %S/Inputs/intel-merge-mangled04-a.ll %S/Inputs/intel-merge-mangled04-b.ll %S/Inputs/intel-merge-mangled04-c.ll %S/Inputs/intel-merge-mangled04-d.ll 2>&1 | FileCheck %s

; This test case checks that the types were correctly merged when there are
; anonymous union inside the class. It is the same test case as
; intel-merge-mangled04.ll but checks the debug information. The goal is to
; check that the anonymous union and templates were correctly merged. It was
; generated from the following example:

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
;     union {
;       T valInner;
;       int valInt;
;       double valDouble;
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

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-mangled04-a.ll
; CHECK:   Source type: %class._ZTS9TestClassIiE.TestClass = type { i32, %union._ZTSN9TestClassIiEUt_E.anon }
; CHECK:     Destination type: None
; CHECK:     Fields that will be repaired:
; CHECK:   Source type: %class._ZTS9TestClassIdE.TestClass = type { double, %union._ZTSN9TestClassIdEUt_E.anon }
; CHECK:     Destination type: None
; CHECK:     Fields that will be repaired:
; CHECK: Destination module passed verification

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-mangled04-b.ll
; CHECK:   Source type: %class._ZTS9TestClassIiE.TestClass.0 = type { i32, %union._ZTSN9TestClassIiEUt_E.anon.1 }
; CHECK:     Destination type: %class._ZTS9TestClassIiE.TestClass = type { i32, %union._ZTSN9TestClassIiEUt_E.anon }
; CHECK:   Source type: %class._ZTS9TestClassIdE.TestClass.2 = type { double, %union._ZTSN9TestClassIdEUt_E.anon.3 }
; CHECK:     Destination type: %class._ZTS9TestClassIdE.TestClass = type { double, %union._ZTSN9TestClassIdEUt_E.anon }
; CHECK: Destination module passed verification

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-mangled04-c.ll
; CHECK:   Source type: %class._ZTS9TestClassIiE.TestClass.4 = type { i32, %union._ZTSN9TestClassIiEUt_E.anon.5 }
; CHECK:     Destination type: %class._ZTS9TestClassIiE.TestClass = type { i32, %union._ZTSN9TestClassIiEUt_E.anon }
; CHECK: Destination module passed verification

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-mangled04-d.ll
; CHECK:   Source type: %class._ZTS9TestClassIdE.TestClass.6 = type { double, %union._ZTSN9TestClassIdEUt_E.anon.7 }
; CHECK:     Destination type: %class._ZTS9TestClassIdE.TestClass = type { double, %union._ZTSN9TestClassIdEUt_E.anon }
; CHECK: Destination module passed verification


; CHECK: %class._ZTS9TestClassIiE.TestClass = type { i32, %union._ZTSN9TestClassIiEUt_E.anon }
; CHECK: %union._ZTSN9TestClassIiEUt_E.anon = type { double }
; CHECK: %class._ZTS9TestClassIdE.TestClass = type { double, %union._ZTSN9TestClassIdEUt_E.anon }
; CHECK: %union._ZTSN9TestClassIdEUt_E.anon = type { double }

; CHECK-NOT: .TestClass.{{[0-9]+}} =
; CHECK-NOT: .anon.{{[0-9]+}} =

; CHECK: !intel.dtrans.types = !{!0, !3, !5, !7, !0, !3, !5, !7, !0, !3, !5, !7}

; CHECK: !0 = !{!"S", %class._ZTS9TestClassIiE.TestClass zeroinitializer, i32 2, !1, !2}
; CHECK: !1 = !{i32 0, i32 0}
; CHECK: !2 = !{%union._ZTSN9TestClassIiEUt_E.anon zeroinitializer, i32 0}
; CHECK: !3 = !{!"S", %union._ZTSN9TestClassIiEUt_E.anon zeroinitializer, i32 1, !4}
; CHECK: !4 = !{double 0.000000e+00, i32 0}
; CHECK: !5 = !{!"S", %class._ZTS9TestClassIdE.TestClass zeroinitializer, i32 2, !4, !6}
; CHECK: !6 = !{%union._ZTSN9TestClassIdEUt_E.anon zeroinitializer, i32 0}
; CHECK: !7 = !{!"S", %union._ZTSN9TestClassIdEUt_E.anon zeroinitializer, i32 1, !4}

;end INTEL_FEATURE_SW_DTRANS