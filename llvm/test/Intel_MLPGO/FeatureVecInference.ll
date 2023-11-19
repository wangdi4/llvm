; RUN: opt -profile-ml-use -debug-only=mlpgo -passes=mlpgo %s -S 2>&1 | FileCheck %s --check-prefixes=PROB_DUMP,PROB_METADATA
; UNSUPPORTED: intel_use_sanitizers


; The IR is generated from
; int TripCount = 64;
;
; int main(int ARGC, char **ARGV) {
;
;   int Res = 0;
;   for (int I = 0; I < TripCount; ++I) {
;     if (ARGC * Res > 2)
;       Res += ARGC;
;     else
;       --Res;
;
;     for (int J = 0; J < I; ++J)
;       Res += J - TripCount;
;   }
;   return Res;
; }


; PROB_DUMP: Input vector is:
; PROB_DUMP: 40 53 53 9 55 55 12 32 77 12 1 0 1 10 1 3 1 1 1 0 0 0 29 12 14 2 0 0 0 0 2 0 2 0 1 0 0 1 3 6 3 0 1 0 0 0 1 0 8 0 1 1 0 0 3 6 1 0
; PROB_DUMP: Model result 0 : 0.887914
; PROB_DUMP: Branch Prediction Model result is: 0.887914
; PROB_DUMP: Input vector is:
; PROB_DUMP: 38 53 53 9 17 17 12 69 71 12 0 0 1 10 1 3 1 1 1 0 1 0 29 12 14 2 0 0 0 0 0 0 5 0 1 0 0 0 3 6 2 0 1 0 0 0 0 0 1 0 1 0 0 0 3 6 2 0
; PROB_DUMP: Model result 0 : 0.728185
; PROB_DUMP: Branch Prediction Model result is: 0.728185
; PROB_DUMP: Input vector is:
; PROB_DUMP: 40 53 53 9 55 55 12 55 55 12 1 0 2 3 0 0 1 1 1 0 0 0 29 12 14 2 0 0 0 0 2 0 1 1 1 0 0 1 3 6 4 0 1 0 0 0 1 0 1 0 1 1 0 0 3 6 1 0
; PROB_DUMP: Model result 0 : 0.644790
; PROB_DUMP: Branch Prediction Model result is: 0.644790

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@TripCount = dso_local global i32 64, align 4

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %ARGC, ptr noundef %ARGV) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc6, %entry
  %Res.0 = phi i32 [ 0, %entry ], [ %Res.2, %for.inc6 ]
  %I.0 = phi i32 [ 0, %entry ], [ %inc7, %for.inc6 ]
  %0 = load i32, ptr @TripCount, align 4
  %cmp = icmp slt i32 %I.0, %0
  br i1 %cmp, label %for.body, label %for.end8
; PROB_METADATA: br i1 %cmp, label %for.body, label %for.end8, !prof ![[PROF1:[0-9]+]]

for.body:                                         ; preds = %for.cond
  %mul = mul nsw i32 %ARGC, %Res.0
  %cmp1 = icmp sgt i32 %mul, 2
  br i1 %cmp1, label %if.then, label %if.else
; PROB_METADATA: br i1 %cmp1, label %if.then, label %if.else, !prof ![[PROF2:[0-9]+]]

if.then:                                          ; preds = %for.body
  %add = add nsw i32 %Res.0, %ARGC
  br label %if.end

if.else:                                          ; preds = %for.body
  %dec = add nsw i32 %Res.0, -1
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %Res.1 = phi i32 [ %add, %if.then ], [ %dec, %if.else ]
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %if.end
  %Res.2 = phi i32 [ %Res.1, %if.end ], [ %add5, %for.inc ]
  %J.0 = phi i32 [ 0, %if.end ], [ %inc, %for.inc ]
  %cmp3 = icmp slt i32 %J.0, %I.0
  br i1 %cmp3, label %for.body4, label %for.end

; PROB_METADATA: br i1 %cmp3, label %for.body4, label %for.end, !prof ![[PROF3:[0-9]+]]

for.body4:                                        ; preds = %for.cond2
  %1 = load i32, ptr @TripCount, align 4
  %sub = sub nsw i32 %J.0, %1
  %add5 = add nsw i32 %Res.2, %sub
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %inc = add nsw i32 %J.0, 1
  br label %for.cond2, !llvm.loop !4

for.end:                                          ; preds = %for.cond2
  br label %for.inc6

for.inc6:                                         ; preds = %for.end
  %inc7 = add nsw i32 %I.0, 1
  br label %for.cond, !llvm.loop !6

for.end8:                                         ; preds = %for.cond
  ret i32 %Res.0
}

attributes #0 = { mustprogress noinline norecurse nounwind uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}


; PROB_METADATA: ![[PROF1]] = !{!"branch_weights", i32 1906781440, i32 240702208}
; PROB_METADATA: ![[PROF2]] = !{!"branch_weights", i32 1563764480, i32 583719168}
; PROB_METADATA: ![[PROF3]] = !{!"branch_weights", i32 1384674944, i32 762808704}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
!6 = distinct !{!6, !5}
