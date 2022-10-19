; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -dtrans-inline-heuristics -intel-libirc-allowed -passes='cgscc(inline)' -inline-report=0xf847 -S 2>&1 | FileCheck %s --check-prefix=CHECK-CL
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xf8c6  < %s -S | opt -dtrans-inline-heuristics -intel-libirc-allowed -passes='cgscc(inline)' -inline-report=0xf8c6 -S | opt -passes='inlinereportemitter' -inline-report=0xf8c6 -S 2>&1 | FileCheck %s --check-prefix=CHECK-MD

target triple = "x86_64-unknown-linux-gnu"

; Check that foo() is not inlined when -dtrans-inline-heuristics -intel-libirc-allowed is specified,
; because it ends in an unreachable instruction. NOTE: The check for '>1'
; ensures that inlining was inhibited because the threshold was reduced to 1.

; CHECK-CL-LABEL: define i32 @main(
; CHECK-CL: call void @foo()
; CHECK-CL: call void @foo()
; CHECK-CL-LABEL: define internal void @foo(
; CHECK-CL: tail call void @exit(
; CHECK-CL: unreachable

; CHECK-CL-LABEL: COMPILE FUNC: foo
; CHECK-CL: EXTERN: exit
; CHECK-CL-LABEL: COMPILE FUNC: main
; CHECK-CL: foo {{.*}}>1 {{.*}}Inlining is not profitable
; CHECK-CL: foo {{.*}}>1 {{.*}}Inlining is not profitable

; CHECK-MD-LABEL: COMPILE FUNC: main
; CHECK-MD: foo {{.*}}>1 {{.*}}Inlining is not profitable
; CHECK-MD: foo {{.*}}>1 {{.*}}Inlining is not profitable
; CHECK-MD-LABEL: COMPILE FUNC: foo
; CHECK-MD: EXTERN: exit

; CHECK-MD-LABEL: define i32 @main(
; CHECK-MD: call void @foo()
; CHECK-MD: call void @foo()
; CHECK-MD-LABEL: define internal void @foo(
; CHECK-MD: tail call void @exit(
; CHECK-MD: unreachable

; Function Attrs: noreturn
declare dso_local void @exit(i32 noundef) #0

define i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @foo()
  call void @foo()
  ret i32 0
}

define internal void @foo() {
entry:
  tail call void @exit(i32 noundef 1)
  unreachable
}

attributes #0 = { noreturn }
; end INTEL_FEATURE_SW_ADVANCED
