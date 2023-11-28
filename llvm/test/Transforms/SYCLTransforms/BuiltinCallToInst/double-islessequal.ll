; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s | FileCheck %s

;;;; OpenCL source code
;;void __kernel foo_double(double* a, double*b, long* c) {
;;  *c = islessequal(*a, *b);
;;}
;;
;;void __kernel foo_double2(double2* a, double2* b, long2* c) {
;;  *c = islessequal(*a, *b);
;;}
;;
;;void __kernel foo_double3(double3* a, double3* b, long3* c) {
;;  *c = islessequal(*a, *b);
;;}
;;
;;void __kernel foo_double4(double4* a, double4* b, long4* c) {
;;  *c = islessequal(*a, *b);
;;}
;;
;;void __kernel foo_double8(double8* a, double8* b, long8* c) {
;;  *c = islessequal(*a, *b);
;;}
;;
;;void __kernel foo_double16(double16* a, double16* b, long16* c) {
;;  *c = islessequal(*a, *b);
;;}

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

; Function Attrs: nounwind
define void @foo_double(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !11 !arg_type_null_val !12 {
entry:
  %0 = load double, ptr %a, align 8, !tbaa !7
  %1 = load double, ptr %b, align 8, !tbaa !7
  %call = tail call i32 @_Z11islessequaldd(double %0, double %1) #2
  %conv = sext i32 %call to i64
  store i64 %conv, ptr %c, align 8, !tbaa !10
  ret void
;CHECK: define void @foo_double
;CHECK-NOT: call i32 @_Z11islessequaldd
;CHECK: [[cmp:%[a-zA-Z0-9]+]] = fcmp ole double %0, %1
;CHECK-NEXT: [[zext:%[a-zA-Z0-9]+]] = zext i1 [[cmp]] to i32
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare i32 @_Z11islessequaldd(double, double) #1

; Function Attrs: nounwind
define void @foo_double2(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !13 !arg_type_null_val !14 {
entry:
  %0 = load <2 x double>, ptr %a, align 16, !tbaa !8
  %1 = load <2 x double>, ptr %b, align 16, !tbaa !8
  %call = tail call <2 x i64> @_Z11islessequalDv2_dS_(<2 x double> %0, <2 x double> %1) #2
  store <2 x i64> %call, ptr %c, align 16, !tbaa !8
  ret void
;CHECK: define void @foo_double2
;CHECK-NOT: call <2 x i64> @_Z11islessequalDv2_dS_
;CHECK: [[cmp2:%[a-zA-Z0-9]+]] = fcmp ole <2 x double> %0, %1
;CHECK-NEXT: [[sext2:%[a-zA-Z0-9]+]] = sext <2 x i1> [[cmp2]] to <2 x i64>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <2 x i64> @_Z11islessequalDv2_dS_(<2 x double>, <2 x double>) #1

; Function Attrs: nounwind
define void @foo_double3(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !15 !arg_type_null_val !16 {
entry:
  %loadVec4 = load <4 x double>, ptr %a, align 32
  %extractVec = shufflevector <4 x double> %loadVec4, <4 x double> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %loadVec42 = load <4 x double>, ptr %b, align 32
  %extractVec3 = shufflevector <4 x double> %loadVec42, <4 x double> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %call = tail call <3 x i64> @_Z11islessequalDv3_dS_(<3 x double> %extractVec, <3 x double> %extractVec3) #2
  %extractVec4 = shufflevector <3 x i64> %call, <3 x i64> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  store <4 x i64> %extractVec4, ptr %c, align 32, !tbaa !8
  ret void
;CHECK: define void @foo_double3
;CHECK-NOT: call <3 x i64> @_Z11islessequalDv3_dS_
;CHECK: [[cmp3:%[a-zA-Z0-9]+]] = fcmp ole <3 x double> %extractVec, %extractVec3
;CHECK-NEXT: [[sext3:%[a-zA-Z0-9]+]] = sext <3 x i1> [[cmp3]] to <3 x i64>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <3 x i64> @_Z11islessequalDv3_dS_(<3 x double>, <3 x double>) #1

; Function Attrs: nounwind
define void @foo_double4(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !17 !arg_type_null_val !18 {
entry:
  %0 = load <4 x double>, ptr %a, align 32, !tbaa !8
  %1 = load <4 x double>, ptr %b, align 32, !tbaa !8
  %call = tail call <4 x i64> @_Z11islessequalDv4_dS_(<4 x double> %0, <4 x double> %1) #2
  store <4 x i64> %call, ptr %c, align 32, !tbaa !8
  ret void
;CHECK: define void @foo_double4
;CHECK-NOT: call <4 x i64> @_Z11islessequalDv4_dS_
;CHECK: [[cmp4:%[a-zA-Z0-9]+]] = fcmp ole <4 x double> %0, %1
;CHECK-NEXT: [[sext4:%[a-zA-Z0-9]+]] = sext <4 x i1> [[cmp4]] to <4 x i64>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z11islessequalDv4_dS_(<4 x double>, <4 x double>) #1

; Function Attrs: nounwind
define void @foo_double8(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !19 !arg_type_null_val !20 {
entry:
  %0 = load <8 x double>, ptr %a, align 64, !tbaa !8
  %1 = load <8 x double>, ptr %b, align 64, !tbaa !8
  %call = tail call <8 x i64> @_Z11islessequalDv8_dS_(<8 x double> %0, <8 x double> %1) #2
  store <8 x i64> %call, ptr %c, align 64, !tbaa !8
  ret void
;CHECK: define void @foo_double8
;CHECK-NOT: call <8 x i64> @_Z11islessequalDv8_dS_
;CHECK: [[cmp8:%[a-zA-Z0-9]+]] = fcmp ole <8 x double> %0, %1
;CHECK-NEXT: [[sext8:%[a-zA-Z0-9]+]] = sext <8 x i1> [[cmp2]] to <8 x i64>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z11islessequalDv8_dS_(<8 x double>, <8 x double>) #1

; Function Attrs: nounwind
define void @foo_double16(ptr nocapture %a, ptr nocapture %b, ptr nocapture %c) #0 !kernel_arg_base_type !21 !arg_type_null_val !22 {
entry:
  %0 = load <16 x double>, ptr %a, align 128, !tbaa !8
  %1 = load <16 x double>, ptr %b, align 128, !tbaa !8
  %call = tail call <16 x i64> @_Z11islessequalDv16_dS_(<16 x double> %0, <16 x double> %1) #2
  store <16 x i64> %call, ptr %c, align 128, !tbaa !8
  ret void
;CHECK: define void @foo_double16
;CHECK-NOT: call <16 x i64> @_Z11islessequalDv16_dS_
;CHECK: [[cmp16:%[a-zA-Z0-9]+]] = fcmp ole <16 x double> %0, %1
;CHECK-NEXT: [[sext16:%[a-zA-Z0-9]+]] = sext <16 x i1> [[cmp2]] to <16 x i64>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z11islessequalDv16_dS_(<16 x double>, <16 x double>) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readnone }

!opencl.kernels = !{!0, !1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{ptr @foo_double}
!1 = !{ptr @foo_double2}
!2 = !{ptr @foo_double3}
!3 = !{ptr @foo_double4}
!4 = !{ptr @foo_double8}
!5 = !{ptr @foo_double16}
!6 = !{!"-cl-std=CL1.2"}
!7 = !{!"double", !8}
!8 = !{!"omnipotent char", !9}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"long", !8}
!11 = !{!"double*", !"double*", !"long*"}
!12 = !{ptr null, ptr null, ptr null}
!13 = !{!"double2*", !"double2*", !"long2*"}
!14 = !{ptr null, ptr null, ptr null}
!15 = !{!"double3*", !"double3*", !"long3*"}
!16 = !{ptr null, ptr null, ptr null}
!17 = !{!"double4*", !"double4*", !"long4*"}
!18 = !{ptr null, ptr null, ptr null}
!19 = !{!"double8*", !"double8*", !"long8*"}
!20 = !{ptr null, ptr null, ptr null}
!21 = !{!"double16*", !"double16*", !"long16*"}
!22 = !{ptr null, ptr null, ptr null}

; DEBUGIFY-NOT: WARNING
