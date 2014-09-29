; RUN: opt -prelegbools -verify -S %s -o - | FileCheck %s

; Test that the pass doesn't do anything illegal/unexpected
@glob.bools = constant <4 x i1> <i1 false, i1 true, i1 false, i1 true>
; CHECK-LABEL: @test_simple
define <4 x i64> @test_simple(<4 x double> %lhs, <4 x double> %rhs) {
; CHECK:     %[[CMP:[A-Za-z0-9.]+]] = sext <4 x i1> %cmp to <4 x i64>
  %cmp = fcmp oeq <4 x double> %lhs, %rhs
; CHECK:     %[[GBOOLS:[A-Za-z0-9.]+]] = sext <4 x i1> %gbools to <4 x i64>
  %gbools = load <4 x i1> * @glob.bools
; CHECK-NOT: and <4 x i1> %gbools, %cmp
; CHECK:     and <4 x i64> %[[GBOOLS]], %[[CMP]]
  %merge = and <4 x i1> %gbools, %cmp
; CHECK-NOT: sext <4 x i1> %merge to <4 x i64>
  %res = sext <4 x i1> %merge to <4 x i64>
  ret <4 x i64> %res
}

; Test that an "if" is handled well and no PHI nodes of type <16 x 1> are left
; CHECK-LABEL: @test_if
define <16 x i16> @test_if(i1 %switch, <16 x i1> %bools, <16 x float> %lhs, <16 x float> %rhs) {
BB0:
; CHECK:     %[[BOOLS:[A-Za-z0-9.]+]] = sext <16 x i1> %bools to <16 x i16>
; CHECK:     %[[CMP0:[A-Za-z0-9.]+]] = sext <16 x i1> %cmp0 to <16 x i16>
  %cmp0 = fcmp oeq <16 x float> %lhs, %rhs
  br i1 %switch, label %BB1, label %BB2
BB1:
; CHECK:     %[[CMP1:[A-Za-z0-9.]+]] = sext <16 x i1> %cmp1 to <16 x i16>
  %cmp1 = fcmp olt <16 x float> %lhs, %rhs
  br label %BB2
BB2:
; CHECK-NOT: phi <16 x i1>
; CHECK:     phi <16 x i16> [ %[[CMP0]], %BB0 ], [ %[[CMP1]], %BB1 ]
; CHECK-NOT: phi <16 x i1>
  %cmp = phi <16 x i1> [ %cmp0, %BB0 ], [ %cmp1, %BB1 ]
; CHECK-NOT: and <16 x i1>
; CHECK:     and <16 x i16>
  %merge = and <16 x i1> %bools, %cmp
; CHECK-NOT: sext <16 x i1>
  %res = sext <16 x i1> %merge to <16 x i16>
  ret <16 x i16> %res
}

; Test that select instruction is handled and no PHI nodes of type <16 x 1> are left
; CHECK-LABEL: @test_select
define <16 x i16> @test_select(i1 %switch, <16 x i1> %bools, <16 x float> %arg0, <16 x float> %arg1,  <16 x float> %arg2) {
BB0:
; CHECK:     %[[CMP00:[A-Za-z0-9.]+]] = sext <16 x i1> %cmp0.0 to <16 x i16>
  %cmp0.0 = fcmp oeq <16 x float> %arg0, %arg1
; CHECK:     %[[CMP01:[A-Za-z0-9.]+]] = sext <16 x i1> %cmp0.1 to <16 x i16>
  %cmp0.1 = fcmp oeq <16 x float> %arg0, %arg2
  br i1 %switch, label %BB1, label %BB2
BB1:
; CHECK:     %[[CMP10:[A-Za-z0-9.]+]] = sext <16 x i1> %cmp1.0 to <16 x i16>
  %cmp1.0 = fcmp olt <16 x float> %arg0, %arg1
; CHECK:     %[[CMP11:[A-Za-z0-9.]+]] = sext <16 x i1> %cmp1.1 to <16 x i16>
  %cmp1.1 = fcmp olt <16 x float> %arg0, %arg2
  br label %BB2
BB2:
; CHECK-NOT: phi <16 x i1>
; CHECK:     %[[CMP0:[A-Za-z0-9.]+]] = phi <16 x i16> [ %[[CMP00]], %BB0 ], [ %[[CMP10]], %BB1 ]
; CHECK:     %[[CMP1:[A-Za-z0-9.]+]] = phi <16 x i16> [ %[[CMP01]], %BB0 ], [ %[[CMP11]], %BB1 ]
; CHECK-NOT: phi <16 x i1>
  %cmp.0 = phi <16 x i1> [ %cmp0.0, %BB0 ], [ %cmp1.0, %BB1 ]
  %cmp.1 = phi <16 x i1> [ %cmp0.1, %BB0 ], [ %cmp1.1, %BB1 ]
; CHECK-NOT: select <16 x i1> %bools, <16 x i1> %cmp.0, <16 x i1> %cmp.1
; CHECK:     select <16 x i1> %bools, <16 x i16> %[[CMP0]], <16 x i16> %[[CMP1]]
  %sel = select <16 x i1> %bools, <16 x i1> %cmp.0, <16 x i1> %cmp.1
; CHECK-NOT: sext <16 x i1>
  %res = sext <16 x i1> %sel to <16 x i16>
  ret <16 x i16> %res
}

