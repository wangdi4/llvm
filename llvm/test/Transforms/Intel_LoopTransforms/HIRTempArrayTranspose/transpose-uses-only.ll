; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose" -hir-create-function-level-region -print-after=hir-temp-array-transpose -disable-output 2>&1 | FileCheck %s

; XFAIL:*

; Note: We don't need to handle this case for GB benchmark. Also, since the loops are conditional,
; it is unsafe to do the copy before the main loop since we may be accessing memory that would
; otherwise not be accessed. More checks would be needed to guarantee safety.

; Check that Temp Array Transpose occurs for array B for the following code. We want to make B
; unit-stride. Note: Test was compiled with -fno-exceptions due to std::vector checking.

; Source Code

; std::vector<unsigned> sum_cols_b(N);
; for (int j = 0; j < N; j++) {
;   sum_cols_b[j] = 0;
;   for (int k = 0; k < K; k++) {
;     sum_cols_b[j] += B[k * N + j];
;   }
; }
; for (int i = 0; i < M; i++) {
;   for (int j = 0; j < N; j++) {
;     unsigned accumulator = C[i * N + j];
;     for (int k = 0; k < K; k++) {
;       accumulator += A[i * K + k] * B[k * N + j];
;     }
;     accumulator -= B_0[j] * sum_cols_b[j];
;     accumulator += K * B_0[j];
;     C[i * N + j] += accumulator;
;   }
; }

; HIR Before Transformation
;    BEGIN REGION { }
;      if (%N < 0)
;      {
;         @_ZSt20__throw_length_errorPKc(&((@.str)[0][0]));
;         unreachable ;
;      }
;      else
;      {
;         %sum_cols_b.sroa.0.0174 = null;
;         if (%N != 0)
;         {
;            %call2 = @_Znwm(4 * sext.i32.i64(%N));
;            %0 = bitcast.i8*.i32*(&((%call2)[0]));
;            @llvm.memset.p0i8.i64(&((%call2)[0]),  0,  4 * sext.i32.i64(%N),  0);
;            if (%K > 0)
;            {
;               + DO i1 = 0, sext.i32.i64(%N) + -1, 1
;               |   (%0)[i1] = 0;
;               |
;               |   + DO i2 = 0, sext.i32.i64(%K) + -1, 1
;               |   |   %3 = (%B)[i1 + sext.i32.i64(%N) * i2];
;               |   |   %4 = (%0)[i1];
;               |   |   (%0)[i1] = %4 + sext.i8.i32(%3);
;               |   + END LOOP
;               + END LOOP
;               %sum_cols_b.sroa.0.0174 = &((i32*)(%call2)[0]);
;            }
;            else
;            {
;               + DO i1 = 0, sext.i32.i64(%N) + -1, 1
;               |   (%0)[i1] = 0;
;               + END LOOP
;               %sum_cols_b.sroa.0.0174 = &((i32*)(%call2)[0]);
;            }
;         }
;         if (%M > 0)
;         {
;            if (%N != 0)
;            {
;               %.pre = (%B_0)[0].0.0.0.0;
;               + DO i1 = 0, zext.i32.i64(%M) + -1, 1
;               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1
;               |   |   %9 = (%C)[sext.i32.i64(%N) * i1 + i2];
;               |   |   %accumulator.0.lcssa.us = %9;
;               |   |
;               |   |      %accumulator.0129.us = %9;
;               |   |   + DO i3 = 0, sext.i32.i64(%K) + -1, 1
;               |   |   |   %13 = (%A)[sext.i32.i64(%K) * i1 + i3];
;               |   |   |   %16 = (%B)[i2 + sext.i32.i64(%N) * i3];
;               |   |   |   %accumulator.0129.us = (sext.i8.i32(%13) * sext.i8.i32(%16))  +  %accumulator.0129.us;
;               |   |   + END LOOP
;               |   |      %accumulator.0.lcssa.us = %accumulator.0129.us;
;               |   |
;               |   |   %10 = (%.pre)[i2];
;               |   |   %11 = (%sum_cols_b.sroa.0.0174)[i2];
;               |   |   (%C)[sext.i32.i64(%N) * i1 + i2] = %9 + (sext.i8.i32(%10) * ((-1 * %11) + %K)) + %accumulator.0.lcssa.us;
;               |   + END LOOP
;               + END LOOP
;            }
;         }
;         if (&((%sum_cols_b.sroa.0.0174)[0]) != null)
;         {
;            @_ZdlPv(&((i8*)(%sum_cols_b.sroa.0.0174)[0]));
;         }
;         ret ;
;      }
;    END REGION


