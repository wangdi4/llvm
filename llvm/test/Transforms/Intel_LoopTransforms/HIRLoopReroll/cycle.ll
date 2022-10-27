; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -hir-lmm -hir-loop-reroll -print-before=hir-loop-reroll -print-after=hir-loop-reroll < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,hir-lmm,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s

; Verify reroll does not cause a transformation but ends quietly. From atg_CMPLRLLVM-20518.c.

; *** IR Dump Before HIR Loop Reroll ***
; CHECK:Function: main
; CHECK:       BEGIN REGION { modified }
; CHECK:                %limm = (%i2)[0][2][1];
; CHECK:                %limm256 = (%cw)[0][1];
; CHECK:                %limm319 = (%i2)[0][5][4];
; CHECK:             + DO i1 = 0, 90, 1   <DO_LOOP>
; CHECK:             |   %add = i1 + 1  +  1;
; CHECK:             |   %11 = (%e)[0][i1 + 2];
; CHECK:             |   (%y)[0] = ((1 + (-1 * %11)) * %11);
; CHECK:             |   (%p4.0100)[0] = (%cw)[0][i1 + 1];
; CHECK:             |   %13 = (%cw)[0][i1 + 2];
; CHECK:             |   %add28 = %13  +  %11;
; CHECK:             |   (%o)[0] = %11 + %13;
; CHECK:             |   %14 = (%y)[0];
; CHECK:             |   %15 = (%o1)[0][i1];
; CHECK:             |   (%e)[0][i1] = %11 + -1 * %14 + %15;
; CHECK:             |   %y.promoted = %14;
; CHECK:             |   %limm255 = %limm;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm256;
; CHECK:             |   %limm256 = %21 + 30;
; CHECK:             |   %limm321 = %limm319;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = (%cw)[0][4];
; CHECK:             |   (%cw)[0][4] = %21 + 30;
; CHECK:             |   %limm323 = %limm;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm256;
; CHECK:             |   %limm256 = %21 + 29;
; CHECK:             |   %limm325 = %limm319;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = (%cw)[0][4];
; CHECK:             |   (%cw)[0][4] = %21 + 29;
; CHECK:             |   %limm327 = %limm;
;                    |   ...
; CHECK:             |   %21 = (%cw)[0][4];
; CHECK:             |   (%cw)[0][4] = %21;
; CHECK:             |   (%y)[0] = %y.promoted;
; CHECK:             |   %p4.0100 = &((%y)[0]);
; CHECK:             + END LOOP
; CHECK:                (%i2)[0][3][4] = %limm439;
; CHECK:                (%i2)[0][3][1] = %limm437;
; CHECK:                (%i2)[0][4][4] = %limm435;
; CHECK:                (%i2)[0][4][1] = %limm433;
; CHECK:                (%i2)[0][5][1] = %limm431;
; CHECK:                (%i2)[0][6][4] = %limm429;
; CHECK:                (%i2)[0][6][1] = %limm427;
; CHECK:                (%i2)[0][7][4] = %limm425;
; CHECK:                (%i2)[0][7][1] = %limm423;
; CHECK:                (%i2)[0][8][4] = %limm421;
; CHECK:                (%i2)[0][8][1] = %limm419;
;                        ...
; CHECK:                (%i2)[0][32][4] = %limm325;
; CHECK:                (%i2)[0][32][1] = %limm323;
; CHECK:                (%i2)[0][33][4] = %limm321;
; CHECK:                (%i2)[0][5][4] = %limm319;
; CHECK:                (%cw)[0][1] = %limm256;
; CHECK:                (%i2)[0][33][1] = %limm255;
; CHECK:       END REGION

