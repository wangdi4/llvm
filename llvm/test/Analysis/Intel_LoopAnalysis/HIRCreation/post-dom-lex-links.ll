; REQUIRES: asserts

; RUN: opt < %s -analyze -hir-creation -hir-cost-model-throttling=0 | FileCheck %s

; Check that the lexical links are correctly built for the big loop (if.end.1422). Loop latch should be the lexically last bblock.
; CHECK: BEGIN REGION
; CHECK-NEXT: if.end.1422
; CHECK: goto if.end.1422
; CHECK: goto for.end.2364.loopexit
; CHECK-NOT: goto
; CHECK: END REGION

; RUN: opt < %s -analyze -hir-creation -debug 2>&1 | FileCheck -check-prefix=COST-MODEL %s
; COST-MODEL: Loop throttled due to presence of too many nested ifs


; ModuleID = 'bugpoint-reduced-function.bc'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@failed = external global i32, align 4
@gcnt2 = external global i32, align 4
@gmask = external global i32, align 4
@.str = external unnamed_addr constant [24 x i8], align 1
@.str.1 = external unnamed_addr constant [28 x i8], align 1
@.str.2 = external unnamed_addr constant [24 x i8], align 1
@.str.3 = external unnamed_addr constant [24 x i8], align 1
@.str.4 = external unnamed_addr constant [24 x i8], align 1
@.str.5 = external unnamed_addr constant [25 x i8], align 1
@.str.6 = external unnamed_addr constant [25 x i8], align 1
@.str.7 = external unnamed_addr constant [25 x i8], align 1
@.str.8 = external unnamed_addr constant [25 x i8], align 1
@glmask = external global i64, align 8
@.str.9 = external unnamed_addr constant [37 x i8], align 1
@.str.10 = external unnamed_addr constant [8 x i8], align 1
@str = external unnamed_addr constant [7 x i8]

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_gt_32_chk(i32, i32, i32, i32) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_lt_32_chk(i32, i32, i32, i32) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_ge_32_chk(i32, i32, i32, i32) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_le_32_chk(i32, i32, i32, i32) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_ugt_32_chk(i32, i32, i32, i32) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_ult_32_chk(i32, i32, i32, i32) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_uge_32_chk(i32, i32, i32, i32) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_ule_32_chk(i32, i32, i32, i32) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_gt_64_chk(i64, i64, i32, i64) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_lt_64_chk(i64, i64, i32, i64) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_ge_64_chk(i64, i64, i32, i64) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_le_64_chk(i64, i64, i32, i64) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_ugt_64_chk(i64, i64, i32, i64) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_ult_64_chk(i64, i64, i32, i64) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_uge_64_chk(i64, i64, i32, i64) 

; Function Attrs: nounwind readnone uwtable
declare i32 @and_shr_ule_64_chk(i64, i64, i32, i64) 

; Function Attrs: nounwind uwtable
define i32 @main(i32 %argc, i8** nocapture readnone %argv)  {
entry:
  %mul = mul nsw i32 %argc, -1000
  %mul1 = mul i32 %argc, 1001
  %cmp.4083 = icmp slt i32 %mul, %mul1
  br i1 %cmp.4083, label %for.body, label %for.end.2364

for.body:                                         ; preds = %if.end.977, %for.body.preheader
  %x.04084 = phi i32 [ %inc, %if.end.977 ], [ %mul, %entry ]
  %shr325 = lshr i32 %x.04084, 31
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %cmp336 = icmp eq i32 %shr325, 1
  br i1 %cmp336, label %if.end.343.thread, label %land.lhs.true.337

if.end.343.thread:                                ; preds = %for.body
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %cmp.i.3954.old = icmp slt i32 %x.04084, 0
  br i1 %cmp.i.3954.old, label %if.end.363, label %if.then.350

land.lhs.true.337:                                ; preds = %for.body
  %cmp.i.3958 = icmp sgt i32 %x.04084, -1
  br i1 %cmp.i.3958, label %if.end.343, label %if.end.343.thread4166

if.end.343.thread4166:                            ; preds = %land.lhs.true.337
  %call341 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 340) 
  %call342 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 31, i32 1) 
  store i32 1, i32* @failed, align 4
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  br label %if.end.363

if.end.343:                                       ; preds = %land.lhs.true.337
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %cmp346 = icmp eq i32 %shr325, 0
  br i1 %cmp346, label %if.end.363, label %if.then.350

if.then.350:                                      ; preds = %if.end.343, %if.end.343.thread
  %call351 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.3, i32 0, i32 0), i32 340) 
  %call352 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 31, i32 1) 
  store i32 1, i32* @failed, align 4
  br label %if.end.363

if.end.363:                                       ; preds = %if.then.350, %if.end.343, %if.end.343.thread4166, %if.end.343.thread
  %and.i.3950 = and i32 %x.04084, -2147483648
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  br i1 %cmp336, label %if.end.383.thread, label %land.lhs.true.377

if.end.383.thread:                                ; preds = %if.end.363
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  br label %land.lhs.true.387

land.lhs.true.377:                                ; preds = %if.end.363
  %cmp.i.3948 = icmp sgt i32 %x.04084, -1
  br i1 %cmp.i.3948, label %if.end.383, label %if.then.380

if.then.380:                                      ; preds = %land.lhs.true.377
  %call381 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.6, i32 0, i32 0), i32 340) 
  %call382 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 31, i32 1) 
  store i32 1, i32* @failed, align 4
  br label %if.end.383

if.end.383:                                       ; preds = %if.then.380, %land.lhs.true.377
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %cmp386 = icmp eq i32 %shr325, 0
  br i1 %cmp386, label %if.end.403, label %land.lhs.true.387

land.lhs.true.387:                                ; preds = %if.end.383, %if.end.383.thread
  %cmp.i.3944 = icmp slt i32 %x.04084, 0
  br i1 %cmp.i.3944, label %land.lhs.true.409.thread, label %if.then.412

land.lhs.true.409.thread:                         ; preds = %land.lhs.true.387
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %cmp4084032.in.2 = bitcast i1 false to i1
  br label %if.end.435

if.end.403:                                       ; preds = %if.end.383
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %cmp4084032.in = bitcast i1 true to i1
  br label %if.end.435

if.then.412:                                      ; preds = %land.lhs.true.387
  %call391 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.7, i32 0, i32 0), i32 340) 
  %call392 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 31, i32 1) 
  store i32 1, i32* @failed, align 4
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %call413 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 341) 
  %call414 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 31, i32 0) 
  store i32 1, i32* @failed, align 4
  %cmp4084032.in.1 = bitcast i1 false to i1
  br label %if.end.435