; HIR After Transformation
; CHECK: BEGIN REGION { modified }
; CHECK:   %call = @llvm.stacksave();
; CHECK:   %TranspTmpArr = alloca (sext.i32.i64(%N) * sext.i32.i64(%K));
; CHECK:   + DO i1 = 0, sext.i32.i64(%N) + -1, 1
; CHECK:   |   + DO i2 = 0, sext.i32.i64(%K) + -1, 1
; CHECK:   |   |   (%TranspTmpArr)[i1][i2] = (%B)[i2][i1];
; CHECK:   |   + END LOOP
; CHECK:   + END LOOP
;      if (%N < 0)
;      {
;         @_ZSt20__throw_length_errorPKc(&((@.str)[0][0]));
;         unreachable ;
;      }
;      else
;      {
;         %sum_cols_b.sroa.0.0174 = null;
;         if (%N != 0)
;         {
;            %call2 = @_Znwm(4 * sext.i32.i64(%N));
;            %0 = bitcast.i8*.i32*(&((%call2)[0]));
;            @llvm.memset.p0i8.i64(&((%call2)[0]),  0,  4 * sext.i32.i64(%N),  0);
;            if (%K > 0)
;            {
; CHECK:        + DO i1 = 0, sext.i32.i64(%N) + -1, 1
;               |   (%0)[i1] = 0;
;               |
; CHECK:        |   + DO i2 = 0, sext.i32.i64(%K) + -1, 1
; CHECK:        |   |   %3 = (%TranspTmpArr)[i1][i2];
;               |   |   %4 = (%0)[i1];
;               |   |   (%0)[i1] = %4 + sext.i8.i32(%3);
;               |   + END LOOP
;               + END LOOP
;               %sum_cols_b.sroa.0.0174 = &((i32*)(%call2)[0]);
;            }
;            else
;            {
;               + DO i1 = 0, sext.i32.i64(%N) + -1, 1
;               |   (%0)[i1] = 0;
;               + END LOOP
;               %sum_cols_b.sroa.0.0174 = &((i32*)(%call2)[0]);
;            }
;         }
;         if (%M > 0)
;         {
;            if (%N != 0)
;            {
;               %.pre = (%B_0)[0].0.0.0.0;
; CHECK:        + DO i1 = 0, zext.i32.i64(%M) + -1, 1
; CHECK:        |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1
;               |   |   %9 = (%C)[sext.i32.i64(%N) * i1 + i2];
;               |   |   %accumulator.0.lcssa.us = %9;
;               |   |
;               |   |      %accumulator.0129.us = %9;
; CHECK:        |   |   + DO i3 = 0, sext.i32.i64(%K) + -1, 1
;               |   |   |   %13 = (%A)[sext.i32.i64(%K) * i1 + i3];
; CHECK:        |   |   |   %16 = (%TranspTmpArr)[i2][i3];
;               |   |   |   %accumulator.0129.us = (sext.i8.i32(%13) * sext.i8.i32(%16))  +  %accumulator.0129.us;
;               |   |   + END LOOP
;               |   |      %accumulator.0.lcssa.us = %accumulator.0129.us;
;               |   |
;               |   |   %10 = (%.pre)[i2];
;               |   |   %11 = (%sum_cols_b.sroa.0.0174)[i2];
;               |   |   (%C)[sext.i32.i64(%N) * i1 + i2] = %9 + (sext.i8.i32(%10) * ((-1 * %11) + %K)) + %accumulator.0.lcssa.us;
;               |   + END LOOP
;               + END LOOP
;            }
;         }
;         if (&((%sum_cols_b.sroa.0.0174)[0]) != null)
;         {
;            @_ZdlPv(&((i8*)(%sum_cols_b.sroa.0.0174)[0]));
;         }
;         ret ;
;      }
; CHECK:   @llvm.stackrestore(&((%call)[0]));
;    END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
%"class.std::ios_base::Init" = type { i8 }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl" }
%"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl" = type { %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data" }
%"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data" = type { i8*, i8*, i8* }

@.str = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @_Z16gemm_lowp_matmuliiiPKcS0_PjSt6vectorIcSaIcEE(i32 noundef %M, i32 noundef %N, i32 noundef %K, i8* nocapture noundef readonly %A, i8* nocapture noundef readonly %B, i32* noalias nocapture noundef %C, %"class.std::vector"* nocapture noundef readonly %B_0) local_unnamed_addr #3 {
entry:
  %conv = sext i32 %N to i64
  %cmp.i.i = icmp slt i32 %N, 0
  br i1 %cmp.i.i, label %if.then.i.i, label %_ZNSt6vectorIjSaIjEE17_S_check_init_lenEmRKS0_.exit.i

if.then.i.i:                                      ; preds = %entry
  tail call void @_ZSt20__throw_length_errorPKc(i8* noundef getelementptr inbounds ([49 x i8], [49 x i8]* @.str, i64 0, i64 0)) #9
  unreachable

_ZNSt6vectorIjSaIjEE17_S_check_init_lenEmRKS0_.exit.i: ; preds = %entry
  %cmp.not.i.i.i.i = icmp eq i32 %N, 0
  br i1 %cmp.not.i.i.i.i, label %for.cond13.preheader, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %_ZNSt6vectorIjSaIjEE17_S_check_init_lenEmRKS0_.exit.i
  %mul.i.i.i.i.i.i = shl nuw nsw i64 %conv, 2
  %call2 = tail call noalias noundef nonnull i8* @_Znwm(i64 noundef %mul.i.i.i.i.i.i) #10
  %0 = bitcast i8* %call2 to i32*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull align 4 %call2, i8 0, i64 %mul.i.i.i.i.i.i, i1 false) #11, !tbaa !3
  %cmp3136 = icmp sgt i32 %K, 0
  br i1 %cmp3136, label %for.body.us.preheader, label %for.body.preheader

