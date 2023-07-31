; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-general-unroll,print<hir>" -hir-cost-model-throttling=0 < %s 2>&1 | FileCheck %s

; Verify that non-profitable (no reuse) unknown loop is unrolled due to presence of "llvm.loop.unroll.enable" metadata.

; + UNKNOWN LOOP i1
; |   <i1 = 0>
; |   loop:
; |   %t523 = (%t521)[i1];
; |   if (&((%t523)[0]) != null)
; |   {
; |      @free(&((%t523)[0]));
; |      %t527 = (@g_inputLine)[0];
; |      %t519 = (@rule_count)[0];
; |      %t521 = &((%t527)[0]);
; |   }
; |   if (i1 + 1 < %t519)
; |   {
; |      <i1 = i1 + 1>
; |      goto loop;
; |   }
; + END LOOP

; CHECK: + UNKNOWN LOOP i1 <unroll>
; CHECK: + END LOOP

; CHECK: REGION { modified }
; CHECK: + UNKNOWN LOOP i1 <nounroll>
; CHECK: + END LOOP


@rule_count = internal unnamed_addr global i32 0, align 4
@g_inputLine = internal unnamed_addr global ptr null, align 8

declare void @free(ptr nocapture)

define void @foo(i32 %t515, ptr %t512) {
entry:
  br label %loop

loop:                                    ; preds = %entry, %backedge
  %t519 = phi i32 [ %t530, %backedge ], [ %t515, %entry ]
  %t520 = phi i64 [ %t531, %backedge ], [ 0, %entry ]
  %t521 = phi ptr [ %t529, %backedge ], [ %t512, %entry ]
  %t522 = getelementptr inbounds ptr, ptr %t521, i64 %t520
  %t523 = load ptr, ptr %t522, align 8
  %t524 = icmp eq ptr %t523, null
  br i1 %t524, label %backedge, label %if

if:                                    ; preds = %loop
  tail call void @free(ptr %t523) #9
  %t526 = load i32, ptr @rule_count, align 4
  %t527 = load ptr, ptr @g_inputLine, align 8
  br label %backedge

backedge:                                    ; preds = %if, %loop
  %t529 = phi ptr [ %t521, %loop ], [ %t527, %if ]
  %t530 = phi i32 [ %t519, %loop ], [ %t526, %if ]
  %t531 = add nuw i64 %t520, 1
  %t532 = sext i32 %t530 to i64
  %t533 = icmp slt i64 %t531, %t532
  br i1 %t533, label %loop, label %exit, !llvm.loop !0

exit:                                    ; preds = %backedge
  %t535 = phi ptr [ %t529, %backedge ]
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.enable"}

