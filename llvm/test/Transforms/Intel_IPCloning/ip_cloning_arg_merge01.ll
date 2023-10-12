; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts

; RUN: opt < %s -passes='module(post-inline-ip-cloning)' -debug-only=ipcloning  -disable-output 2>&1 |  FileCheck %s
; CHECK: [IP_CLONING][ARG merge]: Analysing Function(ptr @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678.1)
; CHECK-NEXT: [IP_CLONING][ARG merge]: Detected 1 mergeable argument sets.
; CHECK-NEXT:   [1]: 8 9
; CHECK-NEXT: [IP_CLONING][ARG merge]: Set attribute for (ptr %8) to none 
; CHECK-NEXT: [IP_CLONING][ARG merge]: Function(ptr @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678.1) - ARG(ptr %9) replaced with ptr %8

; The test assesses the compiler's capability to "merge" formal function parameters,
; a process that occurs after the function cloning phase. In this context, "merge"
; involves altering uses of formal function parameters, accomplished by substituting
; them with one chosen from the predefined set of interchangeable parameters.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }
%struct.par_t = type { [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double], [37 x double] }

@.kmpc_loc.0.0.53 = external global %struct.ident_t
@OFF0 = constant [37 x i64] zeroinitializer

define fastcc void @lbm(i32 %0, ptr %1, ptr %2) {
  %4 = alloca ptr, i32 0, align 8
  %5 = alloca ptr, i32 0, align 8
  %6 = alloca ptr, i32 0, align 8
  %7 = alloca ptr, i32 0, align 8
  %8 = alloca ptr, i32 0, align 8
  %9 = alloca ptr, i32 0, align 8
  %10 = alloca ptr, i32 0, align 8
  %11 = alloca ptr, i32 0, align 8
  %12 = alloca ptr, i32 0, align 8
  %13 = alloca ptr, i32 0, align 8
  %14 = alloca double, i32 0, align 8
  %15 = alloca double, i32 0, align 8
  %16 = alloca double, i32 0, align 8
  %17 = alloca i32, i32 0, align 4
  %18 = alloca i32, i32 0, align 4
  %19 = alloca i32, i32 0, align 4
  %20 = alloca i32, i32 0, align 4
  %21 = alloca i32, i32 0, align 4
  %22 = alloca i32, i32 0, align 4
  %23 = alloca i32, i32 0, align 4
  %24 = alloca i32, i32 0, align 4
  %25 = load i32, ptr null, align 4
  %26 = mul i32 0, 0
  %27 = sext i32 0 to i64
  %28 = shl i64 0, 0
  %29 = call i32 null(ptr null, i64 0, i64 0)
  %30 = load i32, ptr null, align 4
  %31 = mul i32 0, 0
  %32 = sext i32 0 to i64
  %33 = shl i64 0, 0
  %34 = call i32 null(ptr null, i64 0, i64 0)
  %35 = load i32, ptr null, align 4
  %36 = mul i32 0, 0
  %37 = sext i32 0 to i64
  %38 = shl i64 0, 0
  %39 = call i32 null(ptr null, i64 0, i64 0)
  %40 = load i32, ptr null, align 4
  %41 = mul i32 0, 0
  %42 = sext i32 0 to i64
  %43 = shl i64 0, 0
  %44 = call i32 null(ptr null, i64 0, i64 0)
  %45 = load i64, ptr null, align 8
  %46 = mul i64 0, 0
  %47 = call i32 null(ptr null, i64 0, i64 0)
  %48 = load i64, ptr null, align 8
  %49 = mul i64 0, 0
  %50 = call i32 null(ptr null, i64 0, i64 0)
  %51 = load i64, ptr null, align 8
  %52 = mul i64 0, 0
  %53 = call i32 null(ptr null, i64 0, i64 0)
  %54 = load i64, ptr null, align 8
  %55 = mul i64 0, 0
  %56 = call i32 null(ptr null, i64 0, i64 0)
  %57 = load ptr, ptr null, align 8
  %58 = icmp eq ptr null, null
  %59 = load ptr, ptr null, align 8
  %60 = icmp eq ptr null, null
  %61 = select i1 false, i1 false, i1 false
  %62 = load ptr, ptr null, align 8
  %63 = icmp eq ptr null, null
  %64 = select i1 false, i1 false, i1 false
  %65 = load ptr, ptr null, align 8
  %66 = icmp eq ptr null, null
  %67 = select i1 false, i1 false, i1 false
  %68 = load ptr, ptr null, align 8
  %69 = icmp eq ptr null, null
  %70 = select i1 false, i1 false, i1 false
  %71 = load ptr, ptr null, align 8
  %72 = icmp eq ptr null, null
  %73 = select i1 false, i1 false, i1 false
  %74 = load ptr, ptr null, align 8
  %75 = icmp eq ptr null, null
  %76 = select i1 false, i1 false, i1 false
  %77 = load ptr, ptr null, align 8
  %78 = icmp eq ptr null, null
  %79 = select i1 false, i1 false, i1 false
  br label %80

80:                                               ; preds = %3
  %81 = load ptr, ptr null, align 8
  %82 = call i64 null(ptr null, i64 0, i64 0, ptr null)
  unreachable

83:                                               ; No predecessors!
  %84 = load i64, ptr null, align 8
  %85 = load i64, ptr null, align 8
  %86 = mul i64 0, 0
  %87 = mul i64 0, 0
  %88 = call i32 null(ptr null, i64 0, i64 0)
  %89 = load ptr, ptr null, align 8
  %90 = icmp eq ptr null, null
  br i1 false, label %91, label %94

91:                                               ; preds = %83
  %92 = load ptr, ptr null, align 8
  %93 = call i64 null(ptr null, i64 0, i64 0, ptr null)
  unreachable

94:                                               ; preds = %83
  %95 = call i32 null(ptr null, i64 0, i64 0)
  %96 = load ptr, ptr null, align 8
  %97 = icmp eq ptr null, null
  br label %98

98:                                               ; preds = %94
  %99 = load ptr, ptr null, align 8
  %100 = call i64 null(ptr null, i64 0, i64 0, ptr null)
  unreachable

101:                                              ; No predecessors!
  %102 = load ptr, ptr null, align 8
  %103 = load ptr, ptr null, align 8
  %104 = load i64, ptr null, align 8
  %105 = load i64, ptr null, align 8
  %106 = mul i64 0, 0
  %107 = icmp eq i64 0, 0
  br i1 false, label %112, label %108

108:                                              ; preds = %101
  %109 = load ptr, ptr null, align 8
  %110 = add i64 0, 0
  %111 = and i64 0, 0
  br label %112

112:                                              ; preds = %108, %101
  %113 = load ptr, ptr null, align 8
  %114 = load ptr, ptr null, align 8
  %115 = load i32, ptr null, align 4
  %116 = load i32, ptr null, align 4
  %117 = load i32, ptr null, align 4
  %118 = sdiv i32 0, 0
  %119 = mul i32 0, 0
  %120 = call ptr @llvm.stacksave.p0()
  %121 = alloca double, i32 0, align 8
  %122 = load i32, ptr null, align 4
  %123 = icmp sgt i32 0, 0
  br label %128

124:                                              ; No predecessors!
  %125 = add i32 0, 0
  %126 = zext i32 0 to i64
  %127 = load double, ptr null, align 8
  br label %128

128:                                              ; preds = %124, %112
  %129 = phi double [ 0.000000e+00, %124 ], [ 0.000000e+00, %112 ]
  %130 = call i32 null(ptr null, ptr null, i32 0, i32 0, i32 0, i32 0, i32 0)
  %131 = call i32 @llvm.bswap.i32(i32 0)
  %132 = icmp slt i32 0, 0
  ret void

133:                                              ; No predecessors!
  %134 = zext i32 0 to i64
  %135 = zext i32 0 to i64
  %136 = add i64 0, 0
  br label %137

137:                                              ; preds = %402, %391, %133
  %138 = phi i64 [ 0, %133 ], [ 0, %402 ], [ 0, %391 ]
  %139 = load i32, ptr null, align 4
  %140 = icmp eq i32 0, 0
  br label %141

141:                                              ; preds = %137
  %142 = call i32 (ptr, ...) null(ptr null, i64 0)
  br label %143

143:                                              ; preds = %141
  %144 = load i32, ptr null, align 4
  %145 = icmp sgt i32 0, 0
  br i1 false, label %146, label %233

146:                                              ; preds = %143
  %147 = load i32, ptr null, align 4
  %148 = add i32 0, 0
  %149 = icmp slt i32 0, 0
  br label %150

150:                                              ; preds = %146
  %151 = load i32, ptr null, align 4
  %152 = icmp sgt i32 0, 0
  br i1 false, label %153, label %159

153:                                              ; preds = %150
  %154 = load ptr, ptr null, align 8
  %155 = load ptr, ptr null, align 8
  %156 = add i32 0, 0
  %157 = zext i32 0 to i64
  %158 = load i32, ptr null, align 4
  br label %159

159:                                              ; preds = %153, %150
  %160 = phi i32 [ 0, %150 ], [ 0, %153 ]
  %161 = icmp sgt i32 0, 0
  br label %171

162:                                              ; No predecessors!
  %163 = load i32, ptr null, align 4
  %164 = icmp sgt i32 0, 0
  br label %165

165:                                              ; preds = %162
  %166 = load ptr, ptr null, align 8
  %167 = load ptr, ptr null, align 8
  %168 = add i32 0, 0
  %169 = zext i32 0 to i64
  %170 = load i32, ptr null, align 4
  br label %171

171:                                              ; preds = %165, %159
  %172 = phi i32 [ 0, %165 ], [ 0, %159 ]
  %173 = load i32, ptr null, align 4
  %174 = add i32 0, 0
  %175 = icmp slt i32 0, 0
  br label %188

176:                                              ; No predecessors!
  %177 = load ptr, ptr null, align 8
  %178 = load i32, ptr null, align 4
  %179 = mul i32 0, 0
  %180 = load i32, ptr null, align 4
  %181 = call i32 null(ptr null, i32 0, i32 0, i32 0, i32 0, i32 0, ptr null)
  %182 = load ptr, ptr null, align 8
  %183 = load i32, ptr null, align 4
  %184 = mul i32 0, 0
  %185 = load i32, ptr null, align 4
  %186 = call i32 null(ptr null, i32 0, i32 0, i32 0, i32 0, i32 0, ptr null)
  %187 = load i32, ptr null, align 4
  br label %188

188:                                              ; preds = %176, %171
  %189 = phi i32 [ 0, %176 ], [ 0, %171 ]
  %190 = icmp sgt i32 0, 0
  br label %203

191:                                              ; No predecessors!
  %192 = load ptr, ptr null, align 8
  %193 = load i32, ptr null, align 4
  %194 = mul i32 0, 0
  %195 = load i32, ptr null, align 4
  %196 = call i32 null(ptr null, i32 0, i32 0, i32 0, i32 0, i32 0, ptr null)
  %197 = load ptr, ptr null, align 8
  %198 = load i32, ptr null, align 4
  %199 = mul i32 0, 0
  %200 = load i32, ptr null, align 4
  %201 = call i32 null(ptr null, i32 0, i32 0, i32 0, i32 0, i32 0, ptr null)
  %202 = load i32, ptr null, align 4
  br label %203

203:                                              ; preds = %191, %188
  %204 = phi i32 [ 0, %191 ], [ 0, %188 ]
  %205 = load i32, ptr null, align 4
  %206 = add i32 0, 0
  %207 = icmp slt i32 0, 0
  br label %220

208:                                              ; No predecessors!
  %209 = call i32 null(ptr null, ptr null)
  %210 = call i32 null(ptr null, ptr null)
  %211 = load i32, ptr null, align 4
  %212 = icmp sgt i32 0, 0
  br label %213

213:                                              ; preds = %208
  %214 = load ptr, ptr null, align 8
  %215 = load ptr, ptr null, align 8
  %216 = add i32 0, 0
  %217 = zext i32 0 to i64
  br label %218

218:                                              ; preds = %213
  %219 = load i32, ptr null, align 4
  br label %220

220:                                              ; preds = %218, %203
  %221 = phi i32 [ 0, %218 ], [ 0, %203 ]
  %222 = icmp sgt i32 0, 0
  br label %233

223:                                              ; No predecessors!
  %224 = call i32 null(ptr null, ptr null)
  %225 = call i32 null(ptr null, ptr null)
  %226 = load i32, ptr null, align 4
  %227 = icmp sgt i32 0, 0
  br label %228

228:                                              ; preds = %223
  %229 = load ptr, ptr null, align 8
  %230 = load ptr, ptr null, align 8
  %231 = add i32 0, 0
  %232 = zext i32 0 to i64
  br label %233

233:                                              ; preds = %228, %220, %143
  %234 = load i64, ptr null, align 8
  %235 = icmp eq i64 0, 0
  br label %236

236:                                              ; preds = %233
  %237 = load ptr, ptr null, align 8
  %238 = load ptr, ptr null, align 8
  %239 = add i64 0, 0
  %240 = and i64 0, 0
  %241 = load i64, ptr null, align 8
  %242 = icmp eq i64 0, 0
  br label %248

243:                                              ; No predecessors!
  %244 = load ptr, ptr null, align 8
  %245 = load ptr, ptr null, align 8
  %246 = add i64 0, 0
  %247 = and i64 0, 0
  br label %248

248:                                              ; preds = %243, %236
  %249 = load i32, ptr null, align 4
  %250 = load i32, ptr null, align 4
  %251 = icmp sgt i32 0, 0
  %252 = icmp sgt i32 0, 0
  %253 = and i1 false, false
  br label %265

254:                                              ; No predecessors!
  %255 = add i32 0, 0
  %256 = load ptr, ptr null, align 8
  %257 = load ptr, ptr null, align 8
  %258 = add i32 0, 0
  %259 = zext i32 0 to i64
  %260 = add i32 0, 0
  %261 = zext i32 0 to i64
  %262 = mul i64 0, 0
  %263 = add i64 0, 0
  %264 = zext i32 0 to i64
  br label %265

265:                                              ; preds = %254, %248
  %266 = load ptr, ptr null, align 8
  %267 = load i64, ptr null, align 8
  %268 = trunc i64 0 to i32
  %269 = mul i32 0, 0
  %270 = load i32, ptr null, align 4
  %271 = call i32 null(ptr null, i32 0, i32 0, i32 0, i32 0, i32 0, ptr null)
  %272 = load ptr, ptr null, align 8
  %273 = load i64, ptr null, align 8
  %274 = trunc i64 0 to i32
  %275 = mul i32 0, 0
  %276 = load i32, ptr null, align 4
  %277 = call i32 null(ptr null, i32 0, i32 0, i32 0, i32 0, i32 0, ptr null)
  %278 = load ptr, ptr null, align 8
  %279 = load i64, ptr null, align 8
  %280 = trunc i64 0 to i32
  %281 = mul i32 0, 0
  %282 = load i32, ptr null, align 4
  %283 = call i32 null(ptr null, i32 0, i32 0, i32 0, i32 0, i32 0, ptr null)
  %284 = load ptr, ptr null, align 8
  %285 = load i64, ptr null, align 8
  %286 = trunc i64 0 to i32
  %287 = mul i32 0, 0
  %288 = load i32, ptr null, align 4
  %289 = call i32 null(ptr null, i32 0, i32 0, i32 0, i32 0, i32 0, ptr null)
  %290 = call i32 null(ptr null, ptr null)
  %291 = call i32 null(ptr null, ptr null)
  %292 = load i64, ptr null, align 8
  %293 = icmp eq i64 0, 0
  br label %299

294:                                              ; No predecessors!
  %295 = load ptr, ptr null, align 8
  %296 = load ptr, ptr null, align 8
  %297 = add i64 0, 0
  %298 = and i64 0, 0
  br label %299

299:                                              ; preds = %294, %265
  %300 = call i32 null(ptr null, ptr null)
  %301 = call i32 null(ptr null, ptr null)
  %302 = load i64, ptr null, align 8
  %303 = icmp eq i64 0, 0
  br label %309

304:                                              ; No predecessors!
  %305 = load ptr, ptr null, align 8
  %306 = load ptr, ptr null, align 8
  %307 = add i64 0, 0
  %308 = and i64 0, 0
  br label %309

309:                                              ; preds = %304, %299
  %310 = load i32, ptr null, align 4
  %311 = icmp sgt i32 0, 0
  br label %312

312:                                              ; preds = %309
  %313 = add i32 0, 0
  %314 = load ptr, ptr null, align 8
  %315 = load ptr, ptr null, align 8
  %316 = add i32 0, 0
  %317 = zext i32 0 to i64
  %318 = mul i64 0, 0
  %319 = add i64 0, 0
  %320 = zext i32 0 to i64
  %321 = load i32, ptr null, align 4
  %322 = icmp sgt i32 0, 0
  br i1 false, label %323, label %334

323:                                              ; preds = %312
  %324 = add i32 0, 0
  %325 = load i32, ptr null, align 4
  %326 = load ptr, ptr null, align 8
  %327 = load ptr, ptr null, align 8
  %328 = add i32 0, 0
  %329 = zext i32 0 to i64
  %330 = mul i64 0, 0
  %331 = add i64 0, 0
  %332 = zext i32 0 to i64
  %333 = zext i32 0 to i64
  br label %334

334:                                              ; preds = %323, %312
  %335 = load i32, ptr null, align 4
  %336 = icmp eq i32 0, 0
  %337 = load ptr, ptr null, align 8
  %338 = load ptr, ptr null, align 8
  %339 = load i32, ptr null, align 4
  br label %340

340:                                              ; preds = %334
  %341 = icmp sgt i32 0, 0
  br label %347

342:                                              ; No predecessors!
  %343 = zext i32 0 to i64
  %344 = shl i64 0, 0
  %345 = add i64 0, 0
  %346 = load i32, ptr null, align 4
  br label %347

347:                                              ; preds = %342, %340
  %348 = phi i32 [ 0, %340 ], [ 0, %342 ]
  %349 = add i32 0, 0
  %350 = icmp ult i32 0, 0
  br label %351

351:                                              ; preds = %347
  %352 = load ptr, ptr null, align 8
  %353 = load ptr, ptr null, align 8
  %354 = sext i32 0 to i64
  %355 = add i64 0, 0
  %356 = load i32, ptr null, align 4
  %357 = icmp sgt i32 0, 0
  br i1 false, label %358, label %371

358:                                              ; preds = %351
  %359 = load ptr, ptr null, align 8
  %360 = zext i32 0 to i64
  %361 = shl i64 0, 0
  %362 = add i64 0, 0
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr nonnull @.kmpc_loc.0.0.53, i32 10, ptr nonnull @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678, ptr %2, ptr nonnull @OFF0, i64 %135, i64 3, i64 16, i64 20, ptr %359, ptr %359, i64 0, i64 %362)
  br label %369