; Reroll does not cause a transformation but ends quietly.
; *** IR Dump After HIR Loop Reroll ***
; CHECK:Function: main
; CHECK:        BEGIN REGION { modified }
; CHECK:                %limm = (%i2)[0][2][1];
; CHECK:                %limm256 = (%cw)[0][1];
; CHECK:                %limm319 = (%i2)[0][5][4];
; CHECK:             + DO i1 = 0, 90, 1   <DO_LOOP>
; CHECK:             |   %add = i1 + 1  +  1;
; CHECK:             |   %11 = (%e)[0][i1 + 2];
; CHECK:             |   (%y)[0] = ((1 + (-1 * %11)) * %11);
; CHECK:             |   (%p4.0100)[0] = (%cw)[0][i1 + 1];
; CHECK:             |   %13 = (%cw)[0][i1 + 2];
; CHECK:             |   %add28 = %13  +  %11;
; CHECK:             |   (%o)[0] = %11 + %13;
; CHECK:             |   %14 = (%y)[0];
; CHECK:             |   %15 = (%o1)[0][i1];
; CHECK:             |   (%e)[0][i1] = %11 + -1 * %14 + %15;
; CHECK:             |   %y.promoted = %14;
; CHECK:             |   %limm255 = %limm;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = %limm256;
; CHECK:             |   %limm256 = %21 + 30;
; CHECK:             |   %limm321 = %limm319;
; CHECK:             |   %y.promoted = ((%11 + %13) * %y.promoted)  +  %y.promoted;
; CHECK:             |   %21 = (%cw)[0][4];
;                    |   ...
; CHECK:              + END LOOP
; CHECK:                (%i2)[0][3][4] = %limm439;
; CHECK:                (%i2)[0][3][1] = %limm437;
; CHECK:                (%i2)[0][4][4] = %limm435;
; CHECK:                (%i2)[0][4][1] = %limm433;
; CHECK:                (%i2)[0][5][1] = %limm431;
; CHECK:                (%i2)[0][6][4] = %limm429;
; CHECK:                (%i2)[0][6][1] = %limm427;
;                        ...
; CHECK:                (%cw)[0][1] = %limm256;
; CHECK:                (%i2)[0][33][1] = %limm255;

; CHECK:         END REGION
;Module Before HIR
; ModuleID = 'cycle.c'
source_filename = "cycle.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
@.str = private unnamed_addr constant [15 x i8] c"%u %u %u %u %u\00", align 1
@.str.1 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1
; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %id4 = alloca i32, align 4
  %y = alloca i32, align 4
  %o = alloca i32, align 4
  %j6 = alloca i32, align 4
  %jy3 = alloca i32, align 4
  %e = alloca [100 x i32], align 16
  %cw = alloca [100 x i32], align 16
  %o1 = alloca [100 x i32], align 16
  %i2 = alloca [100 x [100 x i32]], align 16
  %0 = bitcast i32* %id4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #5
  store i32 44, i32* %id4, align 4, !tbaa !2
  %1 = bitcast i32* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #5
  store i32 92, i32* %y, align 4, !tbaa !2
  %2 = bitcast i32* %o to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #5
  store i32 45, i32* %o, align 4, !tbaa !2
  %3 = bitcast i32* %j6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #5
  store i32 41, i32* %j6, align 4, !tbaa !2
  %4 = bitcast i32* %jy3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %4) #5
  store i32 85, i32* %jy3, align 4, !tbaa !2
  %5 = bitcast [100 x i32]* %e to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %5) #5
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 dereferenceable(400) %5, i8 0, i64 400, i1 false)
  %6 = bitcast [100 x i32]* %cw to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %6) #5
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 dereferenceable(400) %6, i8 0, i64 400, i1 false)
  %7 = bitcast [100 x i32]* %o1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %7) #5
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 dereferenceable(400) %7, i8 0, i64 400, i1 false)
  %8 = bitcast [100 x [100 x i32]]* %i2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 40000, i8* nonnull %8) #5
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 dereferenceable(40000) %8, i8 0, i64 40000, i1 false)
  %arraydecay = getelementptr inbounds [100 x i32], [100 x i32]* %e, i64 0, i64 0
  %call = call i32 (i32*, i32, i32, ...) bitcast (i32 (...)* @init to i32 (i32*, i32, i32, ...)*)(i32* nonnull %arraydecay, i32 100, i32 68) #5
  %arraydecay1 = getelementptr inbounds [100 x i32], [100 x i32]* %cw, i64 0, i64 0
  %call2 = call i32 (i32*, i32, i32, ...) bitcast (i32 (...)* @init to i32 (i32*, i32, i32, ...)*)(i32* nonnull %arraydecay1, i32 100, i32 1) #5
  %arraydecay3 = getelementptr inbounds [100 x i32], [100 x i32]* %o1, i64 0, i64 0
  %call4 = call i32 (i32*, i32, i32, ...) bitcast (i32 (...)* @init to i32 (i32*, i32, i32, ...)*)(i32* nonnull %arraydecay3, i32 100, i32 86) #5
  %9 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %i2, i64 0, i64 0, i64 0
  %call6 = call i32 (i32*, i32, i32, ...) bitcast (i32 (...)* @init to i32 (i32*, i32, i32, ...)*)(i32* nonnull %9, i32 10000, i32 44) #5
  %call7 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i64 0, i64 0), i32* nonnull %id4, i32* nonnull %y, i32* nonnull %o, i32* nonnull %j6, i32* nonnull %jy3)
  store i32 1, i32* %id4, align 4, !tbaa !2
  br label %for.body
