; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s
;
; Verify the code extractor does not duplicate the debug compilation unit or
; subprogram of an inlined function when extracting a parallel region.
;
; CHECK: !llvm.dbg.cu = !{[[UNIT:![0-9]+]]}
; CHECK: [[UNIT]] = distinct !DICompileUnit
; CHECK: !{{[0-9]+}} = distinct !DISubprogram(name: "main"
; CHECK-SAME: unit: [[UNIT]]
; CHECK: !{{[0-9]+}} = distinct !DISubprogram(linkageName: "foo"
; CHECK-SAME: unit: [[UNIT]]
; CHECK: !{{[0-9]+}} = distinct !DISubprogram(name: "main.DIR.OMP.TARGET.{{.*}}"
; CHECK-SAME: unit: [[UNIT]]
; CHECK-NOT: !DICompileUnit
; CHECK-NOT: !DISubprogram

; ModuleID = 'test.bc'
source_filename = "test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @main() local_unnamed_addr #0 !dbg !3 {
entry:
  %end.dir.temp23 = alloca i1, align 1
  br label %DIR.OMP.TARGET.DATA.1

DIR.OMP.TARGET.DATA.1:                            ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TOFROM"([4096 x i32]* undef, [4096 x i32]* undef, i64 16384, i64 35), "QUAL.OMP.MAP.TOFROM"([2 x i32]* undef, [2 x i32]* undef, i64 8, i64 35), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp23) ]
  %temp.load24 = load volatile i1, i1* %end.dir.temp23, align 1
  br label %DIR.OMP.END.TARGET.DATA.13

DIR.OMP.END.TARGET.DATA.13:                       ; preds = %DIR.OMP.TARGET.DATA.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.DATA"() ]
  %guard.uninitialized.i.i73 = icmp eq i8 undef, 0, !dbg !6
  unreachable
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, nameTableKind: None)
!1 = !DIFile(filename: "test.cpp", directory: "/path/to")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 6, type: !4, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!4 = !DISubroutineType(types: !5)
!5 = !{}
!6 = !DILocation(line: 1, scope: !7, inlinedAt: !8)
!7 = distinct !DISubprogram(linkageName: "foo", scope: !1, file: !1, type: !4, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!8 = distinct !DILocation(line: 2, column: 3, scope: !3)
