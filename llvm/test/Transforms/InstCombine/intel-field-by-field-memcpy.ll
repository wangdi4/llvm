; CMPLRLLVM-25540: This test verifies that field-by-field memcpy lowering
; is triggered in instcombine pass.

; RUN: opt -instcombine %s -S | FileCheck %s
; RUN: opt -passes=instcombine %s -S | FileCheck %s

; check memcpy is field-by-field lowered.
; CHECK:  %q9 = load volatile %struct.Pixel**, %struct.Pixel*** %q.addr, align 8
; CHECK:  %p.addr11 = load volatile %struct.Pixel**, %struct.Pixel*** %p.addr.addr, align 8
; CHECK:  i12 = load %struct.Pixel*, %struct.Pixel** %q9, align 8, !tbaa !5
; CHECK:  %i13 = load %struct.Pixel*, %struct.Pixel** %p.addr11, align 8, !tbaa !5
; CHECK:  [[SGEP0:%.*]] = getelementptr inbounds %struct.Pixel, %struct.Pixel* %i13, i64 0, i32 0
; CHECK:  [[DGEP0:%.*]] = getelementptr inbounds %struct.Pixel, %struct.Pixel* %i12, i64 0, i32 0
; CHECK:  [[LD0:%.*]] = load i16, i16* [[SGEP0]], align 2, !tbaa !9
; CHECK:  store i16 [[LD0]], i16* [[DGEP0]], align 2, !tbaa !9
; CHECK:  [[SGEP1:%.*]] = getelementptr inbounds %struct.Pixel, %struct.Pixel* %i13, i64 0, i32 1
; CHECK:  [[DGEP1:%.*]] = getelementptr inbounds %struct.Pixel, %struct.Pixel* %i12, i64 0, i32 1
; CHECK:  [[LD1:%.*]] = load i16, i16* [[SGEP1]], align 2, !tbaa !9
; CHECK:  store i16 [[LD1]], i16* [[DGEP1]], align 2, !tbaa !9
; CHECK:  [[SGEP2:%.*]] = getelementptr inbounds %struct.Pixel, %struct.Pixel* %i13, i64 0, i32 2
; CHECK:  [[DGEP2:%.*]] = getelementptr inbounds %struct.Pixel, %struct.Pixel* %i12, i64 0, i32 2
; CHECK:  [[LD2:%.*]] = load i16, i16* [[SGEP2]], align 2, !tbaa !9
; CHECK:  store i16 [[LD2]], i16* [[DGEP2]], align 2, !tbaa !9
; CHECK:  [[SGEP3:%.*]] = getelementptr inbounds %struct.Pixel, %struct.Pixel* %i13, i64 0, i32 3
; CHECK:  [[DGEP3:%.*]] = getelementptr inbounds %struct.Pixel, %struct.Pixel* %i12, i64 0, i32 3
; CHECK:  [[LD3:%.*]] = load i16, i16* [[SGEP3]], align 2, !tbaa !9
; CHECK:  store i16 [[LD3]], i16* [[DGEP3]], align 2, !tbaa !9


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Pixel = type { i16, i16, i16, i16 }

define dso_local i32 @foo(%struct.Pixel* %p) local_unnamed_addr  {
entry:
  %p.addr = alloca %struct.Pixel*, align 8
  %q = alloca %struct.Pixel*, align 8
  store %struct.Pixel* %p, %struct.Pixel** %p.addr, align 8, !tbaa !5
  %q.addr = alloca %struct.Pixel**, align 8
  %p.addr.addr = alloca %struct.Pixel**, align 8
  store %struct.Pixel** %q, %struct.Pixel*** %q.addr, align 8
  store %struct.Pixel** %p.addr, %struct.Pixel*** %p.addr.addr, align 8
  %q9 = load volatile %struct.Pixel**, %struct.Pixel*** %q.addr, align 8
  %p.addr11 = load volatile %struct.Pixel**, %struct.Pixel*** %p.addr.addr, align 8
  %i12 = load %struct.Pixel*, %struct.Pixel** %q9, align 8, !tbaa !5
  %i13 = load %struct.Pixel*, %struct.Pixel** %p.addr11, align 8, !tbaa !5
  %ptridx = getelementptr inbounds %struct.Pixel, %struct.Pixel* %i13, i64 0
  %i15 = bitcast %struct.Pixel* %i12 to i8*
  %i16 = bitcast %struct.Pixel* %ptridx to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 2 %i15, i8* align 2 %i16, i64 8, i1 false), !tbaa.struct !11
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg)

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"pointer@_ZTSP5Pixel", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !7, i64 0}
!11 = !{i64 0, i64 2, !12, i64 2, i64 2, !12, i64 4, i64 2, !12, i64 6, i64 2, !12}
!12 = !{!13, !13, i64 0}
!13 = !{!"short", !7, i64 0}
!14 = !{!15, !13, i64 0}
!15 = !{!"struct@Pixel", !13, i64 0, !13, i64 2, !13, i64 4, !13, i64 6}
!16 = !{!15, !13, i64 2}
!17 = !{!15, !13, i64 4}
!18 = !{!15, !13, i64 6}
