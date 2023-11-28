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
;    A[i] = B[i]+ C[i] + D[i];
;    // we are reusing the addition of B[i+1] + C[i+1] + D[i+1] in the next loop iteration
;    A[m[i]] = B[i+1]+ C[i+1] + D[i+1];
;  }
;}
;
; RUN: opt -passes="loop-carried-cse" -aa-pipeline="basic-aa" -S 2>&1 < %s | FileCheck %s
;
; CHECK: %1 = add i32 %gepload, %gepload53
; CHECK: %2 = add i32 %1, %gepload54
; CHECK: %t42.0.lccse = phi i32 [ %2, %for.body.preheader ], [ %6, %loop.31 ]
; CHECK-NOT: %t42.0 = phi i32
; CHECK-NOT: %t40.0 = phi i32
; CHECK-NOT: %t38.0 = phi i32
; CHECK: store i32 %t42.0.lccse, ptr %arrayIdx, align 4

@B = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@m = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp33 = icmp sgt i32 %n, 0
  br i1 %cmp33, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  %gepload = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @B, i64 0, i64 0), align 4
  %gepload53 = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 0), align 4
  %gepload54 = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @D, i64 0, i64 0), align 4
  %0 = add i64 %wide.trip.count, -1
  br label %loop.31

for.end:                                          ; preds = %loop.31, %entry
  ret void

loop.31:                                          ; preds = %loop.31, %for.body.preheader
  %i1.i64.0 = phi i64 [ 0, %for.body.preheader ], [ %3, %loop.31 ]
  %t42.0 = phi i32 [ %gepload54, %for.body.preheader ], [ %gepload60, %loop.31 ]
  %t40.0 = phi i32 [ %gepload53, %for.body.preheader ], [ %gepload58, %loop.31 ]
  %t38.0 = phi i32 [ %gepload, %for.body.preheader ], [ %gepload56, %loop.31 ]
  %arrayIdx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %i1.i64.0
  %1 = add i32 %t38.0, %t40.0
  %2 = add i32 %1, %t42.0
  store i32 %2, ptr %arrayIdx, align 4
  %3 = add i64 %i1.i64.0, 1
  %arrayIdx55 = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %3
  %gepload56 = load i32, ptr %arrayIdx55, align 4
  %arrayIdx57 = getelementptr inbounds [1000 x i32], ptr @C, i64 0, i64 %3
  %gepload58 = load i32, ptr %arrayIdx57, align 4
  %arrayIdx59 = getelementptr inbounds [1000 x i32], ptr @D, i64 0, i64 %3
  %gepload60 = load i32, ptr %arrayIdx59, align 4
  %arrayIdx61 = getelementptr inbounds [1000 x i32], ptr @m, i64 0, i64 %i1.i64.0
  %gepload62 = load i32, ptr %arrayIdx61, align 4
  %4 = sext i32 %gepload62 to i64
  %arrayIdx63 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %4
  %5 = add i32 %gepload56, %gepload58
  %6 = add i32 %5, %gepload60
  store i32 %6, ptr %arrayIdx63, align 4
  %condloop.31 = icmp sle i64 %3, %0
  br i1 %condloop.31, label %loop.31, label %for.end, !llvm.loop !7
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 615e08801bde3e37a9290e8ae89002a292dfd3e9) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 33416f3a8bb043fec09b92917b392ce257193038)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8, !9}
!8 = !{!"llvm.loop.vectorize.width", i32 1}
!9 = !{!"llvm.loop.unroll.disable"}
