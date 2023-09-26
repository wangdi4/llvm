; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -debug-only=irmover-dtrans-types -S %S/Inputs/intel-merge-mangled08-a.ll %S/Inputs/intel-merge-mangled08-b.ll %S/Inputs/intel-merge-mangled08-c.ll %S/Inputs/intel-merge-mangled08-d.ll 2>&1 | FileCheck %s

; This test case checks that the types were correctly merged when templates
; are used in Windows. It is the same test case as intel-merge-mangled08.ll but
; checks the debug information. The goal is to check the IR mover correctly
; merged the different versions of the template class TestClass and it
; doesn't generate alternate names. It was generated from the following
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
; CHECK-SAME: Inputs/intel-merge-mangled08-a.ll
; CHECK:   Source type: %"class..?AV?$TestClass@H@@.TestClass" = type { i32 }
; CHECK:     Destination type: None
; CHECK:   Source type: %"class..?AV?$TestClass@N@@.TestClass" = type { double }
; CHECK:     Destination type: None
; CHECK: -------------------------------------------------------

; CHECK: Merging types from source module:
; CHECK-SAME: Inputs/intel-merge-mangled08-b.ll
; CHECK:   Source type: %"class..?AV?$TestClass@H@@.TestClass.0" = type { i32 }
; CHECK:     Destination type: %"class..?AV?$TestClass@H@@.TestClass" = type { i32 }
; CHECK:   Source type: %"class..?AV?$TestClass@N@@.TestClass.1" = type { double }
; CHECK:     Destination type: %"class..?AV?$TestClass@N@@.TestClass" = type { double }
; CHECK: -------------------------------------------------------

; CHECK: Merging types from source module:
; CHECK-SAME: Inputs/intel-merge-mangled08-c.ll
; CHECK:   Source type: %"class..?AV?$TestClass@H@@.TestClass.2" = type { i32 }
; CHECK:     Destination type: %"class..?AV?$TestClass@H@@.TestClass" = type { i32 }
; CHECK: -------------------------------------------------------

; CHECK: Merging types from source module:
; CHECK-SAME: Inputs/intel-merge-mangled08-d.ll
; CHECK:   Source type: %"class..?AV?$TestClass@N@@.TestClass.3" = type { double }
; CHECK:     Destination type: %"class..?AV?$TestClass@N@@.TestClass" = type { double }
; CHECK: -------------------------------------------------------


; CHECK: %"class..?AV?$TestClass@H@@.TestClass" = type { i32 }
; CHECK: %"class..?AV?$TestClass@N@@.TestClass" = type { double }

; CHECK-NOT: .TestClass.{{[0-9]+}} =


; CHECK: !intel.dtrans.types = !{!6, !8, !6, !8, !6, !8}


; CHECK: !6 = !{!"S", %"class..?AV?$TestClass@H@@.TestClass" zeroinitializer, i32 1, !7}
; CHECK: !7 = !{i32 0, i32 0}
; CHECK: !8 = !{!"S", %"class..?AV?$TestClass@N@@.TestClass" zeroinitializer, i32 1, !9}
; CHECK: !9 = !{double 0.000000e+00, i32 0}


;end INTEL_FEATURE_SW_DTRANS
