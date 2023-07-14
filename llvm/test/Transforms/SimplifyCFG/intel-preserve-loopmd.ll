; Verify that we preserve loop info when creating a select/branch

; RUN: opt %s -passes=simplifycfg -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

define void @ZTSZ4mainE3FSM() {
entry:
  br label %while

select.unfold:                                    ; preds = %if.end
  br label %while, !llvm.loop !0

; CHECK-LABEL: while:
while:                                            ; preds = %if.end, %select.unfold, %entry
  %CS = phi i32 [ 0, %entry ], [ 0, %if.end ], [ 2, %select.unfold ]
  %cmp = icmp eq i32 %CS, 1
  br label %if.end

if.end:                                           ; preds = %while
; CHECK: br label %while, !llvm.loop [[LL:![0-9][0-9]*]]
  br i1 %cmp, label %select.unfold, label %while, !llvm.loop !0
}

; CHECK: [[LL]] = distinct !{[[LL]], [[LLII:!.*]]}
!0 = distinct !{!0, !1}
; CHECK: [[LLII]] = !{!"llvm.loop.ii.count", i32 1}
!1 = !{!"llvm.loop.ii.count", i32 1}
