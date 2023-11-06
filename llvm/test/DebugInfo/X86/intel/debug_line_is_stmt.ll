; RUN: llc %s -o - -filetype=asm -intel-adjust-is-stmt=true \
; RUN:   | FileCheck %s --check-prefixes=ASM,ASM-T
; RUN: llc %s -o - -filetype=asm -intel-adjust-is-stmt=false \
; RUN:   | FileCheck %s --check-prefixes=ASM,ASM-F
; RUN: llc %s -o - -filetype=obj -intel-adjust-is-stmt=true \
; RUN:   | llvm-dwarfdump --debug-line - \
; RUN:   | FileCheck %s --check-prefixes=OBJ,OBJ-T
; RUN: llc %s -o - -filetype=obj -intel-adjust-is-stmt=false \
; RUN:   | llvm-dwarfdump --debug-line - \
; RUN:   | FileCheck %s --check-prefixes=OBJ,OBJ-F
;
; ASM:           foo:
; ASM:           .Lfunc_begin0:
; ASM:             .file   1 "/path/to" "test.c"
; ASM-NEXT:        .loc    1 1 1 prologue_end
; ASM-NEXT:        movq    $1, -16(%rsp)
; ASM:             .loc    1 2 1
; ASM-NEXT:        cmpq    -8(%rsp), %rax
; ASM-NEXT:        jne     .LBB0_3
; ASM:           .LBB0_2:
; ASM-T:           .loc    1 2 1
; ASM-F-NOT:       .loc
; ASM-NEXT:        movq    -16(%rsp), %rcx
; ASM:             je      .LBB0_4
; ASM:           .LBB0_3:
; ASM:             .loc    1 3 1
; ASM-NEXT:        movq    %rax, %rcx
; ASM:             jmp     .LBB0_2
; ASM:           .LBB0_4:
; ASM-NEXT:        .loc    1 4 1
; ASM-NEXT:        xorl    %eax, %eax
; ASM:             retq
; ASM:           .Lfunc_end0:
;
; OBJ:       000   1   1   1   0   0   0  is_stmt prologue_end
; OBJ:       017   2   1   1   0   0   0  is_stmt
; OBJ-T:     01e   2   1   1   0   0   0  is_stmt
; OBJ-F-NOT: 01e
; OBJ:       02d   3   1   1   0   0   0  is_stmt
; OBJ:       03b   4   1   1   0   0   0  is_stmt
; OBJ:       03e   4   1   1   0   0   0  is_stmt end_sequence

target triple = "x86_64-unknown-linux-gnu"

define i64 @foo() #0 !dbg !6 {
entry:
  %v1.addr = alloca i64, align 8
  %v2.addr = alloca i64, align 8
  br label %bb1

bb1:
  store i64 1, ptr %v1.addr, align 8, !dbg !11
  store i64 2, ptr %v2.addr, align 8, !dbg !11
  %v1_1 = load i64, ptr %v1.addr, align 8, !dbg !11
  %v2_1 = load i64, ptr %v2.addr, align 8, !dbg !11
  %cmp1 = icmp eq i64 %v1_1, %v2_1, !dbg !12
  br i1 %cmp1, label %bb2, label %bb3, !dbg !12

bb2:
  %noop = add i64 0, 1
  call void @llvm.dbg.declare(metadata ptr %v1.addr, metadata !8, metadata !DIExpression()), !dbg !11
  call void @llvm.dbg.declare(metadata ptr %v2.addr, metadata !9, metadata !DIExpression()), !dbg !11
  %v1_2 = load i64, ptr %v1.addr, align 8, !dbg !12
  %v2_2 = load i64, ptr %v2.addr, align 8, !dbg !12
  %add = add i64 %v1_1, %v2_2, !dbg !12
  %cmp2 = icmp eq i64 %v1_2, %v2_2, !dbg !12
  br i1 %cmp2, label %exit, label %bb3, !dbg !12

bb3:
  %inc = add i64 %v1_1, 1, !dbg !13
  store i64 %inc, ptr %v1.addr, align 8, !dbg !13
  br label %bb2, !dbg !13

exit:
  ret i64 0, !dbg !14
}

declare void @llvm.dbg.declare(metadata, metadata, metadata)

attributes #0 = { noinline optnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, isOptimized: true, debugInfoForProfiling: false, emissionKind: FullDebug)
!1 = !DIFile(filename: "test.c", directory: "/path/to")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{}
!6 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !7, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, unit: !0)
!7 = !DISubroutineType(types: !2)
!8 = !DILocalVariable(name: "v1", scope: !6, file: !1, line: 2, type: !10)
!9 = !DILocalVariable(name: "v2", scope: !6, file: !1, line: 2, type: !10)
!10 = !DIBasicType(name: "int", size: 64, encoding: DW_ATE_unsigned)
!11 = !DILocation(line: 1, column: 1, scope: !6)
!12 = !DILocation(line: 2, column: 1, scope: !6)
!13 = !DILocation(line: 3, column: 1, scope: !6)
!14 = !DILocation(line: 4, column: 1, scope: !6)
