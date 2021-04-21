; int rayobject_bb_intersect_test(
; float *pt1x, float *pt1y, float *pt1z,
; float *pt2x, float *pt2y, float *pt2z, str *dist
;)
;{
;  float t1x = *pt1x; float t1y = *pt1y; float t1z = *pt1z;
;  float t2x = *pt2x; float t2y = *pt2y; float t2z = *pt2z;
;  if (t1x > t2y  || t2x < t1y  || t1x > t2z ||
;      t2x < t1z || t1y > t2z || t2y < t1z) return 0;
;  if (t2x < 0.0f || t2y < 0.0f || t2z < 0.0f) return 0;
;  if (t1x > dist->b || t1y > dist->b || t1z > dist->b) return 0;
;  return 1;
;}
;
; =>
;
;  if (t2x < 0.0f || t2y < 0.0f || t2z < 0.0f) return 0;
;  if (t1x > dist->b || t1y > dist->b || t1z > dist->b) return 0;
;  if (t2y < llvm.maxnum(t1x,t1z) || t2x < llvm.maxnum(t1y,t1z) ||
;    t2z < llvm.maxnum(t1x,t1y)) return 0;
;
; The || patterns form a "ladder" of 12 blocks. The false branches go the
; the next ladder block, and the true branch goes to an exit block.
; All the blocks can be reordered w.r.t each other.
; We reorder the blocks as above, and then apply a maxnum peephole on the
; 6 blocks with t1?/t2? clauses.

; RUN: opt < %s -simplifycfg -S | FileCheck %s

; Updating note: most of the allocas and temp loads/stores in the first block
; can be cut off. Only T1/2, RETVAL, DIST_ADDR, CLEANUP_DEST_SLOT need defs.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


%struct.str = type { float, float }

