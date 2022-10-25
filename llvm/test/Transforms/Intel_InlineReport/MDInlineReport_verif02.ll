; RUN: opt -passes=inlinereportsetup -inline-report=0x180 < %s -S 2>&1 | FileCheck %s

; This test checks that verification correctly works with deleted call sites.
; Note: the test depends on particular number for NinlrDeleted reason. In case
; of adding new inline reasons please update the reasons numbers in metadata to
; match the new state.

; CHECK:define dso_local void @main() local_unnamed_addr !intel.function.inlining.report [[MAIN_FIR:![0-9]+]] {
; CHECK:  tail call void (...) @k(), !intel.callsite.inlining.report [[K_A1_MAIN_CS:![0-9]+]]
; CHECK:  tail call void (...) @x(), !intel.callsite.inlining.report [[X_B1_MAIN_CS:![0-9]+]]
; CHECK:  tail call void (...) @l(), !intel.callsite.inlining.report [[L_A2_MAIN_CS:![0-9]+]]
; CHECK:  tail call void (...) @y(), !intel.callsite.inlining.report [[Y_B2_MAIN_CS:![0-9]+]]

; CHECK: !intel.module.inlining.report = !{[[A_FIR:![0-9]+]], [[K_FIR:![0-9]+]], [[L_FIR:![0-9]+]], [[B_FIR:![0-9]+]], [[X_FIR:![0-9]+]], [[Y_FIR:![0-9]+]], [[MAIN_FIR]]}
; CHECK: [[A_FIR]] = distinct !{!"intel.function.inlining.report", [[A_NAME:![0-9]+]], [[A_CSs:![0-9]+]], [[MODULE_NAME:![0-9]+]], [[IS_DEAD_0:![0-9]+]], [[IS_DECL_0:![0-9]+]], [[LINK_A:![0-9]+]], [[LANG_C:![0-9]+]]}
; CHECK: [[A_NAME]] = !{!"name: a"}
; CHECK: [[A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[K_A_CS:![0-9]+]], [[L_A_CS:![0-9]+]]}
; CHECK: [[K_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[K_NAME:![0-9]+]], null, [[IS_INL_0:![0-9]+]]{{.*}}
; CHECK: [[K_NAME]] = !{!"name: k"}
; CHECK: [[IS_INL_0]] = !{!"isInlined: 0"}
; CHECK: [[L_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[L_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[L_NAME]] = !{!"name: l"}
; CHECK: [[IS_DEAD_0]] = !{!"isDead: 0"}
; CHECK: [[IS_DECL_0]] = !{!"isDeclaration: 0"}
; CHECK: [[LINK_A]] = !{!"linkage: A"}
; CHECK: [[LANG_C]] = !{!"language: C"}
; CHECK: [[K_FIR]] = distinct !{!"intel.function.inlining.report", [[K_NAME]], null{{.*}}
; CHECK: [[L_FIR]] = distinct !{!"intel.function.inlining.report", [[L_NAME]], null{{.*}}
; CHECK: [[B_FIR]] = distinct !{!"intel.function.inlining.report", [[B_NAME:![0-9]+]], [[B_CSs:![0-9]+]]{{.*}}
; CHECK: [[B_NAME]] = !{!"name: b"}
; CHECK: [[B_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_B_CS:![0-9]+]], [[Y_B_CS:![0-9]+]]}
; CHECK: [[X_B_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[X_NAME]] = !{!"name: x"}
; CHECK: [[Y_B_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[Y_NAME]] = !{!"name: y"}
; CHECK: [[X_FIR]] = distinct !{!"intel.function.inlining.report", [[X_NAME]], null{{.*}}
; CHECK: [[Y_FIR]] = distinct !{!"intel.function.inlining.report", [[Y_NAME]], null{{.*}}
; CHECK: [[MAIN_FIR]] = distinct !{!"intel.function.inlining.report", [[MAIN_NAME:![0-9]+]], [[MAIN_CSs:![0-9]+]], [[MODULE_NAME:![0-9]+]], [[IS_DEAD_0]], [[IS_DECL_0]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK: [[MAIN_NAME]] = !{!"name: main"}
; CHECK: [[MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[A1_MAIN_CS:![0-9]+]], [[B1_MAIN_CS:![0-9]+]], [[A2_MAIN_CS:![0-9]+]], [[B2_MAIN_CS:![0-9]+]]}
; CHECK: [[K_A1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[K_NAME:![0-9]+]], null, [[IS_INLINED_0:![0-9]+]], [[REASON_34:![0-9]+]], [[INLINE_COST_NEG1:![0-9]+]], [[OUTER_INLINE_COST_NEG1:![0-9]+]], [[INLINE_THRESHOLD:![0-9]+]], [[EARLY_EXIST_COST:![0-9]+]], [[EARLY_EXIST_THRESHOLD:![0-9]+]], [[LINE_COL_00:![^\!]+]], [[MODULE_NAME:![0-9]+]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK: [[SUPPRESS_PRINT]] = !{!"isSuppressPrint: 0"}
; CHECK-NEXT: [[X_B1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME:![0-9]+]], null, [[IS_INLINED_0:![0-9]+]], [[REASON_34:![0-9]+]], [[INLINE_COST_NEG1:![0-9]+]], [[OUTER_INLINE_COST_NEG1:![0-9]+]], [[INLINE_THRESHOLD:![0-9]+]], [[EARLY_EXIST_COST:![0-9]+]], [[EARLY_EXIST_THRESHOLD:![0-9]+]], [[LINE_COL_00:![^\!]+]], [[MODULE_NAME:![0-9]+]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK-NEXT: [[L_A2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[L_NAME:![0-9]+]], null, [[IS_INLINED_0:![0-9]+]], [[REASON_34:![0-9]+]], [[INLINE_COST_NEG1:![0-9]+]], [[OUTER_INLINE_COST_NEG1:![0-9]+]], [[INLINE_THRESHOLD:![0-9]+]], [[EARLY_EXIST_COST:![0-9]+]], [[EARLY_EXIST_THRESHOLD:![0-9]+]], [[LINE_COL_00:![^\!]+]], [[MODULE_NAME:![0-9]+]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK-NEXT: [[Y_B2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME:![0-9]+]], null, [[IS_INLINED_0:![0-9]+]], [[REASON_34:![0-9]+]], [[INLINE_COST_NEG1:![0-9]+]], [[OUTER_INLINE_COST_NEG1:![0-9]+]], [[INLINE_THRESHOLD:![0-9]+]], [[EARLY_EXIST_COST:![0-9]+]], [[EARLY_EXIST_THRESHOLD:![0-9]+]], [[LINE_COL_00:![^\!]+]], [[MODULE_NAME:![0-9]+]], [[SUPPRESS_PRINT:![0-9]+]]}


;IR with call site inlining reports unlinked from call instructions.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @a(i32 %m) local_unnamed_addr !intel.function.inlining.report !2 {
entry:
  %cmp = icmp slt i32 %m, 15
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  tail call void (...) @k()
  br label %if.end

if.else:                                          ; preds = %entry
  tail call void (...) @l()
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare !intel.function.inlining.report !20 dso_local void @k(...) local_unnamed_addr

declare !intel.function.inlining.report !22 dso_local void @l(...) local_unnamed_addr

define dso_local void @b(i32 %n) local_unnamed_addr !intel.function.inlining.report !23 {
entry:
  %cmp = icmp slt i32 %n, 3
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  tail call void (...) @x()
  br label %if.end

if.else:                                          ; preds = %entry
  tail call void (...) @y()
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare !intel.function.inlining.report !30 dso_local void @x(...) local_unnamed_addr

declare !intel.function.inlining.report !31 dso_local void @y(...) local_unnamed_addr

define dso_local void @main() local_unnamed_addr !intel.function.inlining.report !32 {
entry:
  tail call void (...) @k()
  tail call void (...) @x()
  tail call void (...) @l()
  tail call void (...) @y()
  ret void
}

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!intel.module.inlining.report = !{!2, !20, !22, !23, !30, !31, !32}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = distinct !{!"intel.function.inlining.report", !3, !4, !14, !17, !18, !19, !56}
!3 = !{!"name: a"}
!4 = distinct !{!"intel.callsites.inlining.report", !5, !15}
!5 = distinct !{!"intel.callsite.inlining.report", !6, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!6 = !{!"name: k"}
!7 = !{!"isInlined: 0"}
!8 = !{!"reason: 34"}                         ;NinlrExtern
!9 = !{!"inlineCost: -1"}
!10 = !{!"outerInlineCost: -1"}
!11 = !{!"inlineThreshold: -1"}
!12 = !{!"earlyExitCost: 2147483647"}
!13 = !{!"earlyExitThreshold: 2147483647"}
!14 = !{!"moduleName: test4.c"}
!15 = distinct !{!"intel.callsite.inlining.report", !16, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!16 = !{!"name: l"}
!17 = !{!"isDead: 0"}
!18 = !{!"isDeclaration: 0"}
!19 = !{!"linkage: A"}
!56 = !{!"language: C"}
!20 = distinct !{!"intel.function.inlining.report", !6, null, !14, !17, !21, !19, !56}
!21 = !{!"isDeclaration: 1"}
!22 = distinct !{!"intel.function.inlining.report", !16, null, !14, !17, !21, !19, !56}
!23 = distinct !{!"intel.function.inlining.report", !24, !25, !14, !17, !18, !19, !56}
!24 = !{!"name: b"}
!25 = distinct !{!"intel.callsites.inlining.report", !26, !28}
!26 = distinct !{!"intel.callsite.inlining.report", !27, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!27 = !{!"name: x"}
!28 = distinct !{!"intel.callsite.inlining.report", !29, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!29 = !{!"name: y"}
!30 = distinct !{!"intel.function.inlining.report", !27, null, !14, !17, !21, !19, !56}
!31 = distinct !{!"intel.function.inlining.report", !29, null, !14, !17, !21, !19, !56}
!32 = distinct !{!"intel.function.inlining.report", !33, !34, !14, !17, !18, !19, !56}
!33 = !{!"name: main"}
!34 = distinct !{!"intel.callsites.inlining.report", !35, !44, !48, !52}
!35 = distinct !{!"intel.callsite.inlining.report", !3, !36, !40, !41, !42, !10, !43, !12, !13, !"line: 0 col: 0", !14}
!36 = distinct !{!"intel.callsites.inlining.report", !37, !38}
!37 = distinct !{!"intel.callsite.inlining.report", !6, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!38 = distinct !{!"intel.callsite.inlining.report", !16, null, !7, !39, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!39 = !{!"reason: 31"}                          ;NinlrDeleted
!40 = !{!"isInlined: 1"}
!41 = !{!"reason: 9"}                           ;InlrSingleBasicBlock
!42 = !{!"inlineCost: -5"}
!43 = !{!"inlineThreshold: 337"}
!44 = distinct !{!"intel.callsite.inlining.report", !24, !45, !40, !41, !42, !10, !43, !12, !13, !"line: 0 col: 0", !14}
!45 = distinct !{!"intel.callsites.inlining.report", !46, !47}
!46 = distinct !{!"intel.callsite.inlining.report", !27, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!47 = distinct !{!"intel.callsite.inlining.report", !29, null, !7, !39, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!48 = distinct !{!"intel.callsite.inlining.report", !3, !49, !40, !41, !42, !10, !43, !12, !13, !"line: 0 col: 0", !14}
!49 = distinct !{!"intel.callsites.inlining.report", !50, !51}
!50 = distinct !{!"intel.callsite.inlining.report", !6, null, !7, !39, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!51 = distinct !{!"intel.callsite.inlining.report", !16, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!52 = distinct !{!"intel.callsite.inlining.report", !24, !53, !40, !41, !42, !10, !43, !12, !13, !"line: 0 col: 0", !14}
!53 = distinct !{!"intel.callsites.inlining.report", !54, !55}
!54 = distinct !{!"intel.callsite.inlining.report", !27, null, !7, !39, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!55 = distinct !{!"intel.callsite.inlining.report", !29, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}

