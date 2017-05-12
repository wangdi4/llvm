;RUN: opt -hir-cg -force-hir-cg -S %s | FileCheck %s


;          BEGIN REGION { }
;<11>         + DO i1 = 0, zext.i32.i64((-1 + %N)), 1   <DO_LOOP>
;<2>          |   %0 = alloca %size;
;<4>          |   (%allocs)[i1] = &((%0)[0]);
;<11>         + END LOOP
;          END REGION
;CHECK: region.0:
;CHECK: loop.{{[0-9]+}}:
;CHECK-NEXT: [[SEXT_SIZE:%[0-9]+]] = sext i32 %size to i64
;CHECK-NEXT: = alloca i8, i64 [[SEXT_SIZE]]

; ModuleID = 'alloca.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32 %N, i32 %size, i8** nocapture %allocs) #0 {
entry:
  %cmp.5 = icmp sgt i32 %N, 0
  br i1 %cmp.5, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %conv = sext i32 %size to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %0 = alloca i8, i64 %conv, align 1
  %arrayidx = getelementptr inbounds i8*, i8** %allocs, i64 %indvars.iv
  store i8* %0, i8** %arrayidx, align 8, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1607) (llvm/branches/loopopt 1631)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
