; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,hir-dead-store-elimination,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Verify that the loopnest is completely unrolled and that all stores to
; %ax are eliminated because it is regonized as local to region.

; Incoming HIR-
; + DO i1 = 0, 38, 1   <DO_LOOP>
; |   if (-1 * i1 + 40 == 19)
; |   {
; |      + DO i2 = 0, 5, 1   <DO_LOOP>
; |      |   (%ax)[0][3 * i2] = 38;
; |      + END LOOP
; |   }
; |   %2 = (%ax)[0][2];
; |   (%ax)[0][2] = %2 + -1;
; |   (%ax)[0][2] = (%ax)[0][-1 * i1 + 39];
; |   %4 = (%ax)[0][-1 * i1 + 39];
; |   %5 = (%ax)[0][-1 * i1 + 40];
; |   (%ax)[0][-1 * i1 + 40] = (%5 * %4);
; + END LOOP

; CHECK: %temp{{.*}} = (%ax)[0][39];
; CHECK-NOT: (%ax){{.*}} = 


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() {
entry:
  %ax = alloca [50 x i32], align 16
  %0 = bitcast ptr %ax to ptr
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(200) %0, i8 0, i64 200, i1 false)
  %arrayidx = getelementptr inbounds [50 x i32], ptr %ax, i64 0, i64 2
  br label %for.body

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv41 = phi i64 [ 40, %entry ], [ %indvars.iv.next42, %if.end ]
  %cmp1 = icmp eq i64 %indvars.iv41, 19
  br i1 %cmp1, label %for.body5.preheader, label %if.end

for.body5.preheader:                              ; preds = %for.body
  br label %for.body5

for.body5:                                        ; preds = %for.body5.preheader, %for.body5
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body5 ], [ 1, %for.body5.preheader ]
  %1 = add nsw i64 %indvars.iv, -1
  %arrayidx6 = getelementptr inbounds [50 x i32], ptr %ax, i64 0, i64 %1
  store i32 38, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp3 = icmp ult i64 %indvars.iv, 16
  br i1 %cmp3, label %for.body5, label %if.end.loopexit

if.end.loopexit:                                  ; preds = %for.body5
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit, %for.body
  %2 = load i32, ptr %arrayidx, align 8
  %sub7 = add i32 %2, -1
  store i32 %sub7, ptr %arrayidx, align 8
  %indvars.iv.next42 = add nsw i64 %indvars.iv41, -1
  %arrayidx10 = getelementptr inbounds [50 x i32], ptr %ax, i64 0, i64 %indvars.iv.next42
  %3 = load i32, ptr %arrayidx10, align 4
  store i32 %3, ptr %arrayidx, align 8
  %4 = load i32, ptr %arrayidx10, align 4
  %arrayidx15 = getelementptr inbounds [50 x i32], ptr %ax, i64 0, i64 %indvars.iv41
  %5 = load i32, ptr %arrayidx15, align 4
  %mul = mul i32 %4, %5
  store i32 %mul, ptr %arrayidx15, align 4
  %cmp16 = icmp eq i64 %indvars.iv41, 3
  %cmp = icmp ugt i64 %indvars.iv41, 2
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %if.end
  ret i32 0
}

declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg)

