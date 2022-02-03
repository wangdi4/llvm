; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -print-after=hir-loop-interchange -disable-output  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange" -aa-pipeline="basic-aa" -print-after=hir-loop-interchange -disable-output  < %s 2>&1 | FileCheck %s

; Verify that loop interchange doesn't happen.
; Loop is more helpful to jump-threading without interchange.

; Function: InvMixColumn
;
;         BEGIN REGION { }
;               + DO i1 = 0, zext.i8.i64(%BC) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8>
;               |   + DO i2 = 0, 3, 1   <DO_LOOP>
;               |   |   %2 = (%a)[i2][i1];
;               |   |   %retval.0.i = 0;
;               |   |   if (%2 != 0)
;               |   |   {
;               |   |      %3 = (@Logtable)[0][14];
;               |   |      %4 = (@Logtable)[0][%2];
;               |   |      %rem11.i = zext.i8.i16(%4) + zext.i8.i16(%3)  %u  255;
;               |   |      %retval.0.i = (@Alogtable)[0][%rem11.i];
;               |   |   }
;               |   |   %7 = (%a)[i2 + 1][i1];
;               |   |   %retval.0.i125 = 0;
;               |   |   if (%7 != 0)
;               |   |   {
;               |   |      %8 = (@Logtable)[0][11];
;               |   |      %9 = (@Logtable)[0][%7];
;               |   |      %rem11.i122 = zext.i8.i16(%9) + zext.i8.i16(%8)  %u  255;
;               |   |      %retval.0.i125 = (@Alogtable)[0][%rem11.i122];
;               |   |   }
;               |   |   %xor91 = %retval.0.i  ^  %retval.0.i125;
;               |   |   %13 = (%a)[i2 + -2][i1];
;               |   |   %retval.0.i114 = 0;
;               |   |   if (%13 != 0)
;               |   |   {
;               |   |      %14 = (@Logtable)[0][13];
;               |   |      %15 = (@Logtable)[0][%13];
;               |   |      %rem11.i111 = zext.i8.i16(%15) + zext.i8.i16(%14)  %u  255;
;               |   |      %retval.0.i114 = (@Alogtable)[0][%rem11.i111];
;               |   |   }
;               |   |   %xor2392 = %xor91  ^  %retval.0.i114;
;               |   |   %19 = (%a)[i2 + -1][i1];
;               |   |   %retval.0.i103 = 0;
;               |   |   if (%19 != 0)
;               |   |   {
;               |   |      %20 = (@Logtable)[0][9];
;               |   |      %21 = (@Logtable)[0][%19];
;               |   |      %rem11.i100 = zext.i8.i16(%21) + zext.i8.i16(%20)  %u  255;
;               |   |      %retval.0.i103 = (@Alogtable)[0][%rem11.i100];
;               |   |   }
;               |   |   %xor3293 = %xor2392  ^  %retval.0.i103;
;               |   |   (%b)[0][i2][i1] = %xor3293;
;               |   + END LOOP
;               + END LOOP
;         END REGION

; CHECK: Function: InvMixColumn
; CHECK-NOT: BEGIN REGION { modified }

; ModuleID = 'input.ll'
source_filename = "aes/rijndael-alg-ref.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@Alogtable = external dso_local local_unnamed_addr global [0 x i8], align 1
@Logtable = external dso_local local_unnamed_addr global [0 x i8], align 1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: nofree nosync nounwind uwtable
define dso_local void @InvMixColumn([8 x i8]* nocapture %a, i8 zeroext %BC) local_unnamed_addr #1 {
entry:
  %b = alloca [4 x [8 x i8]], align 16
  %0 = getelementptr inbounds [4 x [8 x i8]], [4 x [8 x i8]]* %b, i64 0, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %0) #3
  %cmp131.not = icmp eq i8 %BC, 0
  br i1 %cmp131.not, label %for.end63, label %for.cond2.preheader.preheader

for.cond2.preheader.preheader:                    ; preds = %entry
  %1 = zext i8 %BC to i64
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.inc38, %for.cond2.preheader.preheader
  %indvars.iv141 = phi i64 [ 0, %for.cond2.preheader.preheader ], [ %indvars.iv.next142, %for.inc38 ]
  br label %for.body5

for.cond41.preheader:                             ; preds = %for.inc38
  br label %for.end63

for.cond45.preheader.us.preheader:                ; preds = %for.cond41.preheader
  br label %for.cond45.preheader.us

