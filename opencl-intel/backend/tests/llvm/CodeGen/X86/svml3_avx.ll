; RUN: llc < %s -march=x86-64 -mcpu=sandybridge | FileCheck %s

declare <4 x i64> @__ocl_svml_f4(<4 x float>)
declare <8 x i64> @__ocl_svml_f8(<8 x float>)

declare <4 x float> @__ocl_svml_l4(<4 x i64>)
declare <8 x float> @__ocl_svml_l8(<8 x i64>)

declare <8 x float> @__ocl_svml_f8_i8(<8 x float>, <8 x i32>)
declare <16 x float> @__ocl_svml_f16_i16(<16 x float>, <16 x i32>)

;( result((xmm0 xmm1)) parameters((xmm0))  )
define <4 x i64> @testf4(<4 x float> %x, <4 x float> %y) nounwind {
entry:
;CHECK: testf4
;CHECK: vaddps {{[^,]*}}, %xmm0
;CHECK-NOT: vinsertf128     
  %x1 = fadd  <4 x float>  %x, %y
  %call = tail call x86_svmlcc <4 x i64> @__ocl_svml_f4(<4 x float> %x1) nounwind
  ret <4 x i64> %call
}

;( result((ymm0 ymm1)) parameters((ymm0))  )
define <8 x i64> @testf8(<8 x float> %x, <8 x float> %y) nounwind {
entry:
;CHECK: testf8
;CHECK: vaddps  {{.*}}, %ymm0
;CHECK: callq 
;CHECK: callq 
;CHECK: vxorps  %ymm0
;CHECK: vxorps  %ymm1
  %x1 = fadd  <8 x float>  %x, %y
  %call1 = tail call x86_svmlcc <8 x i64> @__ocl_svml_f8(<8 x float> %x1) nounwind
  %call2 = tail call x86_svmlcc <8 x i64> @__ocl_svml_f8(<8 x float> %y) nounwind
  %x2 = xor <8 x i64>  %call1, %call2
  ret <8 x i64> %x2
}


;( result((xmm0)) parameters((ymm0))  )
define <4 x float> @testl4(<4 x i64> %x, <4 x i64> %y) nounwind {
entry:
;CHECK: testl4
;CHECK-NOT: vextractf128    
;CHECK: vxorps  {{.*}}, %ymm0     
;CHECK: call
  %x1 = xor  <4 x i64>  %x, %y
  %call = tail call x86_svmlcc <4 x float> @__ocl_svml_l4(<4 x i64> %x1) nounwind
  ret <4 x float> %call
}

;( result((ymm0)) parameters((ymm0 ymm1))  )
define <8 x float> @testl8(<8 x i64> %x, <8 x i64> %y) nounwind {
entry:
;CHECK: testl8
;CHECK-NOT: vextractf128    
;CHECK: vxorps  {{.*}}, %ymm0     
;CHECK: vxorps  {{.*}}, %ymm1
;CHECK: call
  %x1 = xor  <8 x i64>  %x, %y
  %call = tail call x86_svmlcc <8 x float> @__ocl_svml_l8(<8 x i64> %x1) nounwind
  ret <8 x float> %call
}

;( result((ymm0)) parameters((ymm0),(ymm1))  )
define <8 x float> @test_f8_i8(<8 x float> %x, <8 x i32> %y, <8 x i32> %z) nounwind {
entry:
;CHECK: test_f8_i8
;CHECK: vextractf128   
;CHECK: vinsertf128 {{.*}}, %ymm1
;CHECK: call
  %zy = add <8 x i32>%z, %y
  %call = tail call x86_svmlcc <8 x float> @__ocl_svml_f8_i8(<8 x float> %x, <8 x i32> %zy) nounwind
  ret <8 x float> %call
}

