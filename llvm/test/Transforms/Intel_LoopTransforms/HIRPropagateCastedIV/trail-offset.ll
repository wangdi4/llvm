; RUN: opt -passes="hir-ssa-deconstruction,hir-propagate-casted-iv,print<hir>" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Propagate Casted IV ***
;
;<0>          BEGIN REGION { }
;<13>               + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<3>                |   %t2 = zext.i32.i64(i1 + %n);
;<6>                |   (%a)[%t2 + 1].0 = i1;
;<13>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Propagate Casted IV ***
;
; CHECK:     BEGIN REGION { }
; CHECK:              %ptr = &((%a)[1]);
; CHECK:           + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   (%ptr)[i1 + %n].0 = i1;
; CHECK:           + END LOOP
; CHECK:     END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type { i32, i32 }

define dso_local void @foo(ptr nocapture %a, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i32 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %t1 = add nsw i32 %indvars.iv, %n
  %t2 = zext i32 %t1 to i64
  %add1 = add nsw i64 %t2, 1
  %x = getelementptr inbounds %struct.A, ptr %a, i64 %add1, i32 0
  store i32 %indvars.iv, ptr %x, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, %n
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

