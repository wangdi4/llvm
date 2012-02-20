; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc
;
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__compute_lj_force_original(<4 x float> addrspace(1)* nocapture, <4 x float> addrspace(1)* nocapture, i32, i32 addrspace(1)* nocapture, float, float, float, i32) nounwind

declare i64 @get_global_id(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__compute_lj_force_separated_args(<4 x float> addrspace(1)* nocapture %force, <4 x float> addrspace(1)* nocapture %position, i32 %neighCount, i32 addrspace(1)* nocapture %neighList, float %cutsq, float %lj1, float %lj2, i32 %inum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %neighCount, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = and i64 %5, 4294967295
  %7 = getelementptr inbounds <4 x float> addrspace(1)* %position, i64 %6
  %8 = load <4 x float> addrspace(1)* %7, align 16
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB
  %9 = extractelement <4 x float> %8, i32 0
  %10 = extractelement <4 x float> %8, i32 1
  %11 = extractelement <4 x float> %8, i32 2
  %tmp3 = trunc i64 %5 to i32
  br label %12

; <label>:12                                      ; preds = %51, %bb.nph
  %j.02 = phi i32 [ 0, %bb.nph ], [ %52, %51 ]
  %f.11 = phi <4 x float> [ zeroinitializer, %bb.nph ], [ %f.0, %51 ]
  %tmp = mul i32 %j.02, %inum
  %tmp4 = add i32 %tmp3, %tmp
  %13 = zext i32 %tmp4 to i64
  %14 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %13
  %15 = load i32 addrspace(1)* %14, align 4
  %16 = sext i32 %15 to i64
  %17 = getelementptr inbounds <4 x float> addrspace(1)* %position, i64 %16
  %18 = load <4 x float> addrspace(1)* %17, align 16
  %19 = extractelement <4 x float> %18, i32 0
  %20 = fsub float %9, %19
  %21 = extractelement <4 x float> %18, i32 1
  %22 = fsub float %10, %21
  %23 = extractelement <4 x float> %18, i32 2
  %24 = fsub float %11, %23
  %25 = fmul float %20, %20
  %26 = fmul float %22, %22
  %27 = fadd float %25, %26
  %28 = fmul float %24, %24
  %29 = fadd float %27, %28
  %30 = fcmp olt float %29, %cutsq
  br i1 %30, label %31, label %51

; <label>:31                                      ; preds = %12
  %32 = fdiv float 1.000000e+00, %29
  %33 = fmul float %32, %32
  %34 = fmul float %33, %32
  %35 = fmul float %32, %34
  %36 = fmul float %34, %lj1
  %37 = fsub float %36, %lj2
  %38 = fmul float %35, %37
  %39 = fmul float %20, %38
  %40 = extractelement <4 x float> %f.11, i32 0
  %41 = fadd float %40, %39
  %42 = insertelement <4 x float> %f.11, float %41, i32 0
  %43 = fmul float %22, %38
  %44 = extractelement <4 x float> %f.11, i32 1
  %45 = fadd float %44, %43
  %46 = insertelement <4 x float> %42, float %45, i32 1
  %47 = fmul float %24, %38
  %48 = extractelement <4 x float> %f.11, i32 2
  %49 = fadd float %48, %47
  %50 = insertelement <4 x float> %46, float %49, i32 2
  br label %51

; <label>:51                                      ; preds = %31, %12
  %f.0 = phi <4 x float> [ %50, %31 ], [ %f.11, %12 ]
  %52 = add nsw i32 %j.02, 1
  %exitcond = icmp eq i32 %52, %neighCount
  br i1 %exitcond, label %._crit_edge, label %12

._crit_edge:                                      ; preds = %51, %SyncBB
  %f.1.lcssa = phi <4 x float> [ zeroinitializer, %SyncBB ], [ %f.0, %51 ]
  %53 = getelementptr inbounds <4 x float> addrspace(1)* %force, i64 %6
  store <4 x float> %f.1.lcssa, <4 x float> addrspace(1)* %53, align 16
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB5

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB5:                                          ; preds = %._crit_edge
  ret void
}

define void @compute_lj_force(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x float> addrspace(1)**
  %1 = load <4 x float> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x float> addrspace(1)**
  %4 = load <4 x float> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to float*
  %13 = load float* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 36
  %15 = bitcast i8* %14 to float*
  %16 = load float* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 40
  %18 = bitcast i8* %17 to float*
  %19 = load float* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 44
  %21 = bitcast i8* %20 to i32*
  %22 = load i32* %21, align 4
  %23 = getelementptr i8* %pBuffer, i64 72
  %24 = bitcast i8* %23 to %struct.PaddedDimId**
  %25 = load %struct.PaddedDimId** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 80
  %27 = bitcast i8* %26 to %struct.PaddedDimId**
  %28 = load %struct.PaddedDimId** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 96
  %30 = bitcast i8* %29 to i64*
  %31 = load i64* %30, align 8
  %32 = icmp sgt i32 %7, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %33 = getelementptr %struct.PaddedDimId* %28, i64 %CurrWI..0.i, i32 0, i64 0
  %34 = load i64* %33, align 8
  %35 = getelementptr %struct.PaddedDimId* %25, i64 0, i32 0, i64 0
  %36 = load i64* %35, align 8
  %37 = add i64 %34, %36
  %38 = and i64 %37, 4294967295
  %39 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %38
  %40 = load <4 x float> addrspace(1)* %39, align 16
  br i1 %32, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB.i
  %41 = extractelement <4 x float> %40, i32 0
  %42 = extractelement <4 x float> %40, i32 1
  %43 = extractelement <4 x float> %40, i32 2
  %tmp3.i = trunc i64 %37 to i32
  br label %44

; <label>:44                                      ; preds = %83, %bb.nph.i
  %j.02.i = phi i32 [ 0, %bb.nph.i ], [ %84, %83 ]
  %f.11.i = phi <4 x float> [ zeroinitializer, %bb.nph.i ], [ %f.0.i, %83 ]
  %tmp.i = mul i32 %j.02.i, %22
  %tmp4.i = add i32 %tmp3.i, %tmp.i
  %45 = zext i32 %tmp4.i to i64
  %46 = getelementptr inbounds i32 addrspace(1)* %10, i64 %45
  %47 = load i32 addrspace(1)* %46, align 4
  %48 = sext i32 %47 to i64
  %49 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %48
  %50 = load <4 x float> addrspace(1)* %49, align 16
  %51 = extractelement <4 x float> %50, i32 0
  %52 = fsub float %41, %51
  %53 = extractelement <4 x float> %50, i32 1
  %54 = fsub float %42, %53
  %55 = extractelement <4 x float> %50, i32 2
  %56 = fsub float %43, %55
  %57 = fmul float %52, %52
  %58 = fmul float %54, %54
  %59 = fadd float %57, %58
  %60 = fmul float %56, %56
  %61 = fadd float %59, %60
  %62 = fcmp olt float %61, %13
  br i1 %62, label %63, label %83

; <label>:63                                      ; preds = %44
  %64 = fdiv float 1.000000e+00, %61
  %65 = fmul float %64, %64
  %66 = fmul float %65, %64
  %67 = fmul float %64, %66
  %68 = fmul float %66, %16
  %69 = fsub float %68, %19
  %70 = fmul float %67, %69
  %71 = fmul float %52, %70
  %72 = extractelement <4 x float> %f.11.i, i32 0
  %73 = fadd float %72, %71
  %74 = insertelement <4 x float> %f.11.i, float %73, i32 0
  %75 = fmul float %54, %70
  %76 = extractelement <4 x float> %f.11.i, i32 1
  %77 = fadd float %76, %75
  %78 = insertelement <4 x float> %74, float %77, i32 1
  %79 = fmul float %56, %70
  %80 = extractelement <4 x float> %f.11.i, i32 2
  %81 = fadd float %80, %79
  %82 = insertelement <4 x float> %78, float %81, i32 2
  br label %83

; <label>:83                                      ; preds = %63, %44
  %f.0.i = phi <4 x float> [ %82, %63 ], [ %f.11.i, %44 ]
  %84 = add nsw i32 %j.02.i, 1
  %exitcond.i = icmp eq i32 %84, %7
  br i1 %exitcond.i, label %._crit_edge.i, label %44

._crit_edge.i:                                    ; preds = %83, %SyncBB.i
  %f.1.lcssa.i = phi <4 x float> [ zeroinitializer, %SyncBB.i ], [ %f.0.i, %83 ]
  %85 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %38
  store <4 x float> %f.1.lcssa.i, <4 x float> addrspace(1)* %85, align 16
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %31
  br i1 %check.WI.iter.i, label %thenBB.i, label %__compute_lj_force_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__compute_lj_force_separated_args.exit:           ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32 addrspace(1)*, float, float, float, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__compute_lj_force_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, int const, int __attribute__((address_space(1))) *, float const, float const, float const, int const", metadata !"opencl_compute_lj_force_locals_anchor", void (i8*)* @compute_lj_force}
!1 = metadata !{i32 0, i32 0, i32 0}
