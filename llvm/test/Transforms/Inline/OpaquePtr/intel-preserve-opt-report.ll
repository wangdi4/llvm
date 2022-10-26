; RUN: opt -opaque-pointers -passes=always-inline -S %s | FileCheck %s
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

%struct.ident_t = type { i32, i32, i32, i32, ptr }
%struct.__tgt_offload_entry = type { ptr, ptr, i64, i32, i32 }

@.mapname.3 = private unnamed_addr constant [16 x i8] c";X;test.c;2;7;;\00", align 1
@__omp_offloading_35_d7907b29__Z3foo_l5.region_id = weak constant i8 0
@.mapname = private unnamed_addr constant [26 x i8] c";X.val.zext;unknown;0;0;;\00", align 1
@.offload_sizes = private unnamed_addr constant [1 x i64] zeroinitializer
@.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 288]
@.offload_mapnames = private constant [1 x ptr] [ptr @.mapname]
@.source.5.5 = private unnamed_addr constant [55 x i8] c";unknown;foo.DIR.OMP.TARGET.4.split10.split.split;5;5;;"
@.kmpc_loc.5.5 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.5.5 }
@.source.5.1 = private unnamed_addr constant [18 x i8] c";unknown;foo;5;1;;"
@.source.5.5.1 = private unnamed_addr constant [55 x i8] c";unknown;foo.DIR.OMP.TARGET.4.split10.split.split;5;5;;"
@.kmpc_loc.5.5.2 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.5.5.1 }
@.offload_sizes.4 = private unnamed_addr constant [1 x i64] [i64 4]
@.offload_maptypes.5 = private unnamed_addr constant [1 x i64] [i64 1]
@.offload_mapnames.6 = private constant [1 x ptr] [ptr @.mapname.3]
@.source.3.3 = private unnamed_addr constant [46 x i8] c";unknown;foo.DIR.OMP.TARGET.DATA.1.split;3;3;;"
@.kmpc_loc.3.3 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.3.3 }
@.source.3.1 = private unnamed_addr constant [18 x i8] c";unknown;foo;3;1;;"
@.source.3.3.7 = private unnamed_addr constant [46 x i8] c";unknown;foo.DIR.OMP.TARGET.DATA.1.split;3;3;;"
@.kmpc_loc.3.3.8 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.3.3.7 }
@.source.3.1.9 = private unnamed_addr constant [18 x i8] c";unknown;foo;3;1;;"
@.omp_offloading.entry_name = internal target_declare unnamed_addr constant [39 x i8] c"__omp_offloading_35_d7907b29__Z3foo_l5\00"
@.omp_offloading.entry.__omp_offloading_35_d7907b29__Z3foo_l5 = weak target_declare constant %struct.__tgt_offload_entry { ptr @__omp_offloading_35_d7907b29__Z3foo_l5.region_id, ptr @.omp_offloading.entry_name, i64 0, i32 0, i32 0 }, section "omp_offloading_entries"

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 !dbg !8 {
; CHECK-LABEL: define dso_local void @foo(
; CHECK-SAME: !intel.optreport.rootnode !{{[0-9]+}}
DIR.OMP.TARGET.DATA.26:
  %.offload_baseptrs = alloca [1 x ptr], align 8
  %.offload_ptrs = alloca [1 x ptr], align 8
  %.offload_mappers = alloca [1 x ptr], align 8
  %.run_host_version = alloca i32, align 4
  %X = alloca i32, align 4
  store i32 10, ptr %X, align 4, !dbg !10, !tbaa !11
  %i1 = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs, i32 0, i32 0, !dbg !15
  store ptr %X, ptr %i1, align 8, !dbg !15
  %i2 = getelementptr inbounds [1 x ptr], ptr %.offload_ptrs, i32 0, i32 0, !dbg !15
  store ptr %X, ptr %i2, align 8, !dbg !15
  %i4 = getelementptr inbounds [1 x ptr], ptr %.offload_mappers, i32 0, i32 0, !dbg !15
  store ptr null, ptr %i4, align 8, !dbg !15
  %i5 = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs, i32 0, i32 0, !dbg !15
  %i6 = getelementptr inbounds [1 x ptr], ptr %.offload_ptrs, i32 0, i32 0, !dbg !15
  %i7 = call i32 @omp_get_default_device()
  %i8 = zext i32 %i7 to i64, !dbg !15
  call void @__tgt_push_code_location(ptr @.source.3.1, ptr @__tgt_target_data_begin_mapper), !dbg !15
  call void @__tgt_target_data_begin_mapper(ptr @.kmpc_loc.3.3, i64 %i8, i32 1, ptr %i5, ptr %i6, ptr @.offload_sizes.4, ptr @.offload_maptypes.5, ptr @.offload_mapnames.6, ptr null), !dbg !15
  call void @foo.DIR.OMP.TARGET.DATA.1.split(ptr %X), !dbg !15
  %i9 = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs, i32 0, i32 0, !dbg !15
  %i10 = getelementptr inbounds [1 x ptr], ptr %.offload_ptrs, i32 0, i32 0, !dbg !15
  %i11 = call i32 @omp_get_default_device()
  %i12 = zext i32 %i11 to i64, !dbg !15
  call void @__tgt_push_code_location(ptr @.source.3.1.9, ptr @__tgt_target_data_end_mapper), !dbg !15
  call void @__tgt_target_data_end_mapper(ptr @.kmpc_loc.3.3.8, i64 %i12, i32 1, ptr %i9, ptr %i10, ptr @.offload_sizes.4, ptr @.offload_maptypes.5, ptr @.offload_mapnames.6, ptr null), !dbg !15
  ret void, !dbg !16
}

; Function Attrs: nounwind uwtable
define internal void @__omp_offloading_35_d7907b29__Z3foo_l5(i64 %X.val.zext) #1 !dbg !17 {
newFuncRoot:
  %X.fpriv = alloca i32, align 4, !dbg !18
  %X.val.zext.trunc = trunc i64 %X.val.zext to i32, !dbg !18
  store i32 %X.val.zext.trunc, ptr %X.fpriv, align 4, !dbg !18
  ret void
}

