; RUN: opt -passes='early-cse<memssa>' -S %s | FileCheck %s
;
; Original test src:
;
; void foo() {
;   int X = 10;
; #pragma omp target firstprivate(X)
;   {}
; }
;
; Check that Early CSE copies opt-report metadata to the caller when removing
; dead function call. Outlined target region in this test is a dead call.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.ident_t = type { i32, i32, i32, i32, i8* }
%struct.__tgt_offload_entry = type { i8*, i8*, i64, i32, i32 }

@__omp_offloading_35_d7907b26__Z3foo_l3.region_id = weak constant i8 0
@.mapname = private unnamed_addr constant [26 x i8] c";X.val.zext;unknown;0;0;;\00", align 1
@.offload_sizes = private unnamed_addr constant [1 x i64] zeroinitializer
@.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 288]
@.offload_mapnames = private constant [1 x i8*] [i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.mapname, i32 0, i32 0)]
@.source.3.3 = private unnamed_addr constant [53 x i8] c";unknown;foo.DIR.OMP.TARGET.2.split.split.split;3;3;;"
@.kmpc_loc.3.3 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([53 x i8], [53 x i8]* @.source.3.3, i32 0, i32 0) }
@.source.3.1 = private unnamed_addr constant [18 x i8] c";unknown;foo;3;1;;"
@.source.3.3.1 = private unnamed_addr constant [53 x i8] c";unknown;foo.DIR.OMP.TARGET.2.split.split.split;3;3;;"
@.kmpc_loc.3.3.2 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([53 x i8], [53 x i8]* @.source.3.3.1, i32 0, i32 0) }
@.omp_offloading.entry_name = internal target_declare unnamed_addr constant [39 x i8] c"__omp_offloading_35_d7907b26__Z3foo_l3\00"
@.omp_offloading.entry.__omp_offloading_35_d7907b26__Z3foo_l3 = weak target_declare constant %struct.__tgt_offload_entry { i8* @__omp_offloading_35_d7907b26__Z3foo_l3.region_id, i8* getelementptr inbounds ([39 x i8], [39 x i8]* @.omp_offloading.entry_name, i32 0, i32 0), i64 0, i32 0, i32 0 }, section "omp_offloading_entries"

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 !dbg !8 {
; CHECK-LABEL: define dso_local void @foo(
; CHECK-SAME: !intel.optreport.rootnode !{{[0-9]+}}
DIR.OMP.END.TARGET.4:
  %.offload_baseptrs = alloca [1 x i8*], align 8
  %.offload_ptrs = alloca [1 x i8*], align 8
  %X = alloca i32, align 4
  store i32 10, i32* %X, align 4, !dbg !10, !tbaa !11
  %0 = ptrtoint i32* %X to i8, !dbg !15
  %X.val = load i32, i32* %X, align 4, !dbg !15
  %X.val.zext = zext i32 %X.val to i64, !dbg !15
  %1 = inttoptr i64 %X.val.zext to i8*, !dbg !15
  %2 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0, !dbg !15
  store i8* %1, i8** %2, align 8, !dbg !15
  %3 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_ptrs, i32 0, i32 0, !dbg !15
  %4 = inttoptr i64 %X.val.zext to i8*, !dbg !15
  store i8* %4, i8** %3, align 8, !dbg !15
  %5 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0, !dbg !15
  %6 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_ptrs, i32 0, i32 0, !dbg !15
  %7 = call i32 @omp_get_default_device()
  %8 = zext i32 %7 to i64, !dbg !15
  call void @__tgt_push_code_location(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.source.3.1, i32 0, i32 0), i8* bitcast (i32 (%struct.ident_t*, i64, i8*, i32, i8**, i8**, i64*, i64*, i8**, i8**)* @__tgt_target_mapper to i8*)), !dbg !15
  %9 = call i32 @__tgt_target_mapper(%struct.ident_t* @.kmpc_loc.3.3, i64 %8, i8* @__omp_offloading_35_d7907b26__Z3foo_l3.region_id, i32 1, i8** %5, i8** %6, i64* getelementptr inbounds ([1 x i64], [1 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([1 x i64], [1 x i64]* @.offload_maptypes, i32 0, i32 0), i8** getelementptr inbounds ([1 x i8*], [1 x i8*]* @.offload_mapnames, i32 0, i32 0), i8** null), !dbg !15
  %10 = icmp ne i32 %9, 0, !dbg !15
  br i1 %10, label %omp_offload.failed, label %DIR.OMP.END.TARGET.5, !dbg !15

omp_offload.failed:                               ; preds = %DIR.OMP.END.TARGET.4
  call void @__kmpc_push_num_teams(%struct.ident_t* @.kmpc_loc.3.3.2, i32 0, i32 0, i32 1)
  call void @__omp_offloading_35_d7907b26__Z3foo_l3(i64 %X.val.zext), !dbg !15
  br label %DIR.OMP.END.TARGET.5, !dbg !15

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.4, %omp_offload.failed
  ret void, !dbg !16
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn
define internal void @__omp_offloading_35_d7907b26__Z3foo_l3(i64 %X.val.zext) #1 !dbg !17 !intel.optreport.rootnode !18 {
DIR.OMP.END.TARGET.5.exitStub:
  ret void
}

declare i32 @omp_get_default_device()

; Function Attrs: nounwind
declare i32 @__tgt_target_mapper(%struct.ident_t*, i64, i8*, i32, i8**, i8**, i64*, i64*, i8**, i8**) #2

declare void @__tgt_push_code_location(i8*, i8*)

; Function Attrs: nounwind
declare void @__kmpc_push_num_teams(%struct.ident_t*, i32, i32, i32) #2

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "target.declare"="true" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}
!nvvm.annotations = !{}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "/nfs/sc/proj/icl/devshare/sdmitrie/omp")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"openmp", i32 50}
!6 = !{i32 7, !"uwtable", i32 1}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !2)
!10 = !DILocation(line: 2, column: 7, scope: !8)
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
!15 = !DILocation(line: 3, column: 1, scope: !8)
!16 = !DILocation(line: 5, column: 1, scope: !8)
!17 = distinct !DISubprogram(name: "foo.DIR.OMP.TARGET.2.split.split.split", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!18 = distinct !{!"intel.optreport.rootnode", !19}
!19 = distinct !{!"intel.optreport", !20}
!20 = !{!"intel.optreport.first_child", !21}
!21 = distinct !{!"intel.optreport.rootnode", !22}
!22 = distinct !{!"intel.optreport", !23, !25, !26}
!23 = !{!"intel.optreport.debug_location", !24}
!24 = !DILocation(line: 3, column: 1, scope: !17)
!25 = !{!"intel.optreport.title", !"OMP TARGET"}
!26 = !{!"intel.optreport.remarks", !27}
!27 = !{!"intel.optreport.remark", i32 0, !"FIRSTPRIVATE clause for variable 'X' is redundant"}
