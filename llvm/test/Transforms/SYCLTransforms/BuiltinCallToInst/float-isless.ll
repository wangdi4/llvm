; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s | FileCheck %s

;;;; OpenCL source code
;;void __kernel foo_float(float* a, float*b, long* c) {
;;  *c = isless(*a, *b);
;;}
;;
;;void __kernel foo_float2(float2* a, float2* b, long2* c) {
;;  *c = isless(*a, *b);
;;}
;;
;;void __kernel foo_float3(float3* a, float3* b, long3* c) {
;;  *c = isless(*a, *b);
;;}
;;
;;void __kernel foo_float4(float4* a, float4* b, long4* c) {
;;  *c = isless(*a, *b);
;;}
;;
;;void __kernel foo_float8(float8* a, float8* b, long8* c) {
;;  *c = isless(*a, *b);
;;}
;;
;;void __kernel foo_float16(float16* a, float16* b, long16* c) {
;;  *c = isless(*a, *b);
;;}

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i32:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

; Function Attrs: nounwind
define void @foo_float(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !11 !arg_type_null_val !12 {
entry:
  %0 = load float, ptr %a, align 4, !tbaa !7
  %1 = load float, ptr %b, align 4, !tbaa !7
  %call = tail call i32 @_Z6islessff(float %0, float %1) #2
  store i32 %call, ptr %c, align 4, !tbaa !10
  ret void
;CHECK: define void @foo_float
;CHECK-NOT: call i32 @_Z6islessdd
;CHECK: [[cmp:%[a-zA-Z0-9]+]] = fcmp olt float %0, %1
;CHECK-NEXT: [[zext:%[a-zA-Z0-9]+]] = zext i1 [[cmp]] to i32
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare i32 @_Z6islessff(float, float) #1

; Function Attrs: nounwind
define void @foo_float2(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !13 !arg_type_null_val !14 {
entry:
  %0 = load <2 x float>, ptr %a, align 8, !tbaa !8
  %1 = load <2 x float>, ptr %b, align 8, !tbaa !8
  %call = tail call <2 x i32> @_Z6islessDv2_fS_(<2 x float> %0, <2 x float> %1) #2
  store <2 x i32> %call, ptr %c, align 8, !tbaa !8
  ret void
;CHECK: define void @foo_float2
;CHECK-NOT: call <2 x i32> @_Z6islessDv2_dS_
;CHECK: [[cmp2:%[a-zA-Z0-9]+]] = fcmp olt <2 x float> %0, %1
;CHECK-NEXT: [[sext2:%[a-zA-Z0-9]+]] = sext <2 x i1> [[cmp2]] to <2 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <2 x i32> @_Z6islessDv2_fS_(<2 x float>, <2 x float>) #1

; Function Attrs: nounwind
define void @foo_float3(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !15 !arg_type_null_val !16 {
entry:
  %loadVec4 = load <4 x float>, ptr %a, align 16
  %extractVec = shufflevector <4 x float> %loadVec4, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %loadVec42 = load <4 x float>, ptr %b, align 16
  %extractVec3 = shufflevector <4 x float> %loadVec42, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %call = tail call <3 x i32> @_Z6islessDv3_fS_(<3 x float> %extractVec, <3 x float> %extractVec3) #2
  %extractVec4 = shufflevector <3 x i32> %call, <3 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  store <4 x i32> %extractVec4, ptr %c, align 16, !tbaa !8
  ret void
;CHECK: define void @foo_float3
;CHECK-NOT: call <3 x i32> @_Z6islessDv3_dS_
;CHECK: [[cmp3:%[a-zA-Z0-9]+]] = fcmp olt <3 x float> %extractVec, %extractVec3
;CHECK-NEXT: [[sext3:%[a-zA-Z0-9]+]] = sext <3 x i1> [[cmp3]] to <3 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <3 x i32> @_Z6islessDv3_fS_(<3 x float>, <3 x float>) #1

; Function Attrs: nounwind
define void @foo_float4(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !17 !arg_type_null_val !18 {
entry:
  %0 = load <4 x float>, ptr %a, align 16, !tbaa !8
  %1 = load <4 x float>, ptr %b, align 16, !tbaa !8
  %call = tail call <4 x i32> @_Z6islessDv4_fS_(<4 x float> %0, <4 x float> %1) #2
  store <4 x i32> %call, ptr %c, align 16, !tbaa !8
  ret void
;CHECK: define void @foo_float4
;CHECK-NOT: call <4 x i32> @_Z6islessDv4_dS_
;CHECK: [[cmp4:%[a-zA-Z0-9]+]] = fcmp olt <4 x float> %0, %1
;CHECK-NEXT: [[sext4:%[a-zA-Z0-9]+]] = sext <4 x i1> [[cmp4]] to <4 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z6islessDv4_fS_(<4 x float>, <4 x float>) #1

; Function Attrs: nounwind
define void @foo_float8(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !19 !arg_type_null_val !20 {
entry:
  %0 = load <8 x float>, ptr %a, align 32, !tbaa !8
  %1 = load <8 x float>, ptr %b, align 32, !tbaa !8
  %call = tail call <8 x i32> @_Z6islessDv8_fS_(<8 x float> %0, <8 x float> %1) #2
  store <8 x i32> %call, ptr %c, align 32, !tbaa !8
  ret void
;CHECK: define void @foo_float8
;CHECK-NOT: call <8 x i32> @_Z6islessDv8_dS_
;CHECK: [[cmp8:%[a-zA-Z0-9]+]] = fcmp olt <8 x float> %0, %1
;CHECK-NEXT: [[sext8:%[a-zA-Z0-9]+]] = sext <8 x i1> [[cmp2]] to <8 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z6islessDv8_fS_(<8 x float>, <8 x float>) #1

; Function Attrs: nounwind
define void @foo_float16(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !21 !arg_type_null_val !22 {
entry:
  %0 = load <16 x float>, ptr %a, align 64, !tbaa !8
  %1 = load <16 x float>, ptr %b, align 64, !tbaa !8
  %call = tail call <16 x i32> @_Z6islessDv16_fS_(<16 x float> %0, <16 x float> %1) #2
  store <16 x i32> %call, ptr %c, align 64, !tbaa !8
  ret void
;CHECK: define void @foo_float16
;CHECK-NOT: call <16 x i32> @_Z6islessDv16_dS_
;CHECK: [[cmp16:%[a-zA-Z0-9]+]] = fcmp olt <16 x float> %0, %1
;CHECK-NEXT: [[sext16:%[a-zA-Z0-9]+]] = sext <16 x i1> [[cmp2]] to <16 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z6islessDv16_fS_(<16 x float>, <16 x float>) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readnone }

!opencl.kernels = !{!0, !1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{ptr @foo_float}
!1 = !{ptr @foo_float2}
!2 = !{ptr @foo_float3}
!3 = !{ptr @foo_float4}
!4 = !{ptr @foo_float8}
!5 = !{ptr @foo_float16}
!6 = !{!"-cl-std=CL1.2"}
!7 = !{!"float", !8}
!8 = !{!"omnipotent char", !9}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"int", !8}
!11 = !{!"float*", !"float*", !"long*"}
!12 = !{ptr null, ptr null, ptr null}
!13 = !{!"float2*", !"float2*", !"long2*"}
!14 = !{ptr null, ptr null, ptr null}
!15 = !{!"float3*", !"float3*", !"long3*"}
!16 = !{ptr null, ptr null, ptr null}
!17 = !{!"float4*", !"float4*", !"long4*"}
!18 = !{ptr null, ptr null, ptr null}
!19 = !{!"float8*", !"float8*", !"long8*"}
!20 = !{ptr null, ptr null, ptr null}
!21 = !{!"float16*", !"float16*", !"long16*"}
!22 = !{ptr null, ptr null, ptr null}

; DEBUGIFY-NOT: WARNING
