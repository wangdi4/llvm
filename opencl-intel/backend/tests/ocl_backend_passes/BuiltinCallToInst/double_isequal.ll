; RUN: opt -builtin-call-to-inst -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;;;; OpenCL source code
;;void __kernel foo_double(double* a, double*b, long* c) {
;;  *c = isequal(*a, *b);
;;}
;;
;;void __kernel foo_double2(double2* a, double2* b, long2* c) {
;;  *c = isequal(*a, *b);
;;}
;;
;;void __kernel foo_double3(double3* a, double3* b, long3* c) {
;;  *c = isequal(*a, *b);
;;}
;;
;;void __kernel foo_double4(double4* a, double4* b, long4* c) {
;;  *c = isequal(*a, *b);
;;}
;;
;;void __kernel foo_double8(double8* a, double8* b, long8* c) {
;;  *c = isequal(*a, *b);
;;}
;;
;;void __kernel foo_double16(double16* a, double16* b, long16* c) {
;;  *c = isequal(*a, *b);
;;}

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

; Function Attrs: nounwind
define void @foo_double(double* nocapture %a, double* nocapture %b, i64* nocapture %c) #0 {
entry:
  %0 = load double* %a, align 8, !tbaa !7
  %1 = load double* %b, align 8, !tbaa !7
  %call = tail call i32 @_Z7isequaldd(double %0, double %1) #2
  %conv = sext i32 %call to i64
  store i64 %conv, i64* %c, align 8, !tbaa !10
  ret void
;CHECK: define void @foo_double
;CHECK-NOT: call i32 @_Z7isequaldd
;CHECK: [[cmp:%[a-zA-Z0-9]+]] = fcmp oeq double %0, %1
;CHECK-NEXT: [[zext:%[a-zA-Z0-9]+]] = zext i1 [[cmp]] to i32
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare i32 @_Z7isequaldd(double, double) #1

; Function Attrs: nounwind
define void @foo_double2(<2 x double>* nocapture %a, <2 x double>* nocapture %b, <2 x i64>* nocapture %c) #0 {
entry:
  %0 = load <2 x double>* %a, align 16, !tbaa !8
  %1 = load <2 x double>* %b, align 16, !tbaa !8
  %call = tail call <2 x i64> @_Z7isequalDv2_dS_(<2 x double> %0, <2 x double> %1) #2
  store <2 x i64> %call, <2 x i64>* %c, align 16, !tbaa !8
  ret void
;CHECK: define void @foo_double2
;CHECK-NOT: call <2 x i64> @_Z7isequalDv2_dS_
;CHECK: [[cmp2:%[a-zA-Z0-9]+]] = fcmp oeq <2 x double> %0, %1
;CHECK-NEXT: [[sext2:%[a-zA-Z0-9]+]] = sext <2 x i1> [[cmp2]] to <2 x i64>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <2 x i64> @_Z7isequalDv2_dS_(<2 x double>, <2 x double>) #1

; Function Attrs: nounwind
define void @foo_double3(<3 x double>* nocapture %a, <3 x double>* nocapture %b, <3 x i64>* nocapture %c) #0 {
entry:
  %castToVec4 = bitcast <3 x double>* %a to <4 x double>*
  %loadVec4 = load <4 x double>* %castToVec4, align 32
  %extractVec = shufflevector <4 x double> %loadVec4, <4 x double> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %castToVec41 = bitcast <3 x double>* %b to <4 x double>*
  %loadVec42 = load <4 x double>* %castToVec41, align 32
  %extractVec3 = shufflevector <4 x double> %loadVec42, <4 x double> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %call = tail call <3 x i64> @_Z7isequalDv3_dS_(<3 x double> %extractVec, <3 x double> %extractVec3) #2
  %extractVec4 = shufflevector <3 x i64> %call, <3 x i64> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %storetmp = bitcast <3 x i64>* %c to <4 x i64>*
  store <4 x i64> %extractVec4, <4 x i64>* %storetmp, align 32, !tbaa !8
  ret void
