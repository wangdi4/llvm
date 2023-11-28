; RUN: opt -passes='vplan-vec' -S -vplan-entities-dump -vplan-print-after-vpentity-instrs -vplan-print-after-final-cond-transform < %s 2>&1 | FileCheck %s -check-prefixes=LLVMIR,CHECK
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec,print<hir>' -S -vplan-entities-dump -vplan-print-after-vpentity-instrs -vplan-print-after-final-cond-transform -vplan-force-vf=2 -vplan-enable-hir-f90-dv < %s 2>&1 | FileCheck %s -check-prefixes=HIR,CHECK

; Check proper support for SIMD private C1 of integer type.
; Fortran sourcecode used for this test
; integer function sum(c1, c2, n, m)
;      integer :: c1(:)
;      integer :: c2, n, m
;      sum = 1
;        !$omp simd private(c1)
;        do j=1, n
;          c1(j) = m*n + c2
;        enddo
; end function sum

; CHECK:  VPlan after insertion of VPEntities instructions:
; CHECK:  Private list
; CHECK:   Private tag: F90DV
; CHECK:   Linked values: ptr %"sum_$C1.priv", ptr [[VP1:%.*]], 
; CHECK:  Memory: ptr %"sum_$C1.priv"

; CHECK:  [[BB1:BB[0-9]+]]
; CHECK:    ptr [[VP1]] = allocate-priv %"QNCA_a0$ptr$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, OrigAlign = 8
; C_HECK:    call i64 72 ptr [[VP1]] ptr @llvm.lifetime.start.p0
; CHECK:    i64 [[VP4:%.*]] = call ptr [[VP3:%.*]] ptr [[VP2:%.*]] ptr @_f90_dope_vector_init2
; CHECK:    ptr [[VP5:%.*]] = call ptr @llvm.stacksave.p0
; CHECK:    f90-dv-buffer-init i64 [[VP4]] ptr [[VP1]]
; CHECK:    call ptr [[VP5]] ptr @llvm.stackrestore.p0

; CHECK:  VPlan after private finalization instructions transformation:
; CHECK:  Private list
; CHECK:   Private tag: F90DV
; CHECK:   Linked values: ptr %"sum_$C1.priv", ptr [[VP1]], 
; CHECK:  Memory: ptr %"sum_$C1.priv"

; CHECK:  [[BB1]]
; CHECK:      [DA: Div] ptr [[VP1]] = allocate-priv %"QNCA_a0$ptr$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, OrigAlign = 8
; C_HECK:      [DA: Div] call i64 72 ptr [[VP1]] ptr @llvm.lifetime.start.p0 [Serial]
; CHECK:      [DA: Div] i64 [[VP4]] = call ptr [[VP3]] ptr [[VP2:%.*]] ptr @_f90_dope_vector_init2 [Serial]
; CHECK:      [DA: Uni] ptr [[VP5]] = call ptr @llvm.stacksave.p0
; CHECK:      [DA: Div] i64 [[VP6:%.*]] = udiv i64 [[VP4]] i64 4
; CHECK:      [DA: Div] i1 [[VP7:%.*]] = icmp sgt i64 [[VP4]] i64 0
; CHECK:      [DA: Div] br i1 [[VP7]], [[BB20:BB[0-9]+]], [[BB21:BB[0-9]+]]

; CHECK:       [[BB20]]
; CHECK:        [DA: Div] ptr [[VP8:%.*]] = allocate-dv-buffer i32 i64 [[VP6]], OrigAlign = 4
; CHECK:        [DA: Div] store ptr [[VP8]] ptr [[VP1]]
; CHECK:        [DA: Uni] br [[BB3:BB[0-9]+]]
; CHECK:      [DA: Uni] call ptr [[VP5]] ptr @llvm.stackrestore.p0

; LLVMIR-NOT: DominatorTree is different than a freshly computed one!
; LLVMIR-NOT: PostDominatorTree is different than a freshly computed one!

; LLVMIR-LABEL: DIR.OMP.SIMD.2:
; LLVMIR-NEXT:   %"sum_$J.linear.iv" = alloca i32, align 4
; LLVMIR-NEXT:   %"sum_$C1.priv" = alloca %"QNCA_a0$ptr$rank1$", align 8
; LLVMIR-NEXT:   %"sum_$N_fetch.1" = load i32, ptr %"sum_$N", align 1
; LLVMIR-NEXT:   %.dv.init = call i64 @_f90_dope_vector_init2(ptr nonnull %"sum_$C1.priv", ptr nonnull %"sum_$C1")
; LLVMIR-NEXT:   %is.allocated = icmp sgt i64 %.dv.init, 0
; LLVMIR-NEXT:   %"sum_$C1.priv.vec" = alloca [2 x %"QNCA_a0$ptr$rank1$"], align 8
; LLVMIR-NEXT:   %"sum_$C1.priv.vec.base.addr" = getelementptr %"QNCA_a0$ptr$rank1$", ptr %"sum_$C1.priv.vec", <2 x i32> <i32 0, i32 1>
; LLVMIR-NEXT:   %"sum_$C1.priv.vec.base.addr.extract.1." = extractelement <2 x ptr> %"sum_$C1.priv.vec.base.addr", i32 1
; LLVMIR-NEXT:   %"sum_$C1.priv.vec.base.addr.extract.0." = extractelement <2 x ptr> %"sum_$C1.priv.vec.base.addr", i32 0
; LLVMIR-NEXT:   br i1 %is.allocated, label %allocated.then, label %DIR.OMP.SIMD.1
 
