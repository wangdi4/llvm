; REQUIRES: asserts

; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec" -print-before=hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec -debug-only=hir-loop-distribute -hir-loop-distribute-skip-vectorization-profitability-check=true -disable-output < %s 2>&1 | FileCheck %s

; Verify that when distribution bails out in scalar expansion analysis after
; modifiyng HIR, it restores it so the before/after HIR is the same.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, %t4183 + -1, 1   <DO_LOOP>
; CHECK: |   switch(%t4284)
; CHECK: |   {
; CHECK: |   case 1:
; CHECK: |      %t4314 = sitofp.i32.float(i1);
; CHECK: |      %t4315 = %t4176  +  %t4314;
; CHECK: |      %t4302 = %t4295;
; CHECK: |      %t4303 = %t4315;
; CHECK: |      break;
; CHECK: |   case 2:
; CHECK: |      %t4311 = sitofp.i32.float(i1);
; CHECK: |      %t4312 = %t4176  +  %t4311;
; CHECK: |      %t4302 = %t4312;
; CHECK: |      %t4303 = %t4294;
; CHECK: |      break;
; CHECK: |   case 3:
; CHECK: |      %t4308 = sitofp.i32.float(i1);
; CHECK: |      %t4309 = %t4176  -  %t4308;
; CHECK: |      %t4302 = %t4294;
; CHECK: |      %t4303 = %t4309;
; CHECK: |      break;
; CHECK: |   case 0:
; CHECK: |      %t4305 = sitofp.i32.float(i1);
; CHECK: |      %t4306 = %t4176  -  %t4305;
; CHECK: |      %t4302 = %t4306;
; CHECK: |      %t4303 = %t4295;
; CHECK: |      break;
; CHECK: |   default:
; CHECK: |      break;
; CHECK: |   }
; CHECK: |   %t4319 = @__isnanf(%t4303);
; CHECK: |   if (%t4319 == 0)
; CHECK: |   {
; CHECK: |      %t4322 = @__isnanf(%t4302);
; CHECK: |      if (%t4322 == 0)
; CHECK: |      {
; CHECK: |         %t4327 = (%t4303 <u %t4174) ? %t4303 : %t4178;
; CHECK: |         %t4328 = (%t4303 < 0.000000e+00) ? 0.000000e+00 : %t4327;
; CHECK: |         %t4331 = (%t4302 <u %t4174) ? %t4302 : %t4178;
; CHECK: |         %t4332 = (%t4302 < 0.000000e+00) ? 0.000000e+00 : %t4331;
; CHECK: |         %t4333 = @llvm.floor.f32(%t4328);
; CHECK: |         %t4334 = fptosi.float.i32(%t4333);
; CHECK: |         %t4335 = @llvm.floor.f32(%t4332);
; CHECK: |         %t4336 = fptosi.float.i32(%t4335);
; CHECK: |         %t4340 = (%t4334 + 1 < %t4160) ? %t4334 + 1 : %t4177;
; CHECK: |         %t4342 = (%t4336 + 1 < %t4160) ? %t4336 + 1 : %t4177;
; CHECK: |         %t4343 = sitofp.i32.float(%t4334);
; CHECK: |         %t4344 = %t4328  -  %t4343;
; CHECK: |         %t4345 = sitofp.i32.float(%t4336);
; CHECK: |         %t4346 = %t4332  -  %t4345;
; CHECK: |         %t4347 = 1.000000e+00  -  %t4344;
; CHECK: |         %t4353 = (%t4180)[%t4334 + (%t4160 * %t4336)][0]  *  %t4347;
; CHECK: |         %t4356 = (%t4180)[%t4334 + (%t4160 * %t4336)][1]  *  %t4347;
; CHECK: |         %t4359 = (%t4180)[%t4334 + (%t4160 * %t4336)][2]  *  %t4347;
; CHECK: |         %t4364 = (%t4180)[(%t4160 * %t4336) + %t4340][0]  *  %t4344;
; CHECK: |         %t4367 = (%t4180)[(%t4160 * %t4336) + %t4340][1]  *  %t4344;
; CHECK: |         %t4370 = (%t4180)[(%t4160 * %t4336) + %t4340][2]  *  %t4344;
; CHECK: |         %t4376 = (%t4180)[%t4334 + (%t4160 * %t4342)][0]  *  %t4347;
; CHECK: |         %t4379 = (%t4180)[%t4334 + (%t4160 * %t4342)][1]  *  %t4347;
; CHECK: |         %t4382 = (%t4180)[%t4334 + (%t4160 * %t4342)][2]  *  %t4347;
; CHECK: |         %t4387 = (%t4180)[%t4340 + (%t4160 * %t4342)][0]  *  %t4344;
; CHECK: |         %t4390 = (%t4180)[%t4340 + (%t4160 * %t4342)][1]  *  %t4344;
; CHECK: |         %t4393 = (%t4180)[%t4340 + (%t4160 * %t4342)][2]  *  %t4344;
; CHECK: |         %t4394 = %t4364  +  %t4353;
; CHECK: |         %t4395 = %t4367  +  %t4356;
; CHECK: |         %t4396 = %t4370  +  %t4359;
; CHECK: |         %t4397 = %t4387  +  %t4376;
; CHECK: |         %t4398 = %t4390  +  %t4379;
; CHECK: |         %t4399 = %t4393  +  %t4382;
; CHECK: |         %t4407 = %t4397  +  %t4394;
; CHECK: |         (%t4293)[i1][0] = %t4407;
; CHECK: |         %t4408 = %t4398  +  %t4395;
; CHECK: |         (%t4293)[i1][1] = %t4408;
; CHECK: |         %t4410 = %t4399  +  %t4396;
; CHECK: |         (%t4293)[i1][2] = %t4410;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   if (%t4287 != 0)
; CHECK: |   {
; CHECK: |      %t4432 =  - (%t4293)[i1][1];
; CHECK: |      (%t4293)[i1][1] = %t4432;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      if (%t4288 != 0)
; CHECK: |      {
; CHECK: |         %t4426 = (%t4293)[i1][0];
; CHECK: |         (%t4293)[i1][0] = (%t4293)[i1][1];
; CHECK: |         (%t4293)[i1][1] = %t4426;
; CHECK: |      }
; CHECK: |      else
; CHECK: |      {
; CHECK: |         if (%t4289 != 0)
; CHECK: |         {
; CHECK: |            %t4424 =  - (%t4293)[i1][0];
; CHECK: |            (%t4293)[i1][0] = %t4424;
; CHECK: |         }
; CHECK: |         else
; CHECK: |         {
; CHECK: |            if (%t4290 != 0)
; CHECK: |            {
; CHECK: |               %t4417 = (%t4293)[i1][0];
; CHECK: |               %t4420 =  - (%t4293)[i1][1];
; CHECK: |               (%t4293)[i1][0] = %t4420;
; CHECK: |               %t4421 =  - %t4417;
; CHECK: |               (%t4293)[i1][1] = %t4421;
; CHECK: |            }
; CHECK: |         }
; CHECK: |      }
; CHECK: |   }
; CHECK: + END LOOP

; Verify bailout point.

; CHECK: [SCEX] TmpDef inside Switch!
; CHECK: LOOP DISTRIBUTION: Distribution disabled due to Scalar Expansion analysis

; CHECK: Dump After

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, %t4183 + -1, 1   <DO_LOOP>
; CHECK: |   switch(%t4284)
; CHECK: |   {
; CHECK: |   case 1:
; CHECK: |      %t4314 = sitofp.i32.float(i1);
; CHECK: |      %t4315 = %t4176  +  %t4314;
; CHECK: |      %t4302 = %t4295;
; CHECK: |      %t4303 = %t4315;
; CHECK: |      break;
; CHECK: |   case 2:
; CHECK: |      %t4311 = sitofp.i32.float(i1);
; CHECK: |      %t4312 = %t4176  +  %t4311;
; CHECK: |      %t4302 = %t4312;
; CHECK: |      %t4303 = %t4294;
; CHECK: |      break;
; CHECK: |   case 3:
; CHECK: |      %t4308 = sitofp.i32.float(i1);
; CHECK: |      %t4309 = %t4176  -  %t4308;
; CHECK: |      %t4302 = %t4294;
; CHECK: |      %t4303 = %t4309;
; CHECK: |      break;
; CHECK: |   case 0:
; CHECK: |      %t4305 = sitofp.i32.float(i1);
; CHECK: |      %t4306 = %t4176  -  %t4305;
; CHECK: |      %t4302 = %t4306;
; CHECK: |      %t4303 = %t4295;
; CHECK: |      break;
; CHECK: |   default:
; CHECK: |      break;
; CHECK: |   }
; CHECK: |   %t4319 = @__isnanf(%t4303);
; CHECK: |   if (%t4319 == 0)
; CHECK: |   {
; CHECK: |      %t4322 = @__isnanf(%t4302);
; CHECK: |      if (%t4322 == 0)
; CHECK: |      {
; CHECK: |         %t4327 = (%t4303 <u %t4174) ? %t4303 : %t4178;
; CHECK: |         %t4328 = (%t4303 < 0.000000e+00) ? 0.000000e+00 : %t4327;
; CHECK: |         %t4331 = (%t4302 <u %t4174) ? %t4302 : %t4178;
; CHECK: |         %t4332 = (%t4302 < 0.000000e+00) ? 0.000000e+00 : %t4331;
; CHECK: |         %t4333 = @llvm.floor.f32(%t4328);
; CHECK: |         %t4334 = fptosi.float.i32(%t4333);
; CHECK: |         %t4335 = @llvm.floor.f32(%t4332);
; CHECK: |         %t4336 = fptosi.float.i32(%t4335);
; CHECK: |         %t4340 = (%t4334 + 1 < %t4160) ? %t4334 + 1 : %t4177;
; CHECK: |         %t4342 = (%t4336 + 1 < %t4160) ? %t4336 + 1 : %t4177;
; CHECK: |         %t4343 = sitofp.i32.float(%t4334);
; CHECK: |         %t4344 = %t4328  -  %t4343;
; CHECK: |         %t4345 = sitofp.i32.float(%t4336);
; CHECK: |         %t4346 = %t4332  -  %t4345;
; CHECK: |         %t4347 = 1.000000e+00  -  %t4344;
; CHECK: |         %t4353 = (%t4180)[%t4334 + (%t4160 * %t4336)][0]  *  %t4347;
; CHECK: |         %t4356 = (%t4180)[%t4334 + (%t4160 * %t4336)][1]  *  %t4347;
; CHECK: |         %t4359 = (%t4180)[%t4334 + (%t4160 * %t4336)][2]  *  %t4347;
; CHECK: |         %t4364 = (%t4180)[(%t4160 * %t4336) + %t4340][0]  *  %t4344;
; CHECK: |         %t4367 = (%t4180)[(%t4160 * %t4336) + %t4340][1]  *  %t4344;
; CHECK: |         %t4370 = (%t4180)[(%t4160 * %t4336) + %t4340][2]  *  %t4344;
; CHECK: |         %t4376 = (%t4180)[%t4334 + (%t4160 * %t4342)][0]  *  %t4347;
; CHECK: |         %t4379 = (%t4180)[%t4334 + (%t4160 * %t4342)][1]  *  %t4347;
; CHECK: |         %t4382 = (%t4180)[%t4334 + (%t4160 * %t4342)][2]  *  %t4347;
; CHECK: |         %t4387 = (%t4180)[%t4340 + (%t4160 * %t4342)][0]  *  %t4344;
; CHECK: |         %t4390 = (%t4180)[%t4340 + (%t4160 * %t4342)][1]  *  %t4344;
; CHECK: |         %t4393 = (%t4180)[%t4340 + (%t4160 * %t4342)][2]  *  %t4344;
; CHECK: |         %t4394 = %t4364  +  %t4353;
; CHECK: |         %t4395 = %t4367  +  %t4356;
; CHECK: |         %t4396 = %t4370  +  %t4359;
; CHECK: |         %t4397 = %t4387  +  %t4376;
; CHECK: |         %t4398 = %t4390  +  %t4379;
; CHECK: |         %t4399 = %t4393  +  %t4382;
; CHECK: |         %t4407 = %t4397  +  %t4394;
; CHECK: |         (%t4293)[i1][0] = %t4407;
; CHECK: |         %t4408 = %t4398  +  %t4395;
; CHECK: |         (%t4293)[i1][1] = %t4408;
; CHECK: |         %t4410 = %t4399  +  %t4396;
; CHECK: |         (%t4293)[i1][2] = %t4410;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   if (%t4287 != 0)
; CHECK: |   {
; CHECK: |      %t4432 =  - (%t4293)[i1][1];
; CHECK: |      (%t4293)[i1][1] = %t4432;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      if (%t4288 != 0)
; CHECK: |      {
; CHECK: |         %t4426 = (%t4293)[i1][0];
; CHECK: |         (%t4293)[i1][0] = (%t4293)[i1][1];
; CHECK: |         (%t4293)[i1][1] = %t4426;
; CHECK: |      }
; CHECK: |      else
; CHECK: |      {
; CHECK: |         if (%t4289 != 0)
; CHECK: |         {
; CHECK: |            %t4424 =  - (%t4293)[i1][0];
; CHECK: |            (%t4293)[i1][0] = %t4424;
; CHECK: |         }
; CHECK: |         else
; CHECK: |         {
; CHECK: |            if (%t4290 != 0)
; CHECK: |            {
; CHECK: |               %t4417 = (%t4293)[i1][0];
; CHECK: |               %t4420 =  - (%t4293)[i1][1];
; CHECK: |               (%t4293)[i1][0] = %t4420;
; CHECK: |               %t4421 =  - %t4417;
; CHECK: |               (%t4293)[i1][1] = %t4421;
; CHECK: |            }
; CHECK: |         }
; CHECK: |      }
; CHECK: |   }
; CHECK: + END LOOP

define void @foo(ptr noalias %t4180, ptr noalias %t4293, float %t4294, float %t4295, i32 %t4160, float %t4174, float %t4176, i32 %t4177, float %t4178, i32 %t4183, i32 %t4284, i1 %t4287, i1 %t4288, i1 %t4289, i1 %t4290) {
entry:
  br label %loop

loop:                                             ; preds = %latch, %entry
  %t4300 = phi i32 [ 0, %entry ], [ %t4434, %latch ]
  %t4301 = phi ptr [ %t4293, %entry ], [ %t4435, %latch ]
  %t4302 = phi float [ %t4294, %entry ], [ %t4318, %latch ]
  %t4303 = phi float [ %t4295, %entry ], [ %t4317, %latch ]
  switch i32 %t4284, label %t4316 [
    i32 1, label %t4313
    i32 2, label %t4310
    i32 3, label %t4307
    i32 0, label %t4304
  ]

t4304:                                             ; preds = %loop
  %t4305 = sitofp i32 %t4300 to float
  %t4306 = fsub fast float %t4176, %t4305
  br label %t4316

t4307:                                             ; preds = %loop
  %t4308 = sitofp i32 %t4300 to float
  %t4309 = fsub fast float %t4176, %t4308
  br label %t4316

t4310:                                             ; preds = %loop
  %t4311 = sitofp i32 %t4300 to float
  %t4312 = fadd fast float %t4176, %t4311
  br label %t4316

t4313:                                             ; preds = %loop
  %t4314 = sitofp i32 %t4300 to float
  %t4315 = fadd fast float %t4176, %t4314
  br label %t4316

t4316:                                             ; preds = %t4313, %t4310, %t4307, %t4304, %loop
  %t4317 = phi float [ %t4303, %loop ], [ %t4295, %t4304 ], [ %t4309, %t4307 ], [ %t4294, %t4310 ], [ %t4315, %t4313 ]
  %t4318 = phi float [ %t4302, %loop ], [ %t4306, %t4304 ], [ %t4294, %t4307 ], [ %t4312, %t4310 ], [ %t4295, %t4313 ]
  %t4319 = call i32 @__isnanf(float noundef nofpclass(nan inf) %t4317) #14
  %t4320 = icmp eq i32 %t4319, 0
  br i1 %t4320, label %t4321, label %t4412

t4321:                                             ; preds = %t4316
  %t4322 = call i32 @__isnanf(float noundef nofpclass(nan inf) %t4318) #14
  %t4323 = icmp eq i32 %t4322, 0
  br i1 %t4323, label %t4324, label %t4412

t4324:                                             ; preds = %t4321
  %t4325 = fcmp fast olt float %t4317, 0.000000e+00
  %t4326 = fcmp fast ult float %t4317, %t4174
  %t4327 = select i1 %t4326, float %t4317, float %t4178
  %t4328 = select i1 %t4325, float 0.000000e+00, float %t4327
  %t4329 = fcmp fast olt float %t4318, 0.000000e+00
  %t4330 = fcmp fast ult float %t4318, %t4174
  %t4331 = select i1 %t4330, float %t4318, float %t4178
  %t4332 = select i1 %t4329, float 0.000000e+00, float %t4331
  %t4333 = call fast float @llvm.floor.f32(float %t4328)
  %t4334 = fptosi float %t4333 to i32
  %t4335 = call fast float @llvm.floor.f32(float %t4332)
  %t4336 = fptosi float %t4335 to i32
  %t4337 = add nsw i32 %t4334, 1
  %t4338 = add nsw i32 %t4336, 1
  %t4339 = icmp slt i32 %t4337, %t4160
  %t4340 = select i1 %t4339, i32 %t4337, i32 %t4177
  %t4341 = icmp slt i32 %t4338, %t4160
  %t4342 = select i1 %t4341, i32 %t4338, i32 %t4177
  %t4343 = sitofp i32 %t4334 to float
  %t4344 = fsub fast float %t4328, %t4343
  %t4345 = sitofp i32 %t4336 to float
  %t4346 = fsub fast float %t4332, %t4345
  %t4347 = fsub fast float 1.000000e+00, %t4344
  %t4348 = mul nsw i32 %t4160, %t4336
  %t4349 = add nsw i32 %t4348, %t4334
  %t4350 = sext i32 %t4349 to i64
  %t4351 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4350, i64 0
  %t4352 = load float, ptr %t4351, align 4
  %t4353 = fmul fast float %t4352, %t4347
  %t4354 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4350, i64 1
  %t4355 = load float, ptr %t4354, align 4
  %t4356 = fmul fast float %t4355, %t4347
  %t4357 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4350, i64 2
  %t4358 = load float, ptr %t4357, align 4
  %t4359 = fmul fast float %t4358, %t4347
  %t4360 = add nsw i32 %t4340, %t4348
  %t4361 = sext i32 %t4360 to i64
  %t4362 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4361, i64 0
  %t4363 = load float, ptr %t4362, align 4
  %t4364 = fmul fast float %t4363, %t4344
  %t4365 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4361, i64 1
  %t4366 = load float, ptr %t4365, align 4
  %t4367 = fmul fast float %t4366, %t4344
  %t4368 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4361, i64 2
  %t4369 = load float, ptr %t4368, align 4
  %t4370 = fmul fast float %t4369, %t4344
  %t4371 = mul nsw i32 %t4342, %t4160
  %t4372 = add nsw i32 %t4371, %t4334
  %t4373 = sext i32 %t4372 to i64
  %t4374 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4373, i64 0
  %t4375 = load float, ptr %t4374, align 4
  %t4376 = fmul fast float %t4375, %t4347
  %t4377 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4373, i64 1
  %t4378 = load float, ptr %t4377, align 4
  %t4379 = fmul fast float %t4378, %t4347
  %t4380 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4373, i64 2
  %t4381 = load float, ptr %t4380, align 4
  %t4382 = fmul fast float %t4381, %t4347
  %t4383 = add nsw i32 %t4371, %t4340
  %t4384 = sext i32 %t4383 to i64
  %t4385 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4384, i64 0
  %t4386 = load float, ptr %t4385, align 4
  %t4387 = fmul fast float %t4386, %t4344
  %t4388 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4384, i64 1
  %t4389 = load float, ptr %t4388, align 4
  %t4390 = fmul fast float %t4389, %t4344
  %t4391 = getelementptr inbounds [3 x float], ptr %t4180, i64 %t4384, i64 2
  %t4392 = load float, ptr %t4391, align 4
  %t4393 = fmul fast float %t4392, %t4344
  %t4394 = fadd fast float %t4364, %t4353
  %t4395 = fadd fast float %t4367, %t4356
  %t4396 = fadd fast float %t4370, %t4359
  %t4397 = fadd fast float %t4387, %t4376
  %t4398 = fadd fast float %t4390, %t4379
  %t4399 = fadd fast float %t4393, %t4382
  %t4407 = fadd fast float %t4397, %t4394
  store float %t4407, ptr %t4301, align 4
  %t4408 = fadd fast float %t4398, %t4395
  %t4409 = getelementptr inbounds float, ptr %t4301, i64 1
  store float %t4408, ptr %t4409, align 4
  %t4410 = fadd fast float %t4399, %t4396
  %t4411 = getelementptr inbounds float, ptr %t4301, i64 2
  store float %t4410, ptr %t4411, align 4
  br label %t4412

t4412:                                             ; preds = %t4324, %t4321, %t4316
  br i1 %t4287, label %t4429, label %t4413

t4413:                                             ; preds = %t4412
  br i1 %t4288, label %t4425, label %t4414

t4414:                                             ; preds = %t4413
  br i1 %t4289, label %t4422, label %t4415

t4415:                                             ; preds = %t4414
  br i1 %t4290, label %t4416, label %latch

t4416:                                             ; preds = %t4415
  %t4417 = load float, ptr %t4301, align 4
  %t4418 = getelementptr inbounds [3 x float], ptr %t4301, i64 0, i64 1
  %t4419 = load float, ptr %t4418, align 4
  %t4420 = fneg fast float %t4419
  store float %t4420, ptr %t4301, align 4
  %t4421 = fneg fast float %t4417
  store float %t4421, ptr %t4418, align 4
  br label %latch

t4422:                                             ; preds = %t4414
  %t4423 = load float, ptr %t4301, align 4
  %t4424 = fneg fast float %t4423
  store float %t4424, ptr %t4301, align 4
  br label %latch

t4425:                                             ; preds = %t4413
  %t4426 = load float, ptr %t4301, align 4
  %t4427 = getelementptr inbounds [3 x float], ptr %t4301, i64 0, i64 1
  %t4428 = load float, ptr %t4427, align 4
  store float %t4428, ptr %t4301, align 4
  store float %t4426, ptr %t4427, align 4
  br label %latch

t4429:                                             ; preds = %t4412
  %t4430 = getelementptr inbounds [3 x float], ptr %t4301, i64 0, i64 1
  %t4431 = load float, ptr %t4430, align 4
  %t4432 = fneg fast float %t4431
  store float %t4432, ptr %t4430, align 4
  br label %latch

latch:                                             ; preds = %t4429, %t4425, %t4422, %t4416, %t4415
  %t4434 = add nuw nsw i32 %t4300, 1
  %t4435 = getelementptr inbounds [3 x float], ptr %t4301, i64 1
  %t4436 = icmp eq i32 %t4434, %t4183
  br i1 %t4436, label %exit, label %loop

exit:                                             ; preds = %latch
  %t4438 = phi float [ %t4317, %latch ]
  %t4439 = phi float [ %t4318, %latch ]
  ret void
}

declare dso_local i32 @__isnanf(float noundef nofpclass(nan inf)) local_unnamed_addr #5

declare float @llvm.floor.f32(float) #4

attributes #4 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #5 = { mustprogress nofree nosync nounwind willreturn memory(none) }
