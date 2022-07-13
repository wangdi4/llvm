; RUN: opt -passes="early-cse" -S %s | FileCheck %s

; There are 2 calls to hypot() before the OpenMP region, inside it, and after.
; We want to see the duplicate call in each "area" removed, but we don't want
; to see CSE between the areas.
; There should be 3 calls after CSE.
;
; CHECK-LABEL: entry:
; CHECK: call{{.*}}hypot
; CHECK-NOT: call{{.*}}hypot

; CHECK-LABEL: DIR.OMP.PARALLEL.3:
; CHECK: call{{.*}}hypot
; CHECK-NOT: call{{.*}}hypot

; CHECK-LABEL: DIR.OMP.END.PARALLEL.4.split:
; CHECK: call{{.*}}hypot
; CHECK-NOT: call{{.*}}hypot

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local void @foo(float* noundef %p) #0 {
entry:
  %a = alloca float, align 4
  %0 = bitcast float* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store float 0.000000e+00, float* %a, align 4, !tbaa !4
  %call = call fast double @hypot(double noundef 1.000000e+01, double noundef 1.100000e+01) #4
  %1 = load float, float* %a, align 4, !tbaa !4
  %conv = fpext float %1 to double
  %add = fadd fast double %conv, %call
  %conv1 = fptrunc double %add to float
  store float %conv1, float* %a, align 4, !tbaa !4
  %call2 = call fast double @hypot(double noundef 1.000000e+01, double noundef 1.100000e+01) #4
  %2 = load float, float* %a, align 4, !tbaa !4
  %conv3 = fpext float %2 to double
  %add4 = fadd fast double %conv3, %call2
  %conv5 = fptrunc double %add4 to float
  store float %conv5, float* %a, align 4, !tbaa !4
  %a.addr = alloca float*, align 8
  store float* %a, float** %a.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(float* %a), "QUAL.OMP.OPERAND.ADDR"(float* %a, float** %a.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  %cmp = icmp ne i1 %temp.load, false
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %entry
  %a23 = load volatile float*, float** %a.addr, align 8
  %call6 = call fast double @hypot(double noundef 1.000000e+01, double noundef 1.100000e+01) #4
  %4 = load float, float* %a23, align 4, !tbaa !4
  %conv7 = fpext float %4 to double
  %add8 = fadd fast double %conv7, %call6
  %conv9 = fptrunc double %add8 to float
  store float %conv9, float* %a23, align 4, !tbaa !4
  %call10 = call fast double @hypot(double noundef 1.000000e+01, double noundef 1.100000e+01) #4
  %5 = load float, float* %a23, align 4, !tbaa !4
  %conv11 = fpext float %5 to double
  %add12 = fadd fast double %conv11, %call10
  %conv13 = fptrunc double %add12 to float
  store float %conv13, float* %a23, align 4, !tbaa !4
  br label %DIR.OMP.END.PARALLEL.4.split

DIR.OMP.END.PARALLEL.4.split:                     ; preds = %entry, %DIR.OMP.PARALLEL.3
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]
  %call14 = call fast double @hypot(double noundef 1.000000e+01, double noundef 1.100000e+01) #4
  %6 = load float, float* %a, align 4, !tbaa !4
  %conv15 = fpext float %6 to double
  %add16 = fadd fast double %conv15, %call14
  %conv17 = fptrunc double %add16 to float
  store float %conv17, float* %a, align 4, !tbaa !4
  %call18 = call fast double @hypot(double noundef 1.000000e+01, double noundef 1.100000e+01) #4
  %7 = load float, float* %a, align 4, !tbaa !4
  %conv19 = fpext float %7 to double
  %add20 = fadd fast double %conv19, %call18
  %conv21 = fptrunc double %add20 to float
  store float %conv21, float* %a, align 4, !tbaa !4
  %8 = load float, float* %a, align 4, !tbaa !4
  store float %8, float* %p, align 4, !tbaa !4
  %9 = bitcast float* %a to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #3
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind readnone willreturn
declare dso_local double @hypot(double noundef, double noundef) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind readnone willreturn }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!""}
!4 = !{!5, !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
