// REQUIRES: intel_feature_xucc
// RUN: %clang_cc1 %s -O2 -ffreestanding -triple=x86_64_xucc-unknown-unknown -S -o - | FileCheck %s

typedef struct {
  int base;
} loadstoreseg_param_t;

#include <pppe_intrin.h>

void test_int1(void) {
  // CHECK-LABEL: test_int1
  // CHECK:      #APP
  // CHECK-NEXT: int1
  // CHECK-NEXT: #NO_APP
  _int1();
}

void test_iret(void) {
  // CHECK-LABEL: test_iret
  // CHECK:      #APP
  // CHECK-NEXT: iret
  // CHECK-NEXT: #NO_APP
  _iret();
}

__int64 test_spbusmsg_read(__int64 msg) {
  // CHECK-LABEL: test_spbusmsg_read
  // CHECK:      #APP
  // CHECK-NEXT: spbusmsg %rdi, %rax
  // CHECK-NEXT: #NO_APP
  return _spbusmsg_read(msg);
}

void test_spbusmsg_write(__int64 msg, __int64 data) {
  // CHECK-LABEL: test_spbusmsg_write
  // CHECK:      #APP
  // CHECK-NEXT: spbusmsg %rsi, %rdi
  // CHECK-NEXT: #NO_APP
  _spbusmsg_write(msg, data);
}

void test_spbusmsg_nodata(__int64 msg) {
  // CHECK-LABEL: test_spbusmsg_nodata
  // CHECK:      #APP
  // CHECK-NEXT: spbusmsg %rdi, %rdi
  // CHECK-NEXT: #NO_APP
  _spbusmsg_nodata(msg);
}

__int64 test_guestloadlin(void *p) {
  // CHECK-LABEL: test_guestloadlin
  // CHECK:      #APP
  // CHECK-NEXT: gmovlin (%rdi), %rax
  // CHECK-NEXT: #NO_APP
  return _guestloadlin(p);
}

/*
void test_gueststorelin(void *p, __int64 data) {
  // HECK-LABEL: test_gueststorelin
  // HECK:      #APP
  // HECK-NEXT: gmovlin %rsi, (%rdi)
  // HECK-NEXT: NO_APP
  _gueststorelin(p, data);
}
*/

__int64 test_guestloadpphys(void *p) {
  // CHECK-LABEL: test_guestloadpphys
  // CHECK:      #APP
  // CHECK-NEXT: gmovpphys (%rdi), %rax
  // CHECK-NEXT: #NO_APP
  return _guestloadpphys(p);
}

void test_gueststorepphys(void *p, __int64 data) {
  // CHECK-LABEL: test_gueststorepphys
  // CHECK:      #APP
  // CHECK-NEXT: gmovpphys %rsi, (%rdi)
  // CHECK-NEXT: #NO_APP
  _gueststorepphys(p, data);
}

/*
__int64 test_guestloadgphys(void *p) {
  // HECK-LABEL: test_guestloadgphys
  // HECK:      #APP
  // HECK-NEXT: gmovgphys (%rdi), %rax
  // HECK-NEXT: NO_APP
  return _guestloadgphys(p);
}

void test_gueststoregphys(void *p, __int64 data) {
  // HECK-LABEL: test_gueststoregphys
  // HECK:      #APP
  // HECK-NEXT: gmovgphys %rsi, (%rdi)
  // HECK-NEXT: NO_APP
  _gueststoregphys(p, data);
}
*/

void test_saveworld(void *p, __int64 mask) {
  // CHECK-LABEL: test_saveworld
  // CHECK:      #APP
  // CHECK-NEXT: svworld %rsi, (%rdi)
  // CHECK-NEXT: #NO_APP
  _saveworld(p, mask);
}

void test_restoreworld(const void *p, __int64 mask) {
  // CHECK-LABEL: test_restoreworld
  // CHECK:      #APP
  // CHECK-NEXT: rsworld %rsi, (%rdi)
  // CHECK-NEXT: #NO_APP
  _restoreworld(p, mask);
}

