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

declare void @__ratt10_kernel_original(double addrspace(1)* nocapture, double addrspace(1)*, double) nounwind

declare i64 @get_global_id(i32)

declare double @_Z3logd(double)

declare double @_Z3expd(double)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__ratt10_kernel_separated_args(double addrspace(1)* nocapture %T, double addrspace(1)* %RKLOW, double %TCONV, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  br label %SyncBB

SyncBB:                                           ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = getelementptr inbounds double addrspace(1)* %T, i64 %5
  %7 = load double addrspace(1)* %6, align 8
  %8 = fmul double %7, %TCONV
  %9 = tail call double @_Z3logd(double %8) nounwind
  %10 = fmul double %9, 9.000000e-01
  %11 = fsub double 0x404523C4B7549584, %10
  %12 = fdiv double 1.000000e+00, %8
  %13 = fmul double %12, 0x408ABBBF266BA494
  %14 = fadd double %11, %13
  %15 = tail call double @_Z3expd(double %14) nounwind
  %16 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %17 = load i64* %16, align 8
  %18 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %19 = load i64* %18, align 8
  %20 = add i64 %17, %19
  %21 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %20
  store double %15, double addrspace(1)* %21, align 8
  %22 = fmul double %9, 3.420000e+00
  %23 = fsub double 0x404FE5858E49DA3F, %22
  %24 = fmul double %12, 0x40E4B9CA6DC5D639
  %25 = fsub double %23, %24
  %26 = tail call double @_Z3expd(double %25) nounwind
  %27 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %28 = load i64* %27, align 8
  %29 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %30 = load i64* %29, align 8
  %31 = add i64 %28, %30
  %32 = add i64 %31, 110592
  %33 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %32
  store double %26, double addrspace(1)* %33, align 8
  %34 = fmul double %9, 3.740000e+00
  %35 = fsub double 0x40505D9028D78F9E, %34
  %36 = fmul double %12, 0x408E71D1DB445ED5
  %37 = fsub double %35, %36
  %38 = tail call double @_Z3expd(double %37) nounwind
  %39 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %40 = load i64* %39, align 8
  %41 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %42 = load i64* %41, align 8
  %43 = add i64 %40, %42
  %44 = add i64 %43, 221184
  %45 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %44
  store double %38, double addrspace(1)* %45, align 8
  %46 = fmul double %9, 2.570000e+00
  %47 = fsub double 0x404BC7F46D24C689, %46
  %48 = fmul double %12, 0x408668AB85A4F00F
  %49 = fsub double %47, %48
  %50 = tail call double @_Z3expd(double %49) nounwind
  %51 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %52 = load i64* %51, align 8
  %53 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %54 = load i64* %53, align 8
  %55 = add i64 %52, %54
  %56 = add i64 %55, 331776
  %57 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %56
  store double %50, double addrspace(1)* %57, align 8
  %58 = fmul double %9, 3.140000e+00
  %59 = fsub double 0x404FAA9E0CC5E120, %58
  %60 = fmul double %12, 0x408357A6E9FF0CBB
  %61 = fsub double %59, %60
  %62 = tail call double @_Z3expd(double %61) nounwind
  %63 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %64 = load i64* %63, align 8
  %65 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %66 = load i64* %65, align 8
  %67 = add i64 %64, %66
  %68 = add i64 %67, 442368
  %69 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %68
  store double %62, double addrspace(1)* %69, align 8
  %70 = fmul double %9, 5.110000e+00
  %71 = fsub double 0x40533E63EE5181D3, %70
  %72 = fmul double %12, 0x40ABE4A4FF43419E
  %73 = fsub double %71, %72
  %74 = tail call double @_Z3expd(double %73) nounwind
  %75 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %76 = load i64* %75, align 8
  %77 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %78 = load i64* %77, align 8
  %79 = add i64 %76, %78
  %80 = add i64 %79, 552960
  %81 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %80
  store double %74, double addrspace(1)* %81, align 8
  %82 = fmul double %9, 4.800000e+00
  %83 = fsub double 0x4051776CB60BC028, %82
  %84 = fmul double %12, 0x40A5DBC4F3775B81
  %85 = fsub double %83, %84
  %86 = tail call double @_Z3expd(double %85) nounwind
  %87 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %88 = load i64* %87, align 8
  %89 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %90 = load i64* %89, align 8
  %91 = add i64 %88, %90
  %92 = add i64 %91, 663552
  %93 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %92
  store double %86, double addrspace(1)* %93, align 8
  %94 = fmul double %9, 4.760000e+00
  %95 = fsub double 0x4053391C5D2DD880, %94
  %96 = fmul double %12, 0x40932F6509BF9C63
  %97 = fsub double %95, %96
  %98 = tail call double @_Z3expd(double %97) nounwind
  %99 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %100 = load i64* %99, align 8
  %101 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %102 = load i64* %101, align 8
  %103 = add i64 %100, %102
  %104 = add i64 %103, 774144
  %105 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %104
  store double %98, double addrspace(1)* %105, align 8
  %106 = fmul double %9, 9.588000e+00
  %107 = fsub double 0x405BD400B0292817, %106
  %108 = fmul double %12, 2.566405e+03
  %109 = fsub double %107, %108
  %110 = tail call double @_Z3expd(double %109) nounwind
  %111 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %112 = load i64* %111, align 8
  %113 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %114 = load i64* %113, align 8
  %115 = add i64 %112, %114
  %116 = add i64 %115, 884736
  %117 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %116
  store double %110, double addrspace(1)* %117, align 8
  %118 = fmul double %9, 9.670000e+00
  %119 = fsub double 0x405CECD0A2446306, %118
  %120 = fmul double %12, 0x40A87403ED527E52
  %121 = fsub double %119, %120
  %122 = tail call double @_Z3expd(double %121) nounwind
  %123 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %124 = load i64* %123, align 8
  %125 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %126 = load i64* %125, align 8
  %127 = add i64 %124, %126
  %128 = add i64 %127, 995328
  %129 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %128
  store double %122, double addrspace(1)* %129, align 8
  %130 = fmul double %9, 6.400000e-01
  %131 = fsub double 0x4041B7A9A2FC18EB, %130
  %132 = fmul double %12, 0x40D86C7793DD97F6
  %133 = fsub double %131, %132
  %134 = tail call double @_Z3expd(double %133) nounwind
  %135 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %136 = load i64* %135, align 8
  %137 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %138 = load i64* %137, align 8
  %139 = add i64 %136, %138
  %140 = add i64 %139, 1105920
  %141 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %140
  store double %134, double addrspace(1)* %141, align 8
  %142 = fmul double %9, 3.400000e+00
  %143 = fsub double 0x404F8E4E054690DE, %142
  %144 = fmul double %12, 0x40D197A0CE703AFB
  %145 = fsub double %143, %144
  %146 = tail call double @_Z3expd(double %145) nounwind
  %147 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %148 = load i64* %147, align 8
  %149 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %150 = load i64* %149, align 8
  %151 = add i64 %148, %150
  %152 = add i64 %151, 1216512
  %153 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %152
  store double %146, double addrspace(1)* %153, align 8
  %154 = fmul double %9, 7.640000e+00
  %155 = fsub double 0x4057EF6C60E6CAA5, %154
  %156 = fmul double %12, 0x40B76447414A4D2B
  %157 = fsub double %155, %156
  %158 = tail call double @_Z3expd(double %157) nounwind
  %159 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %160 = load i64* %159, align 8
  %161 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %162 = load i64* %161, align 8
  %163 = add i64 %160, %162
  %164 = add i64 %163, 1327104
  %165 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %164
  store double %158, double addrspace(1)* %165, align 8
  %166 = fmul double %9, 3.860000e+00
  %167 = fsub double 0x40515A7F62B6AE7D, %166
  %168 = fmul double %12, 0x409A1AB7A4E7AB75
  %169 = fsub double %167, %168
  %170 = tail call double @_Z3expd(double %169) nounwind
  %171 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %172 = load i64* %171, align 8
  %173 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %174 = load i64* %173, align 8
  %175 = add i64 %172, %174
  %176 = add i64 %175, 1437696
  %177 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %176
  store double %170, double addrspace(1)* %177, align 8
  %178 = fmul double %9, 1.194000e+01
  %179 = fsub double 0x4060E00CB07D0AEE, %178
  %180 = fmul double %12, 0x40B3345381D7DBF5
  %181 = fsub double %179, %180
  %182 = tail call double @_Z3expd(double %181) nounwind
  %183 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %184 = load i64* %183, align 8
  %185 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %186 = load i64* %185, align 8
  %187 = add i64 %184, %186
  %188 = add i64 %187, 1548288
  %189 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %188
  store double %182, double addrspace(1)* %189, align 8
  %190 = fmul double %9, 7.297000e+00
  %191 = fsub double 0x4056DCC43C6FF2D7, %190
  %192 = fmul double %12, 0x40A27A3C970F7B9E
  %193 = fsub double %191, %192
  %194 = tail call double @_Z3expd(double %193) nounwind
  %195 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %196 = load i64* %195, align 8
  %197 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %198 = load i64* %197, align 8
  %199 = add i64 %196, %198
  %200 = add i64 %199, 1658880
  %201 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %200
  store double %194, double addrspace(1)* %201, align 8
  %202 = fmul double %9, 9.310000e+00
  %203 = fsub double 0x405D44CF80DC3372, %202
  %204 = fmul double %12, 0x40E88966ECBFB15B
  %205 = fsub double %203, %204
  %206 = tail call double @_Z3expd(double %205) nounwind
  %207 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %208 = load i64* %207, align 8
  %209 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %210 = load i64* %209, align 8
  %211 = add i64 %208, %210
  %212 = add i64 %211, 1769472
  %213 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %212
  store double %206, double addrspace(1)* %213, align 8
  %214 = fmul double %9, 7.620000e+00
  %215 = fsub double 0x405839046E8F29D4, %214
  %216 = fmul double %12, 0x40AB66D72085B185
  %217 = fsub double %215, %216
  %218 = tail call double @_Z3expd(double %217) nounwind
  %219 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %220 = load i64* %219, align 8
  %221 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %222 = load i64* %221, align 8
  %223 = add i64 %220, %222
  %224 = add i64 %223, 1880064
  %225 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %224
  store double %218, double addrspace(1)* %225, align 8
  %226 = fmul double %9, 7.080000e+00
  %227 = fsub double 0x4057C6061E92923E, %226
  %228 = fmul double %12, 0x40AA4801C044284E
  %229 = fsub double %227, %228
  %230 = tail call double @_Z3expd(double %229) nounwind
  %231 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %232 = load i64* %231, align 8
  %233 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %234 = load i64* %233, align 8
  %235 = add i64 %232, %234
  %236 = add i64 %235, 1990656
  %237 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %236
  store double %230, double addrspace(1)* %237, align 8
  %238 = fmul double %9, 1.200000e+01
  %239 = fsub double 0x40614E16D0917D6B, %238
  %240 = fmul double %12, 0x40A776315F45E0B5
  %241 = fsub double %239, %240
  %242 = tail call double @_Z3expd(double %241) nounwind
  %243 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %244 = load i64* %243, align 8
  %245 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %246 = load i64* %245, align 8
  %247 = add i64 %244, %246
  %248 = add i64 %247, 2101248
  %249 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %248
  store double %242, double addrspace(1)* %249, align 8
  %250 = fmul double %9, 6.660000e+00
  %251 = fsub double 0x40565546441C8F83, %250
  %252 = fmul double %12, 0x40AB850888F861A6
  %253 = fsub double %251, %252
  %254 = tail call double @_Z3expd(double %253) nounwind
  %255 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %256 = load i64* %255, align 8
  %257 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %258 = load i64* %257, align 8
  %259 = add i64 %256, %258
  %260 = add i64 %259, 2211840
  %261 = getelementptr inbounds double addrspace(1)* %RKLOW, i64 %260
  store double %254, double addrspace(1)* %261, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB1

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB1:                                          ; preds = %SyncBB
  ret void
}

