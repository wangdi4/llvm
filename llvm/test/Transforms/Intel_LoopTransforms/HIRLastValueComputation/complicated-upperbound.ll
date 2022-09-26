; When the loop upper bound is complicated, the code generation would not be triggered
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -print-after=hir-last-value-computation < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Last Value Computation ***
;Function: conj_grad.DIR.OMP.DISTRIBUTE.PARLOOP.88.split57
;
;<0>          BEGIN REGION { }
;<71>               + DO i1 = 0, sext.i32.i64((1 + %ub.new)) + -1 * sext.i32.i64(%lb.new) + -1, 1   <DO_LOOP>
;<3>                |   %3 = (%rowstr.map.ptr.tmp306.priv.v)[i1 + sext.i32.i64(%lb.new)];
;<6>                |   %4 = (%rowstr.map.ptr.tmp306.priv.v)[i1 + sext.i32.i64(%lb.new) + 1];
;<8>                |   %d.fpriv538.priv.sroa.0.0 = 0;
;<9>                |   if (%4 > %3)
;<9>                |   {
;<15>               |      %7 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD(&((%d.fpriv538.red)[0])),  QUAL.OMP.LINEAR:IV(&((%k.fpriv.linear.iv)[0])1),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
;<16>               |      (%d.fpriv538.red)[0] = 0.000000e+00;
;<18>               |      %d.fpriv538.priv.sroa.0.1 = 0;
;<19>               |      if (-1 * %3 + %4 + -1 >= 0)
;<19>               |      {
;<29>               |         %11 = 0.000000e+00;
;<72>               |
;<72>               |         + DO i2 = 0, zext.i32.i64(smax(1, ((-1 * %3) + %4))) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <simd>
;<32>               |         |   %12 = i2  +  %3;
;<34>               |         |   %13 = (%colidx.map.ptr.tmp307.priv.v)[i2 + sext.i32.i64(%3)];
;<36>               |         |   %14 = (%a.map.ptr.tmp308.priv.v)[i2 + sext.i32.i64(%3)];
;<39>               |         |   %15 = (%z.map.ptr.tmp309.priv.v)[%13];
;<40>               |         |   %mul358 = %14  *  %15;
;<41>               |         |   %11 = %11  +  %mul358;
;<72>               |         + END LOOP
;<72>               |
;<51>               |         (%k.fpriv.linear.iv)[0] = trunc.i64.i32(%12) + 1;
;<52>               |         (%d.fpriv538.red)[0] = %11;
;<53>               |         (%k.fpriv)[0] = trunc.i64.i32(%12) + 1;
;<54>               |         %18 = %11  +  0.000000e+00;
;<55>               |         %19 = bitcast.double.i64(%18);
;<56>               |         %d.fpriv538.priv.sroa.0.1 = %19;
;<19>               |      }
;<59>               |      @llvm.directive.region.exit(%7); [ DIR.OMP.END.SIMD() ]
;<60>               |      %d.fpriv538.priv.sroa.0.0 = %d.fpriv538.priv.sroa.0.1;
;<9>                |   }
;<65>               |   (i64*)(%r.map.ptr.tmp310.priv.v)[i1 + sext.i32.i64(%lb.new)] = %d.fpriv538.priv.sroa.0.0;
;<71>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;Function: conj_grad.DIR.OMP.DISTRIBUTE.PARLOOP.88.split57
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, sext.i32.i64((1 + %ub.new)) + -1 * sext.i32.i64(%lb.new) + -1, 1   <DO_LOOP>
; CHECK:           |   %3 = (%rowstr.map.ptr.tmp306.priv.v)[i1 + sext.i32.i64(%lb.new)];
; CHECK:           |   %4 = (%rowstr.map.ptr.tmp306.priv.v)[i1 + sext.i32.i64(%lb.new) + 1];
; CHECK:           |   %d.fpriv538.priv.sroa.0.0 = 0;
; CHECK:           |   if (%4 > %3)
; CHECK:           |   {
; CHECK:           |      %7 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD(&((%d.fpriv538.red)[0])),  QUAL.OMP.LINEAR:IV(&((%k.fpriv.linear.iv)[0])1),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; CHECK:           |      (%d.fpriv538.red)[0] = 0.000000e+00;
; CHECK:           |      %d.fpriv538.priv.sroa.0.1 = 0;
; CHECK:           |      if (-1 * %3 + %4 + -1 >= 0)
; CHECK:           |      {
; CHECK:           |         %11 = 0.000000e+00;
; CHECK:           |
; CHECK:           |         + DO i2 = 0, zext.i32.i64(smax(1, ((-1 * %3) + %4))) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <simd>
; CHECK:           |         |   %13 = (%colidx.map.ptr.tmp307.priv.v)[i2 + sext.i32.i64(%3)];
; CHECK:           |         |   %mul358 = (%a.map.ptr.tmp308.priv.v)[i2 + sext.i32.i64(%3)]  *  (%z.map.ptr.tmp309.priv.v)[%13];
; CHECK:           |         |   %11 = %11  +  %mul358;
; CHECK:           |         + END LOOP
; CHECK:           |            %12 = zext.i32.i64(smax(1, ((-1 * %3) + %4))) + -1  +  %3;
; CHECK:           |
; CHECK:           |         (%k.fpriv.linear.iv)[0] = trunc.i64.i32(%12) + 1;
; CHECK:           |         (%d.fpriv538.red)[0] = %11;
; CHECK:           |         (%k.fpriv)[0] = trunc.i64.i32(%12) + 1;
; CHECK:           |         %18 = %11  +  0.000000e+00;
; CHECK:           |         %19 = bitcast.double.i64(%18);
; CHECK:           |         %d.fpriv538.priv.sroa.0.1 = %19;
; CHECK:           |      }
; CHECK:           |      @llvm.directive.region.exit(%7); [ DIR.OMP.END.SIMD() ]
; CHECK:           |      %d.fpriv538.priv.sroa.0.0 = %d.fpriv538.priv.sroa.0.1;
; CHECK:           |   }
; CHECK:           |   (i64*)(%r.map.ptr.tmp310.priv.v)[i1 + sext.i32.i64(%lb.new)] = %d.fpriv538.priv.sroa.0.0;
; CHECK:           + END LOOP
; CHECK:     END REGION
;
; ModuleID = 'module'
source_filename = "cg.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, i8* }

