; Test to verify that VPlan framework imports and handles user-defined reductions
; when the pointer in the SIMD clause is a non-alloca value.

; RUN: opt -passes="vplan-vec" -vplan-entities-dump -vplan-print-legality -vplan-print-after-vpentity-instrs -vplan-force-vf=2 -disable-output < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; ------------------------------------------------------------------------------

; Check that UDRs are captured in legality lists
; CHECK-LABEL: VPOLegality UDRList:
; CHECK:       Ref:   %sum_a.fast_red = getelementptr inbounds %struct.fast_red_t, ptr %fast_red_struct, i64 0, i32 0
; CHECK:         UpdateInstructions:
; CHECK-NEXT:    none
; CHECK-NEXT:    RedDescr: {Kind: call, IsSigned: 0, IsComplex: 0}
; CHECK-NEXT:    RedDescrUDR: {Combiner: .omp_combiner..4, Initializer: none, Ctor: _ZTSSt7complexIfE.omp.def_constr, Dtor: _ZTSSt7complexIfE.omp.destr}

; CHECK:       Ref:   %sum_b.fast_red = getelementptr inbounds %struct.fast_red_t, ptr %fast_red_struct, i64 0, i32 1
; CHECK:         UpdateInstructions:
; CHECK-NEXT:    none
; CHECK-NEXT:    RedDescr: {Kind: call, IsSigned: 0, IsComplex: 0}
; CHECK-NEXT:    RedDescrUDR: {Combiner: .omp_combiner..5, Initializer: none, Ctor: _ZTSSt7complexIfE.omp.def_constr, Dtor: _ZTSSt7complexIfE.omp.destr}

; ------------------------------------------------------------------------------

; Check that UDRs are imported as VPEntities and lowered to VPInstructions.
; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:       Reduction list

; CHECK-NEXT:   (UDR) Start: ptr %sum_a.fast_red
; CHECK-NEXT:   Linked values: ptr [[PVT1:%vp.*]], 
; CHECK-NEXT:  udr Combiner: .omp_combiner..4, Initializer: none, Ctor: _ZTSSt7complexIfE.omp.def_constr, Dtor: _ZTSSt7complexIfE.omp.destr}
; CHECK-NEXT:  Memory: ptr %sum_a.fast_red
; CHECK-NEXT:   (UDR) Start: ptr %sum_b.fast_red
; CHECK-NEXT:   Linked values: ptr [[PVT2:%vp.*]], 
; CHECK-NEXT:  udr Combiner: .omp_combiner..5, Initializer: none, Ctor: _ZTSSt7complexIfE.omp.def_constr, Dtor: _ZTSSt7complexIfE.omp.destr}
; CHECK-NEXT:  Memory: ptr %sum_b.fast_red

; Initialization
; CHECK:        ptr [[PVT2]] = allocate-priv %"class.std::complex" = type { { float, float } }, OrigAlign = 8
; CHECK-NEXT:   ptr [[PVT1]] = allocate-priv %"class.std::complex" = type { { float, float } }, OrigAlign = 8
; CHECK-NEXT:   ptr {{.*}} = call ptr [[PVT1]] ptr @_ZTSSt7complexIfE.omp.def_constr
; CHECK-NEXT:   ptr {{.*}} = call ptr [[PVT2]] ptr @_ZTSSt7complexIfE.omp.def_constr

; Finalization
; CHECK:        reduction-final-udr ptr [[PVT1]] ptr %sum_a.fast_red, Combiner: .omp_combiner..4
; CHECK-NEXT:   call ptr [[PVT1]] ptr @_ZTSSt7complexIfE.omp.destr
; CHECK-NEXT:   reduction-final-udr ptr [[PVT2]] ptr %sum_b.fast_red, Combiner: .omp_combiner..5
; CHECK-NEXT:   call ptr [[PVT2]] ptr @_ZTSSt7complexIfE.omp.destr

; ------------------------------------------------------------------------------

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::complex" = type { { float, float } }
%struct.fast_red_t = type <{ %"class.std::complex", %"class.std::complex" }>

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: alwaysinline argmemonly mustprogress nofree norecurse nosync nounwind willreturn uwtable
declare hidden void @.omp_combiner..4(ptr noalias nocapture noundef, ptr noalias nocapture noundef readonly)