if.end.435:                                       ; preds = %if.then.412, %if.end.403, %land.lhs.true.409.thread
  %cmp4084032 = phi i1 [ true, %if.end.403 ], [ false, %if.then.412 ], [ false, %land.lhs.true.409.thread ]
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  br i1 %cmp336, label %if.end.445.thread, label %land.lhs.true.439

if.end.445.thread:                                ; preds = %if.end.435
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %cmp.i.3928.old = icmp slt i32 %x.04084, 0
  br i1 %cmp.i.3928.old, label %if.end.475, label %if.then.452

land.lhs.true.439:                                ; preds = %if.end.435
  %cmp.i.3932 = icmp sgt i32 %x.04084, -1
  br i1 %cmp.i.3932, label %if.end.445, label %if.end.445.thread4170

if.end.445.thread4170:                            ; preds = %land.lhs.true.439
  %call443 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.4, i32 0, i32 0), i32 341) 
  %call444 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 31, i32 0) 
  store i32 1, i32* @failed, align 4
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  br label %if.end.475

if.end.445:                                       ; preds = %land.lhs.true.439
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  br i1 %cmp4084032, label %if.end.475, label %if.then.452

if.then.452:                                      ; preds = %if.end.445, %if.end.445.thread
  %call453 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.5, i32 0, i32 0), i32 341) 
  %call454 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 31, i32 0) 
  store i32 1, i32* @failed, align 4
  br label %if.end.475

if.end.475:                                       ; preds = %if.then.452, %if.end.445, %if.end.445.thread4170, %if.end.445.thread
  store i32 31, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %cmp.i.3922 = icmp sgt i32 %x.04084, -1
  %or.cond4058 = or i1 %cmp.i.3922, %cmp336
  br i1 %or.cond4058, label %if.end.567, label %if.then.482

if.then.482:                                      ; preds = %if.end.475
  %call483 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 341) 
  %call484 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 31, i32 0) 
  store i32 1, i32* @failed, align 4
  br label %if.end.567

if.end.567:                                       ; preds = %if.then.482, %if.end.475
  store i32 30, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  br i1 %cmp.i.3922, label %land.lhs.true.603, label %land.lhs.true.573

land.lhs.true.573:                                ; preds = %if.end.567
  %cmp.i.3910 = icmp eq i32 %and.i.3950, 0
  br i1 %cmp.i.3910, label %if.then.576, label %if.end.639.thread.critedge

if.then.576:                                      ; preds = %land.lhs.true.573
  %call577 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 343) 
  %call578 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 30, i32 0) 
  store i32 1, i32* @failed, align 4
  store i32 30, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %call617 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.5, i32 0, i32 0), i32 343) 
  %call618 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 30, i32 0) 
  store i32 1, i32* @failed, align 4
  br label %if.end.639.thread

land.lhs.true.603:                                ; preds = %if.end.567
  store i32 30, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %cmp.i.3904 = icmp eq i32 %and.i.3950, 0
  br i1 %cmp.i.3904, label %if.end.649.critedge, label %if.then.606

if.then.606:                                      ; preds = %land.lhs.true.603
  %call607 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.4, i32 0, i32 0), i32 343) 
  %call608 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 30, i32 0) 
  store i32 1, i32* @failed, align 4
  store i32 30, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  %call647 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 343) 
  %call648 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -2147483648, i32 30, i32 0) 
  store i32 1, i32* @failed, align 4
  br label %if.end.649

if.end.639.thread.critedge:                       ; preds = %land.lhs.true.573
  store i32 30, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  br label %if.end.639.thread

if.end.639.thread:                                ; preds = %if.end.639.thread.critedge, %if.then.576
  store i32 30, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  br label %if.end.649

if.end.649.critedge:                              ; preds = %land.lhs.true.603
  store i32 30, i32* @gcnt2, align 4
  store i32 -2147483648, i32* @gmask, align 4
  br label %if.end.649

if.end.649:                                       ; preds = %if.end.649.critedge, %if.end.639.thread, %if.then.606
  store i32 24, i32* @gcnt2, align 4
  store i32 -285212672, i32* @gmask, align 4
  %and652 = lshr i32 %x.04084, 24
  %shr653 = and i32 %and652, 239
  %cmp654 = icmp ugt i32 %shr653, 79
  br i1 %cmp654, label %land.lhs.true.655, label %if.end.661

land.lhs.true.655:                                ; preds = %if.end.649
  %shr.i.3889 = and i32 %x.04084, -536870912
  %cmp.i.3890 = icmp ugt i32 %shr.i.3889, 1325400064
  br i1 %cmp.i.3890, label %if.end.671.thread.thread4088, label %if.end.671.thread

if.end.671.thread.thread4088:                     ; preds = %land.lhs.true.655
  br label %if.end.701.thread

if.end.661:                                       ; preds = %if.end.649
  store i32 24, i32* @gcnt2, align 4
  store i32 -285212672, i32* @gmask, align 4
  %cmp664 = icmp ult i32 %shr653, 79
  br i1 %cmp664, label %land.lhs.true.665, label %if.end.691.thread

land.lhs.true.665:                                ; preds = %if.end.661
  %shr.i.3885 = and i32 %x.04084, -285212672
  %cmp.i.3886 = icmp ult i32 %shr.i.3885, 1325400064
  br i1 %cmp.i.3886, label %land.lhs.true.685.thread4092, label %land.lhs.true.685

land.lhs.true.685.thread4092:                     ; preds = %land.lhs.true.665
  br label %if.end.711

if.end.671.thread:                                ; preds = %land.lhs.true.655
  %call659 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 345) 
  %call660 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -285212672, i32 24, i32 79) 
  store i32 1, i32* @failed, align 4
  br label %if.end.701.thread

land.lhs.true.685:                                ; preds = %land.lhs.true.665
  %call669 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 345) 
  %call670 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -285212672, i32 24, i32 79) 
  store i32 1, i32* @failed, align 4
  br label %if.end.711

if.end.691.thread:                                ; preds = %if.end.661
  br label %if.end.711.thread

if.end.701.thread:                                ; preds = %if.end.671.thread, %if.end.671.thread.thread4088
  store i32 24, i32* @gcnt2, align 4
  store i32 -285212672, i32* @gmask, align 4
  br label %if.end.711.thread

if.end.711.thread:                                ; preds = %if.end.701.thread, %if.end.691.thread
  store i32 24, i32* @gcnt2, align 4
  store i32 -285212672, i32* @gmask, align 4
  store i32 24, i32* @gcnt2, align 4
  store i32 -285212672, i32* @gmask, align 4
  %shr.i.3865 = and i32 %x.04084, -285212672
  %cmp.i.3866 = icmp ugt i32 %shr.i.3865, 1308622848
  br i1 %cmp.i.3866, label %if.end.721, label %if.then.718

