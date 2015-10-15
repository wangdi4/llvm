; RUN: opt -builtin-call-to-inst -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;;;; OpenCL source code
;;void __kernel foo_float(float* a, float*b, long* c) {
;;  *c = islessequal(*a, *b);
;;}
;;
;;void __kernel foo_float2(float2* a, float2* b, long2* c) {
;;  *c = islessequal(*a, *b);
;;}
;;
;;void __kernel foo_float3(float3* a, float3* b, long3* c) {
;;  *c = islessequal(*a, *b);
;;}
;;
;;void __kernel foo_float4(float4* a, float4* b, long4* c) {
;;  *c = islessequal(*a, *b);
;;}
;;
;;void __kernel foo_float8(float8* a, float8* b, long8* c) {
;;  *c = islessequal(*a, *b);
;;}
;;
;;void __kernel foo_float16(float16* a, float16* b, long16* c) {
;;  *c = islessequal(*a, *b);
;;}

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i32:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

; Function Attrs: nounwind
define void @foo_float(float* nocapture %a, float* nocapture %b, i32* nocapture %c) #0 {
entry:
  %0 = load float* %a, align 4, !tbaa !7
  %1 = load float* %b, align 4, !tbaa !7
  %call = tail call i32 @_Z11islessequalff(float %0, float %1) #2
  store i32 %call, i32* %c, align 4, !tbaa !10
  ret void
;CHECK: define void @foo_float
;CHECK-NOT: call i32 @_Z11islessequaldd
;CHECK: [[cmp:%[a-zA-Z0-9]+]] = fcmp ole float %0, %1
;CHECK-NEXT: [[zext:%[a-zA-Z0-9]+]] = zext i1 [[cmp]] to i32
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare i32 @_Z11islessequalff(float, float) #1

; Function Attrs: nounwind
define void @foo_float2(<2 x float>* nocapture %a, <2 x float>* nocapture %b, <2 x i32>* nocapture %c) #0 {
entry:
  %0 = load <2 x float>* %a, align 8, !tbaa !8
  %1 = load <2 x float>* %b, align 8, !tbaa !8
  %call = tail call <2 x i32> @_Z11islessequalDv2_fS_(<2 x float> %0, <2 x float> %1) #2
  store <2 x i32> %call, <2 x i32>* %c, align 8, !tbaa !8
  ret void
;CHECK: define void @foo_float2
;CHECK-NOT: call <2 x i32> @_Z11islessequalDv2_dS_
;CHECK: [[cmp2:%[a-zA-Z0-9]+]] = fcmp ole <2 x float> %0, %1
;CHECK-NEXT: [[sext2:%[a-zA-Z0-9]+]] = sext <2 x i1> [[cmp2]] to <2 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <2 x i32> @_Z11islessequalDv2_fS_(<2 x float>, <2 x float>) #1

; Function Attrs: nounwind
define void @foo_float3(<3 x float>* nocapture %a, <3 x float>* nocapture %b, <3 x i32>* nocapture %c) #0 {
entry:
  %castToVec4 = bitcast <3 x float>* %a to <4 x float>*
  %loadVec4 = load <4 x float>* %castToVec4, align 16
  %extractVec = shufflevector <4 x float> %loadVec4, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %castToVec41 = bitcast <3 x float>* %b to <4 x float>*
  %loadVec42 = load <4 x float>* %castToVec41, align 16
  %extractVec3 = shufflevector <4 x float> %loadVec42, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %call = tail call <3 x i32> @_Z11islessequalDv3_fS_(<3 x float> %extractVec, <3 x float> %extractVec3) #2
  %extractVec4 = shufflevector <3 x i32> %call, <3 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %storetmp = bitcast <3 x i32>* %c to <4 x i32>*
  store <4 x i32> %extractVec4, <4 x i32>* %storetmp, align 16, !tbaa !8
  ret void
;CHECK: define void @foo_float3
;CHECK-NOT: call <3 x i32> @_Z11islessequalDv3_dS_
;CHECK: [[cmp3:%[a-zA-Z0-9]+]] = fcmp ole <3 x float> %extractVec, %extractVec3
;CHECK-NEXT: [[sext3:%[a-zA-Z0-9]+]] = sext <3 x i1> [[cmp3]] to <3 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <3 x i32> @_Z11islessequalDv3_fS_(<3 x float>, <3 x float>) #1

