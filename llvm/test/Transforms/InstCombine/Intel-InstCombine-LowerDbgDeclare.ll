; RUN: opt -S -passes='instcombine<no-verify-fixpoint>' -instcombine-lower-dbg-declare=1 < %s \
; RUN:   | FileCheck %s
;
; Intel-specific validation of LowerDbgDeclare.
; Verify llvm.dbg.declare lowering tracks values through address space casts.
;
; CHECK:       define {{.*}} @main() !dbg [[MAIN:![0-9]+]] {
; CHECK-LABEL: entry
; CHECK-NOT:   call {{.*}} @llvm.dbg.declare
; CHECK:       call void @llvm.dbg.value(metadata i32 8
; CHECK-SAME:    metadata [[VARIABLE:![0-9]+]]
; CHECK-SAME:    metadata !DIExpression()), !dbg !11
; CHECK:       call void @llvm.dbg.value(metadata i32 8
; CHECK-SAME:    metadata [[VARIABLE]]
; CHECK-SAME:    metadata !DIExpression()), !dbg !11
; CHECK:       }
;
; CHECK:       [[MAIN]] = distinct !DISubprogram(name: "main"
; CHECK:       [[VARIABLE]] = !DILocalVariable(name: "variable"
; CHECK-SAME:    scope: [[MAIN]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"

declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

define dso_local i32 @main() !dbg !5 {
entry:
  %variable = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32* %variable, metadata !10, metadata !DIExpression()), !dbg !11
  %cast = addrspacecast i32* %variable to i32 addrspace(4)*
  store i32 8, i32 addrspace(4)* %cast, align 4
  %value = load i32, i32 addrspace(4)* %cast, align 4
  ret i32 %value
}

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!llvm.ident = !{!4}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test.cpp", directory: "/path/to")
!2 = !{i32 2, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{!"clang"}
!5 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !6, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !9)
!6 = !DISubroutineType(types: !7)
!7 = !{!8}
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = !{!10}
!10 = !DILocalVariable(name: "variable", scope: !5, file: !1, line: 2, type: !8)
!11 = !DILocation(line: 3, scope: !5)
