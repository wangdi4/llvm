; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing)' -S %s | FileCheck %s

; Original code:
;class C {
;public:
;  int *_p;
;
;  void foo(int *p) {
;#pragma omp target
;    _p = p;
;  }
;};
;
;C c;
;
;extern void bar() {
;  int x;
;  c.foo(&x);
;}

; Check that we do not set WILOCAL for the FIRSTPRIVATE this pointer.
; FIRSTPRIVATE is misleading here, because this pointer is mapped
; with the map type that conflicts with FIRSTPRIVATE mapping.
; CHECK-NOT: QUAL.OMP.FIRSTPRIVATE:WILOCAL

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.C = type { i32 addrspace(4)* }

$_ZN1C3fooEPi = comdat any

; Function Attrs: convergent mustprogress noinline nounwind
define linkonce_odr hidden spir_func void @_ZN1C3fooEPi(%class.C addrspace(4)* align 8 dereferenceable_or_null(8) %this, i32 addrspace(4)* %p) #0 comdat align 2 {
entry:
  %this.addr = alloca %class.C addrspace(4)*, align 8
  %this.addr.ascast = addrspacecast %class.C addrspace(4)** %this.addr to %class.C addrspace(4)* addrspace(4)*
  %p.addr = alloca i32 addrspace(4)*, align 8
  %p.addr.ascast = addrspacecast i32 addrspace(4)** %p.addr to i32 addrspace(4)* addrspace(4)*
  %p.map.ptr.tmp = alloca i32 addrspace(4)*, align 8
  %p.map.ptr.tmp.ascast = addrspacecast i32 addrspace(4)** %p.map.ptr.tmp to i32 addrspace(4)* addrspace(4)*
  store %class.C addrspace(4)* %this, %class.C addrspace(4)* addrspace(4)* %this.addr.ascast, align 8, !tbaa !8
  store i32 addrspace(4)* %p, i32 addrspace(4)* addrspace(4)* %p.addr.ascast, align 8, !tbaa !12
  %this1 = load %class.C addrspace(4)*, %class.C addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %p.addr.ascast, align 8, !tbaa !12
  %_p = getelementptr inbounds %class.C, %class.C addrspace(4)* %this1, i32 0, i32 0, !intel-tbaa !14
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(%class.C addrspace(4)* %this1, i32 addrspace(4)* addrspace(4)* %_p, i64 8, i64 547, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %0, i32 addrspace(4)* %0, i64 0, i64 544, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %p.map.ptr.tmp.ascast), "QUAL.OMP.FIRSTPRIVATE"(%class.C addrspace(4)* %this1) ]
  store i32 addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %p.map.ptr.tmp.ascast, align 8
  %2 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %p.map.ptr.tmp.ascast, align 8, !tbaa !12
  %_p2 = getelementptr inbounds %class.C, %class.C addrspace(4)* %this1, i32 0, i32 0, !intel-tbaa !14
  store i32 addrspace(4)* %2, i32 addrspace(4)* addrspace(4)* %_p2, align 8, !tbaa !14
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent mustprogress noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 2053, i32 12460685, !"_ZN1C3fooEPi", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 13.0.0"}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSP1C", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"pointer@_ZTSPi", !10, i64 0}
!14 = !{!15, !13, i64 0}
!15 = !{!"struct@_ZTS1C", !13, i64 0}
