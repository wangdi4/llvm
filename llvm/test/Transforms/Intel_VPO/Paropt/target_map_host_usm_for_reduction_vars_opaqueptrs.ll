; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-transform -debug-only=vpo-paropt-target -S <%s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-transform -debug-only=vpo-paropt-target -S <%s 2>&1 | FileCheck %s
;
; // C++ test
; void foo(int n, int *ccc_arrsect, int &ddd_scalar_byref, int (&eee_array_byref)[100], int *&fff_arrsect_byref) {
;   int aaa_scalar;
;   int bbb_array[100];
;   int ggg_arrsect_varoffset[100];
;   int hhh_arrsect_varlen[100];
;   int iii_vla[n];
;
;   #pragma omp target teams distribute parallel for \
;      reduction( + : aaa_scalar, bbb_array, ccc_arrsect[20:60], \
;                     ddd_scalar_byref, eee_array_byref, fff_arrsect_byref[30:60], \
;                     ggg_arrsect_varoffset[n:60] , hhh_arrsect_varlen[40:n], iii_vla )
;   for (int i=0; i<10; i++)
;   {
;     aaa_scalar                = 111; // expected to use HOST_MEM
;     bbb_array[22]             = 222; // expected to use HOST_MEM
;     ccc_arrsect[33]           = 333; // expected to use HOST_MEM
;
;     ddd_scalar_byref          = 444; // expected to use HOST_MEM
;     eee_array_byref[55]       = 555; // expected to use HOST_MEM
;     fff_arrsect_byref[66]     = 666; // expected to use HOST_MEM
;
;     ggg_arrsect_varoffset[77] = 777; // not AFReduction; don't use HOST_MEM
;     hhh_arrsect_varlen[88]    = 888; // not AFReduction; don't use HOST_MEM
;     iii_vla[99]               = 999; // not AFReduction; don't use HOST_MEM
;   }
; }
;
; The CFE adds the USE_HOST_MEM MapType (bit 0x8000) to the implicit MAP of a REDUCTION var.
; This MapType should only be used if the var is in an atomic-free reduction, so Paropt
; removes the bit if it finds that the reduction is not atomic-free.
;
; CHECK-LABEL: Enter VPOParoptTransform::genReductionCode
;
; CHECK: MapOrig: ptr %aaa_scalar is used in atomic-free reduction.
; CHECK: MapOrig: ptr %bbb_array is used in atomic-free reduction.
; CHECK: MapOrig: ptr %ccc_arrsect is used in atomic-free reduction.
; CHECK: MapOrig: ptr %ddd_scalar_byref is used in atomic-free reduction.
; CHECK: MapOrig: ptr %eee_array_byref is used in atomic-free reduction.
; CHECK: MapOrig: ptr %fff_arrsect_byref is used in atomic-free reduction.
; CHECK-NOT: MapOrig: ptr %ggg_arrsect_varoffset is used in atomic-free reduction.
; CHECK-NOT: MapOrig: ptr %hhh_arrsect_varlen is used in atomic-free reduction.
; CHECK-NOT: MapOrig: ptr %vla is used in atomic-free reduction.
;
;
; CHECK-LABEL: Enter VPOParoptTransform::genTgtInformationForPtrs
;
; CHECK: Working with Map Item: 'CHAIN(<ptr %aaa_scalar, ptr %aaa_scalar, i64 4, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit and it is honored.
;
; CHECK: Working with Map Item: 'CHAIN(<ptr %bbb_array, ptr %bbb_array, i64 400, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit and it is honored.
;
; CHECK: Working with Map Item: 'CHAIN(<ptr %ccc_arrsect, ptr %arrayidx7, i64 240, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit and it is honored.
;
; CHECK: Working with Map Item: 'CHAIN(<ptr %ddd_scalar_byref, ptr %ddd_scalar_byref, i64 4, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit and it is honored.
;
; CHECK: Working with Map Item: 'CHAIN(<ptr %eee_array_byref, ptr %eee_array_byref, i64 400, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit and it is honored.
;
; CHECK: Working with Map Item: 'CHAIN(<ptr %fff_arrsect_byref, ptr %arrayidx8, i64 240, 33331 (0x0000000000008233), null, null> ) '.
; CHECK: MapType 33331 has HOST_MEM bit and it is honored.
;
; CHECK: Working with Map Item: 'CHAIN(<ptr %ggg_arrsect_varoffset, ptr %arrayidx9, i64 240, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit but it is removed.
; CHECK: MapType changed from '33315 (0x0000000000008223)' to '547 (0x0000000000000223)'.
;
; CHECK: Working with Map Item: 'CHAIN(<ptr %hhh_arrsect_varlen, ptr %arrayidx10, i64 %5, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit but it is removed.
; CHECK: MapType changed from '33315 (0x0000000000008223)' to '547 (0x0000000000000223)'.
;
; CHECK: Working with Map Item: 'CHAIN,VARLEN(<ptr %vla, ptr %vla, i64 %6, 33315 (0x0000000000008223), null, null> ) '.
; CHECK: MapType 33315 has HOST_MEM bit but it is removed.
; CHECK: MapType changed from '33315 (0x0000000000008223)' to '547 (0x0000000000000223)'.
;
;
; CHECK: @.offload_maptypes = private unnamed_addr constant [28 x i64] [i64 33315, i64 33315, i64 33315, i64 33315, i64 33315, i64 33331, i64 547, i64 547, i64 547,

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @_Z3fooiPiRiRA100_iRS_(i32 noundef %n, ptr noundef %ccc_arrsect, ptr noundef nonnull align 4 dereferenceable(4) %ddd_scalar_byref, ptr noundef nonnull align 4 dereferenceable(400) %eee_array_byref, ptr noundef nonnull align 8 dereferenceable(8) %fff_arrsect_byref) local_unnamed_addr {
DIR.OMP.TARGET.3102:
  %n.addr = alloca i32, align 4
  %aaa_scalar = alloca i32, align 4
  %bbb_array = alloca [100 x i32], align 16
  %ggg_arrsect_varoffset = alloca [100 x i32], align 16
  %hhh_arrsect_varlen = alloca [100 x i32], align 16
  %omp.vla.tmp = alloca i64, align 8
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %ccc_arrsect.map.ptr.tmp = alloca ptr, align 8
  %ddd_scalar_byref.map.ptr.tmp = alloca ptr, align 8
  %eee_array_byref.map.ptr.tmp = alloca ptr, align 8
  %fff_arrsect_byref.map.ptr.tmp = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4
  %0 = zext i32 %n to i64
  %1 = call ptr @llvm.stacksave()
  %vla = alloca i32, i64 %0, align 16
  store i64 %0, ptr %omp.vla.tmp, align 8
  store i32 0, ptr %.omp.lb, align 4
  store volatile i32 9, ptr %.omp.ub, align 4
  %2 = load ptr, ptr %fff_arrsect_byref, align 8
  %3 = load i32, ptr %n.addr, align 4
  %4 = sext i32 %3 to i64
  %arrayidx7 = getelementptr inbounds i32, ptr %ccc_arrsect, i64 20
  %arrayidx8 = getelementptr inbounds i32, ptr %2, i64 30
  %arrayidx9 = getelementptr inbounds [100 x i32], ptr %ggg_arrsect_varoffset, i64 0, i64 %4
  %arrayidx10 = getelementptr inbounds [100 x i32], ptr %hhh_arrsect_varlen, i64 0, i64 40
  %5 = shl nuw nsw i64 %4, 2
  %6 = shl nuw nsw i64 %0, 2
  %end.dir.temp90 = alloca i1, align 1
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %DIR.OMP.TARGET.3102
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %aaa_scalar, ptr %aaa_scalar, i64 4, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %bbb_array, ptr %bbb_array, i64 400, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %ccc_arrsect, ptr %arrayidx7, i64 240, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %ddd_scalar_byref, ptr %ddd_scalar_byref, i64 4, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %eee_array_byref, ptr %eee_array_byref, i64 400, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %fff_arrsect_byref, ptr %arrayidx8, i64 240, i64 33331, ptr null, ptr null), ; MAP type: 33331 = 0x8233 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | PTR_AND_OBJ (0x10) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %ggg_arrsect_varoffset, ptr %arrayidx9, i64 240, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %hhh_arrsect_varlen, ptr %arrayidx10, i64 %5, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM:VARLEN"(ptr %vla, ptr %vla, i64 %6, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = USE_HOST_MEM (0x8000) | IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %n.addr, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr %omp.vla.tmp, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %ccc_arrsect.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %ddd_scalar_byref.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %eee_array_byref.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %fff_arrsect_byref.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.OFFLOAD.NDRANGE"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.OFFLOAD.HAS.TEAMS.REDUCTION"(),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp90) ]

  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2
  %temp.load91 = load volatile i1, ptr %end.dir.temp90, align 1
  br i1 %temp.load91, label %DIR.OMP.END.TARGET.14, label %DIR.OMP.TEAMS.6103

DIR.OMP.TEAMS.6103:                               ; preds = %DIR.OMP.TARGET.3
  %8 = load i64, ptr %omp.vla.tmp, align 8
  store ptr %ccc_arrsect, ptr %ccc_arrsect.map.ptr.tmp, align 8
  store ptr %ddd_scalar_byref, ptr %ddd_scalar_byref.map.ptr.tmp, align 8
  store ptr %eee_array_byref, ptr %eee_array_byref.map.ptr.tmp, align 8
  store ptr %fff_arrsect_byref, ptr %fff_arrsect_byref.map.ptr.tmp, align 8
  %9 = load i32, ptr %n.addr, align 4
  %conv15 = sext i32 %9 to i64
  %end.dir.temp87 = alloca i1, align 1
  br label %DIR.OMP.TEAMS.4

DIR.OMP.TEAMS.4:                                  ; preds = %DIR.OMP.TEAMS.6103
  br label %DIR.OMP.TEAMS.5

DIR.OMP.TEAMS.5:                                  ; preds = %DIR.OMP.TEAMS.4
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %aaa_scalar, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %bbb_array, i32 0, i64 100),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"(ptr %ccc_arrsect.map.ptr.tmp, i32 0, i64 60, i64 20),
    "QUAL.OMP.REDUCTION.ADD:BYREF.TYPED"(ptr %ddd_scalar_byref.map.ptr.tmp, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:BYREF.TYPED"(ptr %eee_array_byref.map.ptr.tmp, [100 x i32] zeroinitializer, i32 1),
    "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.PTR_TO_PTR.TYPED"(ptr %fff_arrsect_byref.map.ptr.tmp, i32 0, i64 60, i64 30),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %ggg_arrsect_varoffset, i32 0, i64 60, i64 %conv15),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %hhh_arrsect_varlen, i32 0, i64 %conv15, i64 40),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %vla, i32 0, i64 %8),
    "QUAL.OMP.SHARED:TYPED"(ptr %n.addr, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.vla.tmp, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp87) ]

  br label %DIR.OMP.TEAMS.5.split

