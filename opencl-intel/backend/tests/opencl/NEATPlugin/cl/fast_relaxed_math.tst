; RUN_xx: SATest -OCL -VAL --force_ref --neat -config=%s.cfg | FileCheck %s
; see CSSD100018906
; RUN: echo "Hello world!"
; CHECK: Test Passed