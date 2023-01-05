; RUN: opt -passes=always-inline -S %s | FileCheck %s
;
; Original test src:
;
; void foo() {
;   int X = 10;
; #pragma omp target data map(to:X)
;   {
; #pragma omp target firstprivate(X)
;     {}
;   }
; }
;
; Check that inliner copies opt-report metadata to the caller when inlining a
; function. In this test outlined 'target data' region with which has attached
; opt-report metadata is inlined into a foo().

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.ident_t = type { i32, i32, i32, i32, i8* }
%struct.__tgt_offload_entry = type { i8*, i8*, i64, i32, i32 }

@.mapname.3 = private unnamed_addr constant [16 x i8] c";X;test.c;2;7;;\00", align 1
@__omp_offloading_35_d7907b29__Z3foo_l5.region_id = weak constant i8 0
@.mapname = private unnamed_addr constant [26 x i8] c";X.val.zext;unknown;0;0;;\00", align 1
@.offload_sizes = private unnamed_addr constant [1 x i64] zeroinitializer
@.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 288]
@.offload_mapnames = private constant [1 x i8*] [i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.mapname, i32 0, i32 0)]
@.source.5.5 = private unnamed_addr constant [55 x i8] c";unknown;foo.DIR.OMP.TARGET.4.split10.split.split;5;5;;"
@.kmpc_loc.5.5 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([55 x i8], [55 x i8]* @.source.5.5, i32 0, i32 0) }
@.source.5.1 = private unnamed_addr constant [18 x i8] c";unknown;foo;5;1;;"
@.source.5.5.1 = private unnamed_addr constant [55 x i8] c";unknown;foo.DIR.OMP.TARGET.4.split10.split.split;5;5;;"
@.kmpc_loc.5.5.2 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([55 x i8], [55 x i8]* @.source.5.5.1, i32 0, i32 0) }
@.offload_sizes.4 = private unnamed_addr constant [1 x i64] [i64 4]
@.offload_maptypes.5 = private unnamed_addr constant [1 x i64] [i64 1]
@.offload_mapnames.6 = private constant [1 x i8*] [i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.mapname.3, i32 0, i32 0)]
@.source.3.3 = private unnamed_addr constant [46 x i8] c";unknown;foo.DIR.OMP.TARGET.DATA.1.split;3;3;;"
@.kmpc_loc.3.3 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([46 x i8], [46 x i8]* @.source.3.3, i32 0, i32 0) }
@.source.3.1 = private unnamed_addr constant [18 x i8] c";unknown;foo;3;1;;"
@.source.3.3.7 = private unnamed_addr constant [46 x i8] c";unknown;foo.DIR.OMP.TARGET.DATA.1.split;3;3;;"
@.kmpc_loc.3.3.8 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([46 x i8], [46 x i8]* @.source.3.3.7, i32 0, i32 0) }
@.source.3.1.9 = private unnamed_addr constant [18 x i8] c";unknown;foo;3;1;;"
@.omp_offloading.entry_name = internal target_declare unnamed_addr constant [39 x i8] c"__omp_offloading_35_d7907b29__Z3foo_l5\00"
@.omp_offloading.entry.__omp_offloading_35_d7907b29__Z3foo_l5 = weak target_declare constant %struct.__tgt_offload_entry { i8* @__omp_offloading_35_d7907b29__Z3foo_l5.region_id, i8* getelementptr inbounds ([39 x i8], [39 x i8]* @.omp_offloading.entry_name, i32 0, i32 0), i64 0, i32 0, i32 0 }, section "omp_offloading_entries"

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 !dbg !8 {
; CHECK-LABEL: define dso_local void @foo(
; CHECK-SAME: !intel.optreport.rootnode !{{[0-9]+}}
DIR.OMP.TARGET.DATA.26:
  %.offload_baseptrs = alloca [1 x i8*], align 8
  %.offload_ptrs = alloca [1 x i8*], align 8
  %.offload_mappers = alloca [1 x i8*], align 8
  %.run_host_version = alloca i32, align 4
  %X = alloca i32, align 4
  store i32 10, i32* %X, align 4, !dbg !10, !tbaa !11
  %0 = bitcast i32* %X to i8*, !dbg !15
  %1 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0, !dbg !15
  store i8* %0, i8** %1, align 8, !dbg !15
  %2 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_ptrs, i32 0, i32 0, !dbg !15
  %3 = bitcast i32* %X to i8*, !dbg !15
  store i8* %3, i8** %2, align 8, !dbg !15
  %4 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_mappers, i32 0, i32 0, !dbg !15
  store i8* null, i8** %4, align 8, !dbg !15
  %5 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0, !dbg !15
  %6 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_ptrs, i32 0, i32 0, !dbg !15
  %7 = call i32 @omp_get_default_device()
  %8 = zext i32 %7 to i64, !dbg !15
  call void @__tgt_push_code_location(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.source.3.1, i32 0, i32 0), i8* bitcast (void (%struct.ident_t*, i64, i32, i8**, i8**, i64*, i64*, i8**, i8**)* @__tgt_target_data_begin_mapper to i8*)), !dbg !15
  call void @__tgt_target_data_begin_mapper(%struct.ident_t* @.kmpc_loc.3.3, i64 %8, i32 1, i8** %5, i8** %6, i64* getelementptr inbounds ([1 x i64], [1 x i64]* @.offload_sizes.4, i32 0, i32 0), i64* getelementptr inbounds ([1 x i64], [1 x i64]* @.offload_maptypes.5, i32 0, i32 0), i8** getelementptr inbounds ([1 x i8*], [1 x i8*]* @.offload_mapnames.6, i32 0, i32 0), i8** null), !dbg !15
  call void @foo.DIR.OMP.TARGET.DATA.1.split(i32* %X), !dbg !15
  %9 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0, !dbg !15
  %10 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_ptrs, i32 0, i32 0, !dbg !15
  %11 = call i32 @omp_get_default_device()
  %12 = zext i32 %11 to i64, !dbg !15
  call void @__tgt_push_code_location(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.source.3.1.9, i32 0, i32 0), i8* bitcast (void (%struct.ident_t*, i64, i32, i8**, i8**, i64*, i64*, i8**, i8**)* @__tgt_target_data_end_mapper to i8*)), !dbg !15
  call void @__tgt_target_data_end_mapper(%struct.ident_t* @.kmpc_loc.3.3.8, i64 %12, i32 1, i8** %9, i8** %10, i64* getelementptr inbounds ([1 x i64], [1 x i64]* @.offload_sizes.4, i32 0, i32 0), i64* getelementptr inbounds ([1 x i64], [1 x i64]* @.offload_maptypes.5, i32 0, i32 0), i8** getelementptr inbounds ([1 x i8*], [1 x i8*]* @.offload_mapnames.6, i32 0, i32 0), i8** null), !dbg !15
  ret void, !dbg !16
}