@.str = private unnamed_addr constant [22 x i8] c"%f %f %f %f %f %f %d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @rayobject_bb_intersect_test(float* %pt1x, float* %pt1y, float* %pt1z, float* %pt2x, float* %pt2y, float* %pt2z, %struct.str* %dist) #0 {
; CHECK-LABEL: @rayobject_bb_intersect_test(
; CHECK-NEXT:  entry:
; CHECK:    [[RETVAL:%.*]] = alloca i32, align 4
; CHECK:    [[DIST_ADDR:%.*]] = alloca %struct.str*, align 8
; CHECK:    [[T1X:%.*]] = alloca float, align 4
; CHECK:    [[T1Y:%.*]] = alloca float, align 4
; CHECK:    [[T1Z:%.*]] = alloca float, align 4
; CHECK:    [[T2X:%.*]] = alloca float, align 4
; CHECK:    [[T2Y:%.*]] = alloca float, align 4
; CHECK:    [[T2Z:%.*]] = alloca float, align 4
; CHECK:    [[CLEANUP_DEST_SLOT:%.*]] = alloca i32, align 4
; CHECK:    [[TMP18:%.*]] = load float, float* [[T2X]], align 4
; CHECK:    [[CMP10:%.*]] = fcmp fast olt float [[TMP18]], 0.000000e+00
; CHECK:    [[TMP19:%.*]] = load float, float* [[T2Y]], align 4
; CHECK:    [[CMP12:%.*]] = fcmp fast olt float [[TMP19]], 0.000000e+00
; CHECK:    [[OR_COND:%.*]] = {{select|or}} i1 [[CMP10]]{{.*}}[[CMP12]]
; CHECK:    [[TMP20:%.*]] = load float, float* [[T2Z]], align 4
; CHECK:    [[CMP14:%.*]] = fcmp fast olt float [[TMP20]], 0.000000e+00
; CHECK:    [[OR_COND5:%.*]] = {{select|or}} i1 [[OR_COND]]{{.*}}[[CMP14]]
; CHECK:    br i1 [[OR_COND5]], label [[IF_THEN15:%.*]], label [[IF_END16:%.*]]
; CHECK:       if.end16:
; CHECK-NEXT:    [[TMP21:%.*]] = load float, float* [[T1X]], align 4
; CHECK-NEXT:    [[TMP22:%.*]] = load %struct.str*, %struct.str** [[DIST_ADDR]], align 8
; CHECK-NEXT:    [[B:%.*]] = getelementptr inbounds [[STRUCT_STR:%.*]], %struct.str* [[TMP22]], i32 0, i32 1
; CHECK-NEXT:    [[TMP23:%.*]] = load float, float* [[B]], align 4
; CHECK-NEXT:    [[CMP17:%.*]] = fcmp fast ogt float [[TMP21]], [[TMP23]]
; CHECK-NEXT:    br i1 [[CMP17]], label [[IF_THEN24:%.*]], label [[LOR_LHS_FALSE18:%.*]]
; CHECK:       lor.lhs.false18:
; CHECK-NEXT:    [[TMP24:%.*]] = load float, float* [[T1Y]], align 4
; CHECK-NEXT:    [[TMP25:%.*]] = load %struct.str*, %struct.str** [[DIST_ADDR]], align 8
; CHECK-NEXT:    [[B19:%.*]] = getelementptr inbounds [[STRUCT_STR]], %struct.str* [[TMP25]], i32 0, i32 1
; CHECK-NEXT:    [[TMP26:%.*]] = load float, float* [[B19]], align 4
; CHECK-NEXT:    [[CMP20:%.*]] = fcmp fast ogt float [[TMP24]], [[TMP26]]
; CHECK-NEXT:    br i1 [[CMP20]], label [[IF_THEN24]], label [[LOR_LHS_FALSE21:%.*]]
; CHECK:       lor.lhs.false21:
; CHECK-NEXT:    [[TMP27:%.*]] = load float, float* [[T1Z]], align 4
; CHECK-NEXT:    [[TMP28:%.*]] = load %struct.str*, %struct.str** [[DIST_ADDR]], align 8
; CHECK-NEXT:    [[B22:%.*]] = getelementptr inbounds [[STRUCT_STR]], %struct.str* [[TMP28]], i32 0, i32 1
; CHECK-NEXT:    [[TMP29:%.*]] = load float, float* [[B22]], align 4
; CHECK-NEXT:    [[CMP23:%.*]] = fcmp fast ogt float [[TMP27]], [[TMP29]]
; CHECK-NEXT:    br i1 [[CMP23]], label [[IF_THEN24]], label [[LADDER:%.*]]
; CHECK:       ladder:
; CHECK-NEXT:    [[TMP30:%.*]] = load float, float* [[T1X]], align 4
; CHECK-NEXT:    [[TMP31:%.*]] = load float, float* [[T2Y]], align 4
; CHECK-NEXT:    [[TMP32:%.*]] = load float, float* [[T2X]], align 4
; CHECK-NEXT:    [[TMP33:%.*]] = load float, float* [[T1Y]], align 4
; CHECK-NEXT:    [[TMP34:%.*]] = load float, float* [[T1X]], align 4
; CHECK-NEXT:    [[TMP35:%.*]] = load float, float* [[T2Z]], align 4
; CHECK-NEXT:    [[TMP36:%.*]] = load float, float* [[T2X]], align 4
; CHECK-NEXT:    [[TMP37:%.*]] = load float, float* [[T1Z]], align 4
; CHECK-NEXT:    [[FMAXF1:%.*]] = call float @llvm.maxnum.f32(float [[TMP33]], float [[TMP37]])
; CHECK-NEXT:    [[FMAXFCMP2:%.*]] = fcmp fast olt float [[TMP32]], [[FMAXF1]]
; CHECK-NEXT:    br i1 [[FMAXFCMP2]], label [[IF_THEN:%.*]], label [[LOR_LHS_FALSE6:%.*]]
; CHECK:       lor.lhs.false6:
; CHECK-NEXT:    [[TMP38:%.*]] = load float, float* [[T1Y]], align 4
; CHECK-NEXT:    [[TMP39:%.*]] = load float, float* [[T2Z]], align 4
; CHECK-NEXT:    [[FMAXF3:%.*]] = call float @llvm.maxnum.f32(float [[TMP34]], float [[TMP38]])
; CHECK-NEXT:    [[FMAXFCMP4:%.*]] = fcmp fast olt float [[TMP35]], [[FMAXF3]]
; CHECK-NEXT:    br i1 [[FMAXFCMP4]], label [[IF_THEN]], label [[LOR_LHS_FALSE8:%.*]]
; CHECK:       lor.lhs.false8:
; CHECK-NEXT:    [[TMP40:%.*]] = load float, float* [[T2Y]], align 4
; CHECK-NEXT:    [[TMP41:%.*]] = load float, float* [[T1Z]], align 4
; CHECK-NEXT:    [[FMAXF:%.*]] = call float @llvm.maxnum.f32(float [[TMP30]], float [[TMP41]])
; CHECK-NEXT:    [[FMAXFCMP:%.*]] = fcmp fast olt float [[TMP31]], [[FMAXF]]
; CHECK-NEXT:    br i1 [[FMAXFCMP]], label [[IF_THEN]], label [[IF_END25:%.*]]
; CHECK:       if.then:
; CHECK-NEXT:    store i32 0, i32* [[RETVAL]], align 4
; CHECK-NEXT:    store i32 1, i32* [[CLEANUP_DEST_SLOT]], align 4
; CHECK-NEXT:    br label [[CLEANUP:%.*]]
; CHECK:       if.then15:
; CHECK-NEXT:    store i32 0, i32* [[RETVAL]], align 4
; CHECK-NEXT:    store i32 1, i32* [[CLEANUP_DEST_SLOT]], align 4
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       if.then24:
; CHECK-NEXT:    store i32 0, i32* [[RETVAL]], align 4
; CHECK-NEXT:    store i32 1, i32* [[CLEANUP_DEST_SLOT]], align 4
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       if.end25:
; CHECK-NEXT:    store i32 1, i32* [[RETVAL]], align 4
; CHECK-NEXT:    store i32 1, i32* [[CLEANUP_DEST_SLOT]], align 4
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[TMP42:%.*]] = bitcast float* [[T2Z]] to i8*
; CHECK-NEXT:    call void @llvm.lifetime.end.p0i8(i64 4, i8* [[TMP42]])
; CHECK-NEXT:    [[TMP43:%.*]] = bitcast float* [[T2Y]] to i8*
; CHECK-NEXT:    call void @llvm.lifetime.end.p0i8(i64 4, i8* [[TMP43]])
; CHECK-NEXT:    [[TMP44:%.*]] = bitcast float* [[T2X]] to i8*
; CHECK-NEXT:    call void @llvm.lifetime.end.p0i8(i64 4, i8* [[TMP44]])
; CHECK-NEXT:    [[TMP45:%.*]] = bitcast float* [[T1Z]] to i8*
; CHECK-NEXT:    call void @llvm.lifetime.end.p0i8(i64 4, i8* [[TMP45]])
; CHECK-NEXT:    [[TMP46:%.*]] = bitcast float* [[T1Y]] to i8*
; CHECK-NEXT:    call void @llvm.lifetime.end.p0i8(i64 4, i8* [[TMP46]])
; CHECK-NEXT:    [[TMP47:%.*]] = bitcast float* [[T1X]] to i8*
; CHECK-NEXT:    call void @llvm.lifetime.end.p0i8(i64 4, i8* [[TMP47]])
; CHECK-NEXT:    [[TMP48:%.*]] = load i32, i32* [[RETVAL]], align 4
; CHECK-NEXT:    ret i32 [[TMP48]]
;
entry:
  %retval = alloca i32, align 4
  %pt1x.addr = alloca float*, align 8
  %pt1y.addr = alloca float*, align 8
  %pt1z.addr = alloca float*, align 8
  %pt2x.addr = alloca float*, align 8
  %pt2y.addr = alloca float*, align 8
  %pt2z.addr = alloca float*, align 8
  %dist.addr = alloca %struct.str*, align 8
  %t1x = alloca float, align 4
  %t1y = alloca float, align 4
  %t1z = alloca float, align 4
  %t2x = alloca float, align 4
  %t2y = alloca float, align 4
  %t2z = alloca float, align 4
  %cleanup.dest.slot = alloca i32, align 4
  store float* %pt1x, float** %pt1x.addr, align 8
  store float* %pt1y, float** %pt1y.addr, align 8
  store float* %pt1z, float** %pt1z.addr, align 8
  store float* %pt2x, float** %pt2x.addr, align 8
  store float* %pt2y, float** %pt2y.addr, align 8
  store float* %pt2z, float** %pt2z.addr, align 8
  store %struct.str* %dist, %struct.str** %dist.addr, align 8
  %0 = bitcast float* %t1x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #4
  %1 = load float*, float** %pt1x.addr, align 8
  %2 = load float, float* %1, align 4
  store float %2, float* %t1x, align 4
  %3 = bitcast float* %t1y to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #4
  %4 = load float*, float** %pt1y.addr, align 8
  %5 = load float, float* %4, align 4
  store float %5, float* %t1y, align 4
  %6 = bitcast float* %t1z to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #4
  %7 = load float*, float** %pt1z.addr, align 8
  %8 = load float, float* %7, align 4
  store float %8, float* %t1z, align 4
  %9 = bitcast float* %t2x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #4
  %10 = load float*, float** %pt2x.addr, align 8
  %11 = load float, float* %10, align 4
  store float %11, float* %t2x, align 4
  %12 = bitcast float* %t2y to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %12) #4
  %13 = load float*, float** %pt2y.addr, align 8
  %14 = load float, float* %13, align 4
  store float %14, float* %t2y, align 4
  %15 = bitcast float* %t2z to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %15) #4
  %16 = load float*, float** %pt2z.addr, align 8
  %17 = load float, float* %16, align 4
  store float %17, float* %t2z, align 4
  %18 = load float, float* %t1x, align 4
  %19 = load float, float* %t2y, align 4
  %cmp = fcmp fast ogt float %18, %19
  br i1 %cmp, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %20 = load float, float* %t2x, align 4
  %21 = load float, float* %t1y, align 4
  %cmp1 = fcmp fast olt float %20, %21
  br i1 %cmp1, label %if.then, label %lor.lhs.false2

