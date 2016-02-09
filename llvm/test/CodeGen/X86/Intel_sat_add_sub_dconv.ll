; RUN: llc -mtriple=x86_64-pc-linux -mcpu=core-avx2 < %s | FileCheck %s -check-prefix=YMM
; RUN: llc -mtriple=x86_64-pc-linux -mcpu=corei7-avx < %s | FileCheck %s
; RUN: llc -mtriple=x86_64-pc-linux < %s | FileCheck %s

; signed/unsigned saturation add/sub/downconvert
; 256bit wide load is legal on AVX and AVX2.
;
; Checks for the usage for these SSE2 instructions
;   packssdw, packsswb, packuswb
;   paddsw, paddsb, paddusw, paddusb
;   psubsw, psubsb, psubusw, psubusb
;   pminsw, pmaxsw, pminub, pmaxub

define void @test_usat_add_char(i8* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i8, i8* %a, i64 0
  %addr1 = bitcast i8* %aaddr to <16 x i8>*
  %wide.load = load <16 x i8>, <16 x i8>* %addr1, align 1
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr1 = bitcast i8* %baddr to <16 x i8>*
  %wide.load62 = load <16 x i8>, <16 x i8>* %baddr1, align 1
  %res = call <16 x i8> @llvm.usat.add.v16i8(<16 x i8> %wide.load62, <16 x i8> %wide.load, <16 x i8> zeroinitializer, <16 x i8> <i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>)
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_add_char:
; CHECK: paddusb
; YMM-LABEL: test_usat_add_char:
; YMM: vpaddusb

define void @test_usat_sub_char(i8* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i8, i8* %a, i64 0
  %addr1 = bitcast i8* %aaddr to <16 x i8>*
  %wide.load = load <16 x i8>, <16 x i8>* %addr1, align 1
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr1 = bitcast i8* %baddr to <16 x i8>*
  %wide.load62 = load <16 x i8>, <16 x i8>* %baddr1, align 1
  %res = call <16 x i8> @llvm.usat.sub.v16i8(<16 x i8> %wide.load62, <16 x i8> %wide.load, <16 x i8> zeroinitializer, <16 x i8> <i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>)
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_sub_char:
; CHECK: psubusb
; YMM-LABEL: test_usat_sub_char:
; YMM: vpsubusb

define void @test_ssat_add_char(i8* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i8, i8* %a, i64 0
  %addr1 = bitcast i8* %aaddr to <16 x i8>*
  %wide.load = load <16 x i8>, <16 x i8>* %addr1, align 1
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr1 = bitcast i8* %baddr to <16 x i8>*
  %wide.load62 = load <16 x i8>, <16 x i8>* %baddr1, align 1
  %res = call <16 x i8> @llvm.ssat.add.v16i8(<16 x i8> %wide.load62, <16 x i8> %wide.load, <16 x i8> <i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128>, <16 x i8> <i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127>)
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_add_char:
; CHECK: paddsb
; YMM-LABEL: test_ssat_add_char:
; YMM: vpaddsb

define void @test_ssat_sub_char(i8* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i8, i8* %a, i64 0
  %addr1 = bitcast i8* %aaddr to <16 x i8>*
  %wide.load = load <16 x i8>, <16 x i8>* %addr1, align 1
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr1 = bitcast i8* %baddr to <16 x i8>*
  %wide.load62 = load <16 x i8>, <16 x i8>* %baddr1, align 1
  %res = call <16 x i8> @llvm.ssat.sub.v16i8(<16 x i8> %wide.load62, <16 x i8> %wide.load, <16 x i8> <i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128>, <16 x i8> <i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127>)
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_sub_char:
; CHECK: psubsb
; YMM-LABEL: test_ssat_sub_char:
; YMM: vpsubsb

define void @test_usat_add_short(i16* nocapture readonly %a, i16* nocapture %b)
 {
entry:
  %aaddr = getelementptr inbounds i16, i16* %a, i64 0
  %addr1 = bitcast i16* %aaddr to <8 x i16>*
  %wide.load = load <8 x i16>, <8 x i16>* %addr1, align 1
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr1 = bitcast i16* %baddr to <8 x i16>*
  %wide.load62 = load <8 x i16>, <8 x i16>* %baddr1, align 1
  %res = call <8 x i16> @llvm.usat.add.v8i16(<8 x i16> %wide.load62, <8 x i16> %wide.load, <8x i16> zeroinitializer, <8 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>)
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_add_short:
; CHECK: paddusw
; YMM-LABEL: test_usat_add_short:
; YMM: vpaddusw

define void @test_usat_sub_short(i16* nocapture readonly %a, i16* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i16, i16* %a, i64 0
  %addr1 = bitcast i16* %aaddr to <8 x i16>*
  %wide.load = load <8 x i16>, <8 x i16>* %addr1, align 1
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr1 = bitcast i16* %baddr to <8 x i16>*
  %wide.load62 = load <8 x i16>, <8 x i16>* %baddr1, align 1
  %res = call <8 x i16> @llvm.usat.sub.v8i16(<8 x i16> %wide.load62, <8 x i16> %wide.load, <8x i16> zeroinitializer, <8 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>)
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_sub_short:
; CHECK: psubusw
; YMM-LABEL: test_usat_sub_short:
; YMM: vpsubusw

define void @test_ssat_add_short(i16* nocapture readonly %a, i16* nocapture %b)
 {
entry:
  %aaddr = getelementptr inbounds i16, i16* %a, i64 0
  %addr1 = bitcast i16* %aaddr to <8 x i16>*
  %wide.load = load <8 x i16>, <8 x i16>* %addr1, align 1
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr1 = bitcast i16* %baddr to <8 x i16>*
  %wide.load62 = load <8 x i16>, <8 x i16>* %baddr1, align 1
  %res = call <8 x i16> @llvm.ssat.add.v8i16(<8 x i16> %wide.load62, <8 x i16> %wide.load, <8x i16> <i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768>, <8 x i16> <i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767>)
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_add_short:
; CHECK: paddsw
; YMM-LABEL: test_ssat_add_short:
; YMM: vpaddsw

define void @test_ssat_sub_short(i16* nocapture readonly %a, i16* nocapture %b)
 {
entry:
  %aaddr = getelementptr inbounds i16, i16* %a, i64 0
  %addr1 = bitcast i16* %aaddr to <8 x i16>*
  %wide.load = load <8 x i16>, <8 x i16>* %addr1, align 1
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr1 = bitcast i16* %baddr to <8 x i16>*
  %wide.load62 = load <8 x i16>, <8 x i16>* %baddr1, align 1
  %res = call <8 x i16> @llvm.ssat.sub.v8i16(<8 x i16> %wide.load62, <8 x i16> %wide.load, <8x i16> <i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768>, <8 x i16> <i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767>)
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_sub_short:
; CHECK: psubsw
; YMM-LABEL: test_ssat_sub_short:
; YMM: vpsubsw

define void @test_usat_dcnv_short_uchar(i16* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i16, i16* %a, i64 0
  %addr1 = bitcast i16* %aaddr to <16 x i16>*
  %wide.load = load <16 x i16>, <16 x i16>* %addr1, align 1
  %res = call <16 x i8> @llvm.usat.dcnv.v16i8(<16 x i16> %wide.load, <16x i8> zeroinitializer, <16 x i8> <i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>)
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_dcnv_short_uchar:
; CHECK: packuswb
; YMM-LABEL: test_usat_dcnv_short_uchar:
; YMM: vextract
; YMM: vpackuswb

define void @test_usat_dcnv_short_uchar1(i16* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i16, i16* %a, i64 0
  %addr1 = bitcast i16* %aaddr to <16 x i16>*
  %wide.load = load <16 x i16>, <16 x i16>* %addr1, align 1
  %res = call <16 x i8> @llvm.usat.dcnv.v16i8(<16 x i16> %wide.load, <16x i8> zeroinitializer, <16 x i8> <i8 254, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>)
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_dcnv_short_uchar1:
; CHECK: packuswb
; CHECK: pminub
; YMM-LABEL: test_usat_dcnv_short_uchar1:
; YMM: vextract
; YMM: vpackuswb
; YMM: vpminub

define void @test_usat_dcnv_short_uchar2(i16* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i16, i16* %a, i64 0
  %addr1 = bitcast i16* %aaddr to <16 x i16>*
  %wide.load = load <16 x i16>, <16 x i16>* %addr1, align 1
  %res = call <16 x i8> @llvm.usat.dcnv.v16i8(<16 x i16> %wide.load, <16x i8> <i8 1, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0>, <16 x i8> <i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>)
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_dcnv_short_uchar2:
; CHECK: packuswb
; CHECK: pmaxub
; YMM-LABEL: test_usat_dcnv_short_uchar2:
; YMM: vextract
; YMM: vpackuswb
; YMM: vpmaxub


define void @test_usat_dcnv_short_uchar3(i16* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i16, i16* %a, i64 0
  %addr1 = bitcast i16* %aaddr to <16 x i16>*
  %wide.load = load <16 x i16>, <16 x i16>* %addr1, align 1
  %res = call <16 x i8> @llvm.usat.dcnv.v16i8(<16 x i16> %wide.load, <16x i8> <i8 1, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 0>, <16 x i8> <i8 254, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>)
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_usat_dcnv_short_uchar3:
; CHECK: packuswb
; CHECK: pmaxub
; CHECK: pminub
; YMM-LABEL: test_usat_dcnv_short_uchar3:
; YMM: vextract
; YMM: vpackuswb
; YMM: vpmaxub
; YMM: vpminub

define void @test_ssat_dcnv_int_short(i32* nocapture readonly %a, i16* nocapture %b)
 {
entry:
  %aaddr = getelementptr inbounds i32, i32* %a, i64 0
  %addr1 = bitcast i32* %aaddr to <8 x i32>*
  %wide.load = load <8 x i32>, <8 x i32>* %addr1, align 1
  %res = call <8 x i16> @llvm.ssat.dcnv.v8i16(<8 x i32> %wide.load, <8x i16> <i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768>, <8 x i16> <i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767>)
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_dcnv_int_short:
; CHECK: packssdw

define void @test_ssat_dcnv_int_short1(i32* nocapture readonly %a, i16* nocapture %b)
 {
entry:
  %aaddr = getelementptr inbounds i32, i32* %a, i64 0
  %addr1 = bitcast i32* %aaddr to <8 x i32>*
  %wide.load = load <8 x i32>, <8 x i32>* %addr1, align 1
  %res = call <8 x i16> @llvm.ssat.dcnv.v8i16(<8 x i32> %wide.load, <8x i16> <i16 -32767, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768>, <8 x i16> <i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767>)
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_dcnv_int_short1:
; CHECK: packssdw
; CHECK: pmaxsw
; YMM-LABEL: test_ssat_dcnv_int_short1:
; YMM: vextract
; YMM: vpackssdw
; YMM: vpmaxsw

define void @test_ssat_dcnv_int_short2(i32* nocapture readonly %a, i16* nocapture %b)
 {
entry:
  %aaddr = getelementptr inbounds i32, i32* %a, i64 0
  %addr1 = bitcast i32* %aaddr to <8 x i32>*
  %wide.load = load <8 x i32>, <8 x i32>* %addr1, align 1
  %res = call <8 x i16> @llvm.ssat.dcnv.v8i16(<8 x i32> %wide.load, <8x i16> <i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768>, <8 x i16> <i16 32766, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767>)
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_dcnv_int_short2:
; CHECK: packssdw
; CHECK: pminsw
; YMM-LABEL: test_ssat_dcnv_int_short2:
; YMM: vextract
; YMM: packssdw
; YMM: pminsw

define void @test_ssat_dcnv_int_short3(i32* nocapture readonly %a, i16* nocapture %b)
 {
entry:
  %aaddr = getelementptr inbounds i32, i32* %a, i64 0
  %addr1 = bitcast i32* %aaddr to <8 x i32>*
  %wide.load = load <8 x i32>, <8 x i32>* %addr1, align 1
  %res = call <8 x i16> @llvm.ssat.dcnv.v8i16(<8 x i32> %wide.load, <8x i16> <i16 -32767, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768>, <8 x i16> <i16 32766, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767, i16 32767>)
  %baddr = getelementptr inbounds i16, i16* %b, i64 0
  %baddr2 = bitcast i16* %baddr to <8 x i16>*
  store <8 x i16> %res, <8 x i16>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_dcnv_int_short3:
; CHECK: packssdw
; CHECK: pmaxsw
; CHECK: pminsw
; YMM-LABEL: test_ssat_dcnv_int_short3:
; YMM: vextract
; YMM: vpackssdw
; YMM: vpmaxsw
; YMM: vpminsw

define void @test_ssat_dcnv_short_char(i16* nocapture readonly %a, i8* nocapture %b) {
entry:
  %aaddr = getelementptr inbounds i16, i16* %a, i64 0
  %addr1 = bitcast i16* %aaddr to <16 x i16>*
  %wide.load = load <16 x i16>, <16 x i16>* %addr1, align 1
  %res = call <16 x i8> @llvm.ssat.dcnv.v16i8(<16 x i16> %wide.load, <16 x i8> <i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128>, <16 x i8> <i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127>)
  %baddr = getelementptr inbounds i8, i8* %b, i64 0
  %baddr2 = bitcast i8* %baddr to <16 x i8>*
  store <16 x i8> %res, <16 x i8>* %baddr2, align 1
  ret void
}

; CHECK-LABEL: test_ssat_dcnv_short_char:
; CHECK: packsswb
; YMM-LABEL: test_ssat_dcnv_short_char:
; YMM: vextract
; YMM: vpacksswb

declare <16 x i8> @llvm.usat.add.v16i8(<16 x i8>, <16 x i8>, <16 x i8>, <16 x i8>) #0
declare <16 x i8> @llvm.usat.sub.v16i8(<16 x i8>, <16 x i8>, <16 x i8>, <16 x i8>) #0
declare <16 x i8> @llvm.ssat.add.v16i8(<16 x i8>, <16 x i8>, <16 x i8>, <16 x i8>) #0
declare <16 x i8> @llvm.ssat.sub.v16i8(<16 x i8>, <16 x i8>, <16 x i8>, <16 x i8>) #0

declare <8 x i16> @llvm.usat.add.v8i16(<8 x i16>, <8 x i16>, <8 x i16>, <8 x i16>) #0
declare <8 x i16> @llvm.usat.sub.v8i16(<8 x i16>, <8 x i16>, <8 x i16>, <8 x i16>) #0
declare <8 x i16> @llvm.ssat.add.v8i16(<8 x i16>, <8 x i16>, <8 x i16>, <8 x i16>) #0
declare <8 x i16> @llvm.ssat.sub.v8i16(<8 x i16>, <8 x i16>, <8 x i16>, <8 x i16>) #0

declare <16 x i8> @llvm.usat.dcnv.v16i8(<16 x i16>, <16 x i8>, <16 x i8>) #0
declare <16 x i8> @llvm.ssat.dcnv.v16i8(<16 x i16>, <16 x i8>, <16 x i8>) #0
declare <8 x i16> @llvm.ssat.dcnv.v8i16(<8 x i32>, <8 x i16>, <8 x i16>) #0
