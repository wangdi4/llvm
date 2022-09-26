; LLVM IR generated using following with clang -O1 -S -emit-llvm
; int a1[100], d1[100];
; 
; void init(int n)
; {
;     int i;
; 
;     for(i = 0; i < 100; i++){
;         a1[i] = (i & 7);
;         d1[i] = (i & 7) + 1;
;     }
; }
; 
; Check that loop is successfully vectorized
; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -hir-cg -print-after=hir-vplan-vec 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -S -vplan-force-vf=4 2>&1 < %s | FileCheck %s

; CHECK: (<4 x i32>*)(@a1)[0][i1] =
; CHECK: (<4 x i32>*)(@d1)[0][i1] =

; ModuleID = 'rebuild_03.c'
source_filename = "rebuild_03.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a1 = common global [100 x i32] zeroinitializer, align 16
@d1 = common global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @init(i32 %n) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %and = and i32 %0, 7
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @a1, i64 0, i64 %indvars.iv
  store i32 %and, i32* %arrayidx, align 4, !tbaa !1
  %add = add nuw nsw i32 %and, 1
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @d1, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx3, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15842) (llvm/branches/loopopt 15858)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
