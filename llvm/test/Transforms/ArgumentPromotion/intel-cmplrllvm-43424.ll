; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

; Show that argument promotion does not happen for @"?add_one2@@YAHAEAH@Z"
; because it has vector-variants.

; CHECK: define internal noundef i32 @"?add_one2@@YAHAEAH@Z"(ptr

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30146"

@"?a@@3PAHA" = internal global [1023 x i32] zeroinitializer, align 16
@str = private unnamed_addr constant [7 x i8] c"passed\00", align 1
@str.1 = private unnamed_addr constant [7 x i8] c"failed\00", align 1

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define internal noundef i32 @"?add_one2@@YAHAEAH@Z"(ptr nocapture noundef nonnull readonly align 4 dereferenceable(4) %0) #0 {
  %2 = load i32, ptr %0, align 4, !tbaa !15
  %3 = add nsw i32 %2, 1
  ret i32 %3
}

; Function Attrs: mustprogress nofree norecurse nounwind uwtable
define dso_local noundef i32 @main() #1 {
  br label %1

1:                                                ; preds = %1, %0
  %2 = phi i64 [ 0, %0 ], [ %5, %1 ]
  %3 = getelementptr inbounds [1023 x i32], ptr @"?a@@3PAHA", i64 0, i64 %2, !intel-tbaa !19
  %4 = trunc i64 %2 to i32
  store i32 %4, ptr %3, align 4, !tbaa !19
  %5 = add nuw nsw i64 %2, 1
  %6 = icmp eq i64 %5, 1023
  br i1 %6, label %7, label %1, !llvm.loop !21

7:                                                ; preds = %7, %1
  %8 = phi ptr [ %11, %7 ], [ @"?a@@3PAHA", %1 ]
  %9 = phi i32 [ %12, %7 ], [ 0, %1 ]
  %10 = tail call noundef i32 @"?add_one2@@YAHAEAH@Z"(ptr noundef nonnull align 4 dereferenceable(4) %8)
  store i32 %10, ptr %8, align 4, !tbaa !15
  %11 = getelementptr inbounds i32, ptr %8, i64 1
  %12 = add nuw nsw i32 %9, 1
  %13 = icmp eq i32 %12, 1023
  br i1 %13, label %16, label %7, !llvm.loop !24

14:                                               ; preds = %16
  %15 = icmp eq i64 %20, 1023
  br i1 %15, label %25, label %16, !llvm.loop !25

16:                                               ; preds = %14, %7
  %17 = phi i64 [ %20, %14 ], [ 0, %7 ]
  %18 = getelementptr inbounds [1023 x i32], ptr @"?a@@3PAHA", i64 0, i64 %17, !intel-tbaa !19
  %19 = load i32, ptr %18, align 4, !tbaa !19
  %20 = add nuw nsw i64 %17, 1
  %21 = zext i32 %19 to i64
  %22 = icmp eq i64 %20, %21
  br i1 %22, label %14, label %23, !llvm.loop !25

23:                                               ; preds = %16
  %24 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.1)
  br label %27

25:                                               ; preds = %14
  %26 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %27

27:                                               ; preds = %25, %23
  %28 = phi i32 [ 1, %23 ], [ 0, %25 ]
  ret i32 %28
}

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #2

attributes #0 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(argmem: read) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN4R4_?add_one2@@YAHAEAH@Z,_ZGVbM4R4_?add_one2@@YAHAEAH@Z,_ZGVcN8R4_?add_one2@@YAHAEAH@Z,_ZGVcM8R4_?add_one2@@YAHAEAH@Z,_ZGVdN8R4_?add_one2@@YAHAEAH@Z,_ZGVdM8R4_?add_one2@@YAHAEAH@Z,_ZGVeN16R4_?add_one2@@YAHAEAH@Z,_ZGVeM16R4_?add_one2@@YAHAEAH@Z" }
attributes #1 = { mustprogress nofree norecurse nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nofree nounwind }

!llvm.linker.options = !{!0, !1, !2, !3, !4, !5, !6}
!llvm.ident = !{!7}
!llvm.module.flags = !{!8, !9, !10, !11, !12, !13, !14}

!0 = !{!"/DEFAULTLIB:libcmt.lib"}
!1 = !{!"/DEFAULTLIB:libircmt.lib"}
!2 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!3 = !{!"/DEFAULTLIB:libdecimal.lib"}
!4 = !{!"/DEFAULTLIB:libmmt.lib"}
!5 = !{!"/DEFAULTLIB:oldnames.lib"}
!6 = !{!"/FAILIFMISMATCH:\22_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\22"}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!8 = !{i32 1, !"wchar_size", i32 2}
!9 = !{i32 8, !"PIC Level", i32 2}
!10 = !{i32 7, !"uwtable", i32 2}
!11 = !{i32 1, !"MaxTLSAlign", i32 65536}
!12 = !{i32 1, !"ThinLTO", i32 0}
!13 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!14 = !{i32 1, !"LTOPostLink", i32 1}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C++ TBAA"}
!19 = !{!20, !16, i64 0}
!20 = !{!"array@?AY0DPP@H", !16, i64 0}
!21 = distinct !{!21, !22, !23}
!22 = !{!"llvm.loop.mustprogress"}
!23 = !{!"llvm.loop.vectorize.width", i32 1}
!24 = distinct !{!24, !22}
!25 = distinct !{!25, !22}
