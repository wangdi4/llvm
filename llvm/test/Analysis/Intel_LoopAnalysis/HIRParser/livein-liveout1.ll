; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details | FileCheck %s

; Check that the liveout values of region 1 (%0 and %conv318.maxLen.0) which are part of SCCs are not mapped to base values(%minLen.01049 and %maxLen.01050) in the livein set of region 2. The mapping is not valid for livein sets because they use underlying LLVM values directly (blobs cannot be used here). 

; CHECK: Region 1
; CHECK: SCC1
; CHECK: %conv318.maxLen.0 -> %maxLen.01050
; CHECK: SCC2
; CHECK: %0 -> %minLen.01049

; CHECK: LiveOuts
; CHECK-DAG: %0(sym:{{[0-9]+}})
; CHECK-DAG: %conv318.maxLen.0(sym:{{[0-9]+}})

; CHECK: Region 2
; CHECK: LiveIns
; CHECK-DAG: %minLen.01049(%0)
; CHECK-DAG: %n.025.i(%0)
; CHECK-DAG: %maxLen.01050(%conv318.maxLen.0)

; Mapped values are used in the upper bound calculation in Region 2.
; CHECK: DO i32 i1 = 0, -1 * %minLen.01049 + smax(%maxLen.01050, %minLen.01049)

; Check that borh lval and rval have identical canon exprs for this copy statement. Originally, there was a mismatch due to bug in assigning symbases.
; CHECK: %vec.026.i = 2 * %vec.1.lcssa.i;
; CHECK: <LVAL-REG> NON-LINEAR i32 2 * %vec.1.lcssa.i 
; CHECK: <RVAL-REG> NON-LINEAR i32 2 * %vec.1.lcssa.i 


;Module Before HIR; ModuleID = 'bzip2.c'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@len = common global [6 x [258 x i8]] zeroinitializer, align 1
@code = common global [6 x [258 x i32]] zeroinitializer, align 4

; Function Attrs: nounwind
define void @sendMTFValues(i32 %add1, i32 %t.81054) #2 {
entry:
  br label %for.body315

for.body315:                                      ; preds = %entry, %for.body315
  %i.51051 = phi i32 [ %inc337, %for.body315 ], [ 0, %entry ]
  %maxLen.01050 = phi i32 [ %conv318.maxLen.0, %for.body315 ], [ 0, %entry ]
  %minLen.01049 = phi i32 [ %0, %for.body315 ], [ 32, %entry ]
  %arrayidx317 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 %t.81054, i32 %i.51051
  %ld = load i8, i8* %arrayidx317, align 1
  %conv318 = zext i8 %ld to i32
  %cmp319 = icmp sgt i32 %conv318, %maxLen.01050
  %conv318.maxLen.0 = select i1 %cmp319, i32 %conv318, i32 %maxLen.01050
  %cmp329 = icmp slt i32 %conv318, %minLen.01049
  %0 = select i1 %cmp329, i32 %conv318, i32 %minLen.01049
  %inc337 = add nuw nsw i32 %i.51051, 1
  %cmp313 = icmp slt i32 %inc337, %add1
  br i1 %cmp313, label %for.body315, label %for.cond1.preheader.i.preheader

for.cond1.preheader.i.preheader:                  ; preds = %for.body315
  br label %for.cond1.preheader.i

for.cond1.preheader.i:                            ; preds = %for.cond1.preheader.i.preheader, %for.end.i
  %vec.026.i = phi i32 [ %shl.i, %for.end.i ], [ 0, %for.cond1.preheader.i.preheader ]
  %n.025.i = phi i32 [ %inc9.i, %for.end.i ], [ %0, %for.cond1.preheader.i.preheader ]
  br i1 true, label %for.body3.i.preheader, label %for.end.i

for.body3.i.preheader:                            ; preds = %for.cond1.preheader.i
  br label %for.body3.i

for.body3.i:                                      ; preds = %for.body3.i.preheader, %for.inc.i
  %i.023.i = phi i32 [ %inc7.i, %for.inc.i ], [ 0, %for.body3.i.preheader ]
  %vec.122.i = phi i32 [ %vec.2.i, %for.inc.i ], [ %vec.026.i, %for.body3.i.preheader ]
  %arrayidx.i = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i32 0, i32 %t.81054, i32 %i.023.i
  %1 = load i8, i8* %arrayidx.i, align 1
  %conv.i = zext i8 %1 to i32
  %cmp4.i = icmp eq i32 %conv.i, %n.025.i
  br i1 %cmp4.i, label %if.then.i, label %for.inc.i

if.then.i:                                        ; preds = %for.body3.i
  %arrayidx6.i = getelementptr inbounds [6 x [258 x i32]], [6 x [258 x i32]]* @code, i32 0, i32 %t.81054, i32 %i.023.i
  store i32 %vec.122.i, i32* %arrayidx6.i, align 4
  %inc.i = add nsw i32 %vec.122.i, 1
  br label %for.inc.i

for.inc.i:                                        ; preds = %if.then.i, %for.body3.i
  %vec.2.i = phi i32 [ %inc.i, %if.then.i ], [ %vec.122.i, %for.body3.i ]
  %inc7.i = add nuw nsw i32 %i.023.i, 1
  %exitcond.i = icmp eq i32 %inc7.i, %add1
  br i1 %exitcond.i, label %for.end.i.loopexit, label %for.body3.i

for.end.i.loopexit:                               ; preds = %for.inc.i
  br label %for.end.i

for.end.i:                                        ; preds = %for.end.i.loopexit, %for.cond1.preheader.i
  %vec.1.lcssa.i = phi i32 [ %vec.026.i, %for.cond1.preheader.i ], [ %vec.2.i, %for.end.i.loopexit ]
  %shl.i = shl i32 %vec.1.lcssa.i, 1
  %inc9.i = add nsw i32 %n.025.i, 1
  %cmp.i = icmp slt i32 %n.025.i, %conv318.maxLen.0
  br i1 %cmp.i, label %for.cond1.preheader.i, label %hbAssignCodes.exit.loopexit

hbAssignCodes.exit.loopexit:                      ; preds = %for.end.i
  ret void
}

