
;RUN: opt -hir-ssa-deconstruction -hir-cg -force-hir-cg -S %s | FileCheck %s


;Inside the conditional for b == 47 we have a goto alter bblock
;          BEGIN REGION { }
;<33>         + DO i1 = 0, 63, 1   <DO_LOOP>
;<2>          |   %b.addr.015.out = %b.addr.015;
;<3>          |   %out.016 = %out.016  +  1;
;<6>          |   if (%b.addr.015.out == i1)
;<6>          |   {
;<10>         |      %b.addr.015 = %b.addr.015  +  -1;
;<6>          |   }
;<6>          |   else
;<6>          |   {
;<24>         |      %b.addr.015 = 47;
;<25>         |      if (%b.addr.015.out == 47)
;<25>         |      {
;<26>         |         goto alter;
;<25>         |      }
;<25>         |      else
;<25>         |      {
;<30>         |         %1 = (%a)[i1];
;<31>         |         %out.016 = %1;
;<25>         |      }
;<6>          |   }
;<13>         |   %out.016 = %out.016  +  1;
;<15>         |   alter:
;<16>         |   %out.016 = %out.016  +  -1;
;<33>         + END LOOP
;          END REGION

;CHECK: region.0:
;Look for <25>, comparision against 47
;CHECK: icmp eq i32 {{.*}} 47
;Goto is in true block
;CHECK-NEXT: br i1 %hir.cmp{{.*}}, label %[[T_BLOCK:then.[0-9]+]], label %else

;Block contains only a jump to hir version of alter bblock
;CHECK: [[T_BLOCK]]:
;CHECK-NEXT: br label %hir.alter

;Alter bblock will contain decrement of out
;CHECK: add{{.*}}, -1
; and IV update+loop br
;CHECK: br i1 %condloop
; ModuleID = 'loop_cont.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define i32 @foo(i32* nocapture readonly %a, i32 %b) #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %alter
  ret i32 %dec

for.body:                                         ; preds = %alter, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %alter ]
  %out.016 = phi i32 [ 0, %entry ], [ %dec, %alter ]
  %b.addr.015 = phi i32 [ %b, %entry ], [ %b.addr.2, %alter ]
  %add = add nsw i32 %out.016, 1
  %0 = trunc i64 %indvars.iv to i32
  %cmp1 = icmp eq i32 %b.addr.015, %0
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %sub = add nsw i32 %b.addr.015, -1
  br label %skip

if.else:                                          ; preds = %for.body
  %cmp2 = icmp eq i32 %b.addr.015, 47
  br i1 %cmp2, label %alter, label %if.end.4

if.end.4:                                         ; preds = %if.else
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  br label %skip

skip:                                             ; preds = %if.end.4, %if.then
  %b.addr.1 = phi i32 [ %sub, %if.then ], [ %b.addr.015, %if.end.4 ]
  %out.1 = phi i32 [ %add, %if.then ], [ %1, %if.end.4 ]
  %inc = add nsw i32 %out.1, 1
  br label %alter

alter:                                            ; preds = %if.else, %skip
  %b.addr.2 = phi i32 [ %b.addr.1, %skip ], [ 47, %if.else ]
  %out.2 = phi i32 [ %inc, %skip ], [ %add, %if.else ]
  %dec = add nsw i32 %out.2, -1
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
