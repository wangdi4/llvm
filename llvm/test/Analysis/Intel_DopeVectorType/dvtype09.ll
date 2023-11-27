; RUN: llvm-link -S %s %S/Inputs/dvtype09a.ll %S/Inputs/dvtype09b.ll 2>&1 | FileCheck %s

; Check that after merging three bitcode files an identical type gets merged.
; Note that a type metadata may appear multiple times in !ifx.types.dv, 
; this is an artifact of the IRMover. In this LIT test, we also test that
; dope vector type info collection works if an input file with no dope
; vector metadata is introduced. (This can happen with mixed C/Fortran
; compilation.

; CHECK: %"QNCA_a0$i8*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
; CHECK: %"QNCA_a0$i8*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

; CHECK: !ifx.types.dv = !{!0, !1, !2, !2, !3, !4}

; CHECK: !0 = !{%"QNCA_a0$i8*$rank1$" zeroinitializer, ptr null}
; CHECK: !1 = !{%"QNCA_a0$float*$rank2$" zeroinitializer, float 0.000000e+00}
; CHECK: !2 = !{%"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0}
; CHECK: !3 = !{%"QNCA_a0$float*$rank1$" zeroinitializer, float 0.000000e+00}
; CHECK: !4 = !{%"QNCA_a0$i8*$rank2$" zeroinitializer, ptr null}

%"QNCA_a0$i8*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

!ifx.types.dv = !{!2, !3, !4}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{%"QNCA_a0$i8*$rank1$" zeroinitializer, ptr null}
!3 = !{%"QNCA_a0$float*$rank2$" zeroinitializer, float 0.000000e+00}
!4 = !{%"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0}
