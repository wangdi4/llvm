; At -O3, we cannot forward substitute "alpha" for "tmpcast" in the task
; region, without considering the target clause and the entire enclosing
; target region.

; RUN: opt -S -xmain-opt-level=3 -vpo-cfg-restructuring -vpo-paropt %s | FileCheck %s
; CHECK: tmpcast = bitcast {{.*}} %alpha
; CHECK: call {{.*}}tgt_target_teams
; CHECK: call {{.*}}kmpc_fork_teams{{.*}}tmpcast

; #include <stdio.h>
;
; #define MAX 100
;
; typedef float TYPE;
; struct cmplx { float x,y; };
; TYPE A[MAX];
;
; void Compute(cmplx alpha)
; {
;   #pragma omp target teams map(tofrom: A)
;   for (int k = 0; k < MAX; k++) {
;       A[k] += alpha.x;
;   }
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.cmplx = type { float, float }

@A = dso_local global [100 x float] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: nounwind uwtable
define dso_local void @_Z7Compute5cmplx(<2 x float> %alpha.coerce) local_unnamed_addr #0 {
entry:
  %alpha = alloca <2 x float>, align 8
  %tmpcast = bitcast <2 x float>* %alpha to %struct.cmplx*
  %k = alloca i32, align 4
  store <2 x float> %alpha.coerce, <2 x float>* %alpha, align 8
  %end.dir.temp11 = alloca i1, align 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([100 x float]* @A, [100 x float]* @A, i64 400, i64 35), "QUAL.OMP.MAP.TOFROM"(%struct.cmplx* %tmpcast, %struct.cmplx* %tmpcast, i64 8, i64 547), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp11) ]
  %temp.load12 = load volatile i1, i1* %end.dir.temp11, align 1
  br i1 %temp.load12, label %DIR.OMP.END.TEAMS.6.split, label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %entry
  %end.dir.temp = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"([100 x float]* @A), "QUAL.OMP.SHARED"(%struct.cmplx* %tmpcast), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %for.end.split, label %DIR.OMP.TEAMS.5

DIR.OMP.TEAMS.5:                                  ; preds = %DIR.OMP.TARGET.3
  %2 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #1
  br label %for.cond

for.cond:                                         ; preds = %for.body, %DIR.OMP.TEAMS.5
  %storemerge = phi i32 [ 0, %DIR.OMP.TEAMS.5 ], [ %inc, %for.body ]
  store i32 %storemerge, i32* %k, align 4, !tbaa !3
  %cmp = icmp slt i32 %storemerge, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #1
  br label %for.end.split

for.body:                                         ; preds = %for.cond
  %x = getelementptr inbounds %struct.cmplx, %struct.cmplx* %tmpcast, i64 0, i32 0, !intel-tbaa !7
  %3 = load float, float* %x, align 4, !tbaa !7
  %idxprom = sext i32 %storemerge to i64
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %idxprom, !intel-tbaa !10
  %4 = load float, float* %arrayidx, align 4, !tbaa !10
  %add = fadd float %4, %3
  store float %add, float* %arrayidx, align 4, !tbaa !10
  %5 = load i32, i32* %k, align 4, !tbaa !3
  %inc = add nsw i32 %5, 1
  br label %for.cond

for.end.split:                                    ; preds = %DIR.OMP.TARGET.3, %for.cond.cleanup
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.6.split

DIR.OMP.END.TEAMS.6.split:                        ; preds = %entry, %for.end.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: uwtable
define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="64" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind willreturn }
attributes #3 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 64771, i32 536872841, !"_Z7Compute5cmplx", i32 11, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !9, i64 0}
!8 = !{!"struct@_ZTS5cmplx", !9, i64 0, !9, i64 4}
!9 = !{!"float", !5, i64 0}
!10 = !{!11, !9, i64 0}
!11 = !{!"array@_ZTSA100_f", !9, i64 0}
