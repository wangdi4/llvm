; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-types-02a.ll %S/Inputs/intel-merge-types-02b.ll | FileCheck %s

; This test case checks that the types aren't merged during the IR mover since
; the pointer types don't match. The test case use template class. It
; represents the following C/C++ source code:

; file: simple.h
;   #ifndef __TESTCLASS_H__
;   #define __TESTCLASS_H__
;
;   template <class T>
;   class TestClass {
;   public:
;     TestClass() { }
;     void setVal(T I, int j) { val[j] = I; }
;     T getVal(int i) { return val[i]; }
;
;   private:
;     T *val;
;   };
;
;   #endif // __TESTCLASS_H__

; file: simple.cpp
;   #include "simple.h"
;
;   TestClass<int> *globInt;
;
;   void fooInt(int I, int j) {
;     return globInt->setVal(I, j);
;   }
;
;   int barInt(int i) {
;     return globInt->getVal(i);
; }

; file: simple.cpp
;   struct TestStruct {
;     int *ptr;
;   };
;
;   double bar(int i);
;
;   int foo(TestStruct *T, int i) {
;     return T->ptr[i] + (int)bar(i);
;   }

; file: simple2.cpp
;   #include "simple.h"
;
;   TestClass<int> *globInt;
;
;   void fooInt(int I, int j) {
;     return globInt->setVal(I, j);
;   }
;
;   int barInt(int i) {
;     return globInt->getVal(i);
;   }


; Check that both structures are in the IR

; CHECK: %class._ZTS9TestClassIiE.TestClass = type { ptr }
; CHECK: %class._ZTS9TestClassIdE.TestClass = type { ptr }

; Check that the DTrans metadata wasn't merged

; CHECK: !intel.dtrans.types = !{!2, !4}

; Metadata for the integer instantiation

; CHECK: !2 = !{!"S", %class._ZTS9TestClassIiE.TestClass zeroinitializer, i32 1, !3}
; CHECK: !3 = !{i32 0, i32 1}

; Metadata for the double instantiation

; CHECK: !4 = !{!"S", %class._ZTS9TestClassIdE.TestClass zeroinitializer, i32 1, !5}
; CHECK: !5 = !{double 0.000000e+00, i32 1}

; end INTEL_FEATURE_SW_DTRANS
