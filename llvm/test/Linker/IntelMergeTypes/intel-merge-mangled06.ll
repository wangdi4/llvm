; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-mangled06-a.ll %S/Inputs/intel-merge-mangled06-b.ll %S/Inputs/intel-merge-mangled06-c.ll 2>&1 | FileCheck %s

; This test case checks that the types weren't merged even they have the same
; mangled typeinfo name. This happens when two structures have the same name,
; but they are in different modules. It was created from these sources:

; simple.cpp:
;   int foo(int I);
;   double bar(double I);
;
;   void initFoo(int I);
;   void initBar(double I);
;
;   int callFoo(int I) {
;     initFoo(I);
;     return foo(I);
;   }
;
;   double callBar(double I) {
;     initBar(I);
;     return bar(I);
;   }

; foo.cpp:
;   struct TestStruct {
;     int i;
;     int j;
;   };
;
;   TestStruct globFoo;
;
;   void initFoo(int I) {
;     globFoo.i = I;
;     globFoo.j = I + 100;
;   }
;
;   int foo(int I) {
;     return globFoo.i + globFoo.j + I;
;   }


; bar.cpp:
;   struct TestStruct {
;     double i;
;     double j;
;   };
;
;   TestStruct globBar;
;
;   void initBar(double I) {
;     globBar.i = I;
;     globBar.j = I + 100.0;
;   }
;
;   double bar(double I) {
;     return globBar.i + globBar.j + I;
;   }


; CHECK: %struct._ZTS10TestStruct.TestStruct = type { i32, i32 }
; CHECK: %struct._ZTS10TestStruct.TestStruct.0 = type { double, double }

; CHECK: !intel.dtrans.types = !{!0, !2}

; CHECK: !0 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 2, !1, !1}
; CHECK: !1 = !{i32 0, i32 0}
; CHECK: !2 = !{!"S", %struct._ZTS10TestStruct.TestStruct.0 zeroinitializer, i32 2, !3, !3}
; CHECK: !3 = !{double 0.000000e+00, i32 0}


;end INTEL_FEATURE_SW_DTRANS
