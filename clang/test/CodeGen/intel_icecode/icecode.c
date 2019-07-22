// REQUIRES: intel_feature_icecode
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_icecode-unknown-unknown -emit-llvm -o - -Wall -Werror -pedantic | FileCheck %s

#include <icecode/ceintrin.h>

extern void* mem;

void test_xucode() {
// CHECK: call i16 @llvm.x86.icecode.loadpphys.16(i8* %{{.*}})
// CHECK: call i32 @llvm.x86.icecode.loadpphys.32(i8* %{{.*}})
// CHECK: call i64 @llvm.x86.icecode.loadpphys.64(i8* %{{.*}})
// CHECK: call void @llvm.x86.icecode.storepphys.16(i16 %{{.*}}, i8* %{{.*}})
// CHECK: call void @llvm.x86.icecode.storepphys.32(i32 %{{.*}}, i8* %{{.*}})
// CHECK: call void @llvm.x86.icecode.storepphys.64(i64 %{{.*}}, i8* %{{.*}})
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
  short reg16;
  int reg32;
  long long reg64, info[4];
  reg16 = _ce_loadpphys16(mem);
  reg32 = _ce_loadpphys32(mem);
  reg64 = _ce_loadpphys64(mem);
  _ce_storepphys16(reg16, mem);
  _ce_storepphys32(reg32, mem);
  _ce_storepphys64(reg64, mem);
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
