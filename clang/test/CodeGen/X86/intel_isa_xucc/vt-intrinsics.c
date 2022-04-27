// REQUIRES: intel_feature_xucc
// RUN: %clang_cc1 %s -O2 -ffreestanding -triple=x86_64_xucc-unknown-unknown -S -o - | FileCheck %s

#include <vt_intrin.h>

void test_vmxon(void *vmxonp) {
  // CHECK-LABEL: test_vmxon
  // CHECK:      movq    %rdi, -8(%rsp)
  // CHECK-NEXT: #APP
  // CHECK-NEXT: vmxon   -8(%rsp)
  // CHECK-NEXT: #NO_APP
  _vmxon(vmxonp);
}

void test_vmxoff(void) {
  // CHECK-LABEL: test_vmxoff
  // CHECK:      #APP
  // CHECK-NEXT: vmxoff
  // CHECK-NEXT: #NO_APP
  _vmxoff();
}

void test_vmcall(void) {
  // CHECK-LABEL: test_vmcall
  // CHECK:      #APP
  // CHECK-NEXT: vmcall
  // CHECK-NEXT: #NO_APP
  _vmcall();
}

void test_vmlaunch(void) {
  // CHECK-LABEL: test_vmlaunch
  // CHECK:      #APP
  // CHECK-NEXT: vmlaunch
  // CHECK-NEXT: #NO_APP
  _vmlaunch();
}

void test_vmresume(void) {
  // CHECK-LABEL: test_vmresume
  // CHECK:      #APP
  // CHECK-NEXT: vmresume
  // CHECK-NEXT: #NO_APP
  _vmresume();
}

void test_vmclear(void *vmcsp) {
  // CHECK-LABEL: test_vmclear
  // CHECK:      movq    %rdi, -8(%rsp)
  // CHECK-NEXT: #APP
  // CHECK-NEXT: vmclear   -8(%rsp)
  // CHECK-NEXT: #NO_APP
  _vmclear(vmcsp);
}

void test_vmptrld(void *vmcsp) {
  // CHECK-LABEL: test_vmptrld
  // CHECK:      movq    %rdi, -8(%rsp)
  // CHECK-NEXT: #APP
  // CHECK-NEXT: vmptrld   -8(%rsp)
  // CHECK-NEXT: #NO_APP
  _vmptrld(vmcsp);
}

void* test_vmptrst(void) {
  // CHECK-LABEL: test_vmptrst
  // CHECK:      #APP
  // CHECK-NEXT: vmptrst -8(%rsp)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movq    -8(%rsp), %rax
  return _vmptrst();
}

void test_vmwrite(__int64 field_id, __int64 value) {
  // CHECK-LABEL: test_vmwrite
  // CHECK:      #APP
  // CHECK-NEXT: vmwriteq        %rsi, %rax
  // CHECK-NEXT: #NO_APP
  _vmwrite(field_id, value);
}

__int64 test_vmread(__int64 field_id) {
  // CHECK-LABEL: test_vmread
  // CHECK:      #APP
  // CHECK-NEXT: vmreadq %rdi, %rax
  // CHECK-NEXT: #NO_APP
  return _vmread(field_id);
}

void test_invept(void* descrp, __int64 type) {
  // CHECK-LABEL: test_invept
  // CHECK:      #APP
  // CHECK-NEXT: invept  (%rdi), %rsi
  // CHECK-NEXT: #NO_APP
  _invept(descrp, type);
}

void test_invvpid(void* descrp, __int64 type) {
  // CHECK-LABEL: test_invvpid
  // CHECK:      #APP
  // CHECK-NEXT: invvpid (%rdi), %rsi
  // CHECK-NEXT: #NO_APP
  _invvpid(descrp, type);
}