363:                                              ; No predecessors!
  %364 = icmp sgt i32 0, 0
  br label %365

365:                                              ; preds = %363
  %366 = zext i32 0 to i64
  %367 = shl i64 0, 0
  %368 = add i64 0, 0
  br label %369

369:                                              ; preds = %365, %358
  %370 = load i32, ptr null, align 4
  br label %371

371:                                              ; preds = %369, %351
  %372 = phi i32 [ 0, %369 ], [ 0, %351 ]
  %373 = load i32, ptr null, align 4
  %374 = load i32, ptr null, align 4
  %375 = add i32 0, 0
  %376 = icmp eq i32 0, 0
  %377 = load ptr, ptr null, align 8
  %378 = load ptr, ptr null, align 8
  %379 = load i32, ptr null, align 4
  %380 = add i32 0, 0
  %381 = add i32 0, 0
  br label %382

382:                                              ; preds = %371
  %383 = icmp sgt i32 0, 0
  br i1 false, label %384, label %391

384:                                              ; preds = %382
  %385 = zext i32 0 to i64
  %386 = shl i64 0, 0
  %387 = add i64 0, 0
  %388 = zext i32 0 to i64
  %389 = zext i32 0 to i64
  %390 = load i32, ptr null, align 4
  br label %391

391:                                              ; preds = %384, %382
  %392 = phi i32 [ 0, %382 ], [ 0, %384 ]
  %393 = add i32 0, 0
  %394 = icmp ult i32 0, 0
  br label %137