DIR.OMP.TEAMS.5.split:                            ; preds = %DIR.OMP.TEAMS.5
  %n.addr.fp = alloca i32, align 4
  %n.addr.v = load i32, ptr %n.addr, align 4
  store i32 %n.addr.v, ptr %n.addr.fp, align 4
  %.omp.lb.fp = alloca i32, align 4
  %.omp.lb.v = load i32, ptr %.omp.lb, align 4
  store i32 %.omp.lb.v, ptr %.omp.lb.fp, align 4
  %omp.vla.tmp.fp = alloca i64, align 8
  %omp.vla.tmp.v = load i64, ptr %omp.vla.tmp, align 8
  store i64 %omp.vla.tmp.v, ptr %omp.vla.tmp.fp, align 8
  br label %DIR.OMP.TEAMS.6

DIR.OMP.TEAMS.6:                                  ; preds = %DIR.OMP.TEAMS.5.split
  %temp.load88 = load volatile i1, ptr %end.dir.temp87, align 1
  br i1 %temp.load88, label %DIR.OMP.END.TEAMS.12, label %DIR.OMP.DISTRIBUTE.PARLOOP.9104

DIR.OMP.DISTRIBUTE.PARLOOP.9104:                  ; preds = %DIR.OMP.TEAMS.6
  %11 = load i64, ptr %omp.vla.tmp.fp, align 8
  %12 = load i32, ptr %n.addr.fp, align 4
  %conv24 = sext i32 %12 to i64
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.7

