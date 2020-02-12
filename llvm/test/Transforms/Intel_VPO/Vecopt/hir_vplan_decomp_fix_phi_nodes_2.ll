; Test to verify correctness of the PHI node fixing algorithm for a slightly complex HCFG with two variables. The incoming value for a PHI node
; is not necessarily defined in the predecessor VPBB, but the value rather flows in from another predecessor in the control flow.

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-plain-cfg -S -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,vplan-driver-hir"  -vplan-print-plain-cfg -S -disable-output < %s 2>&1 | FileCheck %s

; Input HIR
; <73>    + DO i1 = 0, 1023, 1   <DO_LOOP>
; <8>     |   %t2.069.out3 = %t2.069;
; <10>    |   %1 = (@a)[0][i1];
; <14>    |   if (i1 > %1)
; <14>    |   {
; <25>    |      if (%1 > 1023)
; <25>    |      {
; <37>    |         %t2.1 = %t2.069.out3;
; <38>    |         %t1.0 = 2 * %1;
; <25>    |      }
; <25>    |      else
; <25>    |      {
; <31>    |         %4 = (@a)[0][i1 + 134];
; <32>    |         %t2.1 = %4;
; <33>    |         %t1.0 = %1;
; <25>    |      }
; <42>    |      %t2.069 = %t2.1 + 1024;
; <43>    |      %t1.1 = %t1.0;
; <14>    |   }
; <14>    |   else
; <14>    |   {
; <20>    |      %6 = (@a)[0][i1 + 1];
; <21>    |      %t1.1 = %6;
; <14>    |   }
; <46>    |   %t2.069.out = %t2.069;
; <48>    |   (@b)[0][i1] = %t1.1;
; <51>    |   (@c)[0][i1] = %t1.1 + 2 * %N;
; <53>    |   %7 = (@d)[0][i1];
; <57>    |   %spec.select = (i1 > %t1.1 + 2 * %N + %7) ? %t1.1 : %7;
; <58>    |   (@d)[0][i1] = %spec.select;
; <73>    + END LOOP

; Here PHI nodes are inserted at multiple VPBBs for %t1.0, %t2.1 and %t2.069. The PHI placed at loop latch VPBB for %t2.069 is of interest
; for us. The incoming value for this PHI node does not directly come the predecessor BB9 (outer else), but rather from BB4 which precedes BB9.


