; Source code
;
;int A[1000];
;int B[1000];
;int C[1000];
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
;  }
;}
;
; RUN: opt -passes="loop-carried-cse" -aa-pipeline="basic-aa" -S 2>&1 < %s | FileCheck %s
;
; CHECK: %1 = add i32 %gepload, %gepload37
; CHECK: %t32.0.lccse = phi i32 [ %1, %for.body.preheader ], [ %4, %loop.25 ]
; CHECK-NOT: %t32.0 = phi i32
; CHECK-NOT: %t30.0 = phi i32
; CHECK: store i32 %t32.0.lccse, ptr %arrayIdx, align 4

@B = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@m = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp24 = icmp sgt i32 %n, 0
  br i1 %cmp24, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  %gepload = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @B, i64 0, i64 0), align 4
  %gepload37 = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 0), align 4
  %0 = add i64 %wide.trip.count, -1
  br label %loop.25

for.end:                                          ; preds = %loop.25, %entry
  ret void

loop.25:                                          ; preds = %loop.25, %for.body.preheader
  %i1.i64.0 = phi i64 [ 0, %for.body.preheader ], [ %2, %loop.25 ]
  %t32.0 = phi i32 [ %gepload37, %for.body.preheader ], [ %gepload41, %loop.25 ]
  %t30.0 = phi i32 [ %gepload, %for.body.preheader ], [ %gepload39, %loop.25 ]
  %arrayIdx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %i1.i64.0
  %1 = add i32 %t30.0, %t32.0
  store i32 %1, ptr %arrayIdx, align 4
  %2 = add i64 %i1.i64.0, 1
  %arrayIdx38 = getelementptr inbounds [1000 x i32], ptr @B, i64 0, i64 %2
  %gepload39 = load i32, ptr %arrayIdx38, align 4
  %arrayIdx40 = getelementptr inbounds [1000 x i32], ptr @C, i64 0, i64 %2
  %gepload41 = load i32, ptr %arrayIdx40, align 4
  %arrayIdx42 = getelementptr inbounds [1000 x i32], ptr @m, i64 0, i64 %i1.i64.0
  %gepload43 = load i32, ptr %arrayIdx42, align 4
  %3 = sext i32 %gepload43 to i64
  %arrayIdx44 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %3
  %4 = add i32 %gepload39, %gepload41
  store i32 %4, ptr %arrayIdx44, align 4
  %condloop.25 = icmp sle i64 %2, %0
  br i1 %condloop.25, label %loop.25, label %for.end, !llvm.loop !7
}
attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

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

