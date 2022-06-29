; RUN: SATest -BUILD -dump-kernel-property -config=%s.cfg 2>&1 | FileCheck %s

; Checks materialized subgroup size is a valid subgroup size (one of CPU_DEV_SUB_GROUP_SIZES).
; Note: `1` is not a valid subgroup size.

; CHECK-DAG: vectorizationWidth: {{1$}}
; CHECK-DAG: Materialized subgroup size: {{4|8|16|32|64}}
; CHECK-DAG: Test program was successfully built.
