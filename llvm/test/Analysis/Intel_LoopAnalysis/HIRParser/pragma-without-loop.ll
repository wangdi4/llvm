; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that we don't compfail when a loop corresponding to prefetch region
; entry intrinsic is not found.

; In this particular case, the loop corresponding to %1 was optimized away.

; CHECK: BEGIN REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 %b) {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 0), "QUAL.PRAGMA.VAR"(ptr null), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1) ]
  %conv = sext i32 %b to i64
  %cmp7 = icmp sgt i32 %b, 0
  br i1 %cmp7, label %for.end.preheader, label %for.end4

for.end.preheader:                                ; preds = %entry
  br label %for.end

for.end:                                          ; preds = %for.end.preheader, %for.end
  %c.08 = phi i64 [ %inc, %for.end ], [ 0, %for.end.preheader ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 0), "QUAL.PRAGMA.VAR"(ptr null), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  %inc = add nuw nsw i64 %c.08, 1
  %exitcond.not = icmp eq i64 %inc, %conv
  br i1 %exitcond.not, label %for.end4.loopexit, label %for.end, !llvm.loop !3

for.end4.loopexit:                                ; preds = %for.end
  br label %for.end4

for.end4:                                         ; preds = %for.end4.loopexit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