; Function Attrs: alwaysinline argmemonly mustprogress nofree norecurse nosync nounwind willreturn uwtable
declare hidden void @.omp_combiner..5(ptr noalias nocapture noundef, ptr noalias nocapture noundef readonly)

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
declare hidden noundef ptr @_ZTSSt7complexIfE.omp.def_constr(ptr noundef returned writeonly)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare hidden void @_ZTSSt7complexIfE.omp.destr(ptr nocapture readnone)

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg)

; Function Attrs: uwtable
define internal void @foo(ptr nocapture readonly %arr.map.ptr.tmp.fp118.0.val, ptr nocapture %sum_a, ptr nocapture %sum_b) {
DIR.OMP.SIMD.10:
  %fast_red_struct = alloca %struct.fast_red_t, align 8
  %sum_a.fast_red = getelementptr inbounds %struct.fast_red_t, ptr %fast_red_struct, i64 0, i32 0
  %sum_b.fast_red = getelementptr inbounds %struct.fast_red_t, ptr %fast_red_struct, i64 0, i32 1
  %_M_value.realp.i.i20 = getelementptr inbounds %"class.std::complex", ptr %sum_b.fast_red, i64 0, i32 0, i32 0
  %_M_value.imagp.i.i21 = getelementptr inbounds %struct.fast_red_t, ptr %fast_red_struct, i64 0, i32 1, i32 0, i32 1
  %_M_value.imagp.i.i23 = getelementptr inbounds %struct.fast_red_t, ptr %fast_red_struct, i64 0, i32 0, i32 0, i32 1
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %sum_a.fast_red, i8 0, i64 16, i1 false)
  br label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:                     ; preds = %DIR.OMP.SIMD.10
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr %sum_a.fast_red, %"class.std::complex" zeroinitializer, i32 1, ptr @_ZTSSt7complexIfE.omp.def_constr, ptr @_ZTSSt7complexIfE.omp.destr, ptr @.omp_combiner..4, ptr null), "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr %sum_b.fast_red, %"class.std::complex" zeroinitializer, i32 1, ptr @_ZTSSt7complexIfE.omp.def_constr, ptr @_ZTSSt7complexIfE.omp.destr, ptr @.omp_combiner..5, ptr null) ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.2

DIR.VPO.END.GUARD.MEM.MOTION.2:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.211, %omp.inner.for.body.preheader
  %.omp.iv.local.0 = phi i32 [ %add17, %DIR.VPO.END.GUARD.MEM.MOTION.211 ], [ 0, %omp.inner.for.body.preheader ]
  br label %DIR.VPO.GUARD.MEM.MOTION.1.split

DIR.VPO.GUARD.MEM.MOTION.1.split:                 ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.2
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %sum_a.fast_red), "QUAL.OMP.LIVEIN"(ptr %sum_b.fast_red) ]
  br label %DIR.VPO.GUARD.MEM.MOTION.1

DIR.VPO.GUARD.MEM.MOTION.1:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.1.split
  %add12 = add i32 %.omp.iv.local.0, 1
  %idxprom = sext i32 %add12 to i64
  %_M_value.realp.i.i = getelementptr inbounds %"class.std::complex", ptr %arr.map.ptr.tmp.fp118.0.val, i64 %idxprom, i32 0, i32 0
  %_M_value.real.i.i = load float, ptr %_M_value.realp.i.i, align 4
  %_M_value.imagp.i.i = getelementptr inbounds %"class.std::complex", ptr %arr.map.ptr.tmp.fp118.0.val, i64 %idxprom, i32 0, i32 1
  %_M_value.imag.i.i = load float, ptr %_M_value.imagp.i.i, align 4
  %_M_value.realp.i99 = getelementptr inbounds %"class.std::complex", ptr %sum_a.fast_red, i64 0, i32 0, i32 0
  %_M_value.real.i = load float, ptr %_M_value.realp.i99, align 4
  %_M_value.imagp.i100 = getelementptr inbounds %"class.std::complex", ptr %sum_a.fast_red, i64 0, i32 0, i32 1
  %_M_value.imag.i = load float, ptr %_M_value.imagp.i100, align 4
  %add.r.i = fadd fast float %_M_value.real.i, %_M_value.real.i.i
  %add.i.i = fadd fast float %_M_value.imag.i, %_M_value.imag.i.i
  store float %add.r.i, ptr %_M_value.realp.i99, align 4
  store float %add.i.i, ptr %_M_value.imagp.i100, align 4
  %_M_value.realp.i107 = getelementptr inbounds %"class.std::complex", ptr %sum_b.fast_red, i64 0, i32 0, i32 0
  %_M_value.real.i108 = load float, ptr %_M_value.realp.i107, align 4
  %_M_value.imagp.i109 = getelementptr inbounds %"class.std::complex", ptr %sum_b.fast_red, i64 0, i32 0, i32 1
  %_M_value.imag.i110 = load float, ptr %_M_value.imagp.i109, align 4
  %sub.r.i = fsub fast float %_M_value.real.i108, %_M_value.real.i.i
  %sub.i.i = fsub fast float %_M_value.imag.i110, %_M_value.imag.i.i
  store float %sub.r.i, ptr %_M_value.realp.i107, align 4
  store float %sub.i.i, ptr %_M_value.imagp.i109, align 4
  %add17 = add nuw i32 %.omp.iv.local.0, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.3