; Function Attrs: nounwind uwtable
define internal void @__omp_offloading_35_d7907b29__Z3foo_l5(i64 %X.val.zext) #3 !dbg !18 {
newFuncRoot:
  %X.fpriv = alloca i32, align 4, !dbg !19
  %X.val.zext.trunc = trunc i64 %X.val.zext to i32, !dbg !19
  store i32 %X.val.zext.trunc, i32* %X.fpriv, align 4, !dbg !19
  ret void
}

declare i32 @omp_get_default_device()

declare i32 @__tgt_target_mapper(%struct.ident_t*, i64, i8*, i32, i8**, i8**, i64*, i64*, i8**, i8**)

declare void @__tgt_push_code_location(i8*, i8*)

declare void @__kmpc_push_num_teams(%struct.ident_t*, i32, i32, i32)

; Function Attrs: alwaysinline nounwind uwtable
define internal void @foo.DIR.OMP.TARGET.DATA.1.split(i32* noalias %X) #4 !dbg !20 !intel.optreport.rootnode !21 {
newFuncRoot:
  %.offload_baseptrs = alloca [1 x i8*], align 8, !dbg !27
  %.offload_ptrs = alloca [1 x i8*], align 8, !dbg !27
  %.offload_mappers = alloca [1 x i8*], align 8, !dbg !27
  %.run_host_version = alloca i32, align 4, !dbg !27
  %promoted.clause.args = alloca i8, align 1, !dbg !27
  %0 = ptrtoint i32* %X to i8, !dbg !38
  store i8 %0, i8* %promoted.clause.args, align 1, !dbg !38
  %X.val = load i32, i32* %X, align 4, !dbg !38
  %X.val.zext = zext i32 %X.val to i64, !dbg !38
  %1 = inttoptr i64 %X.val.zext to i8*, !dbg !38
  %2 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0, !dbg !38
  store i8* %1, i8** %2, align 8, !dbg !38
  %3 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_ptrs, i32 0, i32 0, !dbg !38
  %4 = inttoptr i64 %X.val.zext to i8*, !dbg !38
  store i8* %4, i8** %3, align 8, !dbg !38
  %5 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_mappers, i32 0, i32 0, !dbg !38
  store i8* null, i8** %5, align 8, !dbg !38
  %6 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0, !dbg !38
  %7 = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_ptrs, i32 0, i32 0, !dbg !38
  %8 = call i32 @omp_get_default_device()
  %9 = zext i32 %8 to i64, !dbg !38
  call void @__tgt_push_code_location(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.source.5.1, i32 0, i32 0), i8* bitcast (i32 (%struct.ident_t*, i64, i8*, i32, i8**, i8**, i64*, i64*, i8**, i8**)* @__tgt_target_mapper to i8*)), !dbg !38
  %10 = call i32 @__tgt_target_mapper(%struct.ident_t* @.kmpc_loc.5.5, i64 %9, i8* @__omp_offloading_35_d7907b29__Z3foo_l5.region_id, i32 1, i8** %6, i8** %7, i64* getelementptr inbounds ([1 x i64], [1 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([1 x i64], [1 x i64]* @.offload_maptypes, i32 0, i32 0), i8** getelementptr inbounds ([1 x i8*], [1 x i8*]* @.offload_mapnames, i32 0, i32 0), i8** null), !dbg !38
  store i32 %10, i32* %.run_host_version, align 4, !dbg !38
  %11 = load i32, i32* %.run_host_version, align 4, !dbg !38
  %12 = icmp ne i32 %11, 0, !dbg !38
  br i1 %12, label %omp_offload.failed, label %DIR.OMP.END.TARGET.DATA.79, !dbg !38

omp_offload.failed:                               ; preds = %newFuncRoot
  call void @__kmpc_push_num_teams(%struct.ident_t* @.kmpc_loc.5.5.2, i32 0, i32 0, i32 1)
  call void @__omp_offloading_35_d7907b29__Z3foo_l5(i64 %X.val.zext), !dbg !38
  br label %DIR.OMP.END.TARGET.DATA.79, !dbg !38

DIR.OMP.END.TARGET.DATA.79:                       ; preds = %omp_offload.failed, %newFuncRoot
  ret void
}

declare void @__tgt_target_data_begin_mapper(%struct.ident_t*, i64, i32, i8**, i8**, i64*, i64*, i8**, i8**)

declare void @__tgt_target_data_end_mapper(%struct.ident_t*, i64, i32, i8**, i8**, i64*, i64*, i8**, i8**)

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "target.declare"="true" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { alwaysinline nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "target.declare"="true" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

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
!16 = !DILocation(line: 8, column: 1, scope: !8)
!17 = distinct !DISubprogram(linkageName: ".omp_offloading.requires_reg", scope: !1, file: !1, type: !9, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!18 = distinct !DISubprogram(name: "foo.DIR.OMP.TARGET.4.split10.split.split", scope: !1, file: !1, line: 5, type: !9, scopeLine: 5, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!19 = !DILocation(line: 5, column: 1, scope: !18)
!20 = distinct !DISubprogram(name: "foo.DIR.OMP.TARGET.DATA.1.split", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!21 = distinct !{!"intel.optreport.rootnode", !22}
!22 = distinct !{!"intel.optreport", !23}
!23 = !{!"intel.optreport.first_child", !24}
!24 = distinct !{!"intel.optreport.rootnode", !25}
!25 = distinct !{!"intel.optreport", !26, !28, !29}
!26 = !{!"intel.optreport.debug_location", !27}
!27 = !DILocation(line: 3, column: 1, scope: !20)
!28 = !{!"intel.optreport.title", !"OMP TARGET DATA"}
!29 = !{!"intel.optreport.first_child", !30}
!30 = distinct !{!"intel.optreport.rootnode", !31}
!31 = distinct !{!"intel.optreport", !32, !35, !36}
!32 = !{!"intel.optreport.debug_location", !33}
!33 = !DILocation(line: 5, column: 1, scope: !34)
!34 = distinct !DISubprogram(name: "foo.DIR.OMP.TARGET.4.split10.split.split", scope: !1, file: !1, line: 5, type: !9, scopeLine: 5, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!35 = !{!"intel.optreport.title", !"OMP TARGET"}
!36 = !{!"intel.optreport.remarks", !37}
!37 = !{!"intel.optreport.remark", i32 0, !"FIRSTPRIVATE clause for variable 'X' is redundant"}
!38 = !DILocation(line: 5, column: 1, scope: !20)