@.kmpc_loc.0.0.168 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.170 = external hidden unnamed_addr global %struct.ident_t

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: nounwind
declare void @__kmpc_dist_for_static_init_4(%struct.ident_t*, i32, i32, i32*, i32*, i32*, i32*, i32*, i32, i32) local_unnamed_addr #0

; Function Attrs: nofree nounwind
declare void @__kmpc_for_static_fini(%struct.ident_t* nocapture readonly, i32) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define hidden void @conj_grad.DIR.OMP.DISTRIBUTE.PARLOOP.88.split57(i32* nocapture readonly %tid, i32* nocapture readnone %bid, i32** noalias nocapture readonly %colidx.map.ptr.tmp307.priv, i32** noalias nocapture readonly %rowstr.map.ptr.tmp306.priv, double** noalias nocapture readonly %z.map.ptr.tmp309.priv, double** noalias nocapture readonly %a.map.ptr.tmp308.priv, double** noalias nocapture readonly %r.map.ptr.tmp310.priv, i32* noalias nocapture %k.fpriv, i32* nocapture readnone %tmp3.fpriv, i64 %.omp.lb322.priv.val, i64 %.omp.ub323.priv.val) #2 {
DIR.OMP.DISTRIBUTE.PARLOOP.108:
  %is.last = alloca i32, align 4
  %lower.bnd = alloca i32, align 4
  %upper.bnd = alloca i32, align 4
  %stride = alloca i32, align 4
  %upperD = alloca i32, align 4
  %d.fpriv538.red = alloca double, align 8
  %k.fpriv.linear.iv = alloca i32, align 4
  store i32 0, i32* %is.last, align 4
  %colidx.map.ptr.tmp307.priv.v = load i32*, i32** %colidx.map.ptr.tmp307.priv, align 8, !alias.scope !18, !noalias !23
  %rowstr.map.ptr.tmp306.priv.v = load i32*, i32** %rowstr.map.ptr.tmp306.priv, align 8, !alias.scope !108, !noalias !23
  %z.map.ptr.tmp309.priv.v = load double*, double** %z.map.ptr.tmp309.priv, align 8, !alias.scope !111, !noalias !23
  %a.map.ptr.tmp308.priv.v = load double*, double** %a.map.ptr.tmp308.priv, align 8, !alias.scope !114, !noalias !23
  %r.map.ptr.tmp310.priv.v = load double*, double** %r.map.ptr.tmp310.priv, align 8, !alias.scope !117, !noalias !23
  %0 = trunc i64 %.omp.ub323.priv.val to i32
  %cmp32510 = icmp sgt i32 0, %0
  br i1 %cmp32510, label %omp.precond.end374.exitStub, label %omp.inner.for.body326.lr.ph

