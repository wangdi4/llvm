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

@opencl_fft1D_512_local_smem = internal addrspace(3) global [576 x double] zeroinitializer, align 16
@fft1D_512.reversed8 = internal constant [8 x i32] [i32 0, i32 4, i32 2, i32 6, i32 1, i32 5, i32 3, i32 7], align 16
@opencl_ifft1D_512_local_smem = internal addrspace(3) global [576 x double] zeroinitializer, align 16

declare void @__fft1D_512_original(<2 x double> addrspace(1)* nocapture) nounwind

declare i64 @get_local_id(i32)

declare i64 @get_group_id(i32)

declare fastcc void @__globalLoads8_original(<2 x double>* nocapture, <2 x double> addrspace(1)* nocapture) nounwind inlinehint

declare fastcc <2 x double> @__cmplx_add_original(<2 x double>, <2 x double>) nounwind readnone inlinehint

declare fastcc <2 x double> @__cmplx_sub_original(<2 x double>, <2 x double>) nounwind readnone inlinehint

declare fastcc <2 x double> @__cm_fl_mul_original(<2 x double>) nounwind readnone inlinehint

declare fastcc <2 x double> @__cmplx_mul_original(<2 x double>, <2 x double>) nounwind readnone inlinehint

declare fastcc <2 x double> @__exp_i_original(double) nounwind inlinehint

declare fastcc void @__storex8_original(<2 x double>* nocapture, double addrspace(3)* nocapture, i32) nounwind inlinehint

declare void @barrier(i64)

declare fastcc void @__loadx8_original(<2 x double>* nocapture, double addrspace(3)* nocapture) nounwind inlinehint

declare fastcc void @__storey8_original(<2 x double>* nocapture, double addrspace(3)* nocapture, i32) nounwind inlinehint

declare fastcc void @__loady8_original(<2 x double>* nocapture, double addrspace(3)* nocapture) nounwind inlinehint

declare fastcc void @__globalStores8_original(<2 x double>* nocapture, <2 x double> addrspace(1)* nocapture) nounwind inlinehint

declare void @__ifft1D_512_original(<2 x double> addrspace(1)* nocapture) nounwind

declare void @__chk1D_512_original(<2 x double> addrspace(1)* nocapture, i32, i32 addrspace(1)* nocapture) nounwind

declare double @_Z3cosd(double)

declare double @_Z3sind(double)

declare void @dummybarrier.()

declare i64* @get_curr_wi.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

define void @__fft1D_512_separated_args(<2 x double> addrspace(1)* nocapture %work, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph3:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to [576 x double] addrspace(3)*
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB193

SyncBB193:                                        ; preds = %thenBB195, %bb.nph3
  %CurrSBIndex..1 = phi i64 [ 0, %bb.nph3 ], [ %"loadedCurrSB+Stride201", %thenBB195 ]
  %currWI = load i64* %pCurrWI, align 8
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %currWI, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = trunc i64 %2 to i32
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %3, i32* %CastToValueType, align 4
  %4 = load i64* %pWGId, align 8
  %5 = shl i64 %4, 9
  %6 = add i64 %5, %2
  %7 = ashr i32 %3, 3
  %"&(pSB[currWI].offset)3265" = or i64 %CurrSBIndex..1, 4
  %"&pSB[currWI].offset33" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)3265"
  %CastToValueType34 = bitcast i8* %"&pSB[currWI].offset33" to i32*
  store i32 %7, i32* %CastToValueType34, align 4
  %8 = and i32 %3, 7
  %"&(pSB[currWI].offset)4666" = or i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset47" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)4666"
  %CastToValueType48 = bitcast i8* %"&pSB[currWI].offset47" to i32*
  store i32 %8, i32* %CastToValueType48, align 4
  %sext = shl i64 %6, 32
  %9 = ashr i64 %sext, 32
  %10 = getelementptr inbounds <2 x double> addrspace(1)* %work, i64 %9
  %"&(pSB[currWI].offset)6067" = or i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset61" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)6067"
  %CastToValueType62 = bitcast i8* %"&pSB[currWI].offset61" to <2 x double> addrspace(1)**
  store <2 x double> addrspace(1)* %10, <2 x double> addrspace(1)** %CastToValueType62, align 8
  %"&(pSB[currWI].offset)142" = add nuw i64 %CurrSBIndex..1, 48
  %"&pSB[currWI].offset143" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)142"
  %11 = bitcast i8* %"&pSB[currWI].offset143" to <2 x double>*
  %12 = load <2 x double> addrspace(1)* %10, align 16
  store <2 x double> %12, <2 x double>* %11, align 16
  %"&pSB[currWI].offset143.sum" = add i64 %CurrSBIndex..1, 64
  %scevgep.1.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum"
  %13 = bitcast i8* %scevgep.1.i to <2 x double>*
  %.sum = add i64 %9, 64
  %scevgep2.1.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum
  %14 = load <2 x double> addrspace(1)* %scevgep2.1.i, align 16
  store <2 x double> %14, <2 x double>* %13, align 16
  %"&pSB[currWI].offset143.sum68" = add i64 %CurrSBIndex..1, 80
  %scevgep.2.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum68"
  %15 = bitcast i8* %scevgep.2.i to <2 x double>*
  %.sum69 = add i64 %9, 128
  %scevgep2.2.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum69
  %16 = load <2 x double> addrspace(1)* %scevgep2.2.i, align 16
  store <2 x double> %16, <2 x double>* %15, align 16
  %"&pSB[currWI].offset143.sum70" = add i64 %CurrSBIndex..1, 96
  %scevgep.3.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum70"
  %17 = bitcast i8* %scevgep.3.i to <2 x double>*
  %.sum71 = add i64 %9, 192
  %scevgep2.3.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum71
  %18 = load <2 x double> addrspace(1)* %scevgep2.3.i, align 16
  store <2 x double> %18, <2 x double>* %17, align 16
  %"&pSB[currWI].offset143.sum72" = add i64 %CurrSBIndex..1, 112
  %scevgep.4.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum72"
  %19 = bitcast i8* %scevgep.4.i to <2 x double>*
  %.sum73 = add i64 %9, 256
  %scevgep2.4.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum73
  %20 = load <2 x double> addrspace(1)* %scevgep2.4.i, align 16
  store <2 x double> %20, <2 x double>* %19, align 16
  %"&pSB[currWI].offset143.sum74" = add i64 %CurrSBIndex..1, 128
  %scevgep.5.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum74"
  %21 = bitcast i8* %scevgep.5.i to <2 x double>*
  %.sum75 = add i64 %9, 320
  %scevgep2.5.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum75
  %22 = load <2 x double> addrspace(1)* %scevgep2.5.i, align 16
  store <2 x double> %22, <2 x double>* %21, align 16
  %"&pSB[currWI].offset143.sum76" = add i64 %CurrSBIndex..1, 144
  %scevgep.6.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum76"
  %23 = bitcast i8* %scevgep.6.i to <2 x double>*
  %.sum77 = add i64 %9, 384
  %scevgep2.6.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum77
  %24 = load <2 x double> addrspace(1)* %scevgep2.6.i, align 16
  store <2 x double> %24, <2 x double>* %23, align 16
  %"&pSB[currWI].offset143.sum78" = add i64 %CurrSBIndex..1, 160
  %scevgep.7.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum78"
  %25 = bitcast i8* %scevgep.7.i to <2 x double>*
  %.sum79 = add i64 %9, 448
  %scevgep2.7.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum79
  %26 = load <2 x double> addrspace(1)* %scevgep2.7.i, align 16
  store <2 x double> %26, <2 x double>* %25, align 16
  %27 = load <2 x double>* %11, align 16
  %"&pSB[currWI].offset139.sum" = add i64 %CurrSBIndex..1, 112
  %28 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset139.sum"
  %29 = bitcast i8* %28 to <2 x double>*
  %30 = load <2 x double>* %29, align 16
  %31 = extractelement <2 x double> %27, i32 0
  %32 = extractelement <2 x double> %30, i32 0
  %33 = fadd double %31, %32
  %34 = extractelement <2 x double> %27, i32 1
  %35 = extractelement <2 x double> %30, i32 1
  %36 = fadd double %34, %35
  %37 = extractelement <2 x double> %27, i32 0
  %38 = extractelement <2 x double> %30, i32 0
  %39 = fsub double %37, %38
  %40 = extractelement <2 x double> %27, i32 1
  %41 = extractelement <2 x double> %30, i32 1
  %42 = fsub double %40, %41
  %"&pSB[currWI].offset135.sum" = add i64 %CurrSBIndex..1, 64
  %43 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset135.sum"
  %44 = bitcast i8* %43 to <2 x double>*
  %45 = load <2 x double>* %44, align 16
  %"&pSB[currWI].offset131.sum" = add i64 %CurrSBIndex..1, 128
  %46 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset131.sum"
  %47 = bitcast i8* %46 to <2 x double>*
  %48 = load <2 x double>* %47, align 16
  %49 = extractelement <2 x double> %45, i32 0
  %50 = extractelement <2 x double> %48, i32 0
  %51 = fadd double %49, %50
  %52 = extractelement <2 x double> %45, i32 1
  %53 = extractelement <2 x double> %48, i32 1
  %54 = fadd double %52, %53
  %55 = extractelement <2 x double> %45, i32 0
  %56 = extractelement <2 x double> %48, i32 0
  %57 = fsub double %55, %56
  %58 = extractelement <2 x double> %45, i32 1
  %59 = extractelement <2 x double> %48, i32 1
  %60 = fsub double %58, %59
  %"&pSB[currWI].offset127.sum" = add i64 %CurrSBIndex..1, 80
  %61 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset127.sum"
  %62 = bitcast i8* %61 to <2 x double>*
  %63 = load <2 x double>* %62, align 16
  %"&pSB[currWI].offset123.sum" = add i64 %CurrSBIndex..1, 144
  %64 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset123.sum"
  %65 = bitcast i8* %64 to <2 x double>*
  %66 = load <2 x double>* %65, align 16
  %67 = extractelement <2 x double> %63, i32 0
  %68 = extractelement <2 x double> %66, i32 0
  %69 = fadd double %67, %68
  %70 = extractelement <2 x double> %63, i32 1
  %71 = extractelement <2 x double> %66, i32 1
  %72 = fadd double %70, %71
  %73 = extractelement <2 x double> %63, i32 0
  %74 = extractelement <2 x double> %66, i32 0
  %75 = fsub double %73, %74
  %76 = extractelement <2 x double> %63, i32 1
  %77 = extractelement <2 x double> %66, i32 1
  %78 = fsub double %76, %77
  %"&pSB[currWI].offset119.sum" = add i64 %CurrSBIndex..1, 96
  %79 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset119.sum"
  %80 = bitcast i8* %79 to <2 x double>*
  %81 = load <2 x double>* %80, align 16
  %"&pSB[currWI].offset115.sum" = add i64 %CurrSBIndex..1, 160
  %82 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset115.sum"
  %83 = bitcast i8* %82 to <2 x double>*
  %84 = load <2 x double>* %83, align 16
  %85 = extractelement <2 x double> %81, i32 0
  %86 = extractelement <2 x double> %84, i32 0
  %87 = fadd double %85, %86
  %88 = extractelement <2 x double> %81, i32 1
  %89 = extractelement <2 x double> %84, i32 1
  %90 = fadd double %88, %89
  %91 = extractelement <2 x double> %81, i32 0
  %92 = extractelement <2 x double> %84, i32 0
  %93 = fsub double %91, %92
  %94 = extractelement <2 x double> %81, i32 1
  %95 = extractelement <2 x double> %84, i32 1
  %96 = fsub double %94, %95
  %97 = fmul double %60, -1.000000e+00
  %98 = fsub double %57, %97
  %99 = fmul double %57, -1.000000e+00
  %100 = fadd double %99, %60
  %101 = fmul double %98, 0x3FE6A09E667F3BCD
  %102 = fmul double %100, 0x3FE6A09E667F3BCD
  %103 = fmul double %75, 0.000000e+00
  %104 = fmul double %78, -1.000000e+00
  %105 = fsub double %103, %104
  %106 = fmul double %75, -1.000000e+00
  %107 = fmul double %78, 0.000000e+00
  %108 = fadd double %106, %107
  %109 = fmul double %93, -1.000000e+00
  %110 = fmul double %96, -1.000000e+00
  %111 = fsub double %109, %110
  %112 = fmul double %93, -1.000000e+00
  %113 = fmul double %96, -1.000000e+00
  %114 = fadd double %112, %113
  %115 = fmul double %111, 0x3FE6A09E667F3BCD
  %116 = fmul double %114, 0x3FE6A09E667F3BCD
  %117 = fadd double %33, %69
  %118 = fadd double %36, %72
  %119 = fsub double %33, %69
  %120 = fsub double %36, %72
  %121 = fadd double %51, %87
  %122 = fadd double %54, %90
  %123 = fsub double %51, %87
  %124 = fsub double %54, %90
  %125 = fmul double %123, 0.000000e+00
  %126 = fmul double %124, -1.000000e+00
  %127 = fsub double %125, %126
  %128 = fmul double %123, -1.000000e+00
  %129 = fmul double %124, 0.000000e+00
  %130 = fadd double %128, %129
  %131 = fadd double %117, %121
  %132 = insertelement <2 x double> undef, double %131, i32 0
  %133 = fadd double %118, %122
  %134 = insertelement <2 x double> %132, double %133, i32 1
  store <2 x double> %134, <2 x double>* %11, align 16
  %135 = fsub double %117, %121
  %136 = insertelement <2 x double> undef, double %135, i32 0
  %137 = fsub double %118, %122
  %138 = insertelement <2 x double> %136, double %137, i32 1
  store <2 x double> %138, <2 x double>* %44, align 16
  %139 = fadd double %119, %127
  %140 = insertelement <2 x double> undef, double %139, i32 0
  %141 = fadd double %120, %130
  %142 = insertelement <2 x double> %140, double %141, i32 1
  store <2 x double> %142, <2 x double>* %62, align 16
  %143 = fsub double %119, %127
  %144 = insertelement <2 x double> undef, double %143, i32 0
  %145 = fsub double %120, %130
  %146 = insertelement <2 x double> %144, double %145, i32 1
  store <2 x double> %146, <2 x double>* %80, align 16
  %147 = fadd double %39, %105
  %148 = fadd double %42, %108
  %149 = fsub double %39, %105
  %150 = fsub double %42, %108
  %151 = fadd double %101, %115
  %152 = fadd double %102, %116
  %153 = fsub double %101, %115
  %154 = fsub double %102, %116
  %155 = fmul double %153, 0.000000e+00
  %156 = fmul double %154, -1.000000e+00
  %157 = fsub double %155, %156
  %158 = fmul double %153, -1.000000e+00
  %159 = fmul double %154, 0.000000e+00
  %160 = fadd double %158, %159
  %161 = fadd double %147, %151
  %162 = insertelement <2 x double> undef, double %161, i32 0
  %163 = fadd double %148, %152
  %164 = insertelement <2 x double> %162, double %163, i32 1
  store <2 x double> %164, <2 x double>* %29, align 16
  %165 = fsub double %147, %151
  %166 = insertelement <2 x double> undef, double %165, i32 0
  %167 = fsub double %148, %152
  %168 = insertelement <2 x double> %166, double %167, i32 1
  store <2 x double> %168, <2 x double>* %47, align 16
  %169 = fadd double %149, %157
  %170 = insertelement <2 x double> undef, double %169, i32 0
  %171 = fadd double %150, %160
  %172 = insertelement <2 x double> %170, double %171, i32 1
  store <2 x double> %172, <2 x double>* %65, align 16
  %173 = fsub double %149, %157
  %174 = insertelement <2 x double> undef, double %173, i32 0
  %175 = fsub double %150, %160
  %176 = insertelement <2 x double> %174, double %175, i32 1
  store <2 x double> %176, <2 x double>* %83, align 16
  %177 = sitofp i32 %3 to double
  %"&(pSB[currWI].offset)106" = add nuw i64 %CurrSBIndex..1, 48
  %"&pSB[currWI].offset107" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)106"
  %CastToValueType108 = bitcast i8* %"&pSB[currWI].offset107" to [8 x <2 x double>]*
  br label %178

; <label>:178                                     ; preds = %._crit_edge13, %SyncBB193
  %indvar6 = phi i64 [ 1, %SyncBB193 ], [ %phitmp, %._crit_edge13 ]
  %scevgep10 = getelementptr [8 x <2 x double>]* %CastToValueType108, i64 0, i64 %indvar6
  %scevgep11 = getelementptr [8 x i32]* @fft1D_512.reversed8, i64 0, i64 %indvar6
  %179 = load <2 x double>* %scevgep10, align 16
  %180 = load i32* %scevgep11, align 4
  %181 = sitofp i32 %180 to double
  %182 = fmul double %181, 0xC01921FB54442D18
  %183 = fdiv double %182, 5.120000e+02
  %184 = fmul double %183, %177
  %185 = call double @_Z3cosd(double %184) nounwind
  %186 = call double @_Z3sind(double %184) nounwind
  %187 = extractelement <2 x double> %179, i32 0
  %188 = fmul double %187, %185
  %189 = extractelement <2 x double> %179, i32 1
  %190 = fmul double %189, %186
  %191 = fsub double %188, %190
  %192 = insertelement <2 x double> undef, double %191, i32 0
  %193 = fmul double %187, %186
  %194 = fmul double %189, %185
  %195 = fadd double %193, %194
  %196 = insertelement <2 x double> %192, double %195, i32 1
  store <2 x double> %196, <2 x double>* %scevgep10, align 16
  %exitcond8 = icmp eq i64 %indvar6, 7
  br i1 %exitcond8, label %bb.nph, label %._crit_edge13

._crit_edge13:                                    ; preds = %178
  %phitmp = add i64 %indvar6, 1
  br label %178

bb.nph:                                           ; preds = %178
  %"&pSB[currWI].offset28" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType29 = bitcast i8* %"&pSB[currWI].offset28" to i32*
  %loadedValue30 = load i32* %CastToValueType29, align 4
  %197 = sext i32 %loadedValue30 to i64
  %198 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %197
  %"&(pSB[currWI].offset)6980" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset70" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)6980"
  %CastToValueType71 = bitcast i8* %"&pSB[currWI].offset70" to double addrspace(3)**
  store double addrspace(3)* %198, double addrspace(3)** %CastToValueType71, align 8
  %199 = load <2 x double>* %11, align 16
  %200 = extractelement <2 x double> %199, i32 0
  store double %200, double addrspace(3)* %198, align 8
  %"&pSB[currWI].offset143.sum81" = add i64 %CurrSBIndex..1, 112
  %201 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum81"
  %202 = bitcast i8* %201 to <2 x double>*
  %203 = load <2 x double>* %202, align 16
  %204 = extractelement <2 x double> %203, i32 0
  %.sum82 = add i64 %197, 66
  %205 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum82
  store double %204, double addrspace(3)* %205, align 8
  %"&pSB[currWI].offset143.sum83" = add i64 %CurrSBIndex..1, 80
  %206 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum83"
  %207 = bitcast i8* %206 to <2 x double>*
  %208 = load <2 x double>* %207, align 16
  %209 = extractelement <2 x double> %208, i32 0
  %.sum84 = add i64 %197, 132
  %210 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum84
  store double %209, double addrspace(3)* %210, align 8
  %"&pSB[currWI].offset143.sum85" = add i64 %CurrSBIndex..1, 144
  %211 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum85"
  %212 = bitcast i8* %211 to <2 x double>*
  %213 = load <2 x double>* %212, align 16
  %214 = extractelement <2 x double> %213, i32 0
  %.sum86 = add i64 %197, 198
  %215 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum86
  store double %214, double addrspace(3)* %215, align 8
  %"&pSB[currWI].offset143.sum87" = add i64 %CurrSBIndex..1, 64
  %216 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum87"
  %217 = bitcast i8* %216 to <2 x double>*
  %218 = load <2 x double>* %217, align 16
  %219 = extractelement <2 x double> %218, i32 0
  %.sum88 = add i64 %197, 264
  %220 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum88
  store double %219, double addrspace(3)* %220, align 8
  %"&pSB[currWI].offset143.sum89" = add i64 %CurrSBIndex..1, 128
  %221 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum89"
  %222 = bitcast i8* %221 to <2 x double>*
  %223 = load <2 x double>* %222, align 16
  %224 = extractelement <2 x double> %223, i32 0
  %.sum90 = add i64 %197, 330
  %225 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum90
  store double %224, double addrspace(3)* %225, align 8
  %"&pSB[currWI].offset143.sum91" = add i64 %CurrSBIndex..1, 96
  %226 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum91"
  %227 = bitcast i8* %226 to <2 x double>*
  %228 = load <2 x double>* %227, align 16
  %229 = extractelement <2 x double> %228, i32 0
  %.sum92 = add i64 %197, 396
  %230 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum92
  store double %229, double addrspace(3)* %230, align 8
  %"&pSB[currWI].offset143.sum93" = add i64 %CurrSBIndex..1, 160
  %231 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum93"
  %232 = bitcast i8* %231 to <2 x double>*
  %233 = load <2 x double>* %232, align 16
  %234 = extractelement <2 x double> %233, i32 0
  %.sum94 = add i64 %197, 462
  %235 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum94
  store double %234, double addrspace(3)* %235, align 8
  %loadedCurrWI197 = load i64* %pCurrWI, align 8
  %check.WI.iter198 = icmp ult i64 %loadedCurrWI197, %iterCount
  br i1 %check.WI.iter198, label %thenBB195, label %elseBB196

thenBB195:                                        ; preds = %bb.nph
  %"CurrWI++199" = add nuw i64 %loadedCurrWI197, 1
  store i64 %"CurrWI++199", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride201" = add nuw i64 %CurrSBIndex..1, 608
  br label %SyncBB193

elseBB196:                                        ; preds = %bb.nph
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB186

SyncBB186:                                        ; preds = %thenBB202, %elseBB196
  %CurrSBIndex..2 = phi i64 [ 0, %elseBB196 ], [ %"loadedCurrSB+Stride208", %thenBB202 ]
  %"&(pSB[currWI].offset)5595" = or i64 %CurrSBIndex..2, 8
  %"&pSB[currWI].offset56" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)5595"
  %CastToValueType57 = bitcast i8* %"&pSB[currWI].offset56" to i32*
  %loadedValue58 = load i32* %CastToValueType57, align 4
  %236 = mul nsw i32 %loadedValue58, 66
  %"&(pSB[currWI].offset)3696" = or i64 %CurrSBIndex..2, 4
  %"&pSB[currWI].offset37" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)3696"
  %CastToValueType38 = bitcast i8* %"&pSB[currWI].offset37" to i32*
  %loadedValue39 = load i32* %CastToValueType38, align 4
  %237 = add nsw i32 %236, %loadedValue39
  %238 = sext i32 %237 to i64
  %239 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %238
  %"&(pSB[currWI].offset)88" = add nuw i64 %CurrSBIndex..2, 32
  %"&pSB[currWI].offset89" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)88"
  %CastToValueType90 = bitcast i8* %"&pSB[currWI].offset89" to double addrspace(3)**
  store double addrspace(3)* %239, double addrspace(3)** %CastToValueType90, align 8
  %240 = load double addrspace(3)* %239, align 8
  %241 = load <2 x double>* %11, align 16
  %242 = insertelement <2 x double> %241, double %240, i32 0
  store <2 x double> %242, <2 x double>* %11, align 16
  %"&pSB[currWI].offset143.sum97" = add i64 %CurrSBIndex..1, 64
  %scevgep.1.i51 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum97"
  %243 = bitcast i8* %scevgep.1.i51 to <2 x double>*
  %.sum98 = add i64 %238, 8
  %scevgep2.1.i52 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum98
  %244 = load double addrspace(3)* %scevgep2.1.i52, align 8
  %245 = load <2 x double>* %243, align 16
  %246 = insertelement <2 x double> %245, double %244, i32 0
  store <2 x double> %246, <2 x double>* %243, align 16
  %"&pSB[currWI].offset143.sum99" = add i64 %CurrSBIndex..1, 80
  %scevgep.2.i53 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum99"
  %247 = bitcast i8* %scevgep.2.i53 to <2 x double>*
  %.sum100 = add i64 %238, 16
  %scevgep2.2.i54 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum100
  %248 = load double addrspace(3)* %scevgep2.2.i54, align 8
  %249 = load <2 x double>* %247, align 16
  %250 = insertelement <2 x double> %249, double %248, i32 0
  store <2 x double> %250, <2 x double>* %247, align 16
  %"&pSB[currWI].offset143.sum101" = add i64 %CurrSBIndex..1, 96
  %scevgep.3.i55 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum101"
  %251 = bitcast i8* %scevgep.3.i55 to <2 x double>*
  %.sum102 = add i64 %238, 24
  %scevgep2.3.i56 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum102
  %252 = load double addrspace(3)* %scevgep2.3.i56, align 8
  %253 = load <2 x double>* %251, align 16
  %254 = insertelement <2 x double> %253, double %252, i32 0
  store <2 x double> %254, <2 x double>* %251, align 16
  %"&pSB[currWI].offset143.sum103" = add i64 %CurrSBIndex..1, 112
  %scevgep.4.i57 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum103"
  %255 = bitcast i8* %scevgep.4.i57 to <2 x double>*
  %.sum104 = add i64 %238, 32
  %scevgep2.4.i58 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum104
  %256 = load double addrspace(3)* %scevgep2.4.i58, align 8
  %257 = load <2 x double>* %255, align 16
  %258 = insertelement <2 x double> %257, double %256, i32 0
  store <2 x double> %258, <2 x double>* %255, align 16
  %"&pSB[currWI].offset143.sum105" = add i64 %CurrSBIndex..1, 128
  %scevgep.5.i59 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum105"
  %259 = bitcast i8* %scevgep.5.i59 to <2 x double>*
  %.sum106 = add i64 %238, 40
  %scevgep2.5.i60 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum106
  %260 = load double addrspace(3)* %scevgep2.5.i60, align 8
  %261 = load <2 x double>* %259, align 16
  %262 = insertelement <2 x double> %261, double %260, i32 0
  store <2 x double> %262, <2 x double>* %259, align 16
  %"&pSB[currWI].offset143.sum107" = add i64 %CurrSBIndex..1, 144
  %scevgep.6.i61 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum107"
  %263 = bitcast i8* %scevgep.6.i61 to <2 x double>*
  %.sum108 = add i64 %238, 48
  %scevgep2.6.i62 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum108
  %264 = load double addrspace(3)* %scevgep2.6.i62, align 8
  %265 = load <2 x double>* %263, align 16
  %266 = insertelement <2 x double> %265, double %264, i32 0
  store <2 x double> %266, <2 x double>* %263, align 16
  %"&pSB[currWI].offset143.sum109" = add i64 %CurrSBIndex..1, 160
  %scevgep.7.i63 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum109"
  %267 = bitcast i8* %scevgep.7.i63 to <2 x double>*
  %.sum110 = add i64 %238, 56
  %scevgep2.7.i64 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum110
  %268 = load double addrspace(3)* %scevgep2.7.i64, align 8
  %269 = load <2 x double>* %267, align 16
  %270 = insertelement <2 x double> %269, double %268, i32 0
  store <2 x double> %270, <2 x double>* %267, align 16
  %loadedCurrWI204 = load i64* %pCurrWI, align 8
  %check.WI.iter205 = icmp ult i64 %loadedCurrWI204, %iterCount
  br i1 %check.WI.iter205, label %thenBB202, label %elseBB203

thenBB202:                                        ; preds = %SyncBB186
  %"CurrWI++206" = add nuw i64 %loadedCurrWI204, 1
  store i64 %"CurrWI++206", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride208" = add nuw i64 %CurrSBIndex..2, 608
  br label %SyncBB186

elseBB203:                                        ; preds = %SyncBB186
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB187

SyncBB187:                                        ; preds = %thenBB209, %elseBB203
  %CurrSBIndex..3 = phi i64 [ 0, %elseBB203 ], [ %"loadedCurrSB+Stride215", %thenBB209 ]
  %"&(pSB[currWI].offset)83111" = or i64 %CurrSBIndex..3, 24
  %"&pSB[currWI].offset84" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)83111"
  %CastToValueType85 = bitcast i8* %"&pSB[currWI].offset84" to double addrspace(3)**
  %loadedValue86 = load double addrspace(3)** %CastToValueType85, align 8
  %271 = load <2 x double>* %11, align 16
  %272 = extractelement <2 x double> %271, i32 1
  store double %272, double addrspace(3)* %loadedValue86, align 8
  %"&pSB[currWI].offset143.sum112" = add i64 %CurrSBIndex..1, 112
  %273 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum112"
  %274 = bitcast i8* %273 to <2 x double>*
  %275 = load <2 x double>* %274, align 16
  %276 = extractelement <2 x double> %275, i32 1
  %277 = getelementptr inbounds double addrspace(3)* %loadedValue86, i64 66
  store double %276, double addrspace(3)* %277, align 8
  %"&pSB[currWI].offset143.sum113" = add i64 %CurrSBIndex..1, 80
  %278 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum113"
  %279 = bitcast i8* %278 to <2 x double>*
  %280 = load <2 x double>* %279, align 16
  %281 = extractelement <2 x double> %280, i32 1
  %282 = getelementptr inbounds double addrspace(3)* %loadedValue86, i64 132
  store double %281, double addrspace(3)* %282, align 8
  %"&pSB[currWI].offset143.sum114" = add i64 %CurrSBIndex..1, 144
  %283 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum114"
  %284 = bitcast i8* %283 to <2 x double>*
  %285 = load <2 x double>* %284, align 16
  %286 = extractelement <2 x double> %285, i32 1
  %287 = getelementptr inbounds double addrspace(3)* %loadedValue86, i64 198
  store double %286, double addrspace(3)* %287, align 8
  %"&pSB[currWI].offset143.sum115" = add i64 %CurrSBIndex..1, 64
  %288 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum115"
  %289 = bitcast i8* %288 to <2 x double>*
  %290 = load <2 x double>* %289, align 16
  %291 = extractelement <2 x double> %290, i32 1
  %292 = getelementptr inbounds double addrspace(3)* %loadedValue86, i64 264
  store double %291, double addrspace(3)* %292, align 8
  %"&pSB[currWI].offset143.sum116" = add i64 %CurrSBIndex..1, 128
  %293 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum116"
  %294 = bitcast i8* %293 to <2 x double>*
  %295 = load <2 x double>* %294, align 16
  %296 = extractelement <2 x double> %295, i32 1
  %297 = getelementptr inbounds double addrspace(3)* %loadedValue86, i64 330
  store double %296, double addrspace(3)* %297, align 8
  %"&pSB[currWI].offset143.sum117" = add i64 %CurrSBIndex..1, 96
  %298 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum117"
  %299 = bitcast i8* %298 to <2 x double>*
  %300 = load <2 x double>* %299, align 16
  %301 = extractelement <2 x double> %300, i32 1
  %302 = getelementptr inbounds double addrspace(3)* %loadedValue86, i64 396
  store double %301, double addrspace(3)* %302, align 8
  %"&pSB[currWI].offset143.sum118" = add i64 %CurrSBIndex..1, 160
  %303 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum118"
  %304 = bitcast i8* %303 to <2 x double>*
  %305 = load <2 x double>* %304, align 16
  %306 = extractelement <2 x double> %305, i32 1
  %307 = getelementptr inbounds double addrspace(3)* %loadedValue86, i64 462
  store double %306, double addrspace(3)* %307, align 8
  %loadedCurrWI211 = load i64* %pCurrWI, align 8
  %check.WI.iter212 = icmp ult i64 %loadedCurrWI211, %iterCount
  br i1 %check.WI.iter212, label %thenBB209, label %elseBB210

thenBB209:                                        ; preds = %SyncBB187
  %"CurrWI++213" = add nuw i64 %loadedCurrWI211, 1
  store i64 %"CurrWI++213", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride215" = add nuw i64 %CurrSBIndex..3, 608
  br label %SyncBB187

elseBB210:                                        ; preds = %SyncBB187
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB188

SyncBB188:                                        ; preds = %thenBB216, %elseBB210
  %CurrSBIndex..4 = phi i64 [ 0, %elseBB210 ], [ %"loadedCurrSB+Stride222", %thenBB216 ]
  %"&(pSB[currWI].offset)92" = add nuw i64 %CurrSBIndex..4, 32
  %"&pSB[currWI].offset93" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)92"
  %CastToValueType94 = bitcast i8* %"&pSB[currWI].offset93" to double addrspace(3)**
  %loadedValue95 = load double addrspace(3)** %CastToValueType94, align 8
  %308 = load double addrspace(3)* %loadedValue95, align 8
  %309 = load <2 x double>* %11, align 16
  %310 = insertelement <2 x double> %309, double %308, i32 1
  store <2 x double> %310, <2 x double>* %11, align 16
  %"&pSB[currWI].offset143.sum119" = add i64 %CurrSBIndex..1, 64
  %scevgep.1.i37 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum119"
  %311 = bitcast i8* %scevgep.1.i37 to <2 x double>*
  %scevgep2.1.i38 = getelementptr double addrspace(3)* %loadedValue95, i64 8
  %312 = load double addrspace(3)* %scevgep2.1.i38, align 8
  %313 = load <2 x double>* %311, align 16
  %314 = insertelement <2 x double> %313, double %312, i32 1
  store <2 x double> %314, <2 x double>* %311, align 16
  %"&pSB[currWI].offset143.sum120" = add i64 %CurrSBIndex..1, 80
  %scevgep.2.i39 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum120"
  %315 = bitcast i8* %scevgep.2.i39 to <2 x double>*
  %scevgep2.2.i40 = getelementptr double addrspace(3)* %loadedValue95, i64 16
  %316 = load double addrspace(3)* %scevgep2.2.i40, align 8
  %317 = load <2 x double>* %315, align 16
  %318 = insertelement <2 x double> %317, double %316, i32 1
  store <2 x double> %318, <2 x double>* %315, align 16
  %"&pSB[currWI].offset143.sum121" = add i64 %CurrSBIndex..1, 96
  %scevgep.3.i41 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum121"
  %319 = bitcast i8* %scevgep.3.i41 to <2 x double>*
  %scevgep2.3.i42 = getelementptr double addrspace(3)* %loadedValue95, i64 24
  %320 = load double addrspace(3)* %scevgep2.3.i42, align 8
  %321 = load <2 x double>* %319, align 16
  %322 = insertelement <2 x double> %321, double %320, i32 1
  store <2 x double> %322, <2 x double>* %319, align 16
  %"&pSB[currWI].offset143.sum122" = add i64 %CurrSBIndex..1, 112
  %scevgep.4.i43 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum122"
  %323 = bitcast i8* %scevgep.4.i43 to <2 x double>*
  %scevgep2.4.i44 = getelementptr double addrspace(3)* %loadedValue95, i64 32
  %324 = load double addrspace(3)* %scevgep2.4.i44, align 8
  %325 = load <2 x double>* %323, align 16
  %326 = insertelement <2 x double> %325, double %324, i32 1
  store <2 x double> %326, <2 x double>* %323, align 16
  %"&pSB[currWI].offset143.sum123" = add i64 %CurrSBIndex..1, 128
  %scevgep.5.i45 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum123"
  %327 = bitcast i8* %scevgep.5.i45 to <2 x double>*
  %scevgep2.5.i46 = getelementptr double addrspace(3)* %loadedValue95, i64 40
  %328 = load double addrspace(3)* %scevgep2.5.i46, align 8
  %329 = load <2 x double>* %327, align 16
  %330 = insertelement <2 x double> %329, double %328, i32 1
  store <2 x double> %330, <2 x double>* %327, align 16
  %"&pSB[currWI].offset143.sum124" = add i64 %CurrSBIndex..1, 144
  %scevgep.6.i47 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum124"
  %331 = bitcast i8* %scevgep.6.i47 to <2 x double>*
  %scevgep2.6.i48 = getelementptr double addrspace(3)* %loadedValue95, i64 48
  %332 = load double addrspace(3)* %scevgep2.6.i48, align 8
  %333 = load <2 x double>* %331, align 16
  %334 = insertelement <2 x double> %333, double %332, i32 1
  store <2 x double> %334, <2 x double>* %331, align 16
  %"&pSB[currWI].offset143.sum125" = add i64 %CurrSBIndex..1, 160
  %scevgep.7.i49 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum125"
  %335 = bitcast i8* %scevgep.7.i49 to <2 x double>*
  %scevgep2.7.i50 = getelementptr double addrspace(3)* %loadedValue95, i64 56
  %336 = load double addrspace(3)* %scevgep2.7.i50, align 8
  %337 = load <2 x double>* %335, align 16
  %338 = insertelement <2 x double> %337, double %336, i32 1
  store <2 x double> %338, <2 x double>* %335, align 16
  %loadedCurrWI218 = load i64* %pCurrWI, align 8
  %check.WI.iter219 = icmp ult i64 %loadedCurrWI218, %iterCount
  br i1 %check.WI.iter219, label %thenBB216, label %elseBB217

thenBB216:                                        ; preds = %SyncBB188
  %"CurrWI++220" = add nuw i64 %loadedCurrWI218, 1
  store i64 %"CurrWI++220", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride222" = add nuw i64 %CurrSBIndex..4, 608
  br label %SyncBB188

elseBB217:                                        ; preds = %SyncBB188
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB189

SyncBB189:                                        ; preds = %thenBB, %elseBB217
  %CurrSBIndex..0 = phi i64 [ 0, %elseBB217 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %339 = load <2 x double>* %11, align 16
  %340 = load <2 x double>* %29, align 16
  %341 = extractelement <2 x double> %339, i32 0
  %342 = extractelement <2 x double> %340, i32 0
  %343 = fadd double %341, %342
  %344 = extractelement <2 x double> %339, i32 1
  %345 = extractelement <2 x double> %340, i32 1
  %346 = fadd double %344, %345
  %347 = extractelement <2 x double> %339, i32 0
  %348 = extractelement <2 x double> %340, i32 0
  %349 = fsub double %347, %348
  %350 = extractelement <2 x double> %339, i32 1
  %351 = extractelement <2 x double> %340, i32 1
  %352 = fsub double %350, %351
  %353 = load <2 x double>* %44, align 16
  %354 = load <2 x double>* %47, align 16
  %355 = extractelement <2 x double> %353, i32 0
  %356 = extractelement <2 x double> %354, i32 0
  %357 = fadd double %355, %356
  %358 = extractelement <2 x double> %353, i32 1
  %359 = extractelement <2 x double> %354, i32 1
  %360 = fadd double %358, %359
  %361 = extractelement <2 x double> %353, i32 0
  %362 = extractelement <2 x double> %354, i32 0
  %363 = fsub double %361, %362
  %364 = extractelement <2 x double> %353, i32 1
  %365 = extractelement <2 x double> %354, i32 1
  %366 = fsub double %364, %365
  %367 = load <2 x double>* %62, align 16
  %368 = load <2 x double>* %65, align 16
  %369 = extractelement <2 x double> %367, i32 0
  %370 = extractelement <2 x double> %368, i32 0
  %371 = fadd double %369, %370
  %372 = extractelement <2 x double> %367, i32 1
  %373 = extractelement <2 x double> %368, i32 1
  %374 = fadd double %372, %373
  %375 = extractelement <2 x double> %367, i32 0
  %376 = extractelement <2 x double> %368, i32 0
  %377 = fsub double %375, %376
  %378 = extractelement <2 x double> %367, i32 1
  %379 = extractelement <2 x double> %368, i32 1
  %380 = fsub double %378, %379
  %381 = load <2 x double>* %80, align 16
  %382 = load <2 x double>* %83, align 16
  %383 = extractelement <2 x double> %381, i32 0
  %384 = extractelement <2 x double> %382, i32 0
  %385 = fadd double %383, %384
  %386 = extractelement <2 x double> %381, i32 1
  %387 = extractelement <2 x double> %382, i32 1
  %388 = fadd double %386, %387
  %389 = extractelement <2 x double> %381, i32 0
  %390 = extractelement <2 x double> %382, i32 0
  %391 = fsub double %389, %390
  %392 = extractelement <2 x double> %381, i32 1
  %393 = extractelement <2 x double> %382, i32 1
  %394 = fsub double %392, %393
  %395 = fmul double %366, -1.000000e+00
  %396 = fsub double %363, %395
  %397 = fmul double %363, -1.000000e+00
  %398 = fadd double %397, %366
  %399 = fmul double %396, 0x3FE6A09E667F3BCD
  %400 = fmul double %398, 0x3FE6A09E667F3BCD
  %401 = fmul double %377, 0.000000e+00
  %402 = fmul double %380, -1.000000e+00
  %403 = fsub double %401, %402
  %404 = fmul double %377, -1.000000e+00
  %405 = fmul double %380, 0.000000e+00
  %406 = fadd double %404, %405
  %407 = fmul double %391, -1.000000e+00
  %408 = fmul double %394, -1.000000e+00
  %409 = fsub double %407, %408
  %410 = fmul double %391, -1.000000e+00
  %411 = fmul double %394, -1.000000e+00
  %412 = fadd double %410, %411
  %413 = fmul double %409, 0x3FE6A09E667F3BCD
  %414 = fmul double %412, 0x3FE6A09E667F3BCD
  %415 = fadd double %343, %371
  %416 = fadd double %346, %374
  %417 = fsub double %343, %371
  %418 = fsub double %346, %374
  %419 = fadd double %357, %385
  %420 = fadd double %360, %388
  %421 = fsub double %357, %385
  %422 = fsub double %360, %388
  %423 = fmul double %421, 0.000000e+00
  %424 = fmul double %422, -1.000000e+00
  %425 = fsub double %423, %424
  %426 = fmul double %421, -1.000000e+00
  %427 = fmul double %422, 0.000000e+00
  %428 = fadd double %426, %427
  %429 = fadd double %415, %419
  %430 = insertelement <2 x double> undef, double %429, i32 0
  %431 = fadd double %416, %420
  %432 = insertelement <2 x double> %430, double %431, i32 1
  store <2 x double> %432, <2 x double>* %11, align 16
  %433 = fsub double %415, %419
  %434 = insertelement <2 x double> undef, double %433, i32 0
  %435 = fsub double %416, %420
  %436 = insertelement <2 x double> %434, double %435, i32 1
  store <2 x double> %436, <2 x double>* %44, align 16
  %437 = fadd double %417, %425
  %438 = insertelement <2 x double> undef, double %437, i32 0
  %439 = fadd double %418, %428
  %440 = insertelement <2 x double> %438, double %439, i32 1
  store <2 x double> %440, <2 x double>* %62, align 16
  %441 = fsub double %417, %425
  %442 = insertelement <2 x double> undef, double %441, i32 0
  %443 = fsub double %418, %428
  %444 = insertelement <2 x double> %442, double %443, i32 1
  store <2 x double> %444, <2 x double>* %80, align 16
  %445 = fadd double %349, %403
  %446 = fadd double %352, %406
  %447 = fsub double %349, %403
  %448 = fsub double %352, %406
  %449 = fadd double %399, %413
  %450 = fadd double %400, %414
  %451 = fsub double %399, %413
  %452 = fsub double %400, %414
  %453 = fmul double %451, 0.000000e+00
  %454 = fmul double %452, -1.000000e+00
  %455 = fsub double %453, %454
  %456 = fmul double %451, -1.000000e+00
  %457 = fmul double %452, 0.000000e+00
  %458 = fadd double %456, %457
  %459 = fadd double %445, %449
  %460 = insertelement <2 x double> undef, double %459, i32 0
  %461 = fadd double %446, %450
  %462 = insertelement <2 x double> %460, double %461, i32 1
  store <2 x double> %462, <2 x double>* %29, align 16
  %463 = fsub double %445, %449
  %464 = insertelement <2 x double> undef, double %463, i32 0
  %465 = fsub double %446, %450
  %466 = insertelement <2 x double> %464, double %465, i32 1
  store <2 x double> %466, <2 x double>* %47, align 16
  %467 = fadd double %447, %455
  %468 = insertelement <2 x double> undef, double %467, i32 0
  %469 = fadd double %448, %458
  %470 = insertelement <2 x double> %468, double %469, i32 1
  store <2 x double> %470, <2 x double>* %65, align 16
  %471 = fsub double %447, %455
  %472 = insertelement <2 x double> undef, double %471, i32 0
  %473 = fsub double %448, %458
  %474 = insertelement <2 x double> %472, double %473, i32 1
  store <2 x double> %474, <2 x double>* %83, align 16
  %"&(pSB[currWI].offset)41126" = or i64 %CurrSBIndex..0, 4
  %"&pSB[currWI].offset42" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)41126"
  %CastToValueType43 = bitcast i8* %"&pSB[currWI].offset42" to i32*
  %loadedValue44 = load i32* %CastToValueType43, align 4
  %475 = sitofp i32 %loadedValue44 to double
  %"&(pSB[currWI].offset)110" = add nuw i64 %CurrSBIndex..0, 48
  %"&pSB[currWI].offset111" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)110"
  %CastToValueType112 = bitcast i8* %"&pSB[currWI].offset111" to [8 x <2 x double>]*
  br label %476

; <label>:476                                     ; preds = %._crit_edge12, %SyncBB189
  %indvar = phi i64 [ 1, %SyncBB189 ], [ %phitmp14, %._crit_edge12 ]
  %scevgep = getelementptr [8 x <2 x double>]* %CastToValueType112, i64 0, i64 %indvar
  %scevgep5 = getelementptr [8 x i32]* @fft1D_512.reversed8, i64 0, i64 %indvar
  %477 = load <2 x double>* %scevgep, align 16
  %478 = load i32* %scevgep5, align 4
  %479 = sitofp i32 %478 to double
  %480 = fmul double %479, 0xC01921FB54442D18
  %481 = fdiv double %480, 6.400000e+01
  %482 = fmul double %481, %475
  %483 = call double @_Z3cosd(double %482) nounwind
  %484 = call double @_Z3sind(double %482) nounwind
  %485 = extractelement <2 x double> %477, i32 0
  %486 = fmul double %485, %483
  %487 = extractelement <2 x double> %477, i32 1
  %488 = fmul double %487, %484
  %489 = fsub double %486, %488
  %490 = insertelement <2 x double> undef, double %489, i32 0
  %491 = fmul double %485, %484
  %492 = fmul double %487, %483
  %493 = fadd double %491, %492
  %494 = insertelement <2 x double> %490, double %493, i32 1
  store <2 x double> %494, <2 x double>* %scevgep, align 16
  %exitcond = icmp eq i64 %indvar, 7
  br i1 %exitcond, label %._crit_edge, label %._crit_edge12

._crit_edge12:                                    ; preds = %476
  %phitmp14 = add i64 %indvar, 1
  br label %476

._crit_edge:                                      ; preds = %476
  %"&(pSB[currWI].offset)78127" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset79" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)78127"
  %CastToValueType80 = bitcast i8* %"&pSB[currWI].offset79" to double addrspace(3)**
  %loadedValue81 = load double addrspace(3)** %CastToValueType80, align 8
  %495 = load <2 x double>* %11, align 16
  %496 = extractelement <2 x double> %495, i32 0
  store double %496, double addrspace(3)* %loadedValue81, align 8
  %"&pSB[currWI].offset143.sum128" = add i64 %CurrSBIndex..1, 112
  %497 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum128"
  %498 = bitcast i8* %497 to <2 x double>*
  %499 = load <2 x double>* %498, align 16
  %500 = extractelement <2 x double> %499, i32 0
  %501 = getelementptr inbounds double addrspace(3)* %loadedValue81, i64 72
  store double %500, double addrspace(3)* %501, align 8
  %"&pSB[currWI].offset143.sum129" = add i64 %CurrSBIndex..1, 80
  %502 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum129"
  %503 = bitcast i8* %502 to <2 x double>*
  %504 = load <2 x double>* %503, align 16
  %505 = extractelement <2 x double> %504, i32 0
  %506 = getelementptr inbounds double addrspace(3)* %loadedValue81, i64 144
  store double %505, double addrspace(3)* %506, align 8
  %"&pSB[currWI].offset143.sum130" = add i64 %CurrSBIndex..1, 144
  %507 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum130"
  %508 = bitcast i8* %507 to <2 x double>*
  %509 = load <2 x double>* %508, align 16
  %510 = extractelement <2 x double> %509, i32 0
  %511 = getelementptr inbounds double addrspace(3)* %loadedValue81, i64 216
  store double %510, double addrspace(3)* %511, align 8
  %"&pSB[currWI].offset143.sum131" = add i64 %CurrSBIndex..1, 64
  %512 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum131"
  %513 = bitcast i8* %512 to <2 x double>*
  %514 = load <2 x double>* %513, align 16
  %515 = extractelement <2 x double> %514, i32 0
  %516 = getelementptr inbounds double addrspace(3)* %loadedValue81, i64 288
  store double %515, double addrspace(3)* %516, align 8
  %"&pSB[currWI].offset143.sum132" = add i64 %CurrSBIndex..1, 128
  %517 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum132"
  %518 = bitcast i8* %517 to <2 x double>*
  %519 = load <2 x double>* %518, align 16
  %520 = extractelement <2 x double> %519, i32 0
  %521 = getelementptr inbounds double addrspace(3)* %loadedValue81, i64 360
  store double %520, double addrspace(3)* %521, align 8
  %"&pSB[currWI].offset143.sum133" = add i64 %CurrSBIndex..1, 96
  %522 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum133"
  %523 = bitcast i8* %522 to <2 x double>*
  %524 = load <2 x double>* %523, align 16
  %525 = extractelement <2 x double> %524, i32 0
  %526 = getelementptr inbounds double addrspace(3)* %loadedValue81, i64 432
  store double %525, double addrspace(3)* %526, align 8
  %"&pSB[currWI].offset143.sum134" = add i64 %CurrSBIndex..1, 160
  %527 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum134"
  %528 = bitcast i8* %527 to <2 x double>*
  %529 = load <2 x double>* %528, align 16
  %530 = extractelement <2 x double> %529, i32 0
  %531 = getelementptr inbounds double addrspace(3)* %loadedValue81, i64 504
  store double %530, double addrspace(3)* %531, align 8
  %loadedCurrWI = load i64* %pCurrWI, align 8
  %check.WI.iter = icmp ult i64 %loadedCurrWI, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %loadedCurrWI, 1
  store i64 %"CurrWI++", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 608
  br label %SyncBB189

elseBB:                                           ; preds = %._crit_edge
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB223, %elseBB
  %CurrSBIndex..5 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride229", %thenBB223 ]
  %"&pSB[currWI].offset24" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..5
  %CastToValueType25 = bitcast i8* %"&pSB[currWI].offset24" to i32*
  %loadedValue = load i32* %CastToValueType25, align 4
  %532 = and i32 %loadedValue, -8
  %533 = mul nsw i32 %532, 9
  %"&(pSB[currWI].offset)50135" = or i64 %CurrSBIndex..5, 8
  %"&pSB[currWI].offset51" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)50135"
  %CastToValueType52 = bitcast i8* %"&pSB[currWI].offset51" to i32*
  %loadedValue53 = load i32* %CastToValueType52, align 4
  %534 = or i32 %533, %loadedValue53
  %535 = sext i32 %534 to i64
  %536 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %535
  %"&(pSB[currWI].offset)97" = add nuw i64 %CurrSBIndex..5, 40
  %"&pSB[currWI].offset98" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)97"
  %CastToValueType99 = bitcast i8* %"&pSB[currWI].offset98" to double addrspace(3)**
  store double addrspace(3)* %536, double addrspace(3)** %CastToValueType99, align 8
  %537 = load double addrspace(3)* %536, align 8
  %538 = load <2 x double>* %11, align 16
  %539 = insertelement <2 x double> %538, double %537, i32 0
  store <2 x double> %539, <2 x double>* %11, align 16
  %"&pSB[currWI].offset143.sum136" = add i64 %CurrSBIndex..1, 64
  %scevgep.1.i23 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum136"
  %540 = bitcast i8* %scevgep.1.i23 to <2 x double>*
  %.sum137 = add i64 %535, 8
  %scevgep2.1.i24 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum137
  %541 = load double addrspace(3)* %scevgep2.1.i24, align 8
  %542 = load <2 x double>* %540, align 16
  %543 = insertelement <2 x double> %542, double %541, i32 0
  store <2 x double> %543, <2 x double>* %540, align 16
  %"&pSB[currWI].offset143.sum138" = add i64 %CurrSBIndex..1, 80
  %scevgep.2.i25 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum138"
  %544 = bitcast i8* %scevgep.2.i25 to <2 x double>*
  %.sum139 = add i64 %535, 16
  %scevgep2.2.i26 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum139
  %545 = load double addrspace(3)* %scevgep2.2.i26, align 8
  %546 = load <2 x double>* %544, align 16
  %547 = insertelement <2 x double> %546, double %545, i32 0
  store <2 x double> %547, <2 x double>* %544, align 16
  %"&pSB[currWI].offset143.sum140" = add i64 %CurrSBIndex..1, 96
  %scevgep.3.i27 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum140"
  %548 = bitcast i8* %scevgep.3.i27 to <2 x double>*
  %.sum141 = add i64 %535, 24
  %scevgep2.3.i28 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum141
  %549 = load double addrspace(3)* %scevgep2.3.i28, align 8
  %550 = load <2 x double>* %548, align 16
  %551 = insertelement <2 x double> %550, double %549, i32 0
  store <2 x double> %551, <2 x double>* %548, align 16
  %"&pSB[currWI].offset143.sum142" = add i64 %CurrSBIndex..1, 112
  %scevgep.4.i29 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum142"
  %552 = bitcast i8* %scevgep.4.i29 to <2 x double>*
  %.sum143 = add i64 %535, 32
  %scevgep2.4.i30 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum143
  %553 = load double addrspace(3)* %scevgep2.4.i30, align 8
  %554 = load <2 x double>* %552, align 16
  %555 = insertelement <2 x double> %554, double %553, i32 0
  store <2 x double> %555, <2 x double>* %552, align 16
  %"&pSB[currWI].offset143.sum144" = add i64 %CurrSBIndex..1, 128
  %scevgep.5.i31 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum144"
  %556 = bitcast i8* %scevgep.5.i31 to <2 x double>*
  %.sum145 = add i64 %535, 40
  %scevgep2.5.i32 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum145
  %557 = load double addrspace(3)* %scevgep2.5.i32, align 8
  %558 = load <2 x double>* %556, align 16
  %559 = insertelement <2 x double> %558, double %557, i32 0
  store <2 x double> %559, <2 x double>* %556, align 16
  %"&pSB[currWI].offset143.sum146" = add i64 %CurrSBIndex..1, 144
  %scevgep.6.i33 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum146"
  %560 = bitcast i8* %scevgep.6.i33 to <2 x double>*
  %.sum147 = add i64 %535, 48
  %scevgep2.6.i34 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum147
  %561 = load double addrspace(3)* %scevgep2.6.i34, align 8
  %562 = load <2 x double>* %560, align 16
  %563 = insertelement <2 x double> %562, double %561, i32 0
  store <2 x double> %563, <2 x double>* %560, align 16
  %"&pSB[currWI].offset143.sum148" = add i64 %CurrSBIndex..1, 160
  %scevgep.7.i35 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum148"
  %564 = bitcast i8* %scevgep.7.i35 to <2 x double>*
  %.sum149 = add i64 %535, 56
  %scevgep2.7.i36 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum149
  %565 = load double addrspace(3)* %scevgep2.7.i36, align 8
  %566 = load <2 x double>* %564, align 16
  %567 = insertelement <2 x double> %566, double %565, i32 0
  store <2 x double> %567, <2 x double>* %564, align 16
  %loadedCurrWI225 = load i64* %pCurrWI, align 8
  %check.WI.iter226 = icmp ult i64 %loadedCurrWI225, %iterCount
  br i1 %check.WI.iter226, label %thenBB223, label %elseBB224

thenBB223:                                        ; preds = %SyncBB
  %"CurrWI++227" = add nuw i64 %loadedCurrWI225, 1
  store i64 %"CurrWI++227", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride229" = add nuw i64 %CurrSBIndex..5, 608
  br label %SyncBB

elseBB224:                                        ; preds = %SyncBB
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB190

SyncBB190:                                        ; preds = %thenBB230, %elseBB224
  %CurrSBIndex..6 = phi i64 [ 0, %elseBB224 ], [ %"loadedCurrSB+Stride236", %thenBB230 ]
  %"&(pSB[currWI].offset)73150" = or i64 %CurrSBIndex..6, 24
  %"&pSB[currWI].offset74" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)73150"
  %CastToValueType75 = bitcast i8* %"&pSB[currWI].offset74" to double addrspace(3)**
  %loadedValue76 = load double addrspace(3)** %CastToValueType75, align 8
  %568 = load <2 x double>* %11, align 16
  %569 = extractelement <2 x double> %568, i32 1
  store double %569, double addrspace(3)* %loadedValue76, align 8
  %"&pSB[currWI].offset143.sum151" = add i64 %CurrSBIndex..1, 112
  %570 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum151"
  %571 = bitcast i8* %570 to <2 x double>*
  %572 = load <2 x double>* %571, align 16
  %573 = extractelement <2 x double> %572, i32 1
  %574 = getelementptr inbounds double addrspace(3)* %loadedValue76, i64 72
  store double %573, double addrspace(3)* %574, align 8
  %"&pSB[currWI].offset143.sum152" = add i64 %CurrSBIndex..1, 80
  %575 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum152"
  %576 = bitcast i8* %575 to <2 x double>*
  %577 = load <2 x double>* %576, align 16
  %578 = extractelement <2 x double> %577, i32 1
  %579 = getelementptr inbounds double addrspace(3)* %loadedValue76, i64 144
  store double %578, double addrspace(3)* %579, align 8
  %"&pSB[currWI].offset143.sum153" = add i64 %CurrSBIndex..1, 144
  %580 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum153"
  %581 = bitcast i8* %580 to <2 x double>*
  %582 = load <2 x double>* %581, align 16
  %583 = extractelement <2 x double> %582, i32 1
  %584 = getelementptr inbounds double addrspace(3)* %loadedValue76, i64 216
  store double %583, double addrspace(3)* %584, align 8
  %"&pSB[currWI].offset143.sum154" = add i64 %CurrSBIndex..1, 64
  %585 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum154"
  %586 = bitcast i8* %585 to <2 x double>*
  %587 = load <2 x double>* %586, align 16
  %588 = extractelement <2 x double> %587, i32 1
  %589 = getelementptr inbounds double addrspace(3)* %loadedValue76, i64 288
  store double %588, double addrspace(3)* %589, align 8
  %"&pSB[currWI].offset143.sum155" = add i64 %CurrSBIndex..1, 128
  %590 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum155"
  %591 = bitcast i8* %590 to <2 x double>*
  %592 = load <2 x double>* %591, align 16
  %593 = extractelement <2 x double> %592, i32 1
  %594 = getelementptr inbounds double addrspace(3)* %loadedValue76, i64 360
  store double %593, double addrspace(3)* %594, align 8
  %"&pSB[currWI].offset143.sum156" = add i64 %CurrSBIndex..1, 96
  %595 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum156"
  %596 = bitcast i8* %595 to <2 x double>*
  %597 = load <2 x double>* %596, align 16
  %598 = extractelement <2 x double> %597, i32 1
  %599 = getelementptr inbounds double addrspace(3)* %loadedValue76, i64 432
  store double %598, double addrspace(3)* %599, align 8
  %"&pSB[currWI].offset143.sum157" = add i64 %CurrSBIndex..1, 160
  %600 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum157"
  %601 = bitcast i8* %600 to <2 x double>*
  %602 = load <2 x double>* %601, align 16
  %603 = extractelement <2 x double> %602, i32 1
  %604 = getelementptr inbounds double addrspace(3)* %loadedValue76, i64 504
  store double %603, double addrspace(3)* %604, align 8
  %loadedCurrWI232 = load i64* %pCurrWI, align 8
  %check.WI.iter233 = icmp ult i64 %loadedCurrWI232, %iterCount
  br i1 %check.WI.iter233, label %thenBB230, label %elseBB231

thenBB230:                                        ; preds = %SyncBB190
  %"CurrWI++234" = add nuw i64 %loadedCurrWI232, 1
  store i64 %"CurrWI++234", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride236" = add nuw i64 %CurrSBIndex..6, 608
  br label %SyncBB190

elseBB231:                                        ; preds = %SyncBB190
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB191

SyncBB191:                                        ; preds = %thenBB237, %elseBB231
  %CurrSBIndex..7 = phi i64 [ 0, %elseBB231 ], [ %"loadedCurrSB+Stride243", %thenBB237 ]
  %"&(pSB[currWI].offset)101" = add nuw i64 %CurrSBIndex..7, 40
  %"&pSB[currWI].offset102" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)101"
  %CastToValueType103 = bitcast i8* %"&pSB[currWI].offset102" to double addrspace(3)**
  %loadedValue104 = load double addrspace(3)** %CastToValueType103, align 8
  %605 = load double addrspace(3)* %loadedValue104, align 8
  %606 = load <2 x double>* %11, align 16
  %607 = insertelement <2 x double> %606, double %605, i32 1
  store <2 x double> %607, <2 x double>* %11, align 16
  %"&pSB[currWI].offset143.sum158" = add i64 %CurrSBIndex..1, 64
  %scevgep.1.i9 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum158"
  %608 = bitcast i8* %scevgep.1.i9 to <2 x double>*
  %scevgep2.1.i10 = getelementptr double addrspace(3)* %loadedValue104, i64 8
  %609 = load double addrspace(3)* %scevgep2.1.i10, align 8
  %610 = load <2 x double>* %608, align 16
  %611 = insertelement <2 x double> %610, double %609, i32 1
  store <2 x double> %611, <2 x double>* %608, align 16
  %"&pSB[currWI].offset143.sum159" = add i64 %CurrSBIndex..1, 80
  %scevgep.2.i11 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum159"
  %612 = bitcast i8* %scevgep.2.i11 to <2 x double>*
  %scevgep2.2.i12 = getelementptr double addrspace(3)* %loadedValue104, i64 16
  %613 = load double addrspace(3)* %scevgep2.2.i12, align 8
  %614 = load <2 x double>* %612, align 16
  %615 = insertelement <2 x double> %614, double %613, i32 1
  store <2 x double> %615, <2 x double>* %612, align 16
  %"&pSB[currWI].offset143.sum160" = add i64 %CurrSBIndex..1, 96
  %scevgep.3.i13 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum160"
  %616 = bitcast i8* %scevgep.3.i13 to <2 x double>*
  %scevgep2.3.i14 = getelementptr double addrspace(3)* %loadedValue104, i64 24
  %617 = load double addrspace(3)* %scevgep2.3.i14, align 8
  %618 = load <2 x double>* %616, align 16
  %619 = insertelement <2 x double> %618, double %617, i32 1
  store <2 x double> %619, <2 x double>* %616, align 16
  %"&pSB[currWI].offset143.sum161" = add i64 %CurrSBIndex..1, 112
  %scevgep.4.i15 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum161"
  %620 = bitcast i8* %scevgep.4.i15 to <2 x double>*
  %scevgep2.4.i16 = getelementptr double addrspace(3)* %loadedValue104, i64 32
  %621 = load double addrspace(3)* %scevgep2.4.i16, align 8
  %622 = load <2 x double>* %620, align 16
  %623 = insertelement <2 x double> %622, double %621, i32 1
  store <2 x double> %623, <2 x double>* %620, align 16
  %"&pSB[currWI].offset143.sum162" = add i64 %CurrSBIndex..1, 128
  %scevgep.5.i17 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum162"
  %624 = bitcast i8* %scevgep.5.i17 to <2 x double>*
  %scevgep2.5.i18 = getelementptr double addrspace(3)* %loadedValue104, i64 40
  %625 = load double addrspace(3)* %scevgep2.5.i18, align 8
  %626 = load <2 x double>* %624, align 16
  %627 = insertelement <2 x double> %626, double %625, i32 1
  store <2 x double> %627, <2 x double>* %624, align 16
  %"&pSB[currWI].offset143.sum163" = add i64 %CurrSBIndex..1, 144
  %scevgep.6.i19 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum163"
  %628 = bitcast i8* %scevgep.6.i19 to <2 x double>*
  %scevgep2.6.i20 = getelementptr double addrspace(3)* %loadedValue104, i64 48
  %629 = load double addrspace(3)* %scevgep2.6.i20, align 8
  %630 = load <2 x double>* %628, align 16
  %631 = insertelement <2 x double> %630, double %629, i32 1
  store <2 x double> %631, <2 x double>* %628, align 16
  %"&pSB[currWI].offset143.sum164" = add i64 %CurrSBIndex..1, 160
  %scevgep.7.i21 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum164"
  %632 = bitcast i8* %scevgep.7.i21 to <2 x double>*
  %scevgep2.7.i22 = getelementptr double addrspace(3)* %loadedValue104, i64 56
  %633 = load double addrspace(3)* %scevgep2.7.i22, align 8
  %634 = load <2 x double>* %632, align 16
  %635 = insertelement <2 x double> %634, double %633, i32 1
  store <2 x double> %635, <2 x double>* %632, align 16
  %636 = load <2 x double>* %11, align 16
  %637 = load <2 x double>* %29, align 16
  %638 = extractelement <2 x double> %636, i32 0
  %639 = extractelement <2 x double> %637, i32 0
  %640 = fadd double %638, %639
  %641 = extractelement <2 x double> %636, i32 1
  %642 = extractelement <2 x double> %637, i32 1
  %643 = fadd double %641, %642
  %644 = extractelement <2 x double> %636, i32 0
  %645 = extractelement <2 x double> %637, i32 0
  %646 = fsub double %644, %645
  %647 = extractelement <2 x double> %636, i32 1
  %648 = extractelement <2 x double> %637, i32 1
  %649 = fsub double %647, %648
  %650 = load <2 x double>* %44, align 16
  %651 = load <2 x double>* %47, align 16
  %652 = extractelement <2 x double> %650, i32 0
  %653 = extractelement <2 x double> %651, i32 0
  %654 = fadd double %652, %653
  %655 = extractelement <2 x double> %650, i32 1
  %656 = extractelement <2 x double> %651, i32 1
  %657 = fadd double %655, %656
  %658 = extractelement <2 x double> %650, i32 0
  %659 = extractelement <2 x double> %651, i32 0
  %660 = fsub double %658, %659
  %661 = extractelement <2 x double> %650, i32 1
  %662 = extractelement <2 x double> %651, i32 1
  %663 = fsub double %661, %662
  %664 = load <2 x double>* %62, align 16
  %665 = load <2 x double>* %65, align 16
  %666 = extractelement <2 x double> %664, i32 0
  %667 = extractelement <2 x double> %665, i32 0
  %668 = fadd double %666, %667
  %669 = extractelement <2 x double> %664, i32 1
  %670 = extractelement <2 x double> %665, i32 1
  %671 = fadd double %669, %670
  %672 = extractelement <2 x double> %664, i32 0
  %673 = extractelement <2 x double> %665, i32 0
  %674 = fsub double %672, %673
  %675 = extractelement <2 x double> %664, i32 1
  %676 = extractelement <2 x double> %665, i32 1
  %677 = fsub double %675, %676
  %678 = load <2 x double>* %80, align 16
  %679 = load <2 x double>* %83, align 16
  %680 = extractelement <2 x double> %678, i32 0
  %681 = extractelement <2 x double> %679, i32 0
  %682 = fadd double %680, %681
  %683 = extractelement <2 x double> %678, i32 1
  %684 = extractelement <2 x double> %679, i32 1
  %685 = fadd double %683, %684
  %686 = extractelement <2 x double> %678, i32 0
  %687 = extractelement <2 x double> %679, i32 0
  %688 = fsub double %686, %687
  %689 = extractelement <2 x double> %678, i32 1
  %690 = extractelement <2 x double> %679, i32 1
  %691 = fsub double %689, %690
  %692 = fmul double %663, -1.000000e+00
  %693 = fsub double %660, %692
  %694 = fmul double %660, -1.000000e+00
  %695 = fadd double %694, %663
  %696 = fmul double %693, 0x3FE6A09E667F3BCD
  %697 = fmul double %695, 0x3FE6A09E667F3BCD
  %698 = fmul double %674, 0.000000e+00
  %699 = fmul double %677, -1.000000e+00
  %700 = fsub double %698, %699
  %701 = fmul double %674, -1.000000e+00
  %702 = fmul double %677, 0.000000e+00
  %703 = fadd double %701, %702
  %704 = fmul double %688, -1.000000e+00
  %705 = fmul double %691, -1.000000e+00
  %706 = fsub double %704, %705
  %707 = fmul double %688, -1.000000e+00
  %708 = fmul double %691, -1.000000e+00
  %709 = fadd double %707, %708
  %710 = fmul double %706, 0x3FE6A09E667F3BCD
  %711 = fmul double %709, 0x3FE6A09E667F3BCD
  %712 = fadd double %640, %668
  %713 = fadd double %643, %671
  %714 = fsub double %640, %668
  %715 = fsub double %643, %671
  %716 = fadd double %654, %682
  %717 = fadd double %657, %685
  %718 = fsub double %654, %682
  %719 = fsub double %657, %685
  %720 = fmul double %718, 0.000000e+00
  %721 = fmul double %719, -1.000000e+00
  %722 = fsub double %720, %721
  %723 = fmul double %718, -1.000000e+00
  %724 = fmul double %719, 0.000000e+00
  %725 = fadd double %723, %724
  %726 = fadd double %712, %716
  %727 = insertelement <2 x double> undef, double %726, i32 0
  %728 = fadd double %713, %717
  %729 = insertelement <2 x double> %727, double %728, i32 1
  store <2 x double> %729, <2 x double>* %11, align 16
  %730 = fsub double %712, %716
  %731 = insertelement <2 x double> undef, double %730, i32 0
  %732 = fsub double %713, %717
  %733 = insertelement <2 x double> %731, double %732, i32 1
  store <2 x double> %733, <2 x double>* %44, align 16
  %734 = fadd double %714, %722
  %735 = insertelement <2 x double> undef, double %734, i32 0
  %736 = fadd double %715, %725
  %737 = insertelement <2 x double> %735, double %736, i32 1
  store <2 x double> %737, <2 x double>* %62, align 16
  %738 = fsub double %714, %722
  %739 = insertelement <2 x double> undef, double %738, i32 0
  %740 = fsub double %715, %725
  %741 = insertelement <2 x double> %739, double %740, i32 1
  store <2 x double> %741, <2 x double>* %80, align 16
  %742 = fadd double %646, %700
  %743 = fadd double %649, %703
  %744 = fsub double %646, %700
  %745 = fsub double %649, %703
  %746 = fadd double %696, %710
  %747 = fadd double %697, %711
  %748 = fsub double %696, %710
  %749 = fsub double %697, %711
  %750 = fmul double %748, 0.000000e+00
  %751 = fmul double %749, -1.000000e+00
  %752 = fsub double %750, %751
  %753 = fmul double %748, -1.000000e+00
  %754 = fmul double %749, 0.000000e+00
  %755 = fadd double %753, %754
  %756 = fadd double %742, %746
  %757 = insertelement <2 x double> undef, double %756, i32 0
  %758 = fadd double %743, %747
  %759 = insertelement <2 x double> %757, double %758, i32 1
  store <2 x double> %759, <2 x double>* %29, align 16
  %760 = fsub double %742, %746
  %761 = insertelement <2 x double> undef, double %760, i32 0
  %762 = fsub double %743, %747
  %763 = insertelement <2 x double> %761, double %762, i32 1
  store <2 x double> %763, <2 x double>* %47, align 16
  %764 = fadd double %744, %752
  %765 = insertelement <2 x double> undef, double %764, i32 0
  %766 = fadd double %745, %755
  %767 = insertelement <2 x double> %765, double %766, i32 1
  store <2 x double> %767, <2 x double>* %65, align 16
  %768 = fsub double %744, %752
  %769 = insertelement <2 x double> undef, double %768, i32 0
  %770 = fsub double %745, %755
  %771 = insertelement <2 x double> %769, double %770, i32 1
  store <2 x double> %771, <2 x double>* %83, align 16
  %"&(pSB[currWI].offset)64165" = or i64 %CurrSBIndex..7, 16
  %"&pSB[currWI].offset65" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)64165"
  %CastToValueType66 = bitcast i8* %"&pSB[currWI].offset65" to <2 x double> addrspace(1)**
  %loadedValue67 = load <2 x double> addrspace(1)** %CastToValueType66, align 8
  %772 = load <2 x double>* %11, align 16
  store <2 x double> %772, <2 x double> addrspace(1)* %loadedValue67, align 16
  %scevgep2.1.i2 = getelementptr <2 x double> addrspace(1)* %loadedValue67, i64 64
  %"&pSB[currWI].offset143.sum166" = add i64 %CurrSBIndex..1, 112
  %773 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum166"
  %774 = bitcast i8* %773 to <2 x double>*
  %775 = load <2 x double>* %774, align 16
  store <2 x double> %775, <2 x double> addrspace(1)* %scevgep2.1.i2, align 16
  %scevgep2.2.i3 = getelementptr <2 x double> addrspace(1)* %loadedValue67, i64 128
  %"&pSB[currWI].offset143.sum167" = add i64 %CurrSBIndex..1, 80
  %776 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum167"
  %777 = bitcast i8* %776 to <2 x double>*
  %778 = load <2 x double>* %777, align 16
  store <2 x double> %778, <2 x double> addrspace(1)* %scevgep2.2.i3, align 16
  %scevgep2.3.i4 = getelementptr <2 x double> addrspace(1)* %loadedValue67, i64 192
  %"&pSB[currWI].offset143.sum168" = add i64 %CurrSBIndex..1, 144
  %779 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum168"
  %780 = bitcast i8* %779 to <2 x double>*
  %781 = load <2 x double>* %780, align 16
  store <2 x double> %781, <2 x double> addrspace(1)* %scevgep2.3.i4, align 16
  %scevgep2.4.i5 = getelementptr <2 x double> addrspace(1)* %loadedValue67, i64 256
  %"&pSB[currWI].offset143.sum169" = add i64 %CurrSBIndex..1, 64
  %782 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum169"
  %783 = bitcast i8* %782 to <2 x double>*
  %784 = load <2 x double>* %783, align 16
  store <2 x double> %784, <2 x double> addrspace(1)* %scevgep2.4.i5, align 16
  %scevgep2.5.i6 = getelementptr <2 x double> addrspace(1)* %loadedValue67, i64 320
  %"&pSB[currWI].offset143.sum170" = add i64 %CurrSBIndex..1, 128
  %785 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum170"
  %786 = bitcast i8* %785 to <2 x double>*
  %787 = load <2 x double>* %786, align 16
  store <2 x double> %787, <2 x double> addrspace(1)* %scevgep2.5.i6, align 16
  %scevgep2.6.i7 = getelementptr <2 x double> addrspace(1)* %loadedValue67, i64 384
  %"&pSB[currWI].offset143.sum171" = add i64 %CurrSBIndex..1, 96
  %788 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum171"
  %789 = bitcast i8* %788 to <2 x double>*
  %790 = load <2 x double>* %789, align 16
  store <2 x double> %790, <2 x double> addrspace(1)* %scevgep2.6.i7, align 16
  %scevgep2.7.i8 = getelementptr <2 x double> addrspace(1)* %loadedValue67, i64 448
  %"&pSB[currWI].offset143.sum172" = add i64 %CurrSBIndex..1, 160
  %791 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset143.sum172"
  %792 = bitcast i8* %791 to <2 x double>*
  %793 = load <2 x double>* %792, align 16
  store <2 x double> %793, <2 x double> addrspace(1)* %scevgep2.7.i8, align 16
  %loadedCurrWI239 = load i64* %pCurrWI, align 8
  %check.WI.iter240 = icmp ult i64 %loadedCurrWI239, %iterCount
  br i1 %check.WI.iter240, label %thenBB237, label %elseBB238

thenBB237:                                        ; preds = %SyncBB191
  %"CurrWI++241" = add nuw i64 %loadedCurrWI239, 1
  store i64 %"CurrWI++241", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride243" = add nuw i64 %CurrSBIndex..7, 608
  br label %SyncBB191

elseBB238:                                        ; preds = %SyncBB191
  store i64 0, i64* %pCurrWI, align 8
  ret void
}

define void @__ifft1D_512_separated_args(<2 x double> addrspace(1)* nocapture %work, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph6:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to [576 x double] addrspace(3)*
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB189

SyncBB189:                                        ; preds = %thenBB198, %bb.nph6
  %CurrSBIndex..1 = phi i64 [ 0, %bb.nph6 ], [ %"loadedCurrSB+Stride204", %thenBB198 ]
  %currWI = load i64* %pCurrWI, align 8
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %currWI, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = trunc i64 %2 to i32
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..1, 176
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %3, i32* %CastToValueType, align 4
  %4 = load i64* %pWGId, align 8
  %5 = shl i64 %4, 9
  %6 = add i64 %5, %2
  %7 = ashr i32 %3, 3
  %"&(pSB[currWI].offset)35" = add nuw i64 %CurrSBIndex..1, 180
  %"&pSB[currWI].offset36" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)35"
  %CastToValueType37 = bitcast i8* %"&pSB[currWI].offset36" to i32*
  store i32 %7, i32* %CastToValueType37, align 4
  %8 = and i32 %3, 7
  %"&(pSB[currWI].offset)49" = add nuw i64 %CurrSBIndex..1, 184
  %"&pSB[currWI].offset50" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)49"
  %CastToValueType51 = bitcast i8* %"&pSB[currWI].offset50" to i32*
  store i32 %8, i32* %CastToValueType51, align 4
  %sext = shl i64 %6, 32
  %9 = ashr i64 %sext, 32
  %10 = getelementptr inbounds <2 x double> addrspace(1)* %work, i64 %9
  %"&(pSB[currWI].offset)63" = add nuw i64 %CurrSBIndex..1, 192
  %"&pSB[currWI].offset64" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)63"
  %CastToValueType65 = bitcast i8* %"&pSB[currWI].offset64" to <2 x double> addrspace(1)**
  store <2 x double> addrspace(1)* %10, <2 x double> addrspace(1)** %CastToValueType65, align 8
  %"&(pSB[currWI].offset)145" = add nuw i64 %CurrSBIndex..1, 224
  %"&pSB[currWI].offset146" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)145"
  %11 = bitcast i8* %"&pSB[currWI].offset146" to <2 x double>*
  %12 = load <2 x double> addrspace(1)* %10, align 16
  store <2 x double> %12, <2 x double>* %11, align 16
  %"&pSB[currWI].offset146.sum" = add i64 %CurrSBIndex..1, 240
  %scevgep.1.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum"
  %13 = bitcast i8* %scevgep.1.i to <2 x double>*
  %.sum = add i64 %9, 64
  %scevgep2.1.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum
  %14 = load <2 x double> addrspace(1)* %scevgep2.1.i, align 16
  store <2 x double> %14, <2 x double>* %13, align 16
  %"&pSB[currWI].offset146.sum65" = add i64 %CurrSBIndex..1, 256
  %scevgep.2.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum65"
  %15 = bitcast i8* %scevgep.2.i to <2 x double>*
  %.sum66 = add i64 %9, 128
  %scevgep2.2.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum66
  %16 = load <2 x double> addrspace(1)* %scevgep2.2.i, align 16
  store <2 x double> %16, <2 x double>* %15, align 16
  %"&pSB[currWI].offset146.sum67" = add i64 %CurrSBIndex..1, 272
  %scevgep.3.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum67"
  %17 = bitcast i8* %scevgep.3.i to <2 x double>*
  %.sum68 = add i64 %9, 192
  %scevgep2.3.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum68
  %18 = load <2 x double> addrspace(1)* %scevgep2.3.i, align 16
  store <2 x double> %18, <2 x double>* %17, align 16
  %"&pSB[currWI].offset146.sum69" = add i64 %CurrSBIndex..1, 288
  %scevgep.4.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum69"
  %19 = bitcast i8* %scevgep.4.i to <2 x double>*
  %.sum70 = add i64 %9, 256
  %scevgep2.4.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum70
  %20 = load <2 x double> addrspace(1)* %scevgep2.4.i, align 16
  store <2 x double> %20, <2 x double>* %19, align 16
  %"&pSB[currWI].offset146.sum71" = add i64 %CurrSBIndex..1, 304
  %scevgep.5.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum71"
  %21 = bitcast i8* %scevgep.5.i to <2 x double>*
  %.sum72 = add i64 %9, 320
  %scevgep2.5.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum72
  %22 = load <2 x double> addrspace(1)* %scevgep2.5.i, align 16
  store <2 x double> %22, <2 x double>* %21, align 16
  %"&pSB[currWI].offset146.sum73" = add i64 %CurrSBIndex..1, 320
  %scevgep.6.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum73"
  %23 = bitcast i8* %scevgep.6.i to <2 x double>*
  %.sum74 = add i64 %9, 384
  %scevgep2.6.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum74
  %24 = load <2 x double> addrspace(1)* %scevgep2.6.i, align 16
  store <2 x double> %24, <2 x double>* %23, align 16
  %"&pSB[currWI].offset146.sum75" = add i64 %CurrSBIndex..1, 336
  %scevgep.7.i = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum75"
  %25 = bitcast i8* %scevgep.7.i to <2 x double>*
  %.sum76 = add i64 %9, 448
  %scevgep2.7.i = getelementptr <2 x double> addrspace(1)* %work, i64 %.sum76
  %26 = load <2 x double> addrspace(1)* %scevgep2.7.i, align 16
  store <2 x double> %26, <2 x double>* %25, align 16
  %27 = load <2 x double>* %11, align 16
  %"&pSB[currWI].offset142.sum" = add i64 %CurrSBIndex..1, 288
  %28 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset142.sum"
  %29 = bitcast i8* %28 to <2 x double>*
  %30 = load <2 x double>* %29, align 16
  %31 = extractelement <2 x double> %27, i32 0
  %32 = extractelement <2 x double> %30, i32 0
  %33 = fadd double %31, %32
  %34 = extractelement <2 x double> %27, i32 1
  %35 = extractelement <2 x double> %30, i32 1
  %36 = fadd double %34, %35
  %37 = extractelement <2 x double> %27, i32 0
  %38 = extractelement <2 x double> %30, i32 0
  %39 = fsub double %37, %38
  %40 = extractelement <2 x double> %27, i32 1
  %41 = extractelement <2 x double> %30, i32 1
  %42 = fsub double %40, %41
  %"&pSB[currWI].offset138.sum" = add i64 %CurrSBIndex..1, 240
  %43 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset138.sum"
  %44 = bitcast i8* %43 to <2 x double>*
  %45 = load <2 x double>* %44, align 16
  %"&pSB[currWI].offset134.sum" = add i64 %CurrSBIndex..1, 304
  %46 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset134.sum"
  %47 = bitcast i8* %46 to <2 x double>*
  %48 = load <2 x double>* %47, align 16
  %49 = extractelement <2 x double> %45, i32 0
  %50 = extractelement <2 x double> %48, i32 0
  %51 = fadd double %49, %50
  %52 = extractelement <2 x double> %45, i32 1
  %53 = extractelement <2 x double> %48, i32 1
  %54 = fadd double %52, %53
  %55 = extractelement <2 x double> %45, i32 0
  %56 = extractelement <2 x double> %48, i32 0
  %57 = fsub double %55, %56
  %58 = extractelement <2 x double> %45, i32 1
  %59 = extractelement <2 x double> %48, i32 1
  %60 = fsub double %58, %59
  %"&pSB[currWI].offset130.sum" = add i64 %CurrSBIndex..1, 256
  %61 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset130.sum"
  %62 = bitcast i8* %61 to <2 x double>*
  %63 = load <2 x double>* %62, align 16
  %"&pSB[currWI].offset126.sum" = add i64 %CurrSBIndex..1, 320
  %64 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset126.sum"
  %65 = bitcast i8* %64 to <2 x double>*
  %66 = load <2 x double>* %65, align 16
  %67 = extractelement <2 x double> %63, i32 0
  %68 = extractelement <2 x double> %66, i32 0
  %69 = fadd double %67, %68
  %70 = extractelement <2 x double> %63, i32 1
  %71 = extractelement <2 x double> %66, i32 1
  %72 = fadd double %70, %71
  %73 = extractelement <2 x double> %63, i32 0
  %74 = extractelement <2 x double> %66, i32 0
  %75 = fsub double %73, %74
  %76 = extractelement <2 x double> %63, i32 1
  %77 = extractelement <2 x double> %66, i32 1
  %78 = fsub double %76, %77
  %"&pSB[currWI].offset122.sum" = add i64 %CurrSBIndex..1, 272
  %79 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset122.sum"
  %80 = bitcast i8* %79 to <2 x double>*
  %81 = load <2 x double>* %80, align 16
  %"&pSB[currWI].offset118.sum" = add i64 %CurrSBIndex..1, 336
  %82 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset118.sum"
  %83 = bitcast i8* %82 to <2 x double>*
  %84 = load <2 x double>* %83, align 16
  %85 = extractelement <2 x double> %81, i32 0
  %86 = extractelement <2 x double> %84, i32 0
  %87 = fadd double %85, %86
  %88 = extractelement <2 x double> %81, i32 1
  %89 = extractelement <2 x double> %84, i32 1
  %90 = fadd double %88, %89
  %91 = extractelement <2 x double> %81, i32 0
  %92 = extractelement <2 x double> %84, i32 0
  %93 = fsub double %91, %92
  %94 = extractelement <2 x double> %81, i32 1
  %95 = extractelement <2 x double> %84, i32 1
  %96 = fsub double %94, %95
  %97 = fsub double %57, %60
  %98 = fadd double %57, %60
  %99 = fmul double %97, 0x3FE6A09E667F3BCD
  %100 = fmul double %98, 0x3FE6A09E667F3BCD
  %101 = fmul double %75, 0.000000e+00
  %102 = fsub double %101, %78
  %103 = fmul double %78, 0.000000e+00
  %104 = fadd double %75, %103
  %105 = fmul double %93, -1.000000e+00
  %106 = fsub double %105, %96
  %107 = fmul double %96, -1.000000e+00
  %108 = fadd double %93, %107
  %109 = fmul double %106, 0x3FE6A09E667F3BCD
  %110 = fmul double %108, 0x3FE6A09E667F3BCD
  %111 = fadd double %33, %69
  %112 = fadd double %36, %72
  %113 = fsub double %33, %69
  %114 = fsub double %36, %72
  %115 = fadd double %51, %87
  %116 = fadd double %54, %90
  %117 = fsub double %51, %87
  %118 = fsub double %54, %90
  %119 = fmul double %117, 0.000000e+00
  %120 = fsub double %119, %118
  %121 = fmul double %118, 0.000000e+00
  %122 = fadd double %117, %121
  %123 = fadd double %111, %115
  %124 = insertelement <2 x double> undef, double %123, i32 0
  %125 = fadd double %112, %116
  %126 = insertelement <2 x double> %124, double %125, i32 1
  store <2 x double> %126, <2 x double>* %11, align 16
  %127 = fsub double %111, %115
  %128 = insertelement <2 x double> undef, double %127, i32 0
  %129 = fsub double %112, %116
  %130 = insertelement <2 x double> %128, double %129, i32 1
  store <2 x double> %130, <2 x double>* %44, align 16
  %131 = fadd double %113, %120
  %132 = insertelement <2 x double> undef, double %131, i32 0
  %133 = fadd double %114, %122
  %134 = insertelement <2 x double> %132, double %133, i32 1
  store <2 x double> %134, <2 x double>* %62, align 16
  %135 = fsub double %113, %120
  %136 = insertelement <2 x double> undef, double %135, i32 0
  %137 = fsub double %114, %122
  %138 = insertelement <2 x double> %136, double %137, i32 1
  store <2 x double> %138, <2 x double>* %80, align 16
  %139 = fadd double %39, %102
  %140 = fadd double %42, %104
  %141 = fsub double %39, %102
  %142 = fsub double %42, %104
  %143 = fadd double %99, %109
  %144 = fadd double %100, %110
  %145 = fsub double %99, %109
  %146 = fsub double %100, %110
  %147 = fmul double %145, 0.000000e+00
  %148 = fsub double %147, %146
  %149 = fmul double %146, 0.000000e+00
  %150 = fadd double %145, %149
  %151 = fadd double %139, %143
  %152 = insertelement <2 x double> undef, double %151, i32 0
  %153 = fadd double %140, %144
  %154 = insertelement <2 x double> %152, double %153, i32 1
  store <2 x double> %154, <2 x double>* %29, align 16
  %155 = fsub double %139, %143
  %156 = insertelement <2 x double> undef, double %155, i32 0
  %157 = fsub double %140, %144
  %158 = insertelement <2 x double> %156, double %157, i32 1
  store <2 x double> %158, <2 x double>* %47, align 16
  %159 = fadd double %141, %148
  %160 = insertelement <2 x double> undef, double %159, i32 0
  %161 = fadd double %142, %150
  %162 = insertelement <2 x double> %160, double %161, i32 1
  store <2 x double> %162, <2 x double>* %65, align 16
  %163 = fsub double %141, %148
  %164 = insertelement <2 x double> undef, double %163, i32 0
  %165 = fsub double %142, %150
  %166 = insertelement <2 x double> %164, double %165, i32 1
  store <2 x double> %166, <2 x double>* %83, align 16
  %167 = sitofp i32 %3 to double
  %"&(pSB[currWI].offset)109" = add nuw i64 %CurrSBIndex..1, 224
  %"&pSB[currWI].offset110" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)109"
  %CastToValueType111 = bitcast i8* %"&pSB[currWI].offset110" to [8 x <2 x double>]*
  br label %168

; <label>:168                                     ; preds = %._crit_edge16, %SyncBB189
  %indvar10 = phi i64 [ 1, %SyncBB189 ], [ %phitmp, %._crit_edge16 ]
  %scevgep14 = getelementptr [8 x <2 x double>]* %CastToValueType111, i64 0, i64 %indvar10
  %scevgep15 = getelementptr [8 x i32]* @fft1D_512.reversed8, i64 0, i64 %indvar10
  %169 = load <2 x double>* %scevgep14, align 16
  %170 = load i32* %scevgep15, align 4
  %171 = sitofp i32 %170 to double
  %172 = fmul double %171, 0x401921FB54442D18
  %173 = fdiv double %172, 5.120000e+02
  %174 = fmul double %173, %167
  %175 = call double @_Z3cosd(double %174) nounwind
  %176 = call double @_Z3sind(double %174) nounwind
  %177 = extractelement <2 x double> %169, i32 0
  %178 = fmul double %177, %175
  %179 = extractelement <2 x double> %169, i32 1
  %180 = fmul double %179, %176
  %181 = fsub double %178, %180
  %182 = insertelement <2 x double> undef, double %181, i32 0
  %183 = fmul double %177, %176
  %184 = fmul double %179, %175
  %185 = fadd double %183, %184
  %186 = insertelement <2 x double> %182, double %185, i32 1
  store <2 x double> %186, <2 x double>* %scevgep14, align 16
  %exitcond12 = icmp eq i64 %indvar10, 7
  br i1 %exitcond12, label %bb.nph3, label %._crit_edge16

._crit_edge16:                                    ; preds = %168
  %phitmp = add i64 %indvar10, 1
  br label %168

bb.nph3:                                          ; preds = %168
  %"&(pSB[currWI].offset)30" = add nuw i64 %CurrSBIndex..1, 176
  %"&pSB[currWI].offset31" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)30"
  %CastToValueType32 = bitcast i8* %"&pSB[currWI].offset31" to i32*
  %loadedValue33 = load i32* %CastToValueType32, align 4
  %187 = sext i32 %loadedValue33 to i64
  %188 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %187
  %"&(pSB[currWI].offset)72" = add nuw i64 %CurrSBIndex..1, 200
  %"&pSB[currWI].offset73" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)72"
  %CastToValueType74 = bitcast i8* %"&pSB[currWI].offset73" to double addrspace(3)**
  store double addrspace(3)* %188, double addrspace(3)** %CastToValueType74, align 8
  %189 = load <2 x double>* %11, align 16
  %190 = extractelement <2 x double> %189, i32 0
  store double %190, double addrspace(3)* %188, align 8
  %"&pSB[currWI].offset146.sum77" = add i64 %CurrSBIndex..1, 288
  %191 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum77"
  %192 = bitcast i8* %191 to <2 x double>*
  %193 = load <2 x double>* %192, align 16
  %194 = extractelement <2 x double> %193, i32 0
  %.sum78 = add i64 %187, 66
  %195 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum78
  store double %194, double addrspace(3)* %195, align 8
  %"&pSB[currWI].offset146.sum79" = add i64 %CurrSBIndex..1, 256
  %196 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum79"
  %197 = bitcast i8* %196 to <2 x double>*
  %198 = load <2 x double>* %197, align 16
  %199 = extractelement <2 x double> %198, i32 0
  %.sum80 = add i64 %187, 132
  %200 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum80
  store double %199, double addrspace(3)* %200, align 8
  %"&pSB[currWI].offset146.sum81" = add i64 %CurrSBIndex..1, 320
  %201 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum81"
  %202 = bitcast i8* %201 to <2 x double>*
  %203 = load <2 x double>* %202, align 16
  %204 = extractelement <2 x double> %203, i32 0
  %.sum82 = add i64 %187, 198
  %205 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum82
  store double %204, double addrspace(3)* %205, align 8
  %"&pSB[currWI].offset146.sum83" = add i64 %CurrSBIndex..1, 240
  %206 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum83"
  %207 = bitcast i8* %206 to <2 x double>*
  %208 = load <2 x double>* %207, align 16
  %209 = extractelement <2 x double> %208, i32 0
  %.sum84 = add i64 %187, 264
  %210 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum84
  store double %209, double addrspace(3)* %210, align 8
  %"&pSB[currWI].offset146.sum85" = add i64 %CurrSBIndex..1, 304
  %211 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum85"
  %212 = bitcast i8* %211 to <2 x double>*
  %213 = load <2 x double>* %212, align 16
  %214 = extractelement <2 x double> %213, i32 0
  %.sum86 = add i64 %187, 330
  %215 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum86
  store double %214, double addrspace(3)* %215, align 8
  %"&pSB[currWI].offset146.sum87" = add i64 %CurrSBIndex..1, 272
  %216 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum87"
  %217 = bitcast i8* %216 to <2 x double>*
  %218 = load <2 x double>* %217, align 16
  %219 = extractelement <2 x double> %218, i32 0
  %.sum88 = add i64 %187, 396
  %220 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum88
  store double %219, double addrspace(3)* %220, align 8
  %"&pSB[currWI].offset146.sum89" = add i64 %CurrSBIndex..1, 336
  %221 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum89"
  %222 = bitcast i8* %221 to <2 x double>*
  %223 = load <2 x double>* %222, align 16
  %224 = extractelement <2 x double> %223, i32 0
  %.sum90 = add i64 %187, 462
  %225 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %.sum90
  store double %224, double addrspace(3)* %225, align 8
  %loadedCurrWI200 = load i64* %pCurrWI, align 8
  %check.WI.iter201 = icmp ult i64 %loadedCurrWI200, %iterCount
  br i1 %check.WI.iter201, label %thenBB198, label %elseBB199

thenBB198:                                        ; preds = %bb.nph3
  %"CurrWI++202" = add nuw i64 %loadedCurrWI200, 1
  store i64 %"CurrWI++202", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride204" = add nuw i64 %CurrSBIndex..1, 608
  br label %SyncBB189

elseBB199:                                        ; preds = %bb.nph3
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB190

SyncBB190:                                        ; preds = %thenBB205, %elseBB199
  %CurrSBIndex..2 = phi i64 [ 0, %elseBB199 ], [ %"loadedCurrSB+Stride211", %thenBB205 ]
  %"&(pSB[currWI].offset)58" = add nuw i64 %CurrSBIndex..2, 184
  %"&pSB[currWI].offset59" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)58"
  %CastToValueType60 = bitcast i8* %"&pSB[currWI].offset59" to i32*
  %loadedValue61 = load i32* %CastToValueType60, align 4
  %226 = mul nsw i32 %loadedValue61, 66
  %"&(pSB[currWI].offset)39" = add nuw i64 %CurrSBIndex..2, 180
  %"&pSB[currWI].offset40" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)39"
  %CastToValueType41 = bitcast i8* %"&pSB[currWI].offset40" to i32*
  %loadedValue42 = load i32* %CastToValueType41, align 4
  %227 = add nsw i32 %226, %loadedValue42
  %228 = sext i32 %227 to i64
  %229 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %228
  %"&(pSB[currWI].offset)91" = add nuw i64 %CurrSBIndex..2, 208
  %"&pSB[currWI].offset92" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)91"
  %CastToValueType93 = bitcast i8* %"&pSB[currWI].offset92" to double addrspace(3)**
  store double addrspace(3)* %229, double addrspace(3)** %CastToValueType93, align 8
  %230 = load double addrspace(3)* %229, align 8
  %231 = load <2 x double>* %11, align 16
  %232 = insertelement <2 x double> %231, double %230, i32 0
  store <2 x double> %232, <2 x double>* %11, align 16
  %"&pSB[currWI].offset146.sum91" = add i64 %CurrSBIndex..1, 240
  %scevgep.1.i51 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum91"
  %233 = bitcast i8* %scevgep.1.i51 to <2 x double>*
  %.sum92 = add i64 %228, 8
  %scevgep2.1.i52 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum92
  %234 = load double addrspace(3)* %scevgep2.1.i52, align 8
  %235 = load <2 x double>* %233, align 16
  %236 = insertelement <2 x double> %235, double %234, i32 0
  store <2 x double> %236, <2 x double>* %233, align 16
  %"&pSB[currWI].offset146.sum93" = add i64 %CurrSBIndex..1, 256
  %scevgep.2.i53 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum93"
  %237 = bitcast i8* %scevgep.2.i53 to <2 x double>*
  %.sum94 = add i64 %228, 16
  %scevgep2.2.i54 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum94
  %238 = load double addrspace(3)* %scevgep2.2.i54, align 8
  %239 = load <2 x double>* %237, align 16
  %240 = insertelement <2 x double> %239, double %238, i32 0
  store <2 x double> %240, <2 x double>* %237, align 16
  %"&pSB[currWI].offset146.sum95" = add i64 %CurrSBIndex..1, 272
  %scevgep.3.i55 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum95"
  %241 = bitcast i8* %scevgep.3.i55 to <2 x double>*
  %.sum96 = add i64 %228, 24
  %scevgep2.3.i56 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum96
  %242 = load double addrspace(3)* %scevgep2.3.i56, align 8
  %243 = load <2 x double>* %241, align 16
  %244 = insertelement <2 x double> %243, double %242, i32 0
  store <2 x double> %244, <2 x double>* %241, align 16
  %"&pSB[currWI].offset146.sum97" = add i64 %CurrSBIndex..1, 288
  %scevgep.4.i57 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum97"
  %245 = bitcast i8* %scevgep.4.i57 to <2 x double>*
  %.sum98 = add i64 %228, 32
  %scevgep2.4.i58 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum98
  %246 = load double addrspace(3)* %scevgep2.4.i58, align 8
  %247 = load <2 x double>* %245, align 16
  %248 = insertelement <2 x double> %247, double %246, i32 0
  store <2 x double> %248, <2 x double>* %245, align 16
  %"&pSB[currWI].offset146.sum99" = add i64 %CurrSBIndex..1, 304
  %scevgep.5.i59 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum99"
  %249 = bitcast i8* %scevgep.5.i59 to <2 x double>*
  %.sum100 = add i64 %228, 40
  %scevgep2.5.i60 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum100
  %250 = load double addrspace(3)* %scevgep2.5.i60, align 8
  %251 = load <2 x double>* %249, align 16
  %252 = insertelement <2 x double> %251, double %250, i32 0
  store <2 x double> %252, <2 x double>* %249, align 16
  %"&pSB[currWI].offset146.sum101" = add i64 %CurrSBIndex..1, 320
  %scevgep.6.i61 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum101"
  %253 = bitcast i8* %scevgep.6.i61 to <2 x double>*
  %.sum102 = add i64 %228, 48
  %scevgep2.6.i62 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum102
  %254 = load double addrspace(3)* %scevgep2.6.i62, align 8
  %255 = load <2 x double>* %253, align 16
  %256 = insertelement <2 x double> %255, double %254, i32 0
  store <2 x double> %256, <2 x double>* %253, align 16
  %"&pSB[currWI].offset146.sum103" = add i64 %CurrSBIndex..1, 336
  %scevgep.7.i63 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum103"
  %257 = bitcast i8* %scevgep.7.i63 to <2 x double>*
  %.sum104 = add i64 %228, 56
  %scevgep2.7.i64 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum104
  %258 = load double addrspace(3)* %scevgep2.7.i64, align 8
  %259 = load <2 x double>* %257, align 16
  %260 = insertelement <2 x double> %259, double %258, i32 0
  store <2 x double> %260, <2 x double>* %257, align 16
  %loadedCurrWI207 = load i64* %pCurrWI, align 8
  %check.WI.iter208 = icmp ult i64 %loadedCurrWI207, %iterCount
  br i1 %check.WI.iter208, label %thenBB205, label %elseBB206

thenBB205:                                        ; preds = %SyncBB190
  %"CurrWI++209" = add nuw i64 %loadedCurrWI207, 1
  store i64 %"CurrWI++209", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride211" = add nuw i64 %CurrSBIndex..2, 608
  br label %SyncBB190

elseBB206:                                        ; preds = %SyncBB190
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB191

SyncBB191:                                        ; preds = %thenBB212, %elseBB206
  %CurrSBIndex..3 = phi i64 [ 0, %elseBB206 ], [ %"loadedCurrSB+Stride218", %thenBB212 ]
  %"&(pSB[currWI].offset)86" = add nuw i64 %CurrSBIndex..3, 200
  %"&pSB[currWI].offset87" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)86"
  %CastToValueType88 = bitcast i8* %"&pSB[currWI].offset87" to double addrspace(3)**
  %loadedValue89 = load double addrspace(3)** %CastToValueType88, align 8
  %261 = load <2 x double>* %11, align 16
  %262 = extractelement <2 x double> %261, i32 1
  store double %262, double addrspace(3)* %loadedValue89, align 8
  %"&pSB[currWI].offset146.sum105" = add i64 %CurrSBIndex..1, 288
  %263 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum105"
  %264 = bitcast i8* %263 to <2 x double>*
  %265 = load <2 x double>* %264, align 16
  %266 = extractelement <2 x double> %265, i32 1
  %267 = getelementptr inbounds double addrspace(3)* %loadedValue89, i64 66
  store double %266, double addrspace(3)* %267, align 8
  %"&pSB[currWI].offset146.sum106" = add i64 %CurrSBIndex..1, 256
  %268 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum106"
  %269 = bitcast i8* %268 to <2 x double>*
  %270 = load <2 x double>* %269, align 16
  %271 = extractelement <2 x double> %270, i32 1
  %272 = getelementptr inbounds double addrspace(3)* %loadedValue89, i64 132
  store double %271, double addrspace(3)* %272, align 8
  %"&pSB[currWI].offset146.sum107" = add i64 %CurrSBIndex..1, 320
  %273 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum107"
  %274 = bitcast i8* %273 to <2 x double>*
  %275 = load <2 x double>* %274, align 16
  %276 = extractelement <2 x double> %275, i32 1
  %277 = getelementptr inbounds double addrspace(3)* %loadedValue89, i64 198
  store double %276, double addrspace(3)* %277, align 8
  %"&pSB[currWI].offset146.sum108" = add i64 %CurrSBIndex..1, 240
  %278 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum108"
  %279 = bitcast i8* %278 to <2 x double>*
  %280 = load <2 x double>* %279, align 16
  %281 = extractelement <2 x double> %280, i32 1
  %282 = getelementptr inbounds double addrspace(3)* %loadedValue89, i64 264
  store double %281, double addrspace(3)* %282, align 8
  %"&pSB[currWI].offset146.sum109" = add i64 %CurrSBIndex..1, 304
  %283 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum109"
  %284 = bitcast i8* %283 to <2 x double>*
  %285 = load <2 x double>* %284, align 16
  %286 = extractelement <2 x double> %285, i32 1
  %287 = getelementptr inbounds double addrspace(3)* %loadedValue89, i64 330
  store double %286, double addrspace(3)* %287, align 8
  %"&pSB[currWI].offset146.sum110" = add i64 %CurrSBIndex..1, 272
  %288 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum110"
  %289 = bitcast i8* %288 to <2 x double>*
  %290 = load <2 x double>* %289, align 16
  %291 = extractelement <2 x double> %290, i32 1
  %292 = getelementptr inbounds double addrspace(3)* %loadedValue89, i64 396
  store double %291, double addrspace(3)* %292, align 8
  %"&pSB[currWI].offset146.sum111" = add i64 %CurrSBIndex..1, 336
  %293 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum111"
  %294 = bitcast i8* %293 to <2 x double>*
  %295 = load <2 x double>* %294, align 16
  %296 = extractelement <2 x double> %295, i32 1
  %297 = getelementptr inbounds double addrspace(3)* %loadedValue89, i64 462
  store double %296, double addrspace(3)* %297, align 8
  %loadedCurrWI214 = load i64* %pCurrWI, align 8
  %check.WI.iter215 = icmp ult i64 %loadedCurrWI214, %iterCount
  br i1 %check.WI.iter215, label %thenBB212, label %elseBB213

thenBB212:                                        ; preds = %SyncBB191
  %"CurrWI++216" = add nuw i64 %loadedCurrWI214, 1
  store i64 %"CurrWI++216", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride218" = add nuw i64 %CurrSBIndex..3, 608
  br label %SyncBB191

elseBB213:                                        ; preds = %SyncBB191
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB192

SyncBB192:                                        ; preds = %thenBB219, %elseBB213
  %CurrSBIndex..4 = phi i64 [ 0, %elseBB213 ], [ %"loadedCurrSB+Stride225", %thenBB219 ]
  %"&(pSB[currWI].offset)95" = add nuw i64 %CurrSBIndex..4, 208
  %"&pSB[currWI].offset96" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)95"
  %CastToValueType97 = bitcast i8* %"&pSB[currWI].offset96" to double addrspace(3)**
  %loadedValue98 = load double addrspace(3)** %CastToValueType97, align 8
  %298 = load double addrspace(3)* %loadedValue98, align 8
  %299 = load <2 x double>* %11, align 16
  %300 = insertelement <2 x double> %299, double %298, i32 1
  store <2 x double> %300, <2 x double>* %11, align 16
  %"&pSB[currWI].offset146.sum112" = add i64 %CurrSBIndex..1, 240
  %scevgep.1.i37 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum112"
  %301 = bitcast i8* %scevgep.1.i37 to <2 x double>*
  %scevgep2.1.i38 = getelementptr double addrspace(3)* %loadedValue98, i64 8
  %302 = load double addrspace(3)* %scevgep2.1.i38, align 8
  %303 = load <2 x double>* %301, align 16
  %304 = insertelement <2 x double> %303, double %302, i32 1
  store <2 x double> %304, <2 x double>* %301, align 16
  %"&pSB[currWI].offset146.sum113" = add i64 %CurrSBIndex..1, 256
  %scevgep.2.i39 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum113"
  %305 = bitcast i8* %scevgep.2.i39 to <2 x double>*
  %scevgep2.2.i40 = getelementptr double addrspace(3)* %loadedValue98, i64 16
  %306 = load double addrspace(3)* %scevgep2.2.i40, align 8
  %307 = load <2 x double>* %305, align 16
  %308 = insertelement <2 x double> %307, double %306, i32 1
  store <2 x double> %308, <2 x double>* %305, align 16
  %"&pSB[currWI].offset146.sum114" = add i64 %CurrSBIndex..1, 272
  %scevgep.3.i41 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum114"
  %309 = bitcast i8* %scevgep.3.i41 to <2 x double>*
  %scevgep2.3.i42 = getelementptr double addrspace(3)* %loadedValue98, i64 24
  %310 = load double addrspace(3)* %scevgep2.3.i42, align 8
  %311 = load <2 x double>* %309, align 16
  %312 = insertelement <2 x double> %311, double %310, i32 1
  store <2 x double> %312, <2 x double>* %309, align 16
  %"&pSB[currWI].offset146.sum115" = add i64 %CurrSBIndex..1, 288
  %scevgep.4.i43 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum115"
  %313 = bitcast i8* %scevgep.4.i43 to <2 x double>*
  %scevgep2.4.i44 = getelementptr double addrspace(3)* %loadedValue98, i64 32
  %314 = load double addrspace(3)* %scevgep2.4.i44, align 8
  %315 = load <2 x double>* %313, align 16
  %316 = insertelement <2 x double> %315, double %314, i32 1
  store <2 x double> %316, <2 x double>* %313, align 16
  %"&pSB[currWI].offset146.sum116" = add i64 %CurrSBIndex..1, 304
  %scevgep.5.i45 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum116"
  %317 = bitcast i8* %scevgep.5.i45 to <2 x double>*
  %scevgep2.5.i46 = getelementptr double addrspace(3)* %loadedValue98, i64 40
  %318 = load double addrspace(3)* %scevgep2.5.i46, align 8
  %319 = load <2 x double>* %317, align 16
  %320 = insertelement <2 x double> %319, double %318, i32 1
  store <2 x double> %320, <2 x double>* %317, align 16
  %"&pSB[currWI].offset146.sum117" = add i64 %CurrSBIndex..1, 320
  %scevgep.6.i47 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum117"
  %321 = bitcast i8* %scevgep.6.i47 to <2 x double>*
  %scevgep2.6.i48 = getelementptr double addrspace(3)* %loadedValue98, i64 48
  %322 = load double addrspace(3)* %scevgep2.6.i48, align 8
  %323 = load <2 x double>* %321, align 16
  %324 = insertelement <2 x double> %323, double %322, i32 1
  store <2 x double> %324, <2 x double>* %321, align 16
  %"&pSB[currWI].offset146.sum118" = add i64 %CurrSBIndex..1, 336
  %scevgep.7.i49 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum118"
  %325 = bitcast i8* %scevgep.7.i49 to <2 x double>*
  %scevgep2.7.i50 = getelementptr double addrspace(3)* %loadedValue98, i64 56
  %326 = load double addrspace(3)* %scevgep2.7.i50, align 8
  %327 = load <2 x double>* %325, align 16
  %328 = insertelement <2 x double> %327, double %326, i32 1
  store <2 x double> %328, <2 x double>* %325, align 16
  %loadedCurrWI221 = load i64* %pCurrWI, align 8
  %check.WI.iter222 = icmp ult i64 %loadedCurrWI221, %iterCount
  br i1 %check.WI.iter222, label %thenBB219, label %elseBB220

thenBB219:                                        ; preds = %SyncBB192
  %"CurrWI++223" = add nuw i64 %loadedCurrWI221, 1
  store i64 %"CurrWI++223", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride225" = add nuw i64 %CurrSBIndex..4, 608
  br label %SyncBB192

elseBB220:                                        ; preds = %SyncBB192
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB193

SyncBB193:                                        ; preds = %thenBB226, %elseBB220
  %CurrSBIndex..5 = phi i64 [ 0, %elseBB220 ], [ %"loadedCurrSB+Stride232", %thenBB226 ]
  %329 = load <2 x double>* %11, align 16
  %330 = load <2 x double>* %29, align 16
  %331 = extractelement <2 x double> %329, i32 0
  %332 = extractelement <2 x double> %330, i32 0
  %333 = fadd double %331, %332
  %334 = extractelement <2 x double> %329, i32 1
  %335 = extractelement <2 x double> %330, i32 1
  %336 = fadd double %334, %335
  %337 = extractelement <2 x double> %329, i32 0
  %338 = extractelement <2 x double> %330, i32 0
  %339 = fsub double %337, %338
  %340 = extractelement <2 x double> %329, i32 1
  %341 = extractelement <2 x double> %330, i32 1
  %342 = fsub double %340, %341
  %343 = load <2 x double>* %44, align 16
  %344 = load <2 x double>* %47, align 16
  %345 = extractelement <2 x double> %343, i32 0
  %346 = extractelement <2 x double> %344, i32 0
  %347 = fadd double %345, %346
  %348 = extractelement <2 x double> %343, i32 1
  %349 = extractelement <2 x double> %344, i32 1
  %350 = fadd double %348, %349
  %351 = extractelement <2 x double> %343, i32 0
  %352 = extractelement <2 x double> %344, i32 0
  %353 = fsub double %351, %352
  %354 = extractelement <2 x double> %343, i32 1
  %355 = extractelement <2 x double> %344, i32 1
  %356 = fsub double %354, %355
  %357 = load <2 x double>* %62, align 16
  %358 = load <2 x double>* %65, align 16
  %359 = extractelement <2 x double> %357, i32 0
  %360 = extractelement <2 x double> %358, i32 0
  %361 = fadd double %359, %360
  %362 = extractelement <2 x double> %357, i32 1
  %363 = extractelement <2 x double> %358, i32 1
  %364 = fadd double %362, %363
  %365 = extractelement <2 x double> %357, i32 0
  %366 = extractelement <2 x double> %358, i32 0
  %367 = fsub double %365, %366
  %368 = extractelement <2 x double> %357, i32 1
  %369 = extractelement <2 x double> %358, i32 1
  %370 = fsub double %368, %369
  %371 = load <2 x double>* %80, align 16
  %372 = load <2 x double>* %83, align 16
  %373 = extractelement <2 x double> %371, i32 0
  %374 = extractelement <2 x double> %372, i32 0
  %375 = fadd double %373, %374
  %376 = extractelement <2 x double> %371, i32 1
  %377 = extractelement <2 x double> %372, i32 1
  %378 = fadd double %376, %377
  %379 = extractelement <2 x double> %371, i32 0
  %380 = extractelement <2 x double> %372, i32 0
  %381 = fsub double %379, %380
  %382 = extractelement <2 x double> %371, i32 1
  %383 = extractelement <2 x double> %372, i32 1
  %384 = fsub double %382, %383
  %385 = fsub double %353, %356
  %386 = fadd double %353, %356
  %387 = fmul double %385, 0x3FE6A09E667F3BCD
  %388 = fmul double %386, 0x3FE6A09E667F3BCD
  %389 = fmul double %367, 0.000000e+00
  %390 = fsub double %389, %370
  %391 = fmul double %370, 0.000000e+00
  %392 = fadd double %367, %391
  %393 = fmul double %381, -1.000000e+00
  %394 = fsub double %393, %384
  %395 = fmul double %384, -1.000000e+00
  %396 = fadd double %381, %395
  %397 = fmul double %394, 0x3FE6A09E667F3BCD
  %398 = fmul double %396, 0x3FE6A09E667F3BCD
  %399 = fadd double %333, %361
  %400 = fadd double %336, %364
  %401 = fsub double %333, %361
  %402 = fsub double %336, %364
  %403 = fadd double %347, %375
  %404 = fadd double %350, %378
  %405 = fsub double %347, %375
  %406 = fsub double %350, %378
  %407 = fmul double %405, 0.000000e+00
  %408 = fsub double %407, %406
  %409 = fmul double %406, 0.000000e+00
  %410 = fadd double %405, %409
  %411 = fadd double %399, %403
  %412 = insertelement <2 x double> undef, double %411, i32 0
  %413 = fadd double %400, %404
  %414 = insertelement <2 x double> %412, double %413, i32 1
  store <2 x double> %414, <2 x double>* %11, align 16
  %415 = fsub double %399, %403
  %416 = insertelement <2 x double> undef, double %415, i32 0
  %417 = fsub double %400, %404
  %418 = insertelement <2 x double> %416, double %417, i32 1
  store <2 x double> %418, <2 x double>* %44, align 16
  %419 = fadd double %401, %408
  %420 = insertelement <2 x double> undef, double %419, i32 0
  %421 = fadd double %402, %410
  %422 = insertelement <2 x double> %420, double %421, i32 1
  store <2 x double> %422, <2 x double>* %62, align 16
  %423 = fsub double %401, %408
  %424 = insertelement <2 x double> undef, double %423, i32 0
  %425 = fsub double %402, %410
  %426 = insertelement <2 x double> %424, double %425, i32 1
  store <2 x double> %426, <2 x double>* %80, align 16
  %427 = fadd double %339, %390
  %428 = fadd double %342, %392
  %429 = fsub double %339, %390
  %430 = fsub double %342, %392
  %431 = fadd double %387, %397
  %432 = fadd double %388, %398
  %433 = fsub double %387, %397
  %434 = fsub double %388, %398
  %435 = fmul double %433, 0.000000e+00
  %436 = fsub double %435, %434
  %437 = fmul double %434, 0.000000e+00
  %438 = fadd double %433, %437
  %439 = fadd double %427, %431
  %440 = insertelement <2 x double> undef, double %439, i32 0
  %441 = fadd double %428, %432
  %442 = insertelement <2 x double> %440, double %441, i32 1
  store <2 x double> %442, <2 x double>* %29, align 16
  %443 = fsub double %427, %431
  %444 = insertelement <2 x double> undef, double %443, i32 0
  %445 = fsub double %428, %432
  %446 = insertelement <2 x double> %444, double %445, i32 1
  store <2 x double> %446, <2 x double>* %47, align 16
  %447 = fadd double %429, %436
  %448 = insertelement <2 x double> undef, double %447, i32 0
  %449 = fadd double %430, %438
  %450 = insertelement <2 x double> %448, double %449, i32 1
  store <2 x double> %450, <2 x double>* %65, align 16
  %451 = fsub double %429, %436
  %452 = insertelement <2 x double> undef, double %451, i32 0
  %453 = fsub double %430, %438
  %454 = insertelement <2 x double> %452, double %453, i32 1
  store <2 x double> %454, <2 x double>* %83, align 16
  %"&(pSB[currWI].offset)44" = add nuw i64 %CurrSBIndex..5, 180
  %"&pSB[currWI].offset45" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)44"
  %CastToValueType46 = bitcast i8* %"&pSB[currWI].offset45" to i32*
  %loadedValue47 = load i32* %CastToValueType46, align 4
  %455 = sitofp i32 %loadedValue47 to double
  %"&(pSB[currWI].offset)113" = add nuw i64 %CurrSBIndex..5, 224
  %"&pSB[currWI].offset114" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)113"
  %CastToValueType115 = bitcast i8* %"&pSB[currWI].offset114" to [8 x <2 x double>]*
  br label %456

; <label>:456                                     ; preds = %._crit_edge, %SyncBB193
  %indvar = phi i64 [ 1, %SyncBB193 ], [ %phitmp17, %._crit_edge ]
  %scevgep8 = getelementptr [8 x <2 x double>]* %CastToValueType115, i64 0, i64 %indvar
  %scevgep9 = getelementptr [8 x i32]* @fft1D_512.reversed8, i64 0, i64 %indvar
  %457 = load <2 x double>* %scevgep8, align 16
  %458 = load i32* %scevgep9, align 4
  %459 = sitofp i32 %458 to double
  %460 = fmul double %459, 0x401921FB54442D18
  %461 = fdiv double %460, 6.400000e+01
  %462 = fmul double %461, %455
  %463 = call double @_Z3cosd(double %462) nounwind
  %464 = call double @_Z3sind(double %462) nounwind
  %465 = extractelement <2 x double> %457, i32 0
  %466 = fmul double %465, %463
  %467 = extractelement <2 x double> %457, i32 1
  %468 = fmul double %467, %464
  %469 = fsub double %466, %468
  %470 = insertelement <2 x double> undef, double %469, i32 0
  %471 = fmul double %465, %464
  %472 = fmul double %467, %463
  %473 = fadd double %471, %472
  %474 = insertelement <2 x double> %470, double %473, i32 1
  store <2 x double> %474, <2 x double>* %scevgep8, align 16
  %exitcond = icmp eq i64 %indvar, 7
  br i1 %exitcond, label %"Barrier BB22", label %._crit_edge

._crit_edge:                                      ; preds = %456
  %phitmp17 = add i64 %indvar, 1
  br label %456

"Barrier BB22":                                   ; preds = %456
  %"&(pSB[currWI].offset)81" = add nuw i64 %CurrSBIndex..5, 200
  %"&pSB[currWI].offset82" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)81"
  %CastToValueType83 = bitcast i8* %"&pSB[currWI].offset82" to double addrspace(3)**
  %loadedValue84 = load double addrspace(3)** %CastToValueType83, align 8
  %475 = load <2 x double>* %11, align 16
  %476 = extractelement <2 x double> %475, i32 0
  store double %476, double addrspace(3)* %loadedValue84, align 8
  %"&pSB[currWI].offset146.sum119" = add i64 %CurrSBIndex..1, 288
  %477 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum119"
  %478 = bitcast i8* %477 to <2 x double>*
  %479 = load <2 x double>* %478, align 16
  %480 = extractelement <2 x double> %479, i32 0
  %481 = getelementptr inbounds double addrspace(3)* %loadedValue84, i64 72
  store double %480, double addrspace(3)* %481, align 8
  %"&pSB[currWI].offset146.sum120" = add i64 %CurrSBIndex..1, 256
  %482 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum120"
  %483 = bitcast i8* %482 to <2 x double>*
  %484 = load <2 x double>* %483, align 16
  %485 = extractelement <2 x double> %484, i32 0
  %486 = getelementptr inbounds double addrspace(3)* %loadedValue84, i64 144
  store double %485, double addrspace(3)* %486, align 8
  %"&pSB[currWI].offset146.sum121" = add i64 %CurrSBIndex..1, 320
  %487 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum121"
  %488 = bitcast i8* %487 to <2 x double>*
  %489 = load <2 x double>* %488, align 16
  %490 = extractelement <2 x double> %489, i32 0
  %491 = getelementptr inbounds double addrspace(3)* %loadedValue84, i64 216
  store double %490, double addrspace(3)* %491, align 8
  %"&pSB[currWI].offset146.sum122" = add i64 %CurrSBIndex..1, 240
  %492 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum122"
  %493 = bitcast i8* %492 to <2 x double>*
  %494 = load <2 x double>* %493, align 16
  %495 = extractelement <2 x double> %494, i32 0
  %496 = getelementptr inbounds double addrspace(3)* %loadedValue84, i64 288
  store double %495, double addrspace(3)* %496, align 8
  %"&pSB[currWI].offset146.sum123" = add i64 %CurrSBIndex..1, 304
  %497 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum123"
  %498 = bitcast i8* %497 to <2 x double>*
  %499 = load <2 x double>* %498, align 16
  %500 = extractelement <2 x double> %499, i32 0
  %501 = getelementptr inbounds double addrspace(3)* %loadedValue84, i64 360
  store double %500, double addrspace(3)* %501, align 8
  %"&pSB[currWI].offset146.sum124" = add i64 %CurrSBIndex..1, 272
  %502 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum124"
  %503 = bitcast i8* %502 to <2 x double>*
  %504 = load <2 x double>* %503, align 16
  %505 = extractelement <2 x double> %504, i32 0
  %506 = getelementptr inbounds double addrspace(3)* %loadedValue84, i64 432
  store double %505, double addrspace(3)* %506, align 8
  %"&pSB[currWI].offset146.sum125" = add i64 %CurrSBIndex..1, 336
  %507 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum125"
  %508 = bitcast i8* %507 to <2 x double>*
  %509 = load <2 x double>* %508, align 16
  %510 = extractelement <2 x double> %509, i32 0
  %511 = getelementptr inbounds double addrspace(3)* %loadedValue84, i64 504
  store double %510, double addrspace(3)* %511, align 8
  %loadedCurrWI228 = load i64* %pCurrWI, align 8
  %check.WI.iter229 = icmp ult i64 %loadedCurrWI228, %iterCount
  br i1 %check.WI.iter229, label %thenBB226, label %elseBB227

thenBB226:                                        ; preds = %"Barrier BB22"
  %"CurrWI++230" = add nuw i64 %loadedCurrWI228, 1
  store i64 %"CurrWI++230", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride232" = add nuw i64 %CurrSBIndex..5, 608
  br label %SyncBB193

elseBB227:                                        ; preds = %"Barrier BB22"
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB194

SyncBB194:                                        ; preds = %thenBB233, %elseBB227
  %CurrSBIndex..6 = phi i64 [ 0, %elseBB227 ], [ %"loadedCurrSB+Stride239", %thenBB233 ]
  %"&(pSB[currWI].offset)26" = add nuw i64 %CurrSBIndex..6, 176
  %"&pSB[currWI].offset27" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)26"
  %CastToValueType28 = bitcast i8* %"&pSB[currWI].offset27" to i32*
  %loadedValue = load i32* %CastToValueType28, align 4
  %512 = and i32 %loadedValue, -8
  %513 = mul nsw i32 %512, 9
  %"&(pSB[currWI].offset)53" = add nuw i64 %CurrSBIndex..6, 184
  %"&pSB[currWI].offset54" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)53"
  %CastToValueType55 = bitcast i8* %"&pSB[currWI].offset54" to i32*
  %loadedValue56 = load i32* %CastToValueType55, align 4
  %514 = or i32 %513, %loadedValue56
  %515 = sext i32 %514 to i64
  %516 = getelementptr inbounds [576 x double] addrspace(3)* %0, i64 0, i64 %515
  %"&(pSB[currWI].offset)100" = add nuw i64 %CurrSBIndex..6, 216
  %"&pSB[currWI].offset101" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)100"
  %CastToValueType102 = bitcast i8* %"&pSB[currWI].offset101" to double addrspace(3)**
  store double addrspace(3)* %516, double addrspace(3)** %CastToValueType102, align 8
  %517 = load double addrspace(3)* %516, align 8
  %518 = load <2 x double>* %11, align 16
  %519 = insertelement <2 x double> %518, double %517, i32 0
  store <2 x double> %519, <2 x double>* %11, align 16
  %"&pSB[currWI].offset146.sum126" = add i64 %CurrSBIndex..1, 240
  %scevgep.1.i23 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum126"
  %520 = bitcast i8* %scevgep.1.i23 to <2 x double>*
  %.sum127 = add i64 %515, 8
  %scevgep2.1.i24 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum127
  %521 = load double addrspace(3)* %scevgep2.1.i24, align 8
  %522 = load <2 x double>* %520, align 16
  %523 = insertelement <2 x double> %522, double %521, i32 0
  store <2 x double> %523, <2 x double>* %520, align 16
  %"&pSB[currWI].offset146.sum128" = add i64 %CurrSBIndex..1, 256
  %scevgep.2.i25 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum128"
  %524 = bitcast i8* %scevgep.2.i25 to <2 x double>*
  %.sum129 = add i64 %515, 16
  %scevgep2.2.i26 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum129
  %525 = load double addrspace(3)* %scevgep2.2.i26, align 8
  %526 = load <2 x double>* %524, align 16
  %527 = insertelement <2 x double> %526, double %525, i32 0
  store <2 x double> %527, <2 x double>* %524, align 16
  %"&pSB[currWI].offset146.sum130" = add i64 %CurrSBIndex..1, 272
  %scevgep.3.i27 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum130"
  %528 = bitcast i8* %scevgep.3.i27 to <2 x double>*
  %.sum131 = add i64 %515, 24
  %scevgep2.3.i28 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum131
  %529 = load double addrspace(3)* %scevgep2.3.i28, align 8
  %530 = load <2 x double>* %528, align 16
  %531 = insertelement <2 x double> %530, double %529, i32 0
  store <2 x double> %531, <2 x double>* %528, align 16
  %"&pSB[currWI].offset146.sum132" = add i64 %CurrSBIndex..1, 288
  %scevgep.4.i29 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum132"
  %532 = bitcast i8* %scevgep.4.i29 to <2 x double>*
  %.sum133 = add i64 %515, 32
  %scevgep2.4.i30 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum133
  %533 = load double addrspace(3)* %scevgep2.4.i30, align 8
  %534 = load <2 x double>* %532, align 16
  %535 = insertelement <2 x double> %534, double %533, i32 0
  store <2 x double> %535, <2 x double>* %532, align 16
  %"&pSB[currWI].offset146.sum134" = add i64 %CurrSBIndex..1, 304
  %scevgep.5.i31 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum134"
  %536 = bitcast i8* %scevgep.5.i31 to <2 x double>*
  %.sum135 = add i64 %515, 40
  %scevgep2.5.i32 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum135
  %537 = load double addrspace(3)* %scevgep2.5.i32, align 8
  %538 = load <2 x double>* %536, align 16
  %539 = insertelement <2 x double> %538, double %537, i32 0
  store <2 x double> %539, <2 x double>* %536, align 16
  %"&pSB[currWI].offset146.sum136" = add i64 %CurrSBIndex..1, 320
  %scevgep.6.i33 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum136"
  %540 = bitcast i8* %scevgep.6.i33 to <2 x double>*
  %.sum137 = add i64 %515, 48
  %scevgep2.6.i34 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum137
  %541 = load double addrspace(3)* %scevgep2.6.i34, align 8
  %542 = load <2 x double>* %540, align 16
  %543 = insertelement <2 x double> %542, double %541, i32 0
  store <2 x double> %543, <2 x double>* %540, align 16
  %"&pSB[currWI].offset146.sum138" = add i64 %CurrSBIndex..1, 336
  %scevgep.7.i35 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum138"
  %544 = bitcast i8* %scevgep.7.i35 to <2 x double>*
  %.sum139 = add i64 %515, 56
  %scevgep2.7.i36 = getelementptr [576 x double] addrspace(3)* %0, i64 0, i64 %.sum139
  %545 = load double addrspace(3)* %scevgep2.7.i36, align 8
  %546 = load <2 x double>* %544, align 16
  %547 = insertelement <2 x double> %546, double %545, i32 0
  store <2 x double> %547, <2 x double>* %544, align 16
  %loadedCurrWI235 = load i64* %pCurrWI, align 8
  %check.WI.iter236 = icmp ult i64 %loadedCurrWI235, %iterCount
  br i1 %check.WI.iter236, label %thenBB233, label %elseBB234

thenBB233:                                        ; preds = %SyncBB194
  %"CurrWI++237" = add nuw i64 %loadedCurrWI235, 1
  store i64 %"CurrWI++237", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride239" = add nuw i64 %CurrSBIndex..6, 608
  br label %SyncBB194

elseBB234:                                        ; preds = %SyncBB194
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB195

SyncBB195:                                        ; preds = %thenBB240, %elseBB234
  %CurrSBIndex..7 = phi i64 [ 0, %elseBB234 ], [ %"loadedCurrSB+Stride246", %thenBB240 ]
  %"&(pSB[currWI].offset)76" = add nuw i64 %CurrSBIndex..7, 200
  %"&pSB[currWI].offset77" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)76"
  %CastToValueType78 = bitcast i8* %"&pSB[currWI].offset77" to double addrspace(3)**
  %loadedValue79 = load double addrspace(3)** %CastToValueType78, align 8
  %548 = load <2 x double>* %11, align 16
  %549 = extractelement <2 x double> %548, i32 1
  store double %549, double addrspace(3)* %loadedValue79, align 8
  %"&pSB[currWI].offset146.sum140" = add i64 %CurrSBIndex..1, 288
  %550 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum140"
  %551 = bitcast i8* %550 to <2 x double>*
  %552 = load <2 x double>* %551, align 16
  %553 = extractelement <2 x double> %552, i32 1
  %554 = getelementptr inbounds double addrspace(3)* %loadedValue79, i64 72
  store double %553, double addrspace(3)* %554, align 8
  %"&pSB[currWI].offset146.sum141" = add i64 %CurrSBIndex..1, 256
  %555 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum141"
  %556 = bitcast i8* %555 to <2 x double>*
  %557 = load <2 x double>* %556, align 16
  %558 = extractelement <2 x double> %557, i32 1
  %559 = getelementptr inbounds double addrspace(3)* %loadedValue79, i64 144
  store double %558, double addrspace(3)* %559, align 8
  %"&pSB[currWI].offset146.sum142" = add i64 %CurrSBIndex..1, 320
  %560 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum142"
  %561 = bitcast i8* %560 to <2 x double>*
  %562 = load <2 x double>* %561, align 16
  %563 = extractelement <2 x double> %562, i32 1
  %564 = getelementptr inbounds double addrspace(3)* %loadedValue79, i64 216
  store double %563, double addrspace(3)* %564, align 8
  %"&pSB[currWI].offset146.sum143" = add i64 %CurrSBIndex..1, 240
  %565 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum143"
  %566 = bitcast i8* %565 to <2 x double>*
  %567 = load <2 x double>* %566, align 16
  %568 = extractelement <2 x double> %567, i32 1
  %569 = getelementptr inbounds double addrspace(3)* %loadedValue79, i64 288
  store double %568, double addrspace(3)* %569, align 8
  %"&pSB[currWI].offset146.sum144" = add i64 %CurrSBIndex..1, 304
  %570 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum144"
  %571 = bitcast i8* %570 to <2 x double>*
  %572 = load <2 x double>* %571, align 16
  %573 = extractelement <2 x double> %572, i32 1
  %574 = getelementptr inbounds double addrspace(3)* %loadedValue79, i64 360
  store double %573, double addrspace(3)* %574, align 8
  %"&pSB[currWI].offset146.sum145" = add i64 %CurrSBIndex..1, 272
  %575 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum145"
  %576 = bitcast i8* %575 to <2 x double>*
  %577 = load <2 x double>* %576, align 16
  %578 = extractelement <2 x double> %577, i32 1
  %579 = getelementptr inbounds double addrspace(3)* %loadedValue79, i64 432
  store double %578, double addrspace(3)* %579, align 8
  %"&pSB[currWI].offset146.sum146" = add i64 %CurrSBIndex..1, 336
  %580 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum146"
  %581 = bitcast i8* %580 to <2 x double>*
  %582 = load <2 x double>* %581, align 16
  %583 = extractelement <2 x double> %582, i32 1
  %584 = getelementptr inbounds double addrspace(3)* %loadedValue79, i64 504
  store double %583, double addrspace(3)* %584, align 8
  %loadedCurrWI242 = load i64* %pCurrWI, align 8
  %check.WI.iter243 = icmp ult i64 %loadedCurrWI242, %iterCount
  br i1 %check.WI.iter243, label %thenBB240, label %elseBB241

thenBB240:                                        ; preds = %SyncBB195
  %"CurrWI++244" = add nuw i64 %loadedCurrWI242, 1
  store i64 %"CurrWI++244", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride246" = add nuw i64 %CurrSBIndex..7, 608
  br label %SyncBB195

elseBB241:                                        ; preds = %SyncBB195
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB196

SyncBB196:                                        ; preds = %thenBB, %elseBB241
  %CurrSBIndex..0 = phi i64 [ 0, %elseBB241 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %"&(pSB[currWI].offset)104" = add nuw i64 %CurrSBIndex..0, 216
  %"&pSB[currWI].offset105" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)104"
  %CastToValueType106 = bitcast i8* %"&pSB[currWI].offset105" to double addrspace(3)**
  %loadedValue107 = load double addrspace(3)** %CastToValueType106, align 8
  %585 = load double addrspace(3)* %loadedValue107, align 8
  %586 = load <2 x double>* %11, align 16
  %587 = insertelement <2 x double> %586, double %585, i32 1
  store <2 x double> %587, <2 x double>* %11, align 16
  %"&pSB[currWI].offset146.sum147" = add i64 %CurrSBIndex..1, 240
  %scevgep.1.i9 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum147"
  %588 = bitcast i8* %scevgep.1.i9 to <2 x double>*
  %scevgep2.1.i10 = getelementptr double addrspace(3)* %loadedValue107, i64 8
  %589 = load double addrspace(3)* %scevgep2.1.i10, align 8
  %590 = load <2 x double>* %588, align 16
  %591 = insertelement <2 x double> %590, double %589, i32 1
  store <2 x double> %591, <2 x double>* %588, align 16
  %"&pSB[currWI].offset146.sum148" = add i64 %CurrSBIndex..1, 256
  %scevgep.2.i11 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum148"
  %592 = bitcast i8* %scevgep.2.i11 to <2 x double>*
  %scevgep2.2.i12 = getelementptr double addrspace(3)* %loadedValue107, i64 16
  %593 = load double addrspace(3)* %scevgep2.2.i12, align 8
  %594 = load <2 x double>* %592, align 16
  %595 = insertelement <2 x double> %594, double %593, i32 1
  store <2 x double> %595, <2 x double>* %592, align 16
  %"&pSB[currWI].offset146.sum149" = add i64 %CurrSBIndex..1, 272
  %scevgep.3.i13 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum149"
  %596 = bitcast i8* %scevgep.3.i13 to <2 x double>*
  %scevgep2.3.i14 = getelementptr double addrspace(3)* %loadedValue107, i64 24
  %597 = load double addrspace(3)* %scevgep2.3.i14, align 8
  %598 = load <2 x double>* %596, align 16
  %599 = insertelement <2 x double> %598, double %597, i32 1
  store <2 x double> %599, <2 x double>* %596, align 16
  %"&pSB[currWI].offset146.sum150" = add i64 %CurrSBIndex..1, 288
  %scevgep.4.i15 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum150"
  %600 = bitcast i8* %scevgep.4.i15 to <2 x double>*
  %scevgep2.4.i16 = getelementptr double addrspace(3)* %loadedValue107, i64 32
  %601 = load double addrspace(3)* %scevgep2.4.i16, align 8
  %602 = load <2 x double>* %600, align 16
  %603 = insertelement <2 x double> %602, double %601, i32 1
  store <2 x double> %603, <2 x double>* %600, align 16
  %"&pSB[currWI].offset146.sum151" = add i64 %CurrSBIndex..1, 304
  %scevgep.5.i17 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum151"
  %604 = bitcast i8* %scevgep.5.i17 to <2 x double>*
  %scevgep2.5.i18 = getelementptr double addrspace(3)* %loadedValue107, i64 40
  %605 = load double addrspace(3)* %scevgep2.5.i18, align 8
  %606 = load <2 x double>* %604, align 16
  %607 = insertelement <2 x double> %606, double %605, i32 1
  store <2 x double> %607, <2 x double>* %604, align 16
  %"&pSB[currWI].offset146.sum152" = add i64 %CurrSBIndex..1, 320
  %scevgep.6.i19 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum152"
  %608 = bitcast i8* %scevgep.6.i19 to <2 x double>*
  %scevgep2.6.i20 = getelementptr double addrspace(3)* %loadedValue107, i64 48
  %609 = load double addrspace(3)* %scevgep2.6.i20, align 8
  %610 = load <2 x double>* %608, align 16
  %611 = insertelement <2 x double> %610, double %609, i32 1
  store <2 x double> %611, <2 x double>* %608, align 16
  %"&pSB[currWI].offset146.sum153" = add i64 %CurrSBIndex..1, 336
  %scevgep.7.i21 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum153"
  %612 = bitcast i8* %scevgep.7.i21 to <2 x double>*
  %scevgep2.7.i22 = getelementptr double addrspace(3)* %loadedValue107, i64 56
  %613 = load double addrspace(3)* %scevgep2.7.i22, align 8
  %614 = load <2 x double>* %612, align 16
  %615 = insertelement <2 x double> %614, double %613, i32 1
  store <2 x double> %615, <2 x double>* %612, align 16
  %616 = load <2 x double>* %11, align 16
  %617 = load <2 x double>* %29, align 16
  %618 = extractelement <2 x double> %616, i32 0
  %619 = extractelement <2 x double> %617, i32 0
  %620 = fadd double %618, %619
  %621 = extractelement <2 x double> %616, i32 1
  %622 = extractelement <2 x double> %617, i32 1
  %623 = fadd double %621, %622
  %624 = extractelement <2 x double> %616, i32 0
  %625 = extractelement <2 x double> %617, i32 0
  %626 = fsub double %624, %625
  %627 = extractelement <2 x double> %616, i32 1
  %628 = extractelement <2 x double> %617, i32 1
  %629 = fsub double %627, %628
  %630 = load <2 x double>* %44, align 16
  %631 = load <2 x double>* %47, align 16
  %632 = extractelement <2 x double> %630, i32 0
  %633 = extractelement <2 x double> %631, i32 0
  %634 = fadd double %632, %633
  %635 = extractelement <2 x double> %630, i32 1
  %636 = extractelement <2 x double> %631, i32 1
  %637 = fadd double %635, %636
  %638 = extractelement <2 x double> %630, i32 0
  %639 = extractelement <2 x double> %631, i32 0
  %640 = fsub double %638, %639
  %641 = extractelement <2 x double> %630, i32 1
  %642 = extractelement <2 x double> %631, i32 1
  %643 = fsub double %641, %642
  %644 = load <2 x double>* %62, align 16
  %645 = load <2 x double>* %65, align 16
  %646 = extractelement <2 x double> %644, i32 0
  %647 = extractelement <2 x double> %645, i32 0
  %648 = fadd double %646, %647
  %649 = extractelement <2 x double> %644, i32 1
  %650 = extractelement <2 x double> %645, i32 1
  %651 = fadd double %649, %650
  %652 = extractelement <2 x double> %644, i32 0
  %653 = extractelement <2 x double> %645, i32 0
  %654 = fsub double %652, %653
  %655 = extractelement <2 x double> %644, i32 1
  %656 = extractelement <2 x double> %645, i32 1
  %657 = fsub double %655, %656
  %658 = load <2 x double>* %80, align 16
  %659 = load <2 x double>* %83, align 16
  %660 = extractelement <2 x double> %658, i32 0
  %661 = extractelement <2 x double> %659, i32 0
  %662 = fadd double %660, %661
  %663 = extractelement <2 x double> %658, i32 1
  %664 = extractelement <2 x double> %659, i32 1
  %665 = fadd double %663, %664
  %666 = extractelement <2 x double> %658, i32 0
  %667 = extractelement <2 x double> %659, i32 0
  %668 = fsub double %666, %667
  %669 = extractelement <2 x double> %658, i32 1
  %670 = extractelement <2 x double> %659, i32 1
  %671 = fsub double %669, %670
  %672 = fsub double %640, %643
  %673 = fadd double %640, %643
  %674 = fmul double %672, 0x3FE6A09E667F3BCD
  %675 = fmul double %673, 0x3FE6A09E667F3BCD
  %676 = fmul double %654, 0.000000e+00
  %677 = fsub double %676, %657
  %678 = fmul double %657, 0.000000e+00
  %679 = fadd double %654, %678
  %680 = fmul double %668, -1.000000e+00
  %681 = fsub double %680, %671
  %682 = fmul double %671, -1.000000e+00
  %683 = fadd double %668, %682
  %684 = fmul double %681, 0x3FE6A09E667F3BCD
  %685 = fmul double %683, 0x3FE6A09E667F3BCD
  %686 = fadd double %620, %648
  %687 = fadd double %623, %651
  %688 = fsub double %620, %648
  %689 = fsub double %623, %651
  %690 = fadd double %634, %662
  %691 = fadd double %637, %665
  %692 = fsub double %634, %662
  %693 = fsub double %637, %665
  %694 = fmul double %692, 0.000000e+00
  %695 = fsub double %694, %693
  %696 = fmul double %693, 0.000000e+00
  %697 = fadd double %692, %696
  %698 = fadd double %686, %690
  %699 = fadd double %687, %691
  %700 = fsub double %686, %690
  %701 = fsub double %687, %691
  %702 = fadd double %688, %695
  %703 = fadd double %689, %697
  %704 = fsub double %688, %695
  %705 = fsub double %689, %697
  %706 = fadd double %626, %677
  %707 = fadd double %629, %679
  %708 = fsub double %626, %677
  %709 = fsub double %629, %679
  %710 = fadd double %674, %684
  %711 = fadd double %675, %685
  %712 = fsub double %674, %684
  %713 = fsub double %675, %685
  %714 = fmul double %712, 0.000000e+00
  %715 = fsub double %714, %713
  %716 = fmul double %713, 0.000000e+00
  %717 = fadd double %712, %716
  %718 = fadd double %706, %710
  %719 = fadd double %707, %711
  %720 = fsub double %706, %710
  %721 = fsub double %707, %711
  %722 = fadd double %708, %715
  %723 = fadd double %709, %717
  %724 = fsub double %708, %715
  %725 = fsub double %709, %717
  %726 = fdiv double %698, 5.120000e+02
  %727 = insertelement <2 x double> undef, double %726, i32 0
  %728 = fdiv double %699, 5.120000e+02
  %729 = insertelement <2 x double> %727, double %728, i32 1
  store <2 x double> %729, <2 x double>* %11, align 16
  %730 = fdiv double %700, 5.120000e+02
  %731 = insertelement <2 x double> undef, double %730, i32 0
  %732 = fdiv double %701, 5.120000e+02
  %733 = insertelement <2 x double> %731, double %732, i32 1
  store <2 x double> %733, <2 x double>* %44, align 16
  %734 = fdiv double %702, 5.120000e+02
  %735 = insertelement <2 x double> undef, double %734, i32 0
  %736 = fdiv double %703, 5.120000e+02
  %737 = insertelement <2 x double> %735, double %736, i32 1
  store <2 x double> %737, <2 x double>* %62, align 16
  %738 = fdiv double %704, 5.120000e+02
  %739 = insertelement <2 x double> undef, double %738, i32 0
  %740 = fdiv double %705, 5.120000e+02
  %741 = insertelement <2 x double> %739, double %740, i32 1
  store <2 x double> %741, <2 x double>* %80, align 16
  %742 = fdiv double %718, 5.120000e+02
  %743 = insertelement <2 x double> undef, double %742, i32 0
  %744 = fdiv double %719, 5.120000e+02
  %745 = insertelement <2 x double> %743, double %744, i32 1
  store <2 x double> %745, <2 x double>* %29, align 16
  %746 = fdiv double %720, 5.120000e+02
  %747 = insertelement <2 x double> undef, double %746, i32 0
  %748 = fdiv double %721, 5.120000e+02
  %749 = insertelement <2 x double> %747, double %748, i32 1
  store <2 x double> %749, <2 x double>* %47, align 16
  %750 = fdiv double %722, 5.120000e+02
  %751 = insertelement <2 x double> undef, double %750, i32 0
  %752 = fdiv double %723, 5.120000e+02
  %753 = insertelement <2 x double> %751, double %752, i32 1
  store <2 x double> %753, <2 x double>* %65, align 16
  %754 = fdiv double %724, 5.120000e+02
  %755 = insertelement <2 x double> undef, double %754, i32 0
  %756 = fdiv double %725, 5.120000e+02
  %757 = insertelement <2 x double> %755, double %756, i32 1
  store <2 x double> %757, <2 x double>* %83, align 16
  %"&(pSB[currWI].offset)67" = add nuw i64 %CurrSBIndex..0, 192
  %"&pSB[currWI].offset68" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)67"
  %CastToValueType69 = bitcast i8* %"&pSB[currWI].offset68" to <2 x double> addrspace(1)**
  %loadedValue70 = load <2 x double> addrspace(1)** %CastToValueType69, align 8
  %758 = load <2 x double>* %11, align 16
  store <2 x double> %758, <2 x double> addrspace(1)* %loadedValue70, align 16
  %scevgep2.1.i2 = getelementptr <2 x double> addrspace(1)* %loadedValue70, i64 64
  %"&pSB[currWI].offset146.sum154" = add i64 %CurrSBIndex..1, 288
  %759 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum154"
  %760 = bitcast i8* %759 to <2 x double>*
  %761 = load <2 x double>* %760, align 16
  store <2 x double> %761, <2 x double> addrspace(1)* %scevgep2.1.i2, align 16
  %scevgep2.2.i3 = getelementptr <2 x double> addrspace(1)* %loadedValue70, i64 128
  %"&pSB[currWI].offset146.sum155" = add i64 %CurrSBIndex..1, 256
  %762 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum155"
  %763 = bitcast i8* %762 to <2 x double>*
  %764 = load <2 x double>* %763, align 16
  store <2 x double> %764, <2 x double> addrspace(1)* %scevgep2.2.i3, align 16
  %scevgep2.3.i4 = getelementptr <2 x double> addrspace(1)* %loadedValue70, i64 192
  %"&pSB[currWI].offset146.sum156" = add i64 %CurrSBIndex..1, 320
  %765 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum156"
  %766 = bitcast i8* %765 to <2 x double>*
  %767 = load <2 x double>* %766, align 16
  store <2 x double> %767, <2 x double> addrspace(1)* %scevgep2.3.i4, align 16
  %scevgep2.4.i5 = getelementptr <2 x double> addrspace(1)* %loadedValue70, i64 256
  %"&pSB[currWI].offset146.sum157" = add i64 %CurrSBIndex..1, 240
  %768 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum157"
  %769 = bitcast i8* %768 to <2 x double>*
  %770 = load <2 x double>* %769, align 16
  store <2 x double> %770, <2 x double> addrspace(1)* %scevgep2.4.i5, align 16
  %scevgep2.5.i6 = getelementptr <2 x double> addrspace(1)* %loadedValue70, i64 320
  %"&pSB[currWI].offset146.sum158" = add i64 %CurrSBIndex..1, 304
  %771 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum158"
  %772 = bitcast i8* %771 to <2 x double>*
  %773 = load <2 x double>* %772, align 16
  store <2 x double> %773, <2 x double> addrspace(1)* %scevgep2.5.i6, align 16
  %scevgep2.6.i7 = getelementptr <2 x double> addrspace(1)* %loadedValue70, i64 384
  %"&pSB[currWI].offset146.sum159" = add i64 %CurrSBIndex..1, 272
  %774 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum159"
  %775 = bitcast i8* %774 to <2 x double>*
  %776 = load <2 x double>* %775, align 16
  store <2 x double> %776, <2 x double> addrspace(1)* %scevgep2.6.i7, align 16
  %scevgep2.7.i8 = getelementptr <2 x double> addrspace(1)* %loadedValue70, i64 448
  %"&pSB[currWI].offset146.sum160" = add i64 %CurrSBIndex..1, 336
  %777 = getelementptr inbounds i8* %pSpecialBuf, i64 %"&pSB[currWI].offset146.sum160"
  %778 = bitcast i8* %777 to <2 x double>*
  %779 = load <2 x double>* %778, align 16
  store <2 x double> %779, <2 x double> addrspace(1)* %scevgep2.7.i8, align 16
  %loadedCurrWI = load i64* %pCurrWI, align 8
  %check.WI.iter = icmp ult i64 %loadedCurrWI, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %SyncBB196
  %"CurrWI++" = add nuw i64 %loadedCurrWI, 1
  store i64 %"CurrWI++", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 608
  br label %SyncBB196

elseBB:                                           ; preds = %SyncBB196
  store i64 0, i64* %pCurrWI, align 8
  ret void
}

define void @__chk1D_512_separated_args(<2 x double> addrspace(1)* nocapture %work, i32 %half_n_cmplx, i32 addrspace(1)* nocapture %fail, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph:
  %0 = sext i32 %half_n_cmplx to i64
  %tmp11.1 = add i32 %half_n_cmplx, 64
  %1 = sext i32 %tmp11.1 to i64
  %tmp11.2 = add i32 %half_n_cmplx, 128
  %2 = sext i32 %tmp11.2 to i64
  %tmp11.3 = add i32 %half_n_cmplx, 192
  %3 = sext i32 %tmp11.3 to i64
  %tmp11.4 = add i32 %half_n_cmplx, 256
  %4 = sext i32 %tmp11.4 to i64
  %tmp11.5 = add i32 %half_n_cmplx, 320
  %5 = sext i32 %tmp11.5 to i64
  %tmp11.6 = add i32 %half_n_cmplx, 384
  %6 = sext i32 %tmp11.6 to i64
  %tmp11.7 = add i32 %half_n_cmplx, 448
  %7 = sext i32 %tmp11.7 to i64
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %bb.nph
  %CurrWI..0 = phi i64 [ 0, %bb.nph ], [ %"CurrWI++", %thenBB ]
  %CurrSBIndex..0 = phi i64 [ 0, %bb.nph ], [ %"loadedCurrSB+Stride", %thenBB ]
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %9 = load i64* %8, align 8
  %10 = load i64* %pWGId, align 8
  %11 = shl i64 %10, 9
  %12 = add i64 %11, %9
  %sext1 = shl i64 %12, 32
  %13 = ashr i64 %sext1, 32
  %tmp16 = add i64 %9, %11
  %sext = shl i64 %tmp16, 32
  %tmp18 = ashr i64 %sext, 32
  %"&(pSB[currWI].offset)46" = add nuw i64 %CurrSBIndex..0, 352
  %"&pSB[currWI].offset47" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)46"
  %scevgep14 = bitcast i8* %"&pSB[currWI].offset47" to <2 x double>*
  %scevgep20 = getelementptr <2 x double> addrspace(1)* %work, i64 %tmp18
  %14 = load <2 x double> addrspace(1)* %scevgep20, align 16
  store <2 x double> %14, <2 x double>* %scevgep14, align 16
  %"&pSB[currWI].offset43.sum" = add i64 %CurrSBIndex..0, 368
  %scevgep14.1 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset43.sum"
  %15 = bitcast i8* %scevgep14.1 to <2 x double>*
  %tmp19.1 = add i64 %tmp18, 64
  %scevgep20.1 = getelementptr <2 x double> addrspace(1)* %work, i64 %tmp19.1
  %16 = load <2 x double> addrspace(1)* %scevgep20.1, align 16
  store <2 x double> %16, <2 x double>* %15, align 16
  %"&pSB[currWI].offset39.sum" = add i64 %CurrSBIndex..0, 384
  %scevgep14.2 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset39.sum"
  %17 = bitcast i8* %scevgep14.2 to <2 x double>*
  %tmp19.2 = add i64 %tmp18, 128
  %scevgep20.2 = getelementptr <2 x double> addrspace(1)* %work, i64 %tmp19.2
  %18 = load <2 x double> addrspace(1)* %scevgep20.2, align 16
  store <2 x double> %18, <2 x double>* %17, align 16
  %"&pSB[currWI].offset35.sum" = add i64 %CurrSBIndex..0, 400
  %scevgep14.3 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset35.sum"
  %19 = bitcast i8* %scevgep14.3 to <2 x double>*
  %tmp19.3 = add i64 %tmp18, 192
  %scevgep20.3 = getelementptr <2 x double> addrspace(1)* %work, i64 %tmp19.3
  %20 = load <2 x double> addrspace(1)* %scevgep20.3, align 16
  store <2 x double> %20, <2 x double>* %19, align 16
  %"&pSB[currWI].offset31.sum" = add i64 %CurrSBIndex..0, 416
  %scevgep14.4 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset31.sum"
  %21 = bitcast i8* %scevgep14.4 to <2 x double>*
  %tmp19.4 = add i64 %tmp18, 256
  %scevgep20.4 = getelementptr <2 x double> addrspace(1)* %work, i64 %tmp19.4
  %22 = load <2 x double> addrspace(1)* %scevgep20.4, align 16
  store <2 x double> %22, <2 x double>* %21, align 16
  %"&pSB[currWI].offset27.sum" = add i64 %CurrSBIndex..0, 432
  %scevgep14.5 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset27.sum"
  %23 = bitcast i8* %scevgep14.5 to <2 x double>*
  %tmp19.5 = add i64 %tmp18, 320
  %scevgep20.5 = getelementptr <2 x double> addrspace(1)* %work, i64 %tmp19.5
  %24 = load <2 x double> addrspace(1)* %scevgep20.5, align 16
  store <2 x double> %24, <2 x double>* %23, align 16
  %"&pSB[currWI].offset23.sum" = add i64 %CurrSBIndex..0, 448
  %scevgep14.6 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset23.sum"
  %25 = bitcast i8* %scevgep14.6 to <2 x double>*
  %tmp19.6 = add i64 %tmp18, 384
  %scevgep20.6 = getelementptr <2 x double> addrspace(1)* %work, i64 %tmp19.6
  %26 = load <2 x double> addrspace(1)* %scevgep20.6, align 16
  store <2 x double> %26, <2 x double>* %25, align 16
  %"&pSB[currWI].offset.sum" = add i64 %CurrSBIndex..0, 464
  %scevgep14.7 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset.sum"
  %27 = bitcast i8* %scevgep14.7 to <2 x double>*
  %tmp19.7 = add i64 %tmp18, 448
  %scevgep20.7 = getelementptr <2 x double> addrspace(1)* %work, i64 %tmp19.7
  %28 = load <2 x double> addrspace(1)* %scevgep20.7, align 16
  store <2 x double> %28, <2 x double>* %27, align 16
  %"&(pSB[currWI].offset)78" = add nuw i64 %CurrSBIndex..0, 480
  %"&pSB[currWI].offset79" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)78"
  %scevgep13 = bitcast i8* %"&pSB[currWI].offset79" to <2 x double>*
  %.sum = add i64 %0, %13
  %29 = getelementptr inbounds <2 x double> addrspace(1)* %work, i64 %.sum
  %30 = load <2 x double> addrspace(1)* %29, align 16
  store <2 x double> %30, <2 x double>* %scevgep13, align 16
  %"&pSB[currWI].offset75.sum" = add i64 %CurrSBIndex..0, 496
  %scevgep13.1 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset75.sum"
  %31 = bitcast i8* %scevgep13.1 to <2 x double>*
  %.sum.1 = add i64 %1, %13
  %32 = getelementptr inbounds <2 x double> addrspace(1)* %work, i64 %.sum.1
  %33 = load <2 x double> addrspace(1)* %32, align 16
  store <2 x double> %33, <2 x double>* %31, align 16
  %"&pSB[currWI].offset71.sum" = add i64 %CurrSBIndex..0, 512
  %scevgep13.2 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset71.sum"
  %34 = bitcast i8* %scevgep13.2 to <2 x double>*
  %.sum.2 = add i64 %2, %13
  %35 = getelementptr inbounds <2 x double> addrspace(1)* %work, i64 %.sum.2
  %36 = load <2 x double> addrspace(1)* %35, align 16
  store <2 x double> %36, <2 x double>* %34, align 16
  %"&pSB[currWI].offset67.sum" = add i64 %CurrSBIndex..0, 528
  %scevgep13.3 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset67.sum"
  %37 = bitcast i8* %scevgep13.3 to <2 x double>*
  %.sum.3 = add i64 %3, %13
  %38 = getelementptr inbounds <2 x double> addrspace(1)* %work, i64 %.sum.3
  %39 = load <2 x double> addrspace(1)* %38, align 16
  store <2 x double> %39, <2 x double>* %37, align 16
  %"&pSB[currWI].offset63.sum" = add i64 %CurrSBIndex..0, 544
  %scevgep13.4 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset63.sum"
  %40 = bitcast i8* %scevgep13.4 to <2 x double>*
  %.sum.4 = add i64 %4, %13
  %41 = getelementptr inbounds <2 x double> addrspace(1)* %work, i64 %.sum.4
  %42 = load <2 x double> addrspace(1)* %41, align 16
  store <2 x double> %42, <2 x double>* %40, align 16
  %"&pSB[currWI].offset59.sum" = add i64 %CurrSBIndex..0, 560
  %scevgep13.5 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset59.sum"
  %43 = bitcast i8* %scevgep13.5 to <2 x double>*
  %.sum.5 = add i64 %5, %13
  %44 = getelementptr inbounds <2 x double> addrspace(1)* %work, i64 %.sum.5
  %45 = load <2 x double> addrspace(1)* %44, align 16
  store <2 x double> %45, <2 x double>* %43, align 16
  %"&pSB[currWI].offset55.sum" = add i64 %CurrSBIndex..0, 576
  %scevgep13.6 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset55.sum"
  %46 = bitcast i8* %scevgep13.6 to <2 x double>*
  %.sum.6 = add i64 %6, %13
  %47 = getelementptr inbounds <2 x double> addrspace(1)* %work, i64 %.sum.6
  %48 = load <2 x double> addrspace(1)* %47, align 16
  store <2 x double> %48, <2 x double>* %46, align 16
  %"&pSB[currWI].offset51.sum" = add i64 %CurrSBIndex..0, 592
  %scevgep13.7 = getelementptr i8* %pSpecialBuf, i64 %"&pSB[currWI].offset51.sum"
  %49 = bitcast i8* %scevgep13.7 to <2 x double>*
  %.sum.7 = add i64 %7, %13
  %50 = getelementptr inbounds <2 x double> addrspace(1)* %work, i64 %.sum.7
  %51 = load <2 x double> addrspace(1)* %50, align 16
  store <2 x double> %51, <2 x double>* %49, align 16
  %52 = extractelement <2 x double> %14, i32 0
  %53 = extractelement <2 x double> %30, i32 0
  %54 = fcmp une double %52, %53
  br i1 %54, label %59, label %55

; <label>:55                                      ; preds = %SyncBB
  %56 = extractelement <2 x double> %14, i32 1
  %57 = extractelement <2 x double> %30, i32 1
  %58 = fcmp une double %56, %57
  br i1 %58, label %59, label %60

; <label>:59                                      ; preds = %55, %SyncBB
  store i32 1, i32 addrspace(1)* %fail, align 4
  br label %60

; <label>:60                                      ; preds = %59, %55
  %61 = extractelement <2 x double> %16, i32 0
  %62 = extractelement <2 x double> %33, i32 0
  %63 = fcmp une double %61, %62
  br i1 %63, label %68, label %69

; <label>:64                                      ; preds = %69, %68
  %65 = extractelement <2 x double> %18, i32 0
  %66 = extractelement <2 x double> %36, i32 0
  %67 = fcmp une double %65, %66
  br i1 %67, label %77, label %78

; <label>:68                                      ; preds = %69, %60
  store i32 1, i32 addrspace(1)* %fail, align 4
  br label %64

; <label>:69                                      ; preds = %60
  %70 = extractelement <2 x double> %16, i32 1
  %71 = extractelement <2 x double> %33, i32 1
  %72 = fcmp une double %70, %71
  br i1 %72, label %68, label %64

; <label>:73                                      ; preds = %78, %77
  %74 = extractelement <2 x double> %20, i32 0
  %75 = extractelement <2 x double> %39, i32 0
  %76 = fcmp une double %74, %75
  br i1 %76, label %86, label %87

; <label>:77                                      ; preds = %78, %64
  store i32 1, i32 addrspace(1)* %fail, align 4
  br label %73

; <label>:78                                      ; preds = %64
  %79 = extractelement <2 x double> %18, i32 1
  %80 = extractelement <2 x double> %36, i32 1
  %81 = fcmp une double %79, %80
  br i1 %81, label %77, label %73

; <label>:82                                      ; preds = %87, %86
  %83 = extractelement <2 x double> %22, i32 0
  %84 = extractelement <2 x double> %42, i32 0
  %85 = fcmp une double %83, %84
  br i1 %85, label %95, label %96

; <label>:86                                      ; preds = %87, %73
  store i32 1, i32 addrspace(1)* %fail, align 4
  br label %82

; <label>:87                                      ; preds = %73
  %88 = extractelement <2 x double> %20, i32 1
  %89 = extractelement <2 x double> %39, i32 1
  %90 = fcmp une double %88, %89
  br i1 %90, label %86, label %82

; <label>:91                                      ; preds = %96, %95
  %92 = extractelement <2 x double> %24, i32 0
  %93 = extractelement <2 x double> %45, i32 0
  %94 = fcmp une double %92, %93
  br i1 %94, label %104, label %105

; <label>:95                                      ; preds = %96, %82
  store i32 1, i32 addrspace(1)* %fail, align 4
  br label %91

; <label>:96                                      ; preds = %82
  %97 = extractelement <2 x double> %22, i32 1
  %98 = extractelement <2 x double> %42, i32 1
  %99 = fcmp une double %97, %98
  br i1 %99, label %95, label %91

; <label>:100                                     ; preds = %105, %104
  %101 = extractelement <2 x double> %26, i32 0
  %102 = extractelement <2 x double> %48, i32 0
  %103 = fcmp une double %101, %102
  br i1 %103, label %113, label %114

; <label>:104                                     ; preds = %105, %91
  store i32 1, i32 addrspace(1)* %fail, align 4
  br label %100

; <label>:105                                     ; preds = %91
  %106 = extractelement <2 x double> %24, i32 1
  %107 = extractelement <2 x double> %45, i32 1
  %108 = fcmp une double %106, %107
  br i1 %108, label %104, label %100

; <label>:109                                     ; preds = %114, %113
  %110 = extractelement <2 x double> %28, i32 0
  %111 = extractelement <2 x double> %51, i32 0
  %112 = fcmp une double %110, %111
  br i1 %112, label %118, label %119

; <label>:113                                     ; preds = %114, %100
  store i32 1, i32 addrspace(1)* %fail, align 4
  br label %109

; <label>:114                                     ; preds = %100
  %115 = extractelement <2 x double> %26, i32 1
  %116 = extractelement <2 x double> %48, i32 1
  %117 = fcmp une double %115, %116
  br i1 %117, label %113, label %109

; <label>:118                                     ; preds = %119, %109
  store i32 1, i32 addrspace(1)* %fail, align 4
  br label %UnifiedReturnBlock

; <label>:119                                     ; preds = %109
  %120 = extractelement <2 x double> %28, i32 1
  %121 = extractelement <2 x double> %51, i32 1
  %122 = fcmp une double %120, %121
  br i1 %122, label %118, label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %119, %118
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB81

thenBB:                                           ; preds = %UnifiedReturnBlock
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 608
  br label %SyncBB

SyncBB81:                                         ; preds = %UnifiedReturnBlock
  ret void
}

define void @ifft1D_512(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <2 x double> addrspace(1)**
  %1 = load <2 x double> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i8 addrspace(3)**
  %4 = load i8 addrspace(3)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 24
  %6 = bitcast i8* %5 to i64**
  %7 = load i64** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to i8**
  %16 = load i8** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to i64**
  %19 = load i64** %18, align 8
  %20 = bitcast i8 addrspace(3)* %4 to [576 x double] addrspace(3)*
  store i64 0, i64* %19, align 8
  br label %SyncBB189.i

SyncBB189.i:                                      ; preds = %thenBB198.i, %entry
  %CurrSBIndex..1.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride204.i", %thenBB198.i ]
  %currWI.i = load i64* %19, align 8
  %21 = getelementptr %struct.PaddedDimId* %10, i64 %currWI.i, i32 0, i64 0
  %22 = load i64* %21, align 8
  %23 = trunc i64 %22 to i32
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..1.i, 176
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %23, i32* %CastToValueType.i, align 4
  %24 = load i64* %7, align 8
  %25 = shl i64 %24, 9
  %26 = add i64 %25, %22
  %27 = ashr i32 %23, 3
  %"&(pSB[currWI].offset)35.i" = add nuw i64 %CurrSBIndex..1.i, 180
  %"&pSB[currWI].offset36.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)35.i"
  %CastToValueType37.i = bitcast i8* %"&pSB[currWI].offset36.i" to i32*
  store i32 %27, i32* %CastToValueType37.i, align 4
  %28 = and i32 %23, 7
  %"&(pSB[currWI].offset)49.i" = add nuw i64 %CurrSBIndex..1.i, 184
  %"&pSB[currWI].offset50.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)49.i"
  %CastToValueType51.i = bitcast i8* %"&pSB[currWI].offset50.i" to i32*
  store i32 %28, i32* %CastToValueType51.i, align 4
  %sext.i = shl i64 %26, 32
  %29 = ashr i64 %sext.i, 32
  %30 = getelementptr inbounds <2 x double> addrspace(1)* %1, i64 %29
  %"&(pSB[currWI].offset)63.i" = add nuw i64 %CurrSBIndex..1.i, 192
  %"&pSB[currWI].offset64.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)63.i"
  %CastToValueType65.i = bitcast i8* %"&pSB[currWI].offset64.i" to <2 x double> addrspace(1)**
  store <2 x double> addrspace(1)* %30, <2 x double> addrspace(1)** %CastToValueType65.i, align 8
  %"&(pSB[currWI].offset)145.i" = add nuw i64 %CurrSBIndex..1.i, 224
  %"&pSB[currWI].offset146.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)145.i"
  %31 = bitcast i8* %"&pSB[currWI].offset146.i" to <2 x double>*
  %32 = load <2 x double> addrspace(1)* %30, align 16
  store <2 x double> %32, <2 x double>* %31, align 16
  %"&pSB[currWI].offset146.sum.i" = add i64 %CurrSBIndex..1.i, 240
  %scevgep.1.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum.i"
  %33 = bitcast i8* %scevgep.1.i.i to <2 x double>*
  %.sum.i = add i64 %29, 64
  %scevgep2.1.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum.i
  %34 = load <2 x double> addrspace(1)* %scevgep2.1.i.i, align 16
  store <2 x double> %34, <2 x double>* %33, align 16
  %"&pSB[currWI].offset146.sum65.i" = add i64 %CurrSBIndex..1.i, 256
  %scevgep.2.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum65.i"
  %35 = bitcast i8* %scevgep.2.i.i to <2 x double>*
  %.sum66.i = add i64 %29, 128
  %scevgep2.2.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum66.i
  %36 = load <2 x double> addrspace(1)* %scevgep2.2.i.i, align 16
  store <2 x double> %36, <2 x double>* %35, align 16
  %"&pSB[currWI].offset146.sum67.i" = add i64 %CurrSBIndex..1.i, 272
  %scevgep.3.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum67.i"
  %37 = bitcast i8* %scevgep.3.i.i to <2 x double>*
  %.sum68.i = add i64 %29, 192
  %scevgep2.3.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum68.i
  %38 = load <2 x double> addrspace(1)* %scevgep2.3.i.i, align 16
  store <2 x double> %38, <2 x double>* %37, align 16
  %"&pSB[currWI].offset146.sum69.i" = add i64 %CurrSBIndex..1.i, 288
  %scevgep.4.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum69.i"
  %39 = bitcast i8* %scevgep.4.i.i to <2 x double>*
  %.sum70.i = add i64 %29, 256
  %scevgep2.4.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum70.i
  %40 = load <2 x double> addrspace(1)* %scevgep2.4.i.i, align 16
  store <2 x double> %40, <2 x double>* %39, align 16
  %"&pSB[currWI].offset146.sum71.i" = add i64 %CurrSBIndex..1.i, 304
  %scevgep.5.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum71.i"
  %41 = bitcast i8* %scevgep.5.i.i to <2 x double>*
  %.sum72.i = add i64 %29, 320
  %scevgep2.5.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum72.i
  %42 = load <2 x double> addrspace(1)* %scevgep2.5.i.i, align 16
  store <2 x double> %42, <2 x double>* %41, align 16
  %"&pSB[currWI].offset146.sum73.i" = add i64 %CurrSBIndex..1.i, 320
  %scevgep.6.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum73.i"
  %43 = bitcast i8* %scevgep.6.i.i to <2 x double>*
  %.sum74.i = add i64 %29, 384
  %scevgep2.6.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum74.i
  %44 = load <2 x double> addrspace(1)* %scevgep2.6.i.i, align 16
  store <2 x double> %44, <2 x double>* %43, align 16
  %"&pSB[currWI].offset146.sum75.i" = add i64 %CurrSBIndex..1.i, 336
  %scevgep.7.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum75.i"
  %45 = bitcast i8* %scevgep.7.i.i to <2 x double>*
  %.sum76.i = add i64 %29, 448
  %scevgep2.7.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum76.i
  %46 = load <2 x double> addrspace(1)* %scevgep2.7.i.i, align 16
  store <2 x double> %46, <2 x double>* %45, align 16
  %47 = load <2 x double>* %31, align 16
  %"&pSB[currWI].offset142.sum.i" = add i64 %CurrSBIndex..1.i, 288
  %48 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset142.sum.i"
  %49 = bitcast i8* %48 to <2 x double>*
  %50 = load <2 x double>* %49, align 16
  %51 = extractelement <2 x double> %47, i32 0
  %52 = extractelement <2 x double> %50, i32 0
  %53 = fadd double %51, %52
  %54 = extractelement <2 x double> %47, i32 1
  %55 = extractelement <2 x double> %50, i32 1
  %56 = fadd double %54, %55
  %57 = extractelement <2 x double> %47, i32 0
  %58 = extractelement <2 x double> %50, i32 0
  %59 = fsub double %57, %58
  %60 = extractelement <2 x double> %47, i32 1
  %61 = extractelement <2 x double> %50, i32 1
  %62 = fsub double %60, %61
  %"&pSB[currWI].offset138.sum.i" = add i64 %CurrSBIndex..1.i, 240
  %63 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset138.sum.i"
  %64 = bitcast i8* %63 to <2 x double>*
  %65 = load <2 x double>* %64, align 16
  %"&pSB[currWI].offset134.sum.i" = add i64 %CurrSBIndex..1.i, 304
  %66 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset134.sum.i"
  %67 = bitcast i8* %66 to <2 x double>*
  %68 = load <2 x double>* %67, align 16
  %69 = extractelement <2 x double> %65, i32 0
  %70 = extractelement <2 x double> %68, i32 0
  %71 = fadd double %69, %70
  %72 = extractelement <2 x double> %65, i32 1
  %73 = extractelement <2 x double> %68, i32 1
  %74 = fadd double %72, %73
  %75 = extractelement <2 x double> %65, i32 0
  %76 = extractelement <2 x double> %68, i32 0
  %77 = fsub double %75, %76
  %78 = extractelement <2 x double> %65, i32 1
  %79 = extractelement <2 x double> %68, i32 1
  %80 = fsub double %78, %79
  %"&pSB[currWI].offset130.sum.i" = add i64 %CurrSBIndex..1.i, 256
  %81 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset130.sum.i"
  %82 = bitcast i8* %81 to <2 x double>*
  %83 = load <2 x double>* %82, align 16
  %"&pSB[currWI].offset126.sum.i" = add i64 %CurrSBIndex..1.i, 320
  %84 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset126.sum.i"
  %85 = bitcast i8* %84 to <2 x double>*
  %86 = load <2 x double>* %85, align 16
  %87 = extractelement <2 x double> %83, i32 0
  %88 = extractelement <2 x double> %86, i32 0
  %89 = fadd double %87, %88
  %90 = extractelement <2 x double> %83, i32 1
  %91 = extractelement <2 x double> %86, i32 1
  %92 = fadd double %90, %91
  %93 = extractelement <2 x double> %83, i32 0
  %94 = extractelement <2 x double> %86, i32 0
  %95 = fsub double %93, %94
  %96 = extractelement <2 x double> %83, i32 1
  %97 = extractelement <2 x double> %86, i32 1
  %98 = fsub double %96, %97
  %"&pSB[currWI].offset122.sum.i" = add i64 %CurrSBIndex..1.i, 272
  %99 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset122.sum.i"
  %100 = bitcast i8* %99 to <2 x double>*
  %101 = load <2 x double>* %100, align 16
  %"&pSB[currWI].offset118.sum.i" = add i64 %CurrSBIndex..1.i, 336
  %102 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset118.sum.i"
  %103 = bitcast i8* %102 to <2 x double>*
  %104 = load <2 x double>* %103, align 16
  %105 = extractelement <2 x double> %101, i32 0
  %106 = extractelement <2 x double> %104, i32 0
  %107 = fadd double %105, %106
  %108 = extractelement <2 x double> %101, i32 1
  %109 = extractelement <2 x double> %104, i32 1
  %110 = fadd double %108, %109
  %111 = extractelement <2 x double> %101, i32 0
  %112 = extractelement <2 x double> %104, i32 0
  %113 = fsub double %111, %112
  %114 = extractelement <2 x double> %101, i32 1
  %115 = extractelement <2 x double> %104, i32 1
  %116 = fsub double %114, %115
  %117 = fsub double %77, %80
  %118 = fadd double %77, %80
  %119 = fmul double %117, 0x3FE6A09E667F3BCD
  %120 = fmul double %118, 0x3FE6A09E667F3BCD
  %121 = fmul double %95, 0.000000e+00
  %122 = fsub double %121, %98
  %123 = fmul double %98, 0.000000e+00
  %124 = fadd double %95, %123
  %125 = fmul double %113, -1.000000e+00
  %126 = fsub double %125, %116
  %127 = fmul double %116, -1.000000e+00
  %128 = fadd double %113, %127
  %129 = fmul double %126, 0x3FE6A09E667F3BCD
  %130 = fmul double %128, 0x3FE6A09E667F3BCD
  %131 = fadd double %53, %89
  %132 = fadd double %56, %92
  %133 = fsub double %53, %89
  %134 = fsub double %56, %92
  %135 = fadd double %71, %107
  %136 = fadd double %74, %110
  %137 = fsub double %71, %107
  %138 = fsub double %74, %110
  %139 = fmul double %137, 0.000000e+00
  %140 = fsub double %139, %138
  %141 = fmul double %138, 0.000000e+00
  %142 = fadd double %137, %141
  %143 = fadd double %131, %135
  %144 = insertelement <2 x double> undef, double %143, i32 0
  %145 = fadd double %132, %136
  %146 = insertelement <2 x double> %144, double %145, i32 1
  store <2 x double> %146, <2 x double>* %31, align 16
  %147 = fsub double %131, %135
  %148 = insertelement <2 x double> undef, double %147, i32 0
  %149 = fsub double %132, %136
  %150 = insertelement <2 x double> %148, double %149, i32 1
  store <2 x double> %150, <2 x double>* %64, align 16
  %151 = fadd double %133, %140
  %152 = insertelement <2 x double> undef, double %151, i32 0
  %153 = fadd double %134, %142
  %154 = insertelement <2 x double> %152, double %153, i32 1
  store <2 x double> %154, <2 x double>* %82, align 16
  %155 = fsub double %133, %140
  %156 = insertelement <2 x double> undef, double %155, i32 0
  %157 = fsub double %134, %142
  %158 = insertelement <2 x double> %156, double %157, i32 1
  store <2 x double> %158, <2 x double>* %100, align 16
  %159 = fadd double %59, %122
  %160 = fadd double %62, %124
  %161 = fsub double %59, %122
  %162 = fsub double %62, %124
  %163 = fadd double %119, %129
  %164 = fadd double %120, %130
  %165 = fsub double %119, %129
  %166 = fsub double %120, %130
  %167 = fmul double %165, 0.000000e+00
  %168 = fsub double %167, %166
  %169 = fmul double %166, 0.000000e+00
  %170 = fadd double %165, %169
  %171 = fadd double %159, %163
  %172 = insertelement <2 x double> undef, double %171, i32 0
  %173 = fadd double %160, %164
  %174 = insertelement <2 x double> %172, double %173, i32 1
  store <2 x double> %174, <2 x double>* %49, align 16
  %175 = fsub double %159, %163
  %176 = insertelement <2 x double> undef, double %175, i32 0
  %177 = fsub double %160, %164
  %178 = insertelement <2 x double> %176, double %177, i32 1
  store <2 x double> %178, <2 x double>* %67, align 16
  %179 = fadd double %161, %168
  %180 = insertelement <2 x double> undef, double %179, i32 0
  %181 = fadd double %162, %170
  %182 = insertelement <2 x double> %180, double %181, i32 1
  store <2 x double> %182, <2 x double>* %85, align 16
  %183 = fsub double %161, %168
  %184 = insertelement <2 x double> undef, double %183, i32 0
  %185 = fsub double %162, %170
  %186 = insertelement <2 x double> %184, double %185, i32 1
  store <2 x double> %186, <2 x double>* %103, align 16
  %187 = sitofp i32 %23 to double
  %"&(pSB[currWI].offset)109.i" = add nuw i64 %CurrSBIndex..1.i, 224
  %"&pSB[currWI].offset110.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)109.i"
  %CastToValueType111.i = bitcast i8* %"&pSB[currWI].offset110.i" to [8 x <2 x double>]*
  br label %188

; <label>:188                                     ; preds = %._crit_edge16.i, %SyncBB189.i
  %indvar10.i = phi i64 [ 1, %SyncBB189.i ], [ %phitmp.i, %._crit_edge16.i ]
  %scevgep14.i = getelementptr [8 x <2 x double>]* %CastToValueType111.i, i64 0, i64 %indvar10.i
  %scevgep15.i = getelementptr [8 x i32]* @fft1D_512.reversed8, i64 0, i64 %indvar10.i
  %189 = load <2 x double>* %scevgep14.i, align 16
  %190 = load i32* %scevgep15.i, align 4
  %191 = sitofp i32 %190 to double
  %192 = fmul double %191, 0x401921FB54442D18
  %193 = fdiv double %192, 5.120000e+02
  %194 = fmul double %193, %187
  %195 = call double @_Z3cosd(double %194) nounwind
  %196 = call double @_Z3sind(double %194) nounwind
  %197 = extractelement <2 x double> %189, i32 0
  %198 = fmul double %197, %195
  %199 = extractelement <2 x double> %189, i32 1
  %200 = fmul double %199, %196
  %201 = fsub double %198, %200
  %202 = insertelement <2 x double> undef, double %201, i32 0
  %203 = fmul double %197, %196
  %204 = fmul double %199, %195
  %205 = fadd double %203, %204
  %206 = insertelement <2 x double> %202, double %205, i32 1
  store <2 x double> %206, <2 x double>* %scevgep14.i, align 16
  %exitcond12.i = icmp eq i64 %indvar10.i, 7
  br i1 %exitcond12.i, label %bb.nph3.i, label %._crit_edge16.i

._crit_edge16.i:                                  ; preds = %188
  %phitmp.i = add i64 %indvar10.i, 1
  br label %188

bb.nph3.i:                                        ; preds = %188
  %"&(pSB[currWI].offset)30.i" = add nuw i64 %CurrSBIndex..1.i, 176
  %"&pSB[currWI].offset31.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)30.i"
  %CastToValueType32.i = bitcast i8* %"&pSB[currWI].offset31.i" to i32*
  %loadedValue33.i = load i32* %CastToValueType32.i, align 4
  %207 = sext i32 %loadedValue33.i to i64
  %208 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %207
  %"&(pSB[currWI].offset)72.i" = add nuw i64 %CurrSBIndex..1.i, 200
  %"&pSB[currWI].offset73.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)72.i"
  %CastToValueType74.i = bitcast i8* %"&pSB[currWI].offset73.i" to double addrspace(3)**
  store double addrspace(3)* %208, double addrspace(3)** %CastToValueType74.i, align 8
  %209 = load <2 x double>* %31, align 16
  %210 = extractelement <2 x double> %209, i32 0
  store double %210, double addrspace(3)* %208, align 8
  %"&pSB[currWI].offset146.sum77.i" = add i64 %CurrSBIndex..1.i, 288
  %211 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum77.i"
  %212 = bitcast i8* %211 to <2 x double>*
  %213 = load <2 x double>* %212, align 16
  %214 = extractelement <2 x double> %213, i32 0
  %.sum78.i = add i64 %207, 66
  %215 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum78.i
  store double %214, double addrspace(3)* %215, align 8
  %"&pSB[currWI].offset146.sum79.i" = add i64 %CurrSBIndex..1.i, 256
  %216 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum79.i"
  %217 = bitcast i8* %216 to <2 x double>*
  %218 = load <2 x double>* %217, align 16
  %219 = extractelement <2 x double> %218, i32 0
  %.sum80.i = add i64 %207, 132
  %220 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum80.i
  store double %219, double addrspace(3)* %220, align 8
  %"&pSB[currWI].offset146.sum81.i" = add i64 %CurrSBIndex..1.i, 320
  %221 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum81.i"
  %222 = bitcast i8* %221 to <2 x double>*
  %223 = load <2 x double>* %222, align 16
  %224 = extractelement <2 x double> %223, i32 0
  %.sum82.i = add i64 %207, 198
  %225 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum82.i
  store double %224, double addrspace(3)* %225, align 8
  %"&pSB[currWI].offset146.sum83.i" = add i64 %CurrSBIndex..1.i, 240
  %226 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum83.i"
  %227 = bitcast i8* %226 to <2 x double>*
  %228 = load <2 x double>* %227, align 16
  %229 = extractelement <2 x double> %228, i32 0
  %.sum84.i = add i64 %207, 264
  %230 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum84.i
  store double %229, double addrspace(3)* %230, align 8
  %"&pSB[currWI].offset146.sum85.i" = add i64 %CurrSBIndex..1.i, 304
  %231 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum85.i"
  %232 = bitcast i8* %231 to <2 x double>*
  %233 = load <2 x double>* %232, align 16
  %234 = extractelement <2 x double> %233, i32 0
  %.sum86.i = add i64 %207, 330
  %235 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum86.i
  store double %234, double addrspace(3)* %235, align 8
  %"&pSB[currWI].offset146.sum87.i" = add i64 %CurrSBIndex..1.i, 272
  %236 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum87.i"
  %237 = bitcast i8* %236 to <2 x double>*
  %238 = load <2 x double>* %237, align 16
  %239 = extractelement <2 x double> %238, i32 0
  %.sum88.i = add i64 %207, 396
  %240 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum88.i
  store double %239, double addrspace(3)* %240, align 8
  %"&pSB[currWI].offset146.sum89.i" = add i64 %CurrSBIndex..1.i, 336
  %241 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum89.i"
  %242 = bitcast i8* %241 to <2 x double>*
  %243 = load <2 x double>* %242, align 16
  %244 = extractelement <2 x double> %243, i32 0
  %.sum90.i = add i64 %207, 462
  %245 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum90.i
  store double %244, double addrspace(3)* %245, align 8
  %loadedCurrWI200.i = load i64* %19, align 8
  %check.WI.iter201.i = icmp ult i64 %loadedCurrWI200.i, %13
  br i1 %check.WI.iter201.i, label %thenBB198.i, label %elseBB199.i

thenBB198.i:                                      ; preds = %bb.nph3.i
  %"CurrWI++202.i" = add nuw i64 %loadedCurrWI200.i, 1
  store i64 %"CurrWI++202.i", i64* %19, align 8
  %"loadedCurrSB+Stride204.i" = add nuw i64 %CurrSBIndex..1.i, 608
  br label %SyncBB189.i

elseBB199.i:                                      ; preds = %bb.nph3.i
  store i64 0, i64* %19, align 8
  br label %SyncBB190.i

SyncBB190.i:                                      ; preds = %thenBB205.i, %elseBB199.i
  %CurrSBIndex..2.i = phi i64 [ 0, %elseBB199.i ], [ %"loadedCurrSB+Stride211.i", %thenBB205.i ]
  %"&(pSB[currWI].offset)58.i" = add nuw i64 %CurrSBIndex..2.i, 184
  %"&pSB[currWI].offset59.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)58.i"
  %CastToValueType60.i = bitcast i8* %"&pSB[currWI].offset59.i" to i32*
  %loadedValue61.i = load i32* %CastToValueType60.i, align 4
  %246 = mul nsw i32 %loadedValue61.i, 66
  %"&(pSB[currWI].offset)39.i" = add nuw i64 %CurrSBIndex..2.i, 180
  %"&pSB[currWI].offset40.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)39.i"
  %CastToValueType41.i = bitcast i8* %"&pSB[currWI].offset40.i" to i32*
  %loadedValue42.i = load i32* %CastToValueType41.i, align 4
  %247 = add nsw i32 %246, %loadedValue42.i
  %248 = sext i32 %247 to i64
  %249 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %248
  %"&(pSB[currWI].offset)91.i" = add nuw i64 %CurrSBIndex..2.i, 208
  %"&pSB[currWI].offset92.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)91.i"
  %CastToValueType93.i = bitcast i8* %"&pSB[currWI].offset92.i" to double addrspace(3)**
  store double addrspace(3)* %249, double addrspace(3)** %CastToValueType93.i, align 8
  %250 = load double addrspace(3)* %249, align 8
  %251 = load <2 x double>* %31, align 16
  %252 = insertelement <2 x double> %251, double %250, i32 0
  store <2 x double> %252, <2 x double>* %31, align 16
  %"&pSB[currWI].offset146.sum91.i" = add i64 %CurrSBIndex..1.i, 240
  %scevgep.1.i51.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum91.i"
  %253 = bitcast i8* %scevgep.1.i51.i to <2 x double>*
  %.sum92.i = add i64 %248, 8
  %scevgep2.1.i52.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum92.i
  %254 = load double addrspace(3)* %scevgep2.1.i52.i, align 8
  %255 = load <2 x double>* %253, align 16
  %256 = insertelement <2 x double> %255, double %254, i32 0
  store <2 x double> %256, <2 x double>* %253, align 16
  %"&pSB[currWI].offset146.sum93.i" = add i64 %CurrSBIndex..1.i, 256
  %scevgep.2.i53.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum93.i"
  %257 = bitcast i8* %scevgep.2.i53.i to <2 x double>*
  %.sum94.i = add i64 %248, 16
  %scevgep2.2.i54.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum94.i
  %258 = load double addrspace(3)* %scevgep2.2.i54.i, align 8
  %259 = load <2 x double>* %257, align 16
  %260 = insertelement <2 x double> %259, double %258, i32 0
  store <2 x double> %260, <2 x double>* %257, align 16
  %"&pSB[currWI].offset146.sum95.i" = add i64 %CurrSBIndex..1.i, 272
  %scevgep.3.i55.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum95.i"
  %261 = bitcast i8* %scevgep.3.i55.i to <2 x double>*
  %.sum96.i = add i64 %248, 24
  %scevgep2.3.i56.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum96.i
  %262 = load double addrspace(3)* %scevgep2.3.i56.i, align 8
  %263 = load <2 x double>* %261, align 16
  %264 = insertelement <2 x double> %263, double %262, i32 0
  store <2 x double> %264, <2 x double>* %261, align 16
  %"&pSB[currWI].offset146.sum97.i" = add i64 %CurrSBIndex..1.i, 288
  %scevgep.4.i57.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum97.i"
  %265 = bitcast i8* %scevgep.4.i57.i to <2 x double>*
  %.sum98.i = add i64 %248, 32
  %scevgep2.4.i58.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum98.i
  %266 = load double addrspace(3)* %scevgep2.4.i58.i, align 8
  %267 = load <2 x double>* %265, align 16
  %268 = insertelement <2 x double> %267, double %266, i32 0
  store <2 x double> %268, <2 x double>* %265, align 16
  %"&pSB[currWI].offset146.sum99.i" = add i64 %CurrSBIndex..1.i, 304
  %scevgep.5.i59.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum99.i"
  %269 = bitcast i8* %scevgep.5.i59.i to <2 x double>*
  %.sum100.i = add i64 %248, 40
  %scevgep2.5.i60.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum100.i
  %270 = load double addrspace(3)* %scevgep2.5.i60.i, align 8
  %271 = load <2 x double>* %269, align 16
  %272 = insertelement <2 x double> %271, double %270, i32 0
  store <2 x double> %272, <2 x double>* %269, align 16
  %"&pSB[currWI].offset146.sum101.i" = add i64 %CurrSBIndex..1.i, 320
  %scevgep.6.i61.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum101.i"
  %273 = bitcast i8* %scevgep.6.i61.i to <2 x double>*
  %.sum102.i = add i64 %248, 48
  %scevgep2.6.i62.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum102.i
  %274 = load double addrspace(3)* %scevgep2.6.i62.i, align 8
  %275 = load <2 x double>* %273, align 16
  %276 = insertelement <2 x double> %275, double %274, i32 0
  store <2 x double> %276, <2 x double>* %273, align 16
  %"&pSB[currWI].offset146.sum103.i" = add i64 %CurrSBIndex..1.i, 336
  %scevgep.7.i63.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum103.i"
  %277 = bitcast i8* %scevgep.7.i63.i to <2 x double>*
  %.sum104.i = add i64 %248, 56
  %scevgep2.7.i64.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum104.i
  %278 = load double addrspace(3)* %scevgep2.7.i64.i, align 8
  %279 = load <2 x double>* %277, align 16
  %280 = insertelement <2 x double> %279, double %278, i32 0
  store <2 x double> %280, <2 x double>* %277, align 16
  %loadedCurrWI207.i = load i64* %19, align 8
  %check.WI.iter208.i = icmp ult i64 %loadedCurrWI207.i, %13
  br i1 %check.WI.iter208.i, label %thenBB205.i, label %elseBB206.i

thenBB205.i:                                      ; preds = %SyncBB190.i
  %"CurrWI++209.i" = add nuw i64 %loadedCurrWI207.i, 1
  store i64 %"CurrWI++209.i", i64* %19, align 8
  %"loadedCurrSB+Stride211.i" = add nuw i64 %CurrSBIndex..2.i, 608
  br label %SyncBB190.i

elseBB206.i:                                      ; preds = %SyncBB190.i
  store i64 0, i64* %19, align 8
  br label %SyncBB191.i

SyncBB191.i:                                      ; preds = %thenBB212.i, %elseBB206.i
  %CurrSBIndex..3.i = phi i64 [ 0, %elseBB206.i ], [ %"loadedCurrSB+Stride218.i", %thenBB212.i ]
  %"&(pSB[currWI].offset)86.i" = add nuw i64 %CurrSBIndex..3.i, 200
  %"&pSB[currWI].offset87.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)86.i"
  %CastToValueType88.i = bitcast i8* %"&pSB[currWI].offset87.i" to double addrspace(3)**
  %loadedValue89.i = load double addrspace(3)** %CastToValueType88.i, align 8
  %281 = load <2 x double>* %31, align 16
  %282 = extractelement <2 x double> %281, i32 1
  store double %282, double addrspace(3)* %loadedValue89.i, align 8
  %"&pSB[currWI].offset146.sum105.i" = add i64 %CurrSBIndex..1.i, 288
  %283 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum105.i"
  %284 = bitcast i8* %283 to <2 x double>*
  %285 = load <2 x double>* %284, align 16
  %286 = extractelement <2 x double> %285, i32 1
  %287 = getelementptr inbounds double addrspace(3)* %loadedValue89.i, i64 66
  store double %286, double addrspace(3)* %287, align 8
  %"&pSB[currWI].offset146.sum106.i" = add i64 %CurrSBIndex..1.i, 256
  %288 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum106.i"
  %289 = bitcast i8* %288 to <2 x double>*
  %290 = load <2 x double>* %289, align 16
  %291 = extractelement <2 x double> %290, i32 1
  %292 = getelementptr inbounds double addrspace(3)* %loadedValue89.i, i64 132
  store double %291, double addrspace(3)* %292, align 8
  %"&pSB[currWI].offset146.sum107.i" = add i64 %CurrSBIndex..1.i, 320
  %293 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum107.i"
  %294 = bitcast i8* %293 to <2 x double>*
  %295 = load <2 x double>* %294, align 16
  %296 = extractelement <2 x double> %295, i32 1
  %297 = getelementptr inbounds double addrspace(3)* %loadedValue89.i, i64 198
  store double %296, double addrspace(3)* %297, align 8
  %"&pSB[currWI].offset146.sum108.i" = add i64 %CurrSBIndex..1.i, 240
  %298 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum108.i"
  %299 = bitcast i8* %298 to <2 x double>*
  %300 = load <2 x double>* %299, align 16
  %301 = extractelement <2 x double> %300, i32 1
  %302 = getelementptr inbounds double addrspace(3)* %loadedValue89.i, i64 264
  store double %301, double addrspace(3)* %302, align 8
  %"&pSB[currWI].offset146.sum109.i" = add i64 %CurrSBIndex..1.i, 304
  %303 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum109.i"
  %304 = bitcast i8* %303 to <2 x double>*
  %305 = load <2 x double>* %304, align 16
  %306 = extractelement <2 x double> %305, i32 1
  %307 = getelementptr inbounds double addrspace(3)* %loadedValue89.i, i64 330
  store double %306, double addrspace(3)* %307, align 8
  %"&pSB[currWI].offset146.sum110.i" = add i64 %CurrSBIndex..1.i, 272
  %308 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum110.i"
  %309 = bitcast i8* %308 to <2 x double>*
  %310 = load <2 x double>* %309, align 16
  %311 = extractelement <2 x double> %310, i32 1
  %312 = getelementptr inbounds double addrspace(3)* %loadedValue89.i, i64 396
  store double %311, double addrspace(3)* %312, align 8
  %"&pSB[currWI].offset146.sum111.i" = add i64 %CurrSBIndex..1.i, 336
  %313 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum111.i"
  %314 = bitcast i8* %313 to <2 x double>*
  %315 = load <2 x double>* %314, align 16
  %316 = extractelement <2 x double> %315, i32 1
  %317 = getelementptr inbounds double addrspace(3)* %loadedValue89.i, i64 462
  store double %316, double addrspace(3)* %317, align 8
  %loadedCurrWI214.i = load i64* %19, align 8
  %check.WI.iter215.i = icmp ult i64 %loadedCurrWI214.i, %13
  br i1 %check.WI.iter215.i, label %thenBB212.i, label %elseBB213.i

thenBB212.i:                                      ; preds = %SyncBB191.i
  %"CurrWI++216.i" = add nuw i64 %loadedCurrWI214.i, 1
  store i64 %"CurrWI++216.i", i64* %19, align 8
  %"loadedCurrSB+Stride218.i" = add nuw i64 %CurrSBIndex..3.i, 608
  br label %SyncBB191.i

elseBB213.i:                                      ; preds = %SyncBB191.i
  store i64 0, i64* %19, align 8
  br label %SyncBB192.i

SyncBB192.i:                                      ; preds = %thenBB219.i, %elseBB213.i
  %CurrSBIndex..4.i = phi i64 [ 0, %elseBB213.i ], [ %"loadedCurrSB+Stride225.i", %thenBB219.i ]
  %"&(pSB[currWI].offset)95.i" = add nuw i64 %CurrSBIndex..4.i, 208
  %"&pSB[currWI].offset96.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)95.i"
  %CastToValueType97.i = bitcast i8* %"&pSB[currWI].offset96.i" to double addrspace(3)**
  %loadedValue98.i = load double addrspace(3)** %CastToValueType97.i, align 8
  %318 = load double addrspace(3)* %loadedValue98.i, align 8
  %319 = load <2 x double>* %31, align 16
  %320 = insertelement <2 x double> %319, double %318, i32 1
  store <2 x double> %320, <2 x double>* %31, align 16
  %"&pSB[currWI].offset146.sum112.i" = add i64 %CurrSBIndex..1.i, 240
  %scevgep.1.i37.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum112.i"
  %321 = bitcast i8* %scevgep.1.i37.i to <2 x double>*
  %scevgep2.1.i38.i = getelementptr double addrspace(3)* %loadedValue98.i, i64 8
  %322 = load double addrspace(3)* %scevgep2.1.i38.i, align 8
  %323 = load <2 x double>* %321, align 16
  %324 = insertelement <2 x double> %323, double %322, i32 1
  store <2 x double> %324, <2 x double>* %321, align 16
  %"&pSB[currWI].offset146.sum113.i" = add i64 %CurrSBIndex..1.i, 256
  %scevgep.2.i39.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum113.i"
  %325 = bitcast i8* %scevgep.2.i39.i to <2 x double>*
  %scevgep2.2.i40.i = getelementptr double addrspace(3)* %loadedValue98.i, i64 16
  %326 = load double addrspace(3)* %scevgep2.2.i40.i, align 8
  %327 = load <2 x double>* %325, align 16
  %328 = insertelement <2 x double> %327, double %326, i32 1
  store <2 x double> %328, <2 x double>* %325, align 16
  %"&pSB[currWI].offset146.sum114.i" = add i64 %CurrSBIndex..1.i, 272
  %scevgep.3.i41.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum114.i"
  %329 = bitcast i8* %scevgep.3.i41.i to <2 x double>*
  %scevgep2.3.i42.i = getelementptr double addrspace(3)* %loadedValue98.i, i64 24
  %330 = load double addrspace(3)* %scevgep2.3.i42.i, align 8
  %331 = load <2 x double>* %329, align 16
  %332 = insertelement <2 x double> %331, double %330, i32 1
  store <2 x double> %332, <2 x double>* %329, align 16
  %"&pSB[currWI].offset146.sum115.i" = add i64 %CurrSBIndex..1.i, 288
  %scevgep.4.i43.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum115.i"
  %333 = bitcast i8* %scevgep.4.i43.i to <2 x double>*
  %scevgep2.4.i44.i = getelementptr double addrspace(3)* %loadedValue98.i, i64 32
  %334 = load double addrspace(3)* %scevgep2.4.i44.i, align 8
  %335 = load <2 x double>* %333, align 16
  %336 = insertelement <2 x double> %335, double %334, i32 1
  store <2 x double> %336, <2 x double>* %333, align 16
  %"&pSB[currWI].offset146.sum116.i" = add i64 %CurrSBIndex..1.i, 304
  %scevgep.5.i45.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum116.i"
  %337 = bitcast i8* %scevgep.5.i45.i to <2 x double>*
  %scevgep2.5.i46.i = getelementptr double addrspace(3)* %loadedValue98.i, i64 40
  %338 = load double addrspace(3)* %scevgep2.5.i46.i, align 8
  %339 = load <2 x double>* %337, align 16
  %340 = insertelement <2 x double> %339, double %338, i32 1
  store <2 x double> %340, <2 x double>* %337, align 16
  %"&pSB[currWI].offset146.sum117.i" = add i64 %CurrSBIndex..1.i, 320
  %scevgep.6.i47.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum117.i"
  %341 = bitcast i8* %scevgep.6.i47.i to <2 x double>*
  %scevgep2.6.i48.i = getelementptr double addrspace(3)* %loadedValue98.i, i64 48
  %342 = load double addrspace(3)* %scevgep2.6.i48.i, align 8
  %343 = load <2 x double>* %341, align 16
  %344 = insertelement <2 x double> %343, double %342, i32 1
  store <2 x double> %344, <2 x double>* %341, align 16
  %"&pSB[currWI].offset146.sum118.i" = add i64 %CurrSBIndex..1.i, 336
  %scevgep.7.i49.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum118.i"
  %345 = bitcast i8* %scevgep.7.i49.i to <2 x double>*
  %scevgep2.7.i50.i = getelementptr double addrspace(3)* %loadedValue98.i, i64 56
  %346 = load double addrspace(3)* %scevgep2.7.i50.i, align 8
  %347 = load <2 x double>* %345, align 16
  %348 = insertelement <2 x double> %347, double %346, i32 1
  store <2 x double> %348, <2 x double>* %345, align 16
  %loadedCurrWI221.i = load i64* %19, align 8
  %check.WI.iter222.i = icmp ult i64 %loadedCurrWI221.i, %13
  br i1 %check.WI.iter222.i, label %thenBB219.i, label %elseBB220.i

thenBB219.i:                                      ; preds = %SyncBB192.i
  %"CurrWI++223.i" = add nuw i64 %loadedCurrWI221.i, 1
  store i64 %"CurrWI++223.i", i64* %19, align 8
  %"loadedCurrSB+Stride225.i" = add nuw i64 %CurrSBIndex..4.i, 608
  br label %SyncBB192.i

elseBB220.i:                                      ; preds = %SyncBB192.i
  store i64 0, i64* %19, align 8
  br label %SyncBB193.i

SyncBB193.i:                                      ; preds = %thenBB226.i, %elseBB220.i
  %CurrSBIndex..5.i = phi i64 [ 0, %elseBB220.i ], [ %"loadedCurrSB+Stride232.i", %thenBB226.i ]
  %349 = load <2 x double>* %31, align 16
  %350 = load <2 x double>* %49, align 16
  %351 = extractelement <2 x double> %349, i32 0
  %352 = extractelement <2 x double> %350, i32 0
  %353 = fadd double %351, %352
  %354 = extractelement <2 x double> %349, i32 1
  %355 = extractelement <2 x double> %350, i32 1
  %356 = fadd double %354, %355
  %357 = extractelement <2 x double> %349, i32 0
  %358 = extractelement <2 x double> %350, i32 0
  %359 = fsub double %357, %358
  %360 = extractelement <2 x double> %349, i32 1
  %361 = extractelement <2 x double> %350, i32 1
  %362 = fsub double %360, %361
  %363 = load <2 x double>* %64, align 16
  %364 = load <2 x double>* %67, align 16
  %365 = extractelement <2 x double> %363, i32 0
  %366 = extractelement <2 x double> %364, i32 0
  %367 = fadd double %365, %366
  %368 = extractelement <2 x double> %363, i32 1
  %369 = extractelement <2 x double> %364, i32 1
  %370 = fadd double %368, %369
  %371 = extractelement <2 x double> %363, i32 0
  %372 = extractelement <2 x double> %364, i32 0
  %373 = fsub double %371, %372
  %374 = extractelement <2 x double> %363, i32 1
  %375 = extractelement <2 x double> %364, i32 1
  %376 = fsub double %374, %375
  %377 = load <2 x double>* %82, align 16
  %378 = load <2 x double>* %85, align 16
  %379 = extractelement <2 x double> %377, i32 0
  %380 = extractelement <2 x double> %378, i32 0
  %381 = fadd double %379, %380
  %382 = extractelement <2 x double> %377, i32 1
  %383 = extractelement <2 x double> %378, i32 1
  %384 = fadd double %382, %383
  %385 = extractelement <2 x double> %377, i32 0
  %386 = extractelement <2 x double> %378, i32 0
  %387 = fsub double %385, %386
  %388 = extractelement <2 x double> %377, i32 1
  %389 = extractelement <2 x double> %378, i32 1
  %390 = fsub double %388, %389
  %391 = load <2 x double>* %100, align 16
  %392 = load <2 x double>* %103, align 16
  %393 = extractelement <2 x double> %391, i32 0
  %394 = extractelement <2 x double> %392, i32 0
  %395 = fadd double %393, %394
  %396 = extractelement <2 x double> %391, i32 1
  %397 = extractelement <2 x double> %392, i32 1
  %398 = fadd double %396, %397
  %399 = extractelement <2 x double> %391, i32 0
  %400 = extractelement <2 x double> %392, i32 0
  %401 = fsub double %399, %400
  %402 = extractelement <2 x double> %391, i32 1
  %403 = extractelement <2 x double> %392, i32 1
  %404 = fsub double %402, %403
  %405 = fsub double %373, %376
  %406 = fadd double %373, %376
  %407 = fmul double %405, 0x3FE6A09E667F3BCD
  %408 = fmul double %406, 0x3FE6A09E667F3BCD
  %409 = fmul double %387, 0.000000e+00
  %410 = fsub double %409, %390
  %411 = fmul double %390, 0.000000e+00
  %412 = fadd double %387, %411
  %413 = fmul double %401, -1.000000e+00
  %414 = fsub double %413, %404
  %415 = fmul double %404, -1.000000e+00
  %416 = fadd double %401, %415
  %417 = fmul double %414, 0x3FE6A09E667F3BCD
  %418 = fmul double %416, 0x3FE6A09E667F3BCD
  %419 = fadd double %353, %381
  %420 = fadd double %356, %384
  %421 = fsub double %353, %381
  %422 = fsub double %356, %384
  %423 = fadd double %367, %395
  %424 = fadd double %370, %398
  %425 = fsub double %367, %395
  %426 = fsub double %370, %398
  %427 = fmul double %425, 0.000000e+00
  %428 = fsub double %427, %426
  %429 = fmul double %426, 0.000000e+00
  %430 = fadd double %425, %429
  %431 = fadd double %419, %423
  %432 = insertelement <2 x double> undef, double %431, i32 0
  %433 = fadd double %420, %424
  %434 = insertelement <2 x double> %432, double %433, i32 1
  store <2 x double> %434, <2 x double>* %31, align 16
  %435 = fsub double %419, %423
  %436 = insertelement <2 x double> undef, double %435, i32 0
  %437 = fsub double %420, %424
  %438 = insertelement <2 x double> %436, double %437, i32 1
  store <2 x double> %438, <2 x double>* %64, align 16
  %439 = fadd double %421, %428
  %440 = insertelement <2 x double> undef, double %439, i32 0
  %441 = fadd double %422, %430
  %442 = insertelement <2 x double> %440, double %441, i32 1
  store <2 x double> %442, <2 x double>* %82, align 16
  %443 = fsub double %421, %428
  %444 = insertelement <2 x double> undef, double %443, i32 0
  %445 = fsub double %422, %430
  %446 = insertelement <2 x double> %444, double %445, i32 1
  store <2 x double> %446, <2 x double>* %100, align 16
  %447 = fadd double %359, %410
  %448 = fadd double %362, %412
  %449 = fsub double %359, %410
  %450 = fsub double %362, %412
  %451 = fadd double %407, %417
  %452 = fadd double %408, %418
  %453 = fsub double %407, %417
  %454 = fsub double %408, %418
  %455 = fmul double %453, 0.000000e+00
  %456 = fsub double %455, %454
  %457 = fmul double %454, 0.000000e+00
  %458 = fadd double %453, %457
  %459 = fadd double %447, %451
  %460 = insertelement <2 x double> undef, double %459, i32 0
  %461 = fadd double %448, %452
  %462 = insertelement <2 x double> %460, double %461, i32 1
  store <2 x double> %462, <2 x double>* %49, align 16
  %463 = fsub double %447, %451
  %464 = insertelement <2 x double> undef, double %463, i32 0
  %465 = fsub double %448, %452
  %466 = insertelement <2 x double> %464, double %465, i32 1
  store <2 x double> %466, <2 x double>* %67, align 16
  %467 = fadd double %449, %456
  %468 = insertelement <2 x double> undef, double %467, i32 0
  %469 = fadd double %450, %458
  %470 = insertelement <2 x double> %468, double %469, i32 1
  store <2 x double> %470, <2 x double>* %85, align 16
  %471 = fsub double %449, %456
  %472 = insertelement <2 x double> undef, double %471, i32 0
  %473 = fsub double %450, %458
  %474 = insertelement <2 x double> %472, double %473, i32 1
  store <2 x double> %474, <2 x double>* %103, align 16
  %"&(pSB[currWI].offset)44.i" = add nuw i64 %CurrSBIndex..5.i, 180
  %"&pSB[currWI].offset45.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)44.i"
  %CastToValueType46.i = bitcast i8* %"&pSB[currWI].offset45.i" to i32*
  %loadedValue47.i = load i32* %CastToValueType46.i, align 4
  %475 = sitofp i32 %loadedValue47.i to double
  %"&(pSB[currWI].offset)113.i" = add nuw i64 %CurrSBIndex..5.i, 224
  %"&pSB[currWI].offset114.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)113.i"
  %CastToValueType115.i = bitcast i8* %"&pSB[currWI].offset114.i" to [8 x <2 x double>]*
  br label %476

; <label>:476                                     ; preds = %._crit_edge.i, %SyncBB193.i
  %indvar.i = phi i64 [ 1, %SyncBB193.i ], [ %phitmp17.i, %._crit_edge.i ]
  %scevgep8.i = getelementptr [8 x <2 x double>]* %CastToValueType115.i, i64 0, i64 %indvar.i
  %scevgep9.i = getelementptr [8 x i32]* @fft1D_512.reversed8, i64 0, i64 %indvar.i
  %477 = load <2 x double>* %scevgep8.i, align 16
  %478 = load i32* %scevgep9.i, align 4
  %479 = sitofp i32 %478 to double
  %480 = fmul double %479, 0x401921FB54442D18
  %481 = fdiv double %480, 6.400000e+01
  %482 = fmul double %481, %475
  %483 = call double @_Z3cosd(double %482) nounwind
  %484 = call double @_Z3sind(double %482) nounwind
  %485 = extractelement <2 x double> %477, i32 0
  %486 = fmul double %485, %483
  %487 = extractelement <2 x double> %477, i32 1
  %488 = fmul double %487, %484
  %489 = fsub double %486, %488
  %490 = insertelement <2 x double> undef, double %489, i32 0
  %491 = fmul double %485, %484
  %492 = fmul double %487, %483
  %493 = fadd double %491, %492
  %494 = insertelement <2 x double> %490, double %493, i32 1
  store <2 x double> %494, <2 x double>* %scevgep8.i, align 16
  %exitcond.i = icmp eq i64 %indvar.i, 7
  br i1 %exitcond.i, label %"Barrier BB22.i", label %._crit_edge.i

._crit_edge.i:                                    ; preds = %476
  %phitmp17.i = add i64 %indvar.i, 1
  br label %476

"Barrier BB22.i":                                 ; preds = %476
  %"&(pSB[currWI].offset)81.i" = add nuw i64 %CurrSBIndex..5.i, 200
  %"&pSB[currWI].offset82.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)81.i"
  %CastToValueType83.i = bitcast i8* %"&pSB[currWI].offset82.i" to double addrspace(3)**
  %loadedValue84.i = load double addrspace(3)** %CastToValueType83.i, align 8
  %495 = load <2 x double>* %31, align 16
  %496 = extractelement <2 x double> %495, i32 0
  store double %496, double addrspace(3)* %loadedValue84.i, align 8
  %"&pSB[currWI].offset146.sum119.i" = add i64 %CurrSBIndex..1.i, 288
  %497 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum119.i"
  %498 = bitcast i8* %497 to <2 x double>*
  %499 = load <2 x double>* %498, align 16
  %500 = extractelement <2 x double> %499, i32 0
  %501 = getelementptr inbounds double addrspace(3)* %loadedValue84.i, i64 72
  store double %500, double addrspace(3)* %501, align 8
  %"&pSB[currWI].offset146.sum120.i" = add i64 %CurrSBIndex..1.i, 256
  %502 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum120.i"
  %503 = bitcast i8* %502 to <2 x double>*
  %504 = load <2 x double>* %503, align 16
  %505 = extractelement <2 x double> %504, i32 0
  %506 = getelementptr inbounds double addrspace(3)* %loadedValue84.i, i64 144
  store double %505, double addrspace(3)* %506, align 8
  %"&pSB[currWI].offset146.sum121.i" = add i64 %CurrSBIndex..1.i, 320
  %507 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum121.i"
  %508 = bitcast i8* %507 to <2 x double>*
  %509 = load <2 x double>* %508, align 16
  %510 = extractelement <2 x double> %509, i32 0
  %511 = getelementptr inbounds double addrspace(3)* %loadedValue84.i, i64 216
  store double %510, double addrspace(3)* %511, align 8
  %"&pSB[currWI].offset146.sum122.i" = add i64 %CurrSBIndex..1.i, 240
  %512 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum122.i"
  %513 = bitcast i8* %512 to <2 x double>*
  %514 = load <2 x double>* %513, align 16
  %515 = extractelement <2 x double> %514, i32 0
  %516 = getelementptr inbounds double addrspace(3)* %loadedValue84.i, i64 288
  store double %515, double addrspace(3)* %516, align 8
  %"&pSB[currWI].offset146.sum123.i" = add i64 %CurrSBIndex..1.i, 304
  %517 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum123.i"
  %518 = bitcast i8* %517 to <2 x double>*
  %519 = load <2 x double>* %518, align 16
  %520 = extractelement <2 x double> %519, i32 0
  %521 = getelementptr inbounds double addrspace(3)* %loadedValue84.i, i64 360
  store double %520, double addrspace(3)* %521, align 8
  %"&pSB[currWI].offset146.sum124.i" = add i64 %CurrSBIndex..1.i, 272
  %522 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum124.i"
  %523 = bitcast i8* %522 to <2 x double>*
  %524 = load <2 x double>* %523, align 16
  %525 = extractelement <2 x double> %524, i32 0
  %526 = getelementptr inbounds double addrspace(3)* %loadedValue84.i, i64 432
  store double %525, double addrspace(3)* %526, align 8
  %"&pSB[currWI].offset146.sum125.i" = add i64 %CurrSBIndex..1.i, 336
  %527 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum125.i"
  %528 = bitcast i8* %527 to <2 x double>*
  %529 = load <2 x double>* %528, align 16
  %530 = extractelement <2 x double> %529, i32 0
  %531 = getelementptr inbounds double addrspace(3)* %loadedValue84.i, i64 504
  store double %530, double addrspace(3)* %531, align 8
  %loadedCurrWI228.i = load i64* %19, align 8
  %check.WI.iter229.i = icmp ult i64 %loadedCurrWI228.i, %13
  br i1 %check.WI.iter229.i, label %thenBB226.i, label %elseBB227.i

thenBB226.i:                                      ; preds = %"Barrier BB22.i"
  %"CurrWI++230.i" = add nuw i64 %loadedCurrWI228.i, 1
  store i64 %"CurrWI++230.i", i64* %19, align 8
  %"loadedCurrSB+Stride232.i" = add nuw i64 %CurrSBIndex..5.i, 608
  br label %SyncBB193.i

elseBB227.i:                                      ; preds = %"Barrier BB22.i"
  store i64 0, i64* %19, align 8
  br label %SyncBB194.i

SyncBB194.i:                                      ; preds = %thenBB233.i, %elseBB227.i
  %CurrSBIndex..6.i = phi i64 [ 0, %elseBB227.i ], [ %"loadedCurrSB+Stride239.i", %thenBB233.i ]
  %"&(pSB[currWI].offset)26.i" = add nuw i64 %CurrSBIndex..6.i, 176
  %"&pSB[currWI].offset27.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)26.i"
  %CastToValueType28.i = bitcast i8* %"&pSB[currWI].offset27.i" to i32*
  %loadedValue.i = load i32* %CastToValueType28.i, align 4
  %532 = and i32 %loadedValue.i, -8
  %533 = mul nsw i32 %532, 9
  %"&(pSB[currWI].offset)53.i" = add nuw i64 %CurrSBIndex..6.i, 184
  %"&pSB[currWI].offset54.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)53.i"
  %CastToValueType55.i = bitcast i8* %"&pSB[currWI].offset54.i" to i32*
  %loadedValue56.i = load i32* %CastToValueType55.i, align 4
  %534 = or i32 %533, %loadedValue56.i
  %535 = sext i32 %534 to i64
  %536 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %535
  %"&(pSB[currWI].offset)100.i" = add nuw i64 %CurrSBIndex..6.i, 216
  %"&pSB[currWI].offset101.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)100.i"
  %CastToValueType102.i = bitcast i8* %"&pSB[currWI].offset101.i" to double addrspace(3)**
  store double addrspace(3)* %536, double addrspace(3)** %CastToValueType102.i, align 8
  %537 = load double addrspace(3)* %536, align 8
  %538 = load <2 x double>* %31, align 16
  %539 = insertelement <2 x double> %538, double %537, i32 0
  store <2 x double> %539, <2 x double>* %31, align 16
  %"&pSB[currWI].offset146.sum126.i" = add i64 %CurrSBIndex..1.i, 240
  %scevgep.1.i23.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum126.i"
  %540 = bitcast i8* %scevgep.1.i23.i to <2 x double>*
  %.sum127.i = add i64 %535, 8
  %scevgep2.1.i24.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum127.i
  %541 = load double addrspace(3)* %scevgep2.1.i24.i, align 8
  %542 = load <2 x double>* %540, align 16
  %543 = insertelement <2 x double> %542, double %541, i32 0
  store <2 x double> %543, <2 x double>* %540, align 16
  %"&pSB[currWI].offset146.sum128.i" = add i64 %CurrSBIndex..1.i, 256
  %scevgep.2.i25.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum128.i"
  %544 = bitcast i8* %scevgep.2.i25.i to <2 x double>*
  %.sum129.i = add i64 %535, 16
  %scevgep2.2.i26.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum129.i
  %545 = load double addrspace(3)* %scevgep2.2.i26.i, align 8
  %546 = load <2 x double>* %544, align 16
  %547 = insertelement <2 x double> %546, double %545, i32 0
  store <2 x double> %547, <2 x double>* %544, align 16
  %"&pSB[currWI].offset146.sum130.i" = add i64 %CurrSBIndex..1.i, 272
  %scevgep.3.i27.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum130.i"
  %548 = bitcast i8* %scevgep.3.i27.i to <2 x double>*
  %.sum131.i = add i64 %535, 24
  %scevgep2.3.i28.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum131.i
  %549 = load double addrspace(3)* %scevgep2.3.i28.i, align 8
  %550 = load <2 x double>* %548, align 16
  %551 = insertelement <2 x double> %550, double %549, i32 0
  store <2 x double> %551, <2 x double>* %548, align 16
  %"&pSB[currWI].offset146.sum132.i" = add i64 %CurrSBIndex..1.i, 288
  %scevgep.4.i29.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum132.i"
  %552 = bitcast i8* %scevgep.4.i29.i to <2 x double>*
  %.sum133.i = add i64 %535, 32
  %scevgep2.4.i30.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum133.i
  %553 = load double addrspace(3)* %scevgep2.4.i30.i, align 8
  %554 = load <2 x double>* %552, align 16
  %555 = insertelement <2 x double> %554, double %553, i32 0
  store <2 x double> %555, <2 x double>* %552, align 16
  %"&pSB[currWI].offset146.sum134.i" = add i64 %CurrSBIndex..1.i, 304
  %scevgep.5.i31.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum134.i"
  %556 = bitcast i8* %scevgep.5.i31.i to <2 x double>*
  %.sum135.i = add i64 %535, 40
  %scevgep2.5.i32.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum135.i
  %557 = load double addrspace(3)* %scevgep2.5.i32.i, align 8
  %558 = load <2 x double>* %556, align 16
  %559 = insertelement <2 x double> %558, double %557, i32 0
  store <2 x double> %559, <2 x double>* %556, align 16
  %"&pSB[currWI].offset146.sum136.i" = add i64 %CurrSBIndex..1.i, 320
  %scevgep.6.i33.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum136.i"
  %560 = bitcast i8* %scevgep.6.i33.i to <2 x double>*
  %.sum137.i = add i64 %535, 48
  %scevgep2.6.i34.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum137.i
  %561 = load double addrspace(3)* %scevgep2.6.i34.i, align 8
  %562 = load <2 x double>* %560, align 16
  %563 = insertelement <2 x double> %562, double %561, i32 0
  store <2 x double> %563, <2 x double>* %560, align 16
  %"&pSB[currWI].offset146.sum138.i" = add i64 %CurrSBIndex..1.i, 336
  %scevgep.7.i35.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum138.i"
  %564 = bitcast i8* %scevgep.7.i35.i to <2 x double>*
  %.sum139.i = add i64 %535, 56
  %scevgep2.7.i36.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum139.i
  %565 = load double addrspace(3)* %scevgep2.7.i36.i, align 8
  %566 = load <2 x double>* %564, align 16
  %567 = insertelement <2 x double> %566, double %565, i32 0
  store <2 x double> %567, <2 x double>* %564, align 16
  %loadedCurrWI235.i = load i64* %19, align 8
  %check.WI.iter236.i = icmp ult i64 %loadedCurrWI235.i, %13
  br i1 %check.WI.iter236.i, label %thenBB233.i, label %elseBB234.i

thenBB233.i:                                      ; preds = %SyncBB194.i
  %"CurrWI++237.i" = add nuw i64 %loadedCurrWI235.i, 1
  store i64 %"CurrWI++237.i", i64* %19, align 8
  %"loadedCurrSB+Stride239.i" = add nuw i64 %CurrSBIndex..6.i, 608
  br label %SyncBB194.i

elseBB234.i:                                      ; preds = %SyncBB194.i
  store i64 0, i64* %19, align 8
  br label %SyncBB195.i

SyncBB195.i:                                      ; preds = %thenBB240.i, %elseBB234.i
  %CurrSBIndex..7.i = phi i64 [ 0, %elseBB234.i ], [ %"loadedCurrSB+Stride246.i", %thenBB240.i ]
  %"&(pSB[currWI].offset)76.i" = add nuw i64 %CurrSBIndex..7.i, 200
  %"&pSB[currWI].offset77.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)76.i"
  %CastToValueType78.i = bitcast i8* %"&pSB[currWI].offset77.i" to double addrspace(3)**
  %loadedValue79.i = load double addrspace(3)** %CastToValueType78.i, align 8
  %568 = load <2 x double>* %31, align 16
  %569 = extractelement <2 x double> %568, i32 1
  store double %569, double addrspace(3)* %loadedValue79.i, align 8
  %"&pSB[currWI].offset146.sum140.i" = add i64 %CurrSBIndex..1.i, 288
  %570 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum140.i"
  %571 = bitcast i8* %570 to <2 x double>*
  %572 = load <2 x double>* %571, align 16
  %573 = extractelement <2 x double> %572, i32 1
  %574 = getelementptr inbounds double addrspace(3)* %loadedValue79.i, i64 72
  store double %573, double addrspace(3)* %574, align 8
  %"&pSB[currWI].offset146.sum141.i" = add i64 %CurrSBIndex..1.i, 256
  %575 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum141.i"
  %576 = bitcast i8* %575 to <2 x double>*
  %577 = load <2 x double>* %576, align 16
  %578 = extractelement <2 x double> %577, i32 1
  %579 = getelementptr inbounds double addrspace(3)* %loadedValue79.i, i64 144
  store double %578, double addrspace(3)* %579, align 8
  %"&pSB[currWI].offset146.sum142.i" = add i64 %CurrSBIndex..1.i, 320
  %580 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum142.i"
  %581 = bitcast i8* %580 to <2 x double>*
  %582 = load <2 x double>* %581, align 16
  %583 = extractelement <2 x double> %582, i32 1
  %584 = getelementptr inbounds double addrspace(3)* %loadedValue79.i, i64 216
  store double %583, double addrspace(3)* %584, align 8
  %"&pSB[currWI].offset146.sum143.i" = add i64 %CurrSBIndex..1.i, 240
  %585 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum143.i"
  %586 = bitcast i8* %585 to <2 x double>*
  %587 = load <2 x double>* %586, align 16
  %588 = extractelement <2 x double> %587, i32 1
  %589 = getelementptr inbounds double addrspace(3)* %loadedValue79.i, i64 288
  store double %588, double addrspace(3)* %589, align 8
  %"&pSB[currWI].offset146.sum144.i" = add i64 %CurrSBIndex..1.i, 304
  %590 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum144.i"
  %591 = bitcast i8* %590 to <2 x double>*
  %592 = load <2 x double>* %591, align 16
  %593 = extractelement <2 x double> %592, i32 1
  %594 = getelementptr inbounds double addrspace(3)* %loadedValue79.i, i64 360
  store double %593, double addrspace(3)* %594, align 8
  %"&pSB[currWI].offset146.sum145.i" = add i64 %CurrSBIndex..1.i, 272
  %595 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum145.i"
  %596 = bitcast i8* %595 to <2 x double>*
  %597 = load <2 x double>* %596, align 16
  %598 = extractelement <2 x double> %597, i32 1
  %599 = getelementptr inbounds double addrspace(3)* %loadedValue79.i, i64 432
  store double %598, double addrspace(3)* %599, align 8
  %"&pSB[currWI].offset146.sum146.i" = add i64 %CurrSBIndex..1.i, 336
  %600 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum146.i"
  %601 = bitcast i8* %600 to <2 x double>*
  %602 = load <2 x double>* %601, align 16
  %603 = extractelement <2 x double> %602, i32 1
  %604 = getelementptr inbounds double addrspace(3)* %loadedValue79.i, i64 504
  store double %603, double addrspace(3)* %604, align 8
  %loadedCurrWI242.i = load i64* %19, align 8
  %check.WI.iter243.i = icmp ult i64 %loadedCurrWI242.i, %13
  br i1 %check.WI.iter243.i, label %thenBB240.i, label %elseBB241.i

thenBB240.i:                                      ; preds = %SyncBB195.i
  %"CurrWI++244.i" = add nuw i64 %loadedCurrWI242.i, 1
  store i64 %"CurrWI++244.i", i64* %19, align 8
  %"loadedCurrSB+Stride246.i" = add nuw i64 %CurrSBIndex..7.i, 608
  br label %SyncBB195.i

elseBB241.i:                                      ; preds = %SyncBB195.i
  store i64 0, i64* %19, align 8
  br label %SyncBB196.i

SyncBB196.i:                                      ; preds = %thenBB.i, %elseBB241.i
  %CurrSBIndex..0.i = phi i64 [ 0, %elseBB241.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %"&(pSB[currWI].offset)104.i" = add nuw i64 %CurrSBIndex..0.i, 216
  %"&pSB[currWI].offset105.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)104.i"
  %CastToValueType106.i = bitcast i8* %"&pSB[currWI].offset105.i" to double addrspace(3)**
  %loadedValue107.i = load double addrspace(3)** %CastToValueType106.i, align 8
  %605 = load double addrspace(3)* %loadedValue107.i, align 8
  %606 = load <2 x double>* %31, align 16
  %607 = insertelement <2 x double> %606, double %605, i32 1
  store <2 x double> %607, <2 x double>* %31, align 16
  %"&pSB[currWI].offset146.sum147.i" = add i64 %CurrSBIndex..1.i, 240
  %scevgep.1.i9.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum147.i"
  %608 = bitcast i8* %scevgep.1.i9.i to <2 x double>*
  %scevgep2.1.i10.i = getelementptr double addrspace(3)* %loadedValue107.i, i64 8
  %609 = load double addrspace(3)* %scevgep2.1.i10.i, align 8
  %610 = load <2 x double>* %608, align 16
  %611 = insertelement <2 x double> %610, double %609, i32 1
  store <2 x double> %611, <2 x double>* %608, align 16
  %"&pSB[currWI].offset146.sum148.i" = add i64 %CurrSBIndex..1.i, 256
  %scevgep.2.i11.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum148.i"
  %612 = bitcast i8* %scevgep.2.i11.i to <2 x double>*
  %scevgep2.2.i12.i = getelementptr double addrspace(3)* %loadedValue107.i, i64 16
  %613 = load double addrspace(3)* %scevgep2.2.i12.i, align 8
  %614 = load <2 x double>* %612, align 16
  %615 = insertelement <2 x double> %614, double %613, i32 1
  store <2 x double> %615, <2 x double>* %612, align 16
  %"&pSB[currWI].offset146.sum149.i" = add i64 %CurrSBIndex..1.i, 272
  %scevgep.3.i13.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum149.i"
  %616 = bitcast i8* %scevgep.3.i13.i to <2 x double>*
  %scevgep2.3.i14.i = getelementptr double addrspace(3)* %loadedValue107.i, i64 24
  %617 = load double addrspace(3)* %scevgep2.3.i14.i, align 8
  %618 = load <2 x double>* %616, align 16
  %619 = insertelement <2 x double> %618, double %617, i32 1
  store <2 x double> %619, <2 x double>* %616, align 16
  %"&pSB[currWI].offset146.sum150.i" = add i64 %CurrSBIndex..1.i, 288
  %scevgep.4.i15.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum150.i"
  %620 = bitcast i8* %scevgep.4.i15.i to <2 x double>*
  %scevgep2.4.i16.i = getelementptr double addrspace(3)* %loadedValue107.i, i64 32
  %621 = load double addrspace(3)* %scevgep2.4.i16.i, align 8
  %622 = load <2 x double>* %620, align 16
  %623 = insertelement <2 x double> %622, double %621, i32 1
  store <2 x double> %623, <2 x double>* %620, align 16
  %"&pSB[currWI].offset146.sum151.i" = add i64 %CurrSBIndex..1.i, 304
  %scevgep.5.i17.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum151.i"
  %624 = bitcast i8* %scevgep.5.i17.i to <2 x double>*
  %scevgep2.5.i18.i = getelementptr double addrspace(3)* %loadedValue107.i, i64 40
  %625 = load double addrspace(3)* %scevgep2.5.i18.i, align 8
  %626 = load <2 x double>* %624, align 16
  %627 = insertelement <2 x double> %626, double %625, i32 1
  store <2 x double> %627, <2 x double>* %624, align 16
  %"&pSB[currWI].offset146.sum152.i" = add i64 %CurrSBIndex..1.i, 320
  %scevgep.6.i19.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum152.i"
  %628 = bitcast i8* %scevgep.6.i19.i to <2 x double>*
  %scevgep2.6.i20.i = getelementptr double addrspace(3)* %loadedValue107.i, i64 48
  %629 = load double addrspace(3)* %scevgep2.6.i20.i, align 8
  %630 = load <2 x double>* %628, align 16
  %631 = insertelement <2 x double> %630, double %629, i32 1
  store <2 x double> %631, <2 x double>* %628, align 16
  %"&pSB[currWI].offset146.sum153.i" = add i64 %CurrSBIndex..1.i, 336
  %scevgep.7.i21.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset146.sum153.i"
  %632 = bitcast i8* %scevgep.7.i21.i to <2 x double>*
  %scevgep2.7.i22.i = getelementptr double addrspace(3)* %loadedValue107.i, i64 56
  %633 = load double addrspace(3)* %scevgep2.7.i22.i, align 8
  %634 = load <2 x double>* %632, align 16
  %635 = insertelement <2 x double> %634, double %633, i32 1
  store <2 x double> %635, <2 x double>* %632, align 16
  %636 = load <2 x double>* %31, align 16
  %637 = load <2 x double>* %49, align 16
  %638 = extractelement <2 x double> %636, i32 0
  %639 = extractelement <2 x double> %637, i32 0
  %640 = fadd double %638, %639
  %641 = extractelement <2 x double> %636, i32 1
  %642 = extractelement <2 x double> %637, i32 1
  %643 = fadd double %641, %642
  %644 = extractelement <2 x double> %636, i32 0
  %645 = extractelement <2 x double> %637, i32 0
  %646 = fsub double %644, %645
  %647 = extractelement <2 x double> %636, i32 1
  %648 = extractelement <2 x double> %637, i32 1
  %649 = fsub double %647, %648
  %650 = load <2 x double>* %64, align 16
  %651 = load <2 x double>* %67, align 16
  %652 = extractelement <2 x double> %650, i32 0
  %653 = extractelement <2 x double> %651, i32 0
  %654 = fadd double %652, %653
  %655 = extractelement <2 x double> %650, i32 1
  %656 = extractelement <2 x double> %651, i32 1
  %657 = fadd double %655, %656
  %658 = extractelement <2 x double> %650, i32 0
  %659 = extractelement <2 x double> %651, i32 0
  %660 = fsub double %658, %659
  %661 = extractelement <2 x double> %650, i32 1
  %662 = extractelement <2 x double> %651, i32 1
  %663 = fsub double %661, %662
  %664 = load <2 x double>* %82, align 16
  %665 = load <2 x double>* %85, align 16
  %666 = extractelement <2 x double> %664, i32 0
  %667 = extractelement <2 x double> %665, i32 0
  %668 = fadd double %666, %667
  %669 = extractelement <2 x double> %664, i32 1
  %670 = extractelement <2 x double> %665, i32 1
  %671 = fadd double %669, %670
  %672 = extractelement <2 x double> %664, i32 0
  %673 = extractelement <2 x double> %665, i32 0
  %674 = fsub double %672, %673
  %675 = extractelement <2 x double> %664, i32 1
  %676 = extractelement <2 x double> %665, i32 1
  %677 = fsub double %675, %676
  %678 = load <2 x double>* %100, align 16
  %679 = load <2 x double>* %103, align 16
  %680 = extractelement <2 x double> %678, i32 0
  %681 = extractelement <2 x double> %679, i32 0
  %682 = fadd double %680, %681
  %683 = extractelement <2 x double> %678, i32 1
  %684 = extractelement <2 x double> %679, i32 1
  %685 = fadd double %683, %684
  %686 = extractelement <2 x double> %678, i32 0
  %687 = extractelement <2 x double> %679, i32 0
  %688 = fsub double %686, %687
  %689 = extractelement <2 x double> %678, i32 1
  %690 = extractelement <2 x double> %679, i32 1
  %691 = fsub double %689, %690
  %692 = fsub double %660, %663
  %693 = fadd double %660, %663
  %694 = fmul double %692, 0x3FE6A09E667F3BCD
  %695 = fmul double %693, 0x3FE6A09E667F3BCD
  %696 = fmul double %674, 0.000000e+00
  %697 = fsub double %696, %677
  %698 = fmul double %677, 0.000000e+00
  %699 = fadd double %674, %698
  %700 = fmul double %688, -1.000000e+00
  %701 = fsub double %700, %691
  %702 = fmul double %691, -1.000000e+00
  %703 = fadd double %688, %702
  %704 = fmul double %701, 0x3FE6A09E667F3BCD
  %705 = fmul double %703, 0x3FE6A09E667F3BCD
  %706 = fadd double %640, %668
  %707 = fadd double %643, %671
  %708 = fsub double %640, %668
  %709 = fsub double %643, %671
  %710 = fadd double %654, %682
  %711 = fadd double %657, %685
  %712 = fsub double %654, %682
  %713 = fsub double %657, %685
  %714 = fmul double %712, 0.000000e+00
  %715 = fsub double %714, %713
  %716 = fmul double %713, 0.000000e+00
  %717 = fadd double %712, %716
  %718 = fadd double %706, %710
  %719 = fadd double %707, %711
  %720 = fsub double %706, %710
  %721 = fsub double %707, %711
  %722 = fadd double %708, %715
  %723 = fadd double %709, %717
  %724 = fsub double %708, %715
  %725 = fsub double %709, %717
  %726 = fadd double %646, %697
  %727 = fadd double %649, %699
  %728 = fsub double %646, %697
  %729 = fsub double %649, %699
  %730 = fadd double %694, %704
  %731 = fadd double %695, %705
  %732 = fsub double %694, %704
  %733 = fsub double %695, %705
  %734 = fmul double %732, 0.000000e+00
  %735 = fsub double %734, %733
  %736 = fmul double %733, 0.000000e+00
  %737 = fadd double %732, %736
  %738 = fadd double %726, %730
  %739 = fadd double %727, %731
  %740 = fsub double %726, %730
  %741 = fsub double %727, %731
  %742 = fadd double %728, %735
  %743 = fadd double %729, %737
  %744 = fsub double %728, %735
  %745 = fsub double %729, %737
  %746 = fdiv double %718, 5.120000e+02
  %747 = insertelement <2 x double> undef, double %746, i32 0
  %748 = fdiv double %719, 5.120000e+02
  %749 = insertelement <2 x double> %747, double %748, i32 1
  store <2 x double> %749, <2 x double>* %31, align 16
  %750 = fdiv double %720, 5.120000e+02
  %751 = insertelement <2 x double> undef, double %750, i32 0
  %752 = fdiv double %721, 5.120000e+02
  %753 = insertelement <2 x double> %751, double %752, i32 1
  store <2 x double> %753, <2 x double>* %64, align 16
  %754 = fdiv double %722, 5.120000e+02
  %755 = insertelement <2 x double> undef, double %754, i32 0
  %756 = fdiv double %723, 5.120000e+02
  %757 = insertelement <2 x double> %755, double %756, i32 1
  store <2 x double> %757, <2 x double>* %82, align 16
  %758 = fdiv double %724, 5.120000e+02
  %759 = insertelement <2 x double> undef, double %758, i32 0
  %760 = fdiv double %725, 5.120000e+02
  %761 = insertelement <2 x double> %759, double %760, i32 1
  store <2 x double> %761, <2 x double>* %100, align 16
  %762 = fdiv double %738, 5.120000e+02
  %763 = insertelement <2 x double> undef, double %762, i32 0
  %764 = fdiv double %739, 5.120000e+02
  %765 = insertelement <2 x double> %763, double %764, i32 1
  store <2 x double> %765, <2 x double>* %49, align 16
  %766 = fdiv double %740, 5.120000e+02
  %767 = insertelement <2 x double> undef, double %766, i32 0
  %768 = fdiv double %741, 5.120000e+02
  %769 = insertelement <2 x double> %767, double %768, i32 1
  store <2 x double> %769, <2 x double>* %67, align 16
  %770 = fdiv double %742, 5.120000e+02
  %771 = insertelement <2 x double> undef, double %770, i32 0
  %772 = fdiv double %743, 5.120000e+02
  %773 = insertelement <2 x double> %771, double %772, i32 1
  store <2 x double> %773, <2 x double>* %85, align 16
  %774 = fdiv double %744, 5.120000e+02
  %775 = insertelement <2 x double> undef, double %774, i32 0
  %776 = fdiv double %745, 5.120000e+02
  %777 = insertelement <2 x double> %775, double %776, i32 1
  store <2 x double> %777, <2 x double>* %103, align 16
  %"&(pSB[currWI].offset)67.i" = add nuw i64 %CurrSBIndex..0.i, 192
  %"&pSB[currWI].offset68.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)67.i"
  %CastToValueType69.i = bitcast i8* %"&pSB[currWI].offset68.i" to <2 x double> addrspace(1)**
  %loadedValue70.i = load <2 x double> addrspace(1)** %CastToValueType69.i, align 8
  %778 = load <2 x double>* %31, align 16
  store <2 x double> %778, <2 x double> addrspace(1)* %loadedValue70.i, align 16
  %scevgep2.1.i2.i = getelementptr <2 x double> addrspace(1)* %loadedValue70.i, i64 64
  %"&pSB[currWI].offset146.sum154.i" = add i64 %CurrSBIndex..1.i, 288
  %779 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum154.i"
  %780 = bitcast i8* %779 to <2 x double>*
  %781 = load <2 x double>* %780, align 16
  store <2 x double> %781, <2 x double> addrspace(1)* %scevgep2.1.i2.i, align 16
  %scevgep2.2.i3.i = getelementptr <2 x double> addrspace(1)* %loadedValue70.i, i64 128
  %"&pSB[currWI].offset146.sum155.i" = add i64 %CurrSBIndex..1.i, 256
  %782 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum155.i"
  %783 = bitcast i8* %782 to <2 x double>*
  %784 = load <2 x double>* %783, align 16
  store <2 x double> %784, <2 x double> addrspace(1)* %scevgep2.2.i3.i, align 16
  %scevgep2.3.i4.i = getelementptr <2 x double> addrspace(1)* %loadedValue70.i, i64 192
  %"&pSB[currWI].offset146.sum156.i" = add i64 %CurrSBIndex..1.i, 320
  %785 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum156.i"
  %786 = bitcast i8* %785 to <2 x double>*
  %787 = load <2 x double>* %786, align 16
  store <2 x double> %787, <2 x double> addrspace(1)* %scevgep2.3.i4.i, align 16
  %scevgep2.4.i5.i = getelementptr <2 x double> addrspace(1)* %loadedValue70.i, i64 256
  %"&pSB[currWI].offset146.sum157.i" = add i64 %CurrSBIndex..1.i, 240
  %788 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum157.i"
  %789 = bitcast i8* %788 to <2 x double>*
  %790 = load <2 x double>* %789, align 16
  store <2 x double> %790, <2 x double> addrspace(1)* %scevgep2.4.i5.i, align 16
  %scevgep2.5.i6.i = getelementptr <2 x double> addrspace(1)* %loadedValue70.i, i64 320
  %"&pSB[currWI].offset146.sum158.i" = add i64 %CurrSBIndex..1.i, 304
  %791 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum158.i"
  %792 = bitcast i8* %791 to <2 x double>*
  %793 = load <2 x double>* %792, align 16
  store <2 x double> %793, <2 x double> addrspace(1)* %scevgep2.5.i6.i, align 16
  %scevgep2.6.i7.i = getelementptr <2 x double> addrspace(1)* %loadedValue70.i, i64 384
  %"&pSB[currWI].offset146.sum159.i" = add i64 %CurrSBIndex..1.i, 272
  %794 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum159.i"
  %795 = bitcast i8* %794 to <2 x double>*
  %796 = load <2 x double>* %795, align 16
  store <2 x double> %796, <2 x double> addrspace(1)* %scevgep2.6.i7.i, align 16
  %scevgep2.7.i8.i = getelementptr <2 x double> addrspace(1)* %loadedValue70.i, i64 448
  %"&pSB[currWI].offset146.sum160.i" = add i64 %CurrSBIndex..1.i, 336
  %797 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset146.sum160.i"
  %798 = bitcast i8* %797 to <2 x double>*
  %799 = load <2 x double>* %798, align 16
  store <2 x double> %799, <2 x double> addrspace(1)* %scevgep2.7.i8.i, align 16
  %loadedCurrWI.i = load i64* %19, align 8
  %check.WI.iter.i = icmp ult i64 %loadedCurrWI.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__ifft1D_512_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB196.i
  %"CurrWI++.i" = add nuw i64 %loadedCurrWI.i, 1
  store i64 %"CurrWI++.i", i64* %19, align 8
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 608
  br label %SyncBB196.i

__ifft1D_512_separated_args.exit:                 ; preds = %SyncBB196.i
  store i64 0, i64* %19, align 8
  ret void
}

define void @fft1D_512(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <2 x double> addrspace(1)**
  %1 = load <2 x double> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i8 addrspace(3)**
  %4 = load i8 addrspace(3)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 24
  %6 = bitcast i8* %5 to i64**
  %7 = load i64** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to i8**
  %16 = load i8** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to i64**
  %19 = load i64** %18, align 8
  %20 = bitcast i8 addrspace(3)* %4 to [576 x double] addrspace(3)*
  store i64 0, i64* %19, align 8
  br label %SyncBB193.i

SyncBB193.i:                                      ; preds = %thenBB195.i, %entry
  %CurrSBIndex..1.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride201.i", %thenBB195.i ]
  %currWI.i = load i64* %19, align 8
  %21 = getelementptr %struct.PaddedDimId* %10, i64 %currWI.i, i32 0, i64 0
  %22 = load i64* %21, align 8
  %23 = trunc i64 %22 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %16, i64 %CurrSBIndex..1.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %23, i32* %CastToValueType.i, align 4
  %24 = load i64* %7, align 8
  %25 = shl i64 %24, 9
  %26 = add i64 %25, %22
  %27 = ashr i32 %23, 3
  %"&(pSB[currWI].offset)3265.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset33.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)3265.i"
  %CastToValueType34.i = bitcast i8* %"&pSB[currWI].offset33.i" to i32*
  store i32 %27, i32* %CastToValueType34.i, align 4
  %28 = and i32 %23, 7
  %"&(pSB[currWI].offset)4666.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset47.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)4666.i"
  %CastToValueType48.i = bitcast i8* %"&pSB[currWI].offset47.i" to i32*
  store i32 %28, i32* %CastToValueType48.i, align 4
  %sext.i = shl i64 %26, 32
  %29 = ashr i64 %sext.i, 32
  %30 = getelementptr inbounds <2 x double> addrspace(1)* %1, i64 %29
  %"&(pSB[currWI].offset)6067.i" = or i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset61.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)6067.i"
  %CastToValueType62.i = bitcast i8* %"&pSB[currWI].offset61.i" to <2 x double> addrspace(1)**
  store <2 x double> addrspace(1)* %30, <2 x double> addrspace(1)** %CastToValueType62.i, align 8
  %"&(pSB[currWI].offset)142.i" = add nuw i64 %CurrSBIndex..1.i, 48
  %"&pSB[currWI].offset143.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)142.i"
  %31 = bitcast i8* %"&pSB[currWI].offset143.i" to <2 x double>*
  %32 = load <2 x double> addrspace(1)* %30, align 16
  store <2 x double> %32, <2 x double>* %31, align 16
  %"&pSB[currWI].offset143.sum.i" = add i64 %CurrSBIndex..1.i, 64
  %scevgep.1.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum.i"
  %33 = bitcast i8* %scevgep.1.i.i to <2 x double>*
  %.sum.i = add i64 %29, 64
  %scevgep2.1.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum.i
  %34 = load <2 x double> addrspace(1)* %scevgep2.1.i.i, align 16
  store <2 x double> %34, <2 x double>* %33, align 16
  %"&pSB[currWI].offset143.sum68.i" = add i64 %CurrSBIndex..1.i, 80
  %scevgep.2.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum68.i"
  %35 = bitcast i8* %scevgep.2.i.i to <2 x double>*
  %.sum69.i = add i64 %29, 128
  %scevgep2.2.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum69.i
  %36 = load <2 x double> addrspace(1)* %scevgep2.2.i.i, align 16
  store <2 x double> %36, <2 x double>* %35, align 16
  %"&pSB[currWI].offset143.sum70.i" = add i64 %CurrSBIndex..1.i, 96
  %scevgep.3.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum70.i"
  %37 = bitcast i8* %scevgep.3.i.i to <2 x double>*
  %.sum71.i = add i64 %29, 192
  %scevgep2.3.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum71.i
  %38 = load <2 x double> addrspace(1)* %scevgep2.3.i.i, align 16
  store <2 x double> %38, <2 x double>* %37, align 16
  %"&pSB[currWI].offset143.sum72.i" = add i64 %CurrSBIndex..1.i, 112
  %scevgep.4.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum72.i"
  %39 = bitcast i8* %scevgep.4.i.i to <2 x double>*
  %.sum73.i = add i64 %29, 256
  %scevgep2.4.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum73.i
  %40 = load <2 x double> addrspace(1)* %scevgep2.4.i.i, align 16
  store <2 x double> %40, <2 x double>* %39, align 16
  %"&pSB[currWI].offset143.sum74.i" = add i64 %CurrSBIndex..1.i, 128
  %scevgep.5.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum74.i"
  %41 = bitcast i8* %scevgep.5.i.i to <2 x double>*
  %.sum75.i = add i64 %29, 320
  %scevgep2.5.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum75.i
  %42 = load <2 x double> addrspace(1)* %scevgep2.5.i.i, align 16
  store <2 x double> %42, <2 x double>* %41, align 16
  %"&pSB[currWI].offset143.sum76.i" = add i64 %CurrSBIndex..1.i, 144
  %scevgep.6.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum76.i"
  %43 = bitcast i8* %scevgep.6.i.i to <2 x double>*
  %.sum77.i = add i64 %29, 384
  %scevgep2.6.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum77.i
  %44 = load <2 x double> addrspace(1)* %scevgep2.6.i.i, align 16
  store <2 x double> %44, <2 x double>* %43, align 16
  %"&pSB[currWI].offset143.sum78.i" = add i64 %CurrSBIndex..1.i, 160
  %scevgep.7.i.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum78.i"
  %45 = bitcast i8* %scevgep.7.i.i to <2 x double>*
  %.sum79.i = add i64 %29, 448
  %scevgep2.7.i.i = getelementptr <2 x double> addrspace(1)* %1, i64 %.sum79.i
  %46 = load <2 x double> addrspace(1)* %scevgep2.7.i.i, align 16
  store <2 x double> %46, <2 x double>* %45, align 16
  %47 = load <2 x double>* %31, align 16
  %"&pSB[currWI].offset139.sum.i" = add i64 %CurrSBIndex..1.i, 112
  %48 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset139.sum.i"
  %49 = bitcast i8* %48 to <2 x double>*
  %50 = load <2 x double>* %49, align 16
  %51 = extractelement <2 x double> %47, i32 0
  %52 = extractelement <2 x double> %50, i32 0
  %53 = fadd double %51, %52
  %54 = extractelement <2 x double> %47, i32 1
  %55 = extractelement <2 x double> %50, i32 1
  %56 = fadd double %54, %55
  %57 = extractelement <2 x double> %47, i32 0
  %58 = extractelement <2 x double> %50, i32 0
  %59 = fsub double %57, %58
  %60 = extractelement <2 x double> %47, i32 1
  %61 = extractelement <2 x double> %50, i32 1
  %62 = fsub double %60, %61
  %"&pSB[currWI].offset135.sum.i" = add i64 %CurrSBIndex..1.i, 64
  %63 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset135.sum.i"
  %64 = bitcast i8* %63 to <2 x double>*
  %65 = load <2 x double>* %64, align 16
  %"&pSB[currWI].offset131.sum.i" = add i64 %CurrSBIndex..1.i, 128
  %66 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset131.sum.i"
  %67 = bitcast i8* %66 to <2 x double>*
  %68 = load <2 x double>* %67, align 16
  %69 = extractelement <2 x double> %65, i32 0
  %70 = extractelement <2 x double> %68, i32 0
  %71 = fadd double %69, %70
  %72 = extractelement <2 x double> %65, i32 1
  %73 = extractelement <2 x double> %68, i32 1
  %74 = fadd double %72, %73
  %75 = extractelement <2 x double> %65, i32 0
  %76 = extractelement <2 x double> %68, i32 0
  %77 = fsub double %75, %76
  %78 = extractelement <2 x double> %65, i32 1
  %79 = extractelement <2 x double> %68, i32 1
  %80 = fsub double %78, %79
  %"&pSB[currWI].offset127.sum.i" = add i64 %CurrSBIndex..1.i, 80
  %81 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset127.sum.i"
  %82 = bitcast i8* %81 to <2 x double>*
  %83 = load <2 x double>* %82, align 16
  %"&pSB[currWI].offset123.sum.i" = add i64 %CurrSBIndex..1.i, 144
  %84 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset123.sum.i"
  %85 = bitcast i8* %84 to <2 x double>*
  %86 = load <2 x double>* %85, align 16
  %87 = extractelement <2 x double> %83, i32 0
  %88 = extractelement <2 x double> %86, i32 0
  %89 = fadd double %87, %88
  %90 = extractelement <2 x double> %83, i32 1
  %91 = extractelement <2 x double> %86, i32 1
  %92 = fadd double %90, %91
  %93 = extractelement <2 x double> %83, i32 0
  %94 = extractelement <2 x double> %86, i32 0
  %95 = fsub double %93, %94
  %96 = extractelement <2 x double> %83, i32 1
  %97 = extractelement <2 x double> %86, i32 1
  %98 = fsub double %96, %97
  %"&pSB[currWI].offset119.sum.i" = add i64 %CurrSBIndex..1.i, 96
  %99 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset119.sum.i"
  %100 = bitcast i8* %99 to <2 x double>*
  %101 = load <2 x double>* %100, align 16
  %"&pSB[currWI].offset115.sum.i" = add i64 %CurrSBIndex..1.i, 160
  %102 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset115.sum.i"
  %103 = bitcast i8* %102 to <2 x double>*
  %104 = load <2 x double>* %103, align 16
  %105 = extractelement <2 x double> %101, i32 0
  %106 = extractelement <2 x double> %104, i32 0
  %107 = fadd double %105, %106
  %108 = extractelement <2 x double> %101, i32 1
  %109 = extractelement <2 x double> %104, i32 1
  %110 = fadd double %108, %109
  %111 = extractelement <2 x double> %101, i32 0
  %112 = extractelement <2 x double> %104, i32 0
  %113 = fsub double %111, %112
  %114 = extractelement <2 x double> %101, i32 1
  %115 = extractelement <2 x double> %104, i32 1
  %116 = fsub double %114, %115
  %117 = fmul double %80, -1.000000e+00
  %118 = fsub double %77, %117
  %119 = fmul double %77, -1.000000e+00
  %120 = fadd double %119, %80
  %121 = fmul double %118, 0x3FE6A09E667F3BCD
  %122 = fmul double %120, 0x3FE6A09E667F3BCD
  %123 = fmul double %95, 0.000000e+00
  %124 = fmul double %98, -1.000000e+00
  %125 = fsub double %123, %124
  %126 = fmul double %95, -1.000000e+00
  %127 = fmul double %98, 0.000000e+00
  %128 = fadd double %126, %127
  %129 = fmul double %113, -1.000000e+00
  %130 = fmul double %116, -1.000000e+00
  %131 = fsub double %129, %130
  %132 = fmul double %113, -1.000000e+00
  %133 = fmul double %116, -1.000000e+00
  %134 = fadd double %132, %133
  %135 = fmul double %131, 0x3FE6A09E667F3BCD
  %136 = fmul double %134, 0x3FE6A09E667F3BCD
  %137 = fadd double %53, %89
  %138 = fadd double %56, %92
  %139 = fsub double %53, %89
  %140 = fsub double %56, %92
  %141 = fadd double %71, %107
  %142 = fadd double %74, %110
  %143 = fsub double %71, %107
  %144 = fsub double %74, %110
  %145 = fmul double %143, 0.000000e+00
  %146 = fmul double %144, -1.000000e+00
  %147 = fsub double %145, %146
  %148 = fmul double %143, -1.000000e+00
  %149 = fmul double %144, 0.000000e+00
  %150 = fadd double %148, %149
  %151 = fadd double %137, %141
  %152 = insertelement <2 x double> undef, double %151, i32 0
  %153 = fadd double %138, %142
  %154 = insertelement <2 x double> %152, double %153, i32 1
  store <2 x double> %154, <2 x double>* %31, align 16
  %155 = fsub double %137, %141
  %156 = insertelement <2 x double> undef, double %155, i32 0
  %157 = fsub double %138, %142
  %158 = insertelement <2 x double> %156, double %157, i32 1
  store <2 x double> %158, <2 x double>* %64, align 16
  %159 = fadd double %139, %147
  %160 = insertelement <2 x double> undef, double %159, i32 0
  %161 = fadd double %140, %150
  %162 = insertelement <2 x double> %160, double %161, i32 1
  store <2 x double> %162, <2 x double>* %82, align 16
  %163 = fsub double %139, %147
  %164 = insertelement <2 x double> undef, double %163, i32 0
  %165 = fsub double %140, %150
  %166 = insertelement <2 x double> %164, double %165, i32 1
  store <2 x double> %166, <2 x double>* %100, align 16
  %167 = fadd double %59, %125
  %168 = fadd double %62, %128
  %169 = fsub double %59, %125
  %170 = fsub double %62, %128
  %171 = fadd double %121, %135
  %172 = fadd double %122, %136
  %173 = fsub double %121, %135
  %174 = fsub double %122, %136
  %175 = fmul double %173, 0.000000e+00
  %176 = fmul double %174, -1.000000e+00
  %177 = fsub double %175, %176
  %178 = fmul double %173, -1.000000e+00
  %179 = fmul double %174, 0.000000e+00
  %180 = fadd double %178, %179
  %181 = fadd double %167, %171
  %182 = insertelement <2 x double> undef, double %181, i32 0
  %183 = fadd double %168, %172
  %184 = insertelement <2 x double> %182, double %183, i32 1
  store <2 x double> %184, <2 x double>* %49, align 16
  %185 = fsub double %167, %171
  %186 = insertelement <2 x double> undef, double %185, i32 0
  %187 = fsub double %168, %172
  %188 = insertelement <2 x double> %186, double %187, i32 1
  store <2 x double> %188, <2 x double>* %67, align 16
  %189 = fadd double %169, %177
  %190 = insertelement <2 x double> undef, double %189, i32 0
  %191 = fadd double %170, %180
  %192 = insertelement <2 x double> %190, double %191, i32 1
  store <2 x double> %192, <2 x double>* %85, align 16
  %193 = fsub double %169, %177
  %194 = insertelement <2 x double> undef, double %193, i32 0
  %195 = fsub double %170, %180
  %196 = insertelement <2 x double> %194, double %195, i32 1
  store <2 x double> %196, <2 x double>* %103, align 16
  %197 = sitofp i32 %23 to double
  %"&(pSB[currWI].offset)106.i" = add nuw i64 %CurrSBIndex..1.i, 48
  %"&pSB[currWI].offset107.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)106.i"
  %CastToValueType108.i = bitcast i8* %"&pSB[currWI].offset107.i" to [8 x <2 x double>]*
  br label %198

; <label>:198                                     ; preds = %._crit_edge13.i, %SyncBB193.i
  %indvar6.i = phi i64 [ 1, %SyncBB193.i ], [ %phitmp.i, %._crit_edge13.i ]
  %scevgep10.i = getelementptr [8 x <2 x double>]* %CastToValueType108.i, i64 0, i64 %indvar6.i
  %scevgep11.i = getelementptr [8 x i32]* @fft1D_512.reversed8, i64 0, i64 %indvar6.i
  %199 = load <2 x double>* %scevgep10.i, align 16
  %200 = load i32* %scevgep11.i, align 4
  %201 = sitofp i32 %200 to double
  %202 = fmul double %201, 0xC01921FB54442D18
  %203 = fdiv double %202, 5.120000e+02
  %204 = fmul double %203, %197
  %205 = call double @_Z3cosd(double %204) nounwind
  %206 = call double @_Z3sind(double %204) nounwind
  %207 = extractelement <2 x double> %199, i32 0
  %208 = fmul double %207, %205
  %209 = extractelement <2 x double> %199, i32 1
  %210 = fmul double %209, %206
  %211 = fsub double %208, %210
  %212 = insertelement <2 x double> undef, double %211, i32 0
  %213 = fmul double %207, %206
  %214 = fmul double %209, %205
  %215 = fadd double %213, %214
  %216 = insertelement <2 x double> %212, double %215, i32 1
  store <2 x double> %216, <2 x double>* %scevgep10.i, align 16
  %exitcond8.i = icmp eq i64 %indvar6.i, 7
  br i1 %exitcond8.i, label %bb.nph.i, label %._crit_edge13.i

._crit_edge13.i:                                  ; preds = %198
  %phitmp.i = add i64 %indvar6.i, 1
  br label %198

bb.nph.i:                                         ; preds = %198
  %"&pSB[currWI].offset28.i" = getelementptr inbounds i8* %16, i64 %CurrSBIndex..1.i
  %CastToValueType29.i = bitcast i8* %"&pSB[currWI].offset28.i" to i32*
  %loadedValue30.i = load i32* %CastToValueType29.i, align 4
  %217 = sext i32 %loadedValue30.i to i64
  %218 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %217
  %"&(pSB[currWI].offset)6980.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset70.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)6980.i"
  %CastToValueType71.i = bitcast i8* %"&pSB[currWI].offset70.i" to double addrspace(3)**
  store double addrspace(3)* %218, double addrspace(3)** %CastToValueType71.i, align 8
  %219 = load <2 x double>* %31, align 16
  %220 = extractelement <2 x double> %219, i32 0
  store double %220, double addrspace(3)* %218, align 8
  %"&pSB[currWI].offset143.sum81.i" = add i64 %CurrSBIndex..1.i, 112
  %221 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum81.i"
  %222 = bitcast i8* %221 to <2 x double>*
  %223 = load <2 x double>* %222, align 16
  %224 = extractelement <2 x double> %223, i32 0
  %.sum82.i = add i64 %217, 66
  %225 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum82.i
  store double %224, double addrspace(3)* %225, align 8
  %"&pSB[currWI].offset143.sum83.i" = add i64 %CurrSBIndex..1.i, 80
  %226 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum83.i"
  %227 = bitcast i8* %226 to <2 x double>*
  %228 = load <2 x double>* %227, align 16
  %229 = extractelement <2 x double> %228, i32 0
  %.sum84.i = add i64 %217, 132
  %230 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum84.i
  store double %229, double addrspace(3)* %230, align 8
  %"&pSB[currWI].offset143.sum85.i" = add i64 %CurrSBIndex..1.i, 144
  %231 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum85.i"
  %232 = bitcast i8* %231 to <2 x double>*
  %233 = load <2 x double>* %232, align 16
  %234 = extractelement <2 x double> %233, i32 0
  %.sum86.i = add i64 %217, 198
  %235 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum86.i
  store double %234, double addrspace(3)* %235, align 8
  %"&pSB[currWI].offset143.sum87.i" = add i64 %CurrSBIndex..1.i, 64
  %236 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum87.i"
  %237 = bitcast i8* %236 to <2 x double>*
  %238 = load <2 x double>* %237, align 16
  %239 = extractelement <2 x double> %238, i32 0
  %.sum88.i = add i64 %217, 264
  %240 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum88.i
  store double %239, double addrspace(3)* %240, align 8
  %"&pSB[currWI].offset143.sum89.i" = add i64 %CurrSBIndex..1.i, 128
  %241 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum89.i"
  %242 = bitcast i8* %241 to <2 x double>*
  %243 = load <2 x double>* %242, align 16
  %244 = extractelement <2 x double> %243, i32 0
  %.sum90.i = add i64 %217, 330
  %245 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum90.i
  store double %244, double addrspace(3)* %245, align 8
  %"&pSB[currWI].offset143.sum91.i" = add i64 %CurrSBIndex..1.i, 96
  %246 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum91.i"
  %247 = bitcast i8* %246 to <2 x double>*
  %248 = load <2 x double>* %247, align 16
  %249 = extractelement <2 x double> %248, i32 0
  %.sum92.i = add i64 %217, 396
  %250 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum92.i
  store double %249, double addrspace(3)* %250, align 8
  %"&pSB[currWI].offset143.sum93.i" = add i64 %CurrSBIndex..1.i, 160
  %251 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum93.i"
  %252 = bitcast i8* %251 to <2 x double>*
  %253 = load <2 x double>* %252, align 16
  %254 = extractelement <2 x double> %253, i32 0
  %.sum94.i = add i64 %217, 462
  %255 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %.sum94.i
  store double %254, double addrspace(3)* %255, align 8
  %loadedCurrWI197.i = load i64* %19, align 8
  %check.WI.iter198.i = icmp ult i64 %loadedCurrWI197.i, %13
  br i1 %check.WI.iter198.i, label %thenBB195.i, label %elseBB196.i

thenBB195.i:                                      ; preds = %bb.nph.i
  %"CurrWI++199.i" = add nuw i64 %loadedCurrWI197.i, 1
  store i64 %"CurrWI++199.i", i64* %19, align 8
  %"loadedCurrSB+Stride201.i" = add nuw i64 %CurrSBIndex..1.i, 608
  br label %SyncBB193.i

elseBB196.i:                                      ; preds = %bb.nph.i
  store i64 0, i64* %19, align 8
  br label %SyncBB186.i

SyncBB186.i:                                      ; preds = %thenBB202.i, %elseBB196.i
  %CurrSBIndex..2.i = phi i64 [ 0, %elseBB196.i ], [ %"loadedCurrSB+Stride208.i", %thenBB202.i ]
  %"&(pSB[currWI].offset)5595.i" = or i64 %CurrSBIndex..2.i, 8
  %"&pSB[currWI].offset56.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)5595.i"
  %CastToValueType57.i = bitcast i8* %"&pSB[currWI].offset56.i" to i32*
  %loadedValue58.i = load i32* %CastToValueType57.i, align 4
  %256 = mul nsw i32 %loadedValue58.i, 66
  %"&(pSB[currWI].offset)3696.i" = or i64 %CurrSBIndex..2.i, 4
  %"&pSB[currWI].offset37.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)3696.i"
  %CastToValueType38.i = bitcast i8* %"&pSB[currWI].offset37.i" to i32*
  %loadedValue39.i = load i32* %CastToValueType38.i, align 4
  %257 = add nsw i32 %256, %loadedValue39.i
  %258 = sext i32 %257 to i64
  %259 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %258
  %"&(pSB[currWI].offset)88.i" = add nuw i64 %CurrSBIndex..2.i, 32
  %"&pSB[currWI].offset89.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)88.i"
  %CastToValueType90.i = bitcast i8* %"&pSB[currWI].offset89.i" to double addrspace(3)**
  store double addrspace(3)* %259, double addrspace(3)** %CastToValueType90.i, align 8
  %260 = load double addrspace(3)* %259, align 8
  %261 = load <2 x double>* %31, align 16
  %262 = insertelement <2 x double> %261, double %260, i32 0
  store <2 x double> %262, <2 x double>* %31, align 16
  %"&pSB[currWI].offset143.sum97.i" = add i64 %CurrSBIndex..1.i, 64
  %scevgep.1.i51.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum97.i"
  %263 = bitcast i8* %scevgep.1.i51.i to <2 x double>*
  %.sum98.i = add i64 %258, 8
  %scevgep2.1.i52.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum98.i
  %264 = load double addrspace(3)* %scevgep2.1.i52.i, align 8
  %265 = load <2 x double>* %263, align 16
  %266 = insertelement <2 x double> %265, double %264, i32 0
  store <2 x double> %266, <2 x double>* %263, align 16
  %"&pSB[currWI].offset143.sum99.i" = add i64 %CurrSBIndex..1.i, 80
  %scevgep.2.i53.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum99.i"
  %267 = bitcast i8* %scevgep.2.i53.i to <2 x double>*
  %.sum100.i = add i64 %258, 16
  %scevgep2.2.i54.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum100.i
  %268 = load double addrspace(3)* %scevgep2.2.i54.i, align 8
  %269 = load <2 x double>* %267, align 16
  %270 = insertelement <2 x double> %269, double %268, i32 0
  store <2 x double> %270, <2 x double>* %267, align 16
  %"&pSB[currWI].offset143.sum101.i" = add i64 %CurrSBIndex..1.i, 96
  %scevgep.3.i55.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum101.i"
  %271 = bitcast i8* %scevgep.3.i55.i to <2 x double>*
  %.sum102.i = add i64 %258, 24
  %scevgep2.3.i56.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum102.i
  %272 = load double addrspace(3)* %scevgep2.3.i56.i, align 8
  %273 = load <2 x double>* %271, align 16
  %274 = insertelement <2 x double> %273, double %272, i32 0
  store <2 x double> %274, <2 x double>* %271, align 16
  %"&pSB[currWI].offset143.sum103.i" = add i64 %CurrSBIndex..1.i, 112
  %scevgep.4.i57.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum103.i"
  %275 = bitcast i8* %scevgep.4.i57.i to <2 x double>*
  %.sum104.i = add i64 %258, 32
  %scevgep2.4.i58.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum104.i
  %276 = load double addrspace(3)* %scevgep2.4.i58.i, align 8
  %277 = load <2 x double>* %275, align 16
  %278 = insertelement <2 x double> %277, double %276, i32 0
  store <2 x double> %278, <2 x double>* %275, align 16
  %"&pSB[currWI].offset143.sum105.i" = add i64 %CurrSBIndex..1.i, 128
  %scevgep.5.i59.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum105.i"
  %279 = bitcast i8* %scevgep.5.i59.i to <2 x double>*
  %.sum106.i = add i64 %258, 40
  %scevgep2.5.i60.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum106.i
  %280 = load double addrspace(3)* %scevgep2.5.i60.i, align 8
  %281 = load <2 x double>* %279, align 16
  %282 = insertelement <2 x double> %281, double %280, i32 0
  store <2 x double> %282, <2 x double>* %279, align 16
  %"&pSB[currWI].offset143.sum107.i" = add i64 %CurrSBIndex..1.i, 144
  %scevgep.6.i61.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum107.i"
  %283 = bitcast i8* %scevgep.6.i61.i to <2 x double>*
  %.sum108.i = add i64 %258, 48
  %scevgep2.6.i62.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum108.i
  %284 = load double addrspace(3)* %scevgep2.6.i62.i, align 8
  %285 = load <2 x double>* %283, align 16
  %286 = insertelement <2 x double> %285, double %284, i32 0
  store <2 x double> %286, <2 x double>* %283, align 16
  %"&pSB[currWI].offset143.sum109.i" = add i64 %CurrSBIndex..1.i, 160
  %scevgep.7.i63.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum109.i"
  %287 = bitcast i8* %scevgep.7.i63.i to <2 x double>*
  %.sum110.i = add i64 %258, 56
  %scevgep2.7.i64.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum110.i
  %288 = load double addrspace(3)* %scevgep2.7.i64.i, align 8
  %289 = load <2 x double>* %287, align 16
  %290 = insertelement <2 x double> %289, double %288, i32 0
  store <2 x double> %290, <2 x double>* %287, align 16
  %loadedCurrWI204.i = load i64* %19, align 8
  %check.WI.iter205.i = icmp ult i64 %loadedCurrWI204.i, %13
  br i1 %check.WI.iter205.i, label %thenBB202.i, label %elseBB203.i

thenBB202.i:                                      ; preds = %SyncBB186.i
  %"CurrWI++206.i" = add nuw i64 %loadedCurrWI204.i, 1
  store i64 %"CurrWI++206.i", i64* %19, align 8
  %"loadedCurrSB+Stride208.i" = add nuw i64 %CurrSBIndex..2.i, 608
  br label %SyncBB186.i

elseBB203.i:                                      ; preds = %SyncBB186.i
  store i64 0, i64* %19, align 8
  br label %SyncBB187.i

SyncBB187.i:                                      ; preds = %thenBB209.i, %elseBB203.i
  %CurrSBIndex..3.i = phi i64 [ 0, %elseBB203.i ], [ %"loadedCurrSB+Stride215.i", %thenBB209.i ]
  %"&(pSB[currWI].offset)83111.i" = or i64 %CurrSBIndex..3.i, 24
  %"&pSB[currWI].offset84.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)83111.i"
  %CastToValueType85.i = bitcast i8* %"&pSB[currWI].offset84.i" to double addrspace(3)**
  %loadedValue86.i = load double addrspace(3)** %CastToValueType85.i, align 8
  %291 = load <2 x double>* %31, align 16
  %292 = extractelement <2 x double> %291, i32 1
  store double %292, double addrspace(3)* %loadedValue86.i, align 8
  %"&pSB[currWI].offset143.sum112.i" = add i64 %CurrSBIndex..1.i, 112
  %293 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum112.i"
  %294 = bitcast i8* %293 to <2 x double>*
  %295 = load <2 x double>* %294, align 16
  %296 = extractelement <2 x double> %295, i32 1
  %297 = getelementptr inbounds double addrspace(3)* %loadedValue86.i, i64 66
  store double %296, double addrspace(3)* %297, align 8
  %"&pSB[currWI].offset143.sum113.i" = add i64 %CurrSBIndex..1.i, 80
  %298 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum113.i"
  %299 = bitcast i8* %298 to <2 x double>*
  %300 = load <2 x double>* %299, align 16
  %301 = extractelement <2 x double> %300, i32 1
  %302 = getelementptr inbounds double addrspace(3)* %loadedValue86.i, i64 132
  store double %301, double addrspace(3)* %302, align 8
  %"&pSB[currWI].offset143.sum114.i" = add i64 %CurrSBIndex..1.i, 144
  %303 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum114.i"
  %304 = bitcast i8* %303 to <2 x double>*
  %305 = load <2 x double>* %304, align 16
  %306 = extractelement <2 x double> %305, i32 1
  %307 = getelementptr inbounds double addrspace(3)* %loadedValue86.i, i64 198
  store double %306, double addrspace(3)* %307, align 8
  %"&pSB[currWI].offset143.sum115.i" = add i64 %CurrSBIndex..1.i, 64
  %308 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum115.i"
  %309 = bitcast i8* %308 to <2 x double>*
  %310 = load <2 x double>* %309, align 16
  %311 = extractelement <2 x double> %310, i32 1
  %312 = getelementptr inbounds double addrspace(3)* %loadedValue86.i, i64 264
  store double %311, double addrspace(3)* %312, align 8
  %"&pSB[currWI].offset143.sum116.i" = add i64 %CurrSBIndex..1.i, 128
  %313 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum116.i"
  %314 = bitcast i8* %313 to <2 x double>*
  %315 = load <2 x double>* %314, align 16
  %316 = extractelement <2 x double> %315, i32 1
  %317 = getelementptr inbounds double addrspace(3)* %loadedValue86.i, i64 330
  store double %316, double addrspace(3)* %317, align 8
  %"&pSB[currWI].offset143.sum117.i" = add i64 %CurrSBIndex..1.i, 96
  %318 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum117.i"
  %319 = bitcast i8* %318 to <2 x double>*
  %320 = load <2 x double>* %319, align 16
  %321 = extractelement <2 x double> %320, i32 1
  %322 = getelementptr inbounds double addrspace(3)* %loadedValue86.i, i64 396
  store double %321, double addrspace(3)* %322, align 8
  %"&pSB[currWI].offset143.sum118.i" = add i64 %CurrSBIndex..1.i, 160
  %323 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum118.i"
  %324 = bitcast i8* %323 to <2 x double>*
  %325 = load <2 x double>* %324, align 16
  %326 = extractelement <2 x double> %325, i32 1
  %327 = getelementptr inbounds double addrspace(3)* %loadedValue86.i, i64 462
  store double %326, double addrspace(3)* %327, align 8
  %loadedCurrWI211.i = load i64* %19, align 8
  %check.WI.iter212.i = icmp ult i64 %loadedCurrWI211.i, %13
  br i1 %check.WI.iter212.i, label %thenBB209.i, label %elseBB210.i

thenBB209.i:                                      ; preds = %SyncBB187.i
  %"CurrWI++213.i" = add nuw i64 %loadedCurrWI211.i, 1
  store i64 %"CurrWI++213.i", i64* %19, align 8
  %"loadedCurrSB+Stride215.i" = add nuw i64 %CurrSBIndex..3.i, 608
  br label %SyncBB187.i

elseBB210.i:                                      ; preds = %SyncBB187.i
  store i64 0, i64* %19, align 8
  br label %SyncBB188.i

SyncBB188.i:                                      ; preds = %thenBB216.i, %elseBB210.i
  %CurrSBIndex..4.i = phi i64 [ 0, %elseBB210.i ], [ %"loadedCurrSB+Stride222.i", %thenBB216.i ]
  %"&(pSB[currWI].offset)92.i" = add nuw i64 %CurrSBIndex..4.i, 32
  %"&pSB[currWI].offset93.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)92.i"
  %CastToValueType94.i = bitcast i8* %"&pSB[currWI].offset93.i" to double addrspace(3)**
  %loadedValue95.i = load double addrspace(3)** %CastToValueType94.i, align 8
  %328 = load double addrspace(3)* %loadedValue95.i, align 8
  %329 = load <2 x double>* %31, align 16
  %330 = insertelement <2 x double> %329, double %328, i32 1
  store <2 x double> %330, <2 x double>* %31, align 16
  %"&pSB[currWI].offset143.sum119.i" = add i64 %CurrSBIndex..1.i, 64
  %scevgep.1.i37.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum119.i"
  %331 = bitcast i8* %scevgep.1.i37.i to <2 x double>*
  %scevgep2.1.i38.i = getelementptr double addrspace(3)* %loadedValue95.i, i64 8
  %332 = load double addrspace(3)* %scevgep2.1.i38.i, align 8
  %333 = load <2 x double>* %331, align 16
  %334 = insertelement <2 x double> %333, double %332, i32 1
  store <2 x double> %334, <2 x double>* %331, align 16
  %"&pSB[currWI].offset143.sum120.i" = add i64 %CurrSBIndex..1.i, 80
  %scevgep.2.i39.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum120.i"
  %335 = bitcast i8* %scevgep.2.i39.i to <2 x double>*
  %scevgep2.2.i40.i = getelementptr double addrspace(3)* %loadedValue95.i, i64 16
  %336 = load double addrspace(3)* %scevgep2.2.i40.i, align 8
  %337 = load <2 x double>* %335, align 16
  %338 = insertelement <2 x double> %337, double %336, i32 1
  store <2 x double> %338, <2 x double>* %335, align 16
  %"&pSB[currWI].offset143.sum121.i" = add i64 %CurrSBIndex..1.i, 96
  %scevgep.3.i41.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum121.i"
  %339 = bitcast i8* %scevgep.3.i41.i to <2 x double>*
  %scevgep2.3.i42.i = getelementptr double addrspace(3)* %loadedValue95.i, i64 24
  %340 = load double addrspace(3)* %scevgep2.3.i42.i, align 8
  %341 = load <2 x double>* %339, align 16
  %342 = insertelement <2 x double> %341, double %340, i32 1
  store <2 x double> %342, <2 x double>* %339, align 16
  %"&pSB[currWI].offset143.sum122.i" = add i64 %CurrSBIndex..1.i, 112
  %scevgep.4.i43.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum122.i"
  %343 = bitcast i8* %scevgep.4.i43.i to <2 x double>*
  %scevgep2.4.i44.i = getelementptr double addrspace(3)* %loadedValue95.i, i64 32
  %344 = load double addrspace(3)* %scevgep2.4.i44.i, align 8
  %345 = load <2 x double>* %343, align 16
  %346 = insertelement <2 x double> %345, double %344, i32 1
  store <2 x double> %346, <2 x double>* %343, align 16
  %"&pSB[currWI].offset143.sum123.i" = add i64 %CurrSBIndex..1.i, 128
  %scevgep.5.i45.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum123.i"
  %347 = bitcast i8* %scevgep.5.i45.i to <2 x double>*
  %scevgep2.5.i46.i = getelementptr double addrspace(3)* %loadedValue95.i, i64 40
  %348 = load double addrspace(3)* %scevgep2.5.i46.i, align 8
  %349 = load <2 x double>* %347, align 16
  %350 = insertelement <2 x double> %349, double %348, i32 1
  store <2 x double> %350, <2 x double>* %347, align 16
  %"&pSB[currWI].offset143.sum124.i" = add i64 %CurrSBIndex..1.i, 144
  %scevgep.6.i47.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum124.i"
  %351 = bitcast i8* %scevgep.6.i47.i to <2 x double>*
  %scevgep2.6.i48.i = getelementptr double addrspace(3)* %loadedValue95.i, i64 48
  %352 = load double addrspace(3)* %scevgep2.6.i48.i, align 8
  %353 = load <2 x double>* %351, align 16
  %354 = insertelement <2 x double> %353, double %352, i32 1
  store <2 x double> %354, <2 x double>* %351, align 16
  %"&pSB[currWI].offset143.sum125.i" = add i64 %CurrSBIndex..1.i, 160
  %scevgep.7.i49.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum125.i"
  %355 = bitcast i8* %scevgep.7.i49.i to <2 x double>*
  %scevgep2.7.i50.i = getelementptr double addrspace(3)* %loadedValue95.i, i64 56
  %356 = load double addrspace(3)* %scevgep2.7.i50.i, align 8
  %357 = load <2 x double>* %355, align 16
  %358 = insertelement <2 x double> %357, double %356, i32 1
  store <2 x double> %358, <2 x double>* %355, align 16
  %loadedCurrWI218.i = load i64* %19, align 8
  %check.WI.iter219.i = icmp ult i64 %loadedCurrWI218.i, %13
  br i1 %check.WI.iter219.i, label %thenBB216.i, label %elseBB217.i

thenBB216.i:                                      ; preds = %SyncBB188.i
  %"CurrWI++220.i" = add nuw i64 %loadedCurrWI218.i, 1
  store i64 %"CurrWI++220.i", i64* %19, align 8
  %"loadedCurrSB+Stride222.i" = add nuw i64 %CurrSBIndex..4.i, 608
  br label %SyncBB188.i

elseBB217.i:                                      ; preds = %SyncBB188.i
  store i64 0, i64* %19, align 8
  br label %SyncBB189.i

SyncBB189.i:                                      ; preds = %thenBB.i, %elseBB217.i
  %CurrSBIndex..0.i = phi i64 [ 0, %elseBB217.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %359 = load <2 x double>* %31, align 16
  %360 = load <2 x double>* %49, align 16
  %361 = extractelement <2 x double> %359, i32 0
  %362 = extractelement <2 x double> %360, i32 0
  %363 = fadd double %361, %362
  %364 = extractelement <2 x double> %359, i32 1
  %365 = extractelement <2 x double> %360, i32 1
  %366 = fadd double %364, %365
  %367 = extractelement <2 x double> %359, i32 0
  %368 = extractelement <2 x double> %360, i32 0
  %369 = fsub double %367, %368
  %370 = extractelement <2 x double> %359, i32 1
  %371 = extractelement <2 x double> %360, i32 1
  %372 = fsub double %370, %371
  %373 = load <2 x double>* %64, align 16
  %374 = load <2 x double>* %67, align 16
  %375 = extractelement <2 x double> %373, i32 0
  %376 = extractelement <2 x double> %374, i32 0
  %377 = fadd double %375, %376
  %378 = extractelement <2 x double> %373, i32 1
  %379 = extractelement <2 x double> %374, i32 1
  %380 = fadd double %378, %379
  %381 = extractelement <2 x double> %373, i32 0
  %382 = extractelement <2 x double> %374, i32 0
  %383 = fsub double %381, %382
  %384 = extractelement <2 x double> %373, i32 1
  %385 = extractelement <2 x double> %374, i32 1
  %386 = fsub double %384, %385
  %387 = load <2 x double>* %82, align 16
  %388 = load <2 x double>* %85, align 16
  %389 = extractelement <2 x double> %387, i32 0
  %390 = extractelement <2 x double> %388, i32 0
  %391 = fadd double %389, %390
  %392 = extractelement <2 x double> %387, i32 1
  %393 = extractelement <2 x double> %388, i32 1
  %394 = fadd double %392, %393
  %395 = extractelement <2 x double> %387, i32 0
  %396 = extractelement <2 x double> %388, i32 0
  %397 = fsub double %395, %396
  %398 = extractelement <2 x double> %387, i32 1
  %399 = extractelement <2 x double> %388, i32 1
  %400 = fsub double %398, %399
  %401 = load <2 x double>* %100, align 16
  %402 = load <2 x double>* %103, align 16
  %403 = extractelement <2 x double> %401, i32 0
  %404 = extractelement <2 x double> %402, i32 0
  %405 = fadd double %403, %404
  %406 = extractelement <2 x double> %401, i32 1
  %407 = extractelement <2 x double> %402, i32 1
  %408 = fadd double %406, %407
  %409 = extractelement <2 x double> %401, i32 0
  %410 = extractelement <2 x double> %402, i32 0
  %411 = fsub double %409, %410
  %412 = extractelement <2 x double> %401, i32 1
  %413 = extractelement <2 x double> %402, i32 1
  %414 = fsub double %412, %413
  %415 = fmul double %386, -1.000000e+00
  %416 = fsub double %383, %415
  %417 = fmul double %383, -1.000000e+00
  %418 = fadd double %417, %386
  %419 = fmul double %416, 0x3FE6A09E667F3BCD
  %420 = fmul double %418, 0x3FE6A09E667F3BCD
  %421 = fmul double %397, 0.000000e+00
  %422 = fmul double %400, -1.000000e+00
  %423 = fsub double %421, %422
  %424 = fmul double %397, -1.000000e+00
  %425 = fmul double %400, 0.000000e+00
  %426 = fadd double %424, %425
  %427 = fmul double %411, -1.000000e+00
  %428 = fmul double %414, -1.000000e+00
  %429 = fsub double %427, %428
  %430 = fmul double %411, -1.000000e+00
  %431 = fmul double %414, -1.000000e+00
  %432 = fadd double %430, %431
  %433 = fmul double %429, 0x3FE6A09E667F3BCD
  %434 = fmul double %432, 0x3FE6A09E667F3BCD
  %435 = fadd double %363, %391
  %436 = fadd double %366, %394
  %437 = fsub double %363, %391
  %438 = fsub double %366, %394
  %439 = fadd double %377, %405
  %440 = fadd double %380, %408
  %441 = fsub double %377, %405
  %442 = fsub double %380, %408
  %443 = fmul double %441, 0.000000e+00
  %444 = fmul double %442, -1.000000e+00
  %445 = fsub double %443, %444
  %446 = fmul double %441, -1.000000e+00
  %447 = fmul double %442, 0.000000e+00
  %448 = fadd double %446, %447
  %449 = fadd double %435, %439
  %450 = insertelement <2 x double> undef, double %449, i32 0
  %451 = fadd double %436, %440
  %452 = insertelement <2 x double> %450, double %451, i32 1
  store <2 x double> %452, <2 x double>* %31, align 16
  %453 = fsub double %435, %439
  %454 = insertelement <2 x double> undef, double %453, i32 0
  %455 = fsub double %436, %440
  %456 = insertelement <2 x double> %454, double %455, i32 1
  store <2 x double> %456, <2 x double>* %64, align 16
  %457 = fadd double %437, %445
  %458 = insertelement <2 x double> undef, double %457, i32 0
  %459 = fadd double %438, %448
  %460 = insertelement <2 x double> %458, double %459, i32 1
  store <2 x double> %460, <2 x double>* %82, align 16
  %461 = fsub double %437, %445
  %462 = insertelement <2 x double> undef, double %461, i32 0
  %463 = fsub double %438, %448
  %464 = insertelement <2 x double> %462, double %463, i32 1
  store <2 x double> %464, <2 x double>* %100, align 16
  %465 = fadd double %369, %423
  %466 = fadd double %372, %426
  %467 = fsub double %369, %423
  %468 = fsub double %372, %426
  %469 = fadd double %419, %433
  %470 = fadd double %420, %434
  %471 = fsub double %419, %433
  %472 = fsub double %420, %434
  %473 = fmul double %471, 0.000000e+00
  %474 = fmul double %472, -1.000000e+00
  %475 = fsub double %473, %474
  %476 = fmul double %471, -1.000000e+00
  %477 = fmul double %472, 0.000000e+00
  %478 = fadd double %476, %477
  %479 = fadd double %465, %469
  %480 = insertelement <2 x double> undef, double %479, i32 0
  %481 = fadd double %466, %470
  %482 = insertelement <2 x double> %480, double %481, i32 1
  store <2 x double> %482, <2 x double>* %49, align 16
  %483 = fsub double %465, %469
  %484 = insertelement <2 x double> undef, double %483, i32 0
  %485 = fsub double %466, %470
  %486 = insertelement <2 x double> %484, double %485, i32 1
  store <2 x double> %486, <2 x double>* %67, align 16
  %487 = fadd double %467, %475
  %488 = insertelement <2 x double> undef, double %487, i32 0
  %489 = fadd double %468, %478
  %490 = insertelement <2 x double> %488, double %489, i32 1
  store <2 x double> %490, <2 x double>* %85, align 16
  %491 = fsub double %467, %475
  %492 = insertelement <2 x double> undef, double %491, i32 0
  %493 = fsub double %468, %478
  %494 = insertelement <2 x double> %492, double %493, i32 1
  store <2 x double> %494, <2 x double>* %103, align 16
  %"&(pSB[currWI].offset)41126.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset42.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)41126.i"
  %CastToValueType43.i = bitcast i8* %"&pSB[currWI].offset42.i" to i32*
  %loadedValue44.i = load i32* %CastToValueType43.i, align 4
  %495 = sitofp i32 %loadedValue44.i to double
  %"&(pSB[currWI].offset)110.i" = add nuw i64 %CurrSBIndex..0.i, 48
  %"&pSB[currWI].offset111.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)110.i"
  %CastToValueType112.i = bitcast i8* %"&pSB[currWI].offset111.i" to [8 x <2 x double>]*
  br label %496

; <label>:496                                     ; preds = %._crit_edge12.i, %SyncBB189.i
  %indvar.i = phi i64 [ 1, %SyncBB189.i ], [ %phitmp14.i, %._crit_edge12.i ]
  %scevgep.i = getelementptr [8 x <2 x double>]* %CastToValueType112.i, i64 0, i64 %indvar.i
  %scevgep5.i = getelementptr [8 x i32]* @fft1D_512.reversed8, i64 0, i64 %indvar.i
  %497 = load <2 x double>* %scevgep.i, align 16
  %498 = load i32* %scevgep5.i, align 4
  %499 = sitofp i32 %498 to double
  %500 = fmul double %499, 0xC01921FB54442D18
  %501 = fdiv double %500, 6.400000e+01
  %502 = fmul double %501, %495
  %503 = call double @_Z3cosd(double %502) nounwind
  %504 = call double @_Z3sind(double %502) nounwind
  %505 = extractelement <2 x double> %497, i32 0
  %506 = fmul double %505, %503
  %507 = extractelement <2 x double> %497, i32 1
  %508 = fmul double %507, %504
  %509 = fsub double %506, %508
  %510 = insertelement <2 x double> undef, double %509, i32 0
  %511 = fmul double %505, %504
  %512 = fmul double %507, %503
  %513 = fadd double %511, %512
  %514 = insertelement <2 x double> %510, double %513, i32 1
  store <2 x double> %514, <2 x double>* %scevgep.i, align 16
  %exitcond.i = icmp eq i64 %indvar.i, 7
  br i1 %exitcond.i, label %._crit_edge.i, label %._crit_edge12.i

._crit_edge12.i:                                  ; preds = %496
  %phitmp14.i = add i64 %indvar.i, 1
  br label %496

._crit_edge.i:                                    ; preds = %496
  %"&(pSB[currWI].offset)78127.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset79.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)78127.i"
  %CastToValueType80.i = bitcast i8* %"&pSB[currWI].offset79.i" to double addrspace(3)**
  %loadedValue81.i = load double addrspace(3)** %CastToValueType80.i, align 8
  %515 = load <2 x double>* %31, align 16
  %516 = extractelement <2 x double> %515, i32 0
  store double %516, double addrspace(3)* %loadedValue81.i, align 8
  %"&pSB[currWI].offset143.sum128.i" = add i64 %CurrSBIndex..1.i, 112
  %517 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum128.i"
  %518 = bitcast i8* %517 to <2 x double>*
  %519 = load <2 x double>* %518, align 16
  %520 = extractelement <2 x double> %519, i32 0
  %521 = getelementptr inbounds double addrspace(3)* %loadedValue81.i, i64 72
  store double %520, double addrspace(3)* %521, align 8
  %"&pSB[currWI].offset143.sum129.i" = add i64 %CurrSBIndex..1.i, 80
  %522 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum129.i"
  %523 = bitcast i8* %522 to <2 x double>*
  %524 = load <2 x double>* %523, align 16
  %525 = extractelement <2 x double> %524, i32 0
  %526 = getelementptr inbounds double addrspace(3)* %loadedValue81.i, i64 144
  store double %525, double addrspace(3)* %526, align 8
  %"&pSB[currWI].offset143.sum130.i" = add i64 %CurrSBIndex..1.i, 144
  %527 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum130.i"
  %528 = bitcast i8* %527 to <2 x double>*
  %529 = load <2 x double>* %528, align 16
  %530 = extractelement <2 x double> %529, i32 0
  %531 = getelementptr inbounds double addrspace(3)* %loadedValue81.i, i64 216
  store double %530, double addrspace(3)* %531, align 8
  %"&pSB[currWI].offset143.sum131.i" = add i64 %CurrSBIndex..1.i, 64
  %532 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum131.i"
  %533 = bitcast i8* %532 to <2 x double>*
  %534 = load <2 x double>* %533, align 16
  %535 = extractelement <2 x double> %534, i32 0
  %536 = getelementptr inbounds double addrspace(3)* %loadedValue81.i, i64 288
  store double %535, double addrspace(3)* %536, align 8
  %"&pSB[currWI].offset143.sum132.i" = add i64 %CurrSBIndex..1.i, 128
  %537 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum132.i"
  %538 = bitcast i8* %537 to <2 x double>*
  %539 = load <2 x double>* %538, align 16
  %540 = extractelement <2 x double> %539, i32 0
  %541 = getelementptr inbounds double addrspace(3)* %loadedValue81.i, i64 360
  store double %540, double addrspace(3)* %541, align 8
  %"&pSB[currWI].offset143.sum133.i" = add i64 %CurrSBIndex..1.i, 96
  %542 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum133.i"
  %543 = bitcast i8* %542 to <2 x double>*
  %544 = load <2 x double>* %543, align 16
  %545 = extractelement <2 x double> %544, i32 0
  %546 = getelementptr inbounds double addrspace(3)* %loadedValue81.i, i64 432
  store double %545, double addrspace(3)* %546, align 8
  %"&pSB[currWI].offset143.sum134.i" = add i64 %CurrSBIndex..1.i, 160
  %547 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum134.i"
  %548 = bitcast i8* %547 to <2 x double>*
  %549 = load <2 x double>* %548, align 16
  %550 = extractelement <2 x double> %549, i32 0
  %551 = getelementptr inbounds double addrspace(3)* %loadedValue81.i, i64 504
  store double %550, double addrspace(3)* %551, align 8
  %loadedCurrWI.i = load i64* %19, align 8
  %check.WI.iter.i = icmp ult i64 %loadedCurrWI.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %loadedCurrWI.i, 1
  store i64 %"CurrWI++.i", i64* %19, align 8
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 608
  br label %SyncBB189.i

elseBB.i:                                         ; preds = %._crit_edge.i
  store i64 0, i64* %19, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB223.i, %elseBB.i
  %CurrSBIndex..5.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride229.i", %thenBB223.i ]
  %"&pSB[currWI].offset24.i" = getelementptr inbounds i8* %16, i64 %CurrSBIndex..5.i
  %CastToValueType25.i = bitcast i8* %"&pSB[currWI].offset24.i" to i32*
  %loadedValue.i = load i32* %CastToValueType25.i, align 4
  %552 = and i32 %loadedValue.i, -8
  %553 = mul nsw i32 %552, 9
  %"&(pSB[currWI].offset)50135.i" = or i64 %CurrSBIndex..5.i, 8
  %"&pSB[currWI].offset51.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)50135.i"
  %CastToValueType52.i = bitcast i8* %"&pSB[currWI].offset51.i" to i32*
  %loadedValue53.i = load i32* %CastToValueType52.i, align 4
  %554 = or i32 %553, %loadedValue53.i
  %555 = sext i32 %554 to i64
  %556 = getelementptr inbounds [576 x double] addrspace(3)* %20, i64 0, i64 %555
  %"&(pSB[currWI].offset)97.i" = add nuw i64 %CurrSBIndex..5.i, 40
  %"&pSB[currWI].offset98.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)97.i"
  %CastToValueType99.i = bitcast i8* %"&pSB[currWI].offset98.i" to double addrspace(3)**
  store double addrspace(3)* %556, double addrspace(3)** %CastToValueType99.i, align 8
  %557 = load double addrspace(3)* %556, align 8
  %558 = load <2 x double>* %31, align 16
  %559 = insertelement <2 x double> %558, double %557, i32 0
  store <2 x double> %559, <2 x double>* %31, align 16
  %"&pSB[currWI].offset143.sum136.i" = add i64 %CurrSBIndex..1.i, 64
  %scevgep.1.i23.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum136.i"
  %560 = bitcast i8* %scevgep.1.i23.i to <2 x double>*
  %.sum137.i = add i64 %555, 8
  %scevgep2.1.i24.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum137.i
  %561 = load double addrspace(3)* %scevgep2.1.i24.i, align 8
  %562 = load <2 x double>* %560, align 16
  %563 = insertelement <2 x double> %562, double %561, i32 0
  store <2 x double> %563, <2 x double>* %560, align 16
  %"&pSB[currWI].offset143.sum138.i" = add i64 %CurrSBIndex..1.i, 80
  %scevgep.2.i25.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum138.i"
  %564 = bitcast i8* %scevgep.2.i25.i to <2 x double>*
  %.sum139.i = add i64 %555, 16
  %scevgep2.2.i26.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum139.i
  %565 = load double addrspace(3)* %scevgep2.2.i26.i, align 8
  %566 = load <2 x double>* %564, align 16
  %567 = insertelement <2 x double> %566, double %565, i32 0
  store <2 x double> %567, <2 x double>* %564, align 16
  %"&pSB[currWI].offset143.sum140.i" = add i64 %CurrSBIndex..1.i, 96
  %scevgep.3.i27.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum140.i"
  %568 = bitcast i8* %scevgep.3.i27.i to <2 x double>*
  %.sum141.i = add i64 %555, 24
  %scevgep2.3.i28.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum141.i
  %569 = load double addrspace(3)* %scevgep2.3.i28.i, align 8
  %570 = load <2 x double>* %568, align 16
  %571 = insertelement <2 x double> %570, double %569, i32 0
  store <2 x double> %571, <2 x double>* %568, align 16
  %"&pSB[currWI].offset143.sum142.i" = add i64 %CurrSBIndex..1.i, 112
  %scevgep.4.i29.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum142.i"
  %572 = bitcast i8* %scevgep.4.i29.i to <2 x double>*
  %.sum143.i = add i64 %555, 32
  %scevgep2.4.i30.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum143.i
  %573 = load double addrspace(3)* %scevgep2.4.i30.i, align 8
  %574 = load <2 x double>* %572, align 16
  %575 = insertelement <2 x double> %574, double %573, i32 0
  store <2 x double> %575, <2 x double>* %572, align 16
  %"&pSB[currWI].offset143.sum144.i" = add i64 %CurrSBIndex..1.i, 128
  %scevgep.5.i31.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum144.i"
  %576 = bitcast i8* %scevgep.5.i31.i to <2 x double>*
  %.sum145.i = add i64 %555, 40
  %scevgep2.5.i32.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum145.i
  %577 = load double addrspace(3)* %scevgep2.5.i32.i, align 8
  %578 = load <2 x double>* %576, align 16
  %579 = insertelement <2 x double> %578, double %577, i32 0
  store <2 x double> %579, <2 x double>* %576, align 16
  %"&pSB[currWI].offset143.sum146.i" = add i64 %CurrSBIndex..1.i, 144
  %scevgep.6.i33.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum146.i"
  %580 = bitcast i8* %scevgep.6.i33.i to <2 x double>*
  %.sum147.i = add i64 %555, 48
  %scevgep2.6.i34.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum147.i
  %581 = load double addrspace(3)* %scevgep2.6.i34.i, align 8
  %582 = load <2 x double>* %580, align 16
  %583 = insertelement <2 x double> %582, double %581, i32 0
  store <2 x double> %583, <2 x double>* %580, align 16
  %"&pSB[currWI].offset143.sum148.i" = add i64 %CurrSBIndex..1.i, 160
  %scevgep.7.i35.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum148.i"
  %584 = bitcast i8* %scevgep.7.i35.i to <2 x double>*
  %.sum149.i = add i64 %555, 56
  %scevgep2.7.i36.i = getelementptr [576 x double] addrspace(3)* %20, i64 0, i64 %.sum149.i
  %585 = load double addrspace(3)* %scevgep2.7.i36.i, align 8
  %586 = load <2 x double>* %584, align 16
  %587 = insertelement <2 x double> %586, double %585, i32 0
  store <2 x double> %587, <2 x double>* %584, align 16
  %loadedCurrWI225.i = load i64* %19, align 8
  %check.WI.iter226.i = icmp ult i64 %loadedCurrWI225.i, %13
  br i1 %check.WI.iter226.i, label %thenBB223.i, label %elseBB224.i

thenBB223.i:                                      ; preds = %SyncBB.i
  %"CurrWI++227.i" = add nuw i64 %loadedCurrWI225.i, 1
  store i64 %"CurrWI++227.i", i64* %19, align 8
  %"loadedCurrSB+Stride229.i" = add nuw i64 %CurrSBIndex..5.i, 608
  br label %SyncBB.i

elseBB224.i:                                      ; preds = %SyncBB.i
  store i64 0, i64* %19, align 8
  br label %SyncBB190.i

SyncBB190.i:                                      ; preds = %thenBB230.i, %elseBB224.i
  %CurrSBIndex..6.i = phi i64 [ 0, %elseBB224.i ], [ %"loadedCurrSB+Stride236.i", %thenBB230.i ]
  %"&(pSB[currWI].offset)73150.i" = or i64 %CurrSBIndex..6.i, 24
  %"&pSB[currWI].offset74.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)73150.i"
  %CastToValueType75.i = bitcast i8* %"&pSB[currWI].offset74.i" to double addrspace(3)**
  %loadedValue76.i = load double addrspace(3)** %CastToValueType75.i, align 8
  %588 = load <2 x double>* %31, align 16
  %589 = extractelement <2 x double> %588, i32 1
  store double %589, double addrspace(3)* %loadedValue76.i, align 8
  %"&pSB[currWI].offset143.sum151.i" = add i64 %CurrSBIndex..1.i, 112
  %590 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum151.i"
  %591 = bitcast i8* %590 to <2 x double>*
  %592 = load <2 x double>* %591, align 16
  %593 = extractelement <2 x double> %592, i32 1
  %594 = getelementptr inbounds double addrspace(3)* %loadedValue76.i, i64 72
  store double %593, double addrspace(3)* %594, align 8
  %"&pSB[currWI].offset143.sum152.i" = add i64 %CurrSBIndex..1.i, 80
  %595 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum152.i"
  %596 = bitcast i8* %595 to <2 x double>*
  %597 = load <2 x double>* %596, align 16
  %598 = extractelement <2 x double> %597, i32 1
  %599 = getelementptr inbounds double addrspace(3)* %loadedValue76.i, i64 144
  store double %598, double addrspace(3)* %599, align 8
  %"&pSB[currWI].offset143.sum153.i" = add i64 %CurrSBIndex..1.i, 144
  %600 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum153.i"
  %601 = bitcast i8* %600 to <2 x double>*
  %602 = load <2 x double>* %601, align 16
  %603 = extractelement <2 x double> %602, i32 1
  %604 = getelementptr inbounds double addrspace(3)* %loadedValue76.i, i64 216
  store double %603, double addrspace(3)* %604, align 8
  %"&pSB[currWI].offset143.sum154.i" = add i64 %CurrSBIndex..1.i, 64
  %605 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum154.i"
  %606 = bitcast i8* %605 to <2 x double>*
  %607 = load <2 x double>* %606, align 16
  %608 = extractelement <2 x double> %607, i32 1
  %609 = getelementptr inbounds double addrspace(3)* %loadedValue76.i, i64 288
  store double %608, double addrspace(3)* %609, align 8
  %"&pSB[currWI].offset143.sum155.i" = add i64 %CurrSBIndex..1.i, 128
  %610 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum155.i"
  %611 = bitcast i8* %610 to <2 x double>*
  %612 = load <2 x double>* %611, align 16
  %613 = extractelement <2 x double> %612, i32 1
  %614 = getelementptr inbounds double addrspace(3)* %loadedValue76.i, i64 360
  store double %613, double addrspace(3)* %614, align 8
  %"&pSB[currWI].offset143.sum156.i" = add i64 %CurrSBIndex..1.i, 96
  %615 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum156.i"
  %616 = bitcast i8* %615 to <2 x double>*
  %617 = load <2 x double>* %616, align 16
  %618 = extractelement <2 x double> %617, i32 1
  %619 = getelementptr inbounds double addrspace(3)* %loadedValue76.i, i64 432
  store double %618, double addrspace(3)* %619, align 8
  %"&pSB[currWI].offset143.sum157.i" = add i64 %CurrSBIndex..1.i, 160
  %620 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum157.i"
  %621 = bitcast i8* %620 to <2 x double>*
  %622 = load <2 x double>* %621, align 16
  %623 = extractelement <2 x double> %622, i32 1
  %624 = getelementptr inbounds double addrspace(3)* %loadedValue76.i, i64 504
  store double %623, double addrspace(3)* %624, align 8
  %loadedCurrWI232.i = load i64* %19, align 8
  %check.WI.iter233.i = icmp ult i64 %loadedCurrWI232.i, %13
  br i1 %check.WI.iter233.i, label %thenBB230.i, label %elseBB231.i

thenBB230.i:                                      ; preds = %SyncBB190.i
  %"CurrWI++234.i" = add nuw i64 %loadedCurrWI232.i, 1
  store i64 %"CurrWI++234.i", i64* %19, align 8
  %"loadedCurrSB+Stride236.i" = add nuw i64 %CurrSBIndex..6.i, 608
  br label %SyncBB190.i

elseBB231.i:                                      ; preds = %SyncBB190.i
  store i64 0, i64* %19, align 8
  br label %SyncBB191.i

SyncBB191.i:                                      ; preds = %thenBB237.i, %elseBB231.i
  %CurrSBIndex..7.i = phi i64 [ 0, %elseBB231.i ], [ %"loadedCurrSB+Stride243.i", %thenBB237.i ]
  %"&(pSB[currWI].offset)101.i" = add nuw i64 %CurrSBIndex..7.i, 40
  %"&pSB[currWI].offset102.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)101.i"
  %CastToValueType103.i = bitcast i8* %"&pSB[currWI].offset102.i" to double addrspace(3)**
  %loadedValue104.i = load double addrspace(3)** %CastToValueType103.i, align 8
  %625 = load double addrspace(3)* %loadedValue104.i, align 8
  %626 = load <2 x double>* %31, align 16
  %627 = insertelement <2 x double> %626, double %625, i32 1
  store <2 x double> %627, <2 x double>* %31, align 16
  %"&pSB[currWI].offset143.sum158.i" = add i64 %CurrSBIndex..1.i, 64
  %scevgep.1.i9.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum158.i"
  %628 = bitcast i8* %scevgep.1.i9.i to <2 x double>*
  %scevgep2.1.i10.i = getelementptr double addrspace(3)* %loadedValue104.i, i64 8
  %629 = load double addrspace(3)* %scevgep2.1.i10.i, align 8
  %630 = load <2 x double>* %628, align 16
  %631 = insertelement <2 x double> %630, double %629, i32 1
  store <2 x double> %631, <2 x double>* %628, align 16
  %"&pSB[currWI].offset143.sum159.i" = add i64 %CurrSBIndex..1.i, 80
  %scevgep.2.i11.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum159.i"
  %632 = bitcast i8* %scevgep.2.i11.i to <2 x double>*
  %scevgep2.2.i12.i = getelementptr double addrspace(3)* %loadedValue104.i, i64 16
  %633 = load double addrspace(3)* %scevgep2.2.i12.i, align 8
  %634 = load <2 x double>* %632, align 16
  %635 = insertelement <2 x double> %634, double %633, i32 1
  store <2 x double> %635, <2 x double>* %632, align 16
  %"&pSB[currWI].offset143.sum160.i" = add i64 %CurrSBIndex..1.i, 96
  %scevgep.3.i13.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum160.i"
  %636 = bitcast i8* %scevgep.3.i13.i to <2 x double>*
  %scevgep2.3.i14.i = getelementptr double addrspace(3)* %loadedValue104.i, i64 24
  %637 = load double addrspace(3)* %scevgep2.3.i14.i, align 8
  %638 = load <2 x double>* %636, align 16
  %639 = insertelement <2 x double> %638, double %637, i32 1
  store <2 x double> %639, <2 x double>* %636, align 16
  %"&pSB[currWI].offset143.sum161.i" = add i64 %CurrSBIndex..1.i, 112
  %scevgep.4.i15.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum161.i"
  %640 = bitcast i8* %scevgep.4.i15.i to <2 x double>*
  %scevgep2.4.i16.i = getelementptr double addrspace(3)* %loadedValue104.i, i64 32
  %641 = load double addrspace(3)* %scevgep2.4.i16.i, align 8
  %642 = load <2 x double>* %640, align 16
  %643 = insertelement <2 x double> %642, double %641, i32 1
  store <2 x double> %643, <2 x double>* %640, align 16
  %"&pSB[currWI].offset143.sum162.i" = add i64 %CurrSBIndex..1.i, 128
  %scevgep.5.i17.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum162.i"
  %644 = bitcast i8* %scevgep.5.i17.i to <2 x double>*
  %scevgep2.5.i18.i = getelementptr double addrspace(3)* %loadedValue104.i, i64 40
  %645 = load double addrspace(3)* %scevgep2.5.i18.i, align 8
  %646 = load <2 x double>* %644, align 16
  %647 = insertelement <2 x double> %646, double %645, i32 1
  store <2 x double> %647, <2 x double>* %644, align 16
  %"&pSB[currWI].offset143.sum163.i" = add i64 %CurrSBIndex..1.i, 144
  %scevgep.6.i19.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum163.i"
  %648 = bitcast i8* %scevgep.6.i19.i to <2 x double>*
  %scevgep2.6.i20.i = getelementptr double addrspace(3)* %loadedValue104.i, i64 48
  %649 = load double addrspace(3)* %scevgep2.6.i20.i, align 8
  %650 = load <2 x double>* %648, align 16
  %651 = insertelement <2 x double> %650, double %649, i32 1
  store <2 x double> %651, <2 x double>* %648, align 16
  %"&pSB[currWI].offset143.sum164.i" = add i64 %CurrSBIndex..1.i, 160
  %scevgep.7.i21.i = getelementptr i8* %16, i64 %"&pSB[currWI].offset143.sum164.i"
  %652 = bitcast i8* %scevgep.7.i21.i to <2 x double>*
  %scevgep2.7.i22.i = getelementptr double addrspace(3)* %loadedValue104.i, i64 56
  %653 = load double addrspace(3)* %scevgep2.7.i22.i, align 8
  %654 = load <2 x double>* %652, align 16
  %655 = insertelement <2 x double> %654, double %653, i32 1
  store <2 x double> %655, <2 x double>* %652, align 16
  %656 = load <2 x double>* %31, align 16
  %657 = load <2 x double>* %49, align 16
  %658 = extractelement <2 x double> %656, i32 0
  %659 = extractelement <2 x double> %657, i32 0
  %660 = fadd double %658, %659
  %661 = extractelement <2 x double> %656, i32 1
  %662 = extractelement <2 x double> %657, i32 1
  %663 = fadd double %661, %662
  %664 = extractelement <2 x double> %656, i32 0
  %665 = extractelement <2 x double> %657, i32 0
  %666 = fsub double %664, %665
  %667 = extractelement <2 x double> %656, i32 1
  %668 = extractelement <2 x double> %657, i32 1
  %669 = fsub double %667, %668
  %670 = load <2 x double>* %64, align 16
  %671 = load <2 x double>* %67, align 16
  %672 = extractelement <2 x double> %670, i32 0
  %673 = extractelement <2 x double> %671, i32 0
  %674 = fadd double %672, %673
  %675 = extractelement <2 x double> %670, i32 1
  %676 = extractelement <2 x double> %671, i32 1
  %677 = fadd double %675, %676
  %678 = extractelement <2 x double> %670, i32 0
  %679 = extractelement <2 x double> %671, i32 0
  %680 = fsub double %678, %679
  %681 = extractelement <2 x double> %670, i32 1
  %682 = extractelement <2 x double> %671, i32 1
  %683 = fsub double %681, %682
  %684 = load <2 x double>* %82, align 16
  %685 = load <2 x double>* %85, align 16
  %686 = extractelement <2 x double> %684, i32 0
  %687 = extractelement <2 x double> %685, i32 0
  %688 = fadd double %686, %687
  %689 = extractelement <2 x double> %684, i32 1
  %690 = extractelement <2 x double> %685, i32 1
  %691 = fadd double %689, %690
  %692 = extractelement <2 x double> %684, i32 0
  %693 = extractelement <2 x double> %685, i32 0
  %694 = fsub double %692, %693
  %695 = extractelement <2 x double> %684, i32 1
  %696 = extractelement <2 x double> %685, i32 1
  %697 = fsub double %695, %696
  %698 = load <2 x double>* %100, align 16
  %699 = load <2 x double>* %103, align 16
  %700 = extractelement <2 x double> %698, i32 0
  %701 = extractelement <2 x double> %699, i32 0
  %702 = fadd double %700, %701
  %703 = extractelement <2 x double> %698, i32 1
  %704 = extractelement <2 x double> %699, i32 1
  %705 = fadd double %703, %704
  %706 = extractelement <2 x double> %698, i32 0
  %707 = extractelement <2 x double> %699, i32 0
  %708 = fsub double %706, %707
  %709 = extractelement <2 x double> %698, i32 1
  %710 = extractelement <2 x double> %699, i32 1
  %711 = fsub double %709, %710
  %712 = fmul double %683, -1.000000e+00
  %713 = fsub double %680, %712
  %714 = fmul double %680, -1.000000e+00
  %715 = fadd double %714, %683
  %716 = fmul double %713, 0x3FE6A09E667F3BCD
  %717 = fmul double %715, 0x3FE6A09E667F3BCD
  %718 = fmul double %694, 0.000000e+00
  %719 = fmul double %697, -1.000000e+00
  %720 = fsub double %718, %719
  %721 = fmul double %694, -1.000000e+00
  %722 = fmul double %697, 0.000000e+00
  %723 = fadd double %721, %722
  %724 = fmul double %708, -1.000000e+00
  %725 = fmul double %711, -1.000000e+00
  %726 = fsub double %724, %725
  %727 = fmul double %708, -1.000000e+00
  %728 = fmul double %711, -1.000000e+00
  %729 = fadd double %727, %728
  %730 = fmul double %726, 0x3FE6A09E667F3BCD
  %731 = fmul double %729, 0x3FE6A09E667F3BCD
  %732 = fadd double %660, %688
  %733 = fadd double %663, %691
  %734 = fsub double %660, %688
  %735 = fsub double %663, %691
  %736 = fadd double %674, %702
  %737 = fadd double %677, %705
  %738 = fsub double %674, %702
  %739 = fsub double %677, %705
  %740 = fmul double %738, 0.000000e+00
  %741 = fmul double %739, -1.000000e+00
  %742 = fsub double %740, %741
  %743 = fmul double %738, -1.000000e+00
  %744 = fmul double %739, 0.000000e+00
  %745 = fadd double %743, %744
  %746 = fadd double %732, %736
  %747 = insertelement <2 x double> undef, double %746, i32 0
  %748 = fadd double %733, %737
  %749 = insertelement <2 x double> %747, double %748, i32 1
  store <2 x double> %749, <2 x double>* %31, align 16
  %750 = fsub double %732, %736
  %751 = insertelement <2 x double> undef, double %750, i32 0
  %752 = fsub double %733, %737
  %753 = insertelement <2 x double> %751, double %752, i32 1
  store <2 x double> %753, <2 x double>* %64, align 16
  %754 = fadd double %734, %742
  %755 = insertelement <2 x double> undef, double %754, i32 0
  %756 = fadd double %735, %745
  %757 = insertelement <2 x double> %755, double %756, i32 1
  store <2 x double> %757, <2 x double>* %82, align 16
  %758 = fsub double %734, %742
  %759 = insertelement <2 x double> undef, double %758, i32 0
  %760 = fsub double %735, %745
  %761 = insertelement <2 x double> %759, double %760, i32 1
  store <2 x double> %761, <2 x double>* %100, align 16
  %762 = fadd double %666, %720
  %763 = fadd double %669, %723
  %764 = fsub double %666, %720
  %765 = fsub double %669, %723
  %766 = fadd double %716, %730
  %767 = fadd double %717, %731
  %768 = fsub double %716, %730
  %769 = fsub double %717, %731
  %770 = fmul double %768, 0.000000e+00
  %771 = fmul double %769, -1.000000e+00
  %772 = fsub double %770, %771
  %773 = fmul double %768, -1.000000e+00
  %774 = fmul double %769, 0.000000e+00
  %775 = fadd double %773, %774
  %776 = fadd double %762, %766
  %777 = insertelement <2 x double> undef, double %776, i32 0
  %778 = fadd double %763, %767
  %779 = insertelement <2 x double> %777, double %778, i32 1
  store <2 x double> %779, <2 x double>* %49, align 16
  %780 = fsub double %762, %766
  %781 = insertelement <2 x double> undef, double %780, i32 0
  %782 = fsub double %763, %767
  %783 = insertelement <2 x double> %781, double %782, i32 1
  store <2 x double> %783, <2 x double>* %67, align 16
  %784 = fadd double %764, %772
  %785 = insertelement <2 x double> undef, double %784, i32 0
  %786 = fadd double %765, %775
  %787 = insertelement <2 x double> %785, double %786, i32 1
  store <2 x double> %787, <2 x double>* %85, align 16
  %788 = fsub double %764, %772
  %789 = insertelement <2 x double> undef, double %788, i32 0
  %790 = fsub double %765, %775
  %791 = insertelement <2 x double> %789, double %790, i32 1
  store <2 x double> %791, <2 x double>* %103, align 16
  %"&(pSB[currWI].offset)64165.i" = or i64 %CurrSBIndex..7.i, 16
  %"&pSB[currWI].offset65.i" = getelementptr inbounds i8* %16, i64 %"&(pSB[currWI].offset)64165.i"
  %CastToValueType66.i = bitcast i8* %"&pSB[currWI].offset65.i" to <2 x double> addrspace(1)**
  %loadedValue67.i = load <2 x double> addrspace(1)** %CastToValueType66.i, align 8
  %792 = load <2 x double>* %31, align 16
  store <2 x double> %792, <2 x double> addrspace(1)* %loadedValue67.i, align 16
  %scevgep2.1.i2.i = getelementptr <2 x double> addrspace(1)* %loadedValue67.i, i64 64
  %"&pSB[currWI].offset143.sum166.i" = add i64 %CurrSBIndex..1.i, 112
  %793 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum166.i"
  %794 = bitcast i8* %793 to <2 x double>*
  %795 = load <2 x double>* %794, align 16
  store <2 x double> %795, <2 x double> addrspace(1)* %scevgep2.1.i2.i, align 16
  %scevgep2.2.i3.i = getelementptr <2 x double> addrspace(1)* %loadedValue67.i, i64 128
  %"&pSB[currWI].offset143.sum167.i" = add i64 %CurrSBIndex..1.i, 80
  %796 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum167.i"
  %797 = bitcast i8* %796 to <2 x double>*
  %798 = load <2 x double>* %797, align 16
  store <2 x double> %798, <2 x double> addrspace(1)* %scevgep2.2.i3.i, align 16
  %scevgep2.3.i4.i = getelementptr <2 x double> addrspace(1)* %loadedValue67.i, i64 192
  %"&pSB[currWI].offset143.sum168.i" = add i64 %CurrSBIndex..1.i, 144
  %799 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum168.i"
  %800 = bitcast i8* %799 to <2 x double>*
  %801 = load <2 x double>* %800, align 16
  store <2 x double> %801, <2 x double> addrspace(1)* %scevgep2.3.i4.i, align 16
  %scevgep2.4.i5.i = getelementptr <2 x double> addrspace(1)* %loadedValue67.i, i64 256
  %"&pSB[currWI].offset143.sum169.i" = add i64 %CurrSBIndex..1.i, 64
  %802 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum169.i"
  %803 = bitcast i8* %802 to <2 x double>*
  %804 = load <2 x double>* %803, align 16
  store <2 x double> %804, <2 x double> addrspace(1)* %scevgep2.4.i5.i, align 16
  %scevgep2.5.i6.i = getelementptr <2 x double> addrspace(1)* %loadedValue67.i, i64 320
  %"&pSB[currWI].offset143.sum170.i" = add i64 %CurrSBIndex..1.i, 128
  %805 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum170.i"
  %806 = bitcast i8* %805 to <2 x double>*
  %807 = load <2 x double>* %806, align 16
  store <2 x double> %807, <2 x double> addrspace(1)* %scevgep2.5.i6.i, align 16
  %scevgep2.6.i7.i = getelementptr <2 x double> addrspace(1)* %loadedValue67.i, i64 384
  %"&pSB[currWI].offset143.sum171.i" = add i64 %CurrSBIndex..1.i, 96
  %808 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum171.i"
  %809 = bitcast i8* %808 to <2 x double>*
  %810 = load <2 x double>* %809, align 16
  store <2 x double> %810, <2 x double> addrspace(1)* %scevgep2.6.i7.i, align 16
  %scevgep2.7.i8.i = getelementptr <2 x double> addrspace(1)* %loadedValue67.i, i64 448
  %"&pSB[currWI].offset143.sum172.i" = add i64 %CurrSBIndex..1.i, 160
  %811 = getelementptr inbounds i8* %16, i64 %"&pSB[currWI].offset143.sum172.i"
  %812 = bitcast i8* %811 to <2 x double>*
  %813 = load <2 x double>* %812, align 16
  store <2 x double> %813, <2 x double> addrspace(1)* %scevgep2.7.i8.i, align 16
  %loadedCurrWI239.i = load i64* %19, align 8
  %check.WI.iter240.i = icmp ult i64 %loadedCurrWI239.i, %13
  br i1 %check.WI.iter240.i, label %thenBB237.i, label %__fft1D_512_separated_args.exit

thenBB237.i:                                      ; preds = %SyncBB191.i
  %"CurrWI++241.i" = add nuw i64 %loadedCurrWI239.i, 1
  store i64 %"CurrWI++241.i", i64* %19, align 8
  %"loadedCurrSB+Stride243.i" = add nuw i64 %CurrSBIndex..7.i, 608
  br label %SyncBB191.i

__fft1D_512_separated_args.exit:                  ; preds = %SyncBB191.i
  store i64 0, i64* %19, align 8
  ret void
}

define void @chk1D_512(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <2 x double> addrspace(1)**
  %1 = load <2 x double> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to i64**
  %10 = load i64** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 72
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i8**
  %19 = load i8** %18, align 8
  %20 = sext i32 %4 to i64
  %tmp11.1.i = add i32 %4, 64
  %21 = sext i32 %tmp11.1.i to i64
  %tmp11.2.i = add i32 %4, 128
  %22 = sext i32 %tmp11.2.i to i64
  %tmp11.3.i = add i32 %4, 192
  %23 = sext i32 %tmp11.3.i to i64
  %tmp11.4.i = add i32 %4, 256
  %24 = sext i32 %tmp11.4.i to i64
  %tmp11.5.i = add i32 %4, 320
  %25 = sext i32 %tmp11.5.i to i64
  %tmp11.6.i = add i32 %4, 384
  %26 = sext i32 %tmp11.6.i to i64
  %tmp11.7.i = add i32 %4, 448
  %27 = sext i32 %tmp11.7.i to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %28 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %29 = load i64* %28, align 8
  %30 = load i64* %10, align 8
  %31 = shl i64 %30, 9
  %32 = add i64 %31, %29
  %sext1.i = shl i64 %32, 32
  %33 = ashr i64 %sext1.i, 32
  %tmp16.i = add i64 %29, %31
  %sext.i = shl i64 %tmp16.i, 32
  %tmp18.i = ashr i64 %sext.i, 32
  %"&(pSB[currWI].offset)46.i" = add nuw i64 %CurrSBIndex..0.i, 352
  %"&pSB[currWI].offset47.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)46.i"
  %scevgep14.i = bitcast i8* %"&pSB[currWI].offset47.i" to <2 x double>*
  %scevgep20.i = getelementptr <2 x double> addrspace(1)* %1, i64 %tmp18.i
  %34 = load <2 x double> addrspace(1)* %scevgep20.i, align 16
  store <2 x double> %34, <2 x double>* %scevgep14.i, align 16
  %"&pSB[currWI].offset43.sum.i" = add i64 %CurrSBIndex..0.i, 368
  %scevgep14.1.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset43.sum.i"
  %35 = bitcast i8* %scevgep14.1.i to <2 x double>*
  %tmp19.1.i = add i64 %tmp18.i, 64
  %scevgep20.1.i = getelementptr <2 x double> addrspace(1)* %1, i64 %tmp19.1.i
  %36 = load <2 x double> addrspace(1)* %scevgep20.1.i, align 16
  store <2 x double> %36, <2 x double>* %35, align 16
  %"&pSB[currWI].offset39.sum.i" = add i64 %CurrSBIndex..0.i, 384
  %scevgep14.2.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset39.sum.i"
  %37 = bitcast i8* %scevgep14.2.i to <2 x double>*
  %tmp19.2.i = add i64 %tmp18.i, 128
  %scevgep20.2.i = getelementptr <2 x double> addrspace(1)* %1, i64 %tmp19.2.i
  %38 = load <2 x double> addrspace(1)* %scevgep20.2.i, align 16
  store <2 x double> %38, <2 x double>* %37, align 16
  %"&pSB[currWI].offset35.sum.i" = add i64 %CurrSBIndex..0.i, 400
  %scevgep14.3.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset35.sum.i"
  %39 = bitcast i8* %scevgep14.3.i to <2 x double>*
  %tmp19.3.i = add i64 %tmp18.i, 192
  %scevgep20.3.i = getelementptr <2 x double> addrspace(1)* %1, i64 %tmp19.3.i
  %40 = load <2 x double> addrspace(1)* %scevgep20.3.i, align 16
  store <2 x double> %40, <2 x double>* %39, align 16
  %"&pSB[currWI].offset31.sum.i" = add i64 %CurrSBIndex..0.i, 416
  %scevgep14.4.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset31.sum.i"
  %41 = bitcast i8* %scevgep14.4.i to <2 x double>*
  %tmp19.4.i = add i64 %tmp18.i, 256
  %scevgep20.4.i = getelementptr <2 x double> addrspace(1)* %1, i64 %tmp19.4.i
  %42 = load <2 x double> addrspace(1)* %scevgep20.4.i, align 16
  store <2 x double> %42, <2 x double>* %41, align 16
  %"&pSB[currWI].offset27.sum.i" = add i64 %CurrSBIndex..0.i, 432
  %scevgep14.5.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset27.sum.i"
  %43 = bitcast i8* %scevgep14.5.i to <2 x double>*
  %tmp19.5.i = add i64 %tmp18.i, 320
  %scevgep20.5.i = getelementptr <2 x double> addrspace(1)* %1, i64 %tmp19.5.i
  %44 = load <2 x double> addrspace(1)* %scevgep20.5.i, align 16
  store <2 x double> %44, <2 x double>* %43, align 16
  %"&pSB[currWI].offset23.sum.i" = add i64 %CurrSBIndex..0.i, 448
  %scevgep14.6.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset23.sum.i"
  %45 = bitcast i8* %scevgep14.6.i to <2 x double>*
  %tmp19.6.i = add i64 %tmp18.i, 384
  %scevgep20.6.i = getelementptr <2 x double> addrspace(1)* %1, i64 %tmp19.6.i
  %46 = load <2 x double> addrspace(1)* %scevgep20.6.i, align 16
  store <2 x double> %46, <2 x double>* %45, align 16
  %"&pSB[currWI].offset.sum.i" = add i64 %CurrSBIndex..0.i, 464
  %scevgep14.7.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset.sum.i"
  %47 = bitcast i8* %scevgep14.7.i to <2 x double>*
  %tmp19.7.i = add i64 %tmp18.i, 448
  %scevgep20.7.i = getelementptr <2 x double> addrspace(1)* %1, i64 %tmp19.7.i
  %48 = load <2 x double> addrspace(1)* %scevgep20.7.i, align 16
  store <2 x double> %48, <2 x double>* %47, align 16
  %"&(pSB[currWI].offset)78.i" = add nuw i64 %CurrSBIndex..0.i, 480
  %"&pSB[currWI].offset79.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)78.i"
  %scevgep13.i = bitcast i8* %"&pSB[currWI].offset79.i" to <2 x double>*
  %.sum.i = add i64 %20, %33
  %49 = getelementptr inbounds <2 x double> addrspace(1)* %1, i64 %.sum.i
  %50 = load <2 x double> addrspace(1)* %49, align 16
  store <2 x double> %50, <2 x double>* %scevgep13.i, align 16
  %"&pSB[currWI].offset75.sum.i" = add i64 %CurrSBIndex..0.i, 496
  %scevgep13.1.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset75.sum.i"
  %51 = bitcast i8* %scevgep13.1.i to <2 x double>*
  %.sum.1.i = add i64 %21, %33
  %52 = getelementptr inbounds <2 x double> addrspace(1)* %1, i64 %.sum.1.i
  %53 = load <2 x double> addrspace(1)* %52, align 16
  store <2 x double> %53, <2 x double>* %51, align 16
  %"&pSB[currWI].offset71.sum.i" = add i64 %CurrSBIndex..0.i, 512
  %scevgep13.2.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset71.sum.i"
  %54 = bitcast i8* %scevgep13.2.i to <2 x double>*
  %.sum.2.i = add i64 %22, %33
  %55 = getelementptr inbounds <2 x double> addrspace(1)* %1, i64 %.sum.2.i
  %56 = load <2 x double> addrspace(1)* %55, align 16
  store <2 x double> %56, <2 x double>* %54, align 16
  %"&pSB[currWI].offset67.sum.i" = add i64 %CurrSBIndex..0.i, 528
  %scevgep13.3.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset67.sum.i"
  %57 = bitcast i8* %scevgep13.3.i to <2 x double>*
  %.sum.3.i = add i64 %23, %33
  %58 = getelementptr inbounds <2 x double> addrspace(1)* %1, i64 %.sum.3.i
  %59 = load <2 x double> addrspace(1)* %58, align 16
  store <2 x double> %59, <2 x double>* %57, align 16
  %"&pSB[currWI].offset63.sum.i" = add i64 %CurrSBIndex..0.i, 544
  %scevgep13.4.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset63.sum.i"
  %60 = bitcast i8* %scevgep13.4.i to <2 x double>*
  %.sum.4.i = add i64 %24, %33
  %61 = getelementptr inbounds <2 x double> addrspace(1)* %1, i64 %.sum.4.i
  %62 = load <2 x double> addrspace(1)* %61, align 16
  store <2 x double> %62, <2 x double>* %60, align 16
  %"&pSB[currWI].offset59.sum.i" = add i64 %CurrSBIndex..0.i, 560
  %scevgep13.5.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset59.sum.i"
  %63 = bitcast i8* %scevgep13.5.i to <2 x double>*
  %.sum.5.i = add i64 %25, %33
  %64 = getelementptr inbounds <2 x double> addrspace(1)* %1, i64 %.sum.5.i
  %65 = load <2 x double> addrspace(1)* %64, align 16
  store <2 x double> %65, <2 x double>* %63, align 16
  %"&pSB[currWI].offset55.sum.i" = add i64 %CurrSBIndex..0.i, 576
  %scevgep13.6.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset55.sum.i"
  %66 = bitcast i8* %scevgep13.6.i to <2 x double>*
  %.sum.6.i = add i64 %26, %33
  %67 = getelementptr inbounds <2 x double> addrspace(1)* %1, i64 %.sum.6.i
  %68 = load <2 x double> addrspace(1)* %67, align 16
  store <2 x double> %68, <2 x double>* %66, align 16
  %"&pSB[currWI].offset51.sum.i" = add i64 %CurrSBIndex..0.i, 592
  %scevgep13.7.i = getelementptr i8* %19, i64 %"&pSB[currWI].offset51.sum.i"
  %69 = bitcast i8* %scevgep13.7.i to <2 x double>*
  %.sum.7.i = add i64 %27, %33
  %70 = getelementptr inbounds <2 x double> addrspace(1)* %1, i64 %.sum.7.i
  %71 = load <2 x double> addrspace(1)* %70, align 16
  store <2 x double> %71, <2 x double>* %69, align 16
  %72 = extractelement <2 x double> %34, i32 0
  %73 = extractelement <2 x double> %50, i32 0
  %74 = fcmp une double %72, %73
  br i1 %74, label %79, label %75

; <label>:75                                      ; preds = %SyncBB.i
  %76 = extractelement <2 x double> %34, i32 1
  %77 = extractelement <2 x double> %50, i32 1
  %78 = fcmp une double %76, %77
  br i1 %78, label %79, label %80

; <label>:79                                      ; preds = %75, %SyncBB.i
  store i32 1, i32 addrspace(1)* %7, align 4
  br label %80

; <label>:80                                      ; preds = %79, %75
  %81 = extractelement <2 x double> %36, i32 0
  %82 = extractelement <2 x double> %53, i32 0
  %83 = fcmp une double %81, %82
  br i1 %83, label %88, label %89

; <label>:84                                      ; preds = %89, %88
  %85 = extractelement <2 x double> %38, i32 0
  %86 = extractelement <2 x double> %56, i32 0
  %87 = fcmp une double %85, %86
  br i1 %87, label %97, label %98

; <label>:88                                      ; preds = %89, %80
  store i32 1, i32 addrspace(1)* %7, align 4
  br label %84

; <label>:89                                      ; preds = %80
  %90 = extractelement <2 x double> %36, i32 1
  %91 = extractelement <2 x double> %53, i32 1
  %92 = fcmp une double %90, %91
  br i1 %92, label %88, label %84

; <label>:93                                      ; preds = %98, %97
  %94 = extractelement <2 x double> %40, i32 0
  %95 = extractelement <2 x double> %59, i32 0
  %96 = fcmp une double %94, %95
  br i1 %96, label %106, label %107

; <label>:97                                      ; preds = %98, %84
  store i32 1, i32 addrspace(1)* %7, align 4
  br label %93

; <label>:98                                      ; preds = %84
  %99 = extractelement <2 x double> %38, i32 1
  %100 = extractelement <2 x double> %56, i32 1
  %101 = fcmp une double %99, %100
  br i1 %101, label %97, label %93

; <label>:102                                     ; preds = %107, %106
  %103 = extractelement <2 x double> %42, i32 0
  %104 = extractelement <2 x double> %62, i32 0
  %105 = fcmp une double %103, %104
  br i1 %105, label %115, label %116

; <label>:106                                     ; preds = %107, %93
  store i32 1, i32 addrspace(1)* %7, align 4
  br label %102

; <label>:107                                     ; preds = %93
  %108 = extractelement <2 x double> %40, i32 1
  %109 = extractelement <2 x double> %59, i32 1
  %110 = fcmp une double %108, %109
  br i1 %110, label %106, label %102

; <label>:111                                     ; preds = %116, %115
  %112 = extractelement <2 x double> %44, i32 0
  %113 = extractelement <2 x double> %65, i32 0
  %114 = fcmp une double %112, %113
  br i1 %114, label %124, label %125

; <label>:115                                     ; preds = %116, %102
  store i32 1, i32 addrspace(1)* %7, align 4
  br label %111

; <label>:116                                     ; preds = %102
  %117 = extractelement <2 x double> %42, i32 1
  %118 = extractelement <2 x double> %62, i32 1
  %119 = fcmp une double %117, %118
  br i1 %119, label %115, label %111

; <label>:120                                     ; preds = %125, %124
  %121 = extractelement <2 x double> %46, i32 0
  %122 = extractelement <2 x double> %68, i32 0
  %123 = fcmp une double %121, %122
  br i1 %123, label %133, label %134

; <label>:124                                     ; preds = %125, %111
  store i32 1, i32 addrspace(1)* %7, align 4
  br label %120

; <label>:125                                     ; preds = %111
  %126 = extractelement <2 x double> %44, i32 1
  %127 = extractelement <2 x double> %65, i32 1
  %128 = fcmp une double %126, %127
  br i1 %128, label %124, label %120

; <label>:129                                     ; preds = %134, %133
  %130 = extractelement <2 x double> %48, i32 0
  %131 = extractelement <2 x double> %71, i32 0
  %132 = fcmp une double %130, %131
  br i1 %132, label %138, label %139

; <label>:133                                     ; preds = %134, %120
  store i32 1, i32 addrspace(1)* %7, align 4
  br label %129

; <label>:134                                     ; preds = %120
  %135 = extractelement <2 x double> %46, i32 1
  %136 = extractelement <2 x double> %68, i32 1
  %137 = fcmp une double %135, %136
  br i1 %137, label %133, label %129

; <label>:138                                     ; preds = %139, %129
  store i32 1, i32 addrspace(1)* %7, align 4
  br label %UnifiedReturnBlock.i

; <label>:139                                     ; preds = %129
  %140 = extractelement <2 x double> %48, i32 1
  %141 = extractelement <2 x double> %71, i32 1
  %142 = fcmp une double %140, %141
  br i1 %142, label %138, label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %139, %138
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__chk1D_512_separated_args.exit

thenBB.i:                                         ; preds = %UnifiedReturnBlock.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 608
  br label %SyncBB.i

__chk1D_512_separated_args.exit:                  ; preds = %UnifiedReturnBlock.i
  ret void
}

!opencl.kernels = !{!0, !2, !3}
!opencl_fft1D_512_locals_anchor = !{!4}
!opencl_ifft1D_512_locals_anchor = !{!5}

!0 = metadata !{void (<2 x double> addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__fft1D_512_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double2 __attribute__((address_space(1))) *", metadata !"opencl_fft1D_512_locals_anchor", void (i8*)* @fft1D_512}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (<2 x double> addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__ifft1D_512_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double2 __attribute__((address_space(1))) *", metadata !"opencl_ifft1D_512_locals_anchor", void (i8*)* @ifft1D_512}
!3 = metadata !{void (<2 x double> addrspace(1)*, i32, i32 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__chk1D_512_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double2 __attribute__((address_space(1))) *, int, int __attribute__((address_space(1))) *", metadata !"opencl_chk1D_512_locals_anchor", void (i8*)* @chk1D_512}
!4 = metadata !{metadata !"opencl_fft1D_512_local_smem"}
!5 = metadata !{metadata !"opencl_ifft1D_512_local_smem"}


