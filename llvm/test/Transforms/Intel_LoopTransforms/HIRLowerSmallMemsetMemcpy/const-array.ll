; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy,print<hir>" -hir-create-function-level-region -disable-output < %s 2>&1 | FileCheck %s

; The test checks that memcpy intrinsic got recognized by HIR Lower Small Memset/Memcpy pass
; and transformed to a loop. Check that constant array is proccessed correctly.

;HIR:
;            BEGIN REGION { }
;                  @llvm.memcpy.p0.p0.i64(&((%SSS)[0].0.0[0]),  &((@__const.main.SSS)[0].0.0[0]),  10,  0);
;                  %1 = (%SSS)[0].0.0[%n];
;                  ret %1;
;            END REGION

;HIR after transformation:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   (%SSS)[0].0.0[i1] = (@__const.main.SSS)[0].0.0[i1];
; CHECK:           + END LOOP
;        
;                  %1 = (%SSS)[0].0.0[%n];
;                  ret %1;
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.s1 = type { %struct.s2 }
%struct.s2 = type { [10 x i8] }

@__const.main.SSS = private unnamed_addr constant %struct.s1 { %struct.s2 { [10 x i8] c"\00\01\02\03\04\05\06\07\08\09" } }, align 1

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef i32 @main(i32 noundef %n) local_unnamed_addr #0 {
entry:
  %SSS = alloca %struct.s1, align 1
  br label %bb

bb:                                          ; preds = entry%
  %0 = getelementptr inbounds %struct.s1, ptr %SSS, i64 0, i32 0, i32 0, i64 0
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 1 dereferenceable(10) %0, ptr noundef nonnull align 1 dereferenceable(10) @__const.main.SSS, i64 10, i1 false)
  %idxprom5 = zext i32 %n to i64
  %arrayidx6 = getelementptr inbounds %struct.s1, ptr %SSS, i64 0, i32 0, i32 0, i64 %idxprom5, !intel-tbaa !3
  %1 = load i8, ptr %arrayidx6, align 1, !tbaa !3
  %conv7 = sext i8 %1 to i32
  ret i32 %conv7
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !7, i64 0}
!4 = !{!"struct@_ZTS2s1", !5, i64 0}
!5 = !{!"struct@_ZTS2s2", !6, i64 0}
!6 = !{!"array@_ZTSA10_c", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
