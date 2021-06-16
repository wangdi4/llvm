// REQUIRES: intel_feature_icecode
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_icecode-unknown-unknown -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu | FileCheck %s

#include <immintrin.h>

#define MY_CONSTANT_VAL 35

extern void* mem;

void test_xucode() {
// CHECK: call i8 @llvm.x86.icecode.loadpphys.8(i64 %{{.*}})
// CHECK: call i16 @llvm.x86.icecode.loadpphys.16(i64 %{{.*}})
// CHECK: call i32 @llvm.x86.icecode.loadpphys.32(i64 %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.loadpphys.64(i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.storepphys.8(i8 %{{.*}}, i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.storepphys.16(i16 %{{.*}}, i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.storepphys.32(i32 %{{.*}}, i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.storepphys.64(i64 %{{.*}}, i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.loadseg(i8* %{{.*}}, i32 1)
// CHECK: call void @llvm.x86.icecode.loadseg(i8* %{{.*}}, i32 2)
// CHECK: call void @llvm.x86.icecode.loadseg(i8* %{{.*}}, i32 3)
// CHECK: call void @llvm.x86.icecode.loadseg(i8* %{{.*}}, i32 4)
// CHECK: call void @llvm.x86.icecode.loadseg(i8* %{{.*}}, i32 5)
// CHECK: call void @llvm.x86.icecode.loadseg(i8* %{{.*}}, i32 6)
// CHECK: call void @llvm.x86.icecode.storeseg(i8* %{{.*}}, i32 1)
// CHECK: call void @llvm.x86.icecode.storeseg(i8* %{{.*}}, i32 2)
// CHECK: call void @llvm.x86.icecode.storeseg(i8* %{{.*}}, i32 3)
// CHECK: call void @llvm.x86.icecode.storeseg(i8* %{{.*}}, i32 4)
// CHECK: call void @llvm.x86.icecode.storeseg(i8* %{{.*}}, i32 5)
// CHECK: call void @llvm.x86.icecode.storeseg(i8* %{{.*}}, i32 6)
// CHECK: call { i64, i64, i64, i64 } asm sideeffect "gtranslaterd_epc
// CHECK: call { i64, i64, i64, i64 } asm sideeffect "gtranslatewr_epc
// CHECK: call { i64, i64, i64, i64 } asm sideeffect "gtranslaterd_noepc
// CHECK: call { i64, i64, i64, i64 } asm sideeffect "gtranslatewr_noepc
// CHECK: call void asm sideeffect "bcast_seg_state"
  unsigned char reg8;
  unsigned short reg16;
  unsigned int reg32;
  unsigned long long reg64 = 0, info[4];
  reg8 = _ce_loadpphys8(reg64);
  reg16 = _ce_loadpphys16(reg64);
  reg32 = _ce_loadpphys32(reg64);
  reg64 = _ce_loadpphys64(reg64);
  _ce_storepphys8(reg8, reg64);
  _ce_storepphys16(reg16, reg64);
  _ce_storepphys32(reg32, reg64);
  _ce_storepphys64(reg64, reg64);
  _ce_loadseg_cs(mem);
  _ce_loadseg_ds(mem);
  _ce_loadseg_ss(mem);
  _ce_loadseg_es(mem);
  _ce_loadseg_fs(mem);
  _ce_loadseg_gs(mem);
  _ce_storeseg_cs(mem);
  _ce_storeseg_ds(mem);
  _ce_storeseg_ss(mem);
  _ce_storeseg_es(mem);
  _ce_storeseg_fs(mem);
  _ce_storeseg_gs(mem);
  _ce_gtranslaterd_epc(mem, info);
  _ce_gtranslatewr_epc(mem, info);
  _ce_gtranslaterd_noepc(mem, info);
  _ce_gtranslatewr_noepc(mem, info);
  _ce_bcast_seg_state();
}

