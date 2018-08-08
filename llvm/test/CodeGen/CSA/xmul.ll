; RUN: llc -O2 < %s | FileCheck %s
source_filename = "div.cpp"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: norecurse nounwind
define zeroext i8 @divu8(i8 zeroext %val) local_unnamed_addr #0 align 2 {
; CHECK: divu8
; CHECK: mul
entry:
  %div2 = udiv i8 %val, 10
  ret i8 %div2
}

; Function Attrs: nounwind
define zeroext i1 @mulou8(i8 zeroext %lhs, i8 zeroext %rhs, i8* dereferenceable(1) %res) local_unnamed_addr #1 align 2 {
; CHECK: mulou8
; CHECK: xmulu8 %[[DEST:[a-z0-9_]+]]
entry:
  %0 = tail call { i8, i1 } @llvm.umul.with.overflow.i8(i8 %lhs, i8 %rhs)
  %1 = extractvalue { i8, i1 } %0, 1
  %2 = extractvalue { i8, i1 } %0, 0
  store i8 %2, i8* %res, align 1
  ret i1 %1
}

; Function Attrs: nounwind readnone speculatable
declare { i8, i1 } @llvm.umul.with.overflow.i8(i8, i8) #2

; Function Attrs: norecurse nounwind
define signext i8 @divs8(i8 signext %val) local_unnamed_addr #0 align 2 {
; CHECK: divs8
; CHECK: mul
entry:
  %0 = sdiv i8 %val, 10
  ret i8 %0
}

; Function Attrs: nounwind
define zeroext i1 @mulos8(i8 signext %lhs, i8 signext %rhs, i8* dereferenceable(1) %res) local_unnamed_addr #1 align 2 {
; CHECK: mulos8
; CHECK: xmuls8 %[[DEST:[a-z0-9_]+]]
entry:
  %0 = tail call { i8, i1 } @llvm.smul.with.overflow.i8(i8 %lhs, i8 %rhs)
  %1 = extractvalue { i8, i1 } %0, 1
  %2 = extractvalue { i8, i1 } %0, 0
  store i8 %2, i8* %res, align 1
  ret i1 %1
}

; Function Attrs: nounwind readnone speculatable
declare { i8, i1 } @llvm.smul.with.overflow.i8(i8, i8) #2

; Function Attrs: norecurse nounwind
define zeroext i16 @divu16(i16 zeroext %val) local_unnamed_addr #0 align 2 {
entry:
  %div2 = udiv i16 %val, 10
  ret i16 %div2
}

; Function Attrs: nounwind
define zeroext i1 @mulou16(i16 zeroext %lhs, i16 zeroext %rhs, i16* dereferenceable(2) %res) local_unnamed_addr #1 align 2 {
entry:
  %0 = tail call { i16, i1 } @llvm.umul.with.overflow.i16(i16 %lhs, i16 %rhs)
  %1 = extractvalue { i16, i1 } %0, 1
  %2 = extractvalue { i16, i1 } %0, 0
  store i16 %2, i16* %res, align 2
  ret i1 %1
}

; Function Attrs: nounwind readnone speculatable
declare { i16, i1 } @llvm.umul.with.overflow.i16(i16, i16) #2

; Function Attrs: norecurse nounwind
define signext i16 @divs16(i16 signext %val) local_unnamed_addr #0 align 2 {
entry:
  %0 = sdiv i16 %val, 10
  ret i16 %0
}

; Function Attrs: nounwind
define zeroext i1 @mulos16(i16 signext %lhs, i16 signext %rhs, i16* dereferenceable(2) %res) local_unnamed_addr #1 align 2 {
entry:
  %0 = tail call { i16, i1 } @llvm.smul.with.overflow.i16(i16 %lhs, i16 %rhs)
  %1 = extractvalue { i16, i1 } %0, 1
  %2 = extractvalue { i16, i1 } %0, 0
  store i16 %2, i16* %res, align 2
  ret i1 %1
}

; Function Attrs: nounwind readnone speculatable
declare { i16, i1 } @llvm.smul.with.overflow.i16(i16, i16) #2

; Function Attrs: norecurse nounwind
define i32 @divu32(i32 %val) local_unnamed_addr #0 align 2 {
entry:
  %div = udiv i32 %val, 10
  ret i32 %div
}

; Function Attrs: nounwind
define zeroext i1 @mulou32(i32 %lhs, i32 %rhs, i32* dereferenceable(4) %res) local_unnamed_addr #1 align 2 {
entry:
  %0 = tail call { i32, i1 } @llvm.umul.with.overflow.i32(i32 %lhs, i32 %rhs)
  %1 = extractvalue { i32, i1 } %0, 1
  %2 = extractvalue { i32, i1 } %0, 0
  store i32 %2, i32* %res, align 4
  ret i1 %1
}

; Function Attrs: nounwind readnone speculatable
declare { i32, i1 } @llvm.umul.with.overflow.i32(i32, i32) #2

; Function Attrs: norecurse nounwind
define i32 @divs32(i32 %val) local_unnamed_addr #0 align 2 {
entry:
  %div = sdiv i32 %val, 10
  ret i32 %div
}

