; RUN: SATest -BUILD -dump-time-passes=- -config=%s.cfg 2>&1 | FileCheck %s

; CHECK-DAG: SimplifyCFGPass
; CHECK-DAG: vpo::VPlanDriverPass ;INTEL
; CHECK-DAG: DCEPass