; Function Attrs: nounwind
define void @foo_float4(<4 x float>* nocapture %a, <4 x float>* nocapture %b, <4 x i32>* nocapture %c) #0 {
entry:
  %0 = load <4 x float>* %a, align 16, !tbaa !8
  %1 = load <4 x float>* %b, align 16, !tbaa !8
  %call = tail call <4 x i32> @_Z11islessequalDv4_fS_(<4 x float> %0, <4 x float> %1) #2
  store <4 x i32> %call, <4 x i32>* %c, align 16, !tbaa !8
  ret void
;CHECK: define void @foo_float4
;CHECK-NOT: call <4 x i32> @_Z11islessequalDv4_dS_
;CHECK: [[cmp4:%[a-zA-Z0-9]+]] = fcmp ole <4 x float> %0, %1
;CHECK-NEXT: [[sext4:%[a-zA-Z0-9]+]] = sext <4 x i1> [[cmp4]] to <4 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z11islessequalDv4_fS_(<4 x float>, <4 x float>) #1

; Function Attrs: nounwind
define void @foo_float8(<8 x float>* nocapture %a, <8 x float>* nocapture %b, <8 x i32>* nocapture %c) #0 {
entry:
  %0 = load <8 x float>* %a, align 32, !tbaa !8
  %1 = load <8 x float>* %b, align 32, !tbaa !8
  %call = tail call <8 x i32> @_Z11islessequalDv8_fS_(<8 x float> %0, <8 x float> %1) #2
  store <8 x i32> %call, <8 x i32>* %c, align 32, !tbaa !8
  ret void
;CHECK: define void @foo_float8
;CHECK-NOT: call <8 x i32> @_Z11islessequalDv8_dS_
;CHECK: [[cmp8:%[a-zA-Z0-9]+]] = fcmp ole <8 x float> %0, %1
;CHECK-NEXT: [[sext8:%[a-zA-Z0-9]+]] = sext <8 x i1> [[cmp2]] to <8 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z11islessequalDv8_fS_(<8 x float>, <8 x float>) #1

; Function Attrs: nounwind
define void @foo_float16(<16 x float>* nocapture %a, <16 x float>* nocapture %b, <16 x i32>* nocapture %c) #0 {
entry:
  %0 = load <16 x float>* %a, align 64, !tbaa !8
  %1 = load <16 x float>* %b, align 64, !tbaa !8
  %call = tail call <16 x i32> @_Z11islessequalDv16_fS_(<16 x float> %0, <16 x float> %1) #2
  store <16 x i32> %call, <16 x i32>* %c, align 64, !tbaa !8
  ret void
;CHECK: define void @foo_float16
;CHECK-NOT: call <16 x i32> @_Z11islessequalDv16_dS_
;CHECK: [[cmp16:%[a-zA-Z0-9]+]] = fcmp ole <16 x float> %0, %1
;CHECK-NEXT: [[sext16:%[a-zA-Z0-9]+]] = sext <16 x i1> [[cmp2]] to <16 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z11islessequalDv16_fS_(<16 x float>, <16 x float>) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readnone }

!opencl.kernels = !{!0, !1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{void (float*, float*, i32*)* @foo_float}
!1 = !{void (<2 x float>*, <2 x float>*, <2 x i32>*)* @foo_float2}
!2 = !{void (<3 x float>*, <3 x float>*, <3 x i32>*)* @foo_float3}
!3 = !{void (<4 x float>*, <4 x float>*, <4 x i32>*)* @foo_float4}
!4 = !{void (<8 x float>*, <8 x float>*, <8 x i32>*)* @foo_float8}
!5 = !{void (<16 x float>*, <16 x float>*, <16 x i32>*)* @foo_float16}
!6 = !{!"-cl-std=CL1.2"}
!7 = !{!"float", !8}
!8 = !{!"omnipotent char", !9}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"int", !8}
