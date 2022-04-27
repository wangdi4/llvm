; RUN: opt -enable-new-pm=0 -argpromotion -S < %s | FileCheck %s
; RUN: opt -passes=argpromotion -S < %s | FileCheck %s

; Check that 3 fields of a single structure argument are promoted,
; because MaxElements in ArgPromotion.cpp is 3.

; CHECK: define internal noundef i32 @_Z3fooR8MYSTRUCT(i32 %myarg.0.val, i32 %myarg.4.val, i32 %myarg.8.val)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MYSTRUCT = type { i32, i32, i32 }

@mylocalstruct = dso_local global %struct.MYSTRUCT { i32 0, i32 1, i32 2 }, align 4

; Function Attrs: mustprogress nounwind uwtable
define internal dso_local noundef i32 @_Z3fooR8MYSTRUCT(%struct.MYSTRUCT* noundef nonnull align 4 dereferenceable(12) %myarg) #0 {
entry:
  %myint0 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %myarg, i32 0, i32 0, !intel-tbaa !3
  %0 = load i32, i32* %myint0, align 4, !tbaa !3
  %myint1 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %myarg, i32 0, i32 1, !intel-tbaa !8
  %1 = load i32, i32* %myint1, align 4, !tbaa !8
  %add = add nsw i32 %0, %1
  %myint2 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %myarg, i32 0, i32 2, !intel-tbaa !9
  %2 = load i32, i32* %myint2, align 4, !tbaa !9
  %add1 = add nsw i32 %add, %2
  ret i32 %add1
}

; Function Attrs: mustprogress norecurse nounwind uwtable
define dso_local noundef i32 @main() #1 {
entry:
  %call = call noundef i32 @_Z3fooR8MYSTRUCT(%struct.MYSTRUCT* noundef nonnull align 4 dereferenceable(12) @mylocalstruct)
  ret i32 %call
}

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress norecurse nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"struct@_ZTS8MYSTRUCT", !5, i64 0, !5, i64 4, !5, i64 8}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!4, !5, i64 4}
!9 = !{!4, !5, i64 8}
