; RUN: opt -passes=instcombine -instcombine-preserve-for-dtrans=false < %s -S 2>&1 | FileCheck %s

; Check that when -instcombine-preserve-for-dtrans=false, a dope vector store is lowered into a series of field stores.
; This test case checks when opaque pointers are enabled.

; CHECK: define void @foo_
; CHECK: %"foo_$MYB.repack" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i64 0, i32 0
; CHECK: store ptr %phys_prop_mp_physprop__fetch.6.unpack, ptr %"foo_$MYB.repack", align 1
; CHECK: %"foo_$MYB.repack13" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i64 0, i32 1
; CHECK: store i64 %phys_prop_mp_physprop__fetch.6.unpack2, ptr %"foo_$MYB.repack13", align 1
; CHECK: %"foo_$MYB.repack15" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i64 0, i32 2
; CHECK: store i64 %phys_prop_mp_physprop__fetch.6.unpack3, ptr %"foo_$MYB.repack15", align 1
; CHECK: %"foo_$MYB.repack17" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i64 0, i32 3
; CHECK: store i64 %phys_prop_mp_physprop__fetch.6.unpack4, ptr %"foo_$MYB.repack17", align 1
; CHECK: %"foo_$MYB.repack19" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i64 0, i32 4
; CHECK: store i64 %phys_prop_mp_physprop__fetch.6.unpack5, ptr %"foo_$MYB.repack19", align 1
; CHECK: %"foo_$MYB.repack21" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i64 0, i32 5
; CHECK: store i64 %phys_prop_mp_physprop__fetch.6.unpack6, ptr %"foo_$MYB.repack21", align 1
; CHECK: %"foo_$MYB.repack23.repack" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i64 0, i32 6
; CHECK: store i64 %phys_prop_mp_physprop__fetch.6.unpack7.unpack.unpack, ptr %"foo_$MYB.repack23.repack", align 1
; CHECK: %"foo_$MYB.repack23.repack25" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i64 0, i32 6, i64 0, i32 1
; CHECK: store i64 %phys_prop_mp_physprop__fetch.6.unpack7.unpack.unpack10, ptr %"foo_$MYB.repack23.repack25", align 1
; CHECK: %"foo_$MYB.repack23.repack27" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i64 0, i32 6, i64 0, i32 2
; CHECK: store i64 %phys_prop_mp_physprop__fetch.6.unpack7.unpack.unpack11, ptr %"foo_$MYB.repack23.repack27", align 1

; ModuleID = 'intel-dvlower-on.ll'
source_filename = "intel-dvlower-on.ll"

%"QNCA_a0$ptr$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@phys_prop_mp_physprop_ = available_externally global %"QNCA_a0$ptr$rank1$" zeroinitializer

declare i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr

declare i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr

define void @foo_(ptr noalias dereferenceable(72) %"foo_$MYB") local_unnamed_addr {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca { i32 }, align 8
  %fetch.1 = load i64, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 3), align 1
  %and.1 = and i64 %fetch.1, 256
  %lshr.1 = lshr i64 %and.1, 8
  %shl.1 = shl i64 %lshr.1, 8
  %or.1 = or i64 133, %shl.1
  %and.3 = and i64 %fetch.1, 1030792151040
  %lshr.2 = lshr i64 %and.3, 36
  %and.4 = and i64 %or.1, -1030792151041
  %shl.2 = shl i64 %lshr.2, 36
  %or.2 = or i64 %and.4, %shl.2
  store i64 %or.2, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 3), align 1
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 5), align 1
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 1), align 1
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 4), align 1
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 2), align 1
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 6, i32 0, i32 2), align 1
  store i64 100, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 6, i32 0, i32 0), align 1
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 6, i32 0, i32 1), align 1
  %fetch.2 = load i64, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 3), align 1
  %and.5 = and i64 %fetch.2, -68451041281
  %or.3 = or i64 %and.5, 1073741824
  store i64 %or.3, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 3), align 1
  %and.6 = and i64 %fetch.2, 1
  %shl.4 = shl i64 %and.6, 1
  %int_zext = trunc i64 %shl.4 to i32
  %and.9 = and i32 %int_zext, -17
  %and.10 = and i64 %fetch.2, 256
  %lshr.3 = lshr i64 %and.10, 8
  %and.11 = and i32 %and.9, -2097153
  %shl.5 = shl i64 %lshr.3, 21
  %int_zext4 = trunc i64 %shl.5 to i32
  %or.6 = or i32 %and.11, %int_zext4
  %and.12 = and i64 %fetch.2, 1030792151040
  %lshr.4 = lshr i64 %and.12, 36
  %and.13 = and i32 %or.6, -31457281
  %shl.6 = shl i64 %lshr.4, 21
  %int_zext5 = trunc i64 %shl.6 to i32
  %or.7 = or i32 %and.13, %int_zext5
  %and.14 = and i64 %fetch.2, 1099511627776
  %lshr.5 = lshr i64 %and.14, 40
  %and.15 = and i32 %or.7, -33554433
  %shl.7 = shl i64 %lshr.5, 25
  %int_zext6 = trunc i64 %shl.7 to i32
  %or.8 = or i32 %and.15, %int_zext6
  %and.16 = and i32 %or.8, -2031617
  %or.9 = or i32 %and.16, 262144
  %fetch.4 = load i64, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 5), align 1
  %"(ptr)fetch.4$" = inttoptr i64 %fetch.4 to ptr
  %func_result = call i32 @for_alloc_allocatable_handle(i64 400, ptr @phys_prop_mp_physprop_, i32 %or.9, ptr %"(ptr)fetch.4$")
  %"val$[]_fetch.5" = load i64, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, i32 0, i32 6, i32 0, i32 2), align 1
  %phys_prop_mp_physprop__fetch.6 = load %"QNCA_a0$ptr$rank1$", ptr @phys_prop_mp_physprop_, align 1
  store %"QNCA_a0$ptr$rank1$" %phys_prop_mp_physprop__fetch.6, ptr %"foo_$MYB", align 1
  %"foo_$MYB.flags$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i32 0, i32 3
  %"foo_$MYB.flags$_fetch.7" = load i64, ptr %"foo_$MYB.flags$", align 1
  %or.10 = or i64 %"foo_$MYB.flags$_fetch.7", 2
  store i64 %or.10, ptr %"foo_$MYB.flags$", align 1
  %"foo_$MYB.addr_a0$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i32 0, i32 0
  %"foo_$MYB.addr_a0$_fetch.8" = load ptr, ptr %"foo_$MYB.addr_a0$", align 1
  %"foo_$MYB.dim_info$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"foo_$MYB", i32 0, i32 6, i32 0
  %"foo_$MYB.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, ptr %"foo_$MYB.dim_info$", i32 0, i32 1
  %"foo_$MYB.dim_info$.spacing$[]_fetch.9" = load i64, ptr %"foo_$MYB.dim_info$.spacing$", align 1
  %"foo_$MYB.dim_info$.lower_bound$" = getelementptr inbounds { i64, i64, i64 }, ptr %"foo_$MYB.dim_info$", i32 0, i32 2
  %"foo_$MYB.dim_info$.lower_bound$[]_fetch.10" = load i64, ptr %"foo_$MYB.dim_info$.lower_bound$", align 1
  %el = sdiv exact i64 %"foo_$MYB.dim_info$.spacing$[]_fetch.9", 4
  %0 = sub nsw i64 1, %"foo_$MYB.dim_info$.lower_bound$[]_fetch.10"
  %1 = mul nsw i64 %el, %0
  %2 = getelementptr inbounds i32, ptr %"foo_$MYB.addr_a0$_fetch.8", i64 %1
  %"foo_$MYB.addr_a0$_fetch.8[]_fetch.13" = load i32, ptr %2, align 1
  %.fca.0.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i32 0, i32 0
  store i8 9, ptr %.fca.0.gep, align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i32 0, i32 1
  store i8 1, ptr %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i32 0, i32 2
  store i8 1, ptr %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i32 0, i32 3
  store i8 0, ptr %.fca.3.gep, align 1
  %BLKFIELD_ = getelementptr inbounds { i32 }, ptr %argblock, i32 0, i32 0
  store i32 %"foo_$MYB.addr_a0$_fetch.8[]_fetch.13", ptr %BLKFIELD_, align 1
  %"(ptr)$io_ctx$" = bitcast ptr %"$io_ctx" to ptr
  %"(ptr)(&)val$$" = bitcast ptr %"(&)val$" to ptr
  %"(ptr)argblock$" = bitcast ptr %argblock to ptr
  %func_result9 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr %"(ptr)$io_ctx$", i32 -1, i64 1239157112576, ptr %"(ptr)(&)val$$", ptr %"(ptr)argblock$")
  ret void
}