DIR.VPO.END.GUARD.MEM.MOTION.3:                   ; preds = %DIR.VPO.GUARD.MEM.MOTION.1
  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.211

DIR.VPO.END.GUARD.MEM.MOTION.211:                 ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.3
  %cmp19.not.not = icmp ult i32 %add17, 1024
  br i1 %cmp19.not.not, label %DIR.VPO.END.GUARD.MEM.MOTION.2, label %DIR.OMP.END.SIMD.12.loopexit

DIR.OMP.END.SIMD.12.loopexit:                     ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.211
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %loop.region.exit

loop.region.exit:                                 ; preds = %DIR.OMP.END.SIMD.12.loopexit
  %_M_value.real.i.i.i = load float, ptr %sum_a.fast_red, align 4
  %_M_value.imag.i.i.i = load float, ptr %_M_value.imagp.i.i23, align 4
  %_M_value.realp.i.i4 = getelementptr inbounds %"class.std::complex", ptr %sum_a, i64 0, i32 0, i32 0
  %_M_value.real.i.i5 = load float, ptr %_M_value.realp.i.i4, align 4
  %_M_value.imagp.i.i6 = getelementptr inbounds %"class.std::complex", ptr %sum_a, i64 0, i32 0, i32 1
  %_M_value.imag.i.i7 = load float, ptr %_M_value.imagp.i.i6, align 4
  %add.r.i.i = fadd fast float %_M_value.real.i.i5, %_M_value.real.i.i.i
  %add.i.i.i = fadd fast float %_M_value.imag.i.i7, %_M_value.imag.i.i.i
  store float %add.r.i.i, ptr %_M_value.realp.i.i4, align 4
  store float %add.i.i.i, ptr %_M_value.imagp.i.i6, align 4
  %_M_value.real.i.i.i9 = load float, ptr %_M_value.realp.i.i20, align 4
  %_M_value.imag.i.i.i11 = load float, ptr %_M_value.imagp.i.i21, align 4
  %_M_value.realp.i.i14 = getelementptr inbounds %"class.std::complex", ptr %sum_b, i64 0, i32 0, i32 0
  %_M_value.real.i.i15 = load float, ptr %_M_value.realp.i.i14, align 4
  %_M_value.imagp.i.i16 = getelementptr inbounds %"class.std::complex", ptr %sum_b, i64 0, i32 0, i32 1
  %_M_value.imag.i.i17 = load float, ptr %_M_value.imagp.i.i16, align 4
  %add.r.i.i18 = fadd fast float %_M_value.real.i.i15, %_M_value.real.i.i.i9
  %add.i.i.i19 = fadd fast float %_M_value.imag.i.i17, %_M_value.imag.i.i.i11
  store float %add.r.i.i18, ptr %_M_value.realp.i.i14, align 4
  store float %add.i.i.i19, ptr %_M_value.imagp.i.i16, align 4
  br label %DIR.OMP.END.SIMD.12

DIR.OMP.END.SIMD.12:                              ; preds = %loop.region.exit
  ret void
}