if.end.711:                                       ; preds = %land.lhs.true.685, %land.lhs.true.685.thread4092
  store i32 24, i32* @gcnt2, align 4
  store i32 -285212672, i32* @gmask, align 4
  store i32 24, i32* @gcnt2, align 4
  store i32 -285212672, i32* @gmask, align 4
  %.old = and i32 %x.04084, -536870912
  %cmp.i.3862.old = icmp ult i32 %.old, 1342177280
  br i1 %cmp.i.3862.old, label %if.end.731, label %if.then.728

if.then.718:                                      ; preds = %if.end.711.thread
  %call719 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.7, i32 0, i32 0), i32 345) 
  %call720 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -285212672, i32 24, i32 79) 
  store i32 1, i32* @failed, align 4
  br label %if.end.721

if.end.721:                                       ; preds = %if.then.718, %if.end.711.thread
  store i32 24, i32* @gcnt2, align 4
  store i32 -285212672, i32* @gmask, align 4
  %shr.i.3861 = and i32 %x.04084, -536870912
  %cmp.i.3862 = icmp ult i32 %shr.i.3861, 1342177280
  %or.cond4060 = or i1 %cmp.i.3862, %cmp654
  br i1 %or.cond4060, label %if.end.731, label %if.then.728

if.then.728:                                      ; preds = %if.end.721, %if.end.711
  %call729 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 345) 
  %call730 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -285212672, i32 24, i32 79) 
  store i32 1, i32* @failed, align 4
  br label %if.end.731

if.end.731:                                       ; preds = %if.then.728, %if.end.721, %if.end.711
  store i32 24, i32* @gcnt2, align 4
  store i32 -822083584, i32* @gmask, align 4
  %shr735 = and i32 %and652, 207
  %cmp736 = icmp ugt i32 %shr735, 143
  br i1 %cmp736, label %land.lhs.true.737, label %if.end.743

land.lhs.true.737:                                ; preds = %if.end.731
  %shr.i.3857 = and i32 %x.04084, -1073741824
  %cmp.i.3858 = icmp ugt i32 %shr.i.3857, -1895825408
  br i1 %cmp.i.3858, label %if.end.753.thread.thread4096, label %if.end.753.thread

if.end.753.thread.thread4096:                     ; preds = %land.lhs.true.737
  br label %if.end.783.thread

if.end.743:                                       ; preds = %if.end.731
  store i32 24, i32* @gcnt2, align 4
  store i32 -822083584, i32* @gmask, align 4
  %cmp746 = icmp ult i32 %shr735, 143
  br i1 %cmp746, label %land.lhs.true.747, label %if.end.773.thread

land.lhs.true.747:                                ; preds = %if.end.743
  %shr.i.3853 = and i32 %x.04084, -822083584
  %cmp.i.3854 = icmp ult i32 %shr.i.3853, -1895825408
  br i1 %cmp.i.3854, label %land.lhs.true.767.thread4100, label %land.lhs.true.767

land.lhs.true.767.thread4100:                     ; preds = %land.lhs.true.747
  br label %if.end.793

if.end.753.thread:                                ; preds = %land.lhs.true.737
  %call741 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 346) 
  %call742 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -822083584, i32 24, i32 143) 
  store i32 1, i32* @failed, align 4
  br label %if.end.783.thread

land.lhs.true.767:                                ; preds = %land.lhs.true.747
  %call751 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 346) 
  %call752 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -822083584, i32 24, i32 143) 
  store i32 1, i32* @failed, align 4
  br label %if.end.793

if.end.773.thread:                                ; preds = %if.end.743
  br label %if.end.793.thread

if.end.783.thread:                                ; preds = %if.end.753.thread, %if.end.753.thread.thread4096
  store i32 24, i32* @gcnt2, align 4
  store i32 -822083584, i32* @gmask, align 4
  br label %if.end.793.thread

if.end.793.thread:                                ; preds = %if.end.783.thread, %if.end.773.thread
  store i32 24, i32* @gcnt2, align 4
  store i32 -822083584, i32* @gmask, align 4
  store i32 24, i32* @gcnt2, align 4
  store i32 -822083584, i32* @gmask, align 4
  %shr.i.3833 = and i32 %x.04084, -822083584
  %cmp.i.3834 = icmp ugt i32 %shr.i.3833, -1912602624
  br i1 %cmp.i.3834, label %if.end.803, label %if.then.800

if.end.793:                                       ; preds = %land.lhs.true.767, %land.lhs.true.767.thread4100
  store i32 24, i32* @gcnt2, align 4
  store i32 -822083584, i32* @gmask, align 4
  store i32 24, i32* @gcnt2, align 4
  store i32 -822083584, i32* @gmask, align 4
  %.old4061 = and i32 %x.04084, -1073741824
  %cmp.i.3830.old = icmp ult i32 %.old4061, -1879048192
  br i1 %cmp.i.3830.old, label %if.end.813, label %if.then.810

if.then.800:                                      ; preds = %if.end.793.thread
  %call801 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.7, i32 0, i32 0), i32 346) 
  %call802 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -822083584, i32 24, i32 143) 
  store i32 1, i32* @failed, align 4
  br label %if.end.803

if.end.803:                                       ; preds = %if.then.800, %if.end.793.thread
  store i32 24, i32* @gcnt2, align 4
  store i32 -822083584, i32* @gmask, align 4
  %shr.i.3829 = and i32 %x.04084, -1073741824
  %cmp.i.3830 = icmp ult i32 %shr.i.3829, -1879048192
  %or.cond4062 = or i1 %cmp.i.3830, %cmp736
  br i1 %or.cond4062, label %if.end.813, label %if.then.810

if.then.810:                                      ; preds = %if.end.803, %if.end.793
  %call811 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 346) 
  %call812 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -822083584, i32 24, i32 143) 
  store i32 1, i32* @failed, align 4
  br label %if.end.813

if.end.813:                                       ; preds = %if.then.810, %if.end.803, %if.end.793
  store i32 24, i32* @gcnt2, align 4
  store i32 -1358954496, i32* @gmask, align 4
  %shr817 = and i32 %and652, 175
  %cmp818 = icmp ugt i32 %shr817, 112
  br i1 %cmp818, label %land.lhs.true.819, label %if.end.825

land.lhs.true.819:                                ; preds = %if.end.813
  %shr.i.3825 = and i32 %x.04084, -1358954496
  %cmp.i.3826 = icmp ugt i32 %shr.i.3825, 1879048192
  br i1 %cmp.i.3826, label %if.end.835.thread.thread4104, label %if.end.835.thread

if.end.835.thread.thread4104:                     ; preds = %land.lhs.true.819
  br label %if.end.865.thread

