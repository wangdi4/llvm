; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s

; Tests the up-converting vector loads from unsigned ints, supported by KNC.

define <16 x float> @uint8_to_float(<16 x i8>* %ptr) nounwind readonly {
entry:
  ; CHECK-NOT: vmovdqa32
  ; CHECK_NOT: vpandd
  ; CHECK: vcvtfxpntudq2ps $0, (%rdi){uint8}, %zmm0
  ; CHECK-NEXT: ret
  %load = load <16 x i8>* %ptr
  %convert = uitofp <16 x i8> %load to <16 x float>
  ret <16 x float> %convert
}

define <16 x float> @sint8_to_float(<16 x i8>* %ptr) nounwind readonly {
entry:
  ; CHECK-NOT: vmovdqa32
  ; CHECK_NOT: vpandd
  ; CHECK: vcvtfxpntdq2ps $0, (%rdi){sint8}, %zmm0
  ; CHECK-NEXT: ret
  %load = load <16 x i8>* %ptr
  %convert = sitofp <16 x i8> %load to <16 x float>
  ret <16 x float> %convert
}

define <16 x float> @uint16_to_float(<16 x i16>* %ptr) nounwind readonly {
entry:
  ; CHECK-NOT: vmovdqa32
  ; CHECK_NOT: vpandd
  ; CHECK: vcvtfxpntudq2ps $0, (%rdi){uint16}, %zmm0
  ; CHECK-NEXT: ret
  %load = load <16 x i16>* %ptr
  %convert = uitofp <16 x i16> %load to <16 x float>
  ret <16 x float> %convert
}

define <16 x float> @sint16_to_float(<16 x i16>* %ptr) nounwind readonly {
entry:
  ; CHECK-NOT: vmovdqa32
  ; CHECK_NOT: vpandd
  ; CHECK: vcvtfxpntdq2ps $0, (%rdi){sint16}, %zmm0
  ; CHECK-NEXT: ret
  %load = load <16 x i16>* %ptr
  %convert = sitofp <16 x i16> %load to <16 x float>
  ret <16 x float> %convert
}

define <16 x i32> @uint8_to_i32(<16 x i8>* %ptr) nounwind readonly {
entry:
  ; CHECK: vmovdqa32 (%rdi){uint8}, %zmm0
  ; CHECK-NEXT: ret
  %load = load <16 x i8>* %ptr
  %convert = zext <16 x i8> %load to <16 x i32>
  ret <16 x i32> %convert
}

define <16 x i32> @sint8_to_i32(<16 x i8>* %ptr) nounwind readonly {
entry:
  ; CHECK: vmovdqa32 (%rdi){sint8}, %zmm0
  ; CHECK-NEXT: ret
  %load = load <16 x i8>* %ptr
  %convert = sext <16 x i8> %load to <16 x i32>
  ret <16 x i32> %convert
}

define <16 x i32> @uint16_to_i32(<16 x i16>* %ptr) nounwind readonly {
entry:
  ; CHECK: vmovdqa32 (%rdi){uint16}, %zmm0
  ; CHECK-NEXT: ret
  %load = load <16 x i16>* %ptr
  %convert = zext <16 x i16> %load to <16 x i32>
  ret <16 x i32> %convert
}

define <16 x i32> @sint16_to_i32(<16 x i16>* %ptr) nounwind readonly {
entry:
  ; CHECK: vmovdqa32 (%rdi){sint16}, %zmm0
  ; CHECK-NEXT: ret
  %load = load <16 x i16>* %ptr
  %convert = sext <16 x i16> %load to <16 x i32>
  ret <16 x i32> %convert
}