; Function Attrs: nounwind
define zeroext i1 @mulos32(i32 %lhs, i32 %rhs, i32* dereferenceable(4) %res) local_unnamed_addr #1 align 2 {
entry:
  %0 = tail call { i32, i1 } @llvm.smul.with.overflow.i32(i32 %lhs, i32 %rhs)
  %1 = extractvalue { i32, i1 } %0, 1
  %2 = extractvalue { i32, i1 } %0, 0
  store i32 %2, i32* %res, align 4
  ret i1 %1
}

; Function Attrs: nounwind readnone speculatable
declare { i32, i1 } @llvm.smul.with.overflow.i32(i32, i32) #2

; Function Attrs: norecurse nounwind
define i64 @divu64(i64 %val) local_unnamed_addr #0 align 2 {
entry:
  %div = udiv i64 %val, 10
  ret i64 %div
}

; Function Attrs: nounwind
define zeroext i1 @mulou64(i64 %lhs, i64 %rhs, i64* dereferenceable(8) %res) local_unnamed_addr #1 align 2 {
entry:
  %0 = tail call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %lhs, i64 %rhs)
  %1 = extractvalue { i64, i1 } %0, 1
  %2 = extractvalue { i64, i1 } %0, 0
  store i64 %2, i64* %res, align 8
  ret i1 %1
}

; Function Attrs: nounwind readnone speculatable
declare { i64, i1 } @llvm.umul.with.overflow.i64(i64, i64) #2

; Function Attrs: norecurse nounwind
define i64 @divs64(i64 %val) local_unnamed_addr #0 align 2 {
entry:
  %div = sdiv i64 %val, 10
  ret i64 %div
}

; Function Attrs: nounwind
define zeroext i1 @mulos64(i64 %lhs, i64 %rhs, i64* dereferenceable(8) %res) local_unnamed_addr #1 align 2 {
entry:
  %0 = tail call { i64, i1 } @llvm.smul.with.overflow.i64(i64 %lhs, i64 %rhs)
  %1 = extractvalue { i64, i1 } %0, 1
  %2 = extractvalue { i64, i1 } %0, 0
  store i64 %2, i64* %res, align 8
  ret i1 %1
}

; Function Attrs: nounwind readnone speculatable
declare { i64, i1 } @llvm.smul.with.overflow.i64(i64, i64) #2

; Function Attrs: norecurse nounwind
define signext i16 @xmuls8(i8 signext %lhs, i8 signext %rhs) local_unnamed_addr #0 align 2 {
; CHECK: xmuls8
; CHECK: xmuls8 %[[DEST:[a-z0-9_]+]]
entry:
  %conv1 = sext i8 %lhs to i16
  %conv3 = sext i8 %rhs to i16
  %mul = mul nsw i16 %conv3, %conv1
  ret i16 %mul
}

; Function Attrs: norecurse nounwind
define zeroext i16 @xmulu8(i8 zeroext %lhs, i8 zeroext %rhs) local_unnamed_addr #0 align 2 {
; CHECK: xmulu8
; CHECK: xmulu8 %[[DEST:[a-z0-9_]+]]
entry:
  %conv1 = zext i8 %lhs to i16
  %conv3 = zext i8 %rhs to i16
  %mul = mul nuw i16 %conv3, %conv1
  ret i16 %mul
}

; Function Attrs: norecurse nounwind
define i32 @xmuls16(i16 signext %lhs, i16 signext %rhs) local_unnamed_addr #0 align 2 {
; CHECK: xmuls16
; CHECK: xmuls16 %[[DEST:[a-z0-9_]+]]
entry:
  %conv = sext i16 %lhs to i32
  %conv1 = sext i16 %rhs to i32
  %mul = mul nsw i32 %conv1, %conv
  ret i32 %mul
}

; Function Attrs: norecurse nounwind
define i32 @xmulu16(i16 zeroext %lhs, i16 zeroext %rhs) local_unnamed_addr #0 align 2 {
; CHECK: xmulu16
; CHECK: xmulu16 %[[DEST:[a-z0-9_]+]]
entry:
  %conv = zext i16 %lhs to i32
  %conv1 = zext i16 %rhs to i32
  %mul = mul nuw i32 %conv1, %conv
  ret i32 %mul
}

; Function Attrs: norecurse nounwind
define i64 @xmuls32(i32 %lhs, i32 %rhs) local_unnamed_addr #0 align 2 {
; CHECK: xmuls32
; CHECK: .result .lic .i64 %[[DEST:[a-z0-9_]+]]
; CHECK: xmuls32 %[[DEST]]
entry:
  %conv = sext i32 %lhs to i64
  %conv1 = sext i32 %rhs to i64
  %mul = mul nsw i64 %conv1, %conv
  ret i64 %mul
}

; Function Attrs: norecurse nounwind
define i64 @xmulu32(i32 %lhs, i32 %rhs) local_unnamed_addr #0 align 2 {
; CHECK: xmulu32
; CHECK: .result .lic .i64 %[[DEST:[a-z0-9_]+]]
; CHECK: xmulu32 %[[DEST]]
entry:
  %conv = zext i32 %lhs to i64
  %conv1 = zext i32 %rhs to i64
  %mul = mul nuw i64 %conv1, %conv
  ret i64 %mul
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (https://git.llvm.org/git/clang.git/ ca0ff8f6a2597cf765cad1835931ca2fe0d1c79e) (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-llvm 37a1e09caf974340c489b1a95d1a7cdac9de28d2)"}
