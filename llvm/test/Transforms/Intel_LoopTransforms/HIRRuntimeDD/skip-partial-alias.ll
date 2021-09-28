; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -debug-only=hir-runtime-dd< %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-runtime-dd -S < %s 2>&1 | FileCheck %s
;
; In this case, HIRRuntimeDD multiversioning is not triggered, because %arg[0] and %tempcast[0].1 are partial alias
; based on alias analysis result.
; We skip multiversioning the loop if memrefs in different groups are must alias or partial alias.
;
;
; *** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<14>               + DO i1 = 0, 49, 1   <DO_LOOP> <nounroll>
;<2>                |   (i32*)(%arg)[0] = %a1;
;<4>                |   (%Tempcast)[0].1 = %a2;
;<5>                |   %l64 = (%arg)[0];
;<7>                |   (%tt)[0][%l64] = 5;
;<14>               + END LOOP
;<0>          END REGION
;
; CHECK: Runtime DD for loop [[LOOP:[0-9]+]]:
; CHECK: LOOPOPT_OPTREPORT: [RTDD] Loop [[LOOP]]: Loop considered non-profitable due to partial/must alias between groups
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test1 = type { i32, i32 }

@val1 = external dso_local local_unnamed_addr global i32, align 4
@val2 = external dso_local local_unnamed_addr global i32, align 4


; Function Attrs: nofree nosync nounwind uwtable
define dso_local void @foo(i32 %a1, i32 %a2) local_unnamed_addr #0 {
entry:
  %arg = alloca i64, align 8
  %st = alloca %struct.test1, align 8
  %tt = alloca [100 x i32], align 16
  %0 = bitcast [100 x i32]* %tt to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %0) #2
  %1 = load i32, i32* @val1, align 4, !tbaa !3
  %idxprom = sext i32 %1 to i64
  %Tempcast =  bitcast i64* %arg to %struct.test1*
  %arg.temp = bitcast i64* %arg to i32*
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %tt, i64 0, i64 %idxprom, !intel-tbaa !7
  %arrayidx.promoted = load i32, i32* %arrayidx, align 4, !tbaa !7
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond
  %add24.lcssa = phi i32 [ 0, %entry ]
  store i32 %add24.lcssa, i32* %arrayidx, align 4, !tbaa !7
  br label %for.body5

for.cond.cleanup4:                                ; preds = %for.body5
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %0) #2
  ret void

for.body5:                                        ; preds = %for.cond1.preheader, %for.body5
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body5 ]
  store i32 %a1, i32* %arg.temp
  %field2 = getelementptr %struct.test1, %struct.test1* %Tempcast, i32 0, i32 1
  store i32 %a2, i32* %field2
  %l64 = load i64, i64* %arg
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %tt, i32 0, i64 %l64
  store i32 5, i32* %arrayidx7, align 8, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp2 = icmp ult i64 %indvars.iv, 98
  br i1 %cmp2, label %for.body5, label %for.cond.cleanup4, !llvm.loop !12
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn mustprogress }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.4.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA100_i", !4, i64 0}
!9 = distinct !{!9, !10, !11}
!10 = !{!"llvm.loop.mustprogress"}
!11 = !{!"llvm.loop.unroll.disable"}
!12 = distinct !{!12, !10, !11}

