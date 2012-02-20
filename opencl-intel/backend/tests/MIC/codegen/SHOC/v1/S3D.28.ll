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

declare void @__gr_base_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double addrspace(1)*, double addrspace(1)*, double, double) nounwind

declare i64 @get_global_id(i32)

declare double @_Z4fmaxdd(double, double)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__gr_base_separated_args(double addrspace(1)* nocapture %P, double addrspace(1)* nocapture %T, double addrspace(1)* %Y, double addrspace(1)* %C, double %TCONV, double %PCONV, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph:
  br label %SyncBB6

SyncBB6:                                          ; preds = %bb.nph, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %bb.nph ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %5 = getelementptr inbounds double addrspace(1)* %T, i64 %4
  %6 = load double addrspace(1)* %5, align 8
  %7 = fmul double %6, %TCONV
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %9 = load i64* %8, align 8
  %10 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %11 = load i64* %10, align 8
  %12 = add i64 %9, %11
  %13 = getelementptr inbounds double addrspace(1)* %P, i64 %12
  %14 = load double addrspace(1)* %13, align 8
  %15 = fmul double %14, %PCONV
  %16 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %17 = load i64* %16, align 8
  %18 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %19 = load i64* %18, align 8
  %20 = add i64 %17, %19
  %21 = getelementptr inbounds double addrspace(1)* %Y, i64 %20
  %22 = load double addrspace(1)* %21, align 8
  %23 = fmul double %22, 0x3FDFBF39E83F553C
  %24 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %25 = load i64* %24, align 8
  %26 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %27 = load i64* %26, align 8
  %28 = add i64 %25, %27
  %29 = getelementptr inbounds double addrspace(1)* %C, i64 %28
  store double %23, double addrspace(1)* %29, align 8
  %30 = fadd double %23, 0.000000e+00
  %31 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %32 = load i64* %31, align 8
  %33 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %34 = load i64* %33, align 8
  %35 = add i64 %32, %34
  %36 = add i64 %35, 110592
  %37 = getelementptr inbounds double addrspace(1)* %Y, i64 %36
  %38 = load double addrspace(1)* %37, align 8
  %39 = fmul double %38, 0x3FEFBF39E8C8C59B
  %40 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %41 = load i64* %40, align 8
  %42 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %43 = load i64* %42, align 8
  %44 = add i64 %41, %43
  %45 = add i64 %44, 110592
  %46 = getelementptr inbounds double addrspace(1)* %C, i64 %45
  store double %39, double addrspace(1)* %46, align 8
  %47 = fadd double %30, %39
  %48 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %49 = load i64* %48, align 8
  %50 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %51 = load i64* %50, align 8
  %52 = add i64 %49, %51
  %53 = add i64 %52, 221184
  %54 = getelementptr inbounds double addrspace(1)* %Y, i64 %53
  %55 = load double addrspace(1)* %54, align 8
  %56 = fmul double %55, 0x3FB00027506598ED
  %57 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %58 = load i64* %57, align 8
  %59 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %60 = load i64* %59, align 8
  %61 = add i64 %58, %60
  %62 = add i64 %61, 221184
  %63 = getelementptr inbounds double addrspace(1)* %C, i64 %62
  store double %56, double addrspace(1)* %63, align 8
  %64 = fadd double %47, %56
  %65 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %66 = load i64* %65, align 8
  %67 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %68 = load i64* %67, align 8
  %69 = add i64 %66, %68
  %70 = add i64 %69, 331776
  %71 = getelementptr inbounds double addrspace(1)* %Y, i64 %70
  %72 = load double addrspace(1)* %71, align 8
  %73 = fmul double %72, 0x3FA000274FF7A56E
  %74 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %75 = load i64* %74, align 8
  %76 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %77 = load i64* %76, align 8
  %78 = add i64 %75, %77
  %79 = add i64 %78, 331776
  %80 = getelementptr inbounds double addrspace(1)* %C, i64 %79
  store double %73, double addrspace(1)* %80, align 8
  %81 = fadd double %64, %73
  %82 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %83 = load i64* %82, align 8
  %84 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %85 = load i64* %84, align 8
  %86 = add i64 %83, %85
  %87 = add i64 %86, 442368
  %88 = getelementptr inbounds double addrspace(1)* %Y, i64 %87
  %89 = load double addrspace(1)* %88, align 8
  %90 = fmul double %89, 0x3FAE1AC6C7228985
  %91 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %92 = load i64* %91, align 8
  %93 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %94 = load i64* %93, align 8
  %95 = add i64 %92, %94
  %96 = add i64 %95, 442368
  %97 = getelementptr inbounds double addrspace(1)* %C, i64 %96
  store double %90, double addrspace(1)* %97, align 8
  %98 = fadd double %81, %90
  %99 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %100 = load i64* %99, align 8
  %101 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %102 = load i64* %101, align 8
  %103 = add i64 %100, %102
  %104 = add i64 %103, 552960
  %105 = getelementptr inbounds double addrspace(1)* %Y, i64 %104
  %106 = load double addrspace(1)* %105, align 8
  %107 = fmul double %106, 0x3FAC6B93CBF5178C
  %108 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %109 = load i64* %108, align 8
  %110 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %111 = load i64* %110, align 8
  %112 = add i64 %109, %111
  %113 = add i64 %112, 552960
  %114 = getelementptr inbounds double addrspace(1)* %C, i64 %113
  store double %107, double addrspace(1)* %114, align 8
  %115 = fadd double %98, %107
  %116 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %117 = load i64* %116, align 8
  %118 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %119 = load i64* %118, align 8
  %120 = add i64 %117, %119
  %121 = add i64 %120, 663552
  %122 = getelementptr inbounds double addrspace(1)* %Y, i64 %121
  %123 = load double addrspace(1)* %122, align 8
  %124 = fmul double %123, 0x3F9F0620CF851840
  %125 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %126 = load i64* %125, align 8
  %127 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %128 = load i64* %127, align 8
  %129 = add i64 %126, %128
  %130 = add i64 %129, 663552
  %131 = getelementptr inbounds double addrspace(1)* %C, i64 %130
  store double %124, double addrspace(1)* %131, align 8
  %132 = fadd double %115, %124
  %133 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %134 = load i64* %133, align 8
  %135 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %136 = load i64* %135, align 8
  %137 = add i64 %134, %136
  %138 = add i64 %137, 774144
  %139 = getelementptr inbounds double addrspace(1)* %Y, i64 %138
  %140 = load double addrspace(1)* %139, align 8
  %141 = fmul double %140, 0x3F9E1AC6C7FE7084
  %142 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %143 = load i64* %142, align 8
  %144 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %145 = load i64* %144, align 8
  %146 = add i64 %143, %145
  %147 = add i64 %146, 774144
  %148 = getelementptr inbounds double addrspace(1)* %C, i64 %147
  store double %141, double addrspace(1)* %148, align 8
  %149 = fadd double %132, %141
  %150 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %151 = load i64* %150, align 8
  %152 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %153 = load i64* %152, align 8
  %154 = add i64 %151, %153
  %155 = add i64 %154, 884736
  %156 = getelementptr inbounds double addrspace(1)* %Y, i64 %155
  %157 = load double addrspace(1)* %156, align 8
  %158 = fmul double %157, 0x3FB106E0E0BC2922
  %159 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %160 = load i64* %159, align 8
  %161 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %162 = load i64* %161, align 8
  %163 = add i64 %160, %162
  %164 = add i64 %163, 884736
  %165 = getelementptr inbounds double addrspace(1)* %C, i64 %164
  store double %158, double addrspace(1)* %165, align 8
  %166 = fadd double %149, %158
  %167 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %168 = load i64* %167, align 8
  %169 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %170 = load i64* %169, align 8
  %171 = add i64 %168, %170
  %172 = add i64 %171, 995328
  %173 = getelementptr inbounds double addrspace(1)* %Y, i64 %172
  %174 = load double addrspace(1)* %173, align 8
  %175 = fmul double %174, 0x3FAFEA0710DDA145
  %176 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %177 = load i64* %176, align 8
  %178 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %179 = load i64* %178, align 8
  %180 = add i64 %177, %179
  %181 = add i64 %180, 995328
  %182 = getelementptr inbounds double addrspace(1)* %C, i64 %181
  store double %175, double addrspace(1)* %182, align 8
  %183 = fadd double %166, %175
  %184 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %185 = load i64* %184, align 8
  %186 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %187 = load i64* %186, align 8
  %188 = add i64 %185, %187
  %189 = add i64 %188, 1105920
  %190 = getelementptr inbounds double addrspace(1)* %Y, i64 %189
  %191 = load double addrspace(1)* %190, align 8
  %192 = fmul double %191, 0x3FA2476130A51704
  %193 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %194 = load i64* %193, align 8
  %195 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %196 = load i64* %195, align 8
  %197 = add i64 %194, %196
  %198 = add i64 %197, 1105920
  %199 = getelementptr inbounds double addrspace(1)* %C, i64 %198
  store double %192, double addrspace(1)* %199, align 8
  %200 = fadd double %183, %192
  %201 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %202 = load i64* %201, align 8
  %203 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %204 = load i64* %203, align 8
  %205 = add i64 %202, %204
  %206 = add i64 %205, 1216512
  %207 = getelementptr inbounds double addrspace(1)* %Y, i64 %206
  %208 = load double addrspace(1)* %207, align 8
  %209 = fmul double %208, 0x3F9744789B6B579C
  %210 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %211 = load i64* %210, align 8
  %212 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %213 = load i64* %212, align 8
  %214 = add i64 %211, %213
  %215 = add i64 %214, 1216512
  %216 = getelementptr inbounds double addrspace(1)* %C, i64 %215
  store double %209, double addrspace(1)* %216, align 8
  %217 = fadd double %200, %209
  %218 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %219 = load i64* %218, align 8
  %220 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %221 = load i64* %220, align 8
  %222 = add i64 %219, %221
  %223 = add i64 %222, 1327104
  %224 = getelementptr inbounds double addrspace(1)* %Y, i64 %223
  %225 = load double addrspace(1)* %224, align 8
  %226 = fmul double %225, 0x3FA10D364DB3ABD8
  %227 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %228 = load i64* %227, align 8
  %229 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %230 = load i64* %229, align 8
  %231 = add i64 %228, %230
  %232 = add i64 %231, 1327104
  %233 = getelementptr inbounds double addrspace(1)* %C, i64 %232
  store double %226, double addrspace(1)* %233, align 8
  %234 = fadd double %217, %226
  %235 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %236 = load i64* %235, align 8
  %237 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %238 = load i64* %237, align 8
  %239 = add i64 %236, %238
  %240 = add i64 %239, 1437696
  %241 = getelementptr inbounds double addrspace(1)* %Y, i64 %240
  %242 = load double addrspace(1)* %241, align 8
  %243 = fmul double %242, 0x3FA3A9D3B8FA320B
  %244 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %245 = load i64* %244, align 8
  %246 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %247 = load i64* %246, align 8
  %248 = add i64 %245, %247
  %249 = add i64 %248, 1437696
  %250 = getelementptr inbounds double addrspace(1)* %C, i64 %249
  store double %243, double addrspace(1)* %250, align 8
  %251 = fadd double %234, %243
  %252 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %253 = load i64* %252, align 8
  %254 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %255 = load i64* %254, align 8
  %256 = add i64 %253, %255
  %257 = add i64 %256, 1548288
  %258 = getelementptr inbounds double addrspace(1)* %Y, i64 %257
  %259 = load double addrspace(1)* %258, align 8
  %260 = fmul double %259, 0x3FA2401A2BB8302C
  %261 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %262 = load i64* %261, align 8
  %263 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %264 = load i64* %263, align 8
  %265 = add i64 %262, %264
  %266 = add i64 %265, 1548288
  %267 = getelementptr inbounds double addrspace(1)* %C, i64 %266
  store double %260, double addrspace(1)* %267, align 8
  %268 = fadd double %251, %260
  %269 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %270 = load i64* %269, align 8
  %271 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %272 = load i64* %271, align 8
  %273 = add i64 %270, %272
  %274 = add i64 %273, 1658880
  %275 = getelementptr inbounds double addrspace(1)* %Y, i64 %274
  %276 = load double addrspace(1)* %275, align 8
  %277 = fmul double %276, 0x3FA106E0E12A1CA1
  %278 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %279 = load i64* %278, align 8
  %280 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %281 = load i64* %280, align 8
  %282 = add i64 %279, %281
  %283 = add i64 %282, 1658880
  %284 = getelementptr inbounds double addrspace(1)* %C, i64 %283
  store double %277, double addrspace(1)* %284, align 8
  %285 = fadd double %268, %277
  %286 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %287 = load i64* %286, align 8
  %288 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %289 = load i64* %288, align 8
  %290 = add i64 %287, %289
  %291 = add i64 %290, 1769472
  %292 = getelementptr inbounds double addrspace(1)* %Y, i64 %291
  %293 = load double addrspace(1)* %292, align 8
  %294 = fmul double %293, 0x3F98F521E6C0CFFB
  %295 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %296 = load i64* %295, align 8
  %297 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %298 = load i64* %297, align 8
  %299 = add i64 %296, %298
  %300 = add i64 %299, 1769472
  %301 = getelementptr inbounds double addrspace(1)* %C, i64 %300
  store double %294, double addrspace(1)* %301, align 8
  %302 = fadd double %285, %294
  %303 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %304 = load i64* %303, align 8
  %305 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %306 = load i64* %305, align 8
  %307 = add i64 %304, %306
  %308 = add i64 %307, 1880064
  %309 = getelementptr inbounds double addrspace(1)* %Y, i64 %308
  %310 = load double addrspace(1)* %309, align 8
  %311 = fmul double %310, 0x3F985BEF63267548
  %312 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %313 = load i64* %312, align 8
  %314 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %315 = load i64* %314, align 8
  %316 = add i64 %313, %315
  %317 = add i64 %316, 1880064
  %318 = getelementptr inbounds double addrspace(1)* %C, i64 %317
  store double %311, double addrspace(1)* %318, align 8
  %319 = fadd double %302, %311
  %320 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %321 = load i64* %320, align 8
  %322 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %323 = load i64* %322, align 8
  %324 = add i64 %321, %323
  %325 = add i64 %324, 1990656
  %326 = getelementptr inbounds double addrspace(1)* %Y, i64 %325
  %327 = load double addrspace(1)* %326, align 8
  %328 = fmul double %327, 0x3F973E9268CC11FF
  %329 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %330 = load i64* %329, align 8
  %331 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %332 = load i64* %331, align 8
  %333 = add i64 %330, %332
  %334 = add i64 %333, 1990656
  %335 = getelementptr inbounds double addrspace(1)* %C, i64 %334
  store double %328, double addrspace(1)* %335, align 8
  %336 = fadd double %319, %328
  %337 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %338 = load i64* %337, align 8
  %339 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %340 = load i64* %339, align 8
  %341 = add i64 %338, %340
  %342 = add i64 %341, 2101248
  %343 = getelementptr inbounds double addrspace(1)* %Y, i64 %342
  %344 = load double addrspace(1)* %343, align 8
  %345 = fmul double %344, 0x3F98EE5877603FCE
  %346 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %347 = load i64* %346, align 8
  %348 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %349 = load i64* %348, align 8
  %350 = add i64 %347, %349
  %351 = add i64 %350, 2101248
  %352 = getelementptr inbounds double addrspace(1)* %C, i64 %351
  store double %345, double addrspace(1)* %352, align 8
  %353 = fadd double %336, %345
  %354 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %355 = load i64* %354, align 8
  %356 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %357 = load i64* %356, align 8
  %358 = add i64 %355, %357
  %359 = add i64 %358, 2211840
  %360 = getelementptr inbounds double addrspace(1)* %Y, i64 %359
  %361 = load double addrspace(1)* %360, align 8
  %362 = fmul double %361, 0x3F9855783A4AEAE5
  %363 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %364 = load i64* %363, align 8
  %365 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %366 = load i64* %365, align 8
  %367 = add i64 %364, %366
  %368 = add i64 %367, 2211840
  %369 = getelementptr inbounds double addrspace(1)* %C, i64 %368
  store double %362, double addrspace(1)* %369, align 8
  %370 = fadd double %353, %362
  %371 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %372 = load i64* %371, align 8
  %373 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %374 = load i64* %373, align 8
  %375 = add i64 %372, %374
  %376 = add i64 %375, 2322432
  %377 = getelementptr inbounds double addrspace(1)* %Y, i64 %376
  %378 = load double addrspace(1)* %377, align 8
  %379 = fmul double %378, 0x3FA246E7609AF71C
  %380 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %381 = load i64* %380, align 8
  %382 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %383 = load i64* %382, align 8
  %384 = add i64 %381, %383
  %385 = add i64 %384, 2322432
  %386 = getelementptr inbounds double addrspace(1)* %C, i64 %385
  store double %379, double addrspace(1)* %386, align 8
  %387 = fadd double %370, %379
  %388 = fmul double %387, %7
  %389 = fmul double %388, 8.314510e+07
  %390 = fdiv double 1.000000e+00, %389
  %391 = fmul double %15, %390
  br label %392

; <label>:392                                     ; preds = %392, %SyncBB6
  %indvar = phi i64 [ 0, %SyncBB6 ], [ %indvar.next, %392 ]
  %tmp3 = mul i64 %indvar, 110592
  %393 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %394 = load i64* %393, align 8
  %395 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %396 = load i64* %395, align 8
  %397 = add i64 %394, %396
  %tmp4 = add i64 %397, %tmp3
  %398 = getelementptr inbounds double addrspace(1)* %C, i64 %tmp4
  %399 = load double addrspace(1)* %398, align 8
  %400 = tail call double @_Z4fmaxdd(double %399, double 1.000000e-50) nounwind
  %401 = fmul double %400, %391
  %402 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %403 = load i64* %402, align 8
  %404 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %405 = load i64* %404, align 8
  %406 = add i64 %403, %405
  %tmp5 = add i64 %406, %tmp3
  %407 = getelementptr inbounds double addrspace(1)* %C, i64 %tmp5
  store double %401, double addrspace(1)* %407, align 8
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, 22
  br i1 %exitcond, label %._crit_edge, label %392

._crit_edge:                                      ; preds = %392
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB6

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @gr_base(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double addrspace(1)**
  %7 = load double addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to double addrspace(1)**
  %10 = load double addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double*
  %13 = load double* %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to double*
  %16 = load double* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 96
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  br label %SyncBB6.i

SyncBB6.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %26 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %27 = load i64* %26, align 8
  %28 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %29 = load i64* %28, align 8
  %30 = add i64 %27, %29
  %31 = getelementptr inbounds double addrspace(1)* %4, i64 %30
  %32 = load double addrspace(1)* %31, align 8
  %33 = fmul double %32, %13
  %34 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %35 = load i64* %34, align 8
  %36 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %37 = load i64* %36, align 8
  %38 = add i64 %35, %37
  %39 = getelementptr inbounds double addrspace(1)* %1, i64 %38
  %40 = load double addrspace(1)* %39, align 8
  %41 = fmul double %40, %16
  %42 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %43 = load i64* %42, align 8
  %44 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %45 = load i64* %44, align 8
  %46 = add i64 %43, %45
  %47 = getelementptr inbounds double addrspace(1)* %7, i64 %46
  %48 = load double addrspace(1)* %47, align 8
  %49 = fmul double %48, 0x3FDFBF39E83F553C
  %50 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %51 = load i64* %50, align 8
  %52 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %53 = load i64* %52, align 8
  %54 = add i64 %51, %53
  %55 = getelementptr inbounds double addrspace(1)* %10, i64 %54
  store double %49, double addrspace(1)* %55, align 8
  %56 = fadd double %49, 0.000000e+00
  %57 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %58 = load i64* %57, align 8
  %59 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %60 = load i64* %59, align 8
  %61 = add i64 %58, %60
  %62 = add i64 %61, 110592
  %63 = getelementptr inbounds double addrspace(1)* %7, i64 %62
  %64 = load double addrspace(1)* %63, align 8
  %65 = fmul double %64, 0x3FEFBF39E8C8C59B
  %66 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %67 = load i64* %66, align 8
  %68 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %69 = load i64* %68, align 8
  %70 = add i64 %67, %69
  %71 = add i64 %70, 110592
  %72 = getelementptr inbounds double addrspace(1)* %10, i64 %71
  store double %65, double addrspace(1)* %72, align 8
  %73 = fadd double %56, %65
  %74 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %75 = load i64* %74, align 8
  %76 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %77 = load i64* %76, align 8
  %78 = add i64 %75, %77
  %79 = add i64 %78, 221184
  %80 = getelementptr inbounds double addrspace(1)* %7, i64 %79
  %81 = load double addrspace(1)* %80, align 8
  %82 = fmul double %81, 0x3FB00027506598ED
  %83 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %84 = load i64* %83, align 8
  %85 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %86 = load i64* %85, align 8
  %87 = add i64 %84, %86
  %88 = add i64 %87, 221184
  %89 = getelementptr inbounds double addrspace(1)* %10, i64 %88
  store double %82, double addrspace(1)* %89, align 8
  %90 = fadd double %73, %82
  %91 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %92 = load i64* %91, align 8
  %93 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %94 = load i64* %93, align 8
  %95 = add i64 %92, %94
  %96 = add i64 %95, 331776
  %97 = getelementptr inbounds double addrspace(1)* %7, i64 %96
  %98 = load double addrspace(1)* %97, align 8
  %99 = fmul double %98, 0x3FA000274FF7A56E
  %100 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %101 = load i64* %100, align 8
  %102 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %103 = load i64* %102, align 8
  %104 = add i64 %101, %103
  %105 = add i64 %104, 331776
  %106 = getelementptr inbounds double addrspace(1)* %10, i64 %105
  store double %99, double addrspace(1)* %106, align 8
  %107 = fadd double %90, %99
  %108 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %109 = load i64* %108, align 8
  %110 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %111 = load i64* %110, align 8
  %112 = add i64 %109, %111
  %113 = add i64 %112, 442368
  %114 = getelementptr inbounds double addrspace(1)* %7, i64 %113
  %115 = load double addrspace(1)* %114, align 8
  %116 = fmul double %115, 0x3FAE1AC6C7228985
  %117 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %118 = load i64* %117, align 8
  %119 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %120 = load i64* %119, align 8
  %121 = add i64 %118, %120
  %122 = add i64 %121, 442368
  %123 = getelementptr inbounds double addrspace(1)* %10, i64 %122
  store double %116, double addrspace(1)* %123, align 8
  %124 = fadd double %107, %116
  %125 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %126 = load i64* %125, align 8
  %127 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %128 = load i64* %127, align 8
  %129 = add i64 %126, %128
  %130 = add i64 %129, 552960
  %131 = getelementptr inbounds double addrspace(1)* %7, i64 %130
  %132 = load double addrspace(1)* %131, align 8
  %133 = fmul double %132, 0x3FAC6B93CBF5178C
  %134 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %135 = load i64* %134, align 8
  %136 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %137 = load i64* %136, align 8
  %138 = add i64 %135, %137
  %139 = add i64 %138, 552960
  %140 = getelementptr inbounds double addrspace(1)* %10, i64 %139
  store double %133, double addrspace(1)* %140, align 8
  %141 = fadd double %124, %133
  %142 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %143 = load i64* %142, align 8
  %144 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %145 = load i64* %144, align 8
  %146 = add i64 %143, %145
  %147 = add i64 %146, 663552
  %148 = getelementptr inbounds double addrspace(1)* %7, i64 %147
  %149 = load double addrspace(1)* %148, align 8
  %150 = fmul double %149, 0x3F9F0620CF851840
  %151 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %152 = load i64* %151, align 8
  %153 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %154 = load i64* %153, align 8
  %155 = add i64 %152, %154
  %156 = add i64 %155, 663552
  %157 = getelementptr inbounds double addrspace(1)* %10, i64 %156
  store double %150, double addrspace(1)* %157, align 8
  %158 = fadd double %141, %150
  %159 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %160 = load i64* %159, align 8
  %161 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %162 = load i64* %161, align 8
  %163 = add i64 %160, %162
  %164 = add i64 %163, 774144
  %165 = getelementptr inbounds double addrspace(1)* %7, i64 %164
  %166 = load double addrspace(1)* %165, align 8
  %167 = fmul double %166, 0x3F9E1AC6C7FE7084
  %168 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %169 = load i64* %168, align 8
  %170 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %171 = load i64* %170, align 8
  %172 = add i64 %169, %171
  %173 = add i64 %172, 774144
  %174 = getelementptr inbounds double addrspace(1)* %10, i64 %173
  store double %167, double addrspace(1)* %174, align 8
  %175 = fadd double %158, %167
  %176 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %177 = load i64* %176, align 8
  %178 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %179 = load i64* %178, align 8
  %180 = add i64 %177, %179
  %181 = add i64 %180, 884736
  %182 = getelementptr inbounds double addrspace(1)* %7, i64 %181
  %183 = load double addrspace(1)* %182, align 8
  %184 = fmul double %183, 0x3FB106E0E0BC2922
  %185 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %186 = load i64* %185, align 8
  %187 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %188 = load i64* %187, align 8
  %189 = add i64 %186, %188
  %190 = add i64 %189, 884736
  %191 = getelementptr inbounds double addrspace(1)* %10, i64 %190
  store double %184, double addrspace(1)* %191, align 8
  %192 = fadd double %175, %184
  %193 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %194 = load i64* %193, align 8
  %195 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %196 = load i64* %195, align 8
  %197 = add i64 %194, %196
  %198 = add i64 %197, 995328
  %199 = getelementptr inbounds double addrspace(1)* %7, i64 %198
  %200 = load double addrspace(1)* %199, align 8
  %201 = fmul double %200, 0x3FAFEA0710DDA145
  %202 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %203 = load i64* %202, align 8
  %204 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %205 = load i64* %204, align 8
  %206 = add i64 %203, %205
  %207 = add i64 %206, 995328
  %208 = getelementptr inbounds double addrspace(1)* %10, i64 %207
  store double %201, double addrspace(1)* %208, align 8
  %209 = fadd double %192, %201
  %210 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %211 = load i64* %210, align 8
  %212 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %213 = load i64* %212, align 8
  %214 = add i64 %211, %213
  %215 = add i64 %214, 1105920
  %216 = getelementptr inbounds double addrspace(1)* %7, i64 %215
  %217 = load double addrspace(1)* %216, align 8
  %218 = fmul double %217, 0x3FA2476130A51704
  %219 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %220 = load i64* %219, align 8
  %221 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %222 = load i64* %221, align 8
  %223 = add i64 %220, %222
  %224 = add i64 %223, 1105920
  %225 = getelementptr inbounds double addrspace(1)* %10, i64 %224
  store double %218, double addrspace(1)* %225, align 8
  %226 = fadd double %209, %218
  %227 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %228 = load i64* %227, align 8
  %229 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %230 = load i64* %229, align 8
  %231 = add i64 %228, %230
  %232 = add i64 %231, 1216512
  %233 = getelementptr inbounds double addrspace(1)* %7, i64 %232
  %234 = load double addrspace(1)* %233, align 8
  %235 = fmul double %234, 0x3F9744789B6B579C
  %236 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %237 = load i64* %236, align 8
  %238 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %239 = load i64* %238, align 8
  %240 = add i64 %237, %239
  %241 = add i64 %240, 1216512
  %242 = getelementptr inbounds double addrspace(1)* %10, i64 %241
  store double %235, double addrspace(1)* %242, align 8
  %243 = fadd double %226, %235
  %244 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %245 = load i64* %244, align 8
  %246 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %247 = load i64* %246, align 8
  %248 = add i64 %245, %247
  %249 = add i64 %248, 1327104
  %250 = getelementptr inbounds double addrspace(1)* %7, i64 %249
  %251 = load double addrspace(1)* %250, align 8
  %252 = fmul double %251, 0x3FA10D364DB3ABD8
  %253 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %254 = load i64* %253, align 8
  %255 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %256 = load i64* %255, align 8
  %257 = add i64 %254, %256
  %258 = add i64 %257, 1327104
  %259 = getelementptr inbounds double addrspace(1)* %10, i64 %258
  store double %252, double addrspace(1)* %259, align 8
  %260 = fadd double %243, %252
  %261 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %262 = load i64* %261, align 8
  %263 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %264 = load i64* %263, align 8
  %265 = add i64 %262, %264
  %266 = add i64 %265, 1437696
  %267 = getelementptr inbounds double addrspace(1)* %7, i64 %266
  %268 = load double addrspace(1)* %267, align 8
  %269 = fmul double %268, 0x3FA3A9D3B8FA320B
  %270 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %271 = load i64* %270, align 8
  %272 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %273 = load i64* %272, align 8
  %274 = add i64 %271, %273
  %275 = add i64 %274, 1437696
  %276 = getelementptr inbounds double addrspace(1)* %10, i64 %275
  store double %269, double addrspace(1)* %276, align 8
  %277 = fadd double %260, %269
  %278 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %279 = load i64* %278, align 8
  %280 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %281 = load i64* %280, align 8
  %282 = add i64 %279, %281
  %283 = add i64 %282, 1548288
  %284 = getelementptr inbounds double addrspace(1)* %7, i64 %283
  %285 = load double addrspace(1)* %284, align 8
  %286 = fmul double %285, 0x3FA2401A2BB8302C
  %287 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %288 = load i64* %287, align 8
  %289 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %290 = load i64* %289, align 8
  %291 = add i64 %288, %290
  %292 = add i64 %291, 1548288
  %293 = getelementptr inbounds double addrspace(1)* %10, i64 %292
  store double %286, double addrspace(1)* %293, align 8
  %294 = fadd double %277, %286
  %295 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %296 = load i64* %295, align 8
  %297 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %298 = load i64* %297, align 8
  %299 = add i64 %296, %298
  %300 = add i64 %299, 1658880
  %301 = getelementptr inbounds double addrspace(1)* %7, i64 %300
  %302 = load double addrspace(1)* %301, align 8
  %303 = fmul double %302, 0x3FA106E0E12A1CA1
  %304 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %305 = load i64* %304, align 8
  %306 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %307 = load i64* %306, align 8
  %308 = add i64 %305, %307
  %309 = add i64 %308, 1658880
  %310 = getelementptr inbounds double addrspace(1)* %10, i64 %309
  store double %303, double addrspace(1)* %310, align 8
  %311 = fadd double %294, %303
  %312 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %313 = load i64* %312, align 8
  %314 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %315 = load i64* %314, align 8
  %316 = add i64 %313, %315
  %317 = add i64 %316, 1769472
  %318 = getelementptr inbounds double addrspace(1)* %7, i64 %317
  %319 = load double addrspace(1)* %318, align 8
  %320 = fmul double %319, 0x3F98F521E6C0CFFB
  %321 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %322 = load i64* %321, align 8
  %323 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %324 = load i64* %323, align 8
  %325 = add i64 %322, %324
  %326 = add i64 %325, 1769472
  %327 = getelementptr inbounds double addrspace(1)* %10, i64 %326
  store double %320, double addrspace(1)* %327, align 8
  %328 = fadd double %311, %320
  %329 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %330 = load i64* %329, align 8
  %331 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %332 = load i64* %331, align 8
  %333 = add i64 %330, %332
  %334 = add i64 %333, 1880064
  %335 = getelementptr inbounds double addrspace(1)* %7, i64 %334
  %336 = load double addrspace(1)* %335, align 8
  %337 = fmul double %336, 0x3F985BEF63267548
  %338 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %339 = load i64* %338, align 8
  %340 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %341 = load i64* %340, align 8
  %342 = add i64 %339, %341
  %343 = add i64 %342, 1880064
  %344 = getelementptr inbounds double addrspace(1)* %10, i64 %343
  store double %337, double addrspace(1)* %344, align 8
  %345 = fadd double %328, %337
  %346 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %347 = load i64* %346, align 8
  %348 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %349 = load i64* %348, align 8
  %350 = add i64 %347, %349
  %351 = add i64 %350, 1990656
  %352 = getelementptr inbounds double addrspace(1)* %7, i64 %351
  %353 = load double addrspace(1)* %352, align 8
  %354 = fmul double %353, 0x3F973E9268CC11FF
  %355 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %356 = load i64* %355, align 8
  %357 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %358 = load i64* %357, align 8
  %359 = add i64 %356, %358
  %360 = add i64 %359, 1990656
  %361 = getelementptr inbounds double addrspace(1)* %10, i64 %360
  store double %354, double addrspace(1)* %361, align 8
  %362 = fadd double %345, %354
  %363 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %364 = load i64* %363, align 8
  %365 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %366 = load i64* %365, align 8
  %367 = add i64 %364, %366
  %368 = add i64 %367, 2101248
  %369 = getelementptr inbounds double addrspace(1)* %7, i64 %368
  %370 = load double addrspace(1)* %369, align 8
  %371 = fmul double %370, 0x3F98EE5877603FCE
  %372 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %373 = load i64* %372, align 8
  %374 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %375 = load i64* %374, align 8
  %376 = add i64 %373, %375
  %377 = add i64 %376, 2101248
  %378 = getelementptr inbounds double addrspace(1)* %10, i64 %377
  store double %371, double addrspace(1)* %378, align 8
  %379 = fadd double %362, %371
  %380 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %381 = load i64* %380, align 8
  %382 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %383 = load i64* %382, align 8
  %384 = add i64 %381, %383
  %385 = add i64 %384, 2211840
  %386 = getelementptr inbounds double addrspace(1)* %7, i64 %385
  %387 = load double addrspace(1)* %386, align 8
  %388 = fmul double %387, 0x3F9855783A4AEAE5
  %389 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %390 = load i64* %389, align 8
  %391 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %392 = load i64* %391, align 8
  %393 = add i64 %390, %392
  %394 = add i64 %393, 2211840
  %395 = getelementptr inbounds double addrspace(1)* %10, i64 %394
  store double %388, double addrspace(1)* %395, align 8
  %396 = fadd double %379, %388
  %397 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %398 = load i64* %397, align 8
  %399 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %400 = load i64* %399, align 8
  %401 = add i64 %398, %400
  %402 = add i64 %401, 2322432
  %403 = getelementptr inbounds double addrspace(1)* %7, i64 %402
  %404 = load double addrspace(1)* %403, align 8
  %405 = fmul double %404, 0x3FA246E7609AF71C
  %406 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %407 = load i64* %406, align 8
  %408 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %409 = load i64* %408, align 8
  %410 = add i64 %407, %409
  %411 = add i64 %410, 2322432
  %412 = getelementptr inbounds double addrspace(1)* %10, i64 %411
  store double %405, double addrspace(1)* %412, align 8
  %413 = fadd double %396, %405
  %414 = fmul double %413, %33
  %415 = fmul double %414, 8.314510e+07
  %416 = fdiv double 1.000000e+00, %415
  %417 = fmul double %41, %416
  br label %418

; <label>:418                                     ; preds = %418, %SyncBB6.i
  %indvar.i = phi i64 [ 0, %SyncBB6.i ], [ %indvar.next.i, %418 ]
  %tmp3.i = mul i64 %indvar.i, 110592
  %419 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %420 = load i64* %419, align 8
  %421 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %422 = load i64* %421, align 8
  %423 = add i64 %420, %422
  %tmp4.i = add i64 %423, %tmp3.i
  %424 = getelementptr inbounds double addrspace(1)* %10, i64 %tmp4.i
  %425 = load double addrspace(1)* %424, align 8
  %426 = call double @_Z4fmaxdd(double %425, double 1.000000e-50) nounwind
  %427 = fmul double %426, %417
  %428 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %429 = load i64* %428, align 8
  %430 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %431 = load i64* %430, align 8
  %432 = add i64 %429, %431
  %tmp5.i = add i64 %432, %tmp3.i
  %433 = getelementptr inbounds double addrspace(1)* %10, i64 %tmp5.i
  store double %427, double addrspace(1)* %433, align 8
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, 22
  br i1 %exitcond.i, label %._crit_edge.i, label %418

._crit_edge.i:                                    ; preds = %418
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %__gr_base_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB6.i

__gr_base_separated_args.exit:                    ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, double addrspace(1)*, double addrspace(1)*, double addrspace(1)*, double, double, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__gr_base_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double const __attribute__((address_space(1))) *, double const __attribute__((address_space(1))) *, double const __attribute__((address_space(1))) *, double __attribute__((address_space(1))) *, double const, double const", metadata !"opencl_gr_base_locals_anchor", void (i8*)* @gr_base}
!1 = metadata !{i32 0, i32 0, i32 0}