;CHECK: define void @foo_double3
;CHECK-NOT: call <3 x i64> @_Z7isequalDv3_dS_
;CHECK: [[cmp3:%[a-zA-Z0-9]+]] = fcmp oeq <3 x double> %extractVec, %extractVec3
;CHECK-NEXT: [[sext3:%[a-zA-Z0-9]+]] = sext <3 x i1> [[cmp3]] to <3 x i64>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <3 x i64> @_Z7isequalDv3_dS_(<3 x double>, <3 x double>) #1

; Function Attrs: nounwind
define void @foo_double4(<4 x double>* nocapture %a, <4 x double>* nocapture %b, <4 x i64>* nocapture %c) #0 {
entry:
  %0 = load <4 x double>* %a, align 32, !tbaa !8
  %1 = load <4 x double>* %b, align 32, !tbaa !8
  %call = tail call <4 x i64> @_Z7isequalDv4_dS_(<4 x double> %0, <4 x double> %1) #2
  store <4 x i64> %call, <4 x i64>* %c, align 32, !tbaa !8
  ret void
;CHECK: define void @foo_double4
;CHECK-NOT: call <4 x i64> @_Z7isequalDv4_dS_
;CHECK: [[cmp4:%[a-zA-Z0-9]+]] = fcmp oeq <4 x double> %0, %1
;CHECK-NEXT: [[sext4:%[a-zA-Z0-9]+]] = sext <4 x i1> [[cmp4]] to <4 x i64>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z7isequalDv4_dS_(<4 x double>, <4 x double>) #1

; Function Attrs: nounwind
define void @foo_double8(<8 x double>* nocapture %a, <8 x double>* nocapture %b, <8 x i64>* nocapture %c) #0 {
entry:
  %0 = load <8 x double>* %a, align 64, !tbaa !8
  %1 = load <8 x double>* %b, align 64, !tbaa !8
  %call = tail call <8 x i64> @_Z7isequalDv8_dS_(<8 x double> %0, <8 x double> %1) #2
  store <8 x i64> %call, <8 x i64>* %c, align 64, !tbaa !8
  ret void
;CHECK: define void @foo_double8
;CHECK-NOT: call <8 x i64> @_Z7isequalDv8_dS_
;CHECK: [[cmp8:%[a-zA-Z0-9]+]] = fcmp oeq <8 x double> %0, %1
;CHECK-NEXT: [[sext8:%[a-zA-Z0-9]+]] = sext <8 x i1> [[cmp2]] to <8 x i64>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z7isequalDv8_dS_(<8 x double>, <8 x double>) #1

; Function Attrs: nounwind
define void @foo_double16(<16 x double>* nocapture %a, <16 x double>* nocapture %b, <16 x i64>* nocapture %c) #0 {
entry:
  %0 = load <16 x double>* %a, align 128, !tbaa !8
  %1 = load <16 x double>* %b, align 128, !tbaa !8
  %call = tail call <16 x i64> @_Z7isequalDv16_dS_(<16 x double> %0, <16 x double> %1) #2
  store <16 x i64> %call, <16 x i64>* %c, align 128, !tbaa !8
  ret void
;CHECK: define void @foo_double16
;CHECK-NOT: call <16 x i64> @_Z7isequalDv16_dS_
;CHECK: [[cmp16:%[a-zA-Z0-9]+]] = fcmp oeq <16 x double> %0, %1
;CHECK-NEXT: [[sext16:%[a-zA-Z0-9]+]] = sext <16 x i1> [[cmp2]] to <16 x i64>
;CHECK: ret void
}

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z7isequalDv16_dS_(<16 x double>, <16 x double>) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readnone }

!opencl.kernels = !{!0, !1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{void (double*, double*, i64*)* @foo_double}
!1 = !{void (<2 x double>*, <2 x double>*, <2 x i64>*)* @foo_double2}
!2 = !{void (<3 x double>*, <3 x double>*, <3 x i64>*)* @foo_double3}
!3 = !{void (<4 x double>*, <4 x double>*, <4 x i64>*)* @foo_double4}
!4 = !{void (<8 x double>*, <8 x double>*, <8 x i64>*)* @foo_double8}
!5 = !{void (<16 x double>*, <16 x double>*, <16 x i64>*)* @foo_double16}
!6 = !{!"-cl-std=CL1.2"}
!7 = !{!"double", !8}
!8 = !{!"omnipotent char", !9}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"long", !8}