for.cond.loopexit:                                ; preds = %for.inc70
  %add64.lcssa.lcssa = phi i32 [ %add64.lcssa, %for.inc70 ]
  store i32 %add64.lcssa.lcssa, i32* %y, align 4, !tbaa !2
  %cmp = icmp ult i32 %10, 91
  br i1 %cmp, label %for.body, label %for.end73
for.body:                                         ; preds = %entry, %for.cond.loopexit
  %p4.0100 = phi i32* [ %o, %entry ], [ %y, %for.cond.loopexit ]
  %10 = phi i32 [ 1, %entry ], [ %add, %for.cond.loopexit ]
  %add = add nuw nsw i32 %10, 1
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %e, i64 0, i64 %idxprom
  %11 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %sub = add nsw i32 %10, -1
  %idxprom8 = zext i32 %sub to i64
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %e, i64 0, i64 %idxprom8
  %mul = mul i32 %11, %11
  %sub19 = sub i32 %11, %mul
  store i32 %sub19, i32* %y, align 4, !tbaa !2
  %idxprom20 = zext i32 %10 to i64
  %arrayidx21 = getelementptr inbounds [100 x i32], [100 x i32]* %cw, i64 0, i64 %idxprom20, !intel-tbaa !6
  %12 = load i32, i32* %arrayidx21, align 4, !tbaa !6
  store i32 %12, i32* %p4.0100, align 4, !tbaa !2
  %arrayidx27 = getelementptr inbounds [100 x i32], [100 x i32]* %cw, i64 0, i64 %idxprom, !intel-tbaa !6
  %13 = load i32, i32* %arrayidx27, align 4, !tbaa !6
  %add28 = add i32 %13, %11
  store i32 %add28, i32* %o, align 4, !tbaa !2
  %14 = load i32, i32* %y, align 4, !tbaa !2
  %arrayidx34 = getelementptr inbounds [100 x i32], [100 x i32]* %o1, i64 0, i64 %idxprom8, !intel-tbaa !6
  %15 = load i32, i32* %arrayidx34, align 4, !tbaa !6
  %sub35.neg = sub i32 %11, %14
  %sub36 = add i32 %sub35.neg, %15
  store i32 %sub36, i32* %arrayidx9, align 4, !tbaa !6
  br label %for.cond43.preheader
for.cond43.preheader:                             ; preds = %for.body, %for.inc70
  %y.promoted = phi i32 [ %14, %for.body ], [ %add64.lcssa, %for.inc70 ]
  %indvars.iv103 = phi i64 [ 32, %for.body ], [ %indvars.iv.next104, %for.inc70 ]
  %16 = add nuw nsw i64 %indvars.iv103, 1
  %17 = trunc i64 %indvars.iv103 to i32
  %18 = add i32 %17, -2
  br label %for.body45
