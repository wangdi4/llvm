
;RUN: opt -hir-ssa-deconstruction -hir-cg -force-hir-cg -S %s | FileCheck %s
;In this test, the goto is lexically backward, the label will be encountered
;before the goto

;          BEGIN REGION { }
;<29>         + DO i1 = 0, 63, 1   <DO_LOOP>
;<2>          |   %b.addr.018.out = %b.addr.018;
;<3>          |   %out.019 = %out.019  +  1;
;<6>          |   if (%b.addr.018.out == i1)
;<6>          |   {
;<9>          |      convo:
;<10>         |      %b.addr.018 = %b.addr.018  +  -1;
;<6>          |   }
;<6>          |   else
;<6>          |   {
;<20>         |      %b.addr.018 = 67;
;<21>         |      if (%b.addr.018.out == 47)
;<21>         |      {
;<22>         |         goto convo;
;<21>         |      }
;<21>         |      else
;<21>         |      {
;<26>         |         %1 = (%a)[i1];
;<27>         |         %out.019 = %1;
;<21>         |      }
;<6>          |   }
;<29>         + END LOOP
;          END REGION
;CHECK: region:
;if is comparison of shortened IV to b
;CHECK: %[[IV_LD:[0-9]+]] = load i64, i64* %i1.i64
;CHECK-NEXT: trunc i64 %[[IV_LD]]
;CHECK-NEXT: icmp eq 
;CHECK-NEXT: br i1 {{.*}}, label %then.[[IF_NUM:[0-9]+]], label %else.[[IF_NUM]]

;Then block contains only a jump to convo block. In hir this looks like
;a label, but it IS the bblock
;CHECK: then.[[IF_NUM]]:
;CHECK-NEXT: br label %hir.convo

;convo block contains a decrement of B, and a jump to end of if stmt
;CHECK: hir.convo:
;CHECK: add {{.*}}, -1
;CHECK: store
;CHECK br label %ifmerge.[[IF_NUM]]

;inside else block
;CHECK: else.[[IF_NUM]]:
;is a comparision of b to 47
;CHECK icmp eq i32 {{.*}} 47
;CHECK: br i1 {{.*}}, label %then.[[IF2_NUM:[0-9]+]], label %else.[[IF2_NUM]]

;true block contains a goto convo
;CHECK: then.[[IF2_NUM]]:
;CHECK-NEXT: br label %hir.convo

; ModuleID = 'loop_cont.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define i32 @foo(i32* nocapture readonly %a, i32 %b) #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %skip
  %add6 = add nsw i32 %out.1, %b.addr.2
  ret i32 %add6

for.body:                                         ; preds = %skip, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %skip ]
  %out.019 = phi i32 [ 0, %entry ], [ %out.1, %skip ]
  %b.addr.018 = phi i32 [ %b, %entry ], [ %b.addr.2, %skip ]
  %add = add nsw i32 %out.019, 1
  %0 = trunc i64 %indvars.iv to i32
  %cmp1 = icmp eq i32 %b.addr.018, %0
  br i1 %cmp1, label %convo, label %if.else

convo:                                            ; preds = %if.else, %for.body
  %b.addr.1 = phi i32 [ %b.addr.018, %for.body ], [ 67, %if.else ]
  %sub = add nsw i32 %b.addr.1, -1
  br label %skip

if.else:                                          ; preds = %for.body
  %cmp2 = icmp eq i32 %b.addr.018, 47
  br i1 %cmp2, label %convo, label %if.end.4

if.end.4:                                         ; preds = %if.else
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  br label %skip

skip:                                             ; preds = %if.end.4, %convo
  %b.addr.2 = phi i32 [ %sub, %convo ], [ %b.addr.018, %if.end.4 ]
  %out.1 = phi i32 [ %add, %convo ], [ %1, %if.end.4 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1312) (llvm/branches/loopopt 1384)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
