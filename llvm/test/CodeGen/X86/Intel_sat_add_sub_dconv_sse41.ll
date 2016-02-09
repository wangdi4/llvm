; RUN: llc -mtriple=x86_64-pc-linux -mcpu=core-avx2 < %s | FileCheck %s -check-prefix=YMM
; RUN: llc -mtriple=x86_64-pc-linux -mcpu=corei7-avx < %s | FileCheck %s
; RUN: llc -mtriple=x86_64-pc-linux -mattr=sse4.1 < %s | FileCheck %s

; signed/unsigned saturation add/sub/downconvert
; 256bit wide load is legal on AVX and AVX2.
;
; Checks for the usage for these SSE4.2 instructions
; and wider than full vector legalization
;   packusdw
;   pminuw, pmaxuw, pminsb, pmaxsb

define void @test_usat_dcnv_int_ushort(i32* nocapture readonly %a, i16* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i32, i32* %a, i64 0
  %addr1 = bitcast i32* %aaddr to <8 x i32>*
  %wide.load = load <8 x i32>, <8 x i32>* %addr1, align 1
  %res = call <8 x i16> @llvm.usat.dcnv.v8i16(<8 x i32> %wide.load, <8x i16> zeroinitializer, <8 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>)
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_dcnv_int_ushort:
; CHECK: packusdw
; YMM-LABEL: test_usat_dcnv_int_ushort:
; YMM: vextract
; YMM: vpackusdw

define void @test_usat_dcnv_int_ushort1(i32* nocapture readonly %a, i16* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i32, i32* %a, i64 0
  %addr1 = bitcast i32* %aaddr to <8 x i32>*
  %wide.load = load <8 x i32>, <8 x i32>* %addr1, align 1
  %res = call <8 x i16> @llvm.usat.dcnv.v8i16(<8 x i32> %wide.load, <8x i16> zeroinitializer, <8 x i16> <i16 65534, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>)
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_dcnv_int_ushort1:
; CHECK: packusdw
; CHECK: pminuw
; YMM-LABEL: test_usat_dcnv_int_ushort1:
; YMM: vextract
; YMM: vpackusdw
; YMM: vpminuw

define void @test_usat_dcnv_int_ushort2(i32* nocapture readonly %a, i16* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i32, i32* %a, i64 0
  %addr1 = bitcast i32* %aaddr to <8 x i32>*
  %wide.load = load <8 x i32>, <8 x i32>* %addr1, align 1
  %res = call <8 x i16> @llvm.usat.dcnv.v8i16(<8 x i32> %wide.load, <8x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 0, i16 0, i16 0>, <8 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>)
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_dcnv_int_ushort2:
; CHECK: packusdw
; CHECK: pmaxuw
; YMM-LABEL: test_usat_dcnv_int_ushort2:
; YMM: vextract
; YMM: vpackusdw
; YMM: vpmaxuw

define void @test_usat_dcnv_int_ushort3(i32* nocapture readonly %a, i16* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i32, i32* %a, i64 0
  %addr1 = bitcast i32* %aaddr to <8 x i32>*
  %wide.load = load <8 x i32>, <8 x i32>* %addr1, align 1
  %res = call <8 x i16> @llvm.usat.dcnv.v8i16(<8 x i32> %wide.load, <8x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 0, i16 0, i16 0>, <8 x i16> <i16 65534, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>)
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_dcnv_int_ushort3:
; CHECK: packusdw
; CHECK: pmaxuw
; CHECK: pminuw
; YMM-LABEL: test_usat_dcnv_int_ushort3:
; YMM: vextract
; YMM: vpackusdw
; YMM: vpmaxuw
; YMM: vpminuw

define void @test_usat_dcnv_int_ushort4(i32* nocapture readonly %a, i16* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i32, i32* %a, i64 0
  %addr1 = bitcast i32* %aaddr to <16 x i32>*
  %wide.load = load <16 x i32>, <16 x i32>* %addr1, align 1
  %res = call <16 x i16> @llvm.usat.dcnv.v16i16(<16 x i32> %wide.load, <16x i16> zeroinitializer, <16 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>)
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr2 = bitcast i16* %baddr to <16 x i16>*
  store <16 x i16> %res, <16 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_dcnv_int_ushort4:
; CHECK: packusdw
; CHECK: packusdw
; YMM-LABEL: test_usat_dcnv_int_ushort4:
; YMM: vpackusdw
; YMM: vpermq

define void @test_usat_dcnv_int_ushort5(i32* nocapture readonly %a, i16* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i32, i32* %a, i64 0
  %addr1 = bitcast i32* %aaddr to <32 x i32>*
  %wide.load = load <32 x i32>, <32 x i32>* %addr1, align 1
  %res = call <32 x i16> @llvm.usat.dcnv.v32i16(<32 x i32> %wide.load, <32x i16> zeroinitializer, <32 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>)
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr2 = bitcast i16* %baddr to <32 x i16>*
  store <32 x i16> %res, <32 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_dcnv_int_ushort5:
; CHECK: packusdw
; CHECK: packusdw
; CHECK: packusdw
; CHECK: packusdw
; YMM-LABEL: test_usat_dcnv_int_ushort5:
; YMM: vpackusdw
; YMM: vpermq
; YMM: vpackusdw
; YMM: vpermq

define void @test_ssat_add_char1(i8* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i8, i8* %a, i64 0
  %addr1 = bitcast i8* %aaddr to <16 x i8>*
  %wide.load = load <16 x i8>, <16 x i8>* %addr1, align 1
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr1 = bitcast i8* %baddr to <16 x i8>*
  %wide.load62 = load <16 x i8>, <16 x i8>* %baddr1, align 1
  %res = call <16 x i8> @llvm.ssat.add.v16i8(<16 x i8> %wide.load62, <16 x i8> %wide.load, <16 x i8> <i8 -127, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128>, <16 x i8> <i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127>)
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_add_char1:
; CHECK: paddsb
; CHECK: pmaxsb
; YMM-LABEL: test_ssat_add_char1:
; YMM: vpaddsb
; YMM: vpmaxsb

define void @test_ssat_add_char2(i8* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i8, i8* %a, i64 0
  %addr1 = bitcast i8* %aaddr to <16 x i8>*
  %wide.load = load <16 x i8>, <16 x i8>* %addr1, align 1
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr1 = bitcast i8* %baddr to <16 x i8>*
  %wide.load62 = load <16 x i8>, <16 x i8>* %baddr1, align 1
  %res = call <16 x i8> @llvm.ssat.add.v16i8(<16 x i8> %wide.load62, <16 x i8> %wide.load, <16 x i8> <i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128>, <16 x i8> <i8 126, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127>)
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_add_char2:
; CHECK: paddsb
; CHECK: pminsb
; YMM-LABEL: test_ssat_add_char2:
; YMM: vpaddsb
; YMM: vpminsb

define void @test_ssat_add_char3(i8* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i8, i8* %a, i64 0
  %addr1 = bitcast i8* %aaddr to <16 x i8>*
  %wide.load = load <16 x i8>, <16 x i8>* %addr1, align 1
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr1 = bitcast i8* %baddr to <16 x i8>*
  %wide.load62 = load <16 x i8>, <16 x i8>* %baddr1, align 1
  %res = call <16 x i8> @llvm.ssat.add.v16i8(<16 x i8> %wide.load62, <16 x i8> %wide.load, <16 x i8> <i8 -127, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128>, <16 x i8> <i8 126, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127>)
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_add_char3:
; CHECK: paddsb
; CHECK: pmaxsb
; CHECK: pminsb
; YMM-LABEL: test_ssat_add_char3:
; YMM: vpaddsb
; YMM: vpmaxsb
; YMM: vpminsb

define void @test_ssat_add_char4(i8* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i8, i8* %a, i64 0
  %addr1 = bitcast i8* %aaddr to <32 x i8>*
  %wide.load = load <32 x i8>, <32 x i8>* %addr1, align 1
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr1 = bitcast i8* %baddr to <32 x i8>*
  %wide.load62 = load <32 x i8>, <32 x i8>* %baddr1, align 1
  %res = call <32 x i8> @llvm.ssat.add.v32i8(<32 x i8> %wide.load62, <32 x i8> %wide.load, <32 x i8> <i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128>, <32 x i8> <i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127>)
  %baddr2 = bitcast i8* %baddr to <32 x i8>*
  store <32 x i8> %res, <32 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_add_char4:
; CHECK: paddsb
; CHECK: paddsb
; YMM-LABEL: test_ssat_add_char4:
; YMM: vpaddsb

define void @test_ssat_add_char5(i8* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i8, i8* %a, i64 0
  %addr1 = bitcast i8* %aaddr to <64 x i8>*
  %wide.load = load <64 x i8>, <64 x i8>* %addr1, align 1
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr1 = bitcast i8* %baddr to <64 x i8>*
  %wide.load62 = load <64 x i8>, <64 x i8>* %baddr1, align 1
  %res = call <64 x i8> @llvm.ssat.add.v64i8(<64 x i8> %wide.load62, <64 x i8> %wide.load, <64 x i8> <i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128>, <64 x i8> <i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127>)
  %baddr2 = bitcast i8* %baddr to <64 x i8>*
  store <64 x i8> %res, <64 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_add_char5:
; CHECK: paddsb
; CHECK: paddsb
; CHECK: paddsb
; CHECK: paddsb
; YMM-LABEL: test_ssat_add_char5:
; YMM: vpaddsb
; YMM: vpaddsb

declare <16 x i8> @llvm.ssat.add.v16i8(<16 x i8>, <16 x i8>, <16 x i8>, <16 x i8>) #0
declare <32 x i8> @llvm.ssat.add.v32i8(<32 x i8>, <32 x i8>, <32 x i8>, <32 x i8>) #0
declare <64 x i8> @llvm.ssat.add.v64i8(<64 x i8>, <64 x i8>, <64 x i8>, <64 x i8>) #0

declare <8 x i16> @llvm.usat.dcnv.v8i16(<8 x i32>, <8 x i16>, <8 x i16>) #0
declare <16 x i16> @llvm.usat.dcnv.v16i16(<16 x i32>, <16 x i16>, <16 x i16>) #0
declare <32 x i16> @llvm.usat.dcnv.v32i16(<32 x i32>, <32 x i16>, <32 x i16>) #0
