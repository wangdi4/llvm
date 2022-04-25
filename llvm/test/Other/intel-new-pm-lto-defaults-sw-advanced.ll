; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; This test case checks that the passes guarded with INTEL FEATURE SW_ADVANCED
; ran.

; RUN: opt -disable-verify -verify-cfg-preserved=0 -debug-pass-manager \
; RUN:     -passes='lto<O3>' -S  %s -passes-ep-peephole='no-op-function' 2>&1 \
; RUN:     | FileCheck %s --check-prefix=CHECK-O --check-prefix=CHECK-INTEL-SW-ADVANCED

; CHECK-INTEL-SW-ADVANCED: Running pass: IntelIPOPrefetchPass
; CHECK-INTEL-SW-ADVANCED: Running pass: IPArrayTransposePass on [module]
; CHECK-INTEL-SW-ADVANCED-NEXT: Running pass: DeadArrayOpsEliminationPass

; Make sure we get the IR back out without changes when we print the module.
; CHECK-O-LABEL: define void @foo(i32 %n) local_unnamed_addr {
; CHECK-O-NEXT: entry:
; CHECK-O-NEXT:   br label %loop
; CHECK-O:      loop:
; CHECK-O-NEXT:   %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
; CHECK-O-NEXT:   %iv.next = add i32 %iv, 1
; CHECK-O-NEXT:   tail call void @bar()
; CHECK-O-NEXT:   %cmp = icmp eq i32 %iv, %n
; CHECK-O-NEXT:   br i1 %cmp, label %exit, label %loop
; CHECK-O:      exit:
; CHECK-O-NEXT:   ret void
; CHECK-O-NEXT: }
;

declare void @bar() local_unnamed_addr

define void @foo(i32 %n) local_unnamed_addr {
entry:
  br label %loop
loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
  %iv.next = add i32 %iv, 1
  tail call void @bar()
  %cmp = icmp eq i32 %iv, %n
  br i1 %cmp, label %exit, label %loop
exit:
  ret void
}

; end INTEL_FEATURE_SW_ADVANCED