for.body.preheader:                               ; preds = %for.body.lr.ph
  br label %for.body

for.body.us.preheader:                            ; preds = %for.body.lr.ph
  %wide.trip.count161 = sext i32 %K to i64
  br label %for.body.us

for.body.us:                                      ; preds = %for.body.us.preheader, %for.cond2.for.cond.cleanup4_crit_edge.us
  %indvars.iv163 = phi i64 [ 0, %for.body.us.preheader ], [ %indvars.iv.next164, %for.cond2.for.cond.cleanup4_crit_edge.us ]
  %add.ptr.i113.us = getelementptr inbounds i32, i32* %0, i64 %indvars.iv163, !intel-tbaa !3
  store i32 0, i32* %add.ptr.i113.us, align 4, !tbaa !3
  br label %for.body5.us

for.body5.us:                                     ; preds = %for.body.us, %for.body5.us
  %indvars.iv157 = phi i64 [ 0, %for.body.us ], [ %indvars.iv.next158, %for.body5.us ]
  %1 = mul nsw i64 %indvars.iv157, %conv
  %2 = add nsw i64 %1, %indvars.iv163
  %arrayidx.us = getelementptr inbounds i8, i8* %B, i64 %2
  %3 = load i8, i8* %arrayidx.us, align 1, !tbaa !7
  %conv6.us = sext i8 %3 to i32
  %4 = load i32, i32* %add.ptr.i113.us, align 4, !tbaa !3
  %add9.us = add i32 %4, %conv6.us
  store i32 %add9.us, i32* %add.ptr.i113.us, align 4, !tbaa !3
  %indvars.iv.next158 = add nuw nsw i64 %indvars.iv157, 1
  %exitcond162.not = icmp eq i64 %indvars.iv.next158, %wide.trip.count161
  br i1 %exitcond162.not, label %for.cond2.for.cond.cleanup4_crit_edge.us, label %for.body5.us, !llvm.loop !8

for.cond2.for.cond.cleanup4_crit_edge.us:         ; preds = %for.body5.us
  %indvars.iv.next164 = add nuw nsw i64 %indvars.iv163, 1
  %exitcond166.not = icmp eq i64 %indvars.iv.next164, %conv
  br i1 %exitcond166.not, label %for.cond13.preheader.loopexit, label %for.body.us, !llvm.loop !10

for.cond13.preheader.loopexit:                    ; preds = %for.cond2.for.cond.cleanup4_crit_edge.us
  br label %for.cond13.preheader