for.cond45.preheader.us:                          ; preds = %for.cond45.preheader.us, %for.cond45.preheader.us.preheader
  %indvar = phi i64 [ 0, %for.cond45.preheader.us.preheader ], [ %indvar.next, %for.cond45.preheader.us ]
  %scevgep = getelementptr [8 x i8], [8 x i8]* %a, i64 %indvar, i64 0
  %scevgep134 = getelementptr [4 x [8 x i8]], [4 x [8 x i8]]* %b, i64 0, i64 %indvar, i64 0
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %scevgep, i8* align 8 %scevgep134, i64 %1, i1 false)
  %indvar.next = add nuw nsw i64 %indvar, 1
  %exitcond.not = icmp eq i64 %indvar.next, 4
  br i1 %exitcond.not, label %for.end63.loopexit, label %for.cond45.preheader.us

for.body5:                                        ; preds = %mul.exit104, %for.cond2.preheader
  %indvars.iv = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next, %mul.exit104 ]
  %arrayidx7 = getelementptr inbounds [8 x i8], [8 x i8]* %a, i64 %indvars.iv, i64 %indvars.iv141
  %2 = load i8, i8* %arrayidx7, align 1, !tbaa !3
  %tobool2.not.i = icmp eq i8 %2, 0
  br i1 %tobool2.not.i, label %mul.exit, label %if.then.i

if.then.i:                                        ; preds = %for.body5
  %3 = load i8, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @Logtable, i64 0, i64 14), align 1, !tbaa !7
  %conv3.i = zext i8 %3 to i16
  %idxprom4.i = zext i8 %2 to i64
  %arrayidx5.i = getelementptr inbounds [0 x i8], [0 x i8]* @Logtable, i64 0, i64 %idxprom4.i
  %4 = load i8, i8* %arrayidx5.i, align 1, !tbaa !7
  %conv6.i = zext i8 %4 to i16
  %add.i = add nuw nsw i16 %conv6.i, %conv3.i
  %rem11.i = urem i16 %add.i, 255
  %5 = zext i16 %rem11.i to i64
  %arrayidx8.i = getelementptr inbounds [0 x i8], [0 x i8]* @Alogtable, i64 0, i64 %5
  %6 = load i8, i8* %arrayidx8.i, align 1, !tbaa !7
  br label %mul.exit

mul.exit:                                         ; preds = %if.then.i, %for.body5
  %retval.0.i = phi i8 [ %6, %if.then.i ], [ 0, %for.body5 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %rem = and i64 %indvars.iv.next, 3
  %arrayidx12 = getelementptr inbounds [8 x i8], [8 x i8]* %a, i64 %rem, i64 %indvars.iv141
  %7 = load i8, i8* %arrayidx12, align 1, !tbaa !3
  %tobool2.not.i116 = icmp eq i8 %7, 0
  br i1 %tobool2.not.i116, label %mul.exit126, label %if.then.i124

if.then.i124:                                     ; preds = %mul.exit
  %8 = load i8, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @Logtable, i64 0, i64 11), align 1, !tbaa !7
  %conv3.i117 = zext i8 %8 to i16
  %idxprom4.i118 = zext i8 %7 to i64
  %arrayidx5.i119 = getelementptr inbounds [0 x i8], [0 x i8]* @Logtable, i64 0, i64 %idxprom4.i118
  %9 = load i8, i8* %arrayidx5.i119, align 1, !tbaa !7
  %conv6.i120 = zext i8 %9 to i16
  %add.i121 = add nuw nsw i16 %conv6.i120, %conv3.i117
  %rem11.i122 = urem i16 %add.i121, 255
  %10 = zext i16 %rem11.i122 to i64
  %arrayidx8.i123 = getelementptr inbounds [0 x i8], [0 x i8]* @Alogtable, i64 0, i64 %10
  %11 = load i8, i8* %arrayidx8.i123, align 1, !tbaa !7
  br label %mul.exit126

mul.exit126:                                      ; preds = %if.then.i124, %mul.exit
  %retval.0.i125 = phi i8 [ %11, %if.then.i124 ], [ 0, %mul.exit ]
  %xor91 = xor i8 %retval.0.i, %retval.0.i125
  %12 = add nuw i64 %indvars.iv, 2
  %rem16 = and i64 %12, 3
  %arrayidx20 = getelementptr inbounds [8 x i8], [8 x i8]* %a, i64 %rem16, i64 %indvars.iv141
  %13 = load i8, i8* %arrayidx20, align 1, !tbaa !3
  %tobool2.not.i105 = icmp eq i8 %13, 0
  br i1 %tobool2.not.i105, label %mul.exit115, label %if.then.i113