__int64 test_gtranslate_rd_noepc(__int64 physaddr, __int64 linaddr, void *mem) {
  // CHECK-LABEL: test_gtranslate_rd_noepc
  // CHECK:      #APP
  // CHECK-NEXT: gtranslaterd_noepc (%rdx)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movq    %rcx, %rax
  return _gtranslate_rd_noepc(physaddr, linaddr, mem);
}

__int64 test_gtranslate_wr_noepc(__int64 physaddr, __int64 linaddr, void *mem) {
  // CHECK-LABEL: test_gtranslate_wr_noepc
  // CHECK:      #APP
  // CHECK-NEXT: gtranslatewr_noepc (%rdx)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movq    %rcx, %rax
  return _gtranslate_wr_noepc(physaddr, linaddr, mem);
}

 __int64 test_gtranslate_rd_epc(__int64 physaddr, __int64 linaddr, void *mem) {
  // CHECK-LABEL: test_gtranslate_rd_epc
  // CHECK:      #APP
  // CHECK-NEXT: gtranslaterd_epc (%rdx)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movq    %rcx, %rax
  return _gtranslate_rd_epc(physaddr, linaddr, mem);
 }

__int64 test_gtranslate_wr_epc(__int64 physaddr, __int64 linaddr, void *mem) {
  // CHECK-LABEL: test_gtranslate_wr_epc
  // CHECK:      #APP
  // CHECK-NEXT: gtranslatewr_epc (%rdx)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movq    %rcx, %rax
  return _gtranslate_wr_epc(physaddr, linaddr, mem);
}

char test_getbasekey(__int64 basekey_hi, __int64 basekey_lo, int svn) {
  // CHECK-LABEL: test_getbasekey
  // CHECK:      movslq  %edx, %rax
  // CHECK-NEXT: #APP
  // CHECK-NEXT: getbasekey  %rax
  // CHECK-NEXT: sete %al
  // CHECK-NEXT: #NO_APP
  return _getbasekey(basekey_hi, basekey_lo, svn);
}

 __int64 test_cmodeselect(__int64 false_val, __int64 true_val, __int64 cond) {
  // CHECK-LABEL: test_cmodeselect
  // CHECK:      movq    %rdx, %rcx
  // CHECK-NEXT: movq    %rdi, %rax
  // CHECK-NEXT: #APP
  // CHECK-NEXT: cmodemov %rcx, %rsi, %rax
  // CHECK-NEXT: #NO_APP
  return _cmodeselect(false_val, true_val, cond);
}

void test_loadseg_cs(loadstoreseg_param_t seginfo) {
  // CHECK-LABEL: test_loadseg_cs
  // CHECK:      #APP
  // CHECK-NEXT: loadseg %cs:(%edi)
  // CHECK-NEXT: #NO_APP
  _loadseg_cs(seginfo);
}

loadstoreseg_param_t test_storeseg_cs(void) {
  // CHECK-LABEL: test_storeseg_cs
  // CHECK:      #APP
  // CHECK-NEXT: storeseg %cs:-8(%rsp)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movl    -8(%rsp), %eax
  return _storeseg_cs();
}

void test_loadseg_ds(loadstoreseg_param_t seginfo) {
  // CHECK-LABEL: test_loadseg_ds
  // CHECK:      #APP
  // CHECK-NEXT: loadseg %ds:(%edi)
  // CHECK-NEXT: #NO_APP
  _loadseg_ds(seginfo);
}

loadstoreseg_param_t test_storeseg_ds(void) {
  // CHECK-LABEL: test_storeseg_ds
  // CHECK:      #APP
  // CHECK-NEXT: storeseg %ds:-8(%rsp)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movl    -8(%rsp), %eax
  return _storeseg_ds();
}

