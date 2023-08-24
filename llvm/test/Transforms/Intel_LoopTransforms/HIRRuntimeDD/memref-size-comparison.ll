; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; In this case, we access i32 memory in store and i64 memory in load.
;
; Store ref (%arg)[0] (smaller sized ref) should be ordered before
; load ref (%arg)[0] (larger sized ref).
;
; This lets us test the correct address range for the group.
;
;*** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<14>               + DO i1 = 0, 49, 1   <DO_LOOP> <nounroll>
;<2>                |   (%arg)[0] = %a1;
;<4>                |   (%Tempcast)[0].1 = %a2;
;<5>                |   %l64 = (%arg)[0];
;<7>                |   (%tt)[0][%l64] = 5;
;<14>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: foo
;
; CHECK:     BEGIN REGION { }
; CHECK:            %mv.upper.base = &((i32*)(%arg)[0]);
; CHECK:            %mv.test = &((%mv.upper.base)[1]) >=u &((%Tempcast)[0].1);
; CHECK:            %mv.test3 = &((%Tempcast)[0].1) >=u &((%arg)[0]);
; CHECK:            %mv.and = %mv.test  &  %mv.test3;
; CHECK:            if (%mv.and == 0)
; CHECK:            {
; CHECK:               + DO i1 = 0, 49, 1   <DO_LOOP>  <MVTag: 14> <nounroll>
; CHECK:               |   (%arg)[0] = %a1;
; CHECK:               |   (%Tempcast)[0].1 = %a2;
; CHECK:               |   %l64 = (%arg)[0];
; CHECK:               |   (%tt)[0][%l64] = 5;
; CHECK:               + END LOOP
; CHECK:            }
; CHECK:            else
; CHECK:            {
; CHECK:               + DO i1 = 0, 49, 1   <DO_LOOP>  <MVTag: 14> <nounroll> <novectorize>
; CHECK:               |   (%arg)[0] = %a1;
; CHECK:               |   (%Tempcast)[0].1 = %a2;
; CHECK:               |   %l64 = (%arg)[0];
; CHECK:               |   (%tt)[0][%l64] = 5;
; CHECK:               + END LOOP
; CHECK:            }
; CHECK:      END REGION
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test1 = type { i32, i32 }

@val1 = external dso_local local_unnamed_addr global i32, align 4
@val2 = external dso_local local_unnamed_addr global i32, align 4


; Function Attrs: nofree nosync nounwind uwtable
define dso_local void @foo(ptr %arg, ptr %Tempcast, i32 %a1, i32 %a2) local_unnamed_addr #0 {
entry:
  %st = alloca %struct.test1, align 8
  %tt = alloca [100 x i32], align 16
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %tt) #2
  %0 = load i32, ptr @val1, align 4, !tbaa !3
  %idxprom = sext i32 %0 to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr %tt, i64 0, i64 %idxprom, !intel-tbaa !7
  %arrayidx.promoted = load i32, ptr %arrayidx, align 4, !tbaa !7
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond
  %add24.lcssa = phi i32 [ 0, %entry ]
  store i32 %add24.lcssa, ptr %arrayidx, align 4, !tbaa !7
  br label %for.body5

for.cond.cleanup4:                                ; preds = %for.body5
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %tt) #2
  ret void

for.body5:                                        ; preds = %for.cond1.preheader, %for.body5
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body5 ]
  store i32 %a1, ptr %arg
  %field2 = getelementptr %struct.test1, ptr %Tempcast, i32 0, i32 1
  store i32 %a2, ptr %field2
  %l64 = load i64, ptr %arg
  %arrayidx7 = getelementptr inbounds [100 x i32], ptr %tt, i32 0, i64 %l64
  store i32 5, ptr %arrayidx7, align 8, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp2 = icmp ult i64 %indvars.iv, 98
  br i1 %cmp2, label %for.body5, label %for.cond.cleanup4, !llvm.loop !12
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

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

