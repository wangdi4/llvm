; LLVM IR generated from testcase below using clang -O1 -S -emit-llvm
; int a[1024][1024], b[1024][1024], c[1024][1024];
; 
; void foo()
; {
;   int i1, i2;
; 
;   for (i1 = 0; i1 < 1024; i1++)
;     for (i2 = 0; i2 < 1024; i2++)
;       a[i1][i2] = b[i1][i2] + c[i1][i2];
; }
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -hir-cg -S  < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg" -vplan-force-vf=4 -S < %s | FileCheck %s

; CHECK: add <4 x i32>
; CHECK-NEXT: store <4 x i32>
; 
; ModuleID = 'nest.c'
source_filename = "nest.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = common local_unnamed_addr global [1024 x [1024 x i32]] zeroinitializer, align 16
@c = common local_unnamed_addr global [1024 x [1024 x i32]] zeroinitializer, align 16
@a = common local_unnamed_addr global [1024 x [1024 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc14, %entry
  %indvars.iv29 = phi i64 [ 0, %entry ], [ %indvars.iv.next30, %for.inc14 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [1024 x [1024 x i32]], [1024 x [1024 x i32]]* @b, i64 0, i64 %indvars.iv29, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x i32]], [1024 x [1024 x i32]]* @c, i64 0, i64 %indvars.iv29, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx9, align 4, !tbaa !1
  %add = add nsw i32 %1, %0
  %arrayidx13 = getelementptr inbounds [1024 x [1024 x i32]], [1024 x [1024 x i32]]* @a, i64 0, i64 %indvars.iv29, i64 %indvars.iv
  store i32 %add, i32* %arrayidx13, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.inc14, label %for.body3

for.inc14:                                        ; preds = %for.body3
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  %exitcond31 = icmp eq i64 %indvars.iv.next30, 1024
  br i1 %exitcond31, label %for.end16, label %for.cond1.preheader

for.end16:                                        ; preds = %for.inc14
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15884) (llvm/branches/loopopt 16232)"}
!1 = !{!2, !4, i64 0}
!2 = !{!"array@_ZTSA1024_A1024_i", !3, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