if.then.i113:                                     ; preds = %mul.exit126
  %14 = load i8, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @Logtable, i64 0, i64 13), align 1, !tbaa !7
  %conv3.i106 = zext i8 %14 to i16
  %idxprom4.i107 = zext i8 %13 to i64
  %arrayidx5.i108 = getelementptr inbounds [0 x i8], [0 x i8]* @Logtable, i64 0, i64 %idxprom4.i107
  %15 = load i8, i8* %arrayidx5.i108, align 1, !tbaa !7
  %conv6.i109 = zext i8 %15 to i16
  %add.i110 = add nuw nsw i16 %conv6.i109, %conv3.i106
  %rem11.i111 = urem i16 %add.i110, 255
  %16 = zext i16 %rem11.i111 to i64
  %arrayidx8.i112 = getelementptr inbounds [0 x i8], [0 x i8]* @Alogtable, i64 0, i64 %16
  %17 = load i8, i8* %arrayidx8.i112, align 1, !tbaa !7
  br label %mul.exit115

mul.exit115:                                      ; preds = %if.then.i113, %mul.exit126
  %retval.0.i114 = phi i8 [ %17, %if.then.i113 ], [ 0, %mul.exit126 ]
  %xor2392 = xor i8 %xor91, %retval.0.i114
  %18 = add nuw i64 %indvars.iv, 3
  %rem25 = and i64 %18, 3
  %arrayidx29 = getelementptr inbounds [8 x i8], [8 x i8]* %a, i64 %rem25, i64 %indvars.iv141
  %19 = load i8, i8* %arrayidx29, align 1, !tbaa !3
  %tobool2.not.i94 = icmp eq i8 %19, 0
  br i1 %tobool2.not.i94, label %mul.exit104, label %if.then.i102

if.then.i102:                                     ; preds = %mul.exit115
  %20 = load i8, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @Logtable, i64 0, i64 9), align 1, !tbaa !7
  %conv3.i95 = zext i8 %20 to i16
  %idxprom4.i96 = zext i8 %19 to i64
  %arrayidx5.i97 = getelementptr inbounds [0 x i8], [0 x i8]* @Logtable, i64 0, i64 %idxprom4.i96
  %21 = load i8, i8* %arrayidx5.i97, align 1, !tbaa !7
  %conv6.i98 = zext i8 %21 to i16
  %add.i99 = add nuw nsw i16 %conv6.i98, %conv3.i95
  %rem11.i100 = urem i16 %add.i99, 255
  %22 = zext i16 %rem11.i100 to i64
  %arrayidx8.i101 = getelementptr inbounds [0 x i8], [0 x i8]* @Alogtable, i64 0, i64 %22
  %23 = load i8, i8* %arrayidx8.i101, align 1, !tbaa !7
  br label %mul.exit104

mul.exit104:                                      ; preds = %if.then.i102, %mul.exit115
  %retval.0.i103 = phi i8 [ %23, %if.then.i102 ], [ 0, %mul.exit115 ]
  %xor3293 = xor i8 %xor2392, %retval.0.i103
  %arrayidx37 = getelementptr inbounds [4 x [8 x i8]], [4 x [8 x i8]]* %b, i64 0, i64 %indvars.iv, i64 %indvars.iv141, !intel-tbaa !8
  store i8 %xor3293, i8* %arrayidx37, align 1, !tbaa !8
  %exitcond140.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond140.not, label %for.inc38, label %for.body5

for.inc38:                                        ; preds = %mul.exit104
  %indvars.iv.next142 = add nuw nsw i64 %indvars.iv141, 1
  %exitcond144.not = icmp eq i64 %indvars.iv.next142, %1
  br i1 %exitcond144.not, label %for.cond41.preheader, label %for.cond2.preheader

for.end63.loopexit:                               ; preds = %for.cond45.preheader.us
  br label %for.end63

for.end63:                                        ; preds = %for.end63.loopexit, %for.cond41.preheader, %entry
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %0) #3
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
attributes #1 = { nofree nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #2 = { argmemonly nofree nounwind willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA8_h", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!5, !5, i64 0}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA4_A8_h", !4, i64 0}
