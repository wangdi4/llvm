; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; The test checks that memset intrinsic got recognized by HIR Lower Small Memset/Memcpy pass
; and transformed to a loop. Also check that loop is correctly formed on level 2.

; HIR:
;            BEGIN REGION { }
;                  + DO i1 = 0, 9, 1   <DO_LOOP>
;                  |   @llvm.memset.p0.i64(&((%a)[0][2][0]),  97,  6,  0);
;                  + END LOOP
;            END REGION

; HIR after transformation:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 5, 1   <DO_LOOP>
; CHECK:           |   |   (%a)[0][2][i2] = 97;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__const._Z3fooi.a = private unnamed_addr constant [3 x [11 x i8]] [[11 x i8] c"1234567890\00", [11 x i8] c"1234567890\00", [11 x i8] c"1234567890\00"], align 16

; Function Attrs: mustprogress nofree nosync nounwind readnone willreturn uwtable
define dso_local noundef i32 @_Z3fooi(i32 noundef %n) local_unnamed_addr #0 {
entry:
  %a = alloca [3 x [11 x i8]], align 16
  %0 = getelementptr inbounds [3 x [11 x i8]], ptr %a, i64 0, i64 0, i64 0
  call void @llvm.lifetime.start.p0(i64 33, ptr nonnull %0) #4
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(33) %0, ptr noundef nonnull align 16 dereferenceable(33) @__const._Z3fooi.a, i64 33, i1 false)
  %arraydecay = getelementptr inbounds [3 x [11 x i8]], ptr %a, i64 0, i64 2, i64 0
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.07 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 2 dereferenceable(6) %arraydecay, i8 97, i64 6, i1 false)
  %inc = add nuw nsw i32 %i.07, 1
  %exitcond.not = icmp eq i32 %inc, 10
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !3

for.end:                                          ; preds = %for.body
  %idxprom = sext i32 %n to i64
  %arrayidx3 = getelementptr inbounds [3 x [11 x i8]], ptr %a, i64 0, i64 %idxprom, i64 %idxprom, !intel-tbaa !5
  %1 = load i8, ptr %arrayidx3, align 1, !tbaa !5
  %conv = sext i8 %1 to i32
  call void @llvm.lifetime.end.p0(i64 33, ptr nonnull %0) #4
  ret i32 %conv
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly mustprogress nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: argmemonly mustprogress nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { mustprogress nofree nosync nounwind readnone willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #2 = { argmemonly mustprogress nocallback nofree nounwind willreturn }
attributes #3 = { argmemonly mustprogress nocallback nofree nounwind willreturn writeonly }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!6, !8, i64 0}
!6 = !{!"array@_ZTSA3_A11_c", !7, i64 0}
!7 = !{!"array@_ZTSA11_c", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C++ TBAA"}