declare i32 @omp_get_default_device()

declare i32 @__tgt_target_mapper(ptr, i64, ptr, i32, ptr, ptr, ptr, ptr, ptr, ptr)

declare void @__tgt_push_code_location(ptr, ptr)

declare void @__kmpc_push_num_teams(ptr, i32, i32, i32)

; Function Attrs: alwaysinline nounwind uwtable
define internal void @foo.DIR.OMP.TARGET.DATA.1.split(ptr noalias %X) #2 !dbg !19 !intel.optreport.rootnode !20 {
newFuncRoot:
  %.offload_baseptrs = alloca [1 x ptr], align 8, !dbg !26
  %.offload_ptrs = alloca [1 x ptr], align 8, !dbg !26
  %.offload_mappers = alloca [1 x ptr], align 8, !dbg !26
  %.run_host_version = alloca i32, align 4, !dbg !26
  %promoted.clause.args = alloca i8, align 1, !dbg !26
  %i = ptrtoint ptr %X to i8, !dbg !37
  store i8 %i, ptr %promoted.clause.args, align 1, !dbg !37
  %X.val = load i32, ptr %X, align 4, !dbg !37
  %X.val.zext = zext i32 %X.val to i64, !dbg !37
  %i1 = inttoptr i64 %X.val.zext to ptr, !dbg !37
  %i2 = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs, i32 0, i32 0, !dbg !37
  store ptr %i1, ptr %i2, align 8, !dbg !37
  %i3 = getelementptr inbounds [1 x ptr], ptr %.offload_ptrs, i32 0, i32 0, !dbg !37
  %i4 = inttoptr i64 %X.val.zext to ptr, !dbg !37
  store ptr %i4, ptr %i3, align 8, !dbg !37
  %i5 = getelementptr inbounds [1 x ptr], ptr %.offload_mappers, i32 0, i32 0, !dbg !37
  store ptr null, ptr %i5, align 8, !dbg !37
  %i6 = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs, i32 0, i32 0, !dbg !37
  %i7 = getelementptr inbounds [1 x ptr], ptr %.offload_ptrs, i32 0, i32 0, !dbg !37
  %i8 = call i32 @omp_get_default_device()
  %i9 = zext i32 %i8 to i64, !dbg !37
  call void @__tgt_push_code_location(ptr @.source.5.1, ptr @__tgt_target_mapper), !dbg !37
  %i10 = call i32 @__tgt_target_mapper(ptr @.kmpc_loc.5.5, i64 %i9, ptr @__omp_offloading_35_d7907b29__Z3foo_l5.region_id, i32 1, ptr %i6, ptr %i7, ptr @.offload_sizes, ptr @.offload_maptypes, ptr @.offload_mapnames, ptr null), !dbg !37
  store i32 %i10, ptr %.run_host_version, align 4, !dbg !37
  %i11 = load i32, ptr %.run_host_version, align 4, !dbg !37
  %i12 = icmp ne i32 %i11, 0, !dbg !37
  br i1 %i12, label %omp_offload.failed, label %DIR.OMP.END.TARGET.DATA.79, !dbg !37