omp.precond.end374.exitStub:                      ; preds = %loop.region.exit, %DIR.OMP.DISTRIBUTE.PARLOOP.108
  ret void

omp.inner.for.body326.lr.ph:                      ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.108
  %my.tid = load i32, i32* %tid, align 4
  store i32 0, i32* %lower.bnd, align 4
  store i32 %0, i32* %upper.bnd, align 4
  store i32 1, i32* %stride, align 4
  store i32 %0, i32* %upperD, align 4
  call void @__kmpc_dist_for_static_init_4(%struct.ident_t* nonnull @.kmpc_loc.0.0.168, i32 %my.tid, i32 34, i32* nonnull %is.last, i32* nonnull %lower.bnd, i32* nonnull %upper.bnd, i32* nonnull %upperD, i32* nonnull %stride, i32 1, i32 1)
  %lb.new = load i32, i32* %lower.bnd, align 4
  %ub.new = load i32, i32* %upper.bnd, align 4
  %omp.ztt = icmp sgt i32 %lb.new, %ub.new
  br i1 %omp.ztt, label %loop.region.exit, label %omp.inner.for.body326.preheader

omp.inner.for.body326.preheader:                  ; preds = %omp.inner.for.body326.lr.ph
  %1 = sext i32 %lb.new to i64
  %2 = add nsw i32 %ub.new, 1
  %wide.trip.count17 = sext i32 %2 to i64
  br label %omp.inner.for.body326

omp.inner.for.body326:                            ; preds = %omp.precond.end366, %omp.inner.for.body326.preheader
  %indvars.iv15 = phi i64 [ %1, %omp.inner.for.body326.preheader ], [ %indvars.iv.next16, %omp.precond.end366 ]
  %arrayidx330 = getelementptr inbounds i32, i32* %rowstr.map.ptr.tmp306.priv.v, i64 %indvars.iv15
  %3 = load i32, i32* %arrayidx330, align 4, !tbaa !120, !alias.scope !124, !noalias !125
  %indvars.iv.next16 = add nsw i64 %indvars.iv15, 1
  %arrayidx333 = getelementptr inbounds i32, i32* %rowstr.map.ptr.tmp306.priv.v, i64 %indvars.iv.next16
  %4 = load i32, i32* %arrayidx333, align 4, !tbaa !120, !alias.scope !126, !noalias !125
  %cmp343 = icmp sgt i32 %4, %3
  br i1 %cmp343, label %DIR.OMP.SIMD.110, label %omp.precond.end366

loop.region.exit.loopexit:                        ; preds = %omp.precond.end366
  br label %loop.region.exit

loop.region.exit:                                 ; preds = %loop.region.exit.loopexit, %omp.inner.for.body326.lr.ph
  call void @__kmpc_for_static_fini(%struct.ident_t* nonnull @.kmpc_loc.0.0.170, i32 %my.tid)
  br label %omp.precond.end374.exitStub

