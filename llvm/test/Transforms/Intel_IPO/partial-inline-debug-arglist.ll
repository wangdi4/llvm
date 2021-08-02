; RUN: opt < %s -skip-partial-inlining-cost-analysis -partial-inliner -S \
; RUN:  | FileCheck %s
;
; When the partial inliner code extracts a region of code containing debug
; information, make sure the debug emission is code extracted properly.
; For now, "properly" means it is deleted. See the FIXME below for improvement.
;
; CHECK: define internal i32 @inlinedFunc(i1 %cond, i32* align 4 %align.val)
; CHECK:   call void @llvm.dbg.value(metadata !DIArgList(i32* %align.val, i32 42), metadata [[VAR1:![0-9]+]], metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value))
; CHECK: }
;
; CHECK: define internal void @inlinedFunc.1.if.then(i32* %align.val)
; CHECK-NOT: call void @llvm.dbg.value
; FIXME:   call void @llvm.dbg.value(metadata !DIArgList(i32* %align.val, i32 42), metadata [[VAR2:![0-9]+]], metadata !DIExpression(DW_OP_deref, DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value)), !dbg !20
; CHECK: }
;
; CHECK: [[INLINEDFUNC1:![0-9]+]] = distinct !DISubprogram(name: "inlinedFunc"
; CHECK: [[VAR1]] = !DILocalVariable(name: "Var", scope: [[INLINEDFUNC1]]
; CHECK: [[INLINEDFUNC2:![0-9]+]] = distinct !DISubprogram(name: "inlinedFunc.1.if.then"
; FIXME: [[VAR2]] = !DILocalVariable(name: "Var", scope: [[INLINEDFUNC2]]
;

declare void @llvm.dbg.value(metadata, metadata, metadata)

define internal i32 @inlinedFunc(i1 %cond, i32* align 4 %align.val) !dbg !4 {
entry:
  br i1 %cond, label %if.then, label %return
if.then:
  ; Dummy store to have more than 0 uses
  call void @llvm.dbg.value(metadata !DIArgList(i32* %align.val, i32 42), metadata !11, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value)), !dbg !12
  store i32 10, i32* %align.val, align 4
  br label %return
return:             ; preds = %entry
  ret i32 0
}

define internal i32 @caller(i1 %cond, i32* align 2 %align.val) !dbg !5 {
entry:
  %val = call i32 @inlinedFunc(i1 %cond, i32* %align.val), !dbg !13
  ret i32 %val
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "handwritten", isOptimized: true, emissionKind: FullDebug)
!1 = !DIFile(filename: "<stdin>", directory: "/")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "inlinedFunc", scope: !1, file: !1, line: 1, type: !6, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = distinct !DISubprogram(name: "caller", scope: !1, file: !1, line: 2, type: !6, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!6 = !DISubroutineType(types: !7)
!7 = !{!9, !8, !10}
!8 = !DIBasicType(name: "int", size: 8, encoding: DW_ATE_signed)
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64)
!11 = !DILocalVariable(name: "Var", scope: !4, file: !1, line: 3, type: !10)
!12 = !DILocation(line: 4, column: 1, scope: !4)
!13 = !DILocation(line: 5, column: 1, scope: !5)
