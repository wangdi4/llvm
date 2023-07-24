; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Functions voidfunc(), intfunc(), and doublefunc() are both templatized and virtual.
; After their TARGET regions are outlined and the function deleted from device IR,
; check that there is still a declaration left behind, and that the declaraion
; does not have comdat
;
; // C++ test
; template <typename> struct EEE {
;   EEE();
;   virtual void   voidfunc();
;   virtual int    intfunc();
;   virtual double doublefunc();
; };
;
; template <typename f> EEE<f>::EEE() {
;   #pragma omp target
;   ;
; }
;
; template <typename f> void EEE<f>::voidfunc() {
;   #pragma omp target
;   ;
; }
;
; template <typename f> int EEE<f>::intfunc() {
;   #pragma omp target
;   ;
;   return 123;
; }
;
; template <typename f> double EEE<f>::doublefunc() {
;   #pragma omp target
;   ;
;   return 456.0;
; }
;
; template struct EEE<float>;
;
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

$_ZN3EEEIfEC5Ev = comdat any

$_ZN3EEEIfE8voidfuncEv = comdat any

$_ZN3EEEIfE7intfuncEv = comdat any

$_ZN3EEEIfE10doublefuncEv = comdat any

$_ZTV3EEEIfE = comdat any

$_ZTS3EEEIfE = comdat any

$_ZTI3EEEIfE = comdat any

@_ZTV3EEEIfE = weak_odr protected unnamed_addr addrspace(1) constant { [5 x ptr addrspace(4)] } { [5 x ptr addrspace(4)] [ptr addrspace(4) null, ptr addrspace(4) addrspacecast (ptr @_ZTI3EEEIfE to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @_ZN3EEEIfE8voidfuncEv to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @_ZN3EEEIfE7intfuncEv to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr @_ZN3EEEIfE10doublefuncEv to ptr addrspace(4))] }, comdat, align 8
@_ZTVN10__cxxabiv117__class_type_infoE = external addrspace(1) global ptr addrspace(4)
@_ZTS3EEEIfE = weak_odr protected addrspace(1) constant [8 x i8] c"3EEEIfE\00", comdat, align 1
@_ZTI3EEEIfE = weak_odr protected constant { ptr addrspace(4), ptr addrspace(4) } { ptr addrspace(4) getelementptr inbounds (ptr addrspace(4), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv117__class_type_infoE to ptr addrspace(4)), i64 2), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTS3EEEIfE to ptr addrspace(4)) }, comdat, align 8

@_ZN3EEEIfEC1Ev = weak_odr protected unnamed_addr alias void (ptr addrspace(4)), ptr @_ZN3EEEIfEC2Ev

; Function Attrs: convergent noinline nounwind
define weak_odr protected spir_func void @_ZN3EEEIfEC2Ev(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %this) unnamed_addr #0 comdat($_ZN3EEEIfEC5Ev) align 2 {
entry:
  %this.addr = alloca ptr addrspace(4), align 8
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8, !tbaa !11
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  %0 = bitcast ptr addrspace(4) %this1 to ptr addrspace(4)
  store ptr addrspace(4) addrspacecast (ptr addrspace(1) getelementptr inbounds ({ [5 x ptr addrspace(4)] }, ptr addrspace(1) @_ZTV3EEEIfE, i32 0, inrange i32 0, i32 2) to ptr addrspace(4)), ptr addrspace(4) %0, align 8, !tbaa !15
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Check that the function definition of @_ZN3EEEIfE8voidfuncEv is deleted,
; and that there's a declaration without comdat
;
; CHECK-NOT: define {{.*}} void @_ZN3EEEIfE8voidfuncEv
; CHECK: declare protected spir_func void @_ZN3EEEIfE8voidfuncEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8)) unnamed_addr #{{[0-9]+}} align 2
;
; Function Attrs: convergent mustprogress noinline nounwind
define weak_odr protected spir_func void @_ZN3EEEIfE8voidfuncEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca ptr addrspace(4), align 8
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8, !tbaa !11
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Check that the function definition of @_ZN3EEEIfE7intfuncEv is deleted,
; and that there's a declaration without comdat
;
; CHECK-NOT: define {{.*}} i32 @_ZN3EEEIfE7intfuncEv
; CHECK: declare protected spir_func noundef i32 @_ZN3EEEIfE7intfuncEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8)) unnamed_addr #{{[0-9]+}} align 2
;
; Function Attrs: convergent mustprogress noinline nounwind
define weak_odr protected spir_func noundef i32 @_ZN3EEEIfE7intfuncEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %this) unnamed_addr #2 comdat align 2 {
entry:
  %retval = alloca i32, align 4
  %this.addr = alloca ptr addrspace(4), align 8
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8, !tbaa !11
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 123
}

; Check that the function definition of @_ZN3EEEIfE10doublefuncEv is deleted,
; and that there's a declaration without comdat
;
; CHECK-NOT: define {{.*}} double @_ZN3EEEIfE10doublefuncEv
; CHECK: declare protected spir_func noundef double @_ZN3EEEIfE10doublefuncEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8)) unnamed_addr #{{[0-9]+}} align 2
;
; Function Attrs: convergent mustprogress noinline nounwind
define weak_odr protected spir_func noundef double @_ZN3EEEIfE10doublefuncEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %this) unnamed_addr #2 comdat align 2 {
entry:
  %retval = alloca double, align 8
  %this.addr = alloca ptr addrspace(4), align 8
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8, !tbaa !11
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret double 4.560000e+02
}

attributes #0 = { convergent noinline nounwind "approx-func-fp-math"="true" "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent mustprogress noinline nounwind "approx-func-fp-math"="true" "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0, !1, !2, !3}
!llvm.module.flags = !{!4, !5, !6, !7, !8}
!opencl.compiler.options = !{!9}
!llvm.ident = !{!10}

!0 = !{i32 0, i32 57, i32 -687663999, !"_ZN3EEEIfE10doublefuncEv", i32 25, i32 3, i32 0}
!1 = !{i32 0, i32 57, i32 -687663999, !"_ZN3EEEIfE7intfuncEv", i32 19, i32 2, i32 0}
!2 = !{i32 0, i32 57, i32 -687663999, !"_ZN3EEEIfEC1Ev", i32 9, i32 0, i32 0}
!3 = !{i32 0, i32 57, i32 -687663999, !"_ZN3EEEIfE8voidfuncEv", i32 14, i32 1, i32 0}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"openmp", i32 50}
!6 = !{i32 7, !"openmp-device", i32 50}
!7 = !{i32 7, !"PIC Level", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{}
!10 = !{!"clang version 14.0.0"}
!11 = !{!12, !12, i64 0}
!12 = !{!"pointer@_ZTSP3EEEIfE", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C++ TBAA"}
!15 = !{!16, !16, i64 0}
!16 = !{!"vtable pointer", !14, i64 0}
