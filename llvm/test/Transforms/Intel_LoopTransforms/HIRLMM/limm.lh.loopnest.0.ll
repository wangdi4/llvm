; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm,print<hir>" -aa-pipeline="basic-aa" -hir-details  < %s 2>&1 | FileCheck %s
;
; C Source Code:
;double HEKMAN[100][100];
;double G[100][100];
;
;int foo(void) {
;  double FETCH[10000];
;  int i, j;
;
;  for (i = 0; i <= 7; ++i) {
;    for (j = 0; j <= 53; ++j) {
;      double t0 = -(FETCH)[60];
;      if (HEKMAN[i][j] >= t0) {
;        G[i][j] = 0;
;      }
;    }
;  }
;  return 0;
;}
;
;[Note]
; - This is a LIT case demonstrating the proper promotion of a Loop-inv load into outermost lp.
;   Notice that %limm is promoted into outermost loop. Its defined@level is non-linear (10), and its use
;   has def@level: 0 (linear).
;

;*** IR Dump After HIR Loop Memory Motion ***
;
;<0>          BEGIN REGION { modified }
;CHECK:               %limm = (%FETCH.sroa.2)[0];
;CHECK:                <LVAL-REG> NON-LINEAR double %limm
;[Note] the above HLInst is the load promoted into the region level (outside of any loopnest).
;                     ...
;
;                   + DO i64 i1 = 0, 7, 1   <DO_LOOP>
;                   |   + DO i64 i2 = 0, 53, 1   <DO_LOOP>
;CHECK:             |   |   %fneg =  - %limm;
;CHECK:             |   |   <LVAL-REG> NON-LINEAR double %fneg
;CHECK:             |   |   <RVAL-REG> LINEAR double %limm
;[Note] the above HLInst is the use of the load result from before the loopnest. %limm's def@level is 0 (Linear).
;                   ...
;                   |   + END LOOP
;                   + END LOOP
;           END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@HEKMAN = dso_local global [100 x [100 x double]] zeroinitializer, align 16
@G = dso_local global [100 x [100 x double]] zeroinitializer, align 16
@FCORT = dso_local local_unnamed_addr global [100 x [100 x double]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  %FETCH.sroa.2 = alloca double, align 16
  %FETCH.sroa.2.0..sroa_cast37 = bitcast ptr %FETCH.sroa.2 to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %FETCH.sroa.2.0..sroa_cast37)
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc12
  %indvars.iv34 = phi i64 [ 0, %entry ], [ %indvars.iv.next35, %for.inc12 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %if.end
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %if.end ]
  %FETCH.sroa.2.0.FETCH.sroa.2.0.FETCH.sroa.2.0.FETCH.sroa.2.480. = load  double, ptr %FETCH.sroa.2, align 16, !tbaa !2
  %fneg = fneg fast double %FETCH.sroa.2.0.FETCH.sroa.2.0.FETCH.sroa.2.0.FETCH.sroa.2.480.
  %arrayidx6 = getelementptr inbounds [100 x [100 x double]], ptr @HEKMAN, i64 0, i64 %indvars.iv34, i64 %indvars.iv, !intel-tbaa !7
  %0 = load  double, ptr %arrayidx6, align 8, !tbaa !7
  %cmp7 = fcmp fast ult double %0, %fneg
  br i1 %cmp7, label %if.end, label %if.then

if.then:                                          ; preds = %for.body3
  %arrayidx11 = getelementptr inbounds [100 x [100 x double]], ptr @G, i64 0, i64 %indvars.iv34, i64 %indvars.iv, !intel-tbaa !7
  store  double 0.000000e+00, ptr %arrayidx11, align 8, !tbaa !7
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 54
  br i1 %exitcond, label %for.inc12, label %for.body3, !llvm.loop !10

for.inc12:                                        ; preds = %if.end
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond36 = icmp eq i64 %indvars.iv.next35, 8
  br i1 %exitcond36, label %for.end14, label %for.cond1.preheader, !llvm.loop !12

for.end14:                                        ; preds = %for.inc12
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %FETCH.sroa.2.0..sroa_cast37)
  ret i32 0
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10000_d", !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA100_A100_d", !9, i64 0}
!9 = !{!"array@_ZTSA100_d", !4, i64 0}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.unroll.disable"}
!12 = distinct !{!12, !11}