DIR.OMP.DISTRIBUTE.PARLOOP.7:                     ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.9104
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.8

DIR.OMP.DISTRIBUTE.PARLOOP.8:                     ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.7
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %aaa_scalar, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %bbb_array, i32 0, i64 100),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"(ptr %ccc_arrsect.map.ptr.tmp, i32 0, i64 60, i64 20),
    "QUAL.OMP.REDUCTION.ADD:BYREF.TYPED"(ptr %ddd_scalar_byref.map.ptr.tmp, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:BYREF.TYPED"(ptr %eee_array_byref.map.ptr.tmp, [100 x i32] zeroinitializer, i32 1),
    "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.PTR_TO_PTR.TYPED"(ptr %fff_arrsect_byref.map.ptr.tmp, i32 0, i64 60, i64 30),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %ggg_arrsect_varoffset, i32 0, i64 60, i64 %conv24),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %hhh_arrsect_varlen, i32 0, i64 %conv24, i64 40),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %vla, i32 0, i64 %11),
    "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb.fp, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr null, i64 0, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %n.addr.fp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %omp.vla.tmp.fp, i64 0, i32 1) ]

  br label %DIR.OMP.DISTRIBUTE.PARLOOP.9105

DIR.OMP.DISTRIBUTE.PARLOOP.9105:                  ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.8
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.11, label %DIR.OMP.DISTRIBUTE.PARLOOP.9

