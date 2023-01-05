; REQUIRES: asserts
; RUN: opt -disable-output -passes=dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s 2>&1 | FileCheck %s

; Check that compile does not seg fault when dope vector constant propagation is attempted on a function
; whose dope vector pointer is an unused argument.

%"QNCA_a0$i32*$rank1$" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; CHECK-LABEL: DOPE VECTOR CONSTANT PROPAGATION: BEGIN
; CHECK-NEXT:  DV FOUND: ARG #0 dyn_grid_mp_get_block_ldof_d_ 1 x i32
; CHECK-NEXT:  VALID
; CHECK-NEXT:  NO CONSTANT DOPE VECTOR FIELDS
; CHECK-NEXT:  DOPE VECTOR CONSTANT PROPAGATION: END

define internal void @dyn_grid_mp_get_block_ldof_d_(%"QNCA_a0$i32*$rank1$"* noalias nocapture dereferenceable(72) "assumed_shape" "ptrnoalias" %0) #0 {
  ret void
}

attributes #0 = {"intel-lang"="fortran"}