define void @ratt10_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double*
  %7 = load double* %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 48
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 72
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %17 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %18 = load i64* %17, align 8
  %19 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %20 = load i64* %19, align 8
  %21 = add i64 %18, %20
  %22 = getelementptr inbounds double addrspace(1)* %1, i64 %21
  %23 = load double addrspace(1)* %22, align 8
  %24 = fmul double %23, %7
  %25 = call double @_Z3logd(double %24) nounwind
  %26 = fmul double %25, 9.000000e-01
  %27 = fsub double 0x404523C4B7549584, %26
  %28 = fdiv double 1.000000e+00, %24
  %29 = fmul double %28, 0x408ABBBF266BA494
  %30 = fadd double %27, %29
  %31 = call double @_Z3expd(double %30) nounwind
  %32 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %33 = load i64* %32, align 8
  %34 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %35 = load i64* %34, align 8
  %36 = add i64 %33, %35
  %37 = getelementptr inbounds double addrspace(1)* %4, i64 %36
  store double %31, double addrspace(1)* %37, align 8
  %38 = fmul double %25, 3.420000e+00
  %39 = fsub double 0x404FE5858E49DA3F, %38
  %40 = fmul double %28, 0x40E4B9CA6DC5D639
  %41 = fsub double %39, %40
  %42 = call double @_Z3expd(double %41) nounwind
  %43 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %44 = load i64* %43, align 8
  %45 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %46 = load i64* %45, align 8
  %47 = add i64 %44, %46
  %48 = add i64 %47, 110592
  %49 = getelementptr inbounds double addrspace(1)* %4, i64 %48
  store double %42, double addrspace(1)* %49, align 8
  %50 = fmul double %25, 3.740000e+00
  %51 = fsub double 0x40505D9028D78F9E, %50
  %52 = fmul double %28, 0x408E71D1DB445ED5
  %53 = fsub double %51, %52
  %54 = call double @_Z3expd(double %53) nounwind
  %55 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %56 = load i64* %55, align 8
  %57 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %58 = load i64* %57, align 8
  %59 = add i64 %56, %58
  %60 = add i64 %59, 221184
  %61 = getelementptr inbounds double addrspace(1)* %4, i64 %60
  store double %54, double addrspace(1)* %61, align 8
  %62 = fmul double %25, 2.570000e+00
  %63 = fsub double 0x404BC7F46D24C689, %62
  %64 = fmul double %28, 0x408668AB85A4F00F
  %65 = fsub double %63, %64
  %66 = call double @_Z3expd(double %65) nounwind
  %67 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %68 = load i64* %67, align 8
  %69 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %70 = load i64* %69, align 8
  %71 = add i64 %68, %70
  %72 = add i64 %71, 331776
  %73 = getelementptr inbounds double addrspace(1)* %4, i64 %72
  store double %66, double addrspace(1)* %73, align 8
  %74 = fmul double %25, 3.140000e+00
  %75 = fsub double 0x404FAA9E0CC5E120, %74
  %76 = fmul double %28, 0x408357A6E9FF0CBB
  %77 = fsub double %75, %76
  %78 = call double @_Z3expd(double %77) nounwind
  %79 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %80 = load i64* %79, align 8
  %81 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %82 = load i64* %81, align 8
  %83 = add i64 %80, %82
  %84 = add i64 %83, 442368
  %85 = getelementptr inbounds double addrspace(1)* %4, i64 %84
  store double %78, double addrspace(1)* %85, align 8
  %86 = fmul double %25, 5.110000e+00
  %87 = fsub double 0x40533E63EE5181D3, %86
  %88 = fmul double %28, 0x40ABE4A4FF43419E
  %89 = fsub double %87, %88
  %90 = call double @_Z3expd(double %89) nounwind
  %91 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %92 = load i64* %91, align 8
  %93 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %94 = load i64* %93, align 8
  %95 = add i64 %92, %94
  %96 = add i64 %95, 552960
  %97 = getelementptr inbounds double addrspace(1)* %4, i64 %96
  store double %90, double addrspace(1)* %97, align 8
  %98 = fmul double %25, 4.800000e+00
  %99 = fsub double 0x4051776CB60BC028, %98
  %100 = fmul double %28, 0x40A5DBC4F3775B81
  %101 = fsub double %99, %100
  %102 = call double @_Z3expd(double %101) nounwind
  %103 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %104 = load i64* %103, align 8
  %105 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %106 = load i64* %105, align 8
  %107 = add i64 %104, %106
  %108 = add i64 %107, 663552
  %109 = getelementptr inbounds double addrspace(1)* %4, i64 %108
  store double %102, double addrspace(1)* %109, align 8
  %110 = fmul double %25, 4.760000e+00
  %111 = fsub double 0x4053391C5D2DD880, %110
  %112 = fmul double %28, 0x40932F6509BF9C63
  %113 = fsub double %111, %112
  %114 = call double @_Z3expd(double %113) nounwind
  %115 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %116 = load i64* %115, align 8
  %117 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %118 = load i64* %117, align 8
  %119 = add i64 %116, %118
  %120 = add i64 %119, 774144
  %121 = getelementptr inbounds double addrspace(1)* %4, i64 %120
  store double %114, double addrspace(1)* %121, align 8
  %122 = fmul double %25, 9.588000e+00
  %123 = fsub double 0x405BD400B0292817, %122
  %124 = fmul double %28, 2.566405e+03
  %125 = fsub double %123, %124
  %126 = call double @_Z3expd(double %125) nounwind
  %127 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %128 = load i64* %127, align 8
  %129 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %130 = load i64* %129, align 8
  %131 = add i64 %128, %130
  %132 = add i64 %131, 884736
  %133 = getelementptr inbounds double addrspace(1)* %4, i64 %132
  store double %126, double addrspace(1)* %133, align 8
  %134 = fmul double %25, 9.670000e+00
  %135 = fsub double 0x405CECD0A2446306, %134
  %136 = fmul double %28, 0x40A87403ED527E52
  %137 = fsub double %135, %136
  %138 = call double @_Z3expd(double %137) nounwind
  %139 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %140 = load i64* %139, align 8
  %141 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %142 = load i64* %141, align 8
  %143 = add i64 %140, %142
  %144 = add i64 %143, 995328
  %145 = getelementptr inbounds double addrspace(1)* %4, i64 %144
  store double %138, double addrspace(1)* %145, align 8
  %146 = fmul double %25, 6.400000e-01
  %147 = fsub double 0x4041B7A9A2FC18EB, %146
  %148 = fmul double %28, 0x40D86C7793DD97F6
  %149 = fsub double %147, %148
  %150 = call double @_Z3expd(double %149) nounwind
  %151 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %152 = load i64* %151, align 8
  %153 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %154 = load i64* %153, align 8
  %155 = add i64 %152, %154
  %156 = add i64 %155, 1105920
  %157 = getelementptr inbounds double addrspace(1)* %4, i64 %156
  store double %150, double addrspace(1)* %157, align 8
  %158 = fmul double %25, 3.400000e+00
  %159 = fsub double 0x404F8E4E054690DE, %158
  %160 = fmul double %28, 0x40D197A0CE703AFB
  %161 = fsub double %159, %160
  %162 = call double @_Z3expd(double %161) nounwind
  %163 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %164 = load i64* %163, align 8
  %165 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %166 = load i64* %165, align 8
  %167 = add i64 %164, %166
  %168 = add i64 %167, 1216512
  %169 = getelementptr inbounds double addrspace(1)* %4, i64 %168
  store double %162, double addrspace(1)* %169, align 8
  %170 = fmul double %25, 7.640000e+00
  %171 = fsub double 0x4057EF6C60E6CAA5, %170
  %172 = fmul double %28, 0x40B76447414A4D2B
  %173 = fsub double %171, %172
  %174 = call double @_Z3expd(double %173) nounwind
  %175 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %176 = load i64* %175, align 8
  %177 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %178 = load i64* %177, align 8
  %179 = add i64 %176, %178
  %180 = add i64 %179, 1327104
  %181 = getelementptr inbounds double addrspace(1)* %4, i64 %180
  store double %174, double addrspace(1)* %181, align 8
  %182 = fmul double %25, 3.860000e+00
  %183 = fsub double 0x40515A7F62B6AE7D, %182
  %184 = fmul double %28, 0x409A1AB7A4E7AB75
  %185 = fsub double %183, %184
  %186 = call double @_Z3expd(double %185) nounwind
  %187 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %188 = load i64* %187, align 8
  %189 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %190 = load i64* %189, align 8
  %191 = add i64 %188, %190
  %192 = add i64 %191, 1437696
  %193 = getelementptr inbounds double addrspace(1)* %4, i64 %192
  store double %186, double addrspace(1)* %193, align 8
  %194 = fmul double %25, 1.194000e+01
  %195 = fsub double 0x4060E00CB07D0AEE, %194
  %196 = fmul double %28, 0x40B3345381D7DBF5
  %197 = fsub double %195, %196
  %198 = call double @_Z3expd(double %197) nounwind
  %199 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %200 = load i64* %199, align 8
  %201 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %202 = load i64* %201, align 8
  %203 = add i64 %200, %202
  %204 = add i64 %203, 1548288
  %205 = getelementptr inbounds double addrspace(1)* %4, i64 %204
  store double %198, double addrspace(1)* %205, align 8
  %206 = fmul double %25, 7.297000e+00
  %207 = fsub double 0x4056DCC43C6FF2D7, %206
  %208 = fmul double %28, 0x40A27A3C970F7B9E
  %209 = fsub double %207, %208
  %210 = call double @_Z3expd(double %209) nounwind
  %211 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %212 = load i64* %211, align 8
  %213 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %214 = load i64* %213, align 8
  %215 = add i64 %212, %214
  %216 = add i64 %215, 1658880
  %217 = getelementptr inbounds double addrspace(1)* %4, i64 %216
  store double %210, double addrspace(1)* %217, align 8
  %218 = fmul double %25, 9.310000e+00
  %219 = fsub double 0x405D44CF80DC3372, %218
  %220 = fmul double %28, 0x40E88966ECBFB15B
  %221 = fsub double %219, %220
  %222 = call double @_Z3expd(double %221) nounwind
  %223 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %224 = load i64* %223, align 8
  %225 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %226 = load i64* %225, align 8
  %227 = add i64 %224, %226
  %228 = add i64 %227, 1769472
  %229 = getelementptr inbounds double addrspace(1)* %4, i64 %228
  store double %222, double addrspace(1)* %229, align 8
  %230 = fmul double %25, 7.620000e+00
  %231 = fsub double 0x405839046E8F29D4, %230
  %232 = fmul double %28, 0x40AB66D72085B185
  %233 = fsub double %231, %232
  %234 = call double @_Z3expd(double %233) nounwind
  %235 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %236 = load i64* %235, align 8
  %237 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %238 = load i64* %237, align 8
  %239 = add i64 %236, %238
  %240 = add i64 %239, 1880064
  %241 = getelementptr inbounds double addrspace(1)* %4, i64 %240
  store double %234, double addrspace(1)* %241, align 8
  %242 = fmul double %25, 7.080000e+00
  %243 = fsub double 0x4057C6061E92923E, %242
  %244 = fmul double %28, 0x40AA4801C044284E
  %245 = fsub double %243, %244
  %246 = call double @_Z3expd(double %245) nounwind
  %247 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %248 = load i64* %247, align 8
  %249 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %250 = load i64* %249, align 8
  %251 = add i64 %248, %250
  %252 = add i64 %251, 1990656
  %253 = getelementptr inbounds double addrspace(1)* %4, i64 %252
  store double %246, double addrspace(1)* %253, align 8
  %254 = fmul double %25, 1.200000e+01
  %255 = fsub double 0x40614E16D0917D6B, %254
  %256 = fmul double %28, 0x40A776315F45E0B5
  %257 = fsub double %255, %256
  %258 = call double @_Z3expd(double %257) nounwind
  %259 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %260 = load i64* %259, align 8
  %261 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %262 = load i64* %261, align 8
  %263 = add i64 %260, %262
  %264 = add i64 %263, 2101248
  %265 = getelementptr inbounds double addrspace(1)* %4, i64 %264
  store double %258, double addrspace(1)* %265, align 8
  %266 = fmul double %25, 6.660000e+00
  %267 = fsub double 0x40565546441C8F83, %266
  %268 = fmul double %28, 0x40AB850888F861A6
  %269 = fsub double %267, %268
  %270 = call double @_Z3expd(double %269) nounwind
  %271 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %272 = load i64* %271, align 8
  %273 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %274 = load i64* %273, align 8
  %275 = add i64 %272, %274
  %276 = add i64 %275, 2211840
  %277 = getelementptr inbounds double addrspace(1)* %4, i64 %276
  store double %270, double addrspace(1)* %277, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__ratt10_kernel_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__ratt10_kernel_separated_args.exit:              ; preds = %SyncBB.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, double addrspace(1)*, double, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__ratt10_kernel_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double const __attribute__((address_space(1))) *, double __attribute__((address_space(1))) *, double", metadata !"opencl_ratt10_kernel_locals_anchor", void (i8*)* @ratt10_kernel}
!1 = metadata !{i32 0, i32 0, i32 0}


