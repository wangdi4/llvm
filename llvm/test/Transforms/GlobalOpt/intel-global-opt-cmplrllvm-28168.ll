; RUN: opt < %s -S -passes="globalopt" | FileCheck %s

; CMPLRLLVM-28168: Add nullptr check to avoid seg fault when processing
; MemTransferInst in CleanupPointerRootUsers in GlobalOpt.cpp.

; These specific checks are required, as no tranformation should occur.
; We need some checks and must ensure that we do not seg fault.

; CHECK: define void @MAIN__
; CHECK: %"var$145" = alloca %"ROMAN_NUMERALS$.btROMAN"
; CHECK: call void @llvm.memcpy.p0.p0.i64

%"ROMAN_NUMERALS$.btROMAN" = type { i32, %"QNCA_a0$i8*$rank1$" }
%"QNCA_a0$i8*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@"test_roman_numerals_$TOO_BIG" = internal unnamed_addr global %"ROMAN_NUMERALS$.btROMAN" zeroinitializer, align 8

declare void @"llvm.memcpy.p0s_ROMAN_NUMERALS$.btROMANs.p0s_ROMAN_NUMERALS$.btROMANs.i64"(ptr noalias nocapture writeonly %0, ptr noalias nocapture readonly %1, i64 %2, i1 immarg %3) #3

define void @MAIN__() local_unnamed_addr {
  %"var$145" = alloca %"ROMAN_NUMERALS$.btROMAN", align 8
  call void @"llvm.memcpy.p0s_ROMAN_NUMERALS$.btROMANs.p0s_ROMAN_NUMERALS$.btROMANs.i64"(ptr @"test_roman_numerals_$TOO_BIG", ptr %"var$145", i64 80, i1 false)
  ret void
}
