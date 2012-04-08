; XFAIL: win32
; XFAIL: *
;
; RUN: opt < %s -runtimelib %p/../../../vectorizer/Full/runtime.bc \
; RUN:       -std-compile-opts -inline-threshold=4096 -inline -lowerswitch \
; RUN:       -scalarize -mergereturn -loopsimplify -phicanon -predicate \
; RUN:       -mem2reg -dce -packetize -packet-size=16 -resolve -verify -S \
; RUN:     | llc -O2 -mtriple=x86_64-pc-linux \
; RUN:           -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

; ModuleID = 'md.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-linux"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_compute_lj_force_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_compute_lj_force_parameters = appending global [189 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, int const, int __attribute__((address_space(1))) *, float const, float const, float const, int const\00", section "llvm.metadata" ; <[189 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32 addrspace(1)*, float, float, float, i32)* @compute_lj_force to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_compute_lj_force_locals to i8*), i8* getelementptr inbounds ([189 x i8]* @opencl_compute_lj_force_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @compute_lj_force(<4 x float> addrspace(1)* nocapture %force, <4 x float> addrspace(1)* nocapture %position, i32 %neighCount, i32 addrspace(1)* nocapture %neighList, float %cutsq, float %lj1, float %lj2, i32 %inum) nounwind {
; <label>:0
  %1 = tail call i64 @get_global_id(i32 0) nounwind ; <i64> [#uses=2]
  %2 = and i64 %1, 4294967295                     ; <i64> [#uses=2]
  %3 = getelementptr inbounds <4 x float> addrspace(1)* %position, i64 %2 ; <<4 x float> addrspace(1)*> [#uses=1]
  %4 = load <4 x float> addrspace(1)* %3          ; <<4 x float>> [#uses=3]
  %5 = icmp sgt i32 %neighCount, 0                ; <i1> [#uses=1]
  br i1 %5, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %0
  %6 = extractelement <4 x float> %4, i32 0       ; <float> [#uses=1]
  %7 = extractelement <4 x float> %4, i32 1       ; <float> [#uses=1]
  %8 = extractelement <4 x float> %4, i32 2       ; <float> [#uses=1]
  %tmp3 = trunc i64 %1 to i32                     ; <i32> [#uses=1]
  br label %9

; <label>:9                                       ; preds = %48, %bb.nph
  %j.02 = phi i32 [ 0, %bb.nph ], [ %49, %48 ]    ; <i32> [#uses=2]
  %f.11 = phi <4 x float> [ zeroinitializer, %bb.nph ], [ %f.0, %48 ] ; <<4 x float>> [#uses=5]
  %tmp = mul i32 %j.02, %inum                     ; <i32> [#uses=1]
  %tmp4 = add i32 %tmp3, %tmp                     ; <i32> [#uses=1]
  %10 = zext i32 %tmp4 to i64                     ; <i64> [#uses=1]
  %11 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %10 ; <i32 addrspace(1)*> [#uses=1]
  %12 = load i32 addrspace(1)* %11                ; <i32> [#uses=1]
  %13 = sext i32 %12 to i64                       ; <i64> [#uses=1]
  %14 = getelementptr inbounds <4 x float> addrspace(1)* %position, i64 %13 ; <<4 x float> addrspace(1)*> [#uses=1]
  %15 = load <4 x float> addrspace(1)* %14        ; <<4 x float>> [#uses=3]
  %16 = extractelement <4 x float> %15, i32 0     ; <float> [#uses=1]
  %17 = fsub float %6, %16                        ; <float> [#uses=3]
  %18 = extractelement <4 x float> %15, i32 1     ; <float> [#uses=1]
  %19 = fsub float %7, %18                        ; <float> [#uses=3]
  %20 = extractelement <4 x float> %15, i32 2     ; <float> [#uses=1]
  %21 = fsub float %8, %20                        ; <float> [#uses=3]
  %22 = fmul float %17, %17                       ; <float> [#uses=1]
  %23 = fmul float %19, %19                       ; <float> [#uses=1]
  %24 = fadd float %22, %23                       ; <float> [#uses=1]
  %25 = fmul float %21, %21                       ; <float> [#uses=1]
  %26 = fadd float %24, %25                       ; <float> [#uses=2]
  %27 = fcmp olt float %26, %cutsq                ; <i1> [#uses=1]
  br i1 %27, label %28, label %48

; <label>:28                                      ; preds = %9
  %29 = fdiv float 1.000000e+000, %26             ; <float> [#uses=4]
  %30 = fmul float %29, %29                       ; <float> [#uses=1]
  %31 = fmul float %30, %29                       ; <float> [#uses=2]
  %32 = fmul float %29, %31                       ; <float> [#uses=1]
  %33 = fmul float %31, %lj1                      ; <float> [#uses=1]
  %34 = fsub float %33, %lj2                      ; <float> [#uses=1]
  %35 = fmul float %32, %34                       ; <float> [#uses=3]
  %36 = fmul float %17, %35                       ; <float> [#uses=1]
  %37 = extractelement <4 x float> %f.11, i32 0   ; <float> [#uses=1]
  %38 = fadd float %37, %36                       ; <float> [#uses=1]
  %39 = insertelement <4 x float> %f.11, float %38, i32 0 ; <<4 x float>> [#uses=1]
  %40 = fmul float %19, %35                       ; <float> [#uses=1]
  %41 = extractelement <4 x float> %f.11, i32 1   ; <float> [#uses=1]
  %42 = fadd float %41, %40                       ; <float> [#uses=1]
  %43 = insertelement <4 x float> %39, float %42, i32 1 ; <<4 x float>> [#uses=1]
  %44 = fmul float %21, %35                       ; <float> [#uses=1]
  %45 = extractelement <4 x float> %f.11, i32 2   ; <float> [#uses=1]
  %46 = fadd float %45, %44                       ; <float> [#uses=1]
  %47 = insertelement <4 x float> %43, float %46, i32 2 ; <<4 x float>> [#uses=1]
  br label %48

; <label>:48                                      ; preds = %28, %9
  %f.0 = phi <4 x float> [ %47, %28 ], [ %f.11, %9 ] ; <<4 x float>> [#uses=2]
  %49 = add nsw i32 %j.02, 1                      ; <i32> [#uses=2]
  %exitcond = icmp eq i32 %49, %neighCount        ; <i1> [#uses=1]
  br i1 %exitcond, label %._crit_edge, label %9

._crit_edge:                                      ; preds = %48, %0
  %f.1.lcssa = phi <4 x float> [ zeroinitializer, %0 ], [ %f.0, %48 ] ; <<4 x float>> [#uses=1]
  %50 = getelementptr inbounds <4 x float> addrspace(1)* %force, i64 %2 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %f.1.lcssa, <4 x float> addrspace(1)* %50
; KNF: ret
  ret void
}

declare i64 @get_global_id(i32)