lor.lhs.false2:                                   ; preds = %lor.lhs.false
  %22 = load float, float* %t1x, align 4
  %23 = load float, float* %t2z, align 4
  %cmp3 = fcmp fast ogt float %22, %23
  br i1 %cmp3, label %if.then, label %lor.lhs.false4

lor.lhs.false4:                                   ; preds = %lor.lhs.false2
  %24 = load float, float* %t2x, align 4
  %25 = load float, float* %t1z, align 4
  %cmp5 = fcmp fast olt float %24, %25
  br i1 %cmp5, label %if.then, label %lor.lhs.false6

lor.lhs.false6:                                   ; preds = %lor.lhs.false4
  %26 = load float, float* %t1y, align 4
  %27 = load float, float* %t2z, align 4
  %cmp7 = fcmp fast ogt float %26, %27
  br i1 %cmp7, label %if.then, label %lor.lhs.false8

lor.lhs.false8:                                   ; preds = %lor.lhs.false6
  %28 = load float, float* %t2y, align 4
  %29 = load float, float* %t1z, align 4
  %cmp9 = fcmp fast olt float %28, %29
  br i1 %cmp9, label %if.then, label %if.end

if.then:                                          ; preds = %lor.lhs.false8, %lor.lhs.false6, %lor.lhs.false4, %lor.lhs.false2, %lor.lhs.false, %entry
  store i32 0, i32* %retval, align 4
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

