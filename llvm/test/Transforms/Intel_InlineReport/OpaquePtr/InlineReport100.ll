; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xf859 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-INLREP
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xf8d8 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xf8d8 -S | opt -passes='inlinereportemitter' -inline-report=0xf8d8 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD-INLREP

; This tests the setting for the inline report with -qopt-report=3

; CHECK-INLREP: Begin Inlining Report
; CHECK-MD-INLREP: Begin Inlining Report{{.*}}(via metadata)
; CHECK: Option Values:
; CHECK: inline-threshold:
; CHECK: inlinehint-threshold:
; CHECK: inlinecold-threshold:
; CHECK:  inlineoptsize-threshold:
; CHECK: COMPILE FUNC: foo
; CHECK: COMPILE FUNC: main
; CHECK: INLINE: foo <stdin> (15,3) ({{.*}}<={{.*}})
; CHECK: EXTERN: fprintf <stdin> (16,3)
; CHECK-INLREP: End Inlining Report
; CHECK-MD-INLREP: End Inlining Report{{.*}}(via metadata)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@acox = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@bcox = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [7 x i8] c"%d %d\0A\00", align 1

; Function Attrs: nofree norecurse nosync nounwind writeonly uwtable
define dso_local void @foo() local_unnamed_addr #0 !dbg !8 {
entry:
  br label %for.cond1.preheader, !dbg !10

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv34 = phi i64 [ 0, %entry ], [ %indvars.iv.next35, %for.cond.cleanup3 ]
  %i = shl nuw i64 %indvars.iv34, 1, !dbg !11
  br label %for.body4, !dbg !12

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void, !dbg !13

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1, !dbg !14
  %exitcond37.not = icmp eq i64 %indvars.iv.next35, 100, !dbg !15
  br i1 %exitcond37.not, label %for.cond.cleanup, label %for.cond1.preheader, !dbg !10, !llvm.loop !16

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %i1 = mul nuw nsw i64 %indvars.iv, 3, !dbg !19
  %i2 = add nuw nsw i64 %i1, %i, !dbg !20
  %arrayidx7 = getelementptr inbounds [100 x [100 x i32]], ptr @acox, i64 0, i64 %indvars.iv34, i64 %indvars.iv, !dbg !21, !intel-tbaa !22
  %i3 = trunc i64 %i2 to i32, !dbg !28
  store i32 %i3, ptr %arrayidx7, align 4, !dbg !28, !tbaa !22
  %i4 = sub nsw i64 %i, %i1, !dbg !29
  %arrayidx13 = getelementptr inbounds [100 x [100 x i32]], ptr @bcox, i64 0, i64 %indvars.iv34, i64 %indvars.iv, !dbg !30, !intel-tbaa !22
  %i5 = trunc i64 %i4 to i32, !dbg !31
  store i32 %i5, ptr %arrayidx13, align 4, !dbg !31, !tbaa !22
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !32
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100, !dbg !33
  br i1 %exitcond.not, label %for.cond.cleanup3, label %for.body4, !dbg !12, !llvm.loop !34
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 !dbg !35 {
entry:
  call void @foo(), !dbg !36
  %i = load ptr, ptr @stderr, align 8, !dbg !37, !tbaa !38
  %i1 = load i32, ptr getelementptr inbounds ([100 x [100 x i32]], ptr @acox, i64 0, i64 1, i64 2), align 8, !dbg !40, !tbaa !22
  %i2 = load i32, ptr getelementptr inbounds ([100 x [100 x i32]], ptr @bcox, i64 0, i64 3, i64 4), align 16, !dbg !41, !tbaa !22
  %call = call i32 (ptr, ptr, ...) @fprintf(ptr %i, ptr @.str, i32 %i1, i32 %i2) #3, !dbg !42
  ret i32 0, !dbg !43
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #2

attributes #0 = { nofree norecurse nosync nounwind writeonly uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { cold }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "sm.c", directory: "/iusers/rcox2/rgOR")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"uwtable", i32 1}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 6, type: !9, scopeLine: 6, flags: DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !2)
!10 = !DILocation(line: 7, column: 3, scope: !8)
!11 = !DILocation(line: 9, column: 22, scope: !8)
!12 = !DILocation(line: 8, column: 5, scope: !8)
!13 = !DILocation(line: 12, column: 1, scope: !8)
!14 = !DILocation(line: 7, column: 34, scope: !8)
!15 = !DILocation(line: 7, column: 26, scope: !8)
!16 = distinct !{!16, !10, !17, !18}
!17 = !DILocation(line: 11, column: 5, scope: !8)
!18 = !{!"llvm.loop.mustprogress"}
!19 = !DILocation(line: 9, column: 30, scope: !8)
!20 = !DILocation(line: 9, column: 26, scope: !8)
!21 = !DILocation(line: 9, column: 7, scope: !8)
!22 = !{!23, !25, i64 0}
!23 = !{!"array@_ZTSA100_A100_i", !24, i64 0}
!24 = !{!"array@_ZTSA100_i", !25, i64 0}
!25 = !{!"int", !26, i64 0}
!26 = !{!"omnipotent char", !27, i64 0}
!27 = !{!"Simple C/C++ TBAA"}
!28 = !DILocation(line: 9, column: 18, scope: !8)
!29 = !DILocation(line: 10, column: 26, scope: !8)
!30 = !DILocation(line: 10, column: 7, scope: !8)
!31 = !DILocation(line: 10, column: 18, scope: !8)
!32 = !DILocation(line: 8, column: 36, scope: !8)
!33 = !DILocation(line: 8, column: 28, scope: !8)
!34 = distinct !{!34, !12, !17, !18}
!35 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 14, type: !9, scopeLine: 14, flags: DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!36 = !DILocation(line: 15, column: 3, scope: !35)
!37 = !DILocation(line: 16, column: 11, scope: !35)
!38 = !{!39, !39, i64 0}
!39 = !{!"pointer@_ZTSP8_IO_FILE", !26, i64 0}
!40 = !DILocation(line: 16, column: 30, scope: !35)
!41 = !DILocation(line: 16, column: 42, scope: !35)
!42 = !DILocation(line: 16, column: 3, scope: !35)
!43 = !DILocation(line: 17, column: 3, scope: !35)
