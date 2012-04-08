; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <16 x float> @test1(i8* %ptr, <16 x i1> %mask) nounwind readnone ssp {
entry:
; KNF: test1:
; KNF: vloadunpackld {{\(%[a-z]+\)}}, [[R1:%v[0-9]+]]{%k1}
; KNF: vloadunpackhd {{\(%[a-z]+\)}}, [[R1]]{%k1}
  %vall = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> undef, <16 x i1> %mask, i8* %ptr);
  %vals = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %vall, <16 x i1> %mask, i8* %ptr);
  ret <16 x float> %vals
}

define <16 x i32> @test2(i8* %ptr, <16 x i1> %mask) nounwind readnone ssp {
entry:
; KNF: test2:
; KNF: vloadunpackld {{\(%[a-z]+\)}}, [[R1:%v[0-9]+]]{%k1}
; KNF: vloadunpackhd {{\(%[a-z]+\)}}, [[R1]]{%k1}
  %vall = call <16 x i32> @llvm.x86.mic.mask.loadunpackl.pi(<16 x i32> undef, <16 x i1> %mask, i8* %ptr);
  %vals = call <16 x i32> @llvm.x86.mic.mask.loadunpackh.pi(<16 x i32> %vall, <16 x i1> %mask, i8* %ptr);
  ret <16 x i32> %vals
}

define <8 x double> @test3(i8* %ptr, <8 x i1> %mask) nounwind readnone ssp {
entry:
; KNF: test3:
; KNF: vloadunpacklq {{\(%[a-z]+\)}}, [[R1:%v[0-9]+]]{%k1}
; KNF: vloadunpackhq {{\(%[a-z]+\)}}, [[R1]]{%k1}
  %vall = call <8 x double> @llvm.x86.mic.mask.loadunpackl.pd(<8 x double> undef, <8 x i1> %mask, i8* %ptr);
  %vals = call <8 x double> @llvm.x86.mic.mask.loadunpackh.pd(<8 x double> %vall, <8 x i1> %mask, i8* %ptr);
  ret <8 x double> %vals
}

define <8 x i64> @test4(i8* %ptr, <8 x i1> %mask) nounwind readnone ssp {
entry:
; KNF: test4:
; KNF: vloadunpacklq {{\(%[a-z]+\)}}, [[R1:%v[0-9]+]]{%k1}
; KNF: vloadunpackhq {{\(%[a-z]+\)}}, [[R1]]{%k1}
  %vall = call <8 x i64> @llvm.x86.mic.mask.loadunpackl.pq(<8 x i64> undef, <8 x i1> %mask, i8* %ptr);
  %vals = call <8 x i64> @llvm.x86.mic.mask.loadunpackh.pq(<8 x i64> %vall, <8 x i1> %mask, i8* %ptr);
  ret <8 x i64> %vals
}

define void @test5(i8* %ptr, <16 x i1> %mask, <16 x float> %vals) nounwind readnone ssp {
entry:
; KNF: test5:
; KNF: vpackstoreld [[R1:%v[0-9]+]], {{\(%[a-z]+\)}}{%k1}
; KNF: vpackstorehd [[R1]], {{\(%[a-z]+\)}}{%k1}
  call void @llvm.x86.mic.mask.packstorel.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %vals);
  call void @llvm.x86.mic.mask.packstoreh.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %vals);
  ret void
}

define void @test6(i8* %ptr, <16 x i1> %mask, <16 x i32> %vals) nounwind readnone ssp {
entry:
; KNF: test6:
; KNF: vpackstoreld [[R1:%v[0-9]+]], {{\(%[a-z]+\)}}{%k1}
; KNF: vpackstorehd [[R1]], {{\(%[a-z]+\)}}{%k1}
  call void @llvm.x86.mic.mask.packstorel.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %vals);
  call void @llvm.x86.mic.mask.packstoreh.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %vals);
  ret void
}

define void @test7(i8* %ptr, <8 x i1> %mask, <8 x i64> %vals) nounwind readnone ssp {
entry:
; KNF: test7:
; KNF: vpackstorelq [[R1:%v[0-9]+]], {{\(%[a-z]+\)}}{%k1}
; KNF: vpackstorehq [[R1]], {{\(%[a-z]+\)}}{%k1}
  call void @llvm.x86.mic.mask.packstorel.pq(i8* %ptr, <8 x i1> %mask, <8 x i64> %vals);
  call void @llvm.x86.mic.mask.packstoreh.pq(i8* %ptr, <8 x i1> %mask, <8 x i64> %vals);
  ret void
}

define void @test8(i8* %ptr, <8 x i1> %mask, <8 x double> %vals) nounwind readnone ssp {
entry:
; KNF: test8:
; KNF: vpackstorelq [[R1:%v[0-9]+]], {{\(%[a-z]+\)}}{%k1}
; KNF: vpackstorehq [[R1]], {{\(%[a-z]+\)}}{%k1}
  call void @llvm.x86.mic.mask.packstorel.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %vals);
  call void @llvm.x86.mic.mask.packstoreh.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %vals);
  ret void
}

declare <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float>, <16 x i1>, i8*);
declare <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float>, <16 x i1>, i8*);
declare <16 x i32> @llvm.x86.mic.mask.loadunpackl.pi(<16 x i32>, <16 x i1>, i8*);
declare <16 x i32> @llvm.x86.mic.mask.loadunpackh.pi(<16 x i32>, <16 x i1>, i8*);
declare <8 x double> @llvm.x86.mic.mask.loadunpackl.pd(<8 x double>, <8 x i1>, i8*);
declare <8 x double> @llvm.x86.mic.mask.loadunpackh.pd(<8 x double>, <8 x i1>, i8*);
declare <8 x i64> @llvm.x86.mic.mask.loadunpackl.pq(<8 x i64>, <8 x i1>, i8*);
declare <8 x i64> @llvm.x86.mic.mask.loadunpackh.pq(<8 x i64>, <8 x i1>, i8*);

declare void @llvm.x86.mic.mask.packstorel.ps(i8*, <16 x i1>, <16 x float>);
declare void @llvm.x86.mic.mask.packstoreh.ps(i8*, <16 x i1>, <16 x float>);
declare void @llvm.x86.mic.mask.packstorel.pi(i8*, <16 x i1>, <16 x i32>);
declare void @llvm.x86.mic.mask.packstoreh.pi(i8*, <16 x i1>, <16 x i32>);
declare void @llvm.x86.mic.mask.packstorel.pd(i8*, <8 x i1>, <8 x double>);
declare void @llvm.x86.mic.mask.packstoreh.pd(i8*, <8 x i1>, <8 x double>);
declare void @llvm.x86.mic.mask.packstorel.pq(i8*, <8 x i1>, <8 x i64>);
declare void @llvm.x86.mic.mask.packstoreh.pq(i8*, <8 x i1>, <8 x i64>);
