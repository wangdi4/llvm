; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0x80 < %s -S | opt -passes='cgscc(inline)' -inline-report=0x80 -S 2>&1 | FileCheck %s

; This test checks that metadata corresponding to the inlining report was
; updated in accordance with function inlining that happened. a() should
; be inlined in main() twice, resulting in z() call sites in the code.
; b() should be inlined in main() resulting in the x() callsites in the code,
; y() call site from b() should be optimized out by inliner and corresponding
; IR node in the main() inline report tree should be marked deleted.

; CHECK: define dso_local void @main() local_unnamed_addr !intel.function.inlining.report [[MAIN_FIR:![0-9]+]] {
; CHECK-NEXT: entry:
; CHECK-NEXT:   call void (...) @z(), !intel.callsite.inlining.report [[Z_A1_MAIN_CS:![0-9]+]]
; CHECK-NEXT:   call void (...) @x(), !intel.callsite.inlining.report [[X_B_MAIN_CS:![0-9]+]]
; CHECK-NEXT:   call void (...) @z(), !intel.callsite.inlining.report [[Z_A2_MAIN_CS:![0-9]+]]
; CHECK-NEXT:   ret void

; CHECK: !intel.module.inlining.report = !{[[A_FIR:![0-9]+]], [[Z_FIR:![0-9]+]], [[B_FIR:![0-9]+]], [[X_FIR:![0-9]+]], [[Y_FIR:![0-9]+]], [[MAIN_FIR]]}
; CHECK: [[A_FIR]] = distinct !{!"intel.function.inlining.report", [[A_NAME:![0-9]+]], [[A_CSs:![0-9]+]], [[MODULE_NAME:![0-9]+]], [[IS_DEAD_0:![0-9]+]], [[IS_DECL_0:![0-9]+]], [[LINK_A:![0-9]+]], [[LANG_C:![0-9]+]], [[SUPPRESS_PRINT:![0-9]+]]}
; CHECK-NEXT: [[A_NAME]] = !{!"name: a"}
; CHECK-NEXT: [[A_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_A_CS:![0-9]+]]}
; CHECK-NEXT: [[Z_A_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME:![0-9]+]], null, [[IS_INL_0:![0-9]+]], [[REASON_EXTRN:![0-9]+]]{{.*}}, [[SUPPRESS_PRINT]]
; CHECK-NEXT: [[Z_NAME]] = !{!"name: z"}
; CHECK-NEXT: [[IS_INL_0]] = !{!"isInlined: 0"}
; CHECK-NEXT: [[REASON_EXTRN]] = !{!"reason:{{.*}}
; CHECK-NEXT: {{.*}}inlineCost
; CHECK-NEXT: {{.*}}outerInlineCost
; CHECK-NEXT: {{.*}}inlineThreshold
; CHECK-NEXT: {{.*}}earlyExitCost
; CHECK-NEXT: {{.*}}earlyExitThreshold
; CHECK-NEXT: [[MODULE_NAME]] = !{!"moduleName:{{.*}}
; CHECK-NEXT: [[SUPPRESS_PRINT]] = !{!"isSuppressPrint: 0"}
; CHECK-NEXT: [[IS_DEAD_0]] = !{!"isDead: 0"}
; CHECK-NEXT: [[IS_DECL_0]] = !{!"isDeclaration: 0"}
; CHECK-NEXT: [[LINK_A]] = !{!"linkage: A"}
; CHECK-NEXT: [[LANG_C]] = !{!"language: C"}
; CHECK-NEXT: [[Z_FIR]] = distinct !{!"intel.function.inlining.report", [[Z_NAME]], null, [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_1:![0-9]+]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[IS_DECL_1]] = !{!"isDeclaration: 1"}
; CHECK-NEXT: [[B_FIR]] = distinct !{!"intel.function.inlining.report", [[B_NAME:![0-9]+]], [[B_CSs:![0-9]+]], [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_0]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[B_NAME]] = !{!"name: b"}
; CHECK-NEXT: [[B_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_B_CS:![0-9]+]], [[Y_B_CS:![0-9]+]]}
; CHECK-NEXT: [[X_B_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME:![0-9]+]], null, [[IS_INL_0]], [[REASON_EXTRN]]{{.*}}, [[SUPPRESS_PRINT]]
; CHECK-NEXT: [[X_NAME]] = !{!"name: x"}
; CHECK-NEXT: [[Y_B_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME:![0-9]+]], null, [[IS_INL_0]], [[REASON_EXTRN]]{{.*}}, [[SUPPRESS_PRINT]]
; CHECK-NEXT: [[Y_NAME]] = !{!"name: y"}
; CHECK-NEXT: [[X_FIR]] = distinct !{!"intel.function.inlining.report", [[X_NAME]], null, [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_1]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[Y_FIR]] = distinct !{!"intel.function.inlining.report", [[Y_NAME]], null, [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_1]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[MAIN_FIR]] = distinct !{!"intel.function.inlining.report", [[MAIN_NAME:![0-9]+]], [[MAIN_CSs:![0-9]+]], [[MODULE_NAME]], [[IS_DEAD_0]], [[IS_DECL_0]], [[LINK_A]], [[LANG_C]], [[SUPPRESS_PRINT]]}
; CHECK-NEXT: [[MAIN_NAME]] = !{!"name: main"}
; CHECK-NEXT: [[MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[A1_MAIN_CS:![0-9]+]], [[B_MAIN_CS:![0-9]+]], [[A2_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[A1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME]], [[A1_MAIN_CSs:![0-9]+]], [[IS_INL_1:![0-9]+]], [[REASON:![0-9]+]]{{.*}}
; CHECK-NEXT: [[A1_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_A1_MAIN_CS]]}
; CHECK-NEXT: [[Z_A1_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME]], null, [[IS_INL_0]], [[REASON_EXTRN]]{{.*}}
; CHECK-NEXT: [[IS_INL_1]] = !{!"isInlined: 1"}
; CHECK-NEXT: [[REASON]] = !{!"reason{{.*}}
; CHECK-NEXT: {{.*}}inlineCost
; CHECK-NEXT: {{.*}}inlineThreshold
; CHECK-NEXT: [[B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[B_NAME]], [[B_MAIN_CSs:![0-9]+]], [[IS_INL_1]], [[REASON]]{{.*}}
; CHECK-NEXT: [[B_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[X_B_MAIN_CS]], [[Y_B_MAIN_CS:![0-9]+]]}
; CHECK-NEXT: [[X_B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[X_NAME]], null, [[IS_INL_0]], [[REASON_EXTRN]]{{.*}}
; CHECK-NEXT: [[Y_B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Y_NAME]], null, [[IS_INL_0]], [[REASON_DELETED:![0-9]+]]{{.*}}
; CHECK-NEXT: [[REASON_DELETED]] = !{!"reason:{{.*}}
; CHECK-NEXT: {{.*}}inlineCost
; CHECK-NEXT: [[A2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME]], [[A2_MAIN_CSs:![0-9]+]], [[IS_INL_1]], [[REASON]]{{.*}}
; CHECK-NEXT: [[A2_MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[Z_A2_MAIN_CS]]}
; CHECK-NEXT: [[Z_A2_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[Z_NAME]], null, [[IS_INL_0]], [[REASON_EXTRN]]{{.*}}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @a() local_unnamed_addr {
entry:
  call void (...) @z()
  ret void
}

declare dso_local void @z(...) local_unnamed_addr

define dso_local void @b(i32 %n) local_unnamed_addr {
entry:
  %cmp = icmp slt i32 %n, 3
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  call void (...) @x()
  br label %if.end

if.else:                                          ; preds = %entry
  call void (...) @y()
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare dso_local void @x(...) local_unnamed_addr

declare dso_local void @y(...) local_unnamed_addr

define dso_local void @main() local_unnamed_addr {
entry:
  call void @a()
  call void @b(i32 2)
  call void @a()
  ret void
}

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