if.end.825:                                       ; preds = %if.end.813
  store i32 24, i32* @gcnt2, align 4
  store i32 -1358954496, i32* @gmask, align 4
  %cmp828 = icmp ult i32 %shr817, 112
  br i1 %cmp828, label %land.lhs.true.829, label %if.end.855.thread

land.lhs.true.829:                                ; preds = %if.end.825
  %shr.i.3821 = and i32 %x.04084, -1610612736
  %cmp.i.3822 = icmp ult i32 %shr.i.3821, 1879048192
  br i1 %cmp.i.3822, label %land.lhs.true.849.thread4108, label %land.lhs.true.849

land.lhs.true.849.thread4108:                     ; preds = %land.lhs.true.829
  br label %if.end.875

if.end.835.thread:                                ; preds = %land.lhs.true.819
  %call823 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 347) 
  %call824 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -1358954496, i32 24, i32 112) 
  store i32 1, i32* @failed, align 4
  br label %if.end.865.thread

land.lhs.true.849:                                ; preds = %land.lhs.true.829
  %call833 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 347) 
  %call834 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -1358954496, i32 24, i32 112) 
  store i32 1, i32* @failed, align 4
  br label %if.end.875

if.end.855.thread:                                ; preds = %if.end.825
  br label %if.end.875.thread

if.end.865.thread:                                ; preds = %if.end.835.thread, %if.end.835.thread.thread4104
  store i32 24, i32* @gcnt2, align 4
  store i32 -1358954496, i32* @gmask, align 4
  br label %if.end.875.thread

if.end.875.thread:                                ; preds = %if.end.865.thread, %if.end.855.thread
  store i32 24, i32* @gcnt2, align 4
  store i32 -1358954496, i32* @gmask, align 4
  store i32 24, i32* @gcnt2, align 4
  store i32 -1358954496, i32* @gmask, align 4
  %shr.i.3801 = and i32 %x.04084, -1610612736
  %cmp.i.3802 = icmp ugt i32 %shr.i.3801, 1862270976
  br i1 %cmp.i.3802, label %if.end.885, label %if.then.882

if.end.875:                                       ; preds = %land.lhs.true.849, %land.lhs.true.849.thread4108
  store i32 24, i32* @gcnt2, align 4
  store i32 -1358954496, i32* @gmask, align 4
  store i32 24, i32* @gcnt2, align 4
  store i32 -1358954496, i32* @gmask, align 4
  %.old4063 = and i32 %x.04084, -1358954496
  %cmp.i.3798.old = icmp ult i32 %.old4063, 1895825408
  br i1 %cmp.i.3798.old, label %if.end.977, label %if.then.892

if.then.882:                                      ; preds = %if.end.875.thread
  %call883 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.7, i32 0, i32 0), i32 347) 
  %call884 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -1358954496, i32 24, i32 112) 
  store i32 1, i32* @failed, align 4
  br label %if.end.885

if.end.885:                                       ; preds = %if.then.882, %if.end.875.thread
  store i32 24, i32* @gcnt2, align 4
  store i32 -1358954496, i32* @gmask, align 4
  %shr.i.3797 = and i32 %x.04084, -1358954496
  %cmp.i.3798 = icmp ult i32 %shr.i.3797, 1895825408
  %or.cond4064 = or i1 %cmp.i.3798, %cmp818
  br i1 %or.cond4064, label %if.end.977, label %if.then.892

if.then.892:                                      ; preds = %if.end.885, %if.end.875
  %call893 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 347) 
  %call894 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i32 0, i32 0), i32 %x.04084, i32 -1358954496, i32 24, i32 112) 
  store i32 1, i32* @failed, align 4
  br label %if.end.977

if.end.977:                                       ; preds = %if.then.892, %if.end.885, %if.end.875
  store i32 24, i32* @gcnt2, align 4
  store i32 -1090519040, i32* @gmask, align 4
  %inc = add nsw i32 %x.04084, 1
  %exitcond = icmp eq i32 %inc, %mul1
  %x.04084.in = bitcast i32 %inc to i32
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %if.end.977
  %conv981 = sext i32 %mul1 to i64
  br i1 %cmp.4083, label %for.body.984.preheader, label %for.end.2364

for.body.984.preheader:                           ; preds = %for.end
  %conv = sext i32 %mul to i64
  br label %if.end.1422

if.end.1422:                                      ; preds = %if.end.2361, %for.body.984.preheader
  %lx.04080 = phi i64 [ %inc2363, %if.end.2361 ], [ %conv, %for.body.984.preheader ]
  store i32 31, i32* @gcnt2, align 4
  store i64 -2147483648, i64* @glmask, align 8
  %cmp1426 = icmp ult i64 %lx.04080, 2147483648
  store i32 31, i32* @gcnt2, align 4
  store i64 -2147483648, i64* @glmask, align 8
  br i1 %cmp1426, label %if.end.1513, label %if.end.1448

if.end.1448:                                      ; preds = %if.end.1422
  store i32 31, i32* @gcnt2, align 4
  store i64 -2147483648, i64* @glmask, align 8
  store i32 31, i32* @gcnt2, align 4
  store i64 -2147483648, i64* @glmask, align 8
  br label %if.end.1513

if.end.1513:                                      ; preds = %if.end.1448, %if.end.1422
  store i32 27, i32* @gcnt2, align 4
  store i64 -1879048192, i64* @glmask, align 8
  %and1516 = lshr i64 %lx.04080, 27
  %shr1518 = and i64 %and1516, 137438953458
  %cmp1519 = icmp ugt i64 %shr1518, 3
  br i1 %cmp1519, label %land.lhs.true.1521, label %if.end.1528

land.lhs.true.1521:                               ; preds = %if.end.1513
  %shr.i.3689 = and i64 %lx.04080, -2147483648
  %cmp.i.3690 = icmp ugt i64 %shr.i.3689, 402653184
  br i1 %cmp.i.3690, label %if.end.1541.thread.thread4115, label %if.end.1541.thread

if.end.1541.thread.thread4115:                    ; preds = %land.lhs.true.1521
  br label %if.end.1580.thread

if.end.1528:                                      ; preds = %if.end.1513
  store i32 27, i32* @gcnt2, align 4
  store i64 -1879048192, i64* @glmask, align 8
  %cmp1532 = icmp ult i64 %shr1518, 3
  br i1 %cmp1532, label %land.lhs.true.1534, label %if.end.1567.thread

land.lhs.true.1534:                               ; preds = %if.end.1528
  %shr.i.3685 = and i64 %lx.04080, -1879048192
  %cmp.i.3686 = icmp ult i64 %shr.i.3685, 402653184
  br i1 %cmp.i.3686, label %land.lhs.true.1560.thread4119, label %land.lhs.true.1560

land.lhs.true.1560.thread4119:                    ; preds = %land.lhs.true.1534
  br label %if.end.1593