omp.precond.end366:                               ; preds = %DIR.OMP.END.SIMD.92, %omp.inner.for.body326
  %d.fpriv538.priv.sroa.0.0 = phi i64 [ %d.fpriv538.priv.sroa.0.1, %DIR.OMP.END.SIMD.92 ], [ 0, %omp.inner.for.body326 ]
  %arrayidx368 = getelementptr inbounds double, double* %r.map.ptr.tmp310.priv.v, i64 %indvars.iv15
  %5 = bitcast double* %arrayidx368 to i64*
  store i64 %d.fpriv538.priv.sroa.0.0, i64* %5, align 8, !tbaa !127, !alias.scope !129, !noalias !130
  %exitcond18 = icmp eq i64 %indvars.iv.next16, %wide.trip.count17
  br i1 %exitcond18, label %loop.region.exit.loopexit, label %omp.inner.for.body326

DIR.OMP.SIMD.110:                                 ; preds = %omp.inner.for.body326
  %6 = xor i32 %3, -1
  %sub339 = add i32 %4, %6
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"(double* %d.fpriv538.red), "QUAL.OMP.LINEAR:IV"(i32* %k.fpriv.linear.iv, i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  store double 0.000000e+00, double* %d.fpriv538.red, align 8
  %cmp34852 = icmp slt i32 %sub339, 0
  br i1 %cmp34852, label %DIR.OMP.END.SIMD.92, label %omp.inner.for.body349.preheader

omp.inner.for.body349.preheader:                  ; preds = %DIR.OMP.SIMD.110
  %8 = sub i32 %4, %3
  %9 = sext i32 %3 to i64
  %10 = icmp sgt i32 %8, 1
  %smax = select i1 %10, i32 %8, i32 1
  %wide.trip.count = zext i32 %smax to i64
  br label %omp.inner.for.body349

DIR.OMP.END.SIMD.92:                              ; preds = %omp.inner.for.cond347.omp.loop.exit365.split_crit_edge.split.split, %DIR.OMP.SIMD.110
  %d.fpriv538.priv.sroa.0.1 = phi i64 [ 0, %DIR.OMP.SIMD.110 ], [ %19, %omp.inner.for.cond347.omp.loop.exit365.split_crit_edge.split.split ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end366

omp.inner.for.body349:                            ; preds = %omp.inner.for.body349, %omp.inner.for.body349.preheader
  %indvars.iv = phi i64 [ 0, %omp.inner.for.body349.preheader ], [ %indvars.iv.next, %omp.inner.for.body349 ]
  %11 = phi double [ 0.000000e+00, %omp.inner.for.body349.preheader ], [ %add359, %omp.inner.for.body349 ]
  %12 = add nsw i64 %indvars.iv, %9
  %arrayidx353 = getelementptr inbounds i32, i32* %colidx.map.ptr.tmp307.priv.v, i64 %12
  %13 = load i32, i32* %arrayidx353, align 4, !tbaa !120, !alias.scope !134, !noalias !125
  %arrayidx355 = getelementptr inbounds double, double* %a.map.ptr.tmp308.priv.v, i64 %12
  %14 = load double, double* %arrayidx355, align 8, !tbaa !127, !alias.scope !135, !noalias !125
  %idxprom356 = sext i32 %13 to i64
  %arrayidx357 = getelementptr inbounds double, double* %z.map.ptr.tmp309.priv.v, i64 %idxprom356
  %15 = load double, double* %arrayidx357, align 8, !tbaa !127, !alias.scope !136, !noalias !125
  %mul358 = fmul reassoc nnan ninf nsz arcp afn double %14, %15
  %add359 = fadd reassoc nnan ninf nsz arcp afn double %11, %mul358
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.inner.for.cond347.omp.loop.exit365.split_crit_edge.split.split, label %omp.inner.for.body349

omp.inner.for.cond347.omp.loop.exit365.split_crit_edge.split.split: ; preds = %omp.inner.for.body349
  %.lcssa = phi i64 [ %12, %omp.inner.for.body349 ]
  %add359.lcssa = phi double [ %add359, %omp.inner.for.body349 ]
  %16 = trunc i64 %.lcssa to i32
  %17 = add i32 %16, 1
  store i32 %17, i32* %k.fpriv.linear.iv, align 4, !tbaa !120, !alias.scope !137, !noalias !138
  store double %add359.lcssa, double* %d.fpriv538.red, align 8, !tbaa !127, !alias.scope !139, !noalias !140
  store i32 %17, i32* %k.fpriv, align 4
  %18 = fadd double %add359.lcssa, 0.000000e+00
  %19 = bitcast double %18 to i64
  br label %DIR.OMP.END.SIMD.92
}