void test_loadseg_es(loadstoreseg_param_t seginfo) {
  // CHECK-LABEL: test_loadseg_es
  // CHECK:      #APP
  // CHECK-NEXT: loadseg %es:(%edi)
  // CHECK-NEXT: #NO_APP
  _loadseg_es(seginfo);
}

loadstoreseg_param_t test_storeseg_es(void) {
  // CHECK-LABEL: test_storeseg_es
  // CHECK:      #APP
  // CHECK-NEXT: storeseg %es:-8(%rsp)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movl    -8(%rsp), %eax
  return _storeseg_es();
}

void test_loadseg_fs(loadstoreseg_param_t seginfo) {
  // CHECK-LABEL: test_loadseg_fs
  // CHECK:      #APP
  // CHECK-NEXT: loadseg %fs:(%edi)
  // CHECK-NEXT: #NO_APP
  _loadseg_fs(seginfo);
}

loadstoreseg_param_t test_storeseg_fs(void) {
  // CHECK-LABEL: test_storeseg_fs
  // CHECK:      #APP
  // CHECK-NEXT: storeseg %fs:-8(%rsp)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movl    -8(%rsp), %eax
  return _storeseg_fs();
}

void test_loadseg_gs(loadstoreseg_param_t seginfo) {
  // CHECK-LABEL: test_loadseg_gs
  // CHECK:      #APP
  // CHECK-NEXT: loadseg %gs:(%edi)
  // CHECK-NEXT: #NO_APP
  _loadseg_gs(seginfo);
}

loadstoreseg_param_t test_storeseg_gs(void) {
  // CHECK-LABEL: test_storeseg_gs
  // CHECK:      #APP
  // CHECK-NEXT: storeseg %gs:-8(%rsp)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movl    -8(%rsp), %eax
  return _storeseg_gs();
}

void test_loadseg_ss(loadstoreseg_param_t seginfo) {
  // CHECK-LABEL: test_loadseg_ss
  // CHECK:      #APP
  // CHECK-NEXT: loadseg %ss:(%edi)
  // CHECK-NEXT: #NO_APP
  _loadseg_ss(seginfo);
}

loadstoreseg_param_t test_storeseg_ss(void) {
  // CHECK-LABEL: test_storeseg_ss
  // CHECK:      #APP
  // CHECK-NEXT: storeseg %ss:-8(%rsp)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movl    -8(%rsp), %eax
  return _storeseg_ss();
}

void test_set_ldtr(unsigned short sel) {
  // CHECK-LABEL: test_set_ldtr
  // CHECK:      #APP
  // CHECK-NEXT: lldtw %di
  // CHECK-NEXT: #NO_APP
  _set_ldtr(sel);
}

unsigned short test_get_ldtr(void) {
  // CHECK-LABEL: test_get_ldtr
  // CHECK:      #APP
  // CHECK-NEXT: sldtw -2(%rsp)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movzwl  -2(%rsp), %eax
  return _get_ldtr();
}

void test_set_tr(unsigned short sel) {
  // CHECK-LABEL: test_set_tr
  // CHECK:      #APP
  // CHECK-NEXT: ltrw %di
  // CHECK-NEXT: #NO_APP
  _set_tr(sel);
}

unsigned short test_get_tr(void) {
  // CHECK-LABEL: test_get_tr
  // CHECK:      #APP
  // CHECK-NEXT: strw -2(%rsp)
  // CHECK-NEXT: #NO_APP
  // CHECK-NEXT: movzwl  -2(%rsp), %eax
  return _get_tr();
}

void test_asidswitch_tlbflush(__int64 flush_type, void *p) {
  // CHECK-LABEL: test_asidswitch_tlbflush
  // CHECK:      #APP
  // CHECK-NEXT: asidswitch_tlbflush %rsi, %rdi
  // CHECK-NEXT: #NO_APP
  _asidswitch_tlbflush(flush_type, p);
}