for.cond13.preheader.loopexit180:                 ; preds = %for.body
  br label %for.cond13.preheader

for.cond13.preheader:                             ; preds = %for.cond13.preheader.loopexit180, %for.cond13.preheader.loopexit, %_ZNSt6vectorIjSaIjEE17_S_check_init_lenEmRKS0_.exit.i
  %sum_cols_b.sroa.0.0174 = phi i32* [ null, %_ZNSt6vectorIjSaIjEE17_S_check_init_lenEmRKS0_.exit.i ], [ %0, %for.cond13.preheader.loopexit ], [ %0, %for.cond13.preheader.loopexit180 ]
  %cmp14133 = icmp sgt i32 %M, 0
  br i1 %cmp14133, label %for.cond18.preheader.lr.ph, label %for.cond.cleanup15

for.cond18.preheader.lr.ph:                       ; preds = %for.cond13.preheader
  %cmp28128 = icmp sgt i32 %K, 0
  br i1 %cmp.not.i.i.i.i, label %for.cond.cleanup15, label %for.cond18.preheader.us.preheader

for.cond18.preheader.us.preheader:                ; preds = %for.cond18.preheader.lr.ph
  %_M_start.i110 = getelementptr inbounds %"class.std::vector", %"class.std::vector"* %B_0, i64 0, i32 0, i32 0, i32 0, i32 0
  %5 = sext i32 %K to i64
  %wide.trip.count155178 = zext i32 %M to i64
  %.pre = load i8*, i8** %_M_start.i110, align 8, !tbaa !11, !std.container.ptr !14
  br label %for.cond18.preheader.us

for.cond18.preheader.us:                          ; preds = %for.cond18.preheader.us.preheader, %for.cond18.for.cond.cleanup20_crit_edge.us
  %indvars.iv151 = phi i64 [ 0, %for.cond18.preheader.us.preheader ], [ %indvars.iv.next152, %for.cond18.for.cond.cleanup20_crit_edge.us ]
  %6 = mul nsw i64 %indvars.iv151, %conv
  %7 = mul nsw i64 %indvars.iv151, %5
  br label %for.body21.us

for.body21.us:                                    ; preds = %for.cond18.preheader.us, %for.cond.cleanup29.us
  %indvars.iv146 = phi i64 [ 0, %for.cond18.preheader.us ], [ %indvars.iv.next147, %for.cond.cleanup29.us ]
  %8 = add nsw i64 %indvars.iv146, %6
  %arrayidx25.us = getelementptr inbounds i32, i32* %C, i64 %8
  %9 = load i32, i32* %arrayidx25.us, align 4, !tbaa !3
  br i1 %cmp28128, label %for.body30.us.preheader, label %for.cond.cleanup29.us

for.body30.us.preheader:                          ; preds = %for.body21.us
  br label %for.body30.us

for.cond.cleanup29.us.loopexit:                   ; preds = %for.body30.us
  %add42.us.lcssa = phi i32 [ %add42.us, %for.body30.us ]
  br label %for.cond.cleanup29.us

for.cond.cleanup29.us:                            ; preds = %for.cond.cleanup29.us.loopexit, %for.body21.us
  %accumulator.0.lcssa.us = phi i32 [ %9, %for.body21.us ], [ %add42.us.lcssa, %for.cond.cleanup29.us.loopexit ]
  %add.ptr.i111.us = getelementptr inbounds i8, i8* %.pre, i64 %indvars.iv146, !intel-tbaa !7
  %10 = load i8, i8* %add.ptr.i111.us, align 1, !tbaa !7, !std.container.ptr !14
  %conv48.us = sext i8 %10 to i32
  %add.ptr.i109.us = getelementptr inbounds i32, i32* %sum_cols_b.sroa.0.0174, i64 %indvars.iv146, !intel-tbaa !3
  %11 = load i32, i32* %add.ptr.i109.us, align 4, !tbaa !3
  %reass.add.us = sub i32 %K, %11
  %reass.mul.us = mul i32 %reass.add.us, %conv48.us
  %add56.us = add i32 %accumulator.0.lcssa.us, %9
  %add61.us = add i32 %add56.us, %reass.mul.us
  store i32 %add61.us, i32* %arrayidx25.us, align 4, !tbaa !3
  %indvars.iv.next147 = add nuw nsw i64 %indvars.iv146, 1
  %exitcond150.not = icmp eq i64 %indvars.iv.next147, %conv
  br i1 %exitcond150.not, label %for.cond18.for.cond.cleanup20_crit_edge.us, label %for.body21.us, !llvm.loop !15

