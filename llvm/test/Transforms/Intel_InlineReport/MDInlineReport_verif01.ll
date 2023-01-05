; RUN: opt -passes=inlinereportsetup -inline-report=0x180 < %s -S 2>&1 | FileCheck %s

; This test checks that metadata corresponding to the inlining report
; will be properly linked back to the IR.

; CHECK: define dso_local i32 @main() local_unnamed_addr !intel.function.inlining.report [[MAIN_FIR:![0-9]+]] {
; CHECK:  tail call void (...) @x(), !intel.callsite.inlining.report [[X_A1_MAIN_CS:![0-9]+]]
; CHECK:  tail call void (...) @z(), !intel.callsite.inlining.report [[Z_Y_A1_MAIN_CS:![0-9]+]]
; CHECK:  tail call void (...) @b(), !intel.callsite.inlining.report [[B_MAIN_CS:![0-9]+]]
; CHECK:  tail call void (...) @x(), !intel.callsite.inlining.report [[X_A2_MAIN_CS:![0-9]+]]
; CHECK:  tail call void (...) @z(), !intel.callsite.inlining.report [[Z_Y_A2_MAIN_CS:![0-9]+]]
; CHECK:  ret i32 0

; CHECK: declare !intel.function.inlining.report [[B_FIR:![0-9]+]] dso_local void @b(...) local_unnamed_addr

; CHECK: define dso_local void @a() local_unnamed_addr !intel.function.inlining.report [[A_FIR:![0-9]+]] {
; CHECK:  tail call void (...) @x(), !intel.callsite.inlining.report [[X_A_CS:![0-9]+]]
; CHECK:  tail call void (...) @z(), !intel.callsite.inlining.report [[Z_Y_A_CS:![0-9]+]]
; CHECK:  ret void

; CHECK: declare !intel.function.inlining.report [[X_FIR:![0-9]+]] dso_local void @x(...) local_unnamed_addr

; CHECK: declare !intel.function.inlining.report [[Z_FIR:![0-9]+]] dso_local void @z(...) local_unnamed_addr

; CHECK: !intel.module.inlining.report = !{[[MAIN_FIR]], [[B_FIR]], [[A_FIR]], [[X_FIR]], [[Y_FIR:![0-9]+]], [[Z_FIR]]}
; CHECK: [[MAIN_FIR]] = distinct !{!"intel.function.inlining.report", [[MAIN_NAME:![0-9]+]], [[MAIN_CSs:![0-9]+]]{{.*}}
; CHECK: [[MAIN_NAME]] = !{!"name: main"}
; CHECK: [[MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[A1_MAIN_CS:![0-9]+]], [[B_MAIN_CS]], [[A2_MAIN_CS:![0-9]+]]}
; CHECK: [[A1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME:![0-9]+]], [[A1_MAIN_CSs:![0-9]+]], [[IS_INL_1:![0-9]+]]{{.*}}
; CHECK: [[A_NAME]] = !{!"name: a"}
; CHECK: [[A1_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_A1_MAIN_CS]], [[Y_A1_MAIN_CS:![0-9]+]]}
; CHECK: [[X_A1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME:![0-9]+]], null, [[IS_INL_0:![0-9]+]]{{.*}}
; CHECK: [[X_NAME]] = !{!"name: x"}
; CHECK: [[IS_INL_0]] = !{!"isInlined: 0"}
; CHECK: [[Y_A1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME:![0-9]+]], [[Y_A1_MAIN_CSs:![0-9]+]], [[IS_INL_1]]{{.*}}
; CHECK: [[Y_NAME]] = !{!"name: y"}
; CHECK: [[Y_A1_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_A1_MAIN_CS]]}
; CHECK: [[Z_Y_A1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[Z_NAME]] = !{!"name: z"}
; CHECK: [[IS_INL_1]] = !{!"isInlined: 1"}
; CHECK: [[B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[B_NAME:![0-9]+]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[B_NAME]] = !{!"name: b"}
; CHECK: [[A2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME]], [[A2_MAIN_CSs:![0-9]+]], [[IS_INL_1]]{{.*}}
; CHECK: [[A2_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_A2_MAIN_CS]], [[Y_A2_MAIN_CS:![0-9]+]]}
; CHECK: [[X_A2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[Y_A2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME]], [[Y_A2_MAIN_CSs:![0-9]+]], [[IS_INL_1]]{{.*}}
; CHECK: [[Y_A2_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_A2_MAIN_CS]]}
; CHECK: [[Z_Y_A2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[B_FIR]] = distinct !{!"intel.function.inlining.report", [[B_NAME]], null{{.*}}
; CHECK: [[A_FIR]] = distinct !{!"intel.function.inlining.report", [[A_NAME]], [[A_CSs:![0-9]+]]{{.*}}
; CHECK: [[A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_A_CS]], [[Y_A_CS:![0-9]+]]}
; CHECK: [[X_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[Y_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME]], [[Y_A_CSs:![0-9]+]], [[IS_INL_1]]{{.*}}
; CHECK: [[Y_A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_A_CS]]}
; CHECK: [[Z_Y_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[X_FIR]] = distinct !{!"intel.function.inlining.report", [[X_NAME]], null{{.*}}
; CHECK: [[Y_FIR]] = distinct !{!"intel.function.inlining.report", [[Y_NAME]], [[Y_CSs:![0-9]+]]{{.*}}
; CHECK: [[Y_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_Y_CS:![0-9]+]]}
; CHECK: [[Z_Y_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME]], null, [[IS_INL_0]]{{.*}}
; CHECK: [[Z_FIR]] = distinct !{!"intel.function.inlining.report", [[Z_NAME]], null{{.*}}


;IR with call site inlining reports unlinked from call instructions.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() local_unnamed_addr !intel.function.inlining.report !2 {
entry:
  tail call void (...) @x()
  tail call void (...) @z()
  tail call void (...) @b()
  tail call void (...) @x()
  tail call void (...) @z()
  ret i32 0
}

declare !intel.function.inlining.report !40 dso_local void @b(...) local_unnamed_addr

define dso_local void @a() local_unnamed_addr !intel.function.inlining.report !42 {
entry:
  tail call void (...) @x()
  tail call void (...) @z()
  ret void
}

declare !intel.function.inlining.report !48 dso_local void @x(...) local_unnamed_addr

declare !intel.function.inlining.report !55 dso_local void @z(...) local_unnamed_addr

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!intel.module.inlining.report = !{!2, !40, !42, !48, !49, !55}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = distinct !{!"intel.function.inlining.report", !3, !4, !17, !37, !38, !39}
!3 = !{!"name: main"}
!4 = distinct !{!"intel.callsites.inlining.report", !5, !29, !31}
!5 = distinct !{!"intel.callsite.inlining.report", !6, !7, !23, !27, !28, !13, !26, !15, !16, !"line: 0 col: 0", !17}
!6 = !{!"name: a"}
!7 = distinct !{!"intel.callsites.inlining.report", !8, !18}
!8 = distinct !{!"intel.callsite.inlining.report", !9, null, !10, !11, !12, !13, !14, !15, !16, !"line: 0 col: 0", !17}
!9 = !{!"name: x"}
!10 = !{!"isInlined: 0"}
!11 = !{!"reason: 33"}
!12 = !{!"inlineCost: -1"}
!13 = !{!"outerInlineCost: -1"}
!14 = !{!"inlineThreshold: -1"}
!15 = !{!"earlyExitCost: 2147483647"}
!16 = !{!"earlyExitThreshold: 2147483647"}
!17 = !{!"moduleName: test1.c"}
!18 = distinct !{!"intel.callsite.inlining.report", !19, !20, !23, !24, !25, !13, !26, !15, !16, !"line: 0 col: 0", !17}
!19 = !{!"name: y"}
!20 = distinct !{!"intel.callsites.inlining.report", !21}
!21 = distinct !{!"intel.callsite.inlining.report", !22, null, !10, !11, !12, !13, !14, !15, !16, !"line: 0 col: 0", !17}
!22 = !{!"name: z"}
!23 = !{!"isInlined: 1"}
!24 = !{!"reason: 8"}
!25 = !{!"inlineCost: -15000"}
!26 = !{!"inlineThreshold: 337"}
!27 = !{!"reason: 9"}
!28 = !{!"inlineCost: 30"}
!29 = distinct !{!"intel.callsite.inlining.report", !30, null, !10, !11, !12, !13, !14, !15, !16, !"line: 0 col: 0", !17}
!30 = !{!"name: b"}
!31 = distinct !{!"intel.callsite.inlining.report", !6, !32, !23, !27, !28, !13, !26, !15, !16, !"line: 0 col: 0", !17}
!32 = distinct !{!"intel.callsites.inlining.report", !33, !34}
!33 = distinct !{!"intel.callsite.inlining.report", !9, null, !10, !11, !12, !13, !14, !15, !16, !"line: 0 col: 0", !17}
!34 = distinct !{!"intel.callsite.inlining.report", !19, !35, !23, !24, !25, !13, !26, !15, !16, !"line: 0 col: 0", !17}
!35 = distinct !{!"intel.callsites.inlining.report", !36}
!36 = distinct !{!"intel.callsite.inlining.report", !22, null, !10, !11, !12, !13, !14, !15, !16, !"line: 0 col: 0", !17}
!37 = !{!"isDead: 0"}
!38 = !{!"isDeclaration: 0"}
!39 = !{!"linkage: A"}
!40 = distinct !{!"intel.function.inlining.report", !30, null, !17, !37, !41, !39}
!41 = !{!"isDeclaration: 1"}
!42 = distinct !{!"intel.function.inlining.report", !6, !43, !17, !37, !38, !39}
!43 = distinct !{!"intel.callsites.inlining.report", !44, !45}
!44 = distinct !{!"intel.callsite.inlining.report", !9, null, !10, !11, !12, !13, !14, !15, !16, !"line: 0 col: 0", !17}
!45 = distinct !{!"intel.callsite.inlining.report", !19, !46, !23, !24, !25, !13, !26, !15, !16, !"line: 0 col: 0", !17}
!46 = distinct !{!"intel.callsites.inlining.report", !47}
!47 = distinct !{!"intel.callsite.inlining.report", !22, null, !10, !11, !12, !13, !14, !15, !16, !"line: 0 col: 0", !17}
!48 = distinct !{!"intel.function.inlining.report", !9, null, !17, !37, !41, !39}
!49 = distinct !{!"intel.function.inlining.report", !19, !50, !17, !53, !38, !54}
!50 = distinct !{!"intel.callsites.inlining.report", !51}
!51 = distinct !{!"intel.callsite.inlining.report", !22, null, !10, !52, !12, !13, !14, !15, !16, !"line: 0 col: 0", !17}
!52 = !{!"reason: 28"}
!53 = !{!"isDead: 1"}
!54 = !{!"linkage: L"}
!55 = distinct !{!"intel.function.inlining.report", !22, null, !17, !37, !41, !39}