DIR.OMP.DISTRIBUTE.PARLOOP.9:                     ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.9105
  %14 = load i32, ptr %.omp.lb.fp, align 4
  store volatile i32 %14, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.DISTRIBUTE.PARLOOP.9
  %15 = load volatile i32, ptr %.omp.iv, align 4
  %16 = load volatile i32, ptr %.omp.ub, align 4
  %cmp.not = icmp sgt i32 %15, %16
  br i1 %cmp.not, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.11.loopexit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %17 = load volatile i32, ptr %.omp.iv, align 4
  store i32 %17, ptr %i, align 4
  store i32 111, ptr %aaa_scalar, align 4
  %arrayidx30 = getelementptr inbounds [100 x i32], ptr %bbb_array, i64 0, i64 22
  store i32 222, ptr %arrayidx30, align 8
  %18 = load ptr, ptr %ccc_arrsect.map.ptr.tmp, align 8
  %arrayidx31 = getelementptr inbounds i32, ptr %18, i64 33
  store i32 333, ptr %arrayidx31, align 4
  %19 = load ptr, ptr %ddd_scalar_byref.map.ptr.tmp, align 8
  store i32 444, ptr %19, align 4
  %20 = load ptr, ptr %eee_array_byref.map.ptr.tmp, align 8
  %arrayidx32 = getelementptr inbounds [100 x i32], ptr %20, i64 0, i64 55
  store i32 555, ptr %arrayidx32, align 4
  %21 = load ptr, ptr %fff_arrsect_byref.map.ptr.tmp, align 8
  %22 = load ptr, ptr %21, align 8
  %arrayidx33 = getelementptr inbounds i32, ptr %22, i64 66
  store i32 666, ptr %arrayidx33, align 4
  %arrayidx34 = getelementptr inbounds [100 x i32], ptr %ggg_arrsect_varoffset, i64 0, i64 77
  store i32 777, ptr %arrayidx34, align 4
  %arrayidx35 = getelementptr inbounds [100 x i32], ptr %hhh_arrsect_varlen, i64 0, i64 88
  store i32 888, ptr %arrayidx35, align 16
  %arrayidx36 = getelementptr inbounds i32, ptr %vla, i64 99
  store i32 999, ptr %arrayidx36, align 4
  %23 = load volatile i32, ptr %.omp.iv, align 4
  %add37 = add nsw i32 %23, 1
  store volatile i32 %add37, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

DIR.OMP.END.DISTRIBUTE.PARLOOP.11.loopexit:       ; preds = %omp.inner.for.cond
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.11

DIR.OMP.END.DISTRIBUTE.PARLOOP.11:                ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.11.loopexit, %DIR.OMP.DISTRIBUTE.PARLOOP.9105
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.10

DIR.OMP.END.DISTRIBUTE.PARLOOP.10:                ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.11
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.11106

DIR.OMP.END.DISTRIBUTE.PARLOOP.11106:             ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.10
  br label %DIR.OMP.END.TEAMS.12

DIR.OMP.END.TEAMS.12:                             ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.11106, %DIR.OMP.TEAMS.6
  br label %DIR.OMP.END.TEAMS.12107

DIR.OMP.END.TEAMS.12107:                          ; preds = %DIR.OMP.END.TEAMS.12
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TEAMS"() ]

  br label %DIR.OMP.END.TARGET.14

DIR.OMP.END.TARGET.14:                            ; preds = %DIR.OMP.TARGET.3, %DIR.OMP.END.TEAMS.12107
  br label %DIR.OMP.END.TARGET.13

DIR.OMP.END.TARGET.13:                            ; preds = %DIR.OMP.END.TARGET.14
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET"() ]

  br label %DIR.OMP.END.TARGET.14108

DIR.OMP.END.TARGET.14108:                         ; preds = %DIR.OMP.END.TARGET.13
  call void @llvm.stackrestore(ptr %1)
  ret void
}

declare ptr @llvm.stacksave()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.stackrestore(ptr)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 44, i32 -704186047, !"_Z3fooiPiRiRA100_iRS_", i32 8, i32 0, i32 0, i32 0}