omp_offload.failed:                               ; preds = %newFuncRoot
  call void @__kmpc_push_num_teams(ptr @.kmpc_loc.5.5.2, i32 0, i32 0, i32 1)
  call void @__omp_offloading_35_d7907b29__Z3foo_l5(i64 %X.val.zext), !dbg !37
  br label %DIR.OMP.END.TARGET.DATA.79, !dbg !37

DIR.OMP.END.TARGET.DATA.79:                       ; preds = %omp_offload.failed, %newFuncRoot
  ret void
}

declare void @__tgt_target_data_begin_mapper(ptr, i64, i32, ptr, ptr, ptr, ptr, ptr, ptr)

declare void @__tgt_target_data_end_mapper(ptr, i64, i32, ptr, ptr, ptr, ptr, ptr, ptr)

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "target.declare"="true" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { alwaysinline nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "target.declare"="true" "tune-cpu"="generic" "unsafe-fp-math"="true" }

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
!17 = distinct !DISubprogram(name: "foo.DIR.OMP.TARGET.4.split10.split.split", scope: !1, file: !1, line: 5, type: !9, scopeLine: 5, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!18 = !DILocation(line: 5, column: 1, scope: !17)
!19 = distinct !DISubprogram(name: "foo.DIR.OMP.TARGET.DATA.1.split", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!20 = distinct !{!"intel.optreport.rootnode", !21}
!21 = distinct !{!"intel.optreport", !22}
!22 = !{!"intel.optreport.first_child", !23}
!23 = distinct !{!"intel.optreport.rootnode", !24}
!24 = distinct !{!"intel.optreport", !25, !27, !28}
!25 = !{!"intel.optreport.debug_location", !26}
!26 = !DILocation(line: 3, column: 1, scope: !19)
!27 = !{!"intel.optreport.title", !"OMP TARGET DATA"}
!28 = !{!"intel.optreport.first_child", !29}
!29 = distinct !{!"intel.optreport.rootnode", !30}
!30 = distinct !{!"intel.optreport", !31, !34, !35}
!31 = !{!"intel.optreport.debug_location", !32}
!32 = !DILocation(line: 5, column: 1, scope: !33)
!33 = distinct !DISubprogram(name: "foo.DIR.OMP.TARGET.4.split10.split.split", scope: !1, file: !1, line: 5, type: !9, scopeLine: 5, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!34 = !{!"intel.optreport.title", !"OMP TARGET"}
!35 = !{!"intel.optreport.remarks", !36}
!36 = !{!"intel.optreport.remark", i32 0, !"FIRSTPRIVATE clause for variable 'X' is redundant"}
!37 = !DILocation(line: 5, column: 1, scope: !19)