for.body45:                                       ; preds = %for.cond43.preheader, %for.body45
  %indvars.iv = phi i64 [ 1, %for.cond43.preheader ], [ %indvars.iv.next, %for.body45 ]
  %add64101 = phi i32 [ %y.promoted, %for.cond43.preheader ], [ %add64, %for.body45 ]
  %19 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx50 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %i2, i64 0, i64 %19, i64 %indvars.iv, !intel-tbaa !8
  %20 = load i32, i32* %arrayidx50, align 4, !tbaa !8
  %arrayidx55 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %i2, i64 0, i64 %16, i64 %indvars.iv, !intel-tbaa !8
  store i32 %20, i32* %arrayidx55, align 4, !tbaa !8
  %mul63 = mul i32 %add64101, %add28
  %add64 = add i32 %mul63, %add64101
  %arrayidx67 = getelementptr inbounds [100 x i32], [100 x i32]* %cw, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %21 = load i32, i32* %arrayidx67, align 4, !tbaa !6
  %add68 = add i32 %18, %21
  store i32 %add68, i32* %arrayidx67, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp44 = icmp ult i64 %indvars.iv, 3
  br i1 %cmp44, label %for.body45, label %for.inc70
for.inc70:                                        ; preds = %for.body45
  %add64.lcssa = phi i32 [ %add64, %for.body45 ]
  %indvars.iv.next104 = add nsw i64 %indvars.iv103, -1
  %cmp41 = icmp ugt i64 %indvars.iv.next104, 1
  br i1 %cmp41, label %for.cond43.preheader, label %for.cond.loopexit
for.end73:                                        ; preds = %for.cond.loopexit
  %add.lcssa = phi i32 [ %add, %for.cond.loopexit ]
  %add28.lcssa = phi i32 [ %add28, %for.cond.loopexit ]
  %add64.lcssa.lcssa.lcssa = phi i32 [ %add64.lcssa.lcssa, %for.cond.loopexit ]
  store i32 1, i32* %j6, align 4, !tbaa !2
  store i32 7, i32* %jy3, align 4, !tbaa !2
  store i32 %add.lcssa, i32* %id4, align 4, !tbaa !2
  %call77 = call i32 (i32*, i32, ...) bitcast (i32 (...)* @checkSum to i32 (i32*, i32, ...)*)(i32* nonnull %arraydecay, i32 100) #5
  %call79 = call i32 (i32*, i32, ...) bitcast (i32 (...)* @checkSum to i32 (i32*, i32, ...)*)(i32* nonnull %arraydecay1, i32 100) #5
  %call82 = call i32 (i32*, i32, ...) bitcast (i32 (...)* @checkSum to i32 (i32*, i32, ...)*)(i32* nonnull %arraydecay3, i32 100) #5
  %call85 = call i32 (i32*, i32, ...) bitcast (i32 (...)* @checkSum to i32 (i32*, i32, ...)*)(i32* nonnull %9, i32 10000) #5
  %add80 = add i32 %add28.lcssa, %add64.lcssa.lcssa.lcssa
  %sub83 = add i32 %add80, -1
  %add86 = add i32 %sub83, %call77
  %add74 = add i32 %add86, %call79
  %sub75 = sub i32 %add74, %call82
  %add87 = add i32 %sub75, %call85
  %call88 = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([10 x i8], [10 x i8]* @.str.1, i64 0, i64 0), i32 %add87)
  call void @llvm.lifetime.end.p0i8(i64 40000, i8* nonnull %8) #5
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %7) #5
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %6) #5
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %5) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %4) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #5
  ret i32 0
}
; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1
; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #2
declare dso_local i32 @init(...) local_unnamed_addr #3
; Function Attrs: nofree nounwind
declare dso_local i32 @__isoc99_scanf(i8* nocapture readonly, ...) local_unnamed_addr #4
declare dso_local i32 @checkSum(...) local_unnamed_addr #3
; Function Attrs: nofree nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #4
; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1
attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { argmemonly nounwind willreturn writeonly }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }
!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA100_j", !3, i64 0}
!8 = !{!9, !3, i64 0}
!9 = !{!"array@_ZTSA100_A100_j", !7, i64 0}
