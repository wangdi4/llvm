; Check HIR vectorizer codegen inserts reduction init and final for reduction
; temp %sum.039 in loop's preheader and postexit respectively.

; <0>          BEGIN REGION { }
; <50>               + DO i1 = 0, 9, 1   <DO_LOOP>
; <4>                |   %div = %X.addr.036  /  3;
; <51>               |
; <51>               |   + DO i2 = 0, zext.i32.i64(%div) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 9>
; <53>               |   |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; <52>               |   |
; <52>               |   |   + DO i3 = 0, 19, 1   <DO_LOOP>
; <22>               |   |   |   %sum.039 = (@A)[0][i1][10 * i2 + i3]  +  %sum.039; <Safe Reduction>
; <52>               |   |   + END LOOP
; <52>               |   |
; <54>               |   |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <51>               |   + END LOOP
; <51>               |
; <42>               |   %div13 = %X.addr.036  /  2;
; <43>               |   %X.addr.036 = %div13  +  3;
; <50>               + END LOOP
; <0>          END REGION


; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:           + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK-NEXT:           |   %div = %X.addr.036  /  3;
; CHECK-NEXT:           |
; CHECK-NEXT:           |      %red.init = 0;
; CHECK-NEXT:           |      %red.init.insert = insertelement %red.init,  %sum.039,  0;
; CHECK-NEXT:           |      %phi.temp = %red.init.insert;
; CHECK:                |   + DO i2 = 0, zext.i32.i64(%div) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 9>
; CHECK:                |   |   + DO i3 = 0, 19, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:           |   |   |   %.vec = (<4 x i32>*)(@A)[0][i1][10 * i2 + i3];
; CHECK-NEXT:           |   |   |   %.vec3 = %.vec  +  %phi.temp;
; CHECK-NEXT:           |   |   |   %phi.temp = %.vec3;
; CHECK-NEXT:           |   |   + END LOOP
; CHECK:                |   + END LOOP
; CHECK:                |      %sum.039 = @llvm.vector.reduce.add.v4i32(%.vec3);
; CHECK-NEXT:           |
; CHECK-NEXT:           |   %div13 = %X.addr.036  /  2;
; CHECK-NEXT:           |   %X.addr.036 = %div13  +  3;
; CHECK-NEXT:           + END LOOP
; CHECK-NEXT:     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [10 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(i32 %X) local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end12, %entry
  %indvars.iv45 = phi i64 [ 0, %entry ], [ %indvars.iv.next46, %for.end12 ]
  %sum.039 = phi i32 [ 0, %entry ], [ %sum.1.lcssa, %for.end12 ]
  %X.addr.036 = phi i32 [ %X, %entry ], [ %add14, %for.end12 ]
  %div = sdiv i32 %X.addr.036, 3
  %cmp233 = icmp sgt i32 %div, 0
  br i1 %cmp233, label %for.cond4.preheader.lr.ph, label %for.end12

for.cond4.preheader.lr.ph:                        ; preds = %for.cond1.preheader
  %wide.trip.count48 = zext i32 %div to i64
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc10, %for.cond4.preheader.lr.ph
  %indvars.iv41 = phi i64 [ 0, %for.cond4.preheader.lr.ph ], [ %indvars.iv.next42, %for.inc10 ]
  %sum.135 = phi i32 [ %sum.039, %for.cond4.preheader.lr.ph ], [ %add9.lcssa, %for.inc10 ]
  %0 = mul nuw nsw i64 %indvars.iv41, 10
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %sum.232 = phi i32 [ %sum.135, %for.cond4.preheader ], [ %add9, %for.body6 ]
  %1 = add nuw nsw i64 %indvars.iv, %0
  %arrayidx8 = getelementptr inbounds [10 x [100 x i32]], [10 x [100 x i32]]* @A, i64 0, i64 %indvars.iv45, i64 %1
  %2 = load i32, i32* %arrayidx8, align 4
  %add9 = add nsw i32 %2, %sum.232
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond, label %for.inc10, label %for.body6

for.inc10:                                        ; preds = %for.body6
  %add9.lcssa = phi i32 [ %add9, %for.body6 ]
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond44 = icmp eq i64 %indvars.iv.next42, %wide.trip.count48
  br i1 %exitcond44, label %for.end12.loopexit, label %for.cond4.preheader

for.end12.loopexit:                               ; preds = %for.inc10
  %add9.lcssa.lcssa = phi i32 [ %add9.lcssa, %for.inc10 ]
  br label %for.end12

for.end12:                                        ; preds = %for.end12.loopexit, %for.cond1.preheader
  %sum.1.lcssa = phi i32 [ %sum.039, %for.cond1.preheader ], [ %add9.lcssa.lcssa, %for.end12.loopexit ]
  %div13 = sdiv i32 %X.addr.036, 2
  %add14 = add nsw i32 %div13, 3
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond47 = icmp eq i64 %indvars.iv.next46, 10
  br i1 %exitcond47, label %for.end17, label %for.cond1.preheader

for.end17:                                        ; preds = %for.end12
  %sum.1.lcssa.lcssa = phi i32 [ %sum.1.lcssa, %for.end12 ]
  %3 = load i32, i32* getelementptr inbounds ([10 x [100 x i32]], [10 x [100 x i32]]* @A, i64 0, i64 0, i64 0), align 16
  %add18 = add nsw i32 %3, %sum.1.lcssa.lcssa
  ret i32 %add18
}
