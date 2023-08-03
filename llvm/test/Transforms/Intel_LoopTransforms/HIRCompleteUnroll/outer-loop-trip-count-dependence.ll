; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-loopnest,hir-pre-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that the i3 loop which depends on both i1 and i2 loop is not unrolled.

; CHECK: DO i1 = 0, 44
; CHECK: DO i2 = 0, 10
; CHECK: DO i3 = 0, i1 + -1 * i2 + -35, 1

;Module Before HIR; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  %jf = alloca [100 x [100 x i32]], align 16
  %m = alloca [100 x i32], align 16
  call void @llvm.memset.p0.i64(ptr nonnull %jf, i8 0, i64 40000, i32 16, i1 false)
  call void @llvm.memset.p0.i64(ptr nonnull %m, i8 0, i64 400, i32 16, i1 false)
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc16, %entry
  %indvars.iv61 = phi i64 [ 46, %entry ], [ %indvars.iv.next62, %for.inc16 ]
  %indvars.iv53 = phi i32 [ -35, %entry ], [ %indvars.iv.next54, %for.inc16 ]
  %indvars.iv51 = phi i64 [ 47, %entry ], [ %indvars.iv.next52, %for.inc16 ]
  %yz.sroa.0.048 = phi i32 [ 0, %entry ], [ %sub8.lcssa, %for.inc16 ]
  %o.047 = phi i32 [ 0, %entry ], [ %add.lcssa, %for.inc16 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.end
  %indvars.iv59 = phi i64 [ 12, %for.cond1.preheader ], [ %indvars.iv.next60, %for.end ]
  %indvars.iv55 = phi i32 [ %indvars.iv53, %for.cond1.preheader ], [ %indvars.iv.next56, %for.end ]
  %yz.sroa.0.145 = phi i32 [ %yz.sroa.0.048, %for.cond1.preheader ], [ %sub8, %for.end ]
  %o.144 = phi i32 [ %o.047, %for.cond1.preheader ], [ %add, %for.end ]
  %0 = zext i32 %indvars.iv55 to i64
  %1 = add i64 %indvars.iv51, %0
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], ptr %jf, i64 0, i64 %indvars.iv59, i64 0
  %2 = load i32, ptr %arrayidx5, align 16
  %sub = sub i32 %2, %yz.sroa.0.145
  store i32 %sub, ptr %arrayidx5, align 16
  %3 = load i32, ptr %m, align 16
  %sub8 = sub i32 %yz.sroa.0.145, %3
  %cmp1041 = icmp ult i64 %indvars.iv61, %indvars.iv59
  br i1 %cmp1041, label %for.body11.preheader, label %for.end

for.body11.preheader:                             ; preds = %for.body3
  br label %for.body11

for.body11:                                       ; preds = %for.body11.preheader, %for.body11
  %indvars.iv49 = phi i64 [ %indvars.iv.next50, %for.body11 ], [ %indvars.iv61, %for.body11.preheader ]
  %arrayidx13 = getelementptr inbounds [100 x i32], ptr %m, i64 0, i64 %indvars.iv49
  store i32 6, ptr %arrayidx13, align 4
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %exitcond = icmp eq i64 %indvars.iv.next50, %indvars.iv59
  br i1 %exitcond, label %for.end.loopexit, label %for.body11

for.end.loopexit:                                 ; preds = %for.body11
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body3
  %jd.0.lcssa.in = phi i64 [ %indvars.iv61, %for.body3 ], [ %1, %for.end.loopexit ]
  %jd.0.lcssa = trunc i64 %jd.0.lcssa.in to i32
  %add = add i32 %o.144, %jd.0.lcssa
  %indvars.iv.next60 = add nsw i64 %indvars.iv59, -1
  %cmp2 = icmp ugt i64 %indvars.iv.next60, 1
  %indvars.iv.next56 = add nsw i32 %indvars.iv55, -1
  br i1 %cmp2, label %for.body3, label %for.inc16

for.inc16:                                        ; preds = %for.end
  %add.lcssa = phi i32 [ %add, %for.end ]
  %sub8.lcssa = phi i32 [ %sub8, %for.end ]
  %indvars.iv.next62 = add nsw i64 %indvars.iv61, -1
  %indvars.iv.next52 = add nsw i64 %indvars.iv51, -1
  %indvars.iv.next54 = add nsw i32 %indvars.iv53, 1
  %exitcond63 = icmp eq i32 %indvars.iv.next54, 10
  br i1 %exitcond63, label %for.end18, label %for.cond1.preheader

for.end18:                                        ; preds = %for.inc16
  %add.lcssa.lcssa = phi i32 [ %add.lcssa, %for.inc16 ]
  %call = tail call i32 (ptr, ...) @printf(ptr @.str, i32 %add.lcssa.lcssa)
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i32, i1) #1

; Function Attrs: nounwind
declare i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #2

