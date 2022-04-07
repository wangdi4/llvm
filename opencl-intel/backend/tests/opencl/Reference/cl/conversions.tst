; RUN: SATest -VAL --force_ref -config=%s.char2int.cfg | FileCheck %s
; RUN: SATest -VAL --force_ref -config=%s.char2int.sat.cfg | FileCheck %s
; RUN: SATest -VAL --force_ref -config=%s.char2int.rte.cfg | FileCheck %s
; RUN: SATest -VAL --force_ref -config=%s.char2int.sat_rte.cfg | FileCheck %s
; RUN: SATest -VAL --force_ref -config=%s.flt2char.cfg | FileCheck %s
; RUN: SATest -VAL --force_ref -config=%s.flt2char.sat.cfg | FileCheck %s
; RUN: SATest -VAL --force_ref -config=%s.flt2char.rte.cfg | FileCheck %s
; RUN: SATest -VAL --force_ref -config=%s.flt2char.sat_rte.cfg | FileCheck %s
; CHECK: Test Passed.
