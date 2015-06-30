; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -CLBltnPreVec -scalarize -packetize -packet-size=4 -resolve -verify %s -S -o - \
; RUN: | FileCheck %s

;;; Currently vectorizer does not supports double3 calling convention, thus this test should fail!
;; This test:
;; 1. Based on Apple clang calling convention
;; 2. Checks that vectorizer will vectorize non-uniform sincos instruction.

; CHECK: __Vectorized_.math_kernel3
; CHECK-NOT: sincos
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK-NOT: sincos
; CHECK: ret

  ; ModuleID = '-'
  target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a64:64:64-s0:64:64-f80:128:128-n8:16:32:64"
  target triple = "x86_64-applecl-darwin11"
  
  @sgv = internal constant [4 x i8] c"222\00"
  @fgv = internal constant [0 x i8] zeroinitializer
  @lvgv = internal constant [0 x i8*] zeroinitializer
  @llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (double addrspace(1)*, double addrspace(1)*, double addrspace(1)*)* @math_kernel3 to i8*), i8* getelementptr inbounds ([4 x i8]* @sgv, i32 0, i32 0), i8* getelementptr inbounds ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"
  
  declare void @math_kernel3(double addrspace(1)* %out, double addrspace(1)* %out2, double addrspace(1)* %in) nounwind
  
  declare i64 @_Z13get_global_idj(i32)
  
  declare i64 @get_global_size(i32)
  
  declare <3 x double> @_Z6vload3mPKU3AS1d(i64, double addrspace(1)*)
  
  declare <3 x double> @_Z6sincosDv3_dPS_(<3 x double>* byval align 32, <3 x double>*)
  
  declare void @_Z7vstore3Dv3_dmPU3AS1d(<3 x double>* byval align 32, i64, double addrspace(1)*)
  
  declare i64 @_Z14get_local_sizej(i32)
  
  declare i64 @get_base_global_id.(i32)
  
  define void @__Vectorized_.math_kernel3(double addrspace(1)* %out, double addrspace(1)* %out2, double addrspace(1)* %in) nounwind {
     %iout = alloca <3 x double>, align 32
     %1 = alloca <3 x double>, align 32
     %2 = alloca <3 x double>, align 32
     %3 = alloca <3 x double>, align 32
     %iout1 = alloca <3 x double>, align 32
     %4 = alloca <3 x double>, align 32
     %5 = call i64 @_Z13get_global_idj(i32 0) nounwind
     %6 = add i64 %5, 1
     %7 = call i64 @get_global_size(i32 0) nounwind
     %8 = icmp ult i64 %6, %7
     br i1 %8, label %9, label %23
     
     ; <label>:9                                       ; preds = %0
     %10 = mul i64 %5, 3
     %11 = getelementptr inbounds double addrspace(1)* %in, i64 %10
     %12 = call <3 x double> @_Z6vload3mPKU3AS1d(i64 0, double addrspace(1)* %11) nounwind
     %13 = bitcast <3 x double>* %iout to <4 x double>*
     store <4 x double> <double 0x7FF8000000000000, double 0x7FF8000000000000, double 0x7FF8000000000000, double undef>, <4 x double>* %13, align 32, !tbaa !4
     %14 = shufflevector <3 x double> %12, <3 x double> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
     %15 = bitcast <3 x double>* %1 to <4 x double>*
     store <4 x double> %14, <4 x double>* %15, align 32
     %16 = call <3 x double> @_Z6sincosDv3_dPS_(<3 x double>* byval align 32 %1, <3 x double>* %iout) nounwind
     %17 = getelementptr inbounds double addrspace(1)* %out, i64 %10
     %18 = shufflevector <3 x double> %16, <3 x double> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
     %19 = bitcast <3 x double>* %2 to <4 x double>*
     store <4 x double> %18, <4 x double>* %19, align 32
     call void @_Z7vstore3Dv3_dmPU3AS1d(<3 x double>* byval align 32 %2, i64 0, double addrspace(1)* %17) nounwind
     %20 = load <4 x double>* %13, align 32
     %21 = getelementptr inbounds double addrspace(1)* %out2, i64 %10
     %22 = bitcast <3 x double>* %3 to <4 x double>*
     store <4 x double> %20, <4 x double>* %22, align 32
     call void @_Z7vstore3Dv3_dmPU3AS1d(<3 x double>* byval align 32 %3, i64 0, double addrspace(1)* %21) nounwind
     br label %62
     
     ; <label>:23                                      ; preds = %0
     %24 = and i64 %5, 1
     %25 = bitcast <3 x double>* %iout1 to <4 x double>*
     store <4 x double> <double 0x7FF8000000000000, double 0x7FF8000000000000, double 0x7FF8000000000000, double undef>, <4 x double>* %25, align 32, !tbaa !4
     br label %NodeBlock
     
 NodeBlock:                                        ; preds = %23
     %Pivot = icmp eq i64 %24, 0
     br i1 %Pivot, label %LeafBlock, label %LeafBlock1
     
 LeafBlock1:                                       ; preds = %NodeBlock
     %SwitchLeaf2 = icmp eq i64 %24, 0
     br i1 %SwitchLeaf2, label %NewDefault, label %.thread3
     
 LeafBlock:                                        ; preds = %NodeBlock
     %SwitchLeaf = icmp eq i64 %24, 0
     br i1 %SwitchLeaf, label %.thread, label %NewDefault
     
     .thread3:                                         ; preds = %LeafBlock1
     %26 = mul i64 %5, 3
     %27 = getelementptr inbounds double addrspace(1)* %in, i64 %26
     %28 = load double addrspace(1)* %27, align 8, !tbaa !6
     %29 = insertelement <3 x double> undef, double %28, i32 0
     %30 = insertelement <3 x double> %29, double 0x7FF8000000000000, i32 1
     %31 = insertelement <3 x double> %30, double 0x7FF8000000000000, i32 2
     %32 = shufflevector <3 x double> %31, <3 x double> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
     %33 = bitcast <3 x double>* %4 to <4 x double>*
     store <4 x double> %32, <4 x double>* %33, align 32
     %34 = call <3 x double> @_Z6sincosDv3_dPS_(<3 x double>* byval align 32 %4, <3 x double>* %iout1) nounwind
     %.pre = load <3 x double>* %iout1, align 32
     br label %54
     
     .thread:                                          ; preds = %LeafBlock
     %35 = mul i64 %5, 3
     %36 = getelementptr inbounds double addrspace(1)* %in, i64 %35
     %37 = load double addrspace(1)* %36, align 8, !tbaa !6
     %38 = insertelement <3 x double> undef, double %37, i32 0
     %39 = add i64 %35, 1
     %40 = getelementptr inbounds double addrspace(1)* %in, i64 %39
     %41 = load double addrspace(1)* %40, align 8, !tbaa !6
     %42 = insertelement <3 x double> %38, double %41, i32 1
     %43 = insertelement <3 x double> %42, double 0x7FF8000000000000, i32 2
     %44 = shufflevector <3 x double> %43, <3 x double> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
     %45 = bitcast <3 x double>* %4 to <4 x double>*
     store <4 x double> %44, <4 x double>* %45, align 32
     %46 = call <3 x double> @_Z6sincosDv3_dPS_(<3 x double>* byval align 32 %4, <3 x double>* %iout1) nounwind
     %47 = extractelement <3 x double> %46, i32 1
     %48 = getelementptr inbounds double addrspace(1)* %out, i64 %39
     store double %47, double addrspace(1)* %48, align 8, !tbaa !6
     %49 = load <3 x double>* %iout1, align 32
     %50 = extractelement <3 x double> %49, i32 1
     %51 = getelementptr inbounds double addrspace(1)* %out2, i64 %39
     store double %50, double addrspace(1)* %51, align 8, !tbaa !6
     br label %54
     
 NewDefault:                                       ; preds = %LeafBlock1, %LeafBlock
     br label %52
     
     ; <label>:52                                      ; preds = %NewDefault
     %53 = call <3 x double> @_Z6sincosDv3_dPS_(<3 x double>* byval align 32 %4, <3 x double>* %iout1) nounwind
     br label %62
     
     ; <label>:54                                      ; preds = %.thread, %.thread3
     %55 = phi <3 x double> [ %49, %.thread ], [ %.pre, %.thread3 ]
     %56 = phi <3 x double> [ %46, %.thread ], [ %34, %.thread3 ]
     %57 = extractelement <3 x double> %56, i32 0
     %58 = mul i64 %5, 3
     %59 = getelementptr inbounds double addrspace(1)* %out, i64 %58
     store double %57, double addrspace(1)* %59, align 8, !tbaa !6
     %60 = extractelement <3 x double> %55, i32 0
     %61 = getelementptr inbounds double addrspace(1)* %out2, i64 %58
     store double %60, double addrspace(1)* %61, align 8, !tbaa !6
     br label %62
     
     ; <label>:62                                      ; preds = %54, %52, %9
     ret void
 }
  
  !opencl.kernels = !{!0}
  !opencl.kernel_info = !{!7}
  
  !0 = !{void (double addrspace(1)*, double addrspace(1)*, double addrspace(1)*)* @math_kernel3, !1}
  !1 = !{!"apple.cl.arg_metadata", !2, !2, !2}
  !2 = !{!"stream", !"write", !"global"}
  !3 = !{!"no_barrier_path", i1 true}
  !4 = !{!"omnipotent char", !5}
  !5 = !{!"Simple C/C++ TBAA"}
  !6 = !{!"double", !4}

  !7 = !{void (double addrspace(1)*, double addrspace(1)*, double addrspace(1)*)* @math_kernel3, !8}
  !8 = !{!3}