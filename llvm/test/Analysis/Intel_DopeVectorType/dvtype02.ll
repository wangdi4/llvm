; REQUIRES: asserts
; RUN: opt -passes='require<dopevectortype>' -debug-only=dopevectortype -disable-output < %s 2>&1 | FileCheck %s

; Test for a conflict in element types.

%"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"DECOMPMODULE$.btLISTS" = type <{ %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$" }>
%"QNCA_a0$DECOMPMODULE$.btLISTS*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; CHECK: DTYPE: Mismatched element types for %"QNCA_a0$DECOMPMODULE$.btLISTS*$rank1$"

!ifx.types.dv = !{!2, !3, !4}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{%"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0}
!3 = !{%"QNCA_a0$DECOMPMODULE$.btLISTS*$rank1$" zeroinitializer, %"DECOMPMODULE$.btLISTS" zeroinitializer}
!4 = !{%"QNCA_a0$DECOMPMODULE$.btLISTS*$rank1$" zeroinitializer, double 0.000000e+00}

