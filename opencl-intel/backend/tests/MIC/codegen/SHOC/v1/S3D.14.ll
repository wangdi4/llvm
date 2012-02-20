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

declare void @__ratt10_kernel_original(float addrspace(1)* nocapture, float addrspace(1)*, float) nounwind

declare i64 @get_global_id(i32)

declare float @_Z3logf(float)

declare double @_Z3expd(double)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__ratt10_kernel_separated_args(float addrspace(1)* nocapture %T, float addrspace(1)* %RKLOW, float %TCONV, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  br label %SyncBB

SyncBB:                                           ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = getelementptr inbounds float addrspace(1)* %T, i64 %5
  %7 = load float addrspace(1)* %6, align 4
  %8 = fmul float %7, %TCONV
  %9 = tail call float @_Z3logf(float %8) nounwind
  %10 = fpext float %9 to double
  %11 = fmul double %10, 9.000000e-01
  %12 = fsub double 0x404523C4B7549584, %11
  %13 = fdiv float 1.000000e+00, %8
  %14 = fpext float %13 to double
  %15 = fmul double %14, 0x408ABBBF266BA494
  %16 = fadd double %12, %15
  %17 = tail call double @_Z3expd(double %16) nounwind
  %18 = fptrunc double %17 to float
  %19 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %20 = load i64* %19, align 8
  %21 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %22 = load i64* %21, align 8
  %23 = add i64 %20, %22
  %24 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %23
  store float %18, float addrspace(1)* %24, align 4
  %25 = fmul double %10, 3.420000e+00
  %26 = fsub double 0x404FE5858E49DA3F, %25
  %27 = fmul double %14, 0x40E4B9CA6DC5D639
  %28 = fsub double %26, %27
  %29 = tail call double @_Z3expd(double %28) nounwind
  %30 = fptrunc double %29 to float
  %31 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %32 = load i64* %31, align 8
  %33 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %34 = load i64* %33, align 8
  %35 = add i64 %32, %34
  %36 = add i64 %35, 110592
  %37 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %36
  store float %30, float addrspace(1)* %37, align 4
  %38 = fmul double %10, 3.740000e+00
  %39 = fsub double 0x40505D9028D78F9E, %38
  %40 = fmul double %14, 0x408E71D1DB445ED5
  %41 = fsub double %39, %40
  %42 = tail call double @_Z3expd(double %41) nounwind
  %43 = fptrunc double %42 to float
  %44 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %45 = load i64* %44, align 8
  %46 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %47 = load i64* %46, align 8
  %48 = add i64 %45, %47
  %49 = add i64 %48, 221184
  %50 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %49
  store float %43, float addrspace(1)* %50, align 4
  %51 = fmul double %10, 2.570000e+00
  %52 = fsub double 0x404BC7F46D24C689, %51
  %53 = fmul double %14, 0x408668AB85A4F00F
  %54 = fsub double %52, %53
  %55 = tail call double @_Z3expd(double %54) nounwind
  %56 = fptrunc double %55 to float
  %57 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %58 = load i64* %57, align 8
  %59 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %60 = load i64* %59, align 8
  %61 = add i64 %58, %60
  %62 = add i64 %61, 331776
  %63 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %62
  store float %56, float addrspace(1)* %63, align 4
  %64 = fmul double %10, 3.140000e+00
  %65 = fsub double 0x404FAA9E0CC5E120, %64
  %66 = fmul double %14, 0x408357A6E9FF0CBB
  %67 = fsub double %65, %66
  %68 = tail call double @_Z3expd(double %67) nounwind
  %69 = fptrunc double %68 to float
  %70 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %71 = load i64* %70, align 8
  %72 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %73 = load i64* %72, align 8
  %74 = add i64 %71, %73
  %75 = add i64 %74, 442368
  %76 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %75
  store float %69, float addrspace(1)* %76, align 4
  %77 = fmul double %10, 5.110000e+00
  %78 = fsub double 0x40533E63EE5181D3, %77
  %79 = fmul double %14, 0x40ABE4A4FF43419E
  %80 = fsub double %78, %79
  %81 = tail call double @_Z3expd(double %80) nounwind
  %82 = fptrunc double %81 to float
  %83 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %84 = load i64* %83, align 8
  %85 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %86 = load i64* %85, align 8
  %87 = add i64 %84, %86
  %88 = add i64 %87, 552960
  %89 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %88
  store float %82, float addrspace(1)* %89, align 4
  %90 = fmul double %10, 4.800000e+00
  %91 = fsub double 0x4051776CB60BC028, %90
  %92 = fmul double %14, 0x40A5DBC4F3775B81
  %93 = fsub double %91, %92
  %94 = tail call double @_Z3expd(double %93) nounwind
  %95 = fptrunc double %94 to float
  %96 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %97 = load i64* %96, align 8
  %98 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %99 = load i64* %98, align 8
  %100 = add i64 %97, %99
  %101 = add i64 %100, 663552
  %102 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %101
  store float %95, float addrspace(1)* %102, align 4
  %103 = fmul double %10, 4.760000e+00
  %104 = fsub double 0x4053391C5D2DD880, %103
  %105 = fmul double %14, 0x40932F6509BF9C63
  %106 = fsub double %104, %105
  %107 = tail call double @_Z3expd(double %106) nounwind
  %108 = fptrunc double %107 to float
  %109 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %110 = load i64* %109, align 8
  %111 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %112 = load i64* %111, align 8
  %113 = add i64 %110, %112
  %114 = add i64 %113, 774144
  %115 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %114
  store float %108, float addrspace(1)* %115, align 4
  %116 = fmul double %10, 9.588000e+00
  %117 = fsub double 0x405BD400B0292817, %116
  %118 = fmul double %14, 2.566405e+03
  %119 = fsub double %117, %118
  %120 = tail call double @_Z3expd(double %119) nounwind
  %121 = fptrunc double %120 to float
  %122 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %123 = load i64* %122, align 8
  %124 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %125 = load i64* %124, align 8
  %126 = add i64 %123, %125
  %127 = add i64 %126, 884736
  %128 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %127
  store float %121, float addrspace(1)* %128, align 4
  %129 = fmul double %10, 9.670000e+00
  %130 = fsub double 0x405CECD0A2446306, %129
  %131 = fmul double %14, 0x40A87403ED527E52
  %132 = fsub double %130, %131
  %133 = tail call double @_Z3expd(double %132) nounwind
  %134 = fptrunc double %133 to float
  %135 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %136 = load i64* %135, align 8
  %137 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %138 = load i64* %137, align 8
  %139 = add i64 %136, %138
  %140 = add i64 %139, 995328
  %141 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %140
  store float %134, float addrspace(1)* %141, align 4
  %142 = fmul double %10, 6.400000e-01
  %143 = fsub double 0x4041B7A9A2FC18EB, %142
  %144 = fmul double %14, 0x40D86C7793DD97F6
  %145 = fsub double %143, %144
  %146 = tail call double @_Z3expd(double %145) nounwind
  %147 = fptrunc double %146 to float
  %148 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %149 = load i64* %148, align 8
  %150 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %151 = load i64* %150, align 8
  %152 = add i64 %149, %151
  %153 = add i64 %152, 1105920
  %154 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %153
  store float %147, float addrspace(1)* %154, align 4
  %155 = fmul double %10, 3.400000e+00
  %156 = fsub double 0x404F8E4E054690DE, %155
  %157 = fmul double %14, 0x40D197A0CE703AFB
  %158 = fsub double %156, %157
  %159 = tail call double @_Z3expd(double %158) nounwind
  %160 = fptrunc double %159 to float
  %161 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %162 = load i64* %161, align 8
  %163 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %164 = load i64* %163, align 8
  %165 = add i64 %162, %164
  %166 = add i64 %165, 1216512
  %167 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %166
  store float %160, float addrspace(1)* %167, align 4
  %168 = fmul double %10, 7.640000e+00
  %169 = fsub double 0x4057EF6C60E6CAA5, %168
  %170 = fmul double %14, 0x40B76447414A4D2B
  %171 = fsub double %169, %170
  %172 = tail call double @_Z3expd(double %171) nounwind
  %173 = fptrunc double %172 to float
  %174 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %175 = load i64* %174, align 8
  %176 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %177 = load i64* %176, align 8
  %178 = add i64 %175, %177
  %179 = add i64 %178, 1327104
  %180 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %179
  store float %173, float addrspace(1)* %180, align 4
  %181 = fmul double %10, 3.860000e+00
  %182 = fsub double 0x40515A7F62B6AE7D, %181
  %183 = fmul double %14, 0x409A1AB7A4E7AB75
  %184 = fsub double %182, %183
  %185 = tail call double @_Z3expd(double %184) nounwind
  %186 = fptrunc double %185 to float
  %187 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %188 = load i64* %187, align 8
  %189 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %190 = load i64* %189, align 8
  %191 = add i64 %188, %190
  %192 = add i64 %191, 1437696
  %193 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %192
  store float %186, float addrspace(1)* %193, align 4
  %194 = fmul double %10, 1.194000e+01
  %195 = fsub double 0x4060E00CB07D0AEE, %194
  %196 = fmul double %14, 0x40B3345381D7DBF5
  %197 = fsub double %195, %196
  %198 = tail call double @_Z3expd(double %197) nounwind
  %199 = fptrunc double %198 to float
  %200 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %201 = load i64* %200, align 8
  %202 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %203 = load i64* %202, align 8
  %204 = add i64 %201, %203
  %205 = add i64 %204, 1548288
  %206 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %205
  store float %199, float addrspace(1)* %206, align 4
  %207 = fmul double %10, 7.297000e+00
  %208 = fsub double 0x4056DCC43C6FF2D7, %207
  %209 = fmul double %14, 0x40A27A3C970F7B9E
  %210 = fsub double %208, %209
  %211 = tail call double @_Z3expd(double %210) nounwind
  %212 = fptrunc double %211 to float
  %213 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %214 = load i64* %213, align 8
  %215 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %216 = load i64* %215, align 8
  %217 = add i64 %214, %216
  %218 = add i64 %217, 1658880
  %219 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %218
  store float %212, float addrspace(1)* %219, align 4
  %220 = fmul double %10, 9.310000e+00
  %221 = fsub double 0x405D44CF80DC3372, %220
  %222 = fmul double %14, 0x40E88966ECBFB15B
  %223 = fsub double %221, %222
  %224 = tail call double @_Z3expd(double %223) nounwind
  %225 = fptrunc double %224 to float
  %226 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %227 = load i64* %226, align 8
  %228 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %229 = load i64* %228, align 8
  %230 = add i64 %227, %229
  %231 = add i64 %230, 1769472
  %232 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %231
  store float %225, float addrspace(1)* %232, align 4
  %233 = fmul double %10, 7.620000e+00
  %234 = fsub double 0x405839046E8F29D4, %233
  %235 = fmul double %14, 0x40AB66D72085B185
  %236 = fsub double %234, %235
  %237 = tail call double @_Z3expd(double %236) nounwind
  %238 = fptrunc double %237 to float
  %239 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %240 = load i64* %239, align 8
  %241 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %242 = load i64* %241, align 8
  %243 = add i64 %240, %242
  %244 = add i64 %243, 1880064
  %245 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %244
  store float %238, float addrspace(1)* %245, align 4
  %246 = fmul double %10, 7.080000e+00
  %247 = fsub double 0x4057C6061E92923E, %246
  %248 = fmul double %14, 0x40AA4801C044284E
  %249 = fsub double %247, %248
  %250 = tail call double @_Z3expd(double %249) nounwind
  %251 = fptrunc double %250 to float
  %252 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %253 = load i64* %252, align 8
  %254 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %255 = load i64* %254, align 8
  %256 = add i64 %253, %255
  %257 = add i64 %256, 1990656
  %258 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %257
  store float %251, float addrspace(1)* %258, align 4
  %259 = fmul double %10, 1.200000e+01
  %260 = fsub double 0x40614E16D0917D6B, %259
  %261 = fmul double %14, 0x40A776315F45E0B5
  %262 = fsub double %260, %261
  %263 = tail call double @_Z3expd(double %262) nounwind
  %264 = fptrunc double %263 to float
  %265 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %266 = load i64* %265, align 8
  %267 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %268 = load i64* %267, align 8
  %269 = add i64 %266, %268
  %270 = add i64 %269, 2101248
  %271 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %270
  store float %264, float addrspace(1)* %271, align 4
  %272 = fmul double %10, 6.660000e+00
  %273 = fsub double 0x40565546441C8F83, %272
  %274 = fmul double %14, 0x40AB850888F861A6
  %275 = fsub double %273, %274
  %276 = tail call double @_Z3expd(double %275) nounwind
  %277 = fptrunc double %276 to float
  %278 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %279 = load i64* %278, align 8
  %280 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %281 = load i64* %280, align 8
  %282 = add i64 %279, %281
  %283 = add i64 %282, 2211840
  %284 = getelementptr inbounds float addrspace(1)* %RKLOW, i64 %283
  store float %277, float addrspace(1)* %284, align 4
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
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float*
  %7 = load float* %6, align 4
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
  %22 = getelementptr inbounds float addrspace(1)* %1, i64 %21
  %23 = load float addrspace(1)* %22, align 4
  %24 = fmul float %23, %7
  %25 = call float @_Z3logf(float %24) nounwind
  %26 = fpext float %25 to double
  %27 = fmul double %26, 9.000000e-01
  %28 = fsub double 0x404523C4B7549584, %27
  %29 = fdiv float 1.000000e+00, %24
  %30 = fpext float %29 to double
  %31 = fmul double %30, 0x408ABBBF266BA494
  %32 = fadd double %28, %31
  %33 = call double @_Z3expd(double %32) nounwind
  %34 = fptrunc double %33 to float
  %35 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %36 = load i64* %35, align 8
  %37 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %38 = load i64* %37, align 8
  %39 = add i64 %36, %38
  %40 = getelementptr inbounds float addrspace(1)* %4, i64 %39
  store float %34, float addrspace(1)* %40, align 4
  %41 = fmul double %26, 3.420000e+00
  %42 = fsub double 0x404FE5858E49DA3F, %41
  %43 = fmul double %30, 0x40E4B9CA6DC5D639
  %44 = fsub double %42, %43
  %45 = call double @_Z3expd(double %44) nounwind
  %46 = fptrunc double %45 to float
  %47 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %48 = load i64* %47, align 8
  %49 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %50 = load i64* %49, align 8
  %51 = add i64 %48, %50
  %52 = add i64 %51, 110592
  %53 = getelementptr inbounds float addrspace(1)* %4, i64 %52
  store float %46, float addrspace(1)* %53, align 4
  %54 = fmul double %26, 3.740000e+00
  %55 = fsub double 0x40505D9028D78F9E, %54
  %56 = fmul double %30, 0x408E71D1DB445ED5
  %57 = fsub double %55, %56
  %58 = call double @_Z3expd(double %57) nounwind
  %59 = fptrunc double %58 to float
  %60 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %61 = load i64* %60, align 8
  %62 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %63 = load i64* %62, align 8
  %64 = add i64 %61, %63
  %65 = add i64 %64, 221184
  %66 = getelementptr inbounds float addrspace(1)* %4, i64 %65
  store float %59, float addrspace(1)* %66, align 4
  %67 = fmul double %26, 2.570000e+00
  %68 = fsub double 0x404BC7F46D24C689, %67
  %69 = fmul double %30, 0x408668AB85A4F00F
  %70 = fsub double %68, %69
  %71 = call double @_Z3expd(double %70) nounwind
  %72 = fptrunc double %71 to float
  %73 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %74 = load i64* %73, align 8
  %75 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %76 = load i64* %75, align 8
  %77 = add i64 %74, %76
  %78 = add i64 %77, 331776
  %79 = getelementptr inbounds float addrspace(1)* %4, i64 %78
  store float %72, float addrspace(1)* %79, align 4
  %80 = fmul double %26, 3.140000e+00
  %81 = fsub double 0x404FAA9E0CC5E120, %80
  %82 = fmul double %30, 0x408357A6E9FF0CBB
  %83 = fsub double %81, %82
  %84 = call double @_Z3expd(double %83) nounwind
  %85 = fptrunc double %84 to float
  %86 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %87 = load i64* %86, align 8
  %88 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %89 = load i64* %88, align 8
  %90 = add i64 %87, %89
  %91 = add i64 %90, 442368
  %92 = getelementptr inbounds float addrspace(1)* %4, i64 %91
  store float %85, float addrspace(1)* %92, align 4
  %93 = fmul double %26, 5.110000e+00
  %94 = fsub double 0x40533E63EE5181D3, %93
  %95 = fmul double %30, 0x40ABE4A4FF43419E
  %96 = fsub double %94, %95
  %97 = call double @_Z3expd(double %96) nounwind
  %98 = fptrunc double %97 to float
  %99 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %100 = load i64* %99, align 8
  %101 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %102 = load i64* %101, align 8
  %103 = add i64 %100, %102
  %104 = add i64 %103, 552960
  %105 = getelementptr inbounds float addrspace(1)* %4, i64 %104
  store float %98, float addrspace(1)* %105, align 4
  %106 = fmul double %26, 4.800000e+00
  %107 = fsub double 0x4051776CB60BC028, %106
  %108 = fmul double %30, 0x40A5DBC4F3775B81
  %109 = fsub double %107, %108
  %110 = call double @_Z3expd(double %109) nounwind
  %111 = fptrunc double %110 to float
  %112 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %113 = load i64* %112, align 8
  %114 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %115 = load i64* %114, align 8
  %116 = add i64 %113, %115
  %117 = add i64 %116, 663552
  %118 = getelementptr inbounds float addrspace(1)* %4, i64 %117
  store float %111, float addrspace(1)* %118, align 4
  %119 = fmul double %26, 4.760000e+00
  %120 = fsub double 0x4053391C5D2DD880, %119
  %121 = fmul double %30, 0x40932F6509BF9C63
  %122 = fsub double %120, %121
  %123 = call double @_Z3expd(double %122) nounwind
  %124 = fptrunc double %123 to float
  %125 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %126 = load i64* %125, align 8
  %127 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %128 = load i64* %127, align 8
  %129 = add i64 %126, %128
  %130 = add i64 %129, 774144
  %131 = getelementptr inbounds float addrspace(1)* %4, i64 %130
  store float %124, float addrspace(1)* %131, align 4
  %132 = fmul double %26, 9.588000e+00
  %133 = fsub double 0x405BD400B0292817, %132
  %134 = fmul double %30, 2.566405e+03
  %135 = fsub double %133, %134
  %136 = call double @_Z3expd(double %135) nounwind
  %137 = fptrunc double %136 to float
  %138 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %139 = load i64* %138, align 8
  %140 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %141 = load i64* %140, align 8
  %142 = add i64 %139, %141
  %143 = add i64 %142, 884736
  %144 = getelementptr inbounds float addrspace(1)* %4, i64 %143
  store float %137, float addrspace(1)* %144, align 4
  %145 = fmul double %26, 9.670000e+00
  %146 = fsub double 0x405CECD0A2446306, %145
  %147 = fmul double %30, 0x40A87403ED527E52
  %148 = fsub double %146, %147
  %149 = call double @_Z3expd(double %148) nounwind
  %150 = fptrunc double %149 to float
  %151 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %152 = load i64* %151, align 8
  %153 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %154 = load i64* %153, align 8
  %155 = add i64 %152, %154
  %156 = add i64 %155, 995328
  %157 = getelementptr inbounds float addrspace(1)* %4, i64 %156
  store float %150, float addrspace(1)* %157, align 4
  %158 = fmul double %26, 6.400000e-01
  %159 = fsub double 0x4041B7A9A2FC18EB, %158
  %160 = fmul double %30, 0x40D86C7793DD97F6
  %161 = fsub double %159, %160
  %162 = call double @_Z3expd(double %161) nounwind
  %163 = fptrunc double %162 to float
  %164 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %165 = load i64* %164, align 8
  %166 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %167 = load i64* %166, align 8
  %168 = add i64 %165, %167
  %169 = add i64 %168, 1105920
  %170 = getelementptr inbounds float addrspace(1)* %4, i64 %169
  store float %163, float addrspace(1)* %170, align 4
  %171 = fmul double %26, 3.400000e+00
  %172 = fsub double 0x404F8E4E054690DE, %171
  %173 = fmul double %30, 0x40D197A0CE703AFB
  %174 = fsub double %172, %173
  %175 = call double @_Z3expd(double %174) nounwind
  %176 = fptrunc double %175 to float
  %177 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %178 = load i64* %177, align 8
  %179 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %180 = load i64* %179, align 8
  %181 = add i64 %178, %180
  %182 = add i64 %181, 1216512
  %183 = getelementptr inbounds float addrspace(1)* %4, i64 %182
  store float %176, float addrspace(1)* %183, align 4
  %184 = fmul double %26, 7.640000e+00
  %185 = fsub double 0x4057EF6C60E6CAA5, %184
  %186 = fmul double %30, 0x40B76447414A4D2B
  %187 = fsub double %185, %186
  %188 = call double @_Z3expd(double %187) nounwind
  %189 = fptrunc double %188 to float
  %190 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %191 = load i64* %190, align 8
  %192 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %193 = load i64* %192, align 8
  %194 = add i64 %191, %193
  %195 = add i64 %194, 1327104
  %196 = getelementptr inbounds float addrspace(1)* %4, i64 %195
  store float %189, float addrspace(1)* %196, align 4
  %197 = fmul double %26, 3.860000e+00
  %198 = fsub double 0x40515A7F62B6AE7D, %197
  %199 = fmul double %30, 0x409A1AB7A4E7AB75
  %200 = fsub double %198, %199
  %201 = call double @_Z3expd(double %200) nounwind
  %202 = fptrunc double %201 to float
  %203 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %204 = load i64* %203, align 8
  %205 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %206 = load i64* %205, align 8
  %207 = add i64 %204, %206
  %208 = add i64 %207, 1437696
  %209 = getelementptr inbounds float addrspace(1)* %4, i64 %208
  store float %202, float addrspace(1)* %209, align 4
  %210 = fmul double %26, 1.194000e+01
  %211 = fsub double 0x4060E00CB07D0AEE, %210
  %212 = fmul double %30, 0x40B3345381D7DBF5
  %213 = fsub double %211, %212
  %214 = call double @_Z3expd(double %213) nounwind
  %215 = fptrunc double %214 to float
  %216 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %217 = load i64* %216, align 8
  %218 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %219 = load i64* %218, align 8
  %220 = add i64 %217, %219
  %221 = add i64 %220, 1548288
  %222 = getelementptr inbounds float addrspace(1)* %4, i64 %221
  store float %215, float addrspace(1)* %222, align 4
  %223 = fmul double %26, 7.297000e+00
  %224 = fsub double 0x4056DCC43C6FF2D7, %223
  %225 = fmul double %30, 0x40A27A3C970F7B9E
  %226 = fsub double %224, %225
  %227 = call double @_Z3expd(double %226) nounwind
  %228 = fptrunc double %227 to float
  %229 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %230 = load i64* %229, align 8
  %231 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %232 = load i64* %231, align 8
  %233 = add i64 %230, %232
  %234 = add i64 %233, 1658880
  %235 = getelementptr inbounds float addrspace(1)* %4, i64 %234
  store float %228, float addrspace(1)* %235, align 4
  %236 = fmul double %26, 9.310000e+00
  %237 = fsub double 0x405D44CF80DC3372, %236
  %238 = fmul double %30, 0x40E88966ECBFB15B
  %239 = fsub double %237, %238
  %240 = call double @_Z3expd(double %239) nounwind
  %241 = fptrunc double %240 to float
  %242 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %243 = load i64* %242, align 8
  %244 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %245 = load i64* %244, align 8
  %246 = add i64 %243, %245
  %247 = add i64 %246, 1769472
  %248 = getelementptr inbounds float addrspace(1)* %4, i64 %247
  store float %241, float addrspace(1)* %248, align 4
  %249 = fmul double %26, 7.620000e+00
  %250 = fsub double 0x405839046E8F29D4, %249
  %251 = fmul double %30, 0x40AB66D72085B185
  %252 = fsub double %250, %251
  %253 = call double @_Z3expd(double %252) nounwind
  %254 = fptrunc double %253 to float
  %255 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %256 = load i64* %255, align 8
  %257 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %258 = load i64* %257, align 8
  %259 = add i64 %256, %258
  %260 = add i64 %259, 1880064
  %261 = getelementptr inbounds float addrspace(1)* %4, i64 %260
  store float %254, float addrspace(1)* %261, align 4
  %262 = fmul double %26, 7.080000e+00
  %263 = fsub double 0x4057C6061E92923E, %262
  %264 = fmul double %30, 0x40AA4801C044284E
  %265 = fsub double %263, %264
  %266 = call double @_Z3expd(double %265) nounwind
  %267 = fptrunc double %266 to float
  %268 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %269 = load i64* %268, align 8
  %270 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %271 = load i64* %270, align 8
  %272 = add i64 %269, %271
  %273 = add i64 %272, 1990656
  %274 = getelementptr inbounds float addrspace(1)* %4, i64 %273
  store float %267, float addrspace(1)* %274, align 4
  %275 = fmul double %26, 1.200000e+01
  %276 = fsub double 0x40614E16D0917D6B, %275
  %277 = fmul double %30, 0x40A776315F45E0B5
  %278 = fsub double %276, %277
  %279 = call double @_Z3expd(double %278) nounwind
  %280 = fptrunc double %279 to float
  %281 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %282 = load i64* %281, align 8
  %283 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %284 = load i64* %283, align 8
  %285 = add i64 %282, %284
  %286 = add i64 %285, 2101248
  %287 = getelementptr inbounds float addrspace(1)* %4, i64 %286
  store float %280, float addrspace(1)* %287, align 4
  %288 = fmul double %26, 6.660000e+00
  %289 = fsub double 0x40565546441C8F83, %288
  %290 = fmul double %30, 0x40AB850888F861A6
  %291 = fsub double %289, %290
  %292 = call double @_Z3expd(double %291) nounwind
  %293 = fptrunc double %292 to float
  %294 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %295 = load i64* %294, align 8
  %296 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %297 = load i64* %296, align 8
  %298 = add i64 %295, %297
  %299 = add i64 %298, 2211840
  %300 = getelementptr inbounds float addrspace(1)* %4, i64 %299
  store float %293, float addrspace(1)* %300, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__ratt10_kernel_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__ratt10_kernel_separated_args.exit:              ; preds = %SyncBB.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__ratt10_kernel_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float", metadata !"opencl_ratt10_kernel_locals_anchor", void (i8*)* @ratt10_kernel}
!1 = metadata !{i32 0, i32 0, i32 0}


