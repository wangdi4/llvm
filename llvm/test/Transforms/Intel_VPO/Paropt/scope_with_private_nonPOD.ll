; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src: codegen for the scope construct -- with private nonPOD

;class A
;{
;public:
;  A();
;};

;void bar();
;void foo() {
;  A aaa;
;  #pragma omp scope private(aaa)
;  {
;    bar();
;  }
;}

; Note: The test IR is hand-generated

; CHECK: %aaa.priv = alloca %class.A, align 1
; CHECK: call void @__kmpc_scope(%struct.ident_t* {{.*}}, i32 {{.*}}, i8* null)
; CHECK-NEXT: %1 = call %class.A* @_ZTS1A.omp.def_constr(%class.A* %aaa.priv)
; CHECK: call void @_ZTS1A.omp.destr(%class.A* %aaa.priv)
; CHECK: call void @__kmpc_end_scope(%struct.ident_t* {{.*}}, i32 {{.*}}, i8* null)
; CHECK:  call void @__kmpc_barrier(%struct.ident_t* {{.*}}, i32 {{.*}})


; ModuleID = 'scope_with_private_nonPOD.cpp'
source_filename = "scope_with_private_nonPOD.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.A = type { i8 }

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %aaa = alloca %class.A, align 1
  call void @_ZN1AC1Ev(%class.A* nonnull align 1 dereferenceable(1) %aaa)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCOPE"(), "QUAL.OMP.PRIVATE:NONPOD"(%class.A* %aaa, %class.A* (%class.A*)* @_ZTS1A.omp.def_constr, void (%class.A*)* @_ZTS1A.omp.destr) ]
  call void @_Z3barv() #2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SCOPE"() ]
  ret void
}

declare dso_local void @_ZN1AC1Ev(%class.A* nonnull align 1 dereferenceable(1)) unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline uwtable
define internal %class.A* @_ZTS1A.omp.def_constr(%class.A* %0) #3 section ".text.startup" {
entry:
  %.addr = alloca %class.A*, align 8
  store %class.A* %0, %class.A** %.addr, align 8
  %1 = load %class.A*, %class.A** %.addr, align 8
  call void @_ZN1AC1Ev(%class.A* nonnull align 1 dereferenceable(1) %1)
  ret %class.A* %1
}

; Function Attrs: noinline uwtable
define internal void @_ZTS1A.omp.destr(%class.A* %0) #3 section ".text.startup" {
entry:
  %.addr = alloca %class.A*, align 8
  store %class.A* %0, %class.A** %.addr, align 8
  ret void
}

declare dso_local void @_Z3barv() #1

attributes #0 = { mustprogress noinline optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}