; Check the plain CFG structure and correctness of incoming values of PHI nodes
; CHECK-LABEL:  Print after buildPlainCFG
; CHECK-NEXT:    REGION: [[REGION0:region[0-9]+]]
; CHECK-NEXT:    [[BB0:BB[0-9]+]]:
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[BB1:BB[0-9]+]]
; CHECK:         [[BB1]]:
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[BB2:BB[0-9]+]]
; CHECK:         [[BB2]]:
; CHECK-NEXT:     i32 [[VP0:%.*]] = phi  [ i32 [[EXTERN_T2_069:%.*]], [[BB1]] ],  [ i32 [[LOOP_LATCH_PHI_T2:%.*]], [[BB3:BB[0-9]+]] ]
; CHECK-NEXT:     i64 [[VP2:%.*]] = phi  [ i64 0, [[BB1]] ],  [ i64 [[VP3:%.*]], [[BB3]] ]
; CHECK:         SUCCESSORS(2):[[BB4:BB[0-9]+]](i1 [[VP8:%vp.*]]), [[BB5:BB[0-9]+]](!i1 [[VP8]])
; CHECK:           [[BB5]]:
; CHECK:            i32 [[VP11:%.*]] = bitcast i32 {{%vp.*}}
; CHECK-NEXT:      SUCCESSORS(1):[[BB3]]
; CHECK:           [[BB4]]:
; CHECK:           SUCCESSORS(2):[[BB6:BB[0-9]+]](i1 [[VP12:%vp.*]]), [[BB7:BB[0-9]+]](!i1 [[VP12]])
; CHECK:             [[BB7]]:
; CHECK:              i32 [[VP15:%.*]] = bitcast i32 {{%vp.*}}
; CHECK:              i32 [[VP16:%.*]] = bitcast i32 {{%vp.*}}
; CHECK-NEXT:        SUCCESSORS(1):[[BB8:BB[0-9]+]]
; CHECK:             [[BB6]]:
; CHECK:              i32 [[VP17:%.*]] = bitcast i32 {{%vp.*}}
; CHECK:              i32 [[VP19:%.*]] = bitcast i32 {{%vp.*}}
; CHECK-NEXT:        SUCCESSORS(1):[[BB8]]
; CHECK:           [[BB8]]:
; CHECK-NEXT:       i32 [[VP21:%.*]] = phi  [ i32 [[VP19]], [[BB6]] ],  [ i32 [[VP16]], [[BB7]] ]
; CHECK-NEXT:       i32 [[VP20:%.*]] = phi  [ i32 [[VP17]], [[BB6]] ],  [ i32 [[VP15]], [[BB7]] ]
; CHECK:            i32 [[VP22:%.*]] = bitcast i32 {{%vp.*}}
; CHECK-NEXT:       i32 [[VP23:%.*]] = bitcast i32 {{%vp.*}}
; CHECK-NEXT:      SUCCESSORS(1):[[BB3]]
; CHECK:         [[BB3]]:
; CHECK-NEXT:     i32 [[VP24:%.*]] = phi  [ i32 [[VP23]], [[BB8]] ],  [ i32 [[VP11]], [[BB5]] ]
; CHECK-NEXT:     i32 [[LOOP_LATCH_PHI_T2]] = phi  [ i32 [[VP22]], [[BB8]] ],  [ i32 [[VP0]], [[BB5]] ]
; CHECK:          i64 [[VP3]] = add i64 [[VP2]] i64 1
; CHECK-NEXT:     i1 [[VP39:%.*]] = icmp i64 [[VP3]] i64 1023
; CHECK-NEXT:    SUCCESSORS(2):[[BB2]](i1 [[VP39]]), [[BB9:BB[0-9]+]](!i1 [[VP39]])
; CHECK:         [[BB9]]:
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[BB10:BB[0-9]+]]
; CHECK:         [[BB10]]:
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    no SUCCESSORS
; CHECK:         END Region([[REGION0]])


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@d = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(i32 %N) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  %mul23 = shl nsw i32 %N, 1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %if.end20, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %.pre, %if.end20 ], [ 0, %omp.inner.for.body.lr.ph ]
  %t2.069 = phi i32 [ %t2.2, %if.end20 ], [ undef, %omp.inner.for.body.lr.ph ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %2 = sext i32 %1 to i64
  %cmp2 = icmp sgt i64 %indvars.iv, %2
  %.pre = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp2, label %if.then, label %if.else16

if.then:                                          ; preds = %omp.inner.for.body
  %cmp7 = icmp sgt i32 %1, 1023
  br i1 %cmp7, label %if.then8, label %if.else

if.then8:                                         ; preds = %if.then
  %add11 = shl nsw i32 %1, 1
  br label %if.end

if.else:                                          ; preds = %if.then
  %3 = add nuw nsw i64 %indvars.iv, 134
  %arrayidx14 = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %3, !intel-tbaa !2
  %4 = load i32, i32* %arrayidx14, align 4, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then8
  %t2.1 = phi i32 [ %t2.069, %if.then8 ], [ %4, %if.else ]
  %t1.0 = phi i32 [ %add11, %if.then8 ], [ %1, %if.else ]
  %add15 = add nsw i32 %t2.1, 1024
  br label %if.end20

if.else16:                                        ; preds = %omp.inner.for.body
  %5 = add nuw nsw i64 %indvars.iv, 0
  %arrayidx19 = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %.pre, !intel-tbaa !2
  %6 = load i32, i32* %arrayidx19, align 4, !tbaa !2
  br label %if.end20

if.end20:                                         ; preds = %if.else16, %if.end
  %t2.2 = phi i32 [ %t2.069, %if.else16 ], [ %add15, %if.end ]
  %t1.1 = phi i32 [ %6, %if.else16 ], [ %t1.0, %if.end ]
  %arrayidx22 = getelementptr inbounds [1024 x i32], [1024 x i32]* @b, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %t1.1, i32* %arrayidx22, align 4, !tbaa !2
  %add24 = add nsw i32 %t1.1, %mul23
  %arrayidx26 = getelementptr inbounds [1024 x i32], [1024 x i32]* @c, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %add24, i32* %arrayidx26, align 4, !tbaa !2
  %arrayidx30 = getelementptr inbounds [1024 x i32], [1024 x i32]* @d, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %7 = load i32, i32* %arrayidx30, align 4, !tbaa !2
  %add31 = add nsw i32 %7, %add24
  %8 = sext i32 %add31 to i64
  %cmp32 = icmp sgt i64 %indvars.iv, %8
  %spec.select = select i1 %cmp32, i32 %t1.1, i32 %7
  store i32 %spec.select, i32* %arrayidx30, align 4, !tbaa !2
  %exitcond = icmp eq i64 %.pre, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %if.end20
  %t2.2.lcssa = phi i32 [ %t2.2, %if.end20 ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  %9 = load i32, i32* getelementptr inbounds ([1024 x i32], [1024 x i32]* @b, i64 0, i64 0), align 16, !tbaa !2
  %10 = load i32, i32* getelementptr inbounds ([1024 x i32], [1024 x i32]* @c, i64 0, i64 0), align 16, !tbaa !2
  %11 = load i32, i32* getelementptr inbounds ([1024 x i32], [1024 x i32]* @d, i64 0, i64 0), align 16, !tbaa !2
  %add40 = add i32 %9, %t2.2.lcssa
  %add41 = add i32 %add40, %10
  %add42 = add i32 %add41, %11
  ret i32 %add42
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }


!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