; LLVMIR:      VPlannedBB2:                                      ; preds = %VPlannedBB1
; LLVMIR-NEXT:   call void @llvm.lifetime.start.p0(i64 144, ptr %"sum_$C1.priv.vec.base.addr.extract.0.")
; LLVMIR-NEXT:   %2 = call i64 @_f90_dope_vector_init2(ptr %"sum_$C1.priv.vec.base.addr.extract.0.", ptr %"sum_$C1.priv")
; LLVMIR-NEXT:   %3 = insertelement <2 x i64> undef, i64 %2, i32 0
; LLVMIR-NEXT:   %4 = call i64 @_f90_dope_vector_init2(ptr %"sum_$C1.priv.vec.base.addr.extract.1.", ptr %"sum_$C1.priv")
; LLVMIR-NEXT:   %5 = insertelement <2 x i64> %3, i64 %4, i32 1
; LLVMIR-NEXT:   %6 = call ptr @llvm.stacksave.p0()
; LLVMIR-NEXT:   %7 = udiv <2 x i64> %5, <i64 4, i64 4>
; LLVMIR-NEXT:   %.extract.0.5 = extractelement <2 x i64> %7, i32 0
; LLVMIR-NEXT:   %8 = icmp sgt <2 x i64> %5, zeroinitializer
; LLVMIR-NEXT:   %.extract.0. = extractelement <2 x i1> %8, i32 0
; LLVMIR-NEXT:   br i1 %.extract.0., label %VPlannedBB3, label %VPlannedBB4

; LLVMIR:      VPlannedBB3:                                      ; preds = %VPlannedBB2
; LLVMIR-NEXT:   %.array.buffer.lane.0 = alloca i32, i64 %.extract.0.5, align 4
; LLVMIR-NEXT:   %.array.buffer.insert.0 = insertelement <2 x ptr> undef, ptr %.array.buffer.lane.0, i64 0
; LLVMIR-NEXT:   %.array.buffer.lane.1 = alloca i32, i64 %.extract.0.5, align 4
; LLVMIR-NEXT:   %.array.buffer.insert.1 = insertelement <2 x ptr> %.array.buffer.insert.0, ptr %.array.buffer.lane.1, i64 1
; LLVMIR-NEXT:   call void @llvm.masked.scatter.v2p0.v2p0(<2 x ptr> %.array.buffer.insert.1, <2 x ptr> %"sum_$C1.priv.vec.base.addr", i32 1, <2 x i1> <i1 true, i1 true>)
; LLVMIR-NEXT:   br label %VPlannedBB4

