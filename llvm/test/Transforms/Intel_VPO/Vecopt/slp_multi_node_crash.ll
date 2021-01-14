; RUN: opt %s -slp-vectorizer -enable-intel-advanced-opts -mcpu=broadwell -disable-output
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CMPLRLLVM-21660
; The following Multi-node was built:
;         op0 op1       op0,op1 are MN leaves that not yet put into vectorizable tree
;        +--\-/-+
;        |  TE3 | (TE2)  TE2 is a TreeEntry but not a MN leaf because VL is different.
;        |     \ / --+
;        |     TE1   |   TE1 and TE3 are MN trunk elements with TE1 root.
;        +-----------+
;               |
;              TE0
; Note that tree entry TE2 is not a part of the MN while 1 and 3 are.
; Crash was caused due to assumtion that MN trunk elements are adjucent in vectorizable tree.
;
; Function Attrs: nofree norecurse nounwind uwtable
define dso_local <4 x i64> @test(i32 %i8, i64 %val, i32 %i, i32 %i5, i32 %i26, i32 %i49, i32 %i33, i32 %i34)  local_unnamed_addr {
entry:
  %mul283 = shl i32 %i8, 1
  %mul289 = mul i32 %i8, 3
  %mul295 = shl i32 %i8, 2

  %i2 = trunc i64 %val to i32
  %i35 = mul i32 %i34, 7

  %i38 = add i32 %mul283, %i2
  %i42 = add i32 %mul289, %i2
  %i46 = add i32 %mul295, %i2
  %i50 = add i32 %i49, %i35

  %i23 = add i32 %i5, %i33
  %i30 = srem i32 %i26, %i

  %i90 = add i32 %i23, %i38
  %i88 = add i32 %i23, %i42
  %i86 = add i32 %i23, %i46
  %i51 = add i32 %i50, %i30

  %i39 = sext i32 %i90 to i64
  %i43 = sext i32 %i88 to i64
  %i47 = sext i32 %i86 to i64
  %i52 = sext i32 %i51 to i64

  %ra = insertelement <4 x i64> undef, i64 %i39, i32 0
  %rb = insertelement <4 x i64> %ra, i64 %i43, i32 1
  %rc = insertelement <4 x i64> %rb, i64 %i47, i32 2
  %rd = insertelement <4 x i64> %rc, i64 %i52, i32 3
  ret <4 x i64> %rd
}

