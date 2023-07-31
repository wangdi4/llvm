; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:

; int main() {
;   short y = 111;
; #pragma omp target map(to:y)
;   ;
; }

; Since the module has debug information, the var "%0" does not have a name,
; or any associated debug metadata, check that a default "unknown"
; map-name is used for the map operand.
; CHECK: @.mapname = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
; CHECK: @.offload_mapnames = private constant [1 x ptr] [ptr @.mapname]

; Check that tgt_mapper is called using the map-names struct.
; CHECK:  %{{[^ ]+}} = call i32 @__tgt_target_mapper(ptr @{{[^ ,]+}}, i64 %{{[^ ,]+}}, ptr @{{[^ ,]+}}, i32 1, ptr %{{[^ ,]}}, ptr %{{[^ ,]}}, ptr @.offload_sizes, ptr @.offload_maptypes, ptr @.offload_mapnames, ptr null)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define hidden i32 @main() #0 !dbg !9 {
entry:
  %0 = alloca i16, align 2
  store i16 111, ptr %0, align 2

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr %0, ptr %0, i64 2, i64 33, ptr null, ptr null) ] ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)
, !dbg !13
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
, !dbg !13

  ret i32 0, !dbg !15
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline norecurse nounwind optnone uwtable mustprogress "contains-openmp-target"="true" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!3}
!llvm.module.flags = !{!4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "tgt_fp.c", directory: "/path/to/file")
!2 = !{}
!3 = !{i32 0, i32 2055, i32 151396820, !"_Z4main", i32 3, i32 0, i32 0}
!4 = !{i32 7, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 7, !"PIC Level", i32 2}
!8 = !{!"clang 10.0.0"}
!9 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !10, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!10 = !DISubroutineType(types: !11)
!11 = !{!12}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DILocation(line: 3, column: 1, scope: !14)
!14 = distinct !DILexicalBlock(scope: !9, file: !1, line: 3, column: 1)
!15 = !DILocation(line: 5, column: 1, scope: !9)