if.end.1541.thread:                               ; preds = %land.lhs.true.1521
  %call1526 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 358) 
  %call1527 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048192, i32 27, i64 3) 
  br label %if.end.1580.thread

land.lhs.true.1560:                               ; preds = %land.lhs.true.1534
  %call1539 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 358) 
  %call1540 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048192, i32 27, i64 3) 
  br label %if.end.1593

if.end.1567.thread:                               ; preds = %if.end.1528
  br label %if.end.1593.thread

if.end.1580.thread:                               ; preds = %if.end.1541.thread, %if.end.1541.thread.thread4115
  store i32 27, i32* @gcnt2, align 4
  store i64 -1879048192, i64* @glmask, align 8
  br label %if.end.1593.thread

if.end.1593.thread:                               ; preds = %if.end.1580.thread, %if.end.1567.thread
  store i32 27, i32* @gcnt2, align 4
  store i64 -1879048192, i64* @glmask, align 8
  store i32 27, i32* @gcnt2, align 4
  store i64 -1879048192, i64* @glmask, align 8
  %shr.i.3665 = and i64 %lx.04080, -1879048192
  %cmp.i.3666 = icmp ugt i64 %shr.i.3665, 268435456
  br i1 %cmp.i.3666, label %if.end.1606, label %if.then.1603

if.end.1593:                                      ; preds = %land.lhs.true.1560, %land.lhs.true.1560.thread4119
  store i32 27, i32* @gcnt2, align 4
  store i64 -1879048192, i64* @glmask, align 8
  store i32 27, i32* @gcnt2, align 4
  store i64 -1879048192, i64* @glmask, align 8
  %.old4067 = and i64 %lx.04080, -2147483648
  %cmp.i.3662.old = icmp ult i64 %.old4067, 536870912
  br i1 %cmp.i.3662.old, label %if.end.1619, label %if.then.1616

if.then.1603:                                     ; preds = %if.end.1593.thread
  %call1604 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.7, i32 0, i32 0), i32 358) 
  %call1605 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048192, i32 27, i64 3) 
  br label %if.end.1606

if.end.1606:                                      ; preds = %if.then.1603, %if.end.1593.thread
  store i32 27, i32* @gcnt2, align 4
  store i64 -1879048192, i64* @glmask, align 8
  %shr.i.3661 = and i64 %lx.04080, -2147483648
  %cmp.i.3662 = icmp ult i64 %shr.i.3661, 536870912
  %or.cond4068 = or i1 %cmp.i.3662, %cmp1519
  br i1 %or.cond4068, label %if.end.1619, label %if.then.1616

if.then.1616:                                     ; preds = %if.end.1606, %if.end.1593
  %call1617 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 358) 
  %call1618 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048192, i32 27, i64 3) 
  br label %if.end.1619

if.end.1619:                                      ; preds = %if.then.1616, %if.end.1606, %if.end.1593
  store i32 26, i32* @gcnt2, align 4
  store i64 -268435456, i64* @glmask, align 8
  %and1622 = lshr i64 %lx.04080, 26
  %shr1624 = and i64 %and1622, 274877906940
  %cmp1625 = icmp ugt i64 %shr1624, 16
  br i1 %cmp1625, label %land.lhs.true.1627, label %if.end.1634

land.lhs.true.1627:                               ; preds = %if.end.1619
  %shr.i.3657 = and i64 %lx.04080, -268435456
  %cmp.i.3658 = icmp ugt i64 %shr.i.3657, 1073741824
  br i1 %cmp.i.3658, label %if.end.1647.thread.thread4123, label %if.end.1647.thread

if.end.1647.thread.thread4123:                    ; preds = %land.lhs.true.1627
  br label %if.end.1686.thread

if.end.1634:                                      ; preds = %if.end.1619
  store i32 26, i32* @gcnt2, align 4
  store i64 -268435456, i64* @glmask, align 8
  %cmp1638 = icmp ult i64 %shr1624, 16
  br i1 %cmp1638, label %land.lhs.true.1640, label %if.end.1673.thread

land.lhs.true.1640:                               ; preds = %if.end.1634
  %cmp.i.3654 = icmp ult i64 %lx.04080, 1073741824
  br i1 %cmp.i.3654, label %land.lhs.true.1666.thread4127, label %land.lhs.true.1666

land.lhs.true.1666.thread4127:                    ; preds = %land.lhs.true.1640
  br label %if.end.1699

if.end.1647.thread:                               ; preds = %land.lhs.true.1627
  %call1632 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 359) 
  %call1633 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -268435456, i32 26, i64 16) 
  br label %if.end.1686.thread

land.lhs.true.1666:                               ; preds = %land.lhs.true.1640
  %call1645 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 359) 
  %call1646 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -268435456, i32 26, i64 16) 
  br label %if.end.1699

if.end.1673.thread:                               ; preds = %if.end.1634
  br label %if.end.1699.thread

if.end.1686.thread:                               ; preds = %if.end.1647.thread, %if.end.1647.thread.thread4123
  store i32 26, i32* @gcnt2, align 4
  store i64 -268435456, i64* @glmask, align 8
  br label %if.end.1699.thread

if.end.1699.thread:                               ; preds = %if.end.1686.thread, %if.end.1673.thread
  store i32 26, i32* @gcnt2, align 4
  store i64 -268435456, i64* @glmask, align 8
  store i32 26, i32* @gcnt2, align 4
  store i64 -268435456, i64* @glmask, align 8
  %cmp.i.3634 = icmp ugt i64 %lx.04080, 1073741823
  br i1 %cmp.i.3634, label %if.end.1712, label %if.then.1709

if.end.1699:                                      ; preds = %land.lhs.true.1666, %land.lhs.true.1666.thread4127
  store i32 26, i32* @gcnt2, align 4
  store i64 -268435456, i64* @glmask, align 8
  store i32 26, i32* @gcnt2, align 4
  store i64 -268435456, i64* @glmask, align 8
  %.old4069 = and i64 %lx.04080, -268435456
  %cmp.i.3630.old = icmp ult i64 %.old4069, 1140850688
  br i1 %cmp.i.3630.old, label %if.end.1725, label %if.then.1722

if.then.1709:                                     ; preds = %if.end.1699.thread
  %call1710 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.7, i32 0, i32 0), i32 359) 
  %call1711 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -268435456, i32 26, i64 16) 
  br label %if.end.1712

if.end.1712:                                      ; preds = %if.then.1709, %if.end.1699.thread
  store i32 26, i32* @gcnt2, align 4
  store i64 -268435456, i64* @glmask, align 8
  %shr.i.3629 = and i64 %lx.04080, -268435456
  %cmp.i.3630 = icmp ult i64 %shr.i.3629, 1140850688
  %or.cond4070 = or i1 %cmp.i.3630, %cmp1625
  br i1 %or.cond4070, label %if.end.1725, label %if.then.1722

