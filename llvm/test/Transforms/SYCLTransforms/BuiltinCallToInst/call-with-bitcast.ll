; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i32:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

; Function Attrs: nounwind
define void @foo_float(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 {
entry:
  %0 = load float, ptr %a, align 4, !tbaa !7
  %1 = load float, ptr %b, align 4, !tbaa !7
  %call = tail call i32 @_Z7isequalff(float %0, float %1) #2
  store i32 %call, ptr %c, align 4, !tbaa !10
  ret void
;CHECK: define void @foo_float
;CHECK-NOT: call i32 @_Z7isequaldd
;CHECK: [[cmp:%[a-zA-Z0-9]+]] = fcmp oeq float %0, %1
;CHECK-NEXT: [[zext:%[a-zA-Z0-9]+]] = zext i1 [[cmp]] to i32
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare i8 @_Z7isequalff(float, float) #1

; Function Attrs: nounwind
define void @foo_float2(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 {
entry:
  %0 = load <2 x float>, ptr %a, align 8, !tbaa !8
  %1 = load <2 x float>, ptr %b, align 8, !tbaa !8
  %call = tail call <2 x i32> @_Z7isequalDv2_fS_(<2 x float> %0, <2 x float> %1) #2
  store <2 x i32> %call, ptr %c, align 8, !tbaa !8
  ret void
;CHECK: define void @foo_float2
;CHECK-NOT: call <2 x i32> @_Z7isequalDv2_dS_
;CHECK: [[cmp2:%[a-zA-Z0-9]+]] = fcmp oeq <2 x float> %0, %1
;CHECK-NEXT: [[sext2:%[a-zA-Z0-9]+]] = sext <2 x i1> [[cmp2]] to <2 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <2 x i8> @_Z7isequalDv2_fS_(<2 x float>, <2 x float>) #1

; Function Attrs: nounwind
define void @foo_float3(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 {
entry:
  %loadVec4 = load <4 x float>, ptr %a, align 16
  %extractVec = shufflevector <4 x float> %loadVec4, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %loadVec42 = load <4 x float>, ptr %b, align 16
  %extractVec3 = shufflevector <4 x float> %loadVec42, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %call = tail call <3 x i32> @_Z7isequalDv3_fS_(<3 x float> %extractVec, <3 x float> %extractVec3) #2
  %extractVec4 = shufflevector <3 x i32> %call, <3 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  store <4 x i32> %extractVec4, ptr %c, align 16, !tbaa !8
  ret void
;CHECK: define void @foo_float3
;CHECK-NOT: call <3 x i32> @_Z7isequalDv3_dS_
;CHECK: [[cmp3:%[a-zA-Z0-9]+]] = fcmp oeq <3 x float> %extractVec, %extractVec3
;CHECK-NEXT: [[sext3:%[a-zA-Z0-9]+]] = sext <3 x i1> [[cmp3]] to <3 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <3 x i8> @_Z7isequalDv3_fS_(<3 x float>, <3 x float>) #1

; Function Attrs: nounwind
define void @foo_float4(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 {
entry:
  %0 = load <4 x float>, ptr %a, align 16, !tbaa !8
  %1 = load <4 x float>, ptr %b, align 16, !tbaa !8
  %call = tail call <4 x i32> @_Z7isequalDv4_fS_(<4 x float> %0, <4 x float> %1) #2
  store <4 x i32> %call, ptr %c, align 16, !tbaa !8
  ret void
;CHECK: define void @foo_float4
;CHECK-NOT: call <4 x i32> @_Z7isequalDv4_dS_
;CHECK: [[cmp4:%[a-zA-Z0-9]+]] = fcmp oeq <4 x float> %0, %1
;CHECK-NEXT: [[sext4:%[a-zA-Z0-9]+]] = sext <4 x i1> [[cmp4]] to <4 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z7isequalDv4_fS_(<4 x float>, <4 x float>) #1

; Function Attrs: nounwind
define void @foo_float8(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 {
entry:
  %0 = load <8 x float>, ptr %a, align 32, !tbaa !8
  %1 = load <8 x float>, ptr %b, align 32, !tbaa !8
  %call = tail call <8 x i32> @_Z7isequalDv8_fS_(<8 x float> %0, <8 x float> %1) #2
  store <8 x i32> %call, ptr %c, align 32, !tbaa !8
  ret void
;CHECK: define void @foo_float8
;CHECK-NOT: call <8 x i32> @_Z7isequalDv8_dS_
;CHECK: [[cmp8:%[a-zA-Z0-9]+]] = fcmp oeq <8 x float> %0, %1
;CHECK-NEXT: [[sext8:%[a-zA-Z0-9]+]] = sext <8 x i1> [[cmp2]] to <8 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z7isequalDv8_fS_(<8 x float>, <8 x float>) #1

; Function Attrs: nounwind
define void @foo_float16(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 {
entry:
  %0 = load <16 x float>, ptr %a, align 64, !tbaa !8
  %1 = load <16 x float>, ptr %b, align 64, !tbaa !8
  %call = tail call <16 x i32> @_Z7isequalDv16_fS_(<16 x float> %0, <16 x float> %1) #2
  store <16 x i32> %call, ptr %c, align 64, !tbaa !8
  ret void
;CHECK: define void @foo_float16
;CHECK-NOT: call <16 x i32> @_Z7isequalDv16_dS_
;CHECK: [[cmp16:%[a-zA-Z0-9]+]] = fcmp oeq <16 x float> %0, %1
;CHECK-NEXT: [[sext16:%[a-zA-Z0-9]+]] = sext <16 x i1> [[cmp2]] to <16 x i32>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z7isequalDv16_fS_(<16 x float>, <16 x float>) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readnone }

!sycl.kernels = !{!0, !1, !2, !3, !4, !5}

!0 = !{ptr @foo_float}
!1 = !{ptr @foo_float2}
!2 = !{ptr @foo_float3}
!3 = !{ptr @foo_float4}
!4 = !{ptr @foo_float8}
!5 = !{ptr @foo_float16}
!7 = !{!"float", !8}
!8 = !{!"omnipotent char", !9}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"int", !8}

; DEBUGIFY-NOT: WARNING
