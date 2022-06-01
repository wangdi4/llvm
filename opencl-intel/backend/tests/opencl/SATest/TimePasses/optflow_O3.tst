; RUN: SATest -BUILD -dump-time-passes=- -config=%s.cfg | FileCheck %s

; CHECK-DAG: SimplifyCFGPass
; CHECK-DAG: vpo::VPlanDriverPass
; CHECK-DAG: DCEPass
; CHECK-DAG: WGLoopBoundariesPass
