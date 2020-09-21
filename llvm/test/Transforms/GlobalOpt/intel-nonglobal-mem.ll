; RUN: opt < %s -globalopt -S | FileCheck %s
; CHECK: define{{.*}}MAIN
; CHECK: memcpy{{.*}}test_roman_numerals

; Crash in globalopt caused by assumption that memcpy args will both be
; global or local.

source_filename = "gh.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"ROMAN_NUMERALS$.btROMAN.1.10.19.28.37.82.91.100.109.118.127.136.154.181.190.199.226" = type { i32, %"QNCA_a0$i8*$rank1$.0.9.18.27.36.81.90.99.108.117.126.135.153.180.189.198.225" }
%"QNCA_a0$i8*$rank1$.0.9.18.27.36.81.90.99.108.117.126.135.153.180.189.198.225" = type { i8*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@"test_roman_numerals_$TOO_BIG" = internal global %"ROMAN_NUMERALS$.btROMAN.1.10.19.28.37.82.91.100.109.118.127.136.154.181.190.199.226" zeroinitializer, align 8

define void @MAIN__() #0 {
alloca_64:
  br label %bb955

bb955:                                            ; preds = %bb955, %alloca_64
  br i1 undef, label %bb955, label %bb958

bb958:                                            ; preds = %bb955
  call void @"llvm.memcpy.p0s_ROMAN_NUMERALS$.btROMAN.1.10.19.28.37.82.91.100.109.118.127.136.154.181.190.199.226s.p0s_ROMAN_NUMERALS$.btROMAN.1.10.19.28.37.82.91.100.109.118.127.136.154.181.190.199.226s.i64"(%"ROMAN_NUMERALS$.btROMAN.1.10.19.28.37.82.91.100.109.118.127.136.154.181.190.199.226"* @"test_roman_numerals_$TOO_BIG", %"ROMAN_NUMERALS$.btROMAN.1.10.19.28.37.82.91.100.109.118.127.136.154.181.190.199.226"* undef, i64 80, i1 false)
  unreachable
}

; Function Attrs: argmemonly nounwind willreturn
declare void @"llvm.memcpy.p0s_ROMAN_NUMERALS$.btROMAN.1.10.19.28.37.82.91.100.109.118.127.136.154.181.190.199.226s.p0s_ROMAN_NUMERALS$.btROMAN.1.10.19.28.37.82.91.100.109.118.127.136.154.181.190.199.226s.i64"(%"ROMAN_NUMERALS$.btROMAN.1.10.19.28.37.82.91.100.109.118.127.136.154.181.190.199.226"* noalias nocapture writeonly, %"ROMAN_NUMERALS$.btROMAN.1.10.19.28.37.82.91.100.109.118.127.136.154.181.190.199.226"* noalias nocapture readonly, i64, i1 immarg) #1

attributes #0 = { "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { argmemonly nounwind willreturn }

!omp_offload.info = !{}
