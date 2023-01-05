; RUN: opt -passes=inlinereportsetup -inline-report=128 < %s -S 2>&1 | FileCheck %s

; This test checks that metadata corresponding to the inlining report was
; created.

; CHECK: i32 @main(){{.*}}!intel.function.inlining.report [[MAIN_FIR:![0-9]+]]
; CHECK: call {{.*}}@a(){{.*}}!intel.callsite.inlining.report
; CHECK: call {{.*}}@b(){{.*}}!intel.callsite.inlining.report
; CHECK: call {{.*}}@a(){{.*}}!intel.callsite.inlining.report

; CHECK: declare !intel.function.inlining.report [[B_FIR:![0-9]+]] dso_local void @b(...)

; CHECK: define dso_local void @a(){{.*}}!intel.function.inlining.report [[A_FIR:![0-9]+]] {
; CHECK: call {{.*}}@x(){{.*}}!intel.callsite.inlining.report
; CHECK: call {{.*}}@y(){{.*}}!intel.callsite.inlining.report

; CHECK: declare !intel.function.inlining.report [[X_FIR:![0-9]+]] dso_local void @x(...)

; CHECK: define internal fastcc void @y(){{.*}}!intel.function.inlining.report [[Y_FIR:![0-9]+]] {
; CHECK: call{{.*}}@z(){{.*}}!intel.callsite.inlining.report

; CHECK: !intel.function.inlining.report [[Z_FIR:![0-9]+]] dso_local void @z(...)

; CHECK: !intel.module.inlining.report = !{[[MAIN_FIR]], [[B_FIR]], [[A_FIR]], [[X_FIR]], [[Y_FIR]], [[Z_FIR]]}
; CHECK: [[MAIN_FIR]] = distinct !{!"intel.function.inlining.report", [[MAIN_NAME:![0-9]+]], [[MAIN_CSs:![0-9]+]], [[MODULE_NAME:![0-9]+]], [[ISDEAD_0:![0-9]+]], [[ISDECL_0:![0-9]+]], [[LINK_A:![0-9]+]], [[LANG_C:![0-9]+]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK-NEXT: [[MAIN_NAME]] = !{!"name: main"}
; CHECK-NEXT: [[MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[A1_MAIN_CS:![0-9]+]], [[B_MAIN_CS:![0-9]+]], [[A2_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[A1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME:![0-9]+]], null, [[INL_0:![0-9]+]], [[REASON_NO_REASON:![0-9]+]], [[INL_COST:![0-9]+]], [[OUT_INL_COST:![0-9]+]], [[INL_TR:![0-9]+]], [[EE_COST:![0-9]+]], [[EE_TR:![0-9]+]], !"line: 11 col: 3", [[MODULE_NAME]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK-NEXT: [[A_NAME]] = !{!"name: a"}
; CHECK-NEXT: [[INL_0]] = !{!"isInlined: 0"}
; CHECK-NEXT: [[REASON_NO_REASON]] = !{!"reason: {{[0-9]+}}"}
; CHECK-NEXT: [[INL_COST]] = !{!"inlineCost: -1"}
; CHECK-NEXT: [[OUT_INL_COST]] = !{!"outerInlineCost: -1"}
; CHECK-NEXT: [[INL_TR]] = !{!"inlineThreshold: -1"}
; CHECK-NEXT: [[EE_COST]] = !{!"earlyExitCost: 2147483647"}
; CHECK-NEXT: [[EE_TR]] = !{!"earlyExitThreshold: 2147483647"}
; CHECK-NEXT: [[MODULE_NAME]] = !{!"moduleName: <stdin>"}
; CHECK-NEXT: [[SUPPRESS_PRINT]] = !{!"isSuppressPrint: 0"}
; CHECK-NEXT: [[B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[B_NAME:![0-9]+]], null, [[INL_0]], [[REASON_EXTRN:![0-9]+]], [[INL_COST]], [[OUT_INL_COST]], [[INL_TR]], [[EE_COST]], [[EE_TR]], !"line: 12 col: 3", [[MODULE_NAME]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[B_NAME]] = !{!"name: b"}
; CHECK-NEXT: [[REASON_EXTRN]] = !{!"reason: {{[0-9]+}}"}
; CHECK-NEXT: [[A2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME]], null, [[INL_0]], [[REASON_NO_REASON]], [[INL_COST]], [[OUT_INL_COST]], [[INL_TR]], [[EE_COST]], [[EE_TR]], !"line: 13 col: 3", [[MODULE_NAME]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[ISDEAD_0]] = !{!"isDead: 0"}
; CHECK-NEXT: [[ISDECL_0]] = !{!"isDeclaration: 0"}
; CHECK-NEXT: [[LINK_A]] = !{!"linkage: A"}
; CHECK-NEXT: [[LANG_C]] = !{!"language: C"}
; CHECK-NEXT: [[B_FIR]] = distinct !{!"intel.function.inlining.report", [[B_NAME]], null, [[MODULE_NAME]], [[ISDEAD_0]], [[ISDECL_1:![0-9]+]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[ISDECL_1]] = !{!"isDeclaration: 1"}
; CHECK-NEXT: [[A_FIR]] = distinct !{!"intel.function.inlining.report", [[A_NAME]], [[A_CSs:![0-9]+]], [[MODULE_NAME]], [[ISDEAD_0]], [[ISDECL_0]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_A_CS:![0-9]+]], [[Y_A_CS:![0-9]+]]}
; CHECK-NEXT: [[X_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME:![0-9]+]], null, [[INL_0]], [[REASON_EXTRN]], [[INL_COST]], [[OUT_INL_COST]], [[INL_TR]], [[EE_COST]], [[EE_TR]], !"line: 18 col: 3", [[MODULE_NAME]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[X_NAME]] = !{!"name: x"}
; CHECK-NEXT: [[Y_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME:![0-9]+]], null, [[INL_0]], [[REASON_NO_REASON]], [[INL_COST]], [[OUT_INL_COST]], [[INL_TR]], [[EE_COST]], [[EE_TR]], !"line: 19 col: 3", [[MODULE_NAME]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[Y_NAME]] = !{!"name: y"}
; CHECK-NEXT: [[X_FIR]] = distinct !{!"intel.function.inlining.report", [[X_NAME]], null, [[MODULE_NAME]], [[ISDEAD_0]], [[ISDECL_1]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[Y_FIR]] = distinct !{!"intel.function.inlining.report", [[Y_NAME]], [[Y_CSs:![0-9]+]], [[MODULE_NAME]], [[ISDEAD_0]], [[ISDECL_0]], [[LINK_L:![0-9]+]], [[LANG_C]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[Y_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_CS:![0-9]+]]}
; CHECK-NEXT: [[Z_Y_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME:![0-9]+]], null, [[INL_0]], [[REASON_EXTRN]], [[INL_COST]], [[OUT_INL_COST]], [[INL_TR]], [[EE_COST]], [[EE_TR]], !"line: 23 col: 3", [[MODULE_NAME]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[Z_NAME]] = !{!"name: z"}
; CHECK-NEXT: [[LINK_L]] = !{!"linkage: L"}
; CHECK-NEXT: [[Z_FIR]] = distinct !{!"intel.function.inlining.report", [[Z_NAME]], null, [[MODULE_NAME]], [[ISDEAD_0]], [[ISDECL_1]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT]]}


; Original IR with no inline report metadata.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() local_unnamed_addr !dbg !10 {
entry:
  call void @a(), !dbg !14
  call void (...) @b(), !dbg !15
  call void @a(), !dbg !16
  ret i32 0, !dbg !17
}

declare dso_local void @b(...) local_unnamed_addr

define dso_local void @a() local_unnamed_addr !dbg !18 {
entry:
  call void (...) @x(), !dbg !21
  call fastcc void @y(), !dbg !22
  ret void, !dbg !23
}

declare dso_local void @x(...) local_unnamed_addr

define internal fastcc void @y() unnamed_addr !dbg !24 {
entry:
  call void (...) @z(), !dbg !25
  ret void, !dbg !26
}

declare dso_local void @z(...) local_unnamed_addr

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6, !7}
!llvm.dbg.intel.emit_class_debug_always = !{!8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based icx (ICX) 2019.8.2.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test1.c", directory: "/export/iusers/ochupina/inl_report")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 1, !"ThinLTO", i32 0}
!7 = !{i32 1, !"EnableSplitLTOUnit", i32 0}
!8 = !{!"true"}
!9 = !{!"icx (ICX) 2019.8.2.0"}
!10 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !11, scopeLine: 10, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!11 = !DISubroutineType(types: !12)
!12 = !{!13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !DILocation(line: 11, column: 3, scope: !10)
!15 = !DILocation(line: 12, column: 3, scope: !10)
!16 = !DILocation(line: 13, column: 3, scope: !10)
!17 = !DILocation(line: 14, column: 3, scope: !10)
!18 = distinct !DISubprogram(name: "a", scope: !1, file: !1, line: 17, type: !19, scopeLine: 17, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!19 = !DISubroutineType(types: !20)
!20 = !{null}
!21 = !DILocation(line: 18, column: 3, scope: !18)
!22 = !DILocation(line: 19, column: 3, scope: !18)
!23 = !DILocation(line: 20, column: 1, scope: !18)
!24 = distinct !DISubprogram(name: "y", scope: !1, file: !1, line: 22, type: !19, scopeLine: 22, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!25 = !DILocation(line: 23, column: 3, scope: !24)
!26 = !DILocation(line: 24, column: 1, scope: !24)
