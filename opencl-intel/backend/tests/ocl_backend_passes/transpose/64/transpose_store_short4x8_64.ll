;; [LLVM 3.6 UPGRADE] The following LLVM IR is translated into similar but different ASM sequence.
;; Don't see any point to test codegen here. Consider to remove the test or move it to LLVM if it is necessary.
; XFAIL: *

; XFAIL: i686

; RUN: oclopt -runtimelib=clbltfne9.rtl -builtin-import -builtin-call-to-inst -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc %t1.ll -mattr=+avx -mtriple=x86_64 -o %t2.asm
; RUN: FileCheck %s --input-file=%t2.asm -check-prefix=CHECK-AVX

; RUN: oclopt -runtimelib=clbltfnl9.rtl -builtin-import -builtin-call-to-inst -instcombine -inline -scalarrepl -S %s -o %t3.ll
; RUN: llc %t3.ll -mattr=+avx2 -mtriple=x86_64 -o %t4.asm
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
; CHECK-AVX:     vpunpcklwd %x[[MM3:mm[0-7]{1}]], %x[[MM2:mm[0-7]{1}]], %x[[MM4:mm[0-7]{1}]]
; CHECK-AVX:     vpunpcklwd %x[[MM1:mm[0-7]{1}]], %x[[MM0:mm[0-7]{1}]], %x[[MM5:mm[0-7]{1}]]
; CHECK-AVX:     vunpckhps %x[[MM4]], %x[[MM5]], %x[[MM6:mm[0-7]{1}]]
; CHECK-AVX:     vmovups %x[[MM6]],
; CHECK-AVX:     vunpcklps %x[[MM4]], %x[[MM5]], %x[[MM4]]
; CHECK-AVX:     vmovups %x[[MM4]],
; CHECK-AVX:     vpunpckhwd %x[[MM3]], %x[[MM2]], %x[[MM2]]
; CHECK-AVX:     vpunpckhwd %x[[MM1]], %x[[MM0]], %x[[MM0]]
; CHECK-AVX:     vunpckhps %x[[MM2]], %x[[MM0]], %x[[MM1]]
; CHECK-AVX:     vmovups %x[[MM1]],
; CHECK-AVX:     vunpcklps %x[[MM2]], %x[[MM0]], %x[[MM0]]
; CHECK-AVX:     vmovups %x[[MM0]],


;-------------------------------------------------------------------------------

; CHECK-AVX2:    vpunpcklwd %x[[MM3:mm[0-7]{1}]], %x[[MM2:mm[0-7]{1}]], %x[[MM4:mm[0-7]{1}]]
; CHECK-AVX2:    vpunpcklwd %x[[MM1:mm[0-7]{1}]], %x[[MM0:mm[0-7]{1}]], %x[[MM6:mm[0-7]{1}]]
; CHECK-AVX2:    vpunpckhdq %x[[MM4]], %x[[MM6]], %x[[MM5:mm[0-7]{1}]]
; CHECK-AVX2:    vpunpckldq %x[[MM4]], %x[[MM6]], %x[[MM4]]
; CHECK-AVX2:    vinserti128 $1, %x[[MM5]], %y[[MM4]], %y[[MM4]]
; CHECK-AVX2:    vmovdqu %y[[MM4]],
; CHECK-AVX2:    vpunpckhwd %x[[MM3]], %x[[MM2]], %x[[MM2]]
; CHECK-AVX2:    vpunpckhwd %x[[MM1]], %x[[MM0]], %x[[MM1]]
; CHECK-AVX2:    vpunpckhdq %x[[MM2]], %x[[MM1]], %x[[MM0]]
; CHECK-AVX2:    vpunpckldq %x[[MM2]], %x[[MM1]], %x[[MM1]]
; CHECK-AVX2:    vinserti128 $1, %x[[MM0]], %y[[MM1]], %y[[MM0]]
; CHECK-AVX2:    vmovdqu %y[[MM0]],