attributes #0 = { nounwind }
attributes #1 = { nofree nounwind }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "mt-func"="true" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }

!omp_offload.info = !{!0, !1, !2, !3, !4, !5, !6, !7, !8, !9, !10, !11, !12, !13, !14, !15}
!llvm.module.flags = !{!16}
!llvm.ident = !{!17}

!0 = !{i32 0, i32 2054, i32 109382464, !"_Z4main", i32 327, i32 4, i32 0}
!1 = !{i32 0, i32 2054, i32 109382464, !"_Z4main", i32 379, i32 6, i32 0}
!2 = !{i32 0, i32 2054, i32 109382464, !"_Z4main", i32 362, i32 5, i32 0}
!3 = !{i32 0, i32 2054, i32 109382464, !"_Z4main", i32 267, i32 0, i32 0}
!4 = !{i32 0, i32 2054, i32 109382464, !"_Z4main", i32 316, i32 3, i32 0}
!5 = !{i32 0, i32 2054, i32 109382464, !"_Z4main", i32 304, i32 2, i32 0}
!6 = !{i32 0, i32 2054, i32 109382464, !"_Z4main", i32 273, i32 1, i32 0}
!7 = !{i32 0, i32 2054, i32 109382464, !"_ZL9conj_grad", i32 573, i32 12, i32 0}
!8 = !{i32 0, i32 2054, i32 109382464, !"_ZL9conj_grad", i32 497, i32 8, i32 0}
!9 = !{i32 0, i32 2054, i32 109382464, !"_ZL9conj_grad", i32 587, i32 13, i32 0}
!10 = !{i32 0, i32 2054, i32 109382464, !"_ZL9conj_grad", i32 601, i32 14, i32 0}
!11 = !{i32 0, i32 2054, i32 109382464, !"_ZL9conj_grad", i32 541, i32 10, i32 0}
!12 = !{i32 0, i32 2054, i32 109382464, !"_ZL9conj_grad", i32 522, i32 9, i32 0}
!13 = !{i32 0, i32 2054, i32 109382464, !"_ZL9conj_grad", i32 484, i32 7, i32 0}
!14 = !{i32 0, i32 2054, i32 109382464, !"_ZL9conj_grad", i32 619, i32 15, i32 0}
!15 = !{i32 0, i32 2054, i32 109382464, !"_ZL9conj_grad", i32 562, i32 11, i32 0}
!16 = !{i32 1, !"wchar_size", i32 4}
!17 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!18 = !{!19, !21}
!19 = distinct !{!19, !20, !"OMPAliasScope"}
!20 = distinct !{!20, !"OMPDomain"}
!21 = distinct !{!21, !22, !"OMPAliasScope"}
!22 = distinct !{!22, !"OMPDomain"}
!23 = !{!24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107}
!24 = distinct !{!24, !20, !"OMPAliasScope"}
!25 = distinct !{!25, !20, !"OMPAliasScope"}
!26 = distinct !{!26, !20, !"OMPAliasScope"}
!27 = distinct !{!27, !20, !"OMPAliasScope"}
!28 = distinct !{!28, !20, !"OMPAliasScope"}
!29 = distinct !{!29, !20, !"OMPAliasScope"}
!30 = distinct !{!30, !20, !"OMPAliasScope"}
!31 = distinct !{!31, !20, !"OMPAliasScope"}
!32 = distinct !{!32, !20, !"OMPAliasScope"}
!33 = distinct !{!33, !20, !"OMPAliasScope"}
!34 = distinct !{!34, !20, !"OMPAliasScope"}
!35 = distinct !{!35, !20, !"OMPAliasScope"}
!36 = distinct !{!36, !20, !"OMPAliasScope"}
!37 = distinct !{!37, !20, !"OMPAliasScope"}
!38 = distinct !{!38, !20, !"OMPAliasScope"}
!39 = distinct !{!39, !20, !"OMPAliasScope"}
!40 = distinct !{!40, !20, !"OMPAliasScope"}
!41 = distinct !{!41, !20, !"OMPAliasScope"}
!42 = distinct !{!42, !20, !"OMPAliasScope"}
!43 = distinct !{!43, !20, !"OMPAliasScope"}
!44 = distinct !{!44, !20, !"OMPAliasScope"}
!45 = distinct !{!45, !20, !"OMPAliasScope"}
!46 = distinct !{!46, !20, !"OMPAliasScope"}
!47 = distinct !{!47, !20, !"OMPAliasScope"}
!48 = distinct !{!48, !20, !"OMPAliasScope"}
!49 = distinct !{!49, !20, !"OMPAliasScope"}
!50 = distinct !{!50, !20, !"OMPAliasScope"}
!51 = distinct !{!51, !20, !"OMPAliasScope"}
!52 = distinct !{!52, !20, !"OMPAliasScope"}
!53 = distinct !{!53, !20, !"OMPAliasScope"}
!54 = distinct !{!54, !20, !"OMPAliasScope"}
!55 = distinct !{!55, !20, !"OMPAliasScope"}
!56 = distinct !{!56, !20, !"OMPAliasScope"}
!57 = distinct !{!57, !20, !"OMPAliasScope"}
!58 = distinct !{!58, !20, !"OMPAliasScope"}
!59 = distinct !{!59, !20, !"OMPAliasScope"}
!60 = distinct !{!60, !20, !"OMPAliasScope"}
!61 = distinct !{!61, !20, !"OMPAliasScope"}
!62 = distinct !{!62, !20, !"OMPAliasScope"}
!63 = distinct !{!63, !22, !"OMPAliasScope"}
!64 = distinct !{!64, !22, !"OMPAliasScope"}
!65 = distinct !{!65, !22, !"OMPAliasScope"}
!66 = distinct !{!66, !22, !"OMPAliasScope"}
!67 = distinct !{!67, !22, !"OMPAliasScope"}
!68 = distinct !{!68, !22, !"OMPAliasScope"}
!69 = distinct !{!69, !22, !"OMPAliasScope"}
!70 = distinct !{!70, !22, !"OMPAliasScope"}
!71 = distinct !{!71, !22, !"OMPAliasScope"}
!72 = distinct !{!72, !22, !"OMPAliasScope"}
!73 = distinct !{!73, !22, !"OMPAliasScope"}
!74 = distinct !{!74, !22, !"OMPAliasScope"}
!75 = distinct !{!75, !22, !"OMPAliasScope"}
!76 = distinct !{!76, !22, !"OMPAliasScope"}
!77 = distinct !{!77, !22, !"OMPAliasScope"}
!78 = distinct !{!78, !22, !"OMPAliasScope"}
!79 = distinct !{!79, !22, !"OMPAliasScope"}
!80 = distinct !{!80, !22, !"OMPAliasScope"}
!81 = distinct !{!81, !22, !"OMPAliasScope"}
!82 = distinct !{!82, !22, !"OMPAliasScope"}
!83 = distinct !{!83, !22, !"OMPAliasScope"}
!84 = distinct !{!84, !22, !"OMPAliasScope"}
!85 = distinct !{!85, !22, !"OMPAliasScope"}
!86 = distinct !{!86, !22, !"OMPAliasScope"}
!87 = distinct !{!87, !22, !"OMPAliasScope"}
!88 = distinct !{!88, !22, !"OMPAliasScope"}
!89 = distinct !{!89, !22, !"OMPAliasScope"}
!90 = distinct !{!90, !22, !"OMPAliasScope"}
!91 = distinct !{!91, !22, !"OMPAliasScope"}
!92 = distinct !{!92, !22, !"OMPAliasScope"}
!93 = distinct !{!93, !22, !"OMPAliasScope"}
!94 = distinct !{!94, !22, !"OMPAliasScope"}
!95 = distinct !{!95, !22, !"OMPAliasScope"}
!96 = distinct !{!96, !22, !"OMPAliasScope"}
!97 = distinct !{!97, !22, !"OMPAliasScope"}
!98 = distinct !{!98, !22, !"OMPAliasScope"}
!99 = distinct !{!99, !22, !"OMPAliasScope"}
!100 = distinct !{!100, !22, !"OMPAliasScope"}
!101 = distinct !{!101, !22, !"OMPAliasScope"}
!102 = distinct !{!102, !22, !"OMPAliasScope"}
!103 = distinct !{!103, !22, !"OMPAliasScope"}
!104 = distinct !{!104, !22, !"OMPAliasScope"}
!105 = distinct !{!105, !22, !"OMPAliasScope"}
!106 = distinct !{!106, !22, !"OMPAliasScope"}
!107 = distinct !{!107, !22, !"OMPAliasScope"}
!108 = !{!109, !110}
!109 = distinct !{!109, !20, !"OMPAliasScope"}
!110 = distinct !{!110, !22, !"OMPAliasScope"}
!111 = !{!112, !113}
!112 = distinct !{!112, !20, !"OMPAliasScope"}
!113 = distinct !{!113, !22, !"OMPAliasScope"}
!114 = !{!115, !116}
!115 = distinct !{!115, !20, !"OMPAliasScope"}
!116 = distinct !{!116, !22, !"OMPAliasScope"}
!117 = !{!118, !119}
!118 = distinct !{!118, !20, !"OMPAliasScope"}
!119 = distinct !{!119, !22, !"OMPAliasScope"}
!120 = !{!121, !121, i64 0}
!121 = !{!"int", !122, i64 0}
!122 = !{!"omnipotent char", !123, i64 0}
!123 = !{!"Simple C/C++ TBAA"}
!124 = !{!62, !57, !55, !47, !107, !102, !100, !92}
!125 = !{!24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !58, !59, !60, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !103, !104, !105}
!126 = !{!61, !56, !54, !46, !106, !101, !99, !91}
!127 = !{!128, !128, i64 0}
!128 = !{!"double", !122, i64 0}
!129 = !{!48, !49, !50, !51, !52, !53, !45, !54, !55, !93, !94, !95, !96, !97, !98, !90, !99, !100}
!130 = !{!19, !24, !109, !25, !26, !112, !27, !115, !28, !118, !29, !131, !132, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !58, !59, !60, !133, !63, !64, !65, !66, !67, !68, !21, !69, !110, !70, !71, !113, !72, !116, !73, !119, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !103, !104, !105}
!131 = distinct !{!131, !20, !"OMPAliasScope"}
!132 = distinct !{!132, !20, !"OMPAliasScope"}
!133 = distinct !{!133, !22, !"OMPAliasScope"}
!134 = !{!51, !44, !96, !89}
!135 = !{!50, !43, !95, !88}
!136 = !{!49, !42, !94, !87}
!137 = !{!58, !59, !103, !104}
!138 = !{!19, !24, !109, !25, !26, !112, !27, !115, !28, !118, !29, !131, !132, !30, !31, !62, !57, !55, !47, !32, !61, !56, !54, !46, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !48, !49, !50, !51, !52, !53, !60, !133, !63, !64, !65, !66, !67, !68, !21, !69, !110, !70, !71, !113, !72, !116, !73, !119, !74, !75, !76, !107, !102, !100, !92, !77, !106, !101, !99, !91, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !93, !94, !95, !96, !97, !98, !105}
!139 = !{!34, !79, !35}
!140 = !{!24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107}