if.then.1722:                                     ; preds = %if.end.1712, %if.end.1699
  %call1723 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 359) 
  %call1724 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -268435456, i32 26, i64 16) 
  br label %if.end.1725

if.end.1725:                                      ; preds = %if.then.1722, %if.end.1712, %if.end.1699
  store i32 24, i32* @gcnt2, align 4
  store i64 -134217728, i64* @glmask, align 8
  %and1728 = lshr i64 %lx.04080, 24
  %shr1730 = and i64 %and1728, 1099511627768
  %cmp1731 = icmp ugt i64 %shr1730, 240
  br i1 %cmp1731, label %land.lhs.true.1733, label %if.end.1740

land.lhs.true.1733:                               ; preds = %if.end.1725
  %shr.i.3625 = and i64 %lx.04080, -134217728
  %cmp.i.3626 = icmp ugt i64 %shr.i.3625, 4026531840
  br i1 %cmp.i.3626, label %if.end.1753.thread.thread4131, label %if.end.1753.thread

if.end.1753.thread.thread4131:                    ; preds = %land.lhs.true.1733
  br label %if.end.1792.thread

if.end.1740:                                      ; preds = %if.end.1725
  store i32 24, i32* @gcnt2, align 4
  store i64 -134217728, i64* @glmask, align 8
  %cmp1744 = icmp ult i64 %shr1730, 240
  br i1 %cmp1744, label %land.lhs.true.1746, label %if.end.1779.thread

land.lhs.true.1746:                               ; preds = %if.end.1740
  %cmp.i.3622 = icmp ult i64 %lx.04080, 4026531840
  br i1 %cmp.i.3622, label %land.lhs.true.1772.thread4135, label %land.lhs.true.1772

land.lhs.true.1772.thread4135:                    ; preds = %land.lhs.true.1746
  br label %if.end.1805

if.end.1753.thread:                               ; preds = %land.lhs.true.1733
  %call1738 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 360) 
  %call1739 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -134217728, i32 24, i64 240) 
  br label %if.end.1792.thread

land.lhs.true.1772:                               ; preds = %land.lhs.true.1746
  %call1751 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 360) 
  %call1752 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -134217728, i32 24, i64 240) 
  br label %if.end.1805

if.end.1779.thread:                               ; preds = %if.end.1740
  br label %if.end.1805.thread

if.end.1792.thread:                               ; preds = %if.end.1753.thread, %if.end.1753.thread.thread4131
  store i32 24, i32* @gcnt2, align 4
  store i64 -134217728, i64* @glmask, align 8
  br label %if.end.1805.thread

if.end.1805.thread:                               ; preds = %if.end.1792.thread, %if.end.1779.thread
  store i32 24, i32* @gcnt2, align 4
  store i64 -134217728, i64* @glmask, align 8
  store i32 24, i32* @gcnt2, align 4
  store i64 -134217728, i64* @glmask, align 8
  %cmp.i.3602 = icmp ugt i64 %lx.04080, 4026531839
  br i1 %cmp.i.3602, label %if.end.1818, label %if.then.1815

if.end.1805:                                      ; preds = %land.lhs.true.1772, %land.lhs.true.1772.thread4135
  store i32 24, i32* @gcnt2, align 4
  store i64 -134217728, i64* @glmask, align 8
  store i32 24, i32* @gcnt2, align 4
  store i64 -134217728, i64* @glmask, align 8
  %.old4071 = and i64 %lx.04080, -134217728
  %cmp.i.3598.old = icmp ult i64 %.old4071, 4043309056
  br i1 %cmp.i.3598.old, label %if.end.1831, label %if.then.1828

if.then.1815:                                     ; preds = %if.end.1805.thread
  %call1816 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.7, i32 0, i32 0), i32 360) 
  %call1817 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -134217728, i32 24, i64 240) 
  br label %if.end.1818

if.end.1818:                                      ; preds = %if.then.1815, %if.end.1805.thread
  store i32 24, i32* @gcnt2, align 4
  store i64 -134217728, i64* @glmask, align 8
  %shr.i.3597 = and i64 %lx.04080, -134217728
  %cmp.i.3598 = icmp ult i64 %shr.i.3597, 4043309056
  %or.cond4072 = or i1 %cmp.i.3598, %cmp1731
  br i1 %or.cond4072, label %if.end.1831, label %if.then.1828

if.then.1828:                                     ; preds = %if.end.1818, %if.end.1805
  %call1829 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 360) 
  %call1830 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -134217728, i32 24, i64 240) 
  br label %if.end.1831

if.end.1831:                                      ; preds = %if.then.1828, %if.end.1818, %if.end.1805
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  %and1834 = lshr i64 %lx.04080, 1
  %shr1836 = and i64 %and1834, 9223372035915251711
  %cmp1837 = icmp ugt i64 %shr1836, 3458764513820540928
  br i1 %cmp1837, label %land.lhs.true.1839, label %if.end.1846

land.lhs.true.1839:                               ; preds = %if.end.1831
  %shr.i.3593 = and i64 %lx.04080, -1879048194
  %cmp.i.3594 = icmp ugt i64 %shr.i.3593, 6917529027641081856
  br i1 %cmp.i.3594, label %if.end.1859.thread.thread4139, label %if.end.1859.thread

if.end.1859.thread.thread4139:                    ; preds = %land.lhs.true.1839
  br label %if.end.1937

if.end.1846:                                      ; preds = %if.end.1831
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  %cmp1850 = icmp ult i64 %shr1836, 3458764513820540928
  br i1 %cmp1850, label %land.lhs.true.1852, label %if.end.1924.thread

land.lhs.true.1852:                               ; preds = %if.end.1846
  %cmp.i.3590 = icmp ult i64 %lx.04080, 6917529027641081856
  br i1 %cmp.i.3590, label %land.lhs.true.1878.thread4143, label %land.lhs.true.1878

land.lhs.true.1878.thread4143:                    ; preds = %land.lhs.true.1852
  br label %if.end.1911

if.end.1859.thread:                               ; preds = %land.lhs.true.1839
  %call1844 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 362) 
  %call1845 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048194, i32 1, i64 3458764513820540928) 
  br label %if.end.1937

land.lhs.true.1878:                               ; preds = %land.lhs.true.1852
  %call1857 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 362) 
  %call1858 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048194, i32 1, i64 3458764513820540928) 
  br label %if.end.1911

if.end.1924.thread:                               ; preds = %if.end.1846
  br label %land.lhs.true.1930

if.end.1911:                                      ; preds = %land.lhs.true.1878, %land.lhs.true.1878.thread4143
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  br label %land.lhs.true.1930

