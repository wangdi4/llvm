; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s

; Original code:
;struct NP {
;  NP(); ~NP();
;};
;void foo() {
;  NP A;
;#pragma omp parallel private(A)
;  (void)A;
;}

; Check that constructor/destructor are not invoked in a loop, since
; only one element of type %struct.NP is privatized:
; ALL: define internal void @_Z3foov.DIR.OMP.PARALLEL
; ALL: [[A_PRIV:%[A-Za-z0-9._]+]] = alloca %struct.NP
; CHECK: call %struct.NP* @_ZTS2NP.omp.def_constr(%struct.NP* [[A_PRIV]])
; CHECK: call void @_ZTS2NP.omp.destr(%struct.NP* [[A_PRIV]])
; OPQPTR: call ptr @_ZTS2NP.omp.def_constr(ptr [[A_PRIV]])
; OPQPTR: call void @_ZTS2NP.omp.destr(ptr [[A_PRIV]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.NP = type { i8 }

; Function Attrs: mustprogress uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %A = alloca %struct.NP, align 1
  %0 = bitcast %struct.NP* %A to i8*
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %0) #3
  call void @_ZN2NPC1Ev(%struct.NP* nonnull align 1 dereferenceable(1) %A)
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE:NONPOD.TYPED"(%struct.NP* %A, %struct.NP zeroinitializer, i32 1, %struct.NP* (%struct.NP*)* @_ZTS2NP.omp.def_constr, void (%struct.NP*)* @_ZTS2NP.omp.destr) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @_ZN2NPD1Ev(%struct.NP* nonnull align 1 dereferenceable(1) %A) #3
  %2 = bitcast %struct.NP* %A to i8*
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %2) #3
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local void @_ZN2NPC1Ev(%struct.NP* nonnull align 1 dereferenceable(1)) unnamed_addr #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: uwtable
define internal %struct.NP* @_ZTS2NP.omp.def_constr(%struct.NP* %0) #4 section ".text.startup" {
entry:
  %.addr = alloca %struct.NP*, align 8
  store %struct.NP* %0, %struct.NP** %.addr, align 8, !tbaa !4
  %1 = load %struct.NP*, %struct.NP** %.addr, align 8
  call void @_ZN2NPC1Ev(%struct.NP* nonnull align 1 dereferenceable(1) %1)
  ret %struct.NP* %1
}

; Function Attrs: uwtable
define internal void @_ZTS2NP.omp.destr(%struct.NP* %0) #4 section ".text.startup" {
entry:
  %.addr = alloca %struct.NP*, align 8
  store %struct.NP* %0, %struct.NP** %.addr, align 8, !tbaa !4
  %1 = load %struct.NP*, %struct.NP** %.addr, align 8
  call void @_ZN2NPD1Ev(%struct.NP* nonnull align 1 dereferenceable(1) %1) #3
  ret void
}

; Function Attrs: nounwind
declare dso_local void @_ZN2NPD1Ev(%struct.NP* nonnull align 1 dereferenceable(1)) unnamed_addr #5

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }
attributes #4 = { uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #5 = { nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 12.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"pointer@_ZTSP2NP", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