; Test that no PHI nodes of type <8 x 1> are left in a vectorized "loop"
declare i32 @llvm.x86.avx.ptestz.256(<4 x i64>, <4 x i64>) nounwind readnone
; CHECK-LABEL: @test_vloop
define <8 x i32> @test_vloop(<8 x i32> %ind.var, <8 x i32> %trip.cnt, <8 x i32> %step) {
ENTRY:
  %mask.e = icmp ult <8 x i32> %ind.var, %trip.cnt
  %mask.e.s = sext <8 x i1> %mask.e to <8 x i32>
  %mask.e.cast = bitcast <8 x i32> %mask.e.s to <4 x i64>
  %testz.e = call i32 @llvm.x86.avx.ptestz.256(<4 x i64> %mask.e.cast, <4 x i64> %mask.e.cast) nounwind
  %test.e = trunc i32 %testz.e to i1
  br i1 %test.e, label %EXIT, label %BODY
BODY:
; CHECK-NOT: phi <8 x i1>
; CHECK:     %[[MSKPRV:[A-Za-z0-9.]+]] = phi <8 x i32> [ %{{.+}}, %ENTRY ], [ %[[MSKNXT:[A-Za-z0-9.]+]], %BODY ]
  %mask.prev = phi <8 x i1> [ %mask.e, %ENTRY ], [ %mask.next, %BODY ]
  %ind.var.b = phi <8 x i32> [ %ind.var, %ENTRY ], [ %ind.var.next, %BODY ]
  ; just an empty loop
  %ind.var.next = add <8 x i32> %ind.var.b, %step
; CHECK:     %[[MSKB:[A-Za-z0-9.]+]] = sext <8 x i1> %mask.b to <8 x i32>
  %mask.b = icmp ult <8 x i32> %ind.var.next, %trip.cnt
; CHECK:     %[[MSKNXT]] = and <8 x i32> %[[MSKB]], %[[MSKPRV]]
  %mask.next = and <8 x i1> %mask.b, %mask.prev
; CHECK-NOT: sext <8 x i1> %mask.next to <8 x i32>
  %mask.next.s = sext <8 x i1> %mask.next to <8 x i32>
; CHECK:      bitcast <8 x i32> %[[MSKNXT]] to <4 x i64>
  %mask.next.cast = bitcast <8 x i32> %mask.next.s to <4 x i64>
  %testz.b = call i32 @llvm.x86.avx.ptestz.256(<4 x i64> %mask.next.cast, <4 x i64> %mask.next.cast) nounwind
  %test.b = trunc i32 %testz.b to i1
  br i1 %test.b, label %EXIT, label %BODY
EXIT:
  %ind.var.e = phi <8 x i32> [ %ind.var, %ENTRY ], [ %ind.var.next, %BODY ]
  ret <8 x i32> %ind.var.e
}
