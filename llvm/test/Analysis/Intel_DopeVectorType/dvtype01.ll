; REQUIRES: asserts
; RUN: opt -passes='require<dopevectortype>' -debug-only=dopevectortype -disable-output < %s 2>&1 | FileCheck %s

; Test the formation of the DopeVectorTypeMap from Fortran front end metadata.

%"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"DECOMPMODULE$.btLISTS" = type <{ %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$" }>
%"QNCA_a0$i8*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$i8*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$DECOMPMODULE$.btLISTS*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; CHECK: DVTYPE: %"QNCA_a0$i32*$rank1$" -> i32
; CHECK: DVTYPE: %"QNCA_a0$i8*$rank1$" -> ptr
; CHECK: DVTYPE: %"QNCA_a0$float*$rank2$" -> float
; CHECK: DVTYPE: %"QNCA_a0$float*$rank1$" -> float
; CHECK: DVTYPE: %"QNCA_a0$i8*$rank2$" -> ptr
; CHECK: DVTYPE: %"QNCA_a0$double*$rank2$" -> double
; CHECK: DVTYPE: %"QNCA_a0$double*$rank1$" -> double
; CHECK: DVTYPE: %"QNCA_a0$DECOMPMODULE$.btLISTS*$rank1$" -> %"DECOMPMODULE$.btLISTS"

!ifx.types.dv = !{!2, !3, !4, !5, !6, !7, !8, !9}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{%"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0}
!3 = !{%"QNCA_a0$i8*$rank1$" zeroinitializer, ptr null}
!4 = !{%"QNCA_a0$float*$rank2$" zeroinitializer, float 0.000000e+00}
!5 = !{%"QNCA_a0$float*$rank1$" zeroinitializer, float 0.000000e+00}
!6 = !{%"QNCA_a0$i8*$rank2$" zeroinitializer, ptr null}
!7 = !{%"QNCA_a0$double*$rank2$" zeroinitializer, double 0.000000e+00}
!8 = !{%"QNCA_a0$double*$rank1$" zeroinitializer, double 0.000000e+00}
!9 = !{%"QNCA_a0$DECOMPMODULE$.btLISTS*$rank1$" zeroinitializer, %"DECOMPMODULE$.btLISTS" zeroinitializer}

