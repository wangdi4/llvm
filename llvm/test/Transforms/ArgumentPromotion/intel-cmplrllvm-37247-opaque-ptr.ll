; RUN: opt -enable-new-pm=0 -argpromotion -S < %s | FileCheck %s
; RUN: opt -passes=argpromotion -S < %s | FileCheck %s

; Check that argument promotion was not performed on mutually recursive
; functions.

; CHECK: define dso_local noundef i32 @_Z3fooR9_MYSTRUCT(ptr noundef nonnull align 8 dereferenceable(16) %myarg)
; CHECK: define dso_local noundef i32 @_Z3gooR9_MYSTRUCT(ptr noundef nonnull align 8 dereferenceable(16) %myarg)
; CHECK: define dso_local noundef i32 @main()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._MYSTRUCT = type { ptr, i32 }

@mylocalstruct = dso_local global %struct._MYSTRUCT { ptr null, i32 2 }, align 8

; Function Attrs: mustprogress uwtable
define dso_local noundef i32 @_Z3fooR9_MYSTRUCT(ptr noundef nonnull align 8 dereferenceable(16) %myarg) #0 {
entry:
  %myint = getelementptr inbounds %struct._MYSTRUCT, ptr %myarg, i32 0, i32 1, !intel-tbaa !3
  %i = load i32, ptr %myint, align 8, !tbaa !3
  %myptr = getelementptr inbounds %struct._MYSTRUCT, ptr %myarg, i32 0, i32 0, !intel-tbaa !9
  %i1 = load ptr, ptr %myptr, align 8, !tbaa !9
  %call = call noundef i32 @_Z3gooR9_MYSTRUCT(ptr noundef nonnull align 8 dereferenceable(16) %i1)
  %add = add nsw i32 %i, %call
  ret i32 %add
}

; Function Attrs: mustprogress uwtable
define dso_local noundef i32 @_Z3gooR9_MYSTRUCT(ptr noundef nonnull align 8 dereferenceable(16) %myarg) #0 {
entry:
  %myint = getelementptr inbounds %struct._MYSTRUCT, ptr %myarg, i32 0, i32 1, !intel-tbaa !3
  %i = load i32, ptr %myint, align 8, !tbaa !3
  %myptr = getelementptr inbounds %struct._MYSTRUCT, ptr %myarg, i32 0, i32 0, !intel-tbaa !9
  %i1 = load ptr, ptr %myptr, align 8, !tbaa !9
  %call = call noundef i32 @_Z3fooR9_MYSTRUCT(ptr noundef nonnull align 8 dereferenceable(16) %i1)
  %add = add nsw i32 %i, %call
  ret i32 %add
}

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main() #1 {
entry:
  %call = call noundef i32 @_Z3fooR9_MYSTRUCT(ptr noundef nonnull align 8 dereferenceable(16) @mylocalstruct)
  %call1 = call noundef i32 @_Z3gooR9_MYSTRUCT(ptr noundef nonnull align 8 dereferenceable(16) @mylocalstruct)
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

attributes #0 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !8, i64 8}
!4 = !{!"struct@_ZTS9_MYSTRUCT", !5, i64 0, !8, i64 8}
!5 = !{!"pointer@_ZTSP9_MYSTRUCT", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!"int", !6, i64 0}
!9 = !{!4, !5, i64 0}
