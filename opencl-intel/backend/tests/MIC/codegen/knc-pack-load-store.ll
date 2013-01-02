; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc | FileCheck %s 

;
;

target datalayout = "e-p:64:64"

define <16 x float> @test1(i8* %ptr, i16 %mask) nounwind readnone ssp {
entry:
; CHECK: test1:
; CHECK: vloadunpacklps (%rdi), %zmm0{%k1}
; CHECK: vloadunpackhps (%rdi), %zmm0{%k1}

  %vall = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> undef, i16 %mask, i8* %ptr, i32 0, i32 0);
  %vals = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %vall, i16 %mask, i8* %ptr, i32 0, i32 0);
  ret <16 x float> %vals
}

define <16 x i32> @test2(i8* %ptr, i16 %mask) nounwind readnone ssp {
entry:
; CHECK: test2:
; CHECK: vloadunpackld (%rdi), %zmm0{%k1}
; CHECK: vloadunpackhd (%rdi), %zmm0{%k1}
  %vall = call <16 x i32> @llvm.x86.mic.mask.loadunpackl.pi(<16 x i32> undef, i16 %mask, i8* %ptr, i32 0, i32 0); 
  %vals = call <16 x i32> @llvm.x86.mic.mask.loadunpackh.pi(<16 x i32> %vall, i16 %mask, i8* %ptr, i32 0, i32 0);
  ret <16 x i32> %vals
}

define <8 x double> @test3(i8* %ptr, i8 %mask) nounwind readnone ssp {
entry:
; CHECK: test3:
; CHECK: vloadunpacklpd (%rdi), %zmm0{%k1}
; CHECK: vloadunpackhpd (%rdi), %zmm0{%k1}
  %vall = call <8 x double> @llvm.x86.mic.mask.loadunpackl.pd(<8 x double> undef, i8 %mask, i8* %ptr, i32 0, i32 0);
  %vals = call <8 x double> @llvm.x86.mic.mask.loadunpackh.pd(<8 x double> %vall, i8 %mask, i8* %ptr, i32 0, i32 0);
  ret <8 x double> %vals
}

define <8 x i64> @test4(i8* %ptr, i8 %mask) nounwind readnone ssp {
entry:
; CHECK: test4:
; CHECK: vloadunpacklq (%rdi), %zmm0{%k1}
; CHECK: vloadunpackhq (%rdi), %zmm0{%k1}
  %vall = call <8 x i64> @llvm.x86.mic.mask.loadunpackl.pq(<8 x i64> undef, i8 %mask, i8* %ptr, i32 0, i32 0);
  %vals = call <8 x i64> @llvm.x86.mic.mask.loadunpackh.pq(<8 x i64> %vall, i8 %mask, i8* %ptr, i32 0, i32 0);
  ret <8 x i64> %vals
}

define void @test5(i8* %ptr, i16 %mask, <16 x float> %vals) nounwind readnone ssp {
entry:
; CHECK: test5:
; CHECK: vpackstorelps %zmm0, (%rdi){%k1}
; CHECK: vpackstorehps %zmm0, (%rdi){%k1}
  call void @llvm.x86.mic.mask.packstorel.ps(i8* %ptr, i16 %mask, <16 x float> %vals, i32 0, i32 0);
  call void @llvm.x86.mic.mask.packstoreh.ps(i8* %ptr, i16 %mask, <16 x float> %vals, i32 0, i32 0);
  ret void
}

define void @test6(i8* %ptr, i16 %mask, <16 x i32> %vals) nounwind readnone ssp {
entry:
; CHECK: test6:
; CHECK: vpackstoreld %zmm0, (%rdi){%k1}
; CHECK: vpackstorehd %zmm0, (%rdi){%k1}
  call void @llvm.x86.mic.mask.packstorel.pi(i8* %ptr, i16 %mask, <16 x i32> %vals, i32 0, i32 0);
  call void @llvm.x86.mic.mask.packstoreh.pi(i8* %ptr, i16 %mask, <16 x i32> %vals, i32 0, i32 0);
  ret void
}

define void @test7(i8* %ptr, i8 %mask, <8 x i64> %vals) nounwind readnone ssp {
entry:
; CHECK: test7:
; CHECK: vpackstorelq %zmm0, (%rdi){%k1}
; CHECK: vpackstorehq %zmm0, (%rdi){%k1}
  call void @llvm.x86.mic.mask.packstorel.pq(i8* %ptr, i8 %mask, <8 x i64> %vals, i32 0, i32 0);
  call void @llvm.x86.mic.mask.packstoreh.pq(i8* %ptr, i8 %mask, <8 x i64> %vals, i32 0, i32 0);
  ret void
}

define void @test8(i8* %ptr, i8 %mask, <8 x double> %vals) nounwind readnone ssp {
entry:
; CHECK: test8:
; CHECK: vpackstorelpd %zmm0, (%rdi){%k1}
; CHECK: vpackstorehpd %zmm0, (%rdi){%k1}
  call void @llvm.x86.mic.mask.packstorel.pd(i8* %ptr, i8 %mask, <8 x double> %vals, i32 0, i32 0);
  call void @llvm.x86.mic.mask.packstoreh.pd(i8* %ptr, i8 %mask, <8 x double> %vals, i32 0, i32 0);
  ret void
}

declare <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float>, i16, i8*, i32, i32);
declare <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float>, i16, i8*, i32, i32);
declare <16 x i32> @llvm.x86.mic.mask.loadunpackl.pi(<16 x i32>, i16, i8*, i32, i32); 
declare <16 x i32> @llvm.x86.mic.mask.loadunpackh.pi(<16 x i32>, i16, i8*, i32, i32);
declare <8 x double> @llvm.x86.mic.mask.loadunpackl.pd(<8 x double>, i8, i8*, i32, i32);
declare <8 x double> @llvm.x86.mic.mask.loadunpackh.pd(<8 x double>, i8, i8*, i32, i32);
declare <8 x i64> @llvm.x86.mic.mask.loadunpackl.pq(<8 x i64>, i8, i8*, i32, i32);
declare <8 x i64> @llvm.x86.mic.mask.loadunpackh.pq(<8 x i64>, i8, i8*, i32, i32);

declare void @llvm.x86.mic.mask.packstorel.ps(i8*, i16, <16 x float>, i32, i32);
declare void @llvm.x86.mic.mask.packstoreh.ps(i8*, i16, <16 x float>, i32, i32);
declare void @llvm.x86.mic.mask.packstorel.pi(i8*, i16, <16 x i32>, i32, i32);
declare void @llvm.x86.mic.mask.packstoreh.pi(i8*, i16, <16 x i32>, i32, i32);
declare void @llvm.x86.mic.mask.packstorel.pd(i8*, i8, <8 x double>, i32, i32);
declare void @llvm.x86.mic.mask.packstoreh.pd(i8*, i8, <8 x double>, i32, i32);
declare void @llvm.x86.mic.mask.packstorel.pq(i8*, i8, <8 x i64>, i32, i32);
declare void @llvm.x86.mic.mask.packstoreh.pq(i8*, i8, <8 x i64>, i32, i32);