land.lhs.true.1930:                               ; preds = %if.end.1911, %if.end.1924.thread
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  %shr.i.3565 = and i64 %lx.04080, -1879048194
  %cmp.i.3566 = icmp ult i64 %shr.i.3565, 6917529027641081858
  br i1 %cmp.i.3566, label %if.end.1937.thread, label %if.then.1934

if.then.1934:                                     ; preds = %land.lhs.true.1930
  %call1935 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 362) 
  %call1936 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048194, i32 1, i64 3458764513820540928) 
  br label %if.end.1937.thread

if.end.1937.thread:                               ; preds = %if.then.1934, %land.lhs.true.1930
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  br label %land.lhs.true.1958

if.end.1937:                                      ; preds = %if.end.1859.thread, %if.end.1859.thread.thread4139
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  %cmp1943 = icmp ugt i64 %shr1836, 8070450532247928832
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  br i1 %cmp1943, label %if.end.2017.thread, label %if.end.1952

if.end.1952:                                      ; preds = %if.end.1937
  %cmp1956 = icmp ult i64 %shr1836, 8070450532247928832
  br i1 %cmp1956, label %land.lhs.true.1958, label %land.lhs.true.1984.thread

land.lhs.true.1984.thread:                        ; preds = %if.end.1952
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  br label %if.end.1991.thread.thread

land.lhs.true.1958:                               ; preds = %if.end.1952, %if.end.1937.thread
  %cmp.i.3558 = icmp ult i64 %lx.04080, -2305843009213693952
  br i1 %cmp.i.3558, label %land.lhs.true.1984, label %if.then.1962

if.then.1962:                                     ; preds = %land.lhs.true.1958
  %call1963 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 363) 
  %call1964 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048194, i32 1, i64 8070450532247928832) 
  br label %land.lhs.true.1984

land.lhs.true.1984:                               ; preds = %if.then.1962, %land.lhs.true.1958
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  %cmp.i.3550 = icmp ult i64 %shr1836, 8070450532247928833
  br i1 %cmp.i.3550, label %if.end.1991.thread, label %if.then.1988

if.then.1988:                                     ; preds = %land.lhs.true.1984
  %call1989 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.4, i32 0, i32 0), i32 363) 
  %call1990 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048194, i32 1, i64 8070450532247928832) 
  br label %if.end.1991.thread.thread

if.end.1991.thread.thread:                        ; preds = %if.then.1988, %land.lhs.true.1984.thread
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  br label %if.end.2017.thread

if.end.1991.thread:                               ; preds = %land.lhs.true.1984
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  %cmp2008 = icmp ult i64 %shr1836, 8070450532247928832
  br i1 %cmp2008, label %if.end.2017, label %if.end.2017.thread

if.end.2017.thread:                               ; preds = %if.end.1991.thread, %if.end.1991.thread.thread, %if.end.1937
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  %cmp.i.3538 = icmp ugt i64 %lx.04080, -2305843009213693953
  br i1 %cmp.i.3538, label %if.end.2030, label %if.then.2027

if.end.2017:                                      ; preds = %if.end.1991.thread
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  %.old4073 = and i64 %lx.04080, -1879048194
  %cmp.i.3534.old = icmp ult i64 %.old4073, -2305843009213693950
  br i1 %cmp.i.3534.old, label %if.end.2043, label %if.then.2040

if.then.2027:                                     ; preds = %if.end.2017.thread
  %call2028 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.7, i32 0, i32 0), i32 363) 
  %call2029 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048194, i32 1, i64 8070450532247928832) 
  br label %if.end.2030

if.end.2030:                                      ; preds = %if.then.2027, %if.end.2017.thread
  store i32 1, i32* @gcnt2, align 4
  store i64 -1879048194, i64* @glmask, align 8
  %cmp2034 = icmp ugt i64 %shr1836, 8070450532247928832
  %shr.i.3533 = and i64 %lx.04080, -1879048194
  %cmp.i.3534 = icmp ult i64 %shr.i.3533, -2305843009213693950
  %or.cond4074 = or i1 %cmp.i.3534, %cmp2034
  br i1 %or.cond4074, label %if.end.2043, label %if.then.2040

if.then.2040:                                     ; preds = %if.end.2030, %if.end.2017
  %call2041 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 363) 
  %call2042 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048194, i32 1, i64 8070450532247928832) 
  br label %if.end.2043

if.end.2043:                                      ; preds = %if.then.2040, %if.end.2030, %if.end.2017
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048198, i64* @glmask, align 8
  %and2046 = lshr i64 %lx.04080, 2
  %shr2048 = and i64 %and2046, 4611686017957625854
  %cmp2049 = icmp ugt i64 %shr2048, 3458764513820540928
  br i1 %cmp2049, label %land.lhs.true.2051, label %if.end.2058

land.lhs.true.2051:                               ; preds = %if.end.2043
  %shr.i.3529 = and i64 %lx.04080, -1879048200
  %cmp.i.3530 = icmp ugt i64 %shr.i.3529, -4611686018427387904
  br i1 %cmp.i.3530, label %if.end.2071.thread.thread4151, label %if.end.2071.thread

if.end.2071.thread.thread4151:                    ; preds = %land.lhs.true.2051
  br label %if.end.2110.thread

if.end.2058:                                      ; preds = %if.end.2043
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048198, i64* @glmask, align 8
  %cmp2062 = icmp ult i64 %shr2048, 3458764513820540928
  br i1 %cmp2062, label %land.lhs.true.2064, label %if.end.2097.thread

land.lhs.true.2064:                               ; preds = %if.end.2058
  %cmp.i.3526 = icmp ult i64 %lx.04080, -4611686018427387904
  br i1 %cmp.i.3526, label %land.lhs.true.2090.thread4155, label %land.lhs.true.2090

land.lhs.true.2090.thread4155:                    ; preds = %land.lhs.true.2064
  br label %if.end.2123

if.end.2071.thread:                               ; preds = %land.lhs.true.2051
  %call2056 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 364) 
  %call2057 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048198, i32 2, i64 3458764513820540928) 
  br label %if.end.2110.thread

land.lhs.true.2090:                               ; preds = %land.lhs.true.2064
  %call2069 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 364) 
  %call2070 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048198, i32 2, i64 3458764513820540928) 
  br label %if.end.2123

if.end.2097.thread:                               ; preds = %if.end.2058
  br label %if.end.2123.thread

if.end.2110.thread:                               ; preds = %if.end.2071.thread, %if.end.2071.thread.thread4151
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048198, i64* @glmask, align 8
  br label %if.end.2123.thread

if.end.2123.thread:                               ; preds = %if.end.2110.thread, %if.end.2097.thread
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048198, i64* @glmask, align 8
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048198, i64* @glmask, align 8
  %cmp.i.3506 = icmp ugt i64 %lx.04080, -4611686018427387905
  br i1 %cmp.i.3506, label %if.end.2136, label %if.then.2133

