; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="function(loop-simplify,vpo-cfg-restructuring),vpo-paropt" -print-after-all -S %s | FileCheck %s

; This file checks whether the ssa update can generate correct code if
; there is only one live-in value.
;
; typedef double MMD_float;
; typedef int MMD_int;
;
; MMD_float Thermo::temperature(Atom &atom)
; {
;   MMD_int i;
;   MMD_float vx, vy, vz;
;
;   MMD_float t = 0.0;
;   t_act = 0;
; #pragma omp barrier
;
;  MMD_float* v = &atom.v[0][0];
; #pragma omp for schedule(static,666)
;
;  for(i = 0; i < atom.nlocal; i++) {
;     vx = v[i * 3 + 0];
;     vy = v[i * 3 + 1];
;     vz = v[i * 3 + 2];
;     t += (vx * vx + vy * vy + vz * vz) * atom.mass;
;   }
;
; #pragma omp atomic
;  t_act += t;
;
; #pragma omp barrier
;
;  MMD_float t1;
; #pragma omp master
;  {
;     if(sizeof(MMD_float) == 4)
;       t1 = t_act;
;
;     else
;       t1 = t_act;
;
;
;   }
;   return t1 * t_scale;
; }
;
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, i8* }
%class.Thermo = type { i32, i32, i32, i32*, double*, double*, double*, double, double, double, double, double, double, double, double, %class.ThreadData*, double }
%class.ThreadData = type { i32, i32, i32, i32 }
%class.Atom = type { i32, i32, i32, i32, double**, double**, double**, double**, %class.ThreadData*, double, double, i32, i32, i32, %struct.Box }
%struct.Box = type { double, double, double, double, double, double, double, double, double }

@"@tid.addr" = external global i32
@.kmpc_loc.0.0.6 = external hidden unnamed_addr constant %struct.ident_t
@.kmpc_loc.0.0.8 = external hidden unnamed_addr constant %struct.ident_t
@.kmpc_loc.0.0.10 = external hidden unnamed_addr constant %struct.ident_t
@.kmpc_loc.0.0.12 = external hidden unnamed_addr constant %struct.ident_t
@.kmpc_loc.0.0.14 = external hidden unnamed_addr constant %struct.ident_t

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind uwtable
define dso_local double @_ZN6Thermo11temperatureER4Atom(%class.Thermo* %this, %class.Atom* dereferenceable(160) %atom) #2 align 2 {
entry:
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #1
  %t_act = getelementptr inbounds %class.Thermo, %class.Thermo* %this, i32 0, i32 7
  store double 0.000000e+00, double* %t_act, align 8, !tbaa !2
  %my.tid37 = load i32, i32* @"@tid.addr", align 4
  call void @__kmpc_barrier(%struct.ident_t* @.kmpc_loc.0.0.14, i32 %my.tid37)
  %v2 = getelementptr inbounds %class.Atom, %class.Atom* %atom, i32 0, i32 5
  %1 = load double**, double*** %v2, align 8, !tbaa !11
  %2 = load double*, double** %1, align 8, !tbaa !15
  %3 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #1
  %nlocal = getelementptr inbounds %class.Atom, %class.Atom* %atom, i32 0, i32 1
  %4 = load i32, i32* %nlocal, align 4, !tbaa !16
  %sub6 = sub nsw i32 %4, 1
  %cmp = icmp slt i32 0, %4
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %5 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #1
  store i32 0, i32* %.omp.lb, align 4, !tbaa !17
  %6 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #1
  store i32 %sub6, i32* %.omp.ub, align 4, !tbaa !17
  br label %DIR.OMP.LOOP.4

