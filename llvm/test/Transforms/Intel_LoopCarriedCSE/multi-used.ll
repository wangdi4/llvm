; Phi node %t34 has been used once while phi node %t36 has been used twice. The grouping can still happen, but we need to keep the information of phi node %36 here.
; Source code
;
;int A[1000];
;int B[1000];
;int C[1000];
;int D[1000];
;
;int m[1000];
;void sub(int n) {
;  int i;
;#pragma novector
;#pragma unroll(0)
;  for (i=0; i< n; i++) {
;    A[i] = B[i]+ C[i];
;    //we are reusing the addition of B[i+1] + C[i+1] in the next loop iteration
;    A[m[i]] = B[i+1]+ C[i+1];
;    D[i] = C[i] + C[i+1];
;   }
;}
;
; RUN: opt -passes="loop-carried-cse" -S 2>&1 < %s | FileCheck %s
;
; CHECK: %1 = add i32 %gepload, %gepload48
; CHECK: %t36.0 = phi i32
; CHECK: %t34.0.lccse = phi i32 [ %1, %for.body.preheader ], [ %4, %loop.28 ]
; CHECK-NOT: %t34.0 = phi i32
; CHECK: store i32 %t34.0.lccse, ptr %arrayIdx, align 4
;
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@m = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp35 = icmp sgt i32 %n, 0
  br i1 %cmp35, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  %gepload = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @B, i64 0, i64 0), align 4, !tbaa !2
  %gepload48 = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 0), align 4, !tbaa !2
  %0 = add i64 %wide.trip.count, -1
  br label %loop.28

for.end:                                          ; preds = %loop.28, %entry
  ret void

loop.28:                                          ; preds = %loop.28, %for.body.preheader
  %i1.i64.0 = phi i64 [ 0, %for.body.preheader ], [ %2, %loop.28 ]
  %t36.0 = phi i32 [ %gepload48, %for.body.preheader ], [ %gepload52, %loop.28 ]
  %t34.0 = phi i32 [ %gepload, %for.body.preheader ], [ %gepload50, %loop.28 ]
  %arrayIdx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %i1.i64.0
  %1 = add i32 %t34.0, %t36.0
  store i32 %1, ptr %arrayIdx, align 4, !tbaa !2
  %2 = add i64 %i1.i64.0, 1
  %arrayIdx49 = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %2
  %gepload50 = load i32, ptr %arrayIdx49, align 4, !tbaa !2
  %arrayIdx51 = getelementptr inbounds [1000 x i32], ptr @C, i64 0, i64 %2
  %gepload52 = load i32, ptr %arrayIdx51, align 4, !tbaa !2
  %arrayIdx53 = getelementptr inbounds [1000 x i32], ptr @m, i64 0, i64 %i1.i64.0
  %gepload54 = load i32, ptr %arrayIdx53, align 4, !tbaa !2
  %3 = sext i32 %gepload54 to i64
  %arrayIdx55 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %3
  %4 = add i32 %gepload50, %gepload52
  store i32 %4, ptr %arrayIdx55, align 4, !tbaa !2
  %arrayIdx56 = getelementptr inbounds [1000 x i32], ptr @D, i64 0, i64 %i1.i64.0
  %5 = add i32 %t36.0, %gepload52
  store i32 %5, ptr %arrayIdx56, align 4, !tbaa !2
  %condloop.28 = icmp sle i64 %2, %0
  br i1 %condloop.28, label %loop.28, label %for.end, !llvm.loop !7
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 7e90761a76dc7240ea8b0ed962fa119e5267a733) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 606c430d3c790143e60cd9a34cad562698283e7d)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8, !9}
!8 = !{!"llvm.loop.vectorize.width", i32 1}
!9 = !{!"llvm.loop.unroll.disable"}

