; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -CLBltnPreVec -scalarize -packetize -packet-size=4 -resolve -verify %s -S -o - \
; RUN: | FileCheck %s

;; This test:
;; 1. Based on Apple clang calling convention
;; 2. Checks that vectorizer will vectorize non-uniform sincos instruction.

; CHECK: __Vectorized_.math_kernel3
; CHECK-NOT: sincos
; CHECK: call void @_Z14sincos_ret2ptrDv4_fPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_fPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_fPS_S0_
; CHECK-NOT: sincos
; CHECK: ret

  ; ModuleID = '-'
  target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-s0:64:64-f80:128:128-n8:16:32:64"
  target triple = "x86_64-applecl-darwin11"
  
  @sgv = internal constant [4 x i8] c"222\00"
  @fgv = internal constant [0 x i8] zeroinitializer
  @lvgv = internal constant [0 x i8*] zeroinitializer
  @llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @math_kernel3 to i8*), i8* getelementptr inbounds ([4 x i8]* @sgv, i32 0, i32 0), i8* getelementptr inbounds ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"
  
  declare void @math_kernel3(float addrspace(1)* %out, float addrspace(1)* %out2, float addrspace(1)* %in) nounwind
  
  declare i64 @_Z13get_global_idj(i32)
  
  declare i64 @get_global_size(i32)
  
  declare <4 x float> @_Z6vload3mPKU3AS1f(i64, float addrspace(1)*)
  
  declare <4 x float> @_Z6sincosDv3_fPS_(<2 x double>, <3 x float>*)
  
  declare void @_Z7vstore3Dv3_fmPU3AS1f(<2 x double>, i64, float addrspace(1)*)
  
  declare i64 @_Z14get_local_sizej(i32)
  
  declare i64 @get_base_global_id.(i32)
  
  define void @__Vectorized_.math_kernel3(float addrspace(1)* %out, float addrspace(1)* %out2, float addrspace(1)* %in) nounwind {
     %iout = alloca <3 x float>, align 16
     %iout1 = alloca <3 x float>, align 16
     %1 = call i64 @_Z13get_global_idj(i32 0) nounwind
     %2 = add i64 %1, 1
     %3 = call i64 @get_global_size(i32 0) nounwind
     %4 = icmp ult i64 %2, %3
     br i1 %4, label %5, label %17
     
     ; <label>:5                                       ; preds = %0
     %6 = mul i64 %1, 3
     %7 = getelementptr inbounds float addrspace(1)* %in, i64 %6
     %8 = call <4 x float> @_Z6vload3mPKU3AS1f(i64 0, float addrspace(1)* %7) nounwind
     %9 = bitcast <3 x float>* %iout to <4 x float>*
     store <4 x float> <float 0x7FF8000000000000, float 0x7FF8000000000000, float 0x7FF8000000000000, float undef>, <4 x float>* %9, align 16, !tbaa !4
     %10 = bitcast <4 x float> %8 to <2 x double>
     %11 = call <4 x float> @_Z6sincosDv3_fPS_(<2 x double> %10, <3 x float>* %iout) nounwind
     %12 = getelementptr inbounds float addrspace(1)* %out, i64 %6
     %13 = bitcast <4 x float> %11 to <2 x double>
     call void @_Z7vstore3Dv3_fmPU3AS1f(<2 x double> %13, i64 0, float addrspace(1)* %12) nounwind
     %14 = load <4 x float>* %9, align 16
     %15 = getelementptr inbounds float addrspace(1)* %out2, i64 %6
     %16 = bitcast <4 x float> %14 to <2 x double>
     call void @_Z7vstore3Dv3_fmPU3AS1f(<2 x double> %16, i64 0, float addrspace(1)* %15) nounwind
     br label %56
     
     ; <label>:17                                      ; preds = %0
     %18 = and i64 %1, 1
     %19 = bitcast <3 x float>* %iout1 to <4 x float>*
     store <4 x float> <float 0x7FF8000000000000, float 0x7FF8000000000000, float 0x7FF8000000000000, float undef>, <4 x float>* %19, align 16, !tbaa !4
     br label %NodeBlock
     
 NodeBlock:                                        ; preds = %17
     %Pivot = icmp eq i64 %18, 0
     br i1 %Pivot, label %LeafBlock, label %LeafBlock1
     
 LeafBlock1:                                       ; preds = %NodeBlock
     %SwitchLeaf2 = icmp eq i64 %18, 0
     br i1 %SwitchLeaf2, label %NewDefault, label %.thread3
     
 LeafBlock:                                        ; preds = %NodeBlock
     %SwitchLeaf = icmp eq i64 %18, 0
     br i1 %SwitchLeaf, label %.thread, label %NewDefault
     
     .thread3:                                         ; preds = %LeafBlock1
     %20 = mul i64 %1, 3
     %21 = getelementptr inbounds float addrspace(1)* %in, i64 %20
     %22 = load float addrspace(1)* %21, align 4, !tbaa !6
     %23 = insertelement <3 x float> undef, float %22, i32 0
     %24 = insertelement <3 x float> %23, float 0x7FF8000000000000, i32 1
     %25 = insertelement <3 x float> %24, float 0x7FF8000000000000, i32 2
     %26 = shufflevector <3 x float> %25, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
     %27 = bitcast <4 x float> %26 to <2 x double>
     %28 = call <4 x float> @_Z6sincosDv3_fPS_(<2 x double> %27, <3 x float>* %iout1) nounwind
     %.pre = load <3 x float>* %iout1, align 16
     br label %48
     
     .thread:                                          ; preds = %LeafBlock
     %29 = mul i64 %1, 3
     %30 = getelementptr inbounds float addrspace(1)* %in, i64 %29
     %31 = load float addrspace(1)* %30, align 4, !tbaa !6
     %32 = insertelement <3 x float> undef, float %31, i32 0
     %33 = add i64 %29, 1
     %34 = getelementptr inbounds float addrspace(1)* %in, i64 %33
     %35 = load float addrspace(1)* %34, align 4, !tbaa !6
     %36 = insertelement <3 x float> %32, float %35, i32 1
     %37 = insertelement <3 x float> %36, float 0x7FF8000000000000, i32 2
     %38 = shufflevector <3 x float> %37, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
     %39 = bitcast <4 x float> %38 to <2 x double>
     %40 = call <4 x float> @_Z6sincosDv3_fPS_(<2 x double> %39, <3 x float>* %iout1) nounwind
     %41 = extractelement <4 x float> %40, i32 1
     %42 = getelementptr inbounds float addrspace(1)* %out, i64 %33
     store float %41, float addrspace(1)* %42, align 4, !tbaa !6
     %43 = load <3 x float>* %iout1, align 16
     %44 = extractelement <3 x float> %43, i32 1
     %45 = getelementptr inbounds float addrspace(1)* %out2, i64 %33
     store float %44, float addrspace(1)* %45, align 4, !tbaa !6
     br label %48
     
 NewDefault:                                       ; preds = %LeafBlock1, %LeafBlock
     br label %46
     
     ; <label>:46                                      ; preds = %NewDefault
     %47 = call <4 x float> @_Z6sincosDv3_fPS_(<2 x double> undef, <3 x float>* %iout1) nounwind
     br label %56
     
     ; <label>:48                                      ; preds = %.thread, %.thread3
     %49 = phi <3 x float> [ %43, %.thread ], [ %.pre, %.thread3 ]
     %50 = phi <4 x float> [ %40, %.thread ], [ %28, %.thread3 ]
     %51 = extractelement <4 x float> %50, i32 0
     %52 = mul i64 %1, 3
     %53 = getelementptr inbounds float addrspace(1)* %out, i64 %52
     store float %51, float addrspace(1)* %53, align 4, !tbaa !6
     %54 = extractelement <3 x float> %49, i32 0
     %55 = getelementptr inbounds float addrspace(1)* %out2, i64 %52
     store float %54, float addrspace(1)* %55, align 4, !tbaa !6
     br label %56
     
     ; <label>:56                                      ; preds = %48, %46, %5
     ret void
 }
  
  !opencl.kernels = !{!0}
  !opencl.kernel_info = !{!7}
  
  !0 = !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @math_kernel3, !1}
  !1 = !{!"apple.cl.arg_metadata", !2, !2, !2}
  !2 = !{!"stream", !"write", !"global"}
  !3 = !{!"no_barrier_path", i1 true}
  !4 = !{!"omnipotent char", !5}
  !5 = !{!"Simple C/C++ TBAA"}
  !6 = !{!"float", !4}

  !7 = !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @math_kernel3, !8}
  !8 = !{!3}