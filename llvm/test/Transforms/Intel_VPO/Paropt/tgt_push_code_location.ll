; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -vpo-paropt-enable-push-code-location=true -S %s | FileCheck %s --check-prefix=PCL -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -vpo-paropt-enable-push-code-location=true -S %s | FileCheck %s --check-prefix=PCL -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -vpo-paropt-enable-push-code-location=false -S %s | FileCheck %s --check-prefix=NOPCL -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -vpo-paropt-enable-push-code-location=false -S %s | FileCheck %s --check-prefix=NOPCL -check-prefix=ALL
;
;Test src:
;void foo(int* p)
;{
;   #pragma omp target map(p[0:20])
;  {
;     p[2] = 3;
;  }
;}

; ModuleID = '/tmp/icx1ar6aZ.bc'
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; PCL: [[SRC_STR:@[^ ]+]] = private unnamed_addr constant [18 x i8] c";unknown;foo;3;4;;"
; NOPCL-NOT: private unnamed_addr constant [18 x i8] c";unknown;foo;3;4;;"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32* %p) #0 !dbg !8 {
entry:
  %p.addr = alloca i32*, align 8
  %p.map.ptr.tmp = alloca i32*, align 8
  store i32* %p, i32** %p.addr, align 8
  call void @llvm.dbg.declare(metadata i32** %p.addr, metadata !13, metadata !DIExpression()), !dbg !14
  %0 = load i32*, i32** %p.addr, align 8, !dbg !15
  %1 = load i32*, i32** %p.addr, align 8, !dbg !16
  %arrayidx = getelementptr inbounds i32, i32* %1, i64 0, !dbg !16
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32* %0, i32* %arrayidx, i64 80), "QUAL.OMP.PRIVATE"(i32** %p.map.ptr.tmp) ], !dbg !15

; Check that we are generating __tgt_push_code_location only when -vpo-paropt-enable-push-code-location=true
; PCL: call void @__tgt_push_code_location(i8* getelementptr inbounds ([18 x i8], [18 x i8]* [[SRC_STR]], i32 0, i32 0), i8* bitcast (i32 (i64, i8*, i32, i8**, i8**, i64*, i64*)* @__tgt_target to i8*))
; NOPCL-NOT: call void @__tgt_push_code_location
; ALL: call i32 @__tgt_target({{.*}})

  store i32* %0, i32** %p.map.ptr.tmp, align 8, !dbg !15
  %3 = load i32*, i32** %p.map.ptr.tmp, align 8, !dbg !18
  %arrayidx1 = getelementptr inbounds i32, i32* %3, i64 2, !dbg !18
  store i32 3, i32* %arrayidx1, align 4, !dbg !20

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ], !dbg !15
  ret void, !dbg !21
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" !dbg !22 {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare dso_local void @__tgt_register_requires(i64)

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }
attributes #3 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!3}
!llvm.module.flags = !{!4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "jj.c", directory: "/path/to/file/pushCodeLocationTest")
!2 = !{}
!3 = !{i32 0, i32 2055, i32 107889788, !"foo", i32 3, i32 0, i32 0}
!4 = !{i32 7, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{!"clang version 9.0.0"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !9, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DILocalVariable(name: "p", arg: 1, scope: !8, file: !1, line: 1, type: !11)
!14 = !DILocation(line: 1, column: 15, scope: !8)
!15 = !DILocation(line: 3, column: 4, scope: !8)
!16 = !DILocation(line: 3, column: 27, scope: !17)
!17 = distinct !DILexicalBlock(scope: !8, file: !1, line: 3, column: 4)
!18 = !DILocation(line: 5, column: 6, scope: !19)
!19 = distinct !DILexicalBlock(scope: !17, file: !1, line: 4, column: 3)
!20 = !DILocation(line: 5, column: 11, scope: !19)
!21 = !DILocation(line: 7, column: 1, scope: !8)
!22 = distinct !DISubprogram(linkageName: ".omp_offloading.requires_reg", scope: !1, file: !1, type: !23, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !0, retainedNodes: !2)
!23 = !DISubroutineType(types: !2)