DIR.OMP.LOOP.4:                                   ; preds = %omp.precond.then
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 666), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %8 = load i32, i32* %.omp.lb, align 4, !tbaa !17
  store volatile i32 %8, i32* %.omp.iv, align 4, !tbaa !17
  %9 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !17
  %10 = load i32, i32* %.omp.ub, align 4, !tbaa !17
  %cmp833 = icmp sle i32 %9, %10
  br i1 %cmp833, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.4
  %t.0 = phi double [ 0.000000e+00, %DIR.OMP.LOOP.4 ], [ %add27, %omp.inner.for.body ]
  %11 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !17
  store i32 %11, i32* %i, align 4, !tbaa !17
  %mul10 = mul nsw i32 %11, 3
  %idxprom = sext i32 %mul10 to i64
  %arrayidx12 = getelementptr inbounds double, double* %2, i64 %idxprom
  %12 = load double, double* %arrayidx12, align 8, !tbaa !18
  %add14 = add nsw i32 %mul10, 1
  %idxprom15 = sext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds double, double* %2, i64 %idxprom15
  %13 = load double, double* %arrayidx16, align 8, !tbaa !18
  %add18 = add nsw i32 %mul10, 2
  %idxprom19 = sext i32 %add18 to i64
  %arrayidx20 = getelementptr inbounds double, double* %2, i64 %idxprom19
  %14 = load double, double* %arrayidx20, align 8, !tbaa !18
  %mul21 = fmul double %12, %12
  %mul22 = fmul double %13, %13
  %add23 = fadd double %mul21, %mul22
  %mul24 = fmul double %14, %14
  %add25 = fadd double %add23, %mul24
  %mass = getelementptr inbounds %class.Atom, %class.Atom* %atom, i32 0, i32 10
  %15 = load double, double* %mass, align 8, !tbaa !19
  %mul26 = fmul double %add25, %15
  %add27 = fadd double %t.0, %mul26
  %16 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !17
  %add28 = add nsw i32 %16, 1
  store volatile i32 %add28, i32* %.omp.iv, align 4, !tbaa !17
  %17 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !17
  %18 = load i32, i32* %.omp.ub, align 4, !tbaa !17
  %cmp8 = icmp sle i32 %17, %18
  br i1 %cmp8, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.4
  %t.1 = phi double [ %add27, %omp.inner.for.body ], [ 0.000000e+00, %DIR.OMP.LOOP.4 ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %t.2 = phi double [ %t.1, %omp.loop.exit ], [ 0.000000e+00, %entry ]
  %19 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #1
  %20 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %3) #1
  %my.tid36 = load i32, i32* @"@tid.addr", align 4
  call void @__kmpc_atomic_float8_add(%struct.ident_t* @.kmpc_loc.0.0.12, i32 %my.tid36, double* %t_act, double %t.2)
  %my.tid35 = load i32, i32* @"@tid.addr", align 4
  call void @__kmpc_barrier(%struct.ident_t* @.kmpc_loc.0.0.10, i32 %my.tid35)
  %my.tid = load i32, i32* @"@tid.addr", align 4
  %21 = call i32 @__kmpc_master(%struct.ident_t* @.kmpc_loc.0.0.6, i32 %my.tid)
  %22 = icmp eq i32 %21, 1
  br i1 %22, label %if.then.master.3, label %DIR.OMP.END.MASTER.15

if.then.master.3:                                 ; preds = %omp.precond.end
  %23 = load double, double* %t_act, align 8, !tbaa !2
  %my.tid34 = load i32, i32* @"@tid.addr", align 4
  call void @__kmpc_end_master(%struct.ident_t* @.kmpc_loc.0.0.8, i32 %my.tid34)
  br label %DIR.OMP.END.MASTER.15

DIR.OMP.END.MASTER.15:                            ; preds = %if.then.master.3, %omp.precond.end
  %t1.0 = phi double [ %23, %if.then.master.3 ], [ undef, %omp.precond.end ]
  %t_scale = getelementptr inbounds %class.Thermo, %class.Thermo* %this, i32 0, i32 10
  %24 = load double, double* %t_scale, align 8, !tbaa !20
  %mul32 = fmul double %t1.0, %24
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #1
  ret double %mul32
}

declare i32 @__kmpc_master(%struct.ident_t*, i32)

declare void @__kmpc_end_master(%struct.ident_t*, i32)

declare void @__kmpc_barrier(%struct.ident_t*, i32)

declare void @__kmpc_atomic_float8_add(%struct.ident_t*, i32, double*, double)

attributes #0 = { argmemonly nounwind }
attributes #1 = { nounwind }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0"}
!2 = !{!3, !9, i64 48}
!3 = !{!"struct@_ZTS6Thermo", !4, i64 0, !4, i64 4, !4, i64 8, !7, i64 16, !8, i64 24, !8, i64 32, !8, i64 40, !9, i64 48, !9, i64 56, !9, i64 64, !9, i64 72, !9, i64 80, !9, i64 88, !9, i64 96, !9, i64 104, !10, i64 112, !9, i64 120}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!"pointer@_ZTSPi", !5, i64 0}
!8 = !{!"pointer@_ZTSPd", !5, i64 0}
!9 = !{!"double", !5, i64 0}
!10 = !{!"unspecified pointer", !5, i64 0}
!11 = !{!12, !13, i64 24}
!12 = !{!"struct@_ZTS4Atom", !4, i64 0, !4, i64 4, !4, i64 8, !4, i64 12, !13, i64 16, !13, i64 24, !13, i64 32, !13, i64 40, !10, i64 48, !9, i64 56, !9, i64 64, !4, i64 72, !4, i64 76, !4, i64 80, !14, i64 88}
!13 = !{!"pointer@_ZTSPPd", !5, i64 0}
!14 = !{!"struct@_ZTS3Box", !9, i64 0, !9, i64 8, !9, i64 16, !9, i64 24, !9, i64 32, !9, i64 40, !9, i64 48, !9, i64 56, !9, i64 64}
!15 = !{!8, !8, i64 0}
!16 = !{!12, !4, i64 4}
!17 = !{!4, !4, i64 0}
!18 = !{!9, !9, i64 0}
!19 = !{!12, !9, i64 64}
!20 = !{!3, !9, i64 72}

; CHECK-LABEL: dispatch.header
; CHECK: %add272 = phi double [ %{{.*}}, %{{.*}} ], [ 0.000000e+00, %{{.*}} ]

