; In this test the codegen creates a constant i8 1 which is used for both a shift
; amount value and for a compare operand.
; This test exposed the following bug: the converter would widen the constant
; to an i32 for the shift amount usage to satisfy a PCG requirement, and then
; mapped the constant's SDNode to the widened PCG instruction. Later, the
; compare would see the widened i32 which is wrong.

; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

define void @____Vectorized_.radixSortBlocks_separated_args() nounwind alwaysinline {

entry:
 br label %postload3166

postload3166:                                     ; preds = %preload3165, %postload3102
  %loadedValue5711 = load <16 x i1>* undef, align 16
  %extract1042 = extractelement <16 x i1> %loadedValue5711, i32 1
  store i1 %extract1042, i1* undef, align 1
  br i1 false, label %SyncBB20320, label %postload3166



SyncBB20320:                                      ; preds = %SyncBB20319
  ret void
}
