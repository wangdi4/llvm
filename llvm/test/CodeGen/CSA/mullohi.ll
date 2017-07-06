; RUN: llc < %s -march=csa | FileCheck %s
; Test for mullohi{s,u}64 generation. We don't check for smaller mullohis
; because I don't know how to persuade LLVM to generate them other than when
; legalizing multiplication as we do here.

define i64 @_mul128(i64 %a, i64 %b, i64* nocapture %hi) {
entry:
  %conv = sext i64 %a to i128
  %conv1 = sext i64 %b to i128
  %mul = mul nsw i128 %conv1, %conv
  %conv2 = trunc i128 %mul to i64
  %shr7 = lshr i128 %mul, 64
  %conv3 = trunc i128 %shr7 to i64
  store i64 %conv3, i64* %hi, align 8
  ret i64 %conv2
}
; Check for the right mullohi instruction. Also check that the low output
; goes to a result register, while the high output is stored somewhere
; to memory.
; CHECK-LABEL: _mul128
; CHECK: .result .reg .i64 [[LOREG:%r[0-9]+]]
; CHECK: mullohis64 [[LOCHAN:%ci64_[0-9]+]], [[HICHAN:%ci64_[0-9]+]],
; CHECK: st64 [[HIADDR:%ci64_[0-9]+]], [[HICHAN]]
; CHECK: mov64 [[LOREG]], [[LOCHAN]]

define i64 @_umul128(i64 %a, i64 %b, i64* nocapture %hi) {
entry:
  %conv = zext i64 %a to i128
  %conv1 = zext i64 %b to i128
  %mul = mul nuw i128 %conv1, %conv
  %conv2 = trunc i128 %mul to i64
  %shr = lshr i128 %mul, 64
  %conv3 = trunc i128 %shr to i64
  store i64 %conv3, i64* %hi, align 8
  ret i64 %conv2
}
; Check for the right mullohi instruction. Also check that the low output
; goes to a result register, while the high output is stored somewhere
; to memory.
; CHECK-LABEL: _umul128
; CHECK: .result .reg .i64 [[LOREG:%r[0-9]+]]
; CHECK: mullohiu64 [[LOCHAN:%ci64_[0-9]+]], [[HICHAN:%ci64_[0-9]+]],
; CHECK: st64 [[HIADDR:%ci64_[0-9]+]], [[HICHAN]]
; CHECK: mov64 [[LOREG]], [[LOCHAN]]

; Same as _mul128, but with the lo/hi operands swapped.
define i64 @_mul128_2(i64 %a, i64 %b, i64* nocapture %lo) {
entry:
  %conv = sext i64 %a to i128
  %conv1 = sext i64 %b to i128
  %mul = mul nsw i128 %conv1, %conv
  %conv2 = trunc i128 %mul to i64
  %shr7 = lshr i128 %mul, 64
  %conv3 = trunc i128 %shr7 to i64
  store i64 %conv2, i64* %lo, align 8
  ret i64 %conv3
}
; Check for the right mullohi instruction. Also check that the low output
; goes to a result register, while the high output is stored somewhere
; to memory.
; CHECK-LABEL: _mul128_2
; CHECK: .result .reg .i64 [[HIREG:%r[0-9]+]]
; CHECK: mullohis64 [[LOCHAN:%ci64_[0-9]+]], [[HICHAN:%ci64_[0-9]+]],
; CHECK: st64 [[LOADDR:%ci64_[0-9]+]], [[LOCHAN]]
; CHECK: mov64 [[HIREG]], [[HICHAN]]

; Same as _umul128, but with the lo/hi operands swapped.
define i64 @_umul128_2(i64 %a, i64 %b, i64* nocapture %lo) {
entry:
  %conv = zext i64 %a to i128
  %conv1 = zext i64 %b to i128
  %mul = mul nuw i128 %conv1, %conv
  %conv2 = trunc i128 %mul to i64
  %shr = lshr i128 %mul, 64
  %conv3 = trunc i128 %shr to i64
  store i64 %conv2, i64* %lo, align 8
  ret i64 %conv3
}
; Check for the right mullohi instruction. Also check that the low output
; goes to a result register, while the high output is stored somewhere
; to memory.
; CHECK-LABEL: _umul128_2
; CHECK: .result .reg .i64 [[HIREG:%r[0-9]+]]
; CHECK: mullohiu64 [[LOCHAN:%ci64_[0-9]+]], [[HICHAN:%ci64_[0-9]+]],
; CHECK: st64 [[LOADDR:%ci64_[0-9]+]], [[LOCHAN]]
; CHECK: mov64 [[HIREG]], [[HICHAN]]