395:                                              ; No predecessors!
  %396 = load ptr, ptr null, align 8
  %397 = load ptr, ptr null, align 8
  %398 = sext i32 0 to i64
  %399 = add i64 0, 0
  %400 = load i32, ptr null, align 4
  %401 = icmp sgt i32 0, 0
  br label %402

402:                                              ; preds = %395
  %403 = load i32, ptr null, align 4
  %404 = add i32 0, 0
  %405 = add i32 0, 0
  %406 = load ptr, ptr null, align 8
  %407 = zext i32 0 to i64
  %408 = shl i64 0, 0
  %409 = add i64 0, 0
  %410 = zext i32 0 to i64
  %411 = zext i32 0 to i64
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr nonnull @.kmpc_loc.0.0.53, i32 10, ptr nonnull @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678, ptr %2, ptr nonnull @OFF0, i64 %135, i64 3, i64 %410, i64 %411, ptr %406, ptr %406, i64 0, i64 %409)
  br label %137
}

declare !callback !0 void @__kmpc_fork_call(ptr, i32, ptr, ...)

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave.p0() #0

define internal void @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678(ptr %0, ptr %1, ptr %2, ptr %3, i64 %4, i64 %5, i64 %6, i64 %7, ptr %8, ptr %9, i64 %10, i64 %11) {
  %13 = trunc i64 0 to i32
  %14 = trunc i64 0 to i32
  %15 = alloca i32, i32 0, align 4
  %16 = alloca i64, i32 0, align 8
  %17 = alloca i64, i32 0, align 8
  %18 = alloca i64, i32 0, align 8
  %19 = load i32, ptr null, align 4
  %20 = load i64, ptr null, align 8
  %21 = load i64, ptr null, align 8
  %22 = icmp ugt i64 0, 0
  br label %23

23:                                               ; preds = %12
  %24 = freeze i64 0
  %25 = sub i64 0, 0
  %26 = and i64 0, 0
  %27 = load i64, ptr null, align 8
  %28 = load i32, ptr null, align 4
  %29 = load i64, ptr null, align 8
  br label %30

30:                                               ; preds = %330, %23
  %31 = phi i64 [ 0, %330 ], [ 0, %23 ]
  %32 = udiv i64 0, 0
  %33 = add i64 0, 0
  %34 = srem i64 0, 0
  %35 = trunc i64 0 to i32
  %36 = add i32 0, 0
  %37 = shl i64 0, 0
  %38 = ashr i64 0, 0
  %39 = mul i64 0, 0
  %40 = sext i32 0 to i64
  %41 = add i64 0, 0
  %42 = add i32 0, 0
  %43 = add i32 0, 0
  %44 = sub i32 0, 0
  %45 = add i32 0, 0
  %46 = or i32 0, 0
  %47 = lshr i32 0, 0
  %48 = sitofp i32 0 to double
  br label %49

49:                                               ; preds = %49, %30
  %50 = phi i64 [ 0, %30 ], [ 0, %49 ]
  %51 = phi double [ 0.000000e+00, %30 ], [ 0.000000e+00, %49 ]
  %52 = phi double [ 0.000000e+00, %30 ], [ 0.000000e+00, %49 ]
  %53 = phi double [ 0.000000e+00, %30 ], [ 0.000000e+00, %49 ]
  %54 = mul i64 0, 0
  %55 = mul i64 0, 0
  %56 = getelementptr i64, ptr %3, i64 %50
  %57 = load i64, ptr %56, align 8
  %58 = add i64 0, 0
  %59 = or i64 %57, 0
  %60 = getelementptr double, ptr %9, i64 %59
  %61 = load double, ptr %60, align 8
  %62 = fadd double 0.000000e+00, 0.000000e+00
  %63 = getelementptr %struct.par_t, ptr %2, i64 0, i32 1, i64 %50
  %64 = load double, ptr null, align 8
  %65 = fmul double 0.000000e+00, 0.000000e+00
  %66 = fadd double 0.000000e+00, 0.000000e+00
  %67 = getelementptr %struct.par_t, ptr %2, i64 0, i32 2, i64 %50
  %68 = load double, ptr null, align 8
  %69 = fmul double 0.000000e+00, 0.000000e+00
  %70 = fadd double 0.000000e+00, 0.000000e+00
  %71 = add i64 0, 0
  %72 = icmp eq i64 0, 0
  br label %49

73:                                               ; No predecessors!
  %74 = fdiv double 0.000000e+00, 0.000000e+00
  %75 = fmul double 0.000000e+00, 0.000000e+00
  %76 = fmul double 0.000000e+00, 0.000000e+00
  br label %77

77:                                               ; preds = %77, %73
  %78 = phi i64 [ 0, %73 ], [ 0, %77 ]
  %79 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %80 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %81 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %82 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %83 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %84 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %85 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %86 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %87 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %88 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %89 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %90 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %91 = phi double [ 0.000000e+00, %73 ], [ 0.000000e+00, %77 ]
  %92 = mul i64 0, 0
  %93 = mul i64 0, 0
  %94 = getelementptr i64, ptr %3, i64 %78
  %95 = load i64, ptr null, align 8
  %96 = add i64 0, 0
  %97 = add i64 0, 0
  %98 = getelementptr double, ptr %9, i64 %97
  %99 = load double, ptr null, align 8
  %100 = getelementptr %struct.par_t, ptr %2, i64 0, i32 1, i64 %78
  %101 = load double, ptr null, align 8
  %102 = fsub double 0.000000e+00, 0.000000e+00
  %103 = fmul double 0.000000e+00, 0.000000e+00
  %104 = getelementptr %struct.par_t, ptr %2, i64 0, i32 2, i64 %78
  %105 = load double, ptr null, align 8
  %106 = fsub double 0.000000e+00, 0.000000e+00
  %107 = fmul double 0.000000e+00, 0.000000e+00
  %108 = fadd double 0.000000e+00, 0.000000e+00
  %109 = fmul double 0.000000e+00, 0.000000e+00
  %110 = fmul double 0.000000e+00, 0.000000e+00
  %111 = fadd double 0.000000e+00, 0.000000e+00
  %112 = getelementptr %struct.par_t, ptr %2, i64 0, i32 6, i64 %78
  %113 = load double, ptr null, align 8
  %114 = fmul double 0.000000e+00, 0.000000e+00
  %115 = fadd double 0.000000e+00, 0.000000e+00
  %116 = getelementptr %struct.par_t, ptr %2, i64 0, i32 7, i64 %78
  %117 = load double, ptr null, align 8
  %118 = fmul double 0.000000e+00, 0.000000e+00
  %119 = fadd double 0.000000e+00, 0.000000e+00
  %120 = getelementptr %struct.par_t, ptr %2, i64 0, i32 8, i64 %78
  %121 = load double, ptr null, align 8
  %122 = fmul double 0.000000e+00, 0.000000e+00
  %123 = fadd double 0.000000e+00, 0.000000e+00
  %124 = getelementptr %struct.par_t, ptr %2, i64 0, i32 9, i64 %78
  %125 = load double, ptr null, align 8
  %126 = fmul double 0.000000e+00, 0.000000e+00
  %127 = fadd double 0.000000e+00, 0.000000e+00
  %128 = getelementptr %struct.par_t, ptr %2, i64 0, i32 10, i64 %78
  %129 = load double, ptr null, align 8
  %130 = fmul double 0.000000e+00, 0.000000e+00
  %131 = fadd double 0.000000e+00, 0.000000e+00
  %132 = getelementptr %struct.par_t, ptr %2, i64 0, i32 11, i64 %78
  %133 = load double, ptr null, align 8
  %134 = fmul double 0.000000e+00, 0.000000e+00
  %135 = fadd double 0.000000e+00, 0.000000e+00
  %136 = getelementptr %struct.par_t, ptr %2, i64 0, i32 12, i64 %78
  %137 = load double, ptr null, align 8
  %138 = fmul double 0.000000e+00, 0.000000e+00
  %139 = fadd double 0.000000e+00, 0.000000e+00
  %140 = getelementptr %struct.par_t, ptr %2, i64 0, i32 13, i64 %78
  %141 = load double, ptr null, align 8
  %142 = fmul double 0.000000e+00, 0.000000e+00
  %143 = fadd double 0.000000e+00, 0.000000e+00
  %144 = getelementptr %struct.par_t, ptr %2, i64 0, i32 14, i64 %78
  %145 = load double, ptr null, align 8
  %146 = fmul double 0.000000e+00, 0.000000e+00
  %147 = fadd double 0.000000e+00, 0.000000e+00
  %148 = getelementptr %struct.par_t, ptr %2, i64 0, i32 15, i64 %78
  %149 = load double, ptr null, align 8
  %150 = fmul double 0.000000e+00, 0.000000e+00
  %151 = fadd double 0.000000e+00, 0.000000e+00
  %152 = getelementptr %struct.par_t, ptr %2, i64 0, i32 16, i64 %78
  %153 = load double, ptr null, align 8
  %154 = fmul double 0.000000e+00, 0.000000e+00
  %155 = fadd double 0.000000e+00, 0.000000e+00
  %156 = getelementptr %struct.par_t, ptr %2, i64 0, i32 17, i64 %78
  %157 = load double, ptr null, align 8
  %158 = fmul double 0.000000e+00, 0.000000e+00
  %159 = fadd double 0.000000e+00, 0.000000e+00
  %160 = add i64 0, 0
  %161 = icmp eq i64 0, 0
  br label %77

162:                                              ; No predecessors!
  %163 = fsub double 0.000000e+00, 0.000000e+00
  %164 = fmul double 0.000000e+00, 0.000000e+00
  %165 = fdiv double 0.000000e+00, 0.000000e+00
  %166 = fadd double 0.000000e+00, 0.000000e+00
  %167 = fmul double 0.000000e+00, 0.000000e+00
  %168 = fadd double 0.000000e+00, 0.000000e+00
  %169 = fmul double 0.000000e+00, 0.000000e+00
  %170 = fmul double 0.000000e+00, 0.000000e+00
  %171 = fmul double 0.000000e+00, 0.000000e+00
  %172 = fadd double 0.000000e+00, 0.000000e+00
  %173 = fmul double 0.000000e+00, 0.000000e+00
  %174 = fsub double 0.000000e+00, 0.000000e+00
  %175 = fmul double 0.000000e+00, 0.000000e+00
  %176 = fmul double 0.000000e+00, 0.000000e+00
  %177 = fmul double 0.000000e+00, 0.000000e+00
  %178 = fmul double 0.000000e+00, 0.000000e+00
  %179 = fmul double 0.000000e+00, 0.000000e+00
  %180 = fadd double 0.000000e+00, 0.000000e+00
  %181 = fadd double 0.000000e+00, 0.000000e+00
  %182 = fadd double 0.000000e+00, 0.000000e+00
  %183 = fmul double 0.000000e+00, 0.000000e+00
  %184 = fmul double 0.000000e+00, 0.000000e+00
  %185 = fmul double 0.000000e+00, 0.000000e+00
  %186 = fmul double 0.000000e+00, 0.000000e+00
  %187 = fmul double 0.000000e+00, 0.000000e+00
  %188 = fmul double 0.000000e+00, 0.000000e+00
  %189 = fdiv double 0.000000e+00, 0.000000e+00
  %190 = fmul double 0.000000e+00, 0.000000e+00
  br label %191

191:                                              ; preds = %191, %162
  %192 = phi i64 [ 0, %162 ], [ 0, %191 ]
  %193 = getelementptr %struct.par_t, ptr %2, i64 0, i32 1, i64 %192
  %194 = load double, ptr null, align 8
  %195 = fmul double 0.000000e+00, 0.000000e+00
  %196 = getelementptr %struct.par_t, ptr %2, i64 0, i32 2, i64 %192
  %197 = load double, ptr null, align 8
  %198 = fmul double 0.000000e+00, 0.000000e+00
  %199 = fadd double 0.000000e+00, 0.000000e+00
  %200 = fmul double 0.000000e+00, 0.000000e+00
  %201 = fmul double 0.000000e+00, 0.000000e+00
  %202 = fmul double 0.000000e+00, 0.000000e+00
  %203 = fadd double 0.000000e+00, 0.000000e+00
  %204 = getelementptr %struct.par_t, ptr %2, i64 0, i32 6, i64 %192
  %205 = load double, ptr null, align 8
  %206 = fmul double 0.000000e+00, 0.000000e+00
  %207 = getelementptr %struct.par_t, ptr %2, i64 0, i32 7, i64 %192
  %208 = load double, ptr null, align 8
  %209 = fmul double 0.000000e+00, 0.000000e+00
  %210 = fadd double 0.000000e+00, 0.000000e+00
  %211 = getelementptr %struct.par_t, ptr %2, i64 0, i32 8, i64 %192
  %212 = load double, ptr null, align 8
  %213 = fmul double 0.000000e+00, 0.000000e+00
  %214 = fadd double 0.000000e+00, 0.000000e+00
  %215 = getelementptr %struct.par_t, ptr %2, i64 0, i32 9, i64 %192
  %216 = load double, ptr null, align 8
  %217 = fmul double 0.000000e+00, 0.000000e+00
  %218 = getelementptr %struct.par_t, ptr %2, i64 0, i32 10, i64 %192
  %219 = load double, ptr null, align 8
  %220 = fmul double 0.000000e+00, 0.000000e+00
  %221 = getelementptr %struct.par_t, ptr %2, i64 0, i32 11, i64 %192
  %222 = load double, ptr null, align 8
  %223 = fmul double 0.000000e+00, 0.000000e+00
  %224 = getelementptr %struct.par_t, ptr %2, i64 0, i32 12, i64 %192
  %225 = load double, ptr null, align 8
  %226 = fmul double 0.000000e+00, 0.000000e+00
  %227 = getelementptr %struct.par_t, ptr %2, i64 0, i32 13, i64 %192
  %228 = load double, ptr null, align 8
  %229 = fmul double 0.000000e+00, 0.000000e+00
  %230 = getelementptr %struct.par_t, ptr %2, i64 0, i32 14, i64 %192
  %231 = load double, ptr null, align 8
  %232 = fmul double 0.000000e+00, 0.000000e+00
  %233 = getelementptr %struct.par_t, ptr %2, i64 0, i32 15, i64 %192
  %234 = load double, ptr null, align 8
  %235 = fmul double 0.000000e+00, 0.000000e+00
  %236 = getelementptr %struct.par_t, ptr %2, i64 0, i32 16, i64 %192
  %237 = load double, ptr null, align 8
  %238 = fmul double 0.000000e+00, 0.000000e+00
  %239 = getelementptr %struct.par_t, ptr %2, i64 0, i32 17, i64 %192
  %240 = load double, ptr null, align 8
  %241 = fmul double 0.000000e+00, 0.000000e+00
  %242 = fsub double 0.000000e+00, 0.000000e+00
  %243 = fadd double 0.000000e+00, 0.000000e+00
  %244 = fmul double 0.000000e+00, 0.000000e+00
  %245 = fadd double 0.000000e+00, 0.000000e+00
  %246 = fadd double 0.000000e+00, 0.000000e+00
  %247 = fmul double 0.000000e+00, 0.000000e+00
  %248 = fsub double 0.000000e+00, 0.000000e+00
  %249 = fmul double 0.000000e+00, 0.000000e+00
  %250 = fadd double 0.000000e+00, 0.000000e+00
  %251 = fsub double 0.000000e+00, 0.000000e+00
  %252 = fmul double 0.000000e+00, 0.000000e+00
  %253 = fadd double 0.000000e+00, 0.000000e+00
  %254 = fmul double 0.000000e+00, 0.000000e+00
  %255 = fmul double 0.000000e+00, 0.000000e+00
  %256 = fmul double 0.000000e+00, 0.000000e+00
  %257 = fsub double 0.000000e+00, 0.000000e+00
  %258 = fmul double 0.000000e+00, 0.000000e+00
  %259 = fadd double 0.000000e+00, 0.000000e+00
  %260 = fmul double 0.000000e+00, 0.000000e+00
  %261 = fadd double 0.000000e+00, 0.000000e+00
  %262 = fmul double 0.000000e+00, 0.000000e+00
  %263 = fadd double 0.000000e+00, 0.000000e+00
  %264 = fmul double 0.000000e+00, 0.000000e+00
  %265 = fadd double 0.000000e+00, 0.000000e+00
  %266 = fmul double 0.000000e+00, 0.000000e+00
  %267 = fsub double 0.000000e+00, 0.000000e+00
  %268 = fmul double 0.000000e+00, 0.000000e+00
  %269 = fmul double 0.000000e+00, 0.000000e+00
  %270 = fmul double 0.000000e+00, 0.000000e+00
  %271 = fsub double 0.000000e+00, 0.000000e+00
  %272 = fneg double 0.000000e+00
  %273 = fmul double 0.000000e+00, 0.000000e+00
  %274 = fmul double 0.000000e+00, 0.000000e+00
  %275 = fadd double 0.000000e+00, 0.000000e+00
  %276 = fmul double 0.000000e+00, 0.000000e+00
  %277 = fadd double 0.000000e+00, 0.000000e+00
  %278 = fadd double 0.000000e+00, 0.000000e+00
  %279 = fadd double 0.000000e+00, 0.000000e+00
  %280 = fmul double 0.000000e+00, 0.000000e+00
  %281 = fmul double 0.000000e+00, 0.000000e+00
  %282 = fadd double 0.000000e+00, 0.000000e+00
  %283 = fmul double 0.000000e+00, 0.000000e+00
  %284 = fadd double 0.000000e+00, 0.000000e+00
  %285 = fmul double 0.000000e+00, 0.000000e+00
  %286 = fadd double 0.000000e+00, 0.000000e+00
  %287 = fadd double 0.000000e+00, 0.000000e+00
  %288 = fadd double 0.000000e+00, 0.000000e+00
  %289 = fadd double 0.000000e+00, 0.000000e+00
  %290 = fmul double 0.000000e+00, 0.000000e+00
  %291 = fadd double 0.000000e+00, 0.000000e+00
  %292 = mul i64 0, 0
  %293 = mul i64 0, 0
  %294 = getelementptr i64, ptr %3, i64 %192
  %295 = load i64, ptr null, align 8
  %296 = add i64 0, 0
  %297 = add i64 0, 0
  %298 = getelementptr double, ptr %9, i64 %297
  %299 = load double, ptr null, align 8
  %300 = getelementptr %struct.par_t, ptr %2, i64 0, i32 0, i64 %192
  %301 = load double, ptr null, align 8
  %302 = fmul double 0.000000e+00, 0.000000e+00
  %303 = fsub double 0.000000e+00, 0.000000e+00
  %304 = add i64 0, 0
  %305 = icmp eq i64 0, 0
  br i1 false, label %306, label %191

306:                                              ; preds = %306, %191
  %307 = phi i64 [ 0, %306 ], [ 0, %191 ]
  %308 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %309 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %310 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %311 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %312 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %313 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %314 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %315 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %316 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %317 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %318 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %319 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %320 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %321 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %322 = phi double [ 0.000000e+00, %306 ], [ 0.000000e+00, %191 ]
  %323 = mul i64 0, 0
  %324 = mul i64 0, 0
  %325 = getelementptr i64, ptr %3, i64 %307
  %326 = load i64, ptr null, align 8
  %327 = add i64 0, 0
  %328 = add i64 0, 0
  %329 = getelementptr inbounds double, ptr %9, i64 %328
  br label %306

330:                                              ; No predecessors!
  br label %30
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.bswap.i32(i32) #1

; uselistorder directives
uselistorder ptr @__kmpc_fork_call, { 1, 0 }
uselistorder ptr @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678, { 1, 0 }

attributes #0 = { nocallback nofree nosync nounwind willreturn }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i1 true}
; end INTEL_FEATURE_SW_ADVANCED