for.body30.us:                                    ; preds = %for.body30.us.preheader, %for.body30.us
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body30.us ], [ 0, %for.body30.us.preheader ]
  %accumulator.0129.us = phi i32 [ %add42.us, %for.body30.us ], [ %9, %for.body30.us.preheader ]
  %12 = add nsw i64 %indvars.iv, %7
  %arrayidx34.us = getelementptr inbounds i8, i8* %A, i64 %12
  %13 = load i8, i8* %arrayidx34.us, align 1, !tbaa !7
  %conv35.us = sext i8 %13 to i32
  %14 = mul nsw i64 %indvars.iv, %conv
  %15 = add nsw i64 %14, %indvars.iv146
  %arrayidx39.us = getelementptr inbounds i8, i8* %B, i64 %15
  %16 = load i8, i8* %arrayidx39.us, align 1, !tbaa !7
  %conv40.us = sext i8 %16 to i32
  %mul41.us = mul nsw i32 %conv40.us, %conv35.us
  %add42.us = add i32 %mul41.us, %accumulator.0129.us
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %5
  br i1 %exitcond.not, label %for.cond.cleanup29.us.loopexit, label %for.body30.us, !llvm.loop !16

for.cond18.for.cond.cleanup20_crit_edge.us:       ; preds = %for.cond.cleanup29.us
  %indvars.iv.next152 = add nuw nsw i64 %indvars.iv151, 1
  %exitcond156.not = icmp eq i64 %indvars.iv.next152, %wide.trip.count155178
  br i1 %exitcond156.not, label %for.cond.cleanup15.loopexit, label %for.cond18.preheader.us, !llvm.loop !17

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv167 = phi i64 [ %indvars.iv.next168, %for.body ], [ 0, %for.body.preheader ]
  %add.ptr.i113 = getelementptr inbounds i32, i32* %0, i64 %indvars.iv167, !intel-tbaa !3
  store i32 0, i32* %add.ptr.i113, align 4, !tbaa !3
  %indvars.iv.next168 = add nuw nsw i64 %indvars.iv167, 1
  %exitcond170.not = icmp eq i64 %indvars.iv.next168, %conv
  br i1 %exitcond170.not, label %for.cond13.preheader.loopexit180, label %for.body, !llvm.loop !10

for.cond.cleanup15.loopexit:                      ; preds = %for.cond18.for.cond.cleanup20_crit_edge.us
  br label %for.cond.cleanup15

for.cond.cleanup15:                               ; preds = %for.cond.cleanup15.loopexit, %for.cond18.preheader.lr.ph, %for.cond13.preheader
  %tobool.not.i.i.i = icmp eq i32* %sum_cols_b.sroa.0.0174, null
  br i1 %tobool.not.i.i.i, label %_ZNSt6vectorIjSaIjEED2Ev.exit, label %if.then.i.i.i

if.then.i.i.i:                                    ; preds = %for.cond.cleanup15
  %17 = bitcast i32* %sum_cols_b.sroa.0.0174 to i8*
  tail call void @_ZdlPv(i8* noundef nonnull %17) #11
  br label %_ZNSt6vectorIjSaIjEED2Ev.exit

_ZNSt6vectorIjSaIjEED2Ev.exit:                    ; preds = %for.cond.cleanup15, %if.then.i.i.i
  ret void
}

; Function Attrs: nofree noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(i8* noundef) local_unnamed_addr #4

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noundef nonnull i8* @_Znwm(i64 noundef) local_unnamed_addr #5

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(i8* noundef) local_unnamed_addr #6

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #8

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!5, !5, i64 0}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
!11 = !{!12, !13, i64 0}
!12 = !{!"struct@_ZTSNSt12_Vector_baseIcSaIcEE17_Vector_impl_dataE", !13, i64 0, !13, i64 8, !13, i64 16}
!13 = !{!"pointer@_ZTSPc", !5, i64 0}
!14 = !{i32 0}
!15 = distinct !{!15, !9}
!16 = distinct !{!16, !9}
!17 = distinct !{!17, !9}