; HIR:        %priv.mem.bc = &((%"QNCA_a0$ptr$rank1$"*)(%priv.mem)[0]);
; HIR-NEXT:   %serial.temp = undef;
; HIR-NEXT:   %_f90_dope_vector_init2 = @_f90_dope_vector_init2(&((%"QNCA_a0$ptr$rank1$"*)(%priv.mem)[0]),  %"sum_$C1.priv");
; HIR-NEXT:   %serial.temp = insertelement %serial.temp,  %_f90_dope_vector_init2,  0;
; HIR-NEXT:   %extract.1. = extractelement &((<2 x ptr>)(%priv.mem.bc)[<i32 0, i32 1>]),  1;
; HIR-NEXT:   %_f90_dope_vector_init22 = @_f90_dope_vector_init2(%extract.1.,  %"sum_$C1.priv");
; HIR-NEXT:   %serial.temp = insertelement %serial.temp,  %_f90_dope_vector_init22,  1;
; HIR-NEXT:   %llvm.stacksave.p0 = @llvm.stacksave.p0();
; HIR-NEXT:   %.vec4 = %serial.temp  /u  4;
; HIR-NEXT:   %.vec5 = %serial.temp > 0;
; HIR-NEXT:   %extract.0.6 = extractelement %.vec5,  0;
; HIR-NEXT:   if (%extract.0.6 == 1)
; HIR-NEXT:   {
; HIR-NEXT:   }
; HIR-NEXT:   else
; HIR-NEXT:   {
; HIR-NEXT:      goto BB23.62;
; HIR-NEXT:   }
; HIR-NEXT:   %extract.0.7 = extractelement %.vec4,  0;
; HIR-NEXT:   %dv.buffer.vec = undef;
; HIR-NEXT:   %dv.buffer.scal = alloca %extract.0.7;
; HIR-NEXT:   %dv.buffer.vec = insertelement %dv.buffer.vec,  %dv.buffer.scal,  0;
; HIR-NEXT:   %dv.buffer.scal9 = alloca %extract.0.7;
; HIR-NEXT:   %dv.buffer.vec = insertelement %dv.buffer.vec,  %dv.buffer.scal9,  1;
; HIR-NEXT:   (<2 x ptr>*)(%priv.mem.bc)[<i32 0, i32 1>] = %dv.buffer.vec;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$ptr$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: nounwind
define i32 @sum_(ptr noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"sum_$C1", ptr noalias nocapture dereferenceable(4) %"sum_$C2", ptr noalias nocapture dereferenceable(4) %"sum_$N", ptr noalias nocapture dereferenceable(4) %"sum_$M") {
DIR.OMP.SIMD.2:
  %"sum_$J.linear.iv" = alloca i32, align 4
  %"sum_$C1.priv" = alloca %"QNCA_a0$ptr$rank1$", align 8
  %"sum_$N_fetch.1" = load i32, ptr %"sum_$N", align 1
  %.dv.init = call i64 @_f90_dope_vector_init2(ptr nonnull %"sum_$C1.priv", ptr nonnull %"sum_$C1")
  %is.allocated = icmp sgt i64 %.dv.init, 0
  br i1 %is.allocated, label %allocated.then, label %DIR.OMP.SIMD.1

allocated.then:                                   ; preds = %DIR.OMP.SIMD.2
  %"sum_$C1.priv.alloc.num_elements16" = lshr i64 %.dv.init, 2
  %"sum_$C1.priv.data" = alloca i32, i64 %"sum_$C1.priv.alloc.num_elements16", align 4
  store ptr %"sum_$C1.priv.data", ptr %"sum_$C1.priv", align 8
  br label %DIR.OMP.SIMD.1

omp.pdo.body6:                                    ; preds = %DIR.OMP.SIMD.119, %omp.pdo.body6
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.119 ], [ %indvars.iv.next, %omp.pdo.body6 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %"sum_$C1.addr_a0$_fetch.10[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"sum_$C1.dim_info$.spacing$[]_fetch.11", ptr elementtype(i32) %"sum_$C1.addr_a0$_fetch.10", i64 %indvars.iv.next)
  store i32 %add.3, ptr %"sum_$C1.addr_a0$_fetch.10[]", align 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.pdo.body6

DIR.OMP.SIMD.1:                                   ; preds = %allocated.then, %DIR.OMP.SIMD.2
  %rel.1.not13 = icmp slt i32 %"sum_$N_fetch.1", 1
  br i1 %rel.1.not13, label %DIR.OMP.END.SIMD.412, label %DIR.OMP.SIMD.118

DIR.OMP.SIMD.118:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:TYPED.IV"(ptr %"sum_$J.linear.iv", i32 0, i64 1, i32 1), "QUAL.OMP.PRIVATE:F90_DV.TYPED"(ptr %"sum_$C1.priv", %"QNCA_a0$ptr$rank1$" zeroinitializer, i32 0), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LIVEIN"(ptr %"sum_$C2"), "QUAL.OMP.LIVEIN"(ptr %"sum_$M"), "QUAL.OMP.LIVEIN"(ptr %"sum_$N") ]
  br label %DIR.OMP.SIMD.119

DIR.OMP.SIMD.119:                                 ; preds = %DIR.OMP.SIMD.118
  %"sum_$M_fetch.7" = load i32, ptr %"sum_$M", align 1
  %"sum_$N_fetch.8" = load i32, ptr %"sum_$N", align 1
  %mul.1 = mul nsw i32 %"sum_$N_fetch.8", %"sum_$M_fetch.7"
  %"sum_$C2_fetch.9" = load i32, ptr %"sum_$C2", align 1
  %add.3 = add nsw i32 %mul.1, %"sum_$C2_fetch.9"
  %"sum_$C1.addr_a0$_fetch.10" = load ptr, ptr %"sum_$C1.priv", align 8
  %"sum_$C1.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %"sum_$C1.priv", i64 0, i32 6, i64 0, i32 1
  %"sum_$C1.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"sum_$C1.dim_info$.spacing$", i32 0)
  %"sum_$C1.dim_info$.spacing$[]_fetch.11" = load i64, ptr %"sum_$C1.dim_info$.spacing$[]", align 1
  %wide.trip.count = zext i32 %"sum_$N_fetch.1" to i64
  br label %omp.pdo.body6

DIR.OMP.END.SIMD.2:                               ; preds = %omp.pdo.body6
  %add.4.le = add nuw i32 %"sum_$N_fetch.1", 1
  store i32 %add.4.le, ptr %"sum_$J.linear.iv", align 4
  br label %DIR.OMP.END.SIMD.220

DIR.OMP.END.SIMD.220:                             ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.412

DIR.OMP.END.SIMD.412:                             ; preds = %DIR.OMP.SIMD.1, %DIR.OMP.END.SIMD.220
  ret i32 1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #0

; Function Attrs: nounwind
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

declare i64 @_f90_dope_vector_init2(ptr, ptr) local_unnamed_addr

attributes #0 = { nounwind }
