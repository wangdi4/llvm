; RUN: opt -mtriple=x86_64-unknown-linux-gnu -x86-feature-proc-init -codegen-opt-level=1 -S %s | FileCheck %s

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @main() #0 !dbg !7 {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = alloca i32, align 4, !dbg ![[DBG11:[0-9]+]]
; CHECK-NEXT:    [[TMP1:%.*]] = bitcast i32* [[TMP0]] to i8*, !dbg ![[DBG11]]
; CHECK-NEXT:    call void @llvm.lifetime.start.p0i8(i64 4, i8* [[TMP1]]), !dbg ![[DBG11]]
; CHECK-NEXT:    call void @llvm.x86.sse.stmxcsr(i8* [[TMP1]]), !dbg ![[DBG11]]
; CHECK-NEXT:    [[STMXCSR:%.*]] = load i32, i32* [[TMP0]], align 4, !dbg ![[DBG11]]
; CHECK-NEXT:    [[FTZ_DAZ:%.*]] = or i32 [[STMXCSR]], 32832, !dbg ![[DBG11]]
; CHECK-NEXT:    store i32 [[FTZ_DAZ]], i32* [[TMP0]], align 4, !dbg ![[DBG11]]
; CHECK-NEXT:    call void @llvm.x86.sse.ldmxcsr(i8* [[TMP1]]), !dbg ![[DBG11]]
; CHECK-NEXT:    call void @llvm.lifetime.end.p0i8(i64 4, i8* [[TMP1]]), !dbg ![[DBG11]]
; CHECK-NEXT:    ret i32 0, [[DBG12:!dbg !.*]]
;
entry:
  ret i32 0, !dbg !11
}

; CHECK: ![[DBG7:[0-9]+]] = distinct !DISubprogram(name: "main",{{.*}}scopeLine: 2
; CHECK: ![[DBG11]] = !DILocation(line: 2, scope: ![[DBG7]])

attributes #0 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cpp", directory: "/tmp")
!2 = !{}                                                                                                                                                                                                                                     !3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !8, scopeLine: 2, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}                                                                                                                                                                                                                                  !10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocation(line: 3, column: 5, scope: !7)
