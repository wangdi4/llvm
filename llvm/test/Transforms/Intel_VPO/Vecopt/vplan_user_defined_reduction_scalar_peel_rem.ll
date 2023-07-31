; Test to verify that VPlan vectorizers remove VPO.GUARD.MEM.MOTION directives
; from outgoing scalar peel/remainder loops.

; RUN: opt -passes=vplan-vec,instcombine -vplan-vec-scenario="s1;v2;s1" -disable-output -print-after=vplan-vec < %s 2>&1 | FileCheck %s -check-prefixes=IR
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>,hir-cg,simplifycfg,sroa,instcombine' -vplan-vec-scenario="s1;v2;s1" -disable-output < %s 2>&1 | FileCheck %s  -check-prefixes=HIR

; ------------------------------------------------------------------------------

; Check LLVM-IR CG.
; IR-LABEL: define hidden void @foo()
; IR-NOT:     [[GUARD:%.*]] = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr [[UDR:%.*]]) ]
; IR-NOT:     call void @llvm.directive.region.exit(token [[GUARD:%.*]]) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]

; ------------------------------------------------------------------------------

; Check HIR CG.
; HIR-LABEL: Function: foo
; HIR:       + DO i1 = 0, %peel.ub, 1   <DO_LOOP>  <MAX_TC_EST = 1>  <LEGAL_MAX_TC = 1> <vector-peel> <nounroll> <novectorize> <max_trip_count = 1>
; HIR-NOT:   |   %guard.start = @llvm.directive.region.entry(); [ DIR.VPO.GUARD.MEM.MOTION(),  QUAL.OMP.LIVEIN(&((%counter_parallel.red)[0])) ] 
; HIR-NOT:   |   @llvm.directive.region.exit(%guard.start); [ DIR.VPO.END.GUARD.MEM.MOTION() ]
; HIR:       + END LOOP

; HIR:       + DO i1 = %lb.tmp, 181, 1   <DO_LOOP>  <MAX_TC_EST = 1>  <LEGAL_MAX_TC = 1> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 1>
; HIR-NOT:   |   %guard.start = @llvm.directive.region.entry(); [ DIR.VPO.GUARD.MEM.MOTION(),  QUAL.OMP.LIVEIN(&((%counter_parallel.red)[0])) ] 
; HIR-NOT:   |   @llvm.directive.region.exit(%guard.start); [ DIR.VPO.END.GUARD.MEM.MOTION() ]
; HIR:       + END LOOP

; ------------------------------------------------------------------------------

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct.std::complex" = type { { double, double } }

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: alwaysinline argmemonly mustprogress nofree norecurse nosync nounwind willreturn uwtable
declare hidden void @.omp_combiner.(ptr noalias nocapture noundef, ptr noalias nocapture noundef readonly)

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
declare hidden noundef ptr @_ZTSSt7complexIdE.omp.def_constr(ptr noundef returned writeonly)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare hidden void @_ZTSSt7complexIdE.omp.destr(ptr nocapture readnone)

; Function Attrs: nofree nounwind readonly
declare dso_local i32 @omp_get_num_threads() local_unnamed_addr

; Function Attrs: uwtable
define hidden void @foo() {
DIR.OMP.SIMD.145:
  %counter_parallel.red = alloca %"struct.std::complex", align 8
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %counter_parallel.red, i8 0, i64 16, i1 false)
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.145
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr %counter_parallel.red, %"struct.std::complex" zeroinitializer, i32 1, ptr @_ZTSSt7complexIdE.omp.def_constr, ptr @_ZTSSt7complexIdE.omp.destr, ptr @.omp_combiner., ptr null) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.3

DIR.VPO.END.GUARD.MEM.MOTION.3:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.2, %DIR.OMP.SIMD.2
  %.omp.iv.local.063 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add3, %DIR.VPO.END.GUARD.MEM.MOTION.2 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.3

DIR.VPO.GUARD.MEM.MOTION.3:                       ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.3
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %counter_parallel.red) ]
  br label %DIR.VPO.GUARD.MEM.MOTION.1

DIR.VPO.GUARD.MEM.MOTION.1:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.3
  %call = call i32 @omp_get_num_threads() #0
  %conv = sitofp i32 %call to double
  %div = fdiv fast double 1.000000e+00, %conv
  %retval.i.sroa.0.0.copyload = load double, ptr %counter_parallel.red, align 8
  %add.r.i.i = fadd fast double %div, %retval.i.sroa.0.0.copyload
  store double %add.r.i.i, ptr %counter_parallel.red, align 8
  %add3 = add nuw nsw i32 %.omp.iv.local.063, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.5

DIR.VPO.END.GUARD.MEM.MOTION.5:                   ; preds = %DIR.VPO.GUARD.MEM.MOTION.1
  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.2

DIR.VPO.END.GUARD.MEM.MOTION.2:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.5
  %exitcond.not = icmp eq i32 %add3, 182
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.4, label %DIR.VPO.END.GUARD.MEM.MOTION.3

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.7

DIR.OMP.END.SIMD.7:                               ; preds = %DIR.OMP.END.SIMD.4
  ret void
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg)

attributes #0 = { nounwind }