if.end:                                           ; preds = %lor.lhs.false8
  %30 = load float, float* %t2x, align 4
  %cmp10 = fcmp fast olt float %30, 0.000000e+00
  br i1 %cmp10, label %if.then15, label %lor.lhs.false11

lor.lhs.false11:                                  ; preds = %if.end
  %31 = load float, float* %t2y, align 4
  %cmp12 = fcmp fast olt float %31, 0.000000e+00
  br i1 %cmp12, label %if.then15, label %lor.lhs.false13

lor.lhs.false13:                                  ; preds = %lor.lhs.false11
  %32 = load float, float* %t2z, align 4
  %cmp14 = fcmp fast olt float %32, 0.000000e+00
  br i1 %cmp14, label %if.then15, label %if.end16

if.then15:                                        ; preds = %lor.lhs.false13, %lor.lhs.false11, %if.end
  store i32 0, i32* %retval, align 4
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

if.end16:                                         ; preds = %lor.lhs.false13
  %33 = load float, float* %t1x, align 4
  %34 = load %struct.str*, %struct.str** %dist.addr, align 8
  %b = getelementptr inbounds %struct.str, %struct.str* %34, i32 0, i32 1
  %35 = load float, float* %b, align 4
  %cmp17 = fcmp fast ogt float %33, %35
  br i1 %cmp17, label %if.then24, label %lor.lhs.false18

lor.lhs.false18:                                  ; preds = %if.end16
  %36 = load float, float* %t1y, align 4
  %37 = load %struct.str*, %struct.str** %dist.addr, align 8
  %b19 = getelementptr inbounds %struct.str, %struct.str* %37, i32 0, i32 1
  %38 = load float, float* %b19, align 4
  %cmp20 = fcmp fast ogt float %36, %38
  br i1 %cmp20, label %if.then24, label %lor.lhs.false21

lor.lhs.false21:                                  ; preds = %lor.lhs.false18
  %39 = load float, float* %t1z, align 4
  %40 = load %struct.str*, %struct.str** %dist.addr, align 8
  %b22 = getelementptr inbounds %struct.str, %struct.str* %40, i32 0, i32 1
  %41 = load float, float* %b22, align 4
  %cmp23 = fcmp fast ogt float %39, %41
  br i1 %cmp23, label %if.then24, label %if.end25

if.then24:                                        ; preds = %lor.lhs.false21, %lor.lhs.false18, %if.end16
  store i32 0, i32* %retval, align 4
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

if.end25:                                         ; preds = %lor.lhs.false21
  store i32 1, i32* %retval, align 4
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

cleanup:                                          ; preds = %if.end25, %if.then24, %if.then15, %if.then
  %42 = bitcast float* %t2z to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %42) #4
  %43 = bitcast float* %t2y to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %43) #4
  %44 = bitcast float* %t2x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %44) #4
  %45 = bitcast float* %t1z to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %45) #4
  %46 = bitcast float* %t1y to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %46) #4
  %47 = bitcast float* %t1x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %47) #4
  %48 = load i32, i32* %retval, align 4
  ret i32 %48
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1
