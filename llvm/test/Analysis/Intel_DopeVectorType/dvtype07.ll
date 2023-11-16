; RUN: llvm-link -S %s %S/Inputs/dvtype07.ll 2>&1 | FileCheck %s
; XFAIL: *

; Check that after merging two bitcode files an identical type gets merged.
; Note that a type metadata may appear multiple times in !ifx.types.dv,
; this is an artifact of the IRMover.

; CHECK: %"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$i8*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$i8*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$double*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$double*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$DECOMPMODULE$.btLISTS*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
; CHECK: %"DECOMPMODULE$.btLISTS" = type <{ %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$" }>

; CHECK: !ifx.types.dv = !{!0, !1, !2, !0, !3, !4, !5, !6, !7}

; CHECK: !0 = !{%"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0}
; CHECK: !1 = !{%"QNCA_a0$i8*$rank1$" zeroinitializer, ptr null}
; CHECK: !2 = !{%"QNCA_a0$float*$rank2$" zeroinitializer, float 0.000000e+00}
; CHECK: !3 = !{%"QNCA_a0$float*$rank1$" zeroinitializer, float 0.000000e+00}
; CHECK: !4 = !{%"QNCA_a0$i8*$rank2$" zeroinitializer, ptr null}
; CHECK: !5 = !{%"QNCA_a0$double*$rank2$" zeroinitializer, double 0.000000e+00}
; CHECK: !6 = !{%"QNCA_a0$double*$rank1$" zeroinitializer, double 0.000000e+00}
; CHECK: !7 = !{%"QNCA_a0$DECOMPMODULE$.btLISTS*$rank1$" zeroinitializer, %"DECOMPMODULE$.btLISTS" zeroinitializer}

%"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$i8*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

!ifx.types.dv = !{!2, !3, !4}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{%"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0}
!3 = !{%"QNCA_a0$i8*$rank1$" zeroinitializer, ptr null}
!4 = !{%"QNCA_a0$float*$rank2$" zeroinitializer, float 0.000000e+00}
