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

declare void @__compute_lj_force_original(<4 x double> addrspace(1)* nocapture, <4 x double> addrspace(1)* nocapture, i32, i32 addrspace(1)* nocapture, double, double, double, i32) nounwind

declare i64 @get_global_id(i32)

declare void @____Vectorized_.compute_lj_force_original(<4 x double> addrspace(1)* nocapture, <4 x double> addrspace(1)* nocapture, i32, i32 addrspace(1)* nocapture, double, double, double, i32) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  ret i1 %t
}

declare i32 @masked_load0(i1, i32 addrspace(1)*)

declare <4 x double> @masked_load1(i1, <4 x double> addrspace(1)*)

define i1 @allOne_v16(<16 x i1> %pred) {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %val = call i32 @llvm.x86.mic.kortestc(i16 %ipred, i16 %ipred)
  %tmp = and i32 %val, 1
  %res = icmp ne i32 %tmp, 0
  ret i1 %res
}

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__compute_lj_force_separated_args(<4 x double> addrspace(1)* nocapture %force, <4 x double> addrspace(1)* nocapture %position, i32 %neighCount, i32 addrspace(1)* nocapture %neighList, double %cutsq, double %lj1, double %lj2, i32 %inum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %neighCount, 0
  br label %SyncBB5

SyncBB5:                                          ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = and i64 %5, 4294967295
  %7 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %6
  %8 = load <4 x double> addrspace(1)* %7, align 32
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB5
  %9 = extractelement <4 x double> %8, i32 0
  %10 = extractelement <4 x double> %8, i32 1
  %11 = extractelement <4 x double> %8, i32 2
  %tmp3 = trunc i64 %5 to i32
  br label %12

; <label>:12                                      ; preds = %51, %bb.nph
  %j.02 = phi i32 [ 0, %bb.nph ], [ %52, %51 ]
  %f.11 = phi <4 x double> [ zeroinitializer, %bb.nph ], [ %f.0, %51 ]
  %tmp = mul i32 %j.02, %inum
  %tmp4 = add i32 %tmp3, %tmp
  %13 = zext i32 %tmp4 to i64
  %14 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %13
  %15 = load i32 addrspace(1)* %14, align 4
  %16 = sext i32 %15 to i64
  %17 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %16
  %18 = load <4 x double> addrspace(1)* %17, align 32
  %19 = extractelement <4 x double> %18, i32 0
  %20 = fsub double %9, %19
  %21 = extractelement <4 x double> %18, i32 1
  %22 = fsub double %10, %21
  %23 = extractelement <4 x double> %18, i32 2
  %24 = fsub double %11, %23
  %25 = fmul double %20, %20
  %26 = fmul double %22, %22
  %27 = fadd double %25, %26
  %28 = fmul double %24, %24
  %29 = fadd double %27, %28
  %30 = fcmp olt double %29, %cutsq
  br i1 %30, label %31, label %51

; <label>:31                                      ; preds = %12
  %32 = fdiv double 1.000000e+00, %29
  %33 = fmul double %32, %32
  %34 = fmul double %33, %32
  %35 = fmul double %32, %34
  %36 = fmul double %34, %lj1
  %37 = fsub double %36, %lj2
  %38 = fmul double %35, %37
  %39 = fmul double %20, %38
  %40 = extractelement <4 x double> %f.11, i32 0
  %41 = fadd double %40, %39
  %42 = insertelement <4 x double> %f.11, double %41, i32 0
  %43 = fmul double %22, %38
  %44 = extractelement <4 x double> %f.11, i32 1
  %45 = fadd double %44, %43
  %46 = insertelement <4 x double> %42, double %45, i32 1
  %47 = fmul double %24, %38
  %48 = extractelement <4 x double> %f.11, i32 2
  %49 = fadd double %48, %47
  %50 = insertelement <4 x double> %46, double %49, i32 2
  br label %51

; <label>:51                                      ; preds = %31, %12
  %f.0 = phi <4 x double> [ %50, %31 ], [ %f.11, %12 ]
  %52 = add nsw i32 %j.02, 1
  %exitcond = icmp eq i32 %52, %neighCount
  br i1 %exitcond, label %._crit_edge, label %12

._crit_edge:                                      ; preds = %51, %SyncBB5
  %f.1.lcssa = phi <4 x double> [ zeroinitializer, %SyncBB5 ], [ %f.0, %51 ]
  %53 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %6
  store <4 x double> %f.1.lcssa, <4 x double> addrspace(1)* %53, align 32
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB5

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.compute_lj_force_separated_args(<4 x double> addrspace(1)* nocapture %force, <4 x double> addrspace(1)* nocapture %position, i32 %neighCount, i32 addrspace(1)* nocapture %neighList, double %cutsq, double %lj1, double %lj2, i32 %inum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph:
  %temp264 = insertelement <16 x double> undef, double %lj2, i32 0
  %vector265 = shufflevector <16 x double> %temp264, <16 x double> undef, <16 x i32> zeroinitializer
  %temp262 = insertelement <16 x double> undef, double %lj1, i32 0
  %vector263 = shufflevector <16 x double> %temp262, <16 x double> undef, <16 x i32> zeroinitializer
  %temp257 = insertelement <16 x double> undef, double %cutsq, i32 0
  %vector258 = shufflevector <16 x double> %temp257, <16 x double> undef, <16 x i32> zeroinitializer
  %0 = icmp sgt i32 %neighCount, 0
  %temp88 = insertelement <16 x i1> undef, i1 %0, i32 0
  %vector89 = shufflevector <16 x i1> %temp88, <16 x i1> undef, <16 x i32> zeroinitializer
  %negIncomingLoopMask = xor i1 %0, true
  %temp = insertelement <16 x i1> undef, i1 %negIncomingLoopMask, i32 0
  %vector = shufflevector <16 x i1> %temp, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %bb.nph
  %CurrWI..0 = phi i64 [ 0, %bb.nph ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %broadcast1 = insertelement <16 x i64> undef, i64 %5, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %6 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %7 = and <16 x i64> %6, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract = extractelement <16 x i64> %7, i32 0
  %extract66 = extractelement <16 x i64> %7, i32 1
  %extract67 = extractelement <16 x i64> %7, i32 2
  %extract68 = extractelement <16 x i64> %7, i32 3
  %extract69 = extractelement <16 x i64> %7, i32 4
  %extract70 = extractelement <16 x i64> %7, i32 5
  %extract71 = extractelement <16 x i64> %7, i32 6
  %extract72 = extractelement <16 x i64> %7, i32 7
  %extract73 = extractelement <16 x i64> %7, i32 8
  %extract74 = extractelement <16 x i64> %7, i32 9
  %extract75 = extractelement <16 x i64> %7, i32 10
  %extract76 = extractelement <16 x i64> %7, i32 11
  %extract77 = extractelement <16 x i64> %7, i32 12
  %extract78 = extractelement <16 x i64> %7, i32 13
  %extract79 = extractelement <16 x i64> %7, i32 14
  %extract80 = extractelement <16 x i64> %7, i32 15
  %8 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract
  %9 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract66
  %10 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract67
  %11 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract68
  %12 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract69
  %13 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract70
  %14 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract71
  %15 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract72
  %16 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract73
  %17 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract74
  %18 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract75
  %19 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract76
  %20 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract77
  %21 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract78
  %22 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract79
  %23 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %extract80
  %24 = load <4 x double> addrspace(1)* %8, align 32
  %25 = load <4 x double> addrspace(1)* %9, align 32
  %26 = load <4 x double> addrspace(1)* %10, align 32
  %27 = load <4 x double> addrspace(1)* %11, align 32
  %28 = load <4 x double> addrspace(1)* %12, align 32
  %29 = load <4 x double> addrspace(1)* %13, align 32
  %30 = load <4 x double> addrspace(1)* %14, align 32
  %31 = load <4 x double> addrspace(1)* %15, align 32
  %32 = load <4 x double> addrspace(1)* %16, align 32
  %33 = load <4 x double> addrspace(1)* %17, align 32
  %34 = load <4 x double> addrspace(1)* %18, align 32
  %35 = load <4 x double> addrspace(1)* %19, align 32
  %36 = load <4 x double> addrspace(1)* %20, align 32
  %37 = load <4 x double> addrspace(1)* %21, align 32
  %38 = load <4 x double> addrspace(1)* %22, align 32
  %39 = load <4 x double> addrspace(1)* %23, align 32
  %40 = extractelement <4 x double> %24, i32 0
  %41 = extractelement <4 x double> %25, i32 0
  %42 = extractelement <4 x double> %26, i32 0
  %43 = extractelement <4 x double> %27, i32 0
  %44 = extractelement <4 x double> %28, i32 0
  %45 = extractelement <4 x double> %29, i32 0
  %46 = extractelement <4 x double> %30, i32 0
  %47 = extractelement <4 x double> %31, i32 0
  %48 = extractelement <4 x double> %32, i32 0
  %49 = extractelement <4 x double> %33, i32 0
  %50 = extractelement <4 x double> %34, i32 0
  %51 = extractelement <4 x double> %35, i32 0
  %52 = extractelement <4 x double> %36, i32 0
  %53 = extractelement <4 x double> %37, i32 0
  %54 = extractelement <4 x double> %38, i32 0
  %55 = extractelement <4 x double> %39, i32 0
  %temp.vect161 = insertelement <16 x double> undef, double %40, i32 0
  %temp.vect162 = insertelement <16 x double> %temp.vect161, double %41, i32 1
  %temp.vect163 = insertelement <16 x double> %temp.vect162, double %42, i32 2
  %temp.vect164 = insertelement <16 x double> %temp.vect163, double %43, i32 3
  %temp.vect165 = insertelement <16 x double> %temp.vect164, double %44, i32 4
  %temp.vect166 = insertelement <16 x double> %temp.vect165, double %45, i32 5
  %temp.vect167 = insertelement <16 x double> %temp.vect166, double %46, i32 6
  %temp.vect168 = insertelement <16 x double> %temp.vect167, double %47, i32 7
  %temp.vect169 = insertelement <16 x double> %temp.vect168, double %48, i32 8
  %temp.vect170 = insertelement <16 x double> %temp.vect169, double %49, i32 9
  %temp.vect171 = insertelement <16 x double> %temp.vect170, double %50, i32 10
  %temp.vect172 = insertelement <16 x double> %temp.vect171, double %51, i32 11
  %temp.vect173 = insertelement <16 x double> %temp.vect172, double %52, i32 12
  %temp.vect174 = insertelement <16 x double> %temp.vect173, double %53, i32 13
  %temp.vect175 = insertelement <16 x double> %temp.vect174, double %54, i32 14
  %temp.vect176 = insertelement <16 x double> %temp.vect175, double %55, i32 15
  %56 = extractelement <4 x double> %24, i32 1
  %57 = extractelement <4 x double> %25, i32 1
  %58 = extractelement <4 x double> %26, i32 1
  %59 = extractelement <4 x double> %27, i32 1
  %60 = extractelement <4 x double> %28, i32 1
  %61 = extractelement <4 x double> %29, i32 1
  %62 = extractelement <4 x double> %30, i32 1
  %63 = extractelement <4 x double> %31, i32 1
  %64 = extractelement <4 x double> %32, i32 1
  %65 = extractelement <4 x double> %33, i32 1
  %66 = extractelement <4 x double> %34, i32 1
  %67 = extractelement <4 x double> %35, i32 1
  %68 = extractelement <4 x double> %36, i32 1
  %69 = extractelement <4 x double> %37, i32 1
  %70 = extractelement <4 x double> %38, i32 1
  %71 = extractelement <4 x double> %39, i32 1
  %temp.vect193 = insertelement <16 x double> undef, double %56, i32 0
  %temp.vect194 = insertelement <16 x double> %temp.vect193, double %57, i32 1
  %temp.vect195 = insertelement <16 x double> %temp.vect194, double %58, i32 2
  %temp.vect196 = insertelement <16 x double> %temp.vect195, double %59, i32 3
  %temp.vect197 = insertelement <16 x double> %temp.vect196, double %60, i32 4
  %temp.vect198 = insertelement <16 x double> %temp.vect197, double %61, i32 5
  %temp.vect199 = insertelement <16 x double> %temp.vect198, double %62, i32 6
  %temp.vect200 = insertelement <16 x double> %temp.vect199, double %63, i32 7
  %temp.vect201 = insertelement <16 x double> %temp.vect200, double %64, i32 8
  %temp.vect202 = insertelement <16 x double> %temp.vect201, double %65, i32 9
  %temp.vect203 = insertelement <16 x double> %temp.vect202, double %66, i32 10
  %temp.vect204 = insertelement <16 x double> %temp.vect203, double %67, i32 11
  %temp.vect205 = insertelement <16 x double> %temp.vect204, double %68, i32 12
  %temp.vect206 = insertelement <16 x double> %temp.vect205, double %69, i32 13
  %temp.vect207 = insertelement <16 x double> %temp.vect206, double %70, i32 14
  %temp.vect208 = insertelement <16 x double> %temp.vect207, double %71, i32 15
  %72 = extractelement <4 x double> %24, i32 2
  %73 = extractelement <4 x double> %25, i32 2
  %74 = extractelement <4 x double> %26, i32 2
  %75 = extractelement <4 x double> %27, i32 2
  %76 = extractelement <4 x double> %28, i32 2
  %77 = extractelement <4 x double> %29, i32 2
  %78 = extractelement <4 x double> %30, i32 2
  %79 = extractelement <4 x double> %31, i32 2
  %80 = extractelement <4 x double> %32, i32 2
  %81 = extractelement <4 x double> %33, i32 2
  %82 = extractelement <4 x double> %34, i32 2
  %83 = extractelement <4 x double> %35, i32 2
  %84 = extractelement <4 x double> %36, i32 2
  %85 = extractelement <4 x double> %37, i32 2
  %86 = extractelement <4 x double> %38, i32 2
  %87 = extractelement <4 x double> %39, i32 2
  %temp.vect225 = insertelement <16 x double> undef, double %72, i32 0
  %temp.vect226 = insertelement <16 x double> %temp.vect225, double %73, i32 1
  %temp.vect227 = insertelement <16 x double> %temp.vect226, double %74, i32 2
  %temp.vect228 = insertelement <16 x double> %temp.vect227, double %75, i32 3
  %temp.vect229 = insertelement <16 x double> %temp.vect228, double %76, i32 4
  %temp.vect230 = insertelement <16 x double> %temp.vect229, double %77, i32 5
  %temp.vect231 = insertelement <16 x double> %temp.vect230, double %78, i32 6
  %temp.vect232 = insertelement <16 x double> %temp.vect231, double %79, i32 7
  %temp.vect233 = insertelement <16 x double> %temp.vect232, double %80, i32 8
  %temp.vect234 = insertelement <16 x double> %temp.vect233, double %81, i32 9
  %temp.vect235 = insertelement <16 x double> %temp.vect234, double %82, i32 10
  %temp.vect236 = insertelement <16 x double> %temp.vect235, double %83, i32 11
  %temp.vect237 = insertelement <16 x double> %temp.vect236, double %84, i32 12
  %temp.vect238 = insertelement <16 x double> %temp.vect237, double %85, i32 13
  %temp.vect239 = insertelement <16 x double> %temp.vect238, double %86, i32 14
  %temp.vect240 = insertelement <16 x double> %temp.vect239, double %87, i32 15
  %tmp381 = trunc <16 x i64> %6 to <16 x i32>
  br i1 %0, label %._crit_edge, label %.preheader

.preheader:                                       ; preds = %SyncBB, %postload479
  %vectorPHI82 = phi <16 x i1> [ %loop_mask33280, %postload479 ], [ %vector, %SyncBB ]
  %vectorPHI83 = phi <16 x double> [ %out_sel275, %postload479 ], [ undef, %SyncBB ]
  %vectorPHI84 = phi <16 x double> [ %out_sel55273, %postload479 ], [ undef, %SyncBB ]
  %vectorPHI85 = phi <16 x double> [ %out_sel58271, %postload479 ], [ undef, %SyncBB ]
  %vectorPHI86 = phi <16 x double> [ %out_sel61269, %postload479 ], [ undef, %SyncBB ]
  %vectorPHI87 = phi <16 x i1> [ %local_edge284, %postload479 ], [ %vector89, %SyncBB ]
  %j.02 = phi i32 [ %222, %postload479 ], [ 0, %SyncBB ]
  %vectorPHI90 = phi <16 x double> [ %merge274, %postload479 ], [ zeroinitializer, %SyncBB ]
  %vectorPHI91 = phi <16 x double> [ %merge42272, %postload479 ], [ zeroinitializer, %SyncBB ]
  %vectorPHI92 = phi <16 x double> [ %merge44270, %postload479 ], [ zeroinitializer, %SyncBB ]
  %extract113 = extractelement <16 x i1> %vectorPHI87, i32 0
  %extract114 = extractelement <16 x i1> %vectorPHI87, i32 1
  %extract115 = extractelement <16 x i1> %vectorPHI87, i32 2
  %extract116 = extractelement <16 x i1> %vectorPHI87, i32 3
  %extract117 = extractelement <16 x i1> %vectorPHI87, i32 4
  %extract118 = extractelement <16 x i1> %vectorPHI87, i32 5
  %extract119 = extractelement <16 x i1> %vectorPHI87, i32 6
  %extract120 = extractelement <16 x i1> %vectorPHI87, i32 7
  %extract121 = extractelement <16 x i1> %vectorPHI87, i32 8
  %extract122 = extractelement <16 x i1> %vectorPHI87, i32 9
  %extract123 = extractelement <16 x i1> %vectorPHI87, i32 10
  %extract124 = extractelement <16 x i1> %vectorPHI87, i32 11
  %extract125 = extractelement <16 x i1> %vectorPHI87, i32 12
  %extract126 = extractelement <16 x i1> %vectorPHI87, i32 13
  %extract127 = extractelement <16 x i1> %vectorPHI87, i32 14
  %extract128 = extractelement <16 x i1> %vectorPHI87, i32 15
  %tmp = mul i32 %j.02, %inum
  %temp94 = insertelement <16 x i32> undef, i32 %tmp, i32 0
  %vector95 = shufflevector <16 x i32> %temp94, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp496 = add <16 x i32> %tmp381, %vector95
  %88 = extractelement <16 x i32> %tmp496, i32 1
  %89 = zext i32 %88 to i64
  %90 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %89
  %91 = extractelement <16 x i32> %tmp496, i32 2
  %92 = zext i32 %91 to i64
  %93 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %92
  %94 = extractelement <16 x i32> %tmp496, i32 3
  %95 = zext i32 %94 to i64
  %96 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %95
  %97 = extractelement <16 x i32> %tmp496, i32 4
  %98 = zext i32 %97 to i64
  %99 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %98
  %100 = extractelement <16 x i32> %tmp496, i32 5
  %101 = zext i32 %100 to i64
  %102 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %101
  %103 = extractelement <16 x i32> %tmp496, i32 6
  %104 = zext i32 %103 to i64
  %105 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %104
  %106 = extractelement <16 x i32> %tmp496, i32 7
  %107 = zext i32 %106 to i64
  %108 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %107
  %109 = extractelement <16 x i32> %tmp496, i32 8
  %110 = zext i32 %109 to i64
  %111 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %110
  %112 = extractelement <16 x i32> %tmp496, i32 9
  %113 = zext i32 %112 to i64
  %114 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %113
  %115 = extractelement <16 x i32> %tmp496, i32 10
  %116 = zext i32 %115 to i64
  %117 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %116
  %118 = extractelement <16 x i32> %tmp496, i32 11
  %119 = zext i32 %118 to i64
  %120 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %119
  %121 = extractelement <16 x i32> %tmp496, i32 12
  %122 = zext i32 %121 to i64
  %123 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %122
  %124 = extractelement <16 x i32> %tmp496, i32 13
  %125 = zext i32 %124 to i64
  %126 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %125
  %127 = extractelement <16 x i32> %tmp496, i32 14
  %128 = zext i32 %127 to i64
  %129 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %128
  %130 = extractelement <16 x i32> %tmp496, i32 15
  %131 = zext i32 %130 to i64
  %132 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %131
  br i1 %extract113, label %preload, label %postload

preload:                                          ; preds = %.preheader
  %133 = extractelement <16 x i32> %tmp496, i32 0
  %134 = zext i32 %133 to i64
  %135 = getelementptr inbounds i32 addrspace(1)* %neighList, i64 %134
  %masked_load = load i32 addrspace(1)* %135, align 4
  %phitmp15 = sext i32 %masked_load to i64
  br label %postload

postload:                                         ; preds = %preload, %.preheader
  %phi = phi i64 [ 0, %.preheader ], [ %phitmp15, %preload ]
  br i1 %extract114, label %preload391, label %postload392

preload391:                                       ; preds = %postload
  %masked_load357 = load i32 addrspace(1)* %90, align 4
  %phitmp = sext i32 %masked_load357 to i64
  br label %postload392

postload392:                                      ; preds = %preload391, %postload
  %phi393 = phi i64 [ 0, %postload ], [ %phitmp, %preload391 ]
  br i1 %extract115, label %preload397, label %postload398

preload397:                                       ; preds = %postload392
  %masked_load358 = load i32 addrspace(1)* %93, align 4
  %phitmp1 = sext i32 %masked_load358 to i64
  br label %postload398

postload398:                                      ; preds = %preload397, %postload392
  %phi399 = phi i64 [ 0, %postload392 ], [ %phitmp1, %preload397 ]
  br i1 %extract116, label %preload403, label %postload404

preload403:                                       ; preds = %postload398
  %masked_load359 = load i32 addrspace(1)* %96, align 4
  %phitmp2 = sext i32 %masked_load359 to i64
  br label %postload404

postload404:                                      ; preds = %preload403, %postload398
  %phi405 = phi i64 [ 0, %postload398 ], [ %phitmp2, %preload403 ]
  br i1 %extract117, label %preload409, label %postload410

preload409:                                       ; preds = %postload404
  %masked_load360 = load i32 addrspace(1)* %99, align 4
  %phitmp3 = sext i32 %masked_load360 to i64
  br label %postload410

postload410:                                      ; preds = %preload409, %postload404
  %phi411 = phi i64 [ 0, %postload404 ], [ %phitmp3, %preload409 ]
  br i1 %extract118, label %preload415, label %postload416

preload415:                                       ; preds = %postload410
  %masked_load361 = load i32 addrspace(1)* %102, align 4
  %phitmp4 = sext i32 %masked_load361 to i64
  br label %postload416

postload416:                                      ; preds = %preload415, %postload410
  %phi417 = phi i64 [ 0, %postload410 ], [ %phitmp4, %preload415 ]
  br i1 %extract119, label %preload421, label %postload422

preload421:                                       ; preds = %postload416
  %masked_load362 = load i32 addrspace(1)* %105, align 4
  %phitmp5 = sext i32 %masked_load362 to i64
  br label %postload422

postload422:                                      ; preds = %preload421, %postload416
  %phi423 = phi i64 [ 0, %postload416 ], [ %phitmp5, %preload421 ]
  br i1 %extract120, label %preload427, label %postload428

preload427:                                       ; preds = %postload422
  %masked_load363 = load i32 addrspace(1)* %108, align 4
  %phitmp6 = sext i32 %masked_load363 to i64
  br label %postload428

postload428:                                      ; preds = %preload427, %postload422
  %phi429 = phi i64 [ 0, %postload422 ], [ %phitmp6, %preload427 ]
  br i1 %extract121, label %preload433, label %postload434

preload433:                                       ; preds = %postload428
  %masked_load364 = load i32 addrspace(1)* %111, align 4
  %phitmp7 = sext i32 %masked_load364 to i64
  br label %postload434

postload434:                                      ; preds = %preload433, %postload428
  %phi435 = phi i64 [ 0, %postload428 ], [ %phitmp7, %preload433 ]
  br i1 %extract122, label %preload439, label %postload440

preload439:                                       ; preds = %postload434
  %masked_load365 = load i32 addrspace(1)* %114, align 4
  %phitmp8 = sext i32 %masked_load365 to i64
  br label %postload440

postload440:                                      ; preds = %preload439, %postload434
  %phi441 = phi i64 [ 0, %postload434 ], [ %phitmp8, %preload439 ]
  br i1 %extract123, label %preload445, label %postload446

preload445:                                       ; preds = %postload440
  %masked_load366 = load i32 addrspace(1)* %117, align 4
  %phitmp9 = sext i32 %masked_load366 to i64
  br label %postload446

postload446:                                      ; preds = %preload445, %postload440
  %phi447 = phi i64 [ 0, %postload440 ], [ %phitmp9, %preload445 ]
  br i1 %extract124, label %preload451, label %postload452

preload451:                                       ; preds = %postload446
  %masked_load367 = load i32 addrspace(1)* %120, align 4
  %phitmp10 = sext i32 %masked_load367 to i64
  br label %postload452

postload452:                                      ; preds = %preload451, %postload446
  %phi453 = phi i64 [ 0, %postload446 ], [ %phitmp10, %preload451 ]
  br i1 %extract125, label %preload457, label %postload458

preload457:                                       ; preds = %postload452
  %masked_load368 = load i32 addrspace(1)* %123, align 4
  %phitmp11 = sext i32 %masked_load368 to i64
  br label %postload458

postload458:                                      ; preds = %preload457, %postload452
  %phi459 = phi i64 [ 0, %postload452 ], [ %phitmp11, %preload457 ]
  br i1 %extract126, label %preload463, label %postload464

preload463:                                       ; preds = %postload458
  %masked_load369 = load i32 addrspace(1)* %126, align 4
  %phitmp12 = sext i32 %masked_load369 to i64
  br label %postload464

postload464:                                      ; preds = %preload463, %postload458
  %phi465 = phi i64 [ 0, %postload458 ], [ %phitmp12, %preload463 ]
  br i1 %extract127, label %preload469, label %postload470

preload469:                                       ; preds = %postload464
  %masked_load370 = load i32 addrspace(1)* %129, align 4
  %phitmp13 = sext i32 %masked_load370 to i64
  br label %postload470

postload470:                                      ; preds = %preload469, %postload464
  %phi471 = phi i64 [ 0, %postload464 ], [ %phitmp13, %preload469 ]
  br i1 %extract128, label %preload475, label %postload476

preload475:                                       ; preds = %postload470
  %masked_load371 = load i32 addrspace(1)* %132, align 4
  %phitmp14 = sext i32 %masked_load371 to i64
  br label %postload476

postload476:                                      ; preds = %preload475, %postload470
  %phi477 = phi i64 [ 0, %postload470 ], [ %phitmp14, %preload475 ]
  %136 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi393
  %137 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi399
  %138 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi405
  %139 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi411
  %140 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi417
  %141 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi423
  %142 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi429
  %143 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi435
  %144 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi441
  %145 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi447
  %146 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi453
  %147 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi459
  %148 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi465
  %149 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi471
  %150 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi477
  br i1 %extract113, label %preload388, label %postload389

preload388:                                       ; preds = %postload476
  %151 = getelementptr inbounds <4 x double> addrspace(1)* %position, i64 %phi
  %masked_load372 = load <4 x double> addrspace(1)* %151, align 32
  br label %postload389

postload389:                                      ; preds = %preload388, %postload476
  %phi390 = phi <4 x double> [ undef, %postload476 ], [ %masked_load372, %preload388 ]
  br i1 %extract114, label %preload394, label %postload395

preload394:                                       ; preds = %postload389
  %masked_load373 = load <4 x double> addrspace(1)* %136, align 32
  br label %postload395

postload395:                                      ; preds = %preload394, %postload389
  %phi396 = phi <4 x double> [ undef, %postload389 ], [ %masked_load373, %preload394 ]
  br i1 %extract115, label %preload400, label %postload401

preload400:                                       ; preds = %postload395
  %masked_load374 = load <4 x double> addrspace(1)* %137, align 32
  br label %postload401

postload401:                                      ; preds = %preload400, %postload395
  %phi402 = phi <4 x double> [ undef, %postload395 ], [ %masked_load374, %preload400 ]
  br i1 %extract116, label %preload406, label %postload407

preload406:                                       ; preds = %postload401
  %masked_load375 = load <4 x double> addrspace(1)* %138, align 32
  br label %postload407

postload407:                                      ; preds = %preload406, %postload401
  %phi408 = phi <4 x double> [ undef, %postload401 ], [ %masked_load375, %preload406 ]
  br i1 %extract117, label %preload412, label %postload413

preload412:                                       ; preds = %postload407
  %masked_load376 = load <4 x double> addrspace(1)* %139, align 32
  br label %postload413

postload413:                                      ; preds = %preload412, %postload407
  %phi414 = phi <4 x double> [ undef, %postload407 ], [ %masked_load376, %preload412 ]
  br i1 %extract118, label %preload418, label %postload419

preload418:                                       ; preds = %postload413
  %masked_load377 = load <4 x double> addrspace(1)* %140, align 32
  br label %postload419

postload419:                                      ; preds = %preload418, %postload413
  %phi420 = phi <4 x double> [ undef, %postload413 ], [ %masked_load377, %preload418 ]
  br i1 %extract119, label %preload424, label %postload425

preload424:                                       ; preds = %postload419
  %masked_load378 = load <4 x double> addrspace(1)* %141, align 32
  br label %postload425

postload425:                                      ; preds = %preload424, %postload419
  %phi426 = phi <4 x double> [ undef, %postload419 ], [ %masked_load378, %preload424 ]
  br i1 %extract120, label %preload430, label %postload431

preload430:                                       ; preds = %postload425
  %masked_load379 = load <4 x double> addrspace(1)* %142, align 32
  br label %postload431

postload431:                                      ; preds = %preload430, %postload425
  %phi432 = phi <4 x double> [ undef, %postload425 ], [ %masked_load379, %preload430 ]
  br i1 %extract121, label %preload436, label %postload437

preload436:                                       ; preds = %postload431
  %masked_load380 = load <4 x double> addrspace(1)* %143, align 32
  br label %postload437

postload437:                                      ; preds = %preload436, %postload431
  %phi438 = phi <4 x double> [ undef, %postload431 ], [ %masked_load380, %preload436 ]
  br i1 %extract122, label %preload442, label %postload443

preload442:                                       ; preds = %postload437
  %masked_load381 = load <4 x double> addrspace(1)* %144, align 32
  br label %postload443

postload443:                                      ; preds = %preload442, %postload437
  %phi444 = phi <4 x double> [ undef, %postload437 ], [ %masked_load381, %preload442 ]
  br i1 %extract123, label %preload448, label %postload449

preload448:                                       ; preds = %postload443
  %masked_load382 = load <4 x double> addrspace(1)* %145, align 32
  br label %postload449

postload449:                                      ; preds = %preload448, %postload443
  %phi450 = phi <4 x double> [ undef, %postload443 ], [ %masked_load382, %preload448 ]
  br i1 %extract124, label %preload454, label %postload455

preload454:                                       ; preds = %postload449
  %masked_load383 = load <4 x double> addrspace(1)* %146, align 32
  br label %postload455

postload455:                                      ; preds = %preload454, %postload449
  %phi456 = phi <4 x double> [ undef, %postload449 ], [ %masked_load383, %preload454 ]
  br i1 %extract125, label %preload460, label %postload461

preload460:                                       ; preds = %postload455
  %masked_load384 = load <4 x double> addrspace(1)* %147, align 32
  br label %postload461

postload461:                                      ; preds = %preload460, %postload455
  %phi462 = phi <4 x double> [ undef, %postload455 ], [ %masked_load384, %preload460 ]
  br i1 %extract126, label %preload466, label %postload467

preload466:                                       ; preds = %postload461
  %masked_load385 = load <4 x double> addrspace(1)* %148, align 32
  br label %postload467

postload467:                                      ; preds = %preload466, %postload461
  %phi468 = phi <4 x double> [ undef, %postload461 ], [ %masked_load385, %preload466 ]
  br i1 %extract127, label %preload472, label %postload473

preload472:                                       ; preds = %postload467
  %masked_load386 = load <4 x double> addrspace(1)* %149, align 32
  br label %postload473

postload473:                                      ; preds = %preload472, %postload467
  %phi474 = phi <4 x double> [ undef, %postload467 ], [ %masked_load386, %preload472 ]
  br i1 %extract128, label %preload478, label %postload479

preload478:                                       ; preds = %postload473
  %masked_load387 = load <4 x double> addrspace(1)* %150, align 32
  br label %postload479

postload479:                                      ; preds = %preload478, %postload473
  %phi480 = phi <4 x double> [ undef, %postload473 ], [ %masked_load387, %preload478 ]
  %152 = extractelement <4 x double> %phi390, i32 0
  %153 = extractelement <4 x double> %phi396, i32 0
  %154 = extractelement <4 x double> %phi402, i32 0
  %155 = extractelement <4 x double> %phi408, i32 0
  %156 = extractelement <4 x double> %phi414, i32 0
  %157 = extractelement <4 x double> %phi420, i32 0
  %158 = extractelement <4 x double> %phi426, i32 0
  %159 = extractelement <4 x double> %phi432, i32 0
  %160 = extractelement <4 x double> %phi438, i32 0
  %161 = extractelement <4 x double> %phi444, i32 0
  %162 = extractelement <4 x double> %phi450, i32 0
  %163 = extractelement <4 x double> %phi456, i32 0
  %164 = extractelement <4 x double> %phi462, i32 0
  %165 = extractelement <4 x double> %phi468, i32 0
  %166 = extractelement <4 x double> %phi474, i32 0
  %167 = extractelement <4 x double> %phi480, i32 0
  %temp.vect177 = insertelement <16 x double> undef, double %152, i32 0
  %temp.vect178 = insertelement <16 x double> %temp.vect177, double %153, i32 1
  %temp.vect179 = insertelement <16 x double> %temp.vect178, double %154, i32 2
  %temp.vect180 = insertelement <16 x double> %temp.vect179, double %155, i32 3
  %temp.vect181 = insertelement <16 x double> %temp.vect180, double %156, i32 4
  %temp.vect182 = insertelement <16 x double> %temp.vect181, double %157, i32 5
  %temp.vect183 = insertelement <16 x double> %temp.vect182, double %158, i32 6
  %temp.vect184 = insertelement <16 x double> %temp.vect183, double %159, i32 7
  %temp.vect185 = insertelement <16 x double> %temp.vect184, double %160, i32 8
  %temp.vect186 = insertelement <16 x double> %temp.vect185, double %161, i32 9
  %temp.vect187 = insertelement <16 x double> %temp.vect186, double %162, i32 10
  %temp.vect188 = insertelement <16 x double> %temp.vect187, double %163, i32 11
  %temp.vect189 = insertelement <16 x double> %temp.vect188, double %164, i32 12
  %temp.vect190 = insertelement <16 x double> %temp.vect189, double %165, i32 13
  %temp.vect191 = insertelement <16 x double> %temp.vect190, double %166, i32 14
  %temp.vect192 = insertelement <16 x double> %temp.vect191, double %167, i32 15
  %168 = extractelement <4 x double> %phi390, i32 1
  %169 = extractelement <4 x double> %phi396, i32 1
  %170 = extractelement <4 x double> %phi402, i32 1
  %171 = extractelement <4 x double> %phi408, i32 1
  %172 = extractelement <4 x double> %phi414, i32 1
  %173 = extractelement <4 x double> %phi420, i32 1
  %174 = extractelement <4 x double> %phi426, i32 1
  %175 = extractelement <4 x double> %phi432, i32 1
  %176 = extractelement <4 x double> %phi438, i32 1
  %177 = extractelement <4 x double> %phi444, i32 1
  %178 = extractelement <4 x double> %phi450, i32 1
  %179 = extractelement <4 x double> %phi456, i32 1
  %180 = extractelement <4 x double> %phi462, i32 1
  %181 = extractelement <4 x double> %phi468, i32 1
  %182 = extractelement <4 x double> %phi474, i32 1
  %183 = extractelement <4 x double> %phi480, i32 1
  %temp.vect209 = insertelement <16 x double> undef, double %168, i32 0
  %temp.vect210 = insertelement <16 x double> %temp.vect209, double %169, i32 1
  %temp.vect211 = insertelement <16 x double> %temp.vect210, double %170, i32 2
  %temp.vect212 = insertelement <16 x double> %temp.vect211, double %171, i32 3
  %temp.vect213 = insertelement <16 x double> %temp.vect212, double %172, i32 4
  %temp.vect214 = insertelement <16 x double> %temp.vect213, double %173, i32 5
  %temp.vect215 = insertelement <16 x double> %temp.vect214, double %174, i32 6
  %temp.vect216 = insertelement <16 x double> %temp.vect215, double %175, i32 7
  %temp.vect217 = insertelement <16 x double> %temp.vect216, double %176, i32 8
  %temp.vect218 = insertelement <16 x double> %temp.vect217, double %177, i32 9
  %temp.vect219 = insertelement <16 x double> %temp.vect218, double %178, i32 10
  %temp.vect220 = insertelement <16 x double> %temp.vect219, double %179, i32 11
  %temp.vect221 = insertelement <16 x double> %temp.vect220, double %180, i32 12
  %temp.vect222 = insertelement <16 x double> %temp.vect221, double %181, i32 13
  %temp.vect223 = insertelement <16 x double> %temp.vect222, double %182, i32 14
  %temp.vect224 = insertelement <16 x double> %temp.vect223, double %183, i32 15
  %184 = extractelement <4 x double> %phi390, i32 2
  %185 = extractelement <4 x double> %phi396, i32 2
  %186 = extractelement <4 x double> %phi402, i32 2
  %187 = extractelement <4 x double> %phi408, i32 2
  %188 = extractelement <4 x double> %phi414, i32 2
  %189 = extractelement <4 x double> %phi420, i32 2
  %190 = extractelement <4 x double> %phi426, i32 2
  %191 = extractelement <4 x double> %phi432, i32 2
  %192 = extractelement <4 x double> %phi438, i32 2
  %193 = extractelement <4 x double> %phi444, i32 2
  %194 = extractelement <4 x double> %phi450, i32 2
  %195 = extractelement <4 x double> %phi456, i32 2
  %196 = extractelement <4 x double> %phi462, i32 2
  %197 = extractelement <4 x double> %phi468, i32 2
  %198 = extractelement <4 x double> %phi474, i32 2
  %199 = extractelement <4 x double> %phi480, i32 2
  %temp.vect241 = insertelement <16 x double> undef, double %184, i32 0
  %temp.vect242 = insertelement <16 x double> %temp.vect241, double %185, i32 1
  %temp.vect243 = insertelement <16 x double> %temp.vect242, double %186, i32 2
  %temp.vect244 = insertelement <16 x double> %temp.vect243, double %187, i32 3
  %temp.vect245 = insertelement <16 x double> %temp.vect244, double %188, i32 4
  %temp.vect246 = insertelement <16 x double> %temp.vect245, double %189, i32 5
  %temp.vect247 = insertelement <16 x double> %temp.vect246, double %190, i32 6
  %temp.vect248 = insertelement <16 x double> %temp.vect247, double %191, i32 7
  %temp.vect249 = insertelement <16 x double> %temp.vect248, double %192, i32 8
  %temp.vect250 = insertelement <16 x double> %temp.vect249, double %193, i32 9
  %temp.vect251 = insertelement <16 x double> %temp.vect250, double %194, i32 10
  %temp.vect252 = insertelement <16 x double> %temp.vect251, double %195, i32 11
  %temp.vect253 = insertelement <16 x double> %temp.vect252, double %196, i32 12
  %temp.vect254 = insertelement <16 x double> %temp.vect253, double %197, i32 13
  %temp.vect255 = insertelement <16 x double> %temp.vect254, double %198, i32 14
  %temp.vect256 = insertelement <16 x double> %temp.vect255, double %199, i32 15
  %200 = fsub <16 x double> %temp.vect176, %temp.vect192
  %201 = fsub <16 x double> %temp.vect208, %temp.vect224
  %202 = fsub <16 x double> %temp.vect240, %temp.vect256
  %203 = fmul <16 x double> %200, %200
  %204 = fmul <16 x double> %201, %201
  %205 = fadd <16 x double> %203, %204
  %206 = fmul <16 x double> %202, %202
  %207 = fadd <16 x double> %205, %206
  %208 = fcmp olt <16 x double> %207, %vector258
  %_to_32261 = and <16 x i1> %vectorPHI87, %208
  %209 = fdiv <16 x double> <double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00>, %207
  %210 = fmul <16 x double> %209, %209
  %211 = fmul <16 x double> %210, %209
  %212 = fmul <16 x double> %209, %211
  %213 = fmul <16 x double> %211, %vector263
  %214 = fsub <16 x double> %213, %vector265
  %215 = fmul <16 x double> %212, %214
  %216 = fmul <16 x double> %202, %215
  %217 = fmul <16 x double> %201, %215
  %218 = fmul <16 x double> %200, %215
  %219 = fadd <16 x double> %vectorPHI92, %216
  %220 = fadd <16 x double> %vectorPHI91, %217
  %221 = fadd <16 x double> %vectorPHI90, %218
  %out_sel61269 = select <16 x i1> %vectorPHI87, <16 x double> zeroinitializer, <16 x double> %vectorPHI86
  %merge44270 = select <16 x i1> %_to_32261, <16 x double> %219, <16 x double> %vectorPHI92
  %out_sel58271 = select <16 x i1> %vectorPHI87, <16 x double> %merge44270, <16 x double> %vectorPHI85
  %merge42272 = select <16 x i1> %_to_32261, <16 x double> %220, <16 x double> %vectorPHI91
  %out_sel55273 = select <16 x i1> %vectorPHI87, <16 x double> %merge42272, <16 x double> %vectorPHI84
  %merge274 = select <16 x i1> %_to_32261, <16 x double> %221, <16 x double> %vectorPHI90
  %out_sel275 = select <16 x i1> %vectorPHI87, <16 x double> %merge274, <16 x double> %vectorPHI83
  %222 = add nsw i32 %j.02, 1
  %exitcond = icmp eq i32 %222, %neighCount
  %temp276 = insertelement <16 x i1> undef, i1 %exitcond, i32 0
  %vector277 = shufflevector <16 x i1> %temp276, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond = xor i1 %exitcond, true
  %temp282 = insertelement <16 x i1> undef, i1 %notCond, i32 0
  %vector283 = shufflevector <16 x i1> %temp282, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr278 = and <16 x i1> %vectorPHI87, %vector277
  %loop_mask33280 = or <16 x i1> %vectorPHI82, %who_left_tr278
  %curr_loop_mask281 = or <16 x i1> %loop_mask33280, %who_left_tr278
  %ipred.i = bitcast <16 x i1> %curr_loop_mask281 to i16
  %val.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i, i16 %ipred.i) nounwind
  %tmp.i = and i32 %val.i, 1
  %res.i = icmp eq i32 %tmp.i, 0
  %local_edge284 = and <16 x i1> %vectorPHI87, %vector283
  br i1 %res.i, label %.preheader, label %._crit_edge

._crit_edge:                                      ; preds = %postload479, %SyncBB
  %vectorPHI285 = phi <16 x double> [ undef, %SyncBB ], [ %out_sel61269, %postload479 ]
  %vectorPHI286 = phi <16 x double> [ undef, %SyncBB ], [ %out_sel58271, %postload479 ]
  %vectorPHI287 = phi <16 x double> [ undef, %SyncBB ], [ %out_sel55273, %postload479 ]
  %vectorPHI288 = phi <16 x double> [ undef, %SyncBB ], [ %out_sel275, %postload479 ]
  %merge54289 = select i1 %0, <16 x double> %vectorPHI285, <16 x double> zeroinitializer
  %extract341 = extractelement <16 x double> %merge54289, i32 0
  %extract342 = extractelement <16 x double> %merge54289, i32 1
  %extract343 = extractelement <16 x double> %merge54289, i32 2
  %extract344 = extractelement <16 x double> %merge54289, i32 3
  %extract345 = extractelement <16 x double> %merge54289, i32 4
  %extract346 = extractelement <16 x double> %merge54289, i32 5
  %extract347 = extractelement <16 x double> %merge54289, i32 6
  %extract348 = extractelement <16 x double> %merge54289, i32 7
  %extract349 = extractelement <16 x double> %merge54289, i32 8
  %extract350 = extractelement <16 x double> %merge54289, i32 9
  %extract351 = extractelement <16 x double> %merge54289, i32 10
  %extract352 = extractelement <16 x double> %merge54289, i32 11
  %extract353 = extractelement <16 x double> %merge54289, i32 12
  %extract354 = extractelement <16 x double> %merge54289, i32 13
  %extract355 = extractelement <16 x double> %merge54289, i32 14
  %extract356 = extractelement <16 x double> %merge54289, i32 15
  %merge52290 = select i1 %0, <16 x double> %vectorPHI286, <16 x double> zeroinitializer
  %extract325 = extractelement <16 x double> %merge52290, i32 0
  %extract326 = extractelement <16 x double> %merge52290, i32 1
  %extract327 = extractelement <16 x double> %merge52290, i32 2
  %extract328 = extractelement <16 x double> %merge52290, i32 3
  %extract329 = extractelement <16 x double> %merge52290, i32 4
  %extract330 = extractelement <16 x double> %merge52290, i32 5
  %extract331 = extractelement <16 x double> %merge52290, i32 6
  %extract332 = extractelement <16 x double> %merge52290, i32 7
  %extract333 = extractelement <16 x double> %merge52290, i32 8
  %extract334 = extractelement <16 x double> %merge52290, i32 9
  %extract335 = extractelement <16 x double> %merge52290, i32 10
  %extract336 = extractelement <16 x double> %merge52290, i32 11
  %extract337 = extractelement <16 x double> %merge52290, i32 12
  %extract338 = extractelement <16 x double> %merge52290, i32 13
  %extract339 = extractelement <16 x double> %merge52290, i32 14
  %extract340 = extractelement <16 x double> %merge52290, i32 15
  %merge50291 = select i1 %0, <16 x double> %vectorPHI287, <16 x double> zeroinitializer
  %extract309 = extractelement <16 x double> %merge50291, i32 0
  %extract310 = extractelement <16 x double> %merge50291, i32 1
  %extract311 = extractelement <16 x double> %merge50291, i32 2
  %extract312 = extractelement <16 x double> %merge50291, i32 3
  %extract313 = extractelement <16 x double> %merge50291, i32 4
  %extract314 = extractelement <16 x double> %merge50291, i32 5
  %extract315 = extractelement <16 x double> %merge50291, i32 6
  %extract316 = extractelement <16 x double> %merge50291, i32 7
  %extract317 = extractelement <16 x double> %merge50291, i32 8
  %extract318 = extractelement <16 x double> %merge50291, i32 9
  %extract319 = extractelement <16 x double> %merge50291, i32 10
  %extract320 = extractelement <16 x double> %merge50291, i32 11
  %extract321 = extractelement <16 x double> %merge50291, i32 12
  %extract322 = extractelement <16 x double> %merge50291, i32 13
  %extract323 = extractelement <16 x double> %merge50291, i32 14
  %extract324 = extractelement <16 x double> %merge50291, i32 15
  %merge48292 = select i1 %0, <16 x double> %vectorPHI288, <16 x double> zeroinitializer
  %extract293 = extractelement <16 x double> %merge48292, i32 0
  %extract294 = extractelement <16 x double> %merge48292, i32 1
  %extract295 = extractelement <16 x double> %merge48292, i32 2
  %extract296 = extractelement <16 x double> %merge48292, i32 3
  %extract297 = extractelement <16 x double> %merge48292, i32 4
  %extract298 = extractelement <16 x double> %merge48292, i32 5
  %extract299 = extractelement <16 x double> %merge48292, i32 6
  %extract300 = extractelement <16 x double> %merge48292, i32 7
  %extract301 = extractelement <16 x double> %merge48292, i32 8
  %extract302 = extractelement <16 x double> %merge48292, i32 9
  %extract303 = extractelement <16 x double> %merge48292, i32 10
  %extract304 = extractelement <16 x double> %merge48292, i32 11
  %extract305 = extractelement <16 x double> %merge48292, i32 12
  %extract306 = extractelement <16 x double> %merge48292, i32 13
  %extract307 = extractelement <16 x double> %merge48292, i32 14
  %extract308 = extractelement <16 x double> %merge48292, i32 15
  %223 = insertelement <4 x double> undef, double %extract293, i32 0
  %224 = insertelement <4 x double> undef, double %extract294, i32 0
  %225 = insertelement <4 x double> undef, double %extract295, i32 0
  %226 = insertelement <4 x double> undef, double %extract296, i32 0
  %227 = insertelement <4 x double> undef, double %extract297, i32 0
  %228 = insertelement <4 x double> undef, double %extract298, i32 0
  %229 = insertelement <4 x double> undef, double %extract299, i32 0
  %230 = insertelement <4 x double> undef, double %extract300, i32 0
  %231 = insertelement <4 x double> undef, double %extract301, i32 0
  %232 = insertelement <4 x double> undef, double %extract302, i32 0
  %233 = insertelement <4 x double> undef, double %extract303, i32 0
  %234 = insertelement <4 x double> undef, double %extract304, i32 0
  %235 = insertelement <4 x double> undef, double %extract305, i32 0
  %236 = insertelement <4 x double> undef, double %extract306, i32 0
  %237 = insertelement <4 x double> undef, double %extract307, i32 0
  %238 = insertelement <4 x double> undef, double %extract308, i32 0
  %239 = insertelement <4 x double> %223, double %extract309, i32 1
  %240 = insertelement <4 x double> %224, double %extract310, i32 1
  %241 = insertelement <4 x double> %225, double %extract311, i32 1
  %242 = insertelement <4 x double> %226, double %extract312, i32 1
  %243 = insertelement <4 x double> %227, double %extract313, i32 1
  %244 = insertelement <4 x double> %228, double %extract314, i32 1
  %245 = insertelement <4 x double> %229, double %extract315, i32 1
  %246 = insertelement <4 x double> %230, double %extract316, i32 1
  %247 = insertelement <4 x double> %231, double %extract317, i32 1
  %248 = insertelement <4 x double> %232, double %extract318, i32 1
  %249 = insertelement <4 x double> %233, double %extract319, i32 1
  %250 = insertelement <4 x double> %234, double %extract320, i32 1
  %251 = insertelement <4 x double> %235, double %extract321, i32 1
  %252 = insertelement <4 x double> %236, double %extract322, i32 1
  %253 = insertelement <4 x double> %237, double %extract323, i32 1
  %254 = insertelement <4 x double> %238, double %extract324, i32 1
  %255 = insertelement <4 x double> %239, double %extract325, i32 2
  %256 = insertelement <4 x double> %240, double %extract326, i32 2
  %257 = insertelement <4 x double> %241, double %extract327, i32 2
  %258 = insertelement <4 x double> %242, double %extract328, i32 2
  %259 = insertelement <4 x double> %243, double %extract329, i32 2
  %260 = insertelement <4 x double> %244, double %extract330, i32 2
  %261 = insertelement <4 x double> %245, double %extract331, i32 2
  %262 = insertelement <4 x double> %246, double %extract332, i32 2
  %263 = insertelement <4 x double> %247, double %extract333, i32 2
  %264 = insertelement <4 x double> %248, double %extract334, i32 2
  %265 = insertelement <4 x double> %249, double %extract335, i32 2
  %266 = insertelement <4 x double> %250, double %extract336, i32 2
  %267 = insertelement <4 x double> %251, double %extract337, i32 2
  %268 = insertelement <4 x double> %252, double %extract338, i32 2
  %269 = insertelement <4 x double> %253, double %extract339, i32 2
  %270 = insertelement <4 x double> %254, double %extract340, i32 2
  %271 = insertelement <4 x double> %255, double %extract341, i32 3
  %272 = insertelement <4 x double> %256, double %extract342, i32 3
  %273 = insertelement <4 x double> %257, double %extract343, i32 3
  %274 = insertelement <4 x double> %258, double %extract344, i32 3
  %275 = insertelement <4 x double> %259, double %extract345, i32 3
  %276 = insertelement <4 x double> %260, double %extract346, i32 3
  %277 = insertelement <4 x double> %261, double %extract347, i32 3
  %278 = insertelement <4 x double> %262, double %extract348, i32 3
  %279 = insertelement <4 x double> %263, double %extract349, i32 3
  %280 = insertelement <4 x double> %264, double %extract350, i32 3
  %281 = insertelement <4 x double> %265, double %extract351, i32 3
  %282 = insertelement <4 x double> %266, double %extract352, i32 3
  %283 = insertelement <4 x double> %267, double %extract353, i32 3
  %284 = insertelement <4 x double> %268, double %extract354, i32 3
  %285 = insertelement <4 x double> %269, double %extract355, i32 3
  %286 = insertelement <4 x double> %270, double %extract356, i32 3
  %287 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract
  %288 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract66
  %289 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract67
  %290 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract68
  %291 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract69
  %292 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract70
  %293 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract71
  %294 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract72
  %295 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract73
  %296 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract74
  %297 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract75
  %298 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract76
  %299 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract77
  %300 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract78
  %301 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract79
  %302 = getelementptr inbounds <4 x double> addrspace(1)* %force, i64 %extract80
  store <4 x double> %271, <4 x double> addrspace(1)* %287, align 32
  store <4 x double> %272, <4 x double> addrspace(1)* %288, align 32
  store <4 x double> %273, <4 x double> addrspace(1)* %289, align 32
  store <4 x double> %274, <4 x double> addrspace(1)* %290, align 32
  store <4 x double> %275, <4 x double> addrspace(1)* %291, align 32
  store <4 x double> %276, <4 x double> addrspace(1)* %292, align 32
  store <4 x double> %277, <4 x double> addrspace(1)* %293, align 32
  store <4 x double> %278, <4 x double> addrspace(1)* %294, align 32
  store <4 x double> %279, <4 x double> addrspace(1)* %295, align 32
  store <4 x double> %280, <4 x double> addrspace(1)* %296, align 32
  store <4 x double> %281, <4 x double> addrspace(1)* %297, align 32
  store <4 x double> %282, <4 x double> addrspace(1)* %298, align 32
  store <4 x double> %283, <4 x double> addrspace(1)* %299, align 32
  store <4 x double> %284, <4 x double> addrspace(1)* %300, align 32
  store <4 x double> %285, <4 x double> addrspace(1)* %301, align 32
  store <4 x double> %286, <4 x double> addrspace(1)* %302, align 32
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB482

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB482:                                        ; preds = %._crit_edge
  ret void
}

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

define void @compute_lj_force(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x double> addrspace(1)**
  %1 = load <4 x double> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x double> addrspace(1)**
  %4 = load <4 x double> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double*
  %13 = load double* %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to double*
  %16 = load double* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to double*
  %19 = load double* %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to i32*
  %22 = load i32* %21, align 4
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to %struct.PaddedDimId**
  %25 = load %struct.PaddedDimId** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 96
  %27 = bitcast i8* %26 to %struct.PaddedDimId**
  %28 = load %struct.PaddedDimId** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 112
  %30 = bitcast i8* %29 to i64*
  %31 = load i64* %30, align 8
  %32 = icmp sgt i32 %7, 0
  br label %SyncBB5.i

SyncBB5.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %33 = getelementptr %struct.PaddedDimId* %28, i64 %CurrWI..0.i, i32 0, i64 0
  %34 = load i64* %33, align 8
  %35 = getelementptr %struct.PaddedDimId* %25, i64 0, i32 0, i64 0
  %36 = load i64* %35, align 8
  %37 = add i64 %34, %36
  %38 = and i64 %37, 4294967295
  %39 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %38
  %40 = load <4 x double> addrspace(1)* %39, align 32
  br i1 %32, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB5.i
  %41 = extractelement <4 x double> %40, i32 0
  %42 = extractelement <4 x double> %40, i32 1
  %43 = extractelement <4 x double> %40, i32 2
  %tmp3.i = trunc i64 %37 to i32
  br label %44

; <label>:44                                      ; preds = %83, %bb.nph.i
  %j.02.i = phi i32 [ 0, %bb.nph.i ], [ %84, %83 ]
  %f.11.i = phi <4 x double> [ zeroinitializer, %bb.nph.i ], [ %f.0.i, %83 ]
  %tmp.i = mul i32 %j.02.i, %22
  %tmp4.i = add i32 %tmp3.i, %tmp.i
  %45 = zext i32 %tmp4.i to i64
  %46 = getelementptr inbounds i32 addrspace(1)* %10, i64 %45
  %47 = load i32 addrspace(1)* %46, align 4
  %48 = sext i32 %47 to i64
  %49 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %48
  %50 = load <4 x double> addrspace(1)* %49, align 32
  %51 = extractelement <4 x double> %50, i32 0
  %52 = fsub double %41, %51
  %53 = extractelement <4 x double> %50, i32 1
  %54 = fsub double %42, %53
  %55 = extractelement <4 x double> %50, i32 2
  %56 = fsub double %43, %55
  %57 = fmul double %52, %52
  %58 = fmul double %54, %54
  %59 = fadd double %57, %58
  %60 = fmul double %56, %56
  %61 = fadd double %59, %60
  %62 = fcmp olt double %61, %13
  br i1 %62, label %63, label %83

; <label>:63                                      ; preds = %44
  %64 = fdiv double 1.000000e+00, %61
  %65 = fmul double %64, %64
  %66 = fmul double %65, %64
  %67 = fmul double %64, %66
  %68 = fmul double %66, %16
  %69 = fsub double %68, %19
  %70 = fmul double %67, %69
  %71 = fmul double %52, %70
  %72 = extractelement <4 x double> %f.11.i, i32 0
  %73 = fadd double %72, %71
  %74 = insertelement <4 x double> %f.11.i, double %73, i32 0
  %75 = fmul double %54, %70
  %76 = extractelement <4 x double> %f.11.i, i32 1
  %77 = fadd double %76, %75
  %78 = insertelement <4 x double> %74, double %77, i32 1
  %79 = fmul double %56, %70
  %80 = extractelement <4 x double> %f.11.i, i32 2
  %81 = fadd double %80, %79
  %82 = insertelement <4 x double> %78, double %81, i32 2
  br label %83

; <label>:83                                      ; preds = %63, %44
  %f.0.i = phi <4 x double> [ %82, %63 ], [ %f.11.i, %44 ]
  %84 = add nsw i32 %j.02.i, 1
  %exitcond.i = icmp eq i32 %84, %7
  br i1 %exitcond.i, label %._crit_edge.i, label %44

._crit_edge.i:                                    ; preds = %83, %SyncBB5.i
  %f.1.lcssa.i = phi <4 x double> [ zeroinitializer, %SyncBB5.i ], [ %f.0.i, %83 ]
  %85 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %38
  store <4 x double> %f.1.lcssa.i, <4 x double> addrspace(1)* %85, align 32
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %31
  br i1 %check.WI.iter.i, label %thenBB.i, label %__compute_lj_force_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB5.i

__compute_lj_force_separated_args.exit:           ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.compute_lj_force(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x double> addrspace(1)**
  %1 = load <4 x double> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x double> addrspace(1)**
  %4 = load <4 x double> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double*
  %13 = load double* %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to double*
  %16 = load double* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to double*
  %19 = load double* %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to i32*
  %22 = load i32* %21, align 4
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to %struct.PaddedDimId**
  %25 = load %struct.PaddedDimId** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 96
  %27 = bitcast i8* %26 to %struct.PaddedDimId**
  %28 = load %struct.PaddedDimId** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 112
  %30 = bitcast i8* %29 to i64*
  %31 = load i64* %30, align 8
  %temp264.i = insertelement <16 x double> undef, double %19, i32 0
  %vector265.i = shufflevector <16 x double> %temp264.i, <16 x double> undef, <16 x i32> zeroinitializer
  %temp262.i = insertelement <16 x double> undef, double %16, i32 0
  %vector263.i = shufflevector <16 x double> %temp262.i, <16 x double> undef, <16 x i32> zeroinitializer
  %temp257.i = insertelement <16 x double> undef, double %13, i32 0
  %vector258.i = shufflevector <16 x double> %temp257.i, <16 x double> undef, <16 x i32> zeroinitializer
  %32 = icmp sgt i32 %7, 0
  %temp88.i = insertelement <16 x i1> undef, i1 %32, i32 0
  %vector89.i = shufflevector <16 x i1> %temp88.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %negIncomingLoopMask.i = xor i1 %32, true
  %temp.i = insertelement <16 x i1> undef, i1 %negIncomingLoopMask.i, i32 0
  %vector.i = shufflevector <16 x i1> %temp.i, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %33 = getelementptr %struct.PaddedDimId* %28, i64 %CurrWI..0.i, i32 0, i64 0
  %34 = load i64* %33, align 8
  %35 = getelementptr %struct.PaddedDimId* %25, i64 0, i32 0, i64 0
  %36 = load i64* %35, align 8
  %37 = add i64 %34, %36
  %broadcast1.i = insertelement <16 x i64> undef, i64 %37, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %38 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %39 = and <16 x i64> %38, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract.i = extractelement <16 x i64> %39, i32 0
  %extract66.i = extractelement <16 x i64> %39, i32 1
  %extract67.i = extractelement <16 x i64> %39, i32 2
  %extract68.i = extractelement <16 x i64> %39, i32 3
  %extract69.i = extractelement <16 x i64> %39, i32 4
  %extract70.i = extractelement <16 x i64> %39, i32 5
  %extract71.i = extractelement <16 x i64> %39, i32 6
  %extract72.i = extractelement <16 x i64> %39, i32 7
  %extract73.i = extractelement <16 x i64> %39, i32 8
  %extract74.i = extractelement <16 x i64> %39, i32 9
  %extract75.i = extractelement <16 x i64> %39, i32 10
  %extract76.i = extractelement <16 x i64> %39, i32 11
  %extract77.i = extractelement <16 x i64> %39, i32 12
  %extract78.i = extractelement <16 x i64> %39, i32 13
  %extract79.i = extractelement <16 x i64> %39, i32 14
  %extract80.i = extractelement <16 x i64> %39, i32 15
  %40 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract.i
  %41 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract66.i
  %42 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract67.i
  %43 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract68.i
  %44 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract69.i
  %45 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract70.i
  %46 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract71.i
  %47 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract72.i
  %48 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract73.i
  %49 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract74.i
  %50 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract75.i
  %51 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract76.i
  %52 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract77.i
  %53 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract78.i
  %54 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract79.i
  %55 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %extract80.i
  %56 = load <4 x double> addrspace(1)* %40, align 32
  %57 = load <4 x double> addrspace(1)* %41, align 32
  %58 = load <4 x double> addrspace(1)* %42, align 32
  %59 = load <4 x double> addrspace(1)* %43, align 32
  %60 = load <4 x double> addrspace(1)* %44, align 32
  %61 = load <4 x double> addrspace(1)* %45, align 32
  %62 = load <4 x double> addrspace(1)* %46, align 32
  %63 = load <4 x double> addrspace(1)* %47, align 32
  %64 = load <4 x double> addrspace(1)* %48, align 32
  %65 = load <4 x double> addrspace(1)* %49, align 32
  %66 = load <4 x double> addrspace(1)* %50, align 32
  %67 = load <4 x double> addrspace(1)* %51, align 32
  %68 = load <4 x double> addrspace(1)* %52, align 32
  %69 = load <4 x double> addrspace(1)* %53, align 32
  %70 = load <4 x double> addrspace(1)* %54, align 32
  %71 = load <4 x double> addrspace(1)* %55, align 32
  %72 = extractelement <4 x double> %56, i32 0
  %73 = extractelement <4 x double> %57, i32 0
  %74 = extractelement <4 x double> %58, i32 0
  %75 = extractelement <4 x double> %59, i32 0
  %76 = extractelement <4 x double> %60, i32 0
  %77 = extractelement <4 x double> %61, i32 0
  %78 = extractelement <4 x double> %62, i32 0
  %79 = extractelement <4 x double> %63, i32 0
  %80 = extractelement <4 x double> %64, i32 0
  %81 = extractelement <4 x double> %65, i32 0
  %82 = extractelement <4 x double> %66, i32 0
  %83 = extractelement <4 x double> %67, i32 0
  %84 = extractelement <4 x double> %68, i32 0
  %85 = extractelement <4 x double> %69, i32 0
  %86 = extractelement <4 x double> %70, i32 0
  %87 = extractelement <4 x double> %71, i32 0
  %temp.vect161.i = insertelement <16 x double> undef, double %72, i32 0
  %temp.vect162.i = insertelement <16 x double> %temp.vect161.i, double %73, i32 1
  %temp.vect163.i = insertelement <16 x double> %temp.vect162.i, double %74, i32 2
  %temp.vect164.i = insertelement <16 x double> %temp.vect163.i, double %75, i32 3
  %temp.vect165.i = insertelement <16 x double> %temp.vect164.i, double %76, i32 4
  %temp.vect166.i = insertelement <16 x double> %temp.vect165.i, double %77, i32 5
  %temp.vect167.i = insertelement <16 x double> %temp.vect166.i, double %78, i32 6
  %temp.vect168.i = insertelement <16 x double> %temp.vect167.i, double %79, i32 7
  %temp.vect169.i = insertelement <16 x double> %temp.vect168.i, double %80, i32 8
  %temp.vect170.i = insertelement <16 x double> %temp.vect169.i, double %81, i32 9
  %temp.vect171.i = insertelement <16 x double> %temp.vect170.i, double %82, i32 10
  %temp.vect172.i = insertelement <16 x double> %temp.vect171.i, double %83, i32 11
  %temp.vect173.i = insertelement <16 x double> %temp.vect172.i, double %84, i32 12
  %temp.vect174.i = insertelement <16 x double> %temp.vect173.i, double %85, i32 13
  %temp.vect175.i = insertelement <16 x double> %temp.vect174.i, double %86, i32 14
  %temp.vect176.i = insertelement <16 x double> %temp.vect175.i, double %87, i32 15
  %88 = extractelement <4 x double> %56, i32 1
  %89 = extractelement <4 x double> %57, i32 1
  %90 = extractelement <4 x double> %58, i32 1
  %91 = extractelement <4 x double> %59, i32 1
  %92 = extractelement <4 x double> %60, i32 1
  %93 = extractelement <4 x double> %61, i32 1
  %94 = extractelement <4 x double> %62, i32 1
  %95 = extractelement <4 x double> %63, i32 1
  %96 = extractelement <4 x double> %64, i32 1
  %97 = extractelement <4 x double> %65, i32 1
  %98 = extractelement <4 x double> %66, i32 1
  %99 = extractelement <4 x double> %67, i32 1
  %100 = extractelement <4 x double> %68, i32 1
  %101 = extractelement <4 x double> %69, i32 1
  %102 = extractelement <4 x double> %70, i32 1
  %103 = extractelement <4 x double> %71, i32 1
  %temp.vect193.i = insertelement <16 x double> undef, double %88, i32 0
  %temp.vect194.i = insertelement <16 x double> %temp.vect193.i, double %89, i32 1
  %temp.vect195.i = insertelement <16 x double> %temp.vect194.i, double %90, i32 2
  %temp.vect196.i = insertelement <16 x double> %temp.vect195.i, double %91, i32 3
  %temp.vect197.i = insertelement <16 x double> %temp.vect196.i, double %92, i32 4
  %temp.vect198.i = insertelement <16 x double> %temp.vect197.i, double %93, i32 5
  %temp.vect199.i = insertelement <16 x double> %temp.vect198.i, double %94, i32 6
  %temp.vect200.i = insertelement <16 x double> %temp.vect199.i, double %95, i32 7
  %temp.vect201.i = insertelement <16 x double> %temp.vect200.i, double %96, i32 8
  %temp.vect202.i = insertelement <16 x double> %temp.vect201.i, double %97, i32 9
  %temp.vect203.i = insertelement <16 x double> %temp.vect202.i, double %98, i32 10
  %temp.vect204.i = insertelement <16 x double> %temp.vect203.i, double %99, i32 11
  %temp.vect205.i = insertelement <16 x double> %temp.vect204.i, double %100, i32 12
  %temp.vect206.i = insertelement <16 x double> %temp.vect205.i, double %101, i32 13
  %temp.vect207.i = insertelement <16 x double> %temp.vect206.i, double %102, i32 14
  %temp.vect208.i = insertelement <16 x double> %temp.vect207.i, double %103, i32 15
  %104 = extractelement <4 x double> %56, i32 2
  %105 = extractelement <4 x double> %57, i32 2
  %106 = extractelement <4 x double> %58, i32 2
  %107 = extractelement <4 x double> %59, i32 2
  %108 = extractelement <4 x double> %60, i32 2
  %109 = extractelement <4 x double> %61, i32 2
  %110 = extractelement <4 x double> %62, i32 2
  %111 = extractelement <4 x double> %63, i32 2
  %112 = extractelement <4 x double> %64, i32 2
  %113 = extractelement <4 x double> %65, i32 2
  %114 = extractelement <4 x double> %66, i32 2
  %115 = extractelement <4 x double> %67, i32 2
  %116 = extractelement <4 x double> %68, i32 2
  %117 = extractelement <4 x double> %69, i32 2
  %118 = extractelement <4 x double> %70, i32 2
  %119 = extractelement <4 x double> %71, i32 2
  %temp.vect225.i = insertelement <16 x double> undef, double %104, i32 0
  %temp.vect226.i = insertelement <16 x double> %temp.vect225.i, double %105, i32 1
  %temp.vect227.i = insertelement <16 x double> %temp.vect226.i, double %106, i32 2
  %temp.vect228.i = insertelement <16 x double> %temp.vect227.i, double %107, i32 3
  %temp.vect229.i = insertelement <16 x double> %temp.vect228.i, double %108, i32 4
  %temp.vect230.i = insertelement <16 x double> %temp.vect229.i, double %109, i32 5
  %temp.vect231.i = insertelement <16 x double> %temp.vect230.i, double %110, i32 6
  %temp.vect232.i = insertelement <16 x double> %temp.vect231.i, double %111, i32 7
  %temp.vect233.i = insertelement <16 x double> %temp.vect232.i, double %112, i32 8
  %temp.vect234.i = insertelement <16 x double> %temp.vect233.i, double %113, i32 9
  %temp.vect235.i = insertelement <16 x double> %temp.vect234.i, double %114, i32 10
  %temp.vect236.i = insertelement <16 x double> %temp.vect235.i, double %115, i32 11
  %temp.vect237.i = insertelement <16 x double> %temp.vect236.i, double %116, i32 12
  %temp.vect238.i = insertelement <16 x double> %temp.vect237.i, double %117, i32 13
  %temp.vect239.i = insertelement <16 x double> %temp.vect238.i, double %118, i32 14
  %temp.vect240.i = insertelement <16 x double> %temp.vect239.i, double %119, i32 15
  %tmp381.i = trunc <16 x i64> %38 to <16 x i32>
  br i1 %32, label %._crit_edge.i, label %.preheader.i

.preheader.i:                                     ; preds = %postload479.i, %SyncBB.i
  %vectorPHI82.i = phi <16 x i1> [ %loop_mask33280.i, %postload479.i ], [ %vector.i, %SyncBB.i ]
  %vectorPHI83.i = phi <16 x double> [ %out_sel275.i, %postload479.i ], [ undef, %SyncBB.i ]
  %vectorPHI84.i = phi <16 x double> [ %out_sel55273.i, %postload479.i ], [ undef, %SyncBB.i ]
  %vectorPHI85.i = phi <16 x double> [ %out_sel58271.i, %postload479.i ], [ undef, %SyncBB.i ]
  %vectorPHI86.i = phi <16 x double> [ %out_sel61269.i, %postload479.i ], [ undef, %SyncBB.i ]
  %vectorPHI87.i = phi <16 x i1> [ %local_edge284.i, %postload479.i ], [ %vector89.i, %SyncBB.i ]
  %j.02.i = phi i32 [ %254, %postload479.i ], [ 0, %SyncBB.i ]
  %vectorPHI90.i = phi <16 x double> [ %merge274.i, %postload479.i ], [ zeroinitializer, %SyncBB.i ]
  %vectorPHI91.i = phi <16 x double> [ %merge42272.i, %postload479.i ], [ zeroinitializer, %SyncBB.i ]
  %vectorPHI92.i = phi <16 x double> [ %merge44270.i, %postload479.i ], [ zeroinitializer, %SyncBB.i ]
  %extract113.i = extractelement <16 x i1> %vectorPHI87.i, i32 0
  %extract114.i = extractelement <16 x i1> %vectorPHI87.i, i32 1
  %extract115.i = extractelement <16 x i1> %vectorPHI87.i, i32 2
  %extract116.i = extractelement <16 x i1> %vectorPHI87.i, i32 3
  %extract117.i = extractelement <16 x i1> %vectorPHI87.i, i32 4
  %extract118.i = extractelement <16 x i1> %vectorPHI87.i, i32 5
  %extract119.i = extractelement <16 x i1> %vectorPHI87.i, i32 6
  %extract120.i = extractelement <16 x i1> %vectorPHI87.i, i32 7
  %extract121.i = extractelement <16 x i1> %vectorPHI87.i, i32 8
  %extract122.i = extractelement <16 x i1> %vectorPHI87.i, i32 9
  %extract123.i = extractelement <16 x i1> %vectorPHI87.i, i32 10
  %extract124.i = extractelement <16 x i1> %vectorPHI87.i, i32 11
  %extract125.i = extractelement <16 x i1> %vectorPHI87.i, i32 12
  %extract126.i = extractelement <16 x i1> %vectorPHI87.i, i32 13
  %extract127.i = extractelement <16 x i1> %vectorPHI87.i, i32 14
  %extract128.i = extractelement <16 x i1> %vectorPHI87.i, i32 15
  %tmp.i = mul i32 %j.02.i, %22
  %temp94.i = insertelement <16 x i32> undef, i32 %tmp.i, i32 0
  %vector95.i = shufflevector <16 x i32> %temp94.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp496.i = add <16 x i32> %tmp381.i, %vector95.i
  %120 = extractelement <16 x i32> %tmp496.i, i32 1
  %121 = zext i32 %120 to i64
  %122 = getelementptr inbounds i32 addrspace(1)* %10, i64 %121
  %123 = extractelement <16 x i32> %tmp496.i, i32 2
  %124 = zext i32 %123 to i64
  %125 = getelementptr inbounds i32 addrspace(1)* %10, i64 %124
  %126 = extractelement <16 x i32> %tmp496.i, i32 3
  %127 = zext i32 %126 to i64
  %128 = getelementptr inbounds i32 addrspace(1)* %10, i64 %127
  %129 = extractelement <16 x i32> %tmp496.i, i32 4
  %130 = zext i32 %129 to i64
  %131 = getelementptr inbounds i32 addrspace(1)* %10, i64 %130
  %132 = extractelement <16 x i32> %tmp496.i, i32 5
  %133 = zext i32 %132 to i64
  %134 = getelementptr inbounds i32 addrspace(1)* %10, i64 %133
  %135 = extractelement <16 x i32> %tmp496.i, i32 6
  %136 = zext i32 %135 to i64
  %137 = getelementptr inbounds i32 addrspace(1)* %10, i64 %136
  %138 = extractelement <16 x i32> %tmp496.i, i32 7
  %139 = zext i32 %138 to i64
  %140 = getelementptr inbounds i32 addrspace(1)* %10, i64 %139
  %141 = extractelement <16 x i32> %tmp496.i, i32 8
  %142 = zext i32 %141 to i64
  %143 = getelementptr inbounds i32 addrspace(1)* %10, i64 %142
  %144 = extractelement <16 x i32> %tmp496.i, i32 9
  %145 = zext i32 %144 to i64
  %146 = getelementptr inbounds i32 addrspace(1)* %10, i64 %145
  %147 = extractelement <16 x i32> %tmp496.i, i32 10
  %148 = zext i32 %147 to i64
  %149 = getelementptr inbounds i32 addrspace(1)* %10, i64 %148
  %150 = extractelement <16 x i32> %tmp496.i, i32 11
  %151 = zext i32 %150 to i64
  %152 = getelementptr inbounds i32 addrspace(1)* %10, i64 %151
  %153 = extractelement <16 x i32> %tmp496.i, i32 12
  %154 = zext i32 %153 to i64
  %155 = getelementptr inbounds i32 addrspace(1)* %10, i64 %154
  %156 = extractelement <16 x i32> %tmp496.i, i32 13
  %157 = zext i32 %156 to i64
  %158 = getelementptr inbounds i32 addrspace(1)* %10, i64 %157
  %159 = extractelement <16 x i32> %tmp496.i, i32 14
  %160 = zext i32 %159 to i64
  %161 = getelementptr inbounds i32 addrspace(1)* %10, i64 %160
  %162 = extractelement <16 x i32> %tmp496.i, i32 15
  %163 = zext i32 %162 to i64
  %164 = getelementptr inbounds i32 addrspace(1)* %10, i64 %163
  br i1 %extract113.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %.preheader.i
  %165 = extractelement <16 x i32> %tmp496.i, i32 0
  %166 = zext i32 %165 to i64
  %167 = getelementptr inbounds i32 addrspace(1)* %10, i64 %166
  %masked_load.i = load i32 addrspace(1)* %167, align 4
  %phitmp15.i = sext i32 %masked_load.i to i64
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %.preheader.i
  %phi.i = phi i64 [ 0, %.preheader.i ], [ %phitmp15.i, %preload.i ]
  br i1 %extract114.i, label %preload391.i, label %postload392.i

preload391.i:                                     ; preds = %postload.i
  %masked_load357.i = load i32 addrspace(1)* %122, align 4
  %phitmp.i = sext i32 %masked_load357.i to i64
  br label %postload392.i

postload392.i:                                    ; preds = %preload391.i, %postload.i
  %phi393.i = phi i64 [ 0, %postload.i ], [ %phitmp.i, %preload391.i ]
  br i1 %extract115.i, label %preload397.i, label %postload398.i

preload397.i:                                     ; preds = %postload392.i
  %masked_load358.i = load i32 addrspace(1)* %125, align 4
  %phitmp1.i = sext i32 %masked_load358.i to i64
  br label %postload398.i

postload398.i:                                    ; preds = %preload397.i, %postload392.i
  %phi399.i = phi i64 [ 0, %postload392.i ], [ %phitmp1.i, %preload397.i ]
  br i1 %extract116.i, label %preload403.i, label %postload404.i

preload403.i:                                     ; preds = %postload398.i
  %masked_load359.i = load i32 addrspace(1)* %128, align 4
  %phitmp2.i = sext i32 %masked_load359.i to i64
  br label %postload404.i

postload404.i:                                    ; preds = %preload403.i, %postload398.i
  %phi405.i = phi i64 [ 0, %postload398.i ], [ %phitmp2.i, %preload403.i ]
  br i1 %extract117.i, label %preload409.i, label %postload410.i

preload409.i:                                     ; preds = %postload404.i
  %masked_load360.i = load i32 addrspace(1)* %131, align 4
  %phitmp3.i = sext i32 %masked_load360.i to i64
  br label %postload410.i

postload410.i:                                    ; preds = %preload409.i, %postload404.i
  %phi411.i = phi i64 [ 0, %postload404.i ], [ %phitmp3.i, %preload409.i ]
  br i1 %extract118.i, label %preload415.i, label %postload416.i

preload415.i:                                     ; preds = %postload410.i
  %masked_load361.i = load i32 addrspace(1)* %134, align 4
  %phitmp4.i = sext i32 %masked_load361.i to i64
  br label %postload416.i

postload416.i:                                    ; preds = %preload415.i, %postload410.i
  %phi417.i = phi i64 [ 0, %postload410.i ], [ %phitmp4.i, %preload415.i ]
  br i1 %extract119.i, label %preload421.i, label %postload422.i

preload421.i:                                     ; preds = %postload416.i
  %masked_load362.i = load i32 addrspace(1)* %137, align 4
  %phitmp5.i = sext i32 %masked_load362.i to i64
  br label %postload422.i

postload422.i:                                    ; preds = %preload421.i, %postload416.i
  %phi423.i = phi i64 [ 0, %postload416.i ], [ %phitmp5.i, %preload421.i ]
  br i1 %extract120.i, label %preload427.i, label %postload428.i

preload427.i:                                     ; preds = %postload422.i
  %masked_load363.i = load i32 addrspace(1)* %140, align 4
  %phitmp6.i = sext i32 %masked_load363.i to i64
  br label %postload428.i

postload428.i:                                    ; preds = %preload427.i, %postload422.i
  %phi429.i = phi i64 [ 0, %postload422.i ], [ %phitmp6.i, %preload427.i ]
  br i1 %extract121.i, label %preload433.i, label %postload434.i

preload433.i:                                     ; preds = %postload428.i
  %masked_load364.i = load i32 addrspace(1)* %143, align 4
  %phitmp7.i = sext i32 %masked_load364.i to i64
  br label %postload434.i

postload434.i:                                    ; preds = %preload433.i, %postload428.i
  %phi435.i = phi i64 [ 0, %postload428.i ], [ %phitmp7.i, %preload433.i ]
  br i1 %extract122.i, label %preload439.i, label %postload440.i

preload439.i:                                     ; preds = %postload434.i
  %masked_load365.i = load i32 addrspace(1)* %146, align 4
  %phitmp8.i = sext i32 %masked_load365.i to i64
  br label %postload440.i

postload440.i:                                    ; preds = %preload439.i, %postload434.i
  %phi441.i = phi i64 [ 0, %postload434.i ], [ %phitmp8.i, %preload439.i ]
  br i1 %extract123.i, label %preload445.i, label %postload446.i

preload445.i:                                     ; preds = %postload440.i
  %masked_load366.i = load i32 addrspace(1)* %149, align 4
  %phitmp9.i = sext i32 %masked_load366.i to i64
  br label %postload446.i

postload446.i:                                    ; preds = %preload445.i, %postload440.i
  %phi447.i = phi i64 [ 0, %postload440.i ], [ %phitmp9.i, %preload445.i ]
  br i1 %extract124.i, label %preload451.i, label %postload452.i

preload451.i:                                     ; preds = %postload446.i
  %masked_load367.i = load i32 addrspace(1)* %152, align 4
  %phitmp10.i = sext i32 %masked_load367.i to i64
  br label %postload452.i

postload452.i:                                    ; preds = %preload451.i, %postload446.i
  %phi453.i = phi i64 [ 0, %postload446.i ], [ %phitmp10.i, %preload451.i ]
  br i1 %extract125.i, label %preload457.i, label %postload458.i

preload457.i:                                     ; preds = %postload452.i
  %masked_load368.i = load i32 addrspace(1)* %155, align 4
  %phitmp11.i = sext i32 %masked_load368.i to i64
  br label %postload458.i

postload458.i:                                    ; preds = %preload457.i, %postload452.i
  %phi459.i = phi i64 [ 0, %postload452.i ], [ %phitmp11.i, %preload457.i ]
  br i1 %extract126.i, label %preload463.i, label %postload464.i

preload463.i:                                     ; preds = %postload458.i
  %masked_load369.i = load i32 addrspace(1)* %158, align 4
  %phitmp12.i = sext i32 %masked_load369.i to i64
  br label %postload464.i

postload464.i:                                    ; preds = %preload463.i, %postload458.i
  %phi465.i = phi i64 [ 0, %postload458.i ], [ %phitmp12.i, %preload463.i ]
  br i1 %extract127.i, label %preload469.i, label %postload470.i

preload469.i:                                     ; preds = %postload464.i
  %masked_load370.i = load i32 addrspace(1)* %161, align 4
  %phitmp13.i = sext i32 %masked_load370.i to i64
  br label %postload470.i

postload470.i:                                    ; preds = %preload469.i, %postload464.i
  %phi471.i = phi i64 [ 0, %postload464.i ], [ %phitmp13.i, %preload469.i ]
  br i1 %extract128.i, label %preload475.i, label %postload476.i

preload475.i:                                     ; preds = %postload470.i
  %masked_load371.i = load i32 addrspace(1)* %164, align 4
  %phitmp14.i = sext i32 %masked_load371.i to i64
  br label %postload476.i

postload476.i:                                    ; preds = %preload475.i, %postload470.i
  %phi477.i = phi i64 [ 0, %postload470.i ], [ %phitmp14.i, %preload475.i ]
  %168 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi393.i
  %169 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi399.i
  %170 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi405.i
  %171 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi411.i
  %172 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi417.i
  %173 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi423.i
  %174 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi429.i
  %175 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi435.i
  %176 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi441.i
  %177 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi447.i
  %178 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi453.i
  %179 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi459.i
  %180 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi465.i
  %181 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi471.i
  %182 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi477.i
  br i1 %extract113.i, label %preload388.i, label %postload389.i

preload388.i:                                     ; preds = %postload476.i
  %183 = getelementptr inbounds <4 x double> addrspace(1)* %4, i64 %phi.i
  %masked_load372.i = load <4 x double> addrspace(1)* %183, align 32
  br label %postload389.i

postload389.i:                                    ; preds = %preload388.i, %postload476.i
  %phi390.i = phi <4 x double> [ undef, %postload476.i ], [ %masked_load372.i, %preload388.i ]
  br i1 %extract114.i, label %preload394.i, label %postload395.i

preload394.i:                                     ; preds = %postload389.i
  %masked_load373.i = load <4 x double> addrspace(1)* %168, align 32
  br label %postload395.i

postload395.i:                                    ; preds = %preload394.i, %postload389.i
  %phi396.i = phi <4 x double> [ undef, %postload389.i ], [ %masked_load373.i, %preload394.i ]
  br i1 %extract115.i, label %preload400.i, label %postload401.i

preload400.i:                                     ; preds = %postload395.i
  %masked_load374.i = load <4 x double> addrspace(1)* %169, align 32
  br label %postload401.i

postload401.i:                                    ; preds = %preload400.i, %postload395.i
  %phi402.i = phi <4 x double> [ undef, %postload395.i ], [ %masked_load374.i, %preload400.i ]
  br i1 %extract116.i, label %preload406.i, label %postload407.i

preload406.i:                                     ; preds = %postload401.i
  %masked_load375.i = load <4 x double> addrspace(1)* %170, align 32
  br label %postload407.i

postload407.i:                                    ; preds = %preload406.i, %postload401.i
  %phi408.i = phi <4 x double> [ undef, %postload401.i ], [ %masked_load375.i, %preload406.i ]
  br i1 %extract117.i, label %preload412.i, label %postload413.i

preload412.i:                                     ; preds = %postload407.i
  %masked_load376.i = load <4 x double> addrspace(1)* %171, align 32
  br label %postload413.i

postload413.i:                                    ; preds = %preload412.i, %postload407.i
  %phi414.i = phi <4 x double> [ undef, %postload407.i ], [ %masked_load376.i, %preload412.i ]
  br i1 %extract118.i, label %preload418.i, label %postload419.i

preload418.i:                                     ; preds = %postload413.i
  %masked_load377.i = load <4 x double> addrspace(1)* %172, align 32
  br label %postload419.i

postload419.i:                                    ; preds = %preload418.i, %postload413.i
  %phi420.i = phi <4 x double> [ undef, %postload413.i ], [ %masked_load377.i, %preload418.i ]
  br i1 %extract119.i, label %preload424.i, label %postload425.i

preload424.i:                                     ; preds = %postload419.i
  %masked_load378.i = load <4 x double> addrspace(1)* %173, align 32
  br label %postload425.i

postload425.i:                                    ; preds = %preload424.i, %postload419.i
  %phi426.i = phi <4 x double> [ undef, %postload419.i ], [ %masked_load378.i, %preload424.i ]
  br i1 %extract120.i, label %preload430.i, label %postload431.i

preload430.i:                                     ; preds = %postload425.i
  %masked_load379.i = load <4 x double> addrspace(1)* %174, align 32
  br label %postload431.i

postload431.i:                                    ; preds = %preload430.i, %postload425.i
  %phi432.i = phi <4 x double> [ undef, %postload425.i ], [ %masked_load379.i, %preload430.i ]
  br i1 %extract121.i, label %preload436.i, label %postload437.i

preload436.i:                                     ; preds = %postload431.i
  %masked_load380.i = load <4 x double> addrspace(1)* %175, align 32
  br label %postload437.i

postload437.i:                                    ; preds = %preload436.i, %postload431.i
  %phi438.i = phi <4 x double> [ undef, %postload431.i ], [ %masked_load380.i, %preload436.i ]
  br i1 %extract122.i, label %preload442.i, label %postload443.i

preload442.i:                                     ; preds = %postload437.i
  %masked_load381.i = load <4 x double> addrspace(1)* %176, align 32
  br label %postload443.i

postload443.i:                                    ; preds = %preload442.i, %postload437.i
  %phi444.i = phi <4 x double> [ undef, %postload437.i ], [ %masked_load381.i, %preload442.i ]
  br i1 %extract123.i, label %preload448.i, label %postload449.i

preload448.i:                                     ; preds = %postload443.i
  %masked_load382.i = load <4 x double> addrspace(1)* %177, align 32
  br label %postload449.i

postload449.i:                                    ; preds = %preload448.i, %postload443.i
  %phi450.i = phi <4 x double> [ undef, %postload443.i ], [ %masked_load382.i, %preload448.i ]
  br i1 %extract124.i, label %preload454.i, label %postload455.i

preload454.i:                                     ; preds = %postload449.i
  %masked_load383.i = load <4 x double> addrspace(1)* %178, align 32
  br label %postload455.i

postload455.i:                                    ; preds = %preload454.i, %postload449.i
  %phi456.i = phi <4 x double> [ undef, %postload449.i ], [ %masked_load383.i, %preload454.i ]
  br i1 %extract125.i, label %preload460.i, label %postload461.i

preload460.i:                                     ; preds = %postload455.i
  %masked_load384.i = load <4 x double> addrspace(1)* %179, align 32
  br label %postload461.i

postload461.i:                                    ; preds = %preload460.i, %postload455.i
  %phi462.i = phi <4 x double> [ undef, %postload455.i ], [ %masked_load384.i, %preload460.i ]
  br i1 %extract126.i, label %preload466.i, label %postload467.i

preload466.i:                                     ; preds = %postload461.i
  %masked_load385.i = load <4 x double> addrspace(1)* %180, align 32
  br label %postload467.i

postload467.i:                                    ; preds = %preload466.i, %postload461.i
  %phi468.i = phi <4 x double> [ undef, %postload461.i ], [ %masked_load385.i, %preload466.i ]
  br i1 %extract127.i, label %preload472.i, label %postload473.i

preload472.i:                                     ; preds = %postload467.i
  %masked_load386.i = load <4 x double> addrspace(1)* %181, align 32
  br label %postload473.i

postload473.i:                                    ; preds = %preload472.i, %postload467.i
  %phi474.i = phi <4 x double> [ undef, %postload467.i ], [ %masked_load386.i, %preload472.i ]
  br i1 %extract128.i, label %preload478.i, label %postload479.i

preload478.i:                                     ; preds = %postload473.i
  %masked_load387.i = load <4 x double> addrspace(1)* %182, align 32
  br label %postload479.i

postload479.i:                                    ; preds = %preload478.i, %postload473.i
  %phi480.i = phi <4 x double> [ undef, %postload473.i ], [ %masked_load387.i, %preload478.i ]
  %184 = extractelement <4 x double> %phi390.i, i32 0
  %185 = extractelement <4 x double> %phi396.i, i32 0
  %186 = extractelement <4 x double> %phi402.i, i32 0
  %187 = extractelement <4 x double> %phi408.i, i32 0
  %188 = extractelement <4 x double> %phi414.i, i32 0
  %189 = extractelement <4 x double> %phi420.i, i32 0
  %190 = extractelement <4 x double> %phi426.i, i32 0
  %191 = extractelement <4 x double> %phi432.i, i32 0
  %192 = extractelement <4 x double> %phi438.i, i32 0
  %193 = extractelement <4 x double> %phi444.i, i32 0
  %194 = extractelement <4 x double> %phi450.i, i32 0
  %195 = extractelement <4 x double> %phi456.i, i32 0
  %196 = extractelement <4 x double> %phi462.i, i32 0
  %197 = extractelement <4 x double> %phi468.i, i32 0
  %198 = extractelement <4 x double> %phi474.i, i32 0
  %199 = extractelement <4 x double> %phi480.i, i32 0
  %temp.vect177.i = insertelement <16 x double> undef, double %184, i32 0
  %temp.vect178.i = insertelement <16 x double> %temp.vect177.i, double %185, i32 1
  %temp.vect179.i = insertelement <16 x double> %temp.vect178.i, double %186, i32 2
  %temp.vect180.i = insertelement <16 x double> %temp.vect179.i, double %187, i32 3
  %temp.vect181.i = insertelement <16 x double> %temp.vect180.i, double %188, i32 4
  %temp.vect182.i = insertelement <16 x double> %temp.vect181.i, double %189, i32 5
  %temp.vect183.i = insertelement <16 x double> %temp.vect182.i, double %190, i32 6
  %temp.vect184.i = insertelement <16 x double> %temp.vect183.i, double %191, i32 7
  %temp.vect185.i = insertelement <16 x double> %temp.vect184.i, double %192, i32 8
  %temp.vect186.i = insertelement <16 x double> %temp.vect185.i, double %193, i32 9
  %temp.vect187.i = insertelement <16 x double> %temp.vect186.i, double %194, i32 10
  %temp.vect188.i = insertelement <16 x double> %temp.vect187.i, double %195, i32 11
  %temp.vect189.i = insertelement <16 x double> %temp.vect188.i, double %196, i32 12
  %temp.vect190.i = insertelement <16 x double> %temp.vect189.i, double %197, i32 13
  %temp.vect191.i = insertelement <16 x double> %temp.vect190.i, double %198, i32 14
  %temp.vect192.i = insertelement <16 x double> %temp.vect191.i, double %199, i32 15
  %200 = extractelement <4 x double> %phi390.i, i32 1
  %201 = extractelement <4 x double> %phi396.i, i32 1
  %202 = extractelement <4 x double> %phi402.i, i32 1
  %203 = extractelement <4 x double> %phi408.i, i32 1
  %204 = extractelement <4 x double> %phi414.i, i32 1
  %205 = extractelement <4 x double> %phi420.i, i32 1
  %206 = extractelement <4 x double> %phi426.i, i32 1
  %207 = extractelement <4 x double> %phi432.i, i32 1
  %208 = extractelement <4 x double> %phi438.i, i32 1
  %209 = extractelement <4 x double> %phi444.i, i32 1
  %210 = extractelement <4 x double> %phi450.i, i32 1
  %211 = extractelement <4 x double> %phi456.i, i32 1
  %212 = extractelement <4 x double> %phi462.i, i32 1
  %213 = extractelement <4 x double> %phi468.i, i32 1
  %214 = extractelement <4 x double> %phi474.i, i32 1
  %215 = extractelement <4 x double> %phi480.i, i32 1
  %temp.vect209.i = insertelement <16 x double> undef, double %200, i32 0
  %temp.vect210.i = insertelement <16 x double> %temp.vect209.i, double %201, i32 1
  %temp.vect211.i = insertelement <16 x double> %temp.vect210.i, double %202, i32 2
  %temp.vect212.i = insertelement <16 x double> %temp.vect211.i, double %203, i32 3
  %temp.vect213.i = insertelement <16 x double> %temp.vect212.i, double %204, i32 4
  %temp.vect214.i = insertelement <16 x double> %temp.vect213.i, double %205, i32 5
  %temp.vect215.i = insertelement <16 x double> %temp.vect214.i, double %206, i32 6
  %temp.vect216.i = insertelement <16 x double> %temp.vect215.i, double %207, i32 7
  %temp.vect217.i = insertelement <16 x double> %temp.vect216.i, double %208, i32 8
  %temp.vect218.i = insertelement <16 x double> %temp.vect217.i, double %209, i32 9
  %temp.vect219.i = insertelement <16 x double> %temp.vect218.i, double %210, i32 10
  %temp.vect220.i = insertelement <16 x double> %temp.vect219.i, double %211, i32 11
  %temp.vect221.i = insertelement <16 x double> %temp.vect220.i, double %212, i32 12
  %temp.vect222.i = insertelement <16 x double> %temp.vect221.i, double %213, i32 13
  %temp.vect223.i = insertelement <16 x double> %temp.vect222.i, double %214, i32 14
  %temp.vect224.i = insertelement <16 x double> %temp.vect223.i, double %215, i32 15
  %216 = extractelement <4 x double> %phi390.i, i32 2
  %217 = extractelement <4 x double> %phi396.i, i32 2
  %218 = extractelement <4 x double> %phi402.i, i32 2
  %219 = extractelement <4 x double> %phi408.i, i32 2
  %220 = extractelement <4 x double> %phi414.i, i32 2
  %221 = extractelement <4 x double> %phi420.i, i32 2
  %222 = extractelement <4 x double> %phi426.i, i32 2
  %223 = extractelement <4 x double> %phi432.i, i32 2
  %224 = extractelement <4 x double> %phi438.i, i32 2
  %225 = extractelement <4 x double> %phi444.i, i32 2
  %226 = extractelement <4 x double> %phi450.i, i32 2
  %227 = extractelement <4 x double> %phi456.i, i32 2
  %228 = extractelement <4 x double> %phi462.i, i32 2
  %229 = extractelement <4 x double> %phi468.i, i32 2
  %230 = extractelement <4 x double> %phi474.i, i32 2
  %231 = extractelement <4 x double> %phi480.i, i32 2
  %temp.vect241.i = insertelement <16 x double> undef, double %216, i32 0
  %temp.vect242.i = insertelement <16 x double> %temp.vect241.i, double %217, i32 1
  %temp.vect243.i = insertelement <16 x double> %temp.vect242.i, double %218, i32 2
  %temp.vect244.i = insertelement <16 x double> %temp.vect243.i, double %219, i32 3
  %temp.vect245.i = insertelement <16 x double> %temp.vect244.i, double %220, i32 4
  %temp.vect246.i = insertelement <16 x double> %temp.vect245.i, double %221, i32 5
  %temp.vect247.i = insertelement <16 x double> %temp.vect246.i, double %222, i32 6
  %temp.vect248.i = insertelement <16 x double> %temp.vect247.i, double %223, i32 7
  %temp.vect249.i = insertelement <16 x double> %temp.vect248.i, double %224, i32 8
  %temp.vect250.i = insertelement <16 x double> %temp.vect249.i, double %225, i32 9
  %temp.vect251.i = insertelement <16 x double> %temp.vect250.i, double %226, i32 10
  %temp.vect252.i = insertelement <16 x double> %temp.vect251.i, double %227, i32 11
  %temp.vect253.i = insertelement <16 x double> %temp.vect252.i, double %228, i32 12
  %temp.vect254.i = insertelement <16 x double> %temp.vect253.i, double %229, i32 13
  %temp.vect255.i = insertelement <16 x double> %temp.vect254.i, double %230, i32 14
  %temp.vect256.i = insertelement <16 x double> %temp.vect255.i, double %231, i32 15
  %232 = fsub <16 x double> %temp.vect176.i, %temp.vect192.i
  %233 = fsub <16 x double> %temp.vect208.i, %temp.vect224.i
  %234 = fsub <16 x double> %temp.vect240.i, %temp.vect256.i
  %235 = fmul <16 x double> %232, %232
  %236 = fmul <16 x double> %233, %233
  %237 = fadd <16 x double> %235, %236
  %238 = fmul <16 x double> %234, %234
  %239 = fadd <16 x double> %237, %238
  %240 = fcmp olt <16 x double> %239, %vector258.i
  %_to_32261.i = and <16 x i1> %vectorPHI87.i, %240
  %241 = fdiv <16 x double> <double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00, double 1.000000e+00>, %239
  %242 = fmul <16 x double> %241, %241
  %243 = fmul <16 x double> %242, %241
  %244 = fmul <16 x double> %241, %243
  %245 = fmul <16 x double> %243, %vector263.i
  %246 = fsub <16 x double> %245, %vector265.i
  %247 = fmul <16 x double> %244, %246
  %248 = fmul <16 x double> %234, %247
  %249 = fmul <16 x double> %233, %247
  %250 = fmul <16 x double> %232, %247
  %251 = fadd <16 x double> %vectorPHI92.i, %248
  %252 = fadd <16 x double> %vectorPHI91.i, %249
  %253 = fadd <16 x double> %vectorPHI90.i, %250
  %out_sel61269.i = select <16 x i1> %vectorPHI87.i, <16 x double> zeroinitializer, <16 x double> %vectorPHI86.i
  %merge44270.i = select <16 x i1> %_to_32261.i, <16 x double> %251, <16 x double> %vectorPHI92.i
  %out_sel58271.i = select <16 x i1> %vectorPHI87.i, <16 x double> %merge44270.i, <16 x double> %vectorPHI85.i
  %merge42272.i = select <16 x i1> %_to_32261.i, <16 x double> %252, <16 x double> %vectorPHI91.i
  %out_sel55273.i = select <16 x i1> %vectorPHI87.i, <16 x double> %merge42272.i, <16 x double> %vectorPHI84.i
  %merge274.i = select <16 x i1> %_to_32261.i, <16 x double> %253, <16 x double> %vectorPHI90.i
  %out_sel275.i = select <16 x i1> %vectorPHI87.i, <16 x double> %merge274.i, <16 x double> %vectorPHI83.i
  %254 = add nsw i32 %j.02.i, 1
  %exitcond.i = icmp eq i32 %254, %7
  %temp276.i = insertelement <16 x i1> undef, i1 %exitcond.i, i32 0
  %vector277.i = shufflevector <16 x i1> %temp276.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond.i = xor i1 %exitcond.i, true
  %temp282.i = insertelement <16 x i1> undef, i1 %notCond.i, i32 0
  %vector283.i = shufflevector <16 x i1> %temp282.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr278.i = and <16 x i1> %vectorPHI87.i, %vector277.i
  %loop_mask33280.i = or <16 x i1> %vectorPHI82.i, %who_left_tr278.i
  %curr_loop_mask281.i = or <16 x i1> %loop_mask33280.i, %who_left_tr278.i
  %ipred.i.i = bitcast <16 x i1> %curr_loop_mask281.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %tmp.i.i = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %tmp.i.i, 0
  %local_edge284.i = and <16 x i1> %vectorPHI87.i, %vector283.i
  br i1 %res.i.i, label %.preheader.i, label %._crit_edge.i

._crit_edge.i:                                    ; preds = %postload479.i, %SyncBB.i
  %vectorPHI285.i = phi <16 x double> [ undef, %SyncBB.i ], [ %out_sel61269.i, %postload479.i ]
  %vectorPHI286.i = phi <16 x double> [ undef, %SyncBB.i ], [ %out_sel58271.i, %postload479.i ]
  %vectorPHI287.i = phi <16 x double> [ undef, %SyncBB.i ], [ %out_sel55273.i, %postload479.i ]
  %vectorPHI288.i = phi <16 x double> [ undef, %SyncBB.i ], [ %out_sel275.i, %postload479.i ]
  %merge54289.i = select i1 %32, <16 x double> %vectorPHI285.i, <16 x double> zeroinitializer
  %extract341.i = extractelement <16 x double> %merge54289.i, i32 0
  %extract342.i = extractelement <16 x double> %merge54289.i, i32 1
  %extract343.i = extractelement <16 x double> %merge54289.i, i32 2
  %extract344.i = extractelement <16 x double> %merge54289.i, i32 3
  %extract345.i = extractelement <16 x double> %merge54289.i, i32 4
  %extract346.i = extractelement <16 x double> %merge54289.i, i32 5
  %extract347.i = extractelement <16 x double> %merge54289.i, i32 6
  %extract348.i = extractelement <16 x double> %merge54289.i, i32 7
  %extract349.i = extractelement <16 x double> %merge54289.i, i32 8
  %extract350.i = extractelement <16 x double> %merge54289.i, i32 9
  %extract351.i = extractelement <16 x double> %merge54289.i, i32 10
  %extract352.i = extractelement <16 x double> %merge54289.i, i32 11
  %extract353.i = extractelement <16 x double> %merge54289.i, i32 12
  %extract354.i = extractelement <16 x double> %merge54289.i, i32 13
  %extract355.i = extractelement <16 x double> %merge54289.i, i32 14
  %extract356.i = extractelement <16 x double> %merge54289.i, i32 15
  %merge52290.i = select i1 %32, <16 x double> %vectorPHI286.i, <16 x double> zeroinitializer
  %extract325.i = extractelement <16 x double> %merge52290.i, i32 0
  %extract326.i = extractelement <16 x double> %merge52290.i, i32 1
  %extract327.i = extractelement <16 x double> %merge52290.i, i32 2
  %extract328.i = extractelement <16 x double> %merge52290.i, i32 3
  %extract329.i = extractelement <16 x double> %merge52290.i, i32 4
  %extract330.i = extractelement <16 x double> %merge52290.i, i32 5
  %extract331.i = extractelement <16 x double> %merge52290.i, i32 6
  %extract332.i = extractelement <16 x double> %merge52290.i, i32 7
  %extract333.i = extractelement <16 x double> %merge52290.i, i32 8
  %extract334.i = extractelement <16 x double> %merge52290.i, i32 9
  %extract335.i = extractelement <16 x double> %merge52290.i, i32 10
  %extract336.i = extractelement <16 x double> %merge52290.i, i32 11
  %extract337.i = extractelement <16 x double> %merge52290.i, i32 12
  %extract338.i = extractelement <16 x double> %merge52290.i, i32 13
  %extract339.i = extractelement <16 x double> %merge52290.i, i32 14
  %extract340.i = extractelement <16 x double> %merge52290.i, i32 15
  %merge50291.i = select i1 %32, <16 x double> %vectorPHI287.i, <16 x double> zeroinitializer
  %extract309.i = extractelement <16 x double> %merge50291.i, i32 0
  %extract310.i = extractelement <16 x double> %merge50291.i, i32 1
  %extract311.i = extractelement <16 x double> %merge50291.i, i32 2
  %extract312.i = extractelement <16 x double> %merge50291.i, i32 3
  %extract313.i = extractelement <16 x double> %merge50291.i, i32 4
  %extract314.i = extractelement <16 x double> %merge50291.i, i32 5
  %extract315.i = extractelement <16 x double> %merge50291.i, i32 6
  %extract316.i = extractelement <16 x double> %merge50291.i, i32 7
  %extract317.i = extractelement <16 x double> %merge50291.i, i32 8
  %extract318.i = extractelement <16 x double> %merge50291.i, i32 9
  %extract319.i = extractelement <16 x double> %merge50291.i, i32 10
  %extract320.i = extractelement <16 x double> %merge50291.i, i32 11
  %extract321.i = extractelement <16 x double> %merge50291.i, i32 12
  %extract322.i = extractelement <16 x double> %merge50291.i, i32 13
  %extract323.i = extractelement <16 x double> %merge50291.i, i32 14
  %extract324.i = extractelement <16 x double> %merge50291.i, i32 15
  %merge48292.i = select i1 %32, <16 x double> %vectorPHI288.i, <16 x double> zeroinitializer
  %extract293.i = extractelement <16 x double> %merge48292.i, i32 0
  %extract294.i = extractelement <16 x double> %merge48292.i, i32 1
  %extract295.i = extractelement <16 x double> %merge48292.i, i32 2
  %extract296.i = extractelement <16 x double> %merge48292.i, i32 3
  %extract297.i = extractelement <16 x double> %merge48292.i, i32 4
  %extract298.i = extractelement <16 x double> %merge48292.i, i32 5
  %extract299.i = extractelement <16 x double> %merge48292.i, i32 6
  %extract300.i = extractelement <16 x double> %merge48292.i, i32 7
  %extract301.i = extractelement <16 x double> %merge48292.i, i32 8
  %extract302.i = extractelement <16 x double> %merge48292.i, i32 9
  %extract303.i = extractelement <16 x double> %merge48292.i, i32 10
  %extract304.i = extractelement <16 x double> %merge48292.i, i32 11
  %extract305.i = extractelement <16 x double> %merge48292.i, i32 12
  %extract306.i = extractelement <16 x double> %merge48292.i, i32 13
  %extract307.i = extractelement <16 x double> %merge48292.i, i32 14
  %extract308.i = extractelement <16 x double> %merge48292.i, i32 15
  %255 = insertelement <4 x double> undef, double %extract293.i, i32 0
  %256 = insertelement <4 x double> undef, double %extract294.i, i32 0
  %257 = insertelement <4 x double> undef, double %extract295.i, i32 0
  %258 = insertelement <4 x double> undef, double %extract296.i, i32 0
  %259 = insertelement <4 x double> undef, double %extract297.i, i32 0
  %260 = insertelement <4 x double> undef, double %extract298.i, i32 0
  %261 = insertelement <4 x double> undef, double %extract299.i, i32 0
  %262 = insertelement <4 x double> undef, double %extract300.i, i32 0
  %263 = insertelement <4 x double> undef, double %extract301.i, i32 0
  %264 = insertelement <4 x double> undef, double %extract302.i, i32 0
  %265 = insertelement <4 x double> undef, double %extract303.i, i32 0
  %266 = insertelement <4 x double> undef, double %extract304.i, i32 0
  %267 = insertelement <4 x double> undef, double %extract305.i, i32 0
  %268 = insertelement <4 x double> undef, double %extract306.i, i32 0
  %269 = insertelement <4 x double> undef, double %extract307.i, i32 0
  %270 = insertelement <4 x double> undef, double %extract308.i, i32 0
  %271 = insertelement <4 x double> %255, double %extract309.i, i32 1
  %272 = insertelement <4 x double> %256, double %extract310.i, i32 1
  %273 = insertelement <4 x double> %257, double %extract311.i, i32 1
  %274 = insertelement <4 x double> %258, double %extract312.i, i32 1
  %275 = insertelement <4 x double> %259, double %extract313.i, i32 1
  %276 = insertelement <4 x double> %260, double %extract314.i, i32 1
  %277 = insertelement <4 x double> %261, double %extract315.i, i32 1
  %278 = insertelement <4 x double> %262, double %extract316.i, i32 1
  %279 = insertelement <4 x double> %263, double %extract317.i, i32 1
  %280 = insertelement <4 x double> %264, double %extract318.i, i32 1
  %281 = insertelement <4 x double> %265, double %extract319.i, i32 1
  %282 = insertelement <4 x double> %266, double %extract320.i, i32 1
  %283 = insertelement <4 x double> %267, double %extract321.i, i32 1
  %284 = insertelement <4 x double> %268, double %extract322.i, i32 1
  %285 = insertelement <4 x double> %269, double %extract323.i, i32 1
  %286 = insertelement <4 x double> %270, double %extract324.i, i32 1
  %287 = insertelement <4 x double> %271, double %extract325.i, i32 2
  %288 = insertelement <4 x double> %272, double %extract326.i, i32 2
  %289 = insertelement <4 x double> %273, double %extract327.i, i32 2
  %290 = insertelement <4 x double> %274, double %extract328.i, i32 2
  %291 = insertelement <4 x double> %275, double %extract329.i, i32 2
  %292 = insertelement <4 x double> %276, double %extract330.i, i32 2
  %293 = insertelement <4 x double> %277, double %extract331.i, i32 2
  %294 = insertelement <4 x double> %278, double %extract332.i, i32 2
  %295 = insertelement <4 x double> %279, double %extract333.i, i32 2
  %296 = insertelement <4 x double> %280, double %extract334.i, i32 2
  %297 = insertelement <4 x double> %281, double %extract335.i, i32 2
  %298 = insertelement <4 x double> %282, double %extract336.i, i32 2
  %299 = insertelement <4 x double> %283, double %extract337.i, i32 2
  %300 = insertelement <4 x double> %284, double %extract338.i, i32 2
  %301 = insertelement <4 x double> %285, double %extract339.i, i32 2
  %302 = insertelement <4 x double> %286, double %extract340.i, i32 2
  %303 = insertelement <4 x double> %287, double %extract341.i, i32 3
  %304 = insertelement <4 x double> %288, double %extract342.i, i32 3
  %305 = insertelement <4 x double> %289, double %extract343.i, i32 3
  %306 = insertelement <4 x double> %290, double %extract344.i, i32 3
  %307 = insertelement <4 x double> %291, double %extract345.i, i32 3
  %308 = insertelement <4 x double> %292, double %extract346.i, i32 3
  %309 = insertelement <4 x double> %293, double %extract347.i, i32 3
  %310 = insertelement <4 x double> %294, double %extract348.i, i32 3
  %311 = insertelement <4 x double> %295, double %extract349.i, i32 3
  %312 = insertelement <4 x double> %296, double %extract350.i, i32 3
  %313 = insertelement <4 x double> %297, double %extract351.i, i32 3
  %314 = insertelement <4 x double> %298, double %extract352.i, i32 3
  %315 = insertelement <4 x double> %299, double %extract353.i, i32 3
  %316 = insertelement <4 x double> %300, double %extract354.i, i32 3
  %317 = insertelement <4 x double> %301, double %extract355.i, i32 3
  %318 = insertelement <4 x double> %302, double %extract356.i, i32 3
  %319 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract.i
  %320 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract66.i
  %321 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract67.i
  %322 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract68.i
  %323 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract69.i
  %324 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract70.i
  %325 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract71.i
  %326 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract72.i
  %327 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract73.i
  %328 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract74.i
  %329 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract75.i
  %330 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract76.i
  %331 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract77.i
  %332 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract78.i
  %333 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract79.i
  %334 = getelementptr inbounds <4 x double> addrspace(1)* %1, i64 %extract80.i
  store <4 x double> %303, <4 x double> addrspace(1)* %319, align 32
  store <4 x double> %304, <4 x double> addrspace(1)* %320, align 32
  store <4 x double> %305, <4 x double> addrspace(1)* %321, align 32
  store <4 x double> %306, <4 x double> addrspace(1)* %322, align 32
  store <4 x double> %307, <4 x double> addrspace(1)* %323, align 32
  store <4 x double> %308, <4 x double> addrspace(1)* %324, align 32
  store <4 x double> %309, <4 x double> addrspace(1)* %325, align 32
  store <4 x double> %310, <4 x double> addrspace(1)* %326, align 32
  store <4 x double> %311, <4 x double> addrspace(1)* %327, align 32
  store <4 x double> %312, <4 x double> addrspace(1)* %328, align 32
  store <4 x double> %313, <4 x double> addrspace(1)* %329, align 32
  store <4 x double> %314, <4 x double> addrspace(1)* %330, align 32
  store <4 x double> %315, <4 x double> addrspace(1)* %331, align 32
  store <4 x double> %316, <4 x double> addrspace(1)* %332, align 32
  store <4 x double> %317, <4 x double> addrspace(1)* %333, align 32
  store <4 x double> %318, <4 x double> addrspace(1)* %334, align 32
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %31
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.compute_lj_force_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.compute_lj_force_separated_args.exit: ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x double> addrspace(1)*, <4 x double> addrspace(1)*, i32, i32 addrspace(1)*, double, double, double, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__compute_lj_force_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double4 __attribute__((address_space(1))) *, double4 __attribute__((address_space(1))) *, int const, int __attribute__((address_space(1))) *, double const, double const, double const, int const", metadata !"opencl_compute_lj_force_locals_anchor", void (i8*)* @compute_lj_force}
!1 = metadata !{i32 0, i32 0, i32 0}
