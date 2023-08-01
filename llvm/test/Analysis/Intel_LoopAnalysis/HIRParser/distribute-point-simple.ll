; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that we mark the load of (@b)[0][i1] as the distribute point (first HLInst guarded by DISTRIBUTE_POINT intrinsics).

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   %0 = (@a)[0][i1];
; CHECK: |   (@a)[0][i1] = %0 + 3;
; CHECK: |   %2 = (@b)[0][i1]; <distribute_point>
; CHECK: |   %s.012 = %2  +  %s.012;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(i32 %n) local_unnamed_addr {
entry:
  %cmp10 = icmp sgt i32 %n, 0
  br i1 %cmp10, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %s.012 = phi i32 [ 0, %for.body.preheader ], [ %add3, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @a, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, 3
  store i32 %add, ptr %arrayidx, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @b, i64 0, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx2, align 4
  %add3 = add nsw i32 %2, %s.012
  call void @llvm.directive.region.exit(token %1) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %add3.lcssa = phi i32 [ %add3, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %s.0.lcssa = phi i32 [ 0, %entry ], [ %add3.lcssa, %for.end.loopexit ]
  ret i32 %s.0.lcssa
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)


