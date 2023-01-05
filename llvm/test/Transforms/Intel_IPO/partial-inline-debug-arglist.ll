; RUN: opt -S -skip-partial-inlining-cost-analysis -passes=partial-inliner < %s | FileCheck %s
;
; When the partial inliner code extracts a region containing debug information,
; make sure the debug emission is code extracted properly.
; The following cases are checked:
;   1. Plain value, no DIArgList.
;   2. DIArgList using one extracted value and one constant.
;   3. DIArgList using one extracted value, but uses it twice.
;   4. DIArgList using one extracted value and one not-extracted param.
;
; In the cases above, it should be possible to retain the location information
; in all of the cases except the last (where one value isn't available).
;
; For now, the expected behavior for each case is:
;   1. Preserved in the code extracted routine.
;   2. Dropped. (can be improved, see FIXME below)
;   3. Dropped. (can be improved, see FIXME below)
;   4. Dropped. (expected)
;
;
; First, validate the debug information from the original routine is retained.
;
; CHECK:      define internal i32 @inlinedFunc(i1 %param1, i32* align 4 %param2)
; CHECK-SAME: !dbg [[SP1:![0-9]+]]
; CHECK:      call void @llvm.dbg.value(metadata i32* %param2
; CHECK-SAME: metadata [[VAR1:![0-9]+]]
; CHECK-SAME: metadata !DIExpression())
; CHECK-SAME: !dbg [[LOC1:![0-9]+]]
; CHECK:      call void @llvm.dbg.value(metadata !DIArgList(i32* %param2, i32 42)
; CHECK-SAME: metadata [[VAR1]]
; CHECK-SAME: metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value))
; CHECK-SAME: !dbg [[LOC1]]
; CHECK:      call void @llvm.dbg.value(metadata !DIArgList(i32* %param2, i32* %param2)
; CHECK-SAME: metadata [[VAR1]]
; CHECK-SAME: metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value))
; CHECK-SAME: !dbg [[LOC1]]
; CHECK:      call void @llvm.dbg.value(metadata !DIArgList(i1 %param1, i32* %param2)
; CHECK-SAME: metadata [[VAR1]]
; CHECK-SAME: metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value))
; CHECK-SAME: !dbg [[LOC1]]
; CHECK:      }
;
;
; Second, check the partial inlined routine contains correct debug information.
;
; CHECK:      define internal void @inlinedFunc.1.if.then(i32* %param2)
; CHECK-SAME: !dbg [[SP2:![0-9]+]]
; CHECK:      call void @llvm.dbg.value(metadata i32* %param2
; CHECK-SAME: metadata [[VAR2:![0-9]+]]
; CHECK-SAME: metadata !DIExpression())
; CHECK-SAME: !dbg [[LOC2:![0-9]+]]
; FIXME:      call void @llvm.dbg.value(metadata !DIArgList(i32* %param2, i32 42)
; FIXME-SAME: metadata [[VAR2]]
; FIXME-SAME: metadata !DIExpression(DW_OP_deref))
; FIXME-SAME: !dbg [[LOC2]]
; FIXME:      call void @llvm.dbg.value(metadata !DIArgList(i32* %param2, i32* %param2)
; FIXME-SAME: metadata [[VAR2]]
; FIXME-SAME: metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_deref, DW_OP_stack_value))
; FIXME-SAME: !dbg [[LOC2]]
; CHECK-NOT:  call void @llvm.dbg.value
; CHECK:      }
; 
;
; Validate the debug information metadata corresponds to the correct routines.
;
; CHECK: [[SP1]] = distinct !DISubprogram(name: "inlinedFunc"
; CHECK: [[VAR1]] = !DILocalVariable(name: "var"
; CHECK-SAME: scope: [[SP1]]
; CHECK: [[LOC1]] = !DILocation(line: 5, column: 1, scope: [[SP1]])
;
; CHECK: [[SP2]] = distinct !DISubprogram(name: "inlinedFunc.1.if.then"
; CHECK: [[VAR2]] = !DILocalVariable(name: "var", scope: [[SP2]]
; CHECK: [[LOC2]] = !DILocation(line: 5, column: 1, scope: [[SP2]])
;

declare void @llvm.dbg.value(metadata, metadata, metadata)

define internal i32 @inlinedFunc(i1 %param1, i32* align 4 %param2) !dbg !4 {
entry:
  br i1 %param1, label %if.then, label %return
if.then:
  call void @llvm.dbg.value(metadata i32* %param2, metadata !11, metadata !DIExpression()), !dbg !12
  call void @llvm.dbg.value(metadata !DIArgList(i32* %param2, i32 42), metadata !11, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value)), !dbg !12
  call void @llvm.dbg.value(metadata !DIArgList(i32* %param2, i32* %param2), metadata !11, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value)), !dbg !12
  call void @llvm.dbg.value(metadata !DIArgList(i1 %param1, i32* %param2), metadata !11, metadata !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value)), !dbg !12
  store i32 10, i32* %param2, align 4
  br label %return
return:             ; preds = %entry
  ret i32 0
}

define internal i32 @caller(i1 %param1, i32* align 2 %param2) !dbg !5 {
entry:
  %val = call i32 @inlinedFunc(i1 %param1, i32* %param2), !dbg !13
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
!11 = !DILocalVariable(name: "var", scope: !4, file: !1, line: 3, type: !10)
!12 = !DILocation(line: 5, column: 1, scope: !4)
!13 = !DILocation(line: 6, column: 1, scope: !5)
