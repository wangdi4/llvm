; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; Test checks that DD is correctly creating edges between partial ((%data.i.i)[0].0[*])
; and full ((i40*)(%data.i.i)[0]) structure accesses.

;<0>          BEGIN REGION { modified }
;<28>               + DO i1 = 0, 63, 1   <DO_LOOP>
;<2>                |   @llvm.lifetime.start.p0(5,  &((%data.i.i)[0]));
;<31>               |   (%data.i.i)[0].0[0] = i1;
;<32>               |   (%data.i.i)[0].0[1] = i1;
;<33>               |   (%data.i.i)[0].0[2] = i1;
;<34>               |   (%data.i.i)[0].0[3] = i1;
;<10>               |   (%data.i.i)[0].0[4] = i1;
;<18>               |   %retval.sroa.0.0.copyload.i.i = (i40*)(%data.i.i)[0];
;<19>               |   @llvm.lifetime.end.p0(5,  &((%data.i.i)[0]));
;<21>               |   (i40*)(%data.i)[0][i1] = %retval.sroa.0.0.copyload.i.i;
;<28>               + END LOOP
;<0>          END REGION


; CHECK: DD graph for function main:
; CHECK-DAG: (%data.i.i)[0].0[0] --> (i40*)(%data.i.i)[0] FLOW (*) (?)
; CHECK-DAG: (%data.i.i)[0].0[1] --> (i40*)(%data.i.i)[0] FLOW (*) (?)
; CHECK-DAG: (%data.i.i)[0].0[2] --> (i40*)(%data.i.i)[0] FLOW (*) (?)
; CHECK-DAG: (%data.i.i)[0].0[3] --> (i40*)(%data.i.i)[0] FLOW (*) (?)
; CHECK-DAG: (%data.i.i)[0].0[4] --> (i40*)(%data.i.i)[0] FLOW (*) (?)
; CHECK-DAG: (%data.i.i)[0] --> (i40*)(%data.i.i)[0] FLOW (*) (?)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%marray = type { [5 x i8] }

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local noundef i32 @main() local_unnamed_addr {
entry:
  %data.i.i = alloca %marray, align 1
  %data.i = alloca [64 x %marray], align 16
  call void @llvm.lifetime.start.p0(i64 320, ptr nonnull %data.i)
  %array.begin.i = getelementptr inbounds [64 x %marray], ptr %data.i, i64 0, i64 0
  %arrayctor.end.i = getelementptr inbounds [64 x %marray], ptr %data.i, i64 0, i64 64
  br label %for.cond.preheader.i

for.cond.preheader.i:                             ; preds = %entry
  %arrayinit.begin.i.i.i = getelementptr inbounds %marray, ptr %data.i.i, i64 0, i32 0, i64 0
  %arrayinit.end.i.i.i = getelementptr inbounds %marray, ptr %data.i.i, i64 0, i32 0, i64 5
  br label %for.body.i

for.body.i:                                       ; preds = %bb.exit.i, %for.cond.preheader.i
  %i.04.i = phi i64 [ 0, %for.cond.preheader.i ], [ %inc.i, %bb.exit.i ]
  call void @llvm.lifetime.start.p0(i64 5, ptr nonnull %data.i.i)
  br label %for.cond.preheader.i.i

for.cond.preheader.i.i:                           ; preds = %arrayinit.body.i.i.i
  %conv.i.i = trunc i64 %i.04.i to i8
  br label %for.body.i.i

for.body.i.i:                                     ; preds = %for.body.i.i, %for.cond.preheader.i.i
  %i.04.i.i = phi i64 [ 0, %for.cond.preheader.i.i ], [ %inc.i.i, %for.body.i.i ]
  %arrayidx.i.i.i = getelementptr inbounds %marray, ptr %data.i.i, i64 0, i32 0, i64 %i.04.i.i
  store i8 %conv.i.i, ptr %arrayidx.i.i.i, align 1
  %inc.i.i = add nuw nsw i64 %i.04.i.i, 1
  %exitcond.not.i.i = icmp eq i64 %inc.i.i, 5
  br i1 %exitcond.not.i.i, label %bb.exit.i, label %for.body.i.i

bb.exit.i:     ; preds = %for.body.i.i
  %retval.sroa.0.0.copyload.i.i = load i40, ptr %data.i.i, align 1
  call void @llvm.lifetime.end.p0(i64 5, ptr nonnull %data.i.i)
  %arrayidx.i = getelementptr inbounds [64 x %marray], ptr %data.i, i64 0, i64 %i.04.i
  store i40 %retval.sroa.0.0.copyload.i.i, ptr %arrayidx.i, align 1
  %inc.i = add nuw nsw i64 %i.04.i, 1
  %exitcond.not.i = icmp eq i64 %inc.i, 64
  br i1 %exitcond.not.i, label %bb2.exit, label %for.body.i

bb2.exit:   ; preds = %bb.exit.i
  call void @llvm.lifetime.end.p0(i64 320, ptr nonnull %data.i)
  ret i32 0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

