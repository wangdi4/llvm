; RUN: opt -opaque-pointers=0 -passes=inline -S < %s  | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that tbaa and alias scope metadata are generated correctly for one
; level of inlining in Fortran. In particular, "ifx" symbols in the
; metadata should be appended with the inline clone numbers.

; CHECK-LABEL: define void @foo_(
; CHECK: %"foo_$ACOX_fetch.1" = load i32, i32* %"foo_$ACOX", align 1, !tbaa !0
; CHECK: store i32 %add.1, i32* %"foo_$BCOX", align 1, !tbaa !4

; CHECK-LABEL: define void @bar_(
; CHECK: %"foo_$ACOX_fetch.1.i" = load i32, i32* %"bar_$XCOX", align 1, !tbaa !11, !alias.scope !6, !noalias !9
; CHECK: store i32 %add.1.i, i32* %"bar_$YCOX", align 1, !tbaa !15, !alias.scope !9, !noalias !6
; CHECK: %"foo_$ACOX_fetch.1.i1" = load i32, i32* %"bar_$YCOX", align 1, !tbaa !22, !alias.scope !17, !noalias !20
; CHECK: store i32 %add.1.i2, i32* %"bar_$XCOX", align 1, !tbaa !26, !alias.scope !20, !noalias !17

; CHECK-LABEL: !0 = !{!1, !1, i64 0}
; CHECK: !1 = !{!"ifx$unique_sym$1", !2, i64 0}
; CHECK: !2 = !{!"Generic Fortran Symbol", !3, i64 0}
; CHECK: !3 = !{!"ifx$root$1$foo_"}
; CHECK: !4 = !{!5, !5, i64 0}
; CHECK: !5 = !{!"ifx$unique_sym$2", !2, i64 0}
; CHECK: !6 = !{!7}
; CHECK: !7 = distinct !{!7, !8, !"foo_: %foo_$ACOX"}
; CHECK: !8 = distinct !{!8, !"foo_"}
; CHECK: !9 = !{!10}
; CHECK: !10 = distinct !{!10, !8, !"foo_: %foo_$BCOX"}
; CHECK: !11 = !{!12, !12, i64 0}
; CHECK: !12 = !{!"ifx$unique_sym$1$0", !13, i64 0}
; CHECK: !13 = !{!"Generic Fortran Symbol", !14, i64 0}
; CHECK: !14 = !{!"ifx$root$1$foo_$0"}
; CHECK: !15 = !{!16, !16, i64 0}
; CHECK: !16 = !{!"ifx$unique_sym$2$0", !13, i64 0}
; CHECK: !17 = !{!18}
; CHECK: !18 = distinct !{!18, !19, !"foo_: %foo_$ACOX"}
; CHECK: !19 = distinct !{!19, !"foo_"}
; CHECK: !20 = !{!21}
; CHECK: !21 = distinct !{!21, !19, !"foo_: %foo_$BCOX"}
; CHECK: !22 = !{!23, !23, i64 0}
; CHECK: !23 = !{!"ifx$unique_sym$1$1", !24, i64 0}
; CHECK: !24 = !{!"Generic Fortran Symbol", !25, i64 0}
; CHECK: !25 = !{!"ifx$root$1$foo_$1"}
; CHECK: !26 = !{!27, !27, i64 0}
; CHECK: !27 = !{!"ifx$unique_sym$2$1", !24, i64 0}

; Function Attrs: nounwind uwtable
define void @foo_(i32* noalias dereferenceable(4) %"foo_$ACOX", i32* noalias dereferenceable(4) %"foo_$BCOX") local_unnamed_addr #0 {
alloca_0:
  %"foo_$ACOX_fetch.1" = load i32, i32* %"foo_$ACOX", align 1, !tbaa !0
  %add.1 = add nsw i32 %"foo_$ACOX_fetch.1", 1
  store i32 %add.1, i32* %"foo_$BCOX", align 1, !tbaa !4
  ret void
}

; Function Attrs: nounwind uwtable
define void @bar_(i32* noalias dereferenceable(4) %"bar_$XCOX", i32* noalias dereferenceable(4) %"bar_$YCOX") local_unnamed_addr #0 {
alloca_1:
  call void @foo_(i32* nonnull %"bar_$XCOX", i32* nonnull %"bar_$YCOX"), !llfort.type_idx !6
  call void @foo_(i32* nonnull %"bar_$YCOX", i32* nonnull %"bar_$XCOX"), !llfort.type_idx !7
  ret void
}

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"ifx$root$1$foo_"}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$2", !2, i64 0}
!6 = !{i64 35}
!7 = !{i64 36}
