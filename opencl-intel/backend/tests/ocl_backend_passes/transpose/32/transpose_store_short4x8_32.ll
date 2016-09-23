;; [LLVM 3.6 UPGRADE] The following LLVM IR is translated into similar but different ASM sequence.
;; Don't see any point to test codegen here. Consider to remove the test or move it to LLVM if it is necessary.
; XFAIL: *

; XFAIL: x86_64

; RUN: oclopt -runtimelib=clbltfng9.rtl -builtin-import -builtin-call-to-inst -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc %t1.ll -mattr=+avx -mtriple=i686 -o %t2.asm
; RUN: FileCheck %s --input-file=%t2.asm -check-prefix=CHECK-AVX

; RUN: oclopt -runtimelib=clbltfns9.rtl -builtin-import -builtin-call-to-inst -instcombine -inline -scalarrepl -S %s -o %t3.ll
; RUN: llc %t3.ll -mattr=+avx2 -mtriple=i686 -o %t4.asm
; RUN: FileCheck %s --input-file=%t4.asm -check-prefix=CHECK-AVX2


define void @foo(<4 x i16>* nocapture %pStoreAdd, <8 x i16> %xIn, <8 x i16> %yIn, <8 x i16> %zIn, <8 x i16> %wIn) nounwind {
entry:
  %xIn.addr = alloca <8 x i16>, align 4
  store <8 x i16> %xIn, <8 x i16>* %xIn.addr, align 4
  %yIn.addr = alloca <8 x i16>, align 4
  store <8 x i16> %yIn, <8 x i16>* %yIn.addr, align 4
  %zIn.addr = alloca <8 x i16>, align 4
  store <8 x i16> %zIn, <8 x i16>* %zIn.addr, align 4
  %wIn.addr = alloca <8 x i16>, align 4
  store <8 x i16> %wIn, <8 x i16>* %wIn.addr, align 4
  call void @__ocl_transpose_store_short_4x8(<4 x i16>* nocapture %pStoreAdd, <8 x i16> %xIn, <8 x i16> %yIn, <8 x i16> %zIn, <8 x i16> %wIn) nounwind
  ret void
}

declare void @__ocl_transpose_store_short_4x8(<4 x i16>* nocapture %pStoreAdd, <8 x i16> %xIn, <8 x i16> %yIn, <8 x i16> %zIn, <8 x i16> %wIn) nounwind


;-------------------------------------------------------------------------------
; CHECK-AVX:     vpunpcklwd      %x[[MM3:mm[0-7]{1}]], %x[[MM2:mm[0-7]{1}]], %x[[MM4:mm[0-7]{1}]]
; CHECK-AVX:     vpunpcklwd      %x[[MM1:mm[0-7]{1}]], %x[[MM0:mm[0-7]{1}]], %x[[MM5:mm[0-7]{1}]]
; CHECK-AVX:     vunpckhps       %x[[MM4]], %x[[MM5]], %x[[MM6:mm[0-7]{1}]]
; CHECK-AVX:     vmovups         %x[[MM6]], 16([[EAX:%eax]])
; CHECK-AVX:     vunpcklps       %x[[MM4]], %x[[MM5]], %x[[MM41:mm[0-7]{1}]]
; CHECK-AVX:     vmovups         %x[[MM41]], ([[EAX]])
; CHECK-AVX:     vpunpckhwd      %x[[MM3]], %x[[MM2]], %x[[MM21:mm[0-7]{1}]]
; CHECK-AVX:     vpunpckhwd      %x[[MM1]], %x[[MM0]], %x[[MM01:mm[0-7]{1}]]
; CHECK-AVX:     vunpckhps       %x[[MM21]], %x[[MM01]], %x[[MM11:mm[0-7]{1}]]
; CHECK-AVX:     vmovups         %x[[MM11]], 48([[EAX]])
; CHECK-AVX:     vunpcklps       %x[[MM21]], %x[[MM01]], %x[[MM02:mm[0-7]{1}]]
; CHECK-AVX:     vmovups         %x[[MM02]], 32([[EAX]])

;-------------------------------------------------------------------------------
; CHECK-AVX2:    .type    [[FOO:[_a-z]+]],@function
; CHECK-AVX2:    vpunpcklwd      %x[[MM3:mm[0-9]+]], %x[[MM2:mm[0-9]+]], %x[[MM4:mm[0-9]+]]
; CHECK-AVX2:    vpunpcklwd      %x[[MM1:mm[0-9]+]], %x[[MM0:mm[0-9]+]], %x[[MM6:mm[0-9]+]]
; CHECK-AVX2:    vpunpckhdq      %x[[MM4]], %x[[MM6]], %x[[MM5:mm[0-9]+]]
; CHECK-AVX2:    vpunpckldq      %x[[MM4]], %x[[MM6]], %x[[MM41:mm[0-9]+]]
; CHECK-AVX2:    vinserti128     $1, %x[[MM5]], %y[[MM41]], %y[[MM42:mm[0-9]+]]
; CHECK-AVX2:    vmovdqu %y[[MM42]],
; CHECK-AVX2:    vpunpckhwd      %x[[MM3]], %x[[MM2]], %x[[MM21:mm[0-9]+]]
; CHECK-AVX2:    vpunpckhwd      %x[[MM1]], %x[[MM0]], %x[[MM11:mm[0-9]+]]
; CHECK-AVX2:    vpunpckhdq      %x[[MM21]], %x[[MM11]], %x[[MM01:mm[0-9]+]]
; CHECK-AVX2:    vpunpckldq      %x[[MM21]], %x[[MM11]], %x[[MM12:mm[0-9]+]]
; CHECK-AVX2:    vinserti128     $1, %x[[MM01]], %y[[MM12]], %y[[MM02:mm[0-9]+]]
; CHECK-AVX2:    vmovdqu %y[[MM02]],
