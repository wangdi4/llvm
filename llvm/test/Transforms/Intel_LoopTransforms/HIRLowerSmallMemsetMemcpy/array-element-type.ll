; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; The test checks that memcpy intrinsic got recognized by HIR Lower Small Memset/Memcpy pass
; and transformed to a loop. The types of the memset arguments are different. Src argument
; of the memcpy is a pointer to array.

; HIR:
;            BEGIN REGION { }
;                  + DO i1 = 0, 9, 1   <DO_LOOP>
;                  |   @llvm.memcpy.p0.p0.i64(&((i8*)(%SSS)[0]),  &((i8*)(@__const.main.a)[0]),  16,  0);
;                  + END LOOP
;            END REGION

; HIR after optimization:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:           |   |   (%SSS)[0].0.0[i2] = (@__const.main.a)[0][i2];
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.s1 = type { %struct.s2 }
%struct.s2 = type { [100 x i32] }

@__const.main.a = private unnamed_addr constant [10 x i32] [i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57], align 16

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef i32 @main(i32 noundef %n) local_unnamed_addr #0 {
entry:
  %SSS = alloca %struct.s1, align 4
  %0 = bitcast ptr %SSS to ptr
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %0) #3
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.04 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 4 dereferenceable(16) %0, ptr noundef nonnull align 16 dereferenceable(16) @__const.main.a, i64 16, i1 false)
  %inc = add nuw nsw i32 %i.04, 1
  %exitcond.not = icmp eq i32 %inc, 10
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !3

for.end:                                          ; preds = %for.body
  %idxprom = sext i32 %n to i64
  %arrayidx = getelementptr inbounds %struct.s1, ptr %SSS, i64 0, i32 0, i32 0, i64 %idxprom, !intel-tbaa !5
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !5
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %0) #3
  ret i32 %1
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!6, !9, i64 0}
!6 = !{!"struct@_ZTS2s1", !7, i64 0}
!7 = !{!"struct@_ZTS2s2", !8, i64 0}
!8 = !{!"array@_ZTSA100_i", !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
