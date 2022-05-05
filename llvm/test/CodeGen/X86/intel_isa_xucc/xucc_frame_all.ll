; REQUIRES: intel_feature_xucc
; This test shouldn't crash when the '-frame-pointer' set to 'all'
; RUN: llc < %s -mtriple=x86_64_xucc-unknown-unknown -frame-pointer=all 2>&1 | FileCheck %s
%struct.XuVMCS_t.1.3.5.7.9 = type { %union.anon.0.2.4.6.8 }
%union.anon.0.2.4.6.8 = type { [2048 x i8] }

; CHECK-LABEL: gs_is_guest_in_probe_mode
define i1 @gs_is_guest_in_probe_mode(%struct.XuVMCS_t.1.3.5.7.9* %working_vmcs) {
entry:
  ret i1 false
}
