; RUN: opt -passes=inline -S < %s  | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that tbaa and alias scope metadata are generated correctly for two
; levels of inlining in Fortran. In particular, "ifx" symbols in the
; metadata should be appended with the inline clone numbers.

; CHECK-LABEL: define void @foo_(
; CHECK: %"foo_$ACOX_fetch.1" = load i32, ptr %"foo_$ACOX", align 1, !tbaa !0
; CHECK: store i32 %add.1, ptr %"foo_$BCOX", align 1, !tbaa !4

; CHECK-LABEL: define void @bar_(
; CHECK: %"foo_$ACOX_fetch.1.i" = load i32, ptr %"bar_$XCOX", align 1, !tbaa !11, !alias.scope !6, !noalias !9
; CHECK: store i32 %add.1.i, ptr %"bar_$YCOX", align 1, !tbaa !15, !alias.scope !9, !noalias !6
; CHECK: %"foo_$ACOX_fetch.1.i1" = load i32, ptr %"bar_$YCOX", align 1, !tbaa !22, !alias.scope !17, !noalias !20
; CHECK: store i32 %add.1.i2, ptr %"bar_$XCOX", align 1, !tbaa !26, !alias.scope !20, !noalias !17

; CHECK-LABEL: define void @baz_(
; CHECK: %"foo_$ACOX_fetch.1.i.i" = load i32, ptr %"baz_$ZCOX", align 1, !tbaa !38, !alias.scope !42, !noalias !43
; CHECK: store i32 %add.1.i.i, ptr %"baz_$WCOX", align 1, !tbaa !44, !alias.scope !43, !noalias !42
; CHECK: %"foo_$ACOX_fetch.1.i1.i" = load i32, ptr %"baz_$WCOX", align 1, !tbaa !51, !alias.scope !55, !noalias !56
; CHECK: store i32 %add.1.i2.i, ptr %"baz_$ZCOX", align 1, !tbaa !57, !alias.scope !56, !noalias !55

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
; CHECK: !28 = !{!29}
; CHECK: !29 = distinct !{!29, !30, !"bar_: %bar_$XCOX"}
; CHECK: !30 = distinct !{!30, !"bar_"}
; CHECK: !31 = !{!32}
; CHECK: !32 = distinct !{!32, !30, !"bar_: %bar_$YCOX"}
; CHECK: !33 = !{!34}
; CHECK: !34 = distinct !{!34, !35, !"foo_: %foo_$ACOX"}
; CHECK: !35 = distinct !{!35, !"foo_"}
; CHECK: !36 = !{!37}
; CHECK: !37 = distinct !{!37, !35, !"foo_: %foo_$BCOX"}
; CHECK: !38 = !{!39, !39, i64 0}
; CHECK: !39 = !{!"ifx$unique_sym$1$0$2", !40, i64 0}
; CHECK: !40 = !{!"Generic Fortran Symbol", !41, i64 0}
; CHECK: !41 = !{!"ifx$root$1$foo_$0$2"}
; CHECK: !42 = !{!34, !29}
; CHECK: !43 = !{!37, !32}
; CHECK: !44 = !{!45, !45, i64 0}
; CHECK: !45 = !{!"ifx$unique_sym$2$0$2", !40, i64 0}
; CHECK: !46 = !{!47}
; CHECK: !47 = distinct !{!47, !48, !"foo_: %foo_$ACOX"}
; CHECK: !48 = distinct !{!48, !"foo_"}
; CHECK: !49 = !{!50}
; CHECK: !50 = distinct !{!50, !48, !"foo_: %foo_$BCOX"}
; CHECK: !51 = !{!52, !52, i64 0}
; CHECK: !52 = !{!"ifx$unique_sym$1$1$2", !53, i64 0}
; CHECK: !53 = !{!"Generic Fortran Symbol", !54, i64 0}
; CHECK: !54 = !{!"ifx$root$1$foo_$1$2"}
; CHECK: !55 = !{!47, !32}
; CHECK: !56 = !{!50, !29}
; CHECK: !57 = !{!58, !58, i64 0}
; CHECK: !58 = !{!"ifx$unique_sym$2$1$2", !53, i64 0}

; Function Attrs: nounwind uwtable
define void @foo_(ptr noalias dereferenceable(4) %"foo_$ACOX", ptr noalias dereferenceable(4) %"foo_$BCOX") local_unnamed_addr #0 {
alloca_0:
  %"foo_$ACOX_fetch.1" = load i32, ptr %"foo_$ACOX", align 1, !tbaa !0
  %add.1 = add nsw i32 %"foo_$ACOX_fetch.1", 1
  store i32 %add.1, ptr %"foo_$BCOX", align 1, !tbaa !4
  ret void
}

; Function Attrs: nounwind uwtable
define void @bar_(ptr noalias dereferenceable(4) %"bar_$XCOX", ptr noalias dereferenceable(4) %"bar_$YCOX") local_unnamed_addr #0 {
alloca_1:
  call void @foo_(ptr nonnull %"bar_$XCOX", ptr nonnull %"bar_$YCOX"), !llfort.type_idx !6
  call void @foo_(ptr nonnull %"bar_$YCOX", ptr nonnull %"bar_$XCOX"), !llfort.type_idx !7
  ret void
}

; Function Attrs: nounwind uwtable
define void @baz_(ptr noalias dereferenceable(4) %"baz_$ZCOX", ptr noalias dereferenceable(4) %"baz_$WCOX") local_unnamed_addr #0 {
alloca_2:
  call void @bar_(ptr nonnull %"baz_$ZCOX", ptr %"baz_$WCOX"), !llfort.type_idx !8
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
!8 = !{i64 47}