if.end.2123:                                      ; preds = %land.lhs.true.2090, %land.lhs.true.2090.thread4155
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048198, i64* @glmask, align 8
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048198, i64* @glmask, align 8
  %.old4075 = and i64 %lx.04080, -1879048200
  %cmp.i.3502.old = icmp ult i64 %.old4075, -4611686018427387900
  br i1 %cmp.i.3502.old, label %if.end.2149, label %if.then.2146

if.then.2133:                                     ; preds = %if.end.2123.thread
  %call2134 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.7, i32 0, i32 0), i32 364) 
  %call2135 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048198, i32 2, i64 3458764513820540928) 
  br label %if.end.2136

if.end.2136:                                      ; preds = %if.then.2133, %if.end.2123.thread
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048198, i64* @glmask, align 8
  %shr.i.3501 = and i64 %lx.04080, -1879048200
  %cmp.i.3502 = icmp ult i64 %shr.i.3501, -4611686018427387900
  %or.cond4076 = or i1 %cmp.i.3502, %cmp2049
  br i1 %or.cond4076, label %if.end.2149, label %if.then.2146

if.then.2146:                                     ; preds = %if.end.2136, %if.end.2123
  %call2147 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 364) 
  %call2148 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048198, i32 2, i64 3458764513820540928) 
  br label %if.end.2149

if.end.2149:                                      ; preds = %if.then.2146, %if.end.2136, %if.end.2123
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048196, i64* @glmask, align 8
  %shr2154 = and i64 %and2046, 4611686017957625855
  %cmp2155 = icmp ugt i64 %shr2154, 3458764513820540928
  br i1 %cmp2155, label %land.lhs.true.2157, label %if.end.2164

land.lhs.true.2157:                               ; preds = %if.end.2149
  %shr.i.3497 = and i64 %lx.04080, -1879048196
  %cmp.i.3498 = icmp ugt i64 %shr.i.3497, -4611686018427387904
  br i1 %cmp.i.3498, label %if.end.2177.thread.thread4159, label %if.end.2177.thread

if.end.2177.thread.thread4159:                    ; preds = %land.lhs.true.2157
  br label %if.end.2216.thread

if.end.2164:                                      ; preds = %if.end.2149
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048196, i64* @glmask, align 8
  %cmp2168 = icmp ult i64 %shr2154, 3458764513820540928
  br i1 %cmp2168, label %land.lhs.true.2170, label %if.end.2203.thread

land.lhs.true.2170:                               ; preds = %if.end.2164
  %cmp.i.3494 = icmp ult i64 %lx.04080, -4611686018427387904
  br i1 %cmp.i.3494, label %land.lhs.true.2196.thread4163, label %land.lhs.true.2196

land.lhs.true.2196.thread4163:                    ; preds = %land.lhs.true.2170
  br label %if.end.2229

if.end.2177.thread:                               ; preds = %land.lhs.true.2157
  %call2162 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 365) 
  %call2163 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048196, i32 2, i64 3458764513820540928) 
  br label %if.end.2216.thread

land.lhs.true.2196:                               ; preds = %land.lhs.true.2170
  %call2175 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.2, i32 0, i32 0), i32 365) 
  %call2176 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048196, i32 2, i64 3458764513820540928) 
  br label %if.end.2229

if.end.2203.thread:                               ; preds = %if.end.2164
  br label %if.end.2229.thread

if.end.2216.thread:                               ; preds = %if.end.2177.thread, %if.end.2177.thread.thread4159
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048196, i64* @glmask, align 8
  br label %if.end.2229.thread

if.end.2229.thread:                               ; preds = %if.end.2216.thread, %if.end.2203.thread
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048196, i64* @glmask, align 8
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048196, i64* @glmask, align 8
  %cmp.i.3474 = icmp ugt i64 %lx.04080, -4611686018427387905
  br i1 %cmp.i.3474, label %if.end.2242, label %if.then.2239

if.end.2229:                                      ; preds = %land.lhs.true.2196, %land.lhs.true.2196.thread4163
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048196, i64* @glmask, align 8
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048196, i64* @glmask, align 8
  %.old4077 = and i64 %lx.04080, -1879048196
  %cmp.i.3470.old = icmp ult i64 %.old4077, -4611686018427387900
  br i1 %cmp.i.3470.old, label %if.end.2361, label %if.then.2252

if.then.2239:                                     ; preds = %if.end.2229.thread
  %call2240 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.7, i32 0, i32 0), i32 365) 
  %call2241 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048196, i32 2, i64 3458764513820540928) 
  br label %if.end.2242

if.end.2242:                                      ; preds = %if.then.2239, %if.end.2229.thread
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048196, i64* @glmask, align 8
  %shr.i.3469 = and i64 %lx.04080, -1879048196
  %cmp.i.3470 = icmp ult i64 %shr.i.3469, -4611686018427387900
  %or.cond4078 = or i1 %cmp.i.3470, %cmp2155
  br i1 %or.cond4078, label %if.end.2361, label %if.then.2252

if.then.2252:                                     ; preds = %if.end.2242, %if.end.2229
  %call2253 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([25 x i8], [25 x i8]* @.str.8, i32 0, i32 0), i32 365) 
  %call2254 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([37 x i8], [37 x i8]* @.str.9, i32 0, i32 0), i64 %lx.04080, i64 -1879048196, i32 2, i64 3458764513820540928) 
  br label %if.end.2361

if.end.2361:                                      ; preds = %if.then.2252, %if.end.2242, %if.end.2229
  store i32 2, i32* @gcnt2, align 4
  store i64 -1879048196, i64* @glmask, align 8
  %inc2363 = add nsw i64 %lx.04080, 1
  %cmp982 = icmp slt i64 %inc2363, %conv981
  %lx.04080.in = bitcast i64 %inc2363 to i64
  br i1 %cmp982, label %if.end.1422, label %for.end.2364.loopexit

for.end.2364.loopexit:                            ; preds = %if.end.2361
  br label %for.end.2364

for.end.2364:                                     ; preds = %for.end.2364.loopexit, %for.end, %entry
  %0 = load i32, i32* @failed, align 4
  %cmp2365 = icmp eq i32 %0, 0
  br i1 %cmp2365, label %if.then.2367, label %if.end.2369

if.then.2367:                                     ; preds = %for.end.2364
  %puts = tail call i32 @puts(i8* nonnull getelementptr inbounds ([7 x i8], [7 x i8]* @str, i32 0, i32 0))
  br label %if.end.2369

if.end.2369:                                      ; preds = %if.then.2367, %for.end.2364
  ret i32 0
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) 

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) 

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) 

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) 

