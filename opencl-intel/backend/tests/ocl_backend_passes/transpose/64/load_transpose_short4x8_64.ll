; XFAIL: i686

; RUN: oclopt -runtimelib=clbltfne9.rtl -builtin-import -shuffle-call-to-inst -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc %t1.ll -mattr=+avx -mtriple=x86_64 -o %t2.asm
; RUN: FileCheck %s --input-file=%t2.asm -check-prefix=CHECK-AVX

; RUN: oclopt -runtimelib=clbltfnl9.rtl -builtin-import -shuffle-call-to-inst -instcombine -inline -scalarrepl -S %s -o %t3.ll
; RUN: llc %t3.ll -mattr=+avx2 -mtriple=x86_64 -o %t4.asm
; RUN: FileCheck %s --input-file=%t4.asm -check-prefix=CHECK-AVX2


define <8 x i16> @foo(<4 x i16>* nocapture %pLoadAdd){
entry:
  %xOut = alloca  <8 x i16>
  %yOut = alloca  <8 x i16>
  %zOut = alloca  <8 x i16>
  %wOut = alloca  <8 x i16>
  call void @__ocl_load_transpose_short_4x8(<4 x i16>* nocapture %pLoadAdd, <8 x i16>* nocapture %xOut, <8 x i16>* nocapture %yOut, <8 x i16>* nocapture %zOut, <8 x i16>* nocapture %wOut) nounwind
  %temp1 = load <8 x i16>* %xOut
  %temp2 = load <8 x i16>* %yOut
  %temp3 = load <8 x i16>* %zOut
  %temp4 = load <8 x i16>* %wOut
  %re0 = add <8 x i16> %temp1, %temp2
  %re1 = add <8 x i16> %temp3, %temp4
  %ret0 = add <8 x i16> %re0, %re1
  ret <8 x i16> %ret0
}

declare void @__ocl_load_transpose_short_4x8(<4 x i16>* nocapture %pLoadAdd, <8 x i16>* nocapture %xOut, <8 x i16>* nocapture %yOut, <8 x i16>* nocapture %zOut, <8 x i16>* nocapture %wOut) nounwind alwaysinline


;-------------------------------------------------------------------------------
; CHECK-AVX:     .type [[FOO:[_a-z]+]],@function
; CHECK-AVX:     vpunpckhwd %x[[MM3:mm[0-7]{1}]], %x[[MM2:mm[0-7]{1}]], %x[[MM0:mm[0-7]{1}]]
; CHECK-AVX:     vpunpcklwd %x[[MM3]], %x[[MM2]], %x[[MM21:mm[0-7]{1}]]
; CHECK-AVX:     vpunpckhwd %x[[MM0]], %x[[MM21]], %x[[MM5:mm[0-7]{1}]]
; CHECK-AVX:     vpunpckhwd %x[[MM4:mm[0-7]{1}]], %x[[MM1:mm[0-7]{1}]], %x[[MM31:mm[0-7]{1}]]
; CHECK-AVX:     vpunpcklwd %x[[MM4]], %x[[MM1]], %x[[MM11:mm[0-7]{1}]]
; CHECK-AVX:     vpunpckhwd %x[[MM31]], %x[[MM11]], %x[[MM41:mm[0-7]{1}]]
; CHECK-AVX:     vunpckhps  %x[[MM5]], %x[[MM41]], %x[[MM6:mm[0-7]{1}]]
; CHECK-AVX:     vunpcklps  %x[[MM5]], %x[[MM41]], %x[[MM42:mm[0-7]{1}]]
; CHECK-AVX:     vpaddw     %x[[MM6:mm[0-7]{1}]], %x[[MM42]], %x[[MM43:mm[0-7]{1}]]
; CHECK-AVX:     vpunpcklwd %x[[MM0]], %x[[MM21]], %x[[MM01:mm[0-7]{1}]]
; CHECK-AVX:     vpunpcklwd %x[[MM31]], %x[[MM11]], %x[[MM12:mm[0-7]{1}]]
; CHECK-AVX:     vunpckhps  %x[[MM01]], %x[[MM12]], %x[[MM22:mm[0-7]{1}]]
; CHECK-AVX:     vunpcklps  %x[[MM01]], %x[[MM12]], %x[[MM02:mm[0-7]{1}]]
; CHECK-AVX:     vpaddw     %x[[MM22]], %x[[MM02]], %x[[MM03:mm[0-7]{1}]]
; CHECK-AVX:     vpaddw     %x[[MM43]], %x[[MM03]], %x[[MM04:mm[0-7]{1}]]
; CHECK-AVX:     .size [[FOO]]


;-------------------------------------------------------------------------------
; CHECK-AVX2:    .type [[FOO:[_a-z]+]],@function
; CHECK-AVX2:    vmovdqa .[[LABEL:[_a-zA-Z0-9]+]]({{%[a-z0-9]+}}), %y[[MM1:mm[0-7]{1}]]
; CHECK-AVX2:    vpunpckhwd %y[[MM3:mm[0-7]{1}]], %y[[MM0:mm[0-7]{1}]], %y[[MM2:mm[0-7]{1}]]
; CHECK-AVX2:    vpunpcklwd %y[[MM3]], %y[[MM0]], %y[[MM3]]
; CHECK-AVX2:    vpunpcklwd %y[[MM2]], %y[[MM3]], %y[[MM0]]
; CHECK-AVX2:    vpermd %y[[MM0]], %y[[MM1]], %y[[MM0]]
; CHECK-AVX2:    vpunpckhwd %y[[MM2]], %y[[MM3]], %y[[MM2]]
; CHECK-AVX2:    vpermd %y[[MM2]], %y[[MM1]], %y[[MM1]]
; CHECK-AVX2:    vextracti128 $1, %y[[MM1]], %x[[MM2]]
; CHECK-AVX2:    vpaddw %x[[MM2]], %x[[MM1]], %x[[MM1]]
; CHECK-AVX2:    vextracti128 $1, %y[[MM0]], %x[[MM2]]
; CHECK-AVX2:    vpaddw %x[[MM2]], %x[[MM0]], %x[[MM0]]
; CHECK-AVX2:    vpaddw %x[[MM1]], %x[[MM0]], %x[[MM0]]
; CHECK-AVX2:    .size [[FOO]]