void test_icecode() {
  unsigned reg = 0;
  unsigned char data8 = 0;
  unsigned short data16 = 0;
  unsigned int data32 = 0;
  unsigned long long data64 = 0;
// CHECK: call i32 @llvm.x86.icecode.creg.xchg.32(i32 %{{.*}}, i32 %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.creg.xchg.64(i32 %{{.*}}, i64 %{{.*}})
// CHECK: call i32 @llvm.x86.icecode.fscp.xchg.32(i32 %{{.*}}, i32 %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.fscp.xchg.64(i32 %{{.*}}, i64 %{{.*}})
// CHECK: call i32 @llvm.x86.icecode.creg.read.32(i32 %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.creg.read.64(i32 %{{.*}})
// CHECK: call i32 @llvm.x86.icecode.fscp.read.32(i32 %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.fscp.read.64(i32 %{{.*}})
// CHECK: call i32 @llvm.x86.icecode.creg.xchg.mt.32(i32 %{{.*}}, i32 %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.creg.xchg.mt.64(i32 %{{.*}}, i64 %{{.*}})
// CHECK: call i32 @llvm.x86.icecode.creg.read.mt.32(i32 %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.creg.read.mt.64(i32 %{{.*}})
// CHECK: call i8 @llvm.x86.icecode.portin.8(i64 %{{.*}})
// CHECK: call i16 @llvm.x86.icecode.portin.16(i64 %{{.*}})
// CHECK: call i32 @llvm.x86.icecode.portin.32(i64 %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.portin.64(i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.portout.8(i8 %{{.*}}, i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.portout.16(i16 %{{.*}}, i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.portout.32(i32 %{{.*}}, i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.portout.64(i64 %{{.*}}, i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.sta.special(i64 %{{.*}})
// CHECK: call i32 @llvm.x86.icecode.nr.read(i32 0)
// CHECK: call void @llvm.x86.icecode.ucodecall(i32 %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.cmodemov(i64 %{{.*}}, i64 %{{.*}}, i32 0)
// CHECK: call void asm sideeffect "sigeventjump $0,
// CHECK: call void asm sideeffect "sserialize"
// CHECK: call void asm sideeffect "nop_set_sb"
// CHECK: call void asm sideeffect "nop_read_sb"
// CHECK: call void asm sideeffect "get_excl_acc"
// CHECK: call void asm sideeffect "release_excl_acc"
// CHECK: call void asm sideeffect "virt_nuke_point"
// CHECK: call void asm sideeffect "int_trap_point"
// CHECK: call void asm sideeffect "iceret_indirect
// CHECK: call void @llvm.x86.icecode.set.tracker(i32 0)
// CHECK: call i32 asm sideeffect "portin24", "={ax},{dx},~{dirflag},~{fpsr},~{flags}"(i64 %{{.*}})
// CHECK: call void asm sideeffect "portout24", "{ax},{dx},~{dirflag},~{fpsr},~{flags}"(i32 %{{.*}}, i64 %{{.*}})
// CHECK: call void asm sideeffect "load_tickle_gpa", "{ax},~{dirflag},~{fpsr},~{flags}"(i64 %{{.*}})
// CHECK: call void asm sideeffect "store_tickle_gpa", "{ax},~{dirflag},~{fpsr},~{flags}"(i64 %{{.*}})
// CHECK: call void asm sideeffect "flush_ifu"
// CHECK: call i16 @llvm.x86.icecode.loadlin.16(i64 %{{.*}})
// CHECK: call i32 @llvm.x86.icecode.loadlin.32(i64 %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.loadlin.64(i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.storelin.16(i16 %{{.*}}, i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.storelin.32(i32 %{{.*}}, i64 %{{.*}})
// CHECK: call void @llvm.x86.icecode.storelin.64(i64 %{{.*}}, i64 %{{.*}})
// CHECK: call i64 asm sideeffect "cccm $0", "=r,0,~{dirflag},~{fpsr},~{flags}"(i64 %{{.*}})
// CHECK: call i64 asm sideeffect "cccp $0", "=r,0,~{dirflag},~{fpsr},~{flags}"(i64 %{{.*}})
// CHECK: call void asm sideeffect "jmp_nopred $0", "r,~{dirflag},~{fpsr},~{flags}"(i64 %{{.*}})
// CHECK: call void asm sideeffect "fe_serialize", "~{dirflag},~{fpsr},~{flags}"()
  _ce_creg_xchg32(reg, data32);
  _ce_creg_xchg64(reg, data64);
  _ce_fscp_xchg32(reg, data32);
  _ce_fscp_xchg64(reg, data64);
  _ce_creg_read32(reg);
  _ce_creg_read64(reg);
  _ce_fscp_read32(reg);
  _ce_fscp_read64(reg);
  _ce_creg_xchg_mt32(reg, data32);
  _ce_creg_xchg_mt64(reg, data64);
  _ce_creg_read_mt32(reg);
  _ce_creg_read_mt64(reg);
  _ce_portin8(data64);
  _ce_portin16(data64);
  _ce_portin32(data64);
  _ce_portin64(data64);
  _ce_portout8(data8, data64);
  _ce_portout16(data16, data64);
  _ce_portout32(data32, data64);
  _ce_portout64(data64, data64);
  _ce_sta_special(data64);
  _ce_nr_read(0);
  _ce_ucodecall(reg);
  _ce_cmodemov(data64, data64, 0);
  _ce_sigeventjump(data64, data32, MY_CONSTANT_VAL);
  _ce_sserialize();
  _ce_nop_set_sb();
  _ce_nop_read_sb();
  _ce_get_excl_acc();
  _ce_release_excl_acc();
  _ce_virt_nuke_point();
  _ce_int_trap_point();
  _ce_iceret_indirect(reg, reg);
  _ce_set_tracker(0);
  _ce_portin24(data64);
  _ce_portout24(data32, data64);
  _ce_load_tickle_gpa(data64);
  _ce_store_tickle_gpa(data64);
  _ce_flush_ifu();
  data16 = _ce_loadlin16(data64);
  data32 = _ce_loadlin32(data64);
  data64 = _ce_loadlin64(data64);
  _ce_storelin16(data16, data64);
  _ce_storelin32(data32, data64);
  _ce_storelin64(data64, data64);
  data64 = _ce_cccm(data64);
  data64 = _ce_cccp(data64);
  _ce_jmp_nopred(data64);
  _ce_fe_serialize();
}

void test_ce_iceret(unsigned reg) {
// CHECK: call void asm sideeffect "iceret
// CHECK-NEXT: unreachable
  _ce_iceret(reg);
}
