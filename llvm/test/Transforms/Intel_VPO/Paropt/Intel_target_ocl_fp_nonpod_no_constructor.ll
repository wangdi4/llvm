; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -S %s | FileCheck %s
;
; Original code:
;
;struct C {
;  int x;
;  C() = default;
;  C(const C &other) : x(other.x) {}
;};
;void foo(C c1) {
;#pragma omp target firstprivate(c1)
;  (void)&c1;
;}

;CHECK-NOT: call{{.*}}@_ZTS1C.omp.copy_constr
;CHECK-NOT: call{{.*}}@_ZTS1C.omp.destr

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.C = type { i32 }

$_ZN1CC2ERKS_ = comdat any

; Function Attrs: convergent mustprogress noinline nounwind
define hidden spir_func void @_Z3foo1C(%struct.C addrspace(4)* %c1) #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%struct.C addrspace(4)* %c1, void (%struct.C addrspace(4)*, %struct.C addrspace(4)*)* @_ZTS1C.omp.copy_constr, void (%struct.C addrspace(4)*)* @_ZTS1C.omp.destr), "QUAL.OMP.MAP.TO"(%struct.C addrspace(4)* %c1, %struct.C addrspace(4)* %c1, i64 4, i64 161, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent nounwind
define internal void @_ZTS1C.omp.copy_constr(%struct.C addrspace(4)* %0, %struct.C addrspace(4)* %1) #2 {
entry:
  %.addr = alloca %struct.C addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %struct.C addrspace(4)** %.addr to %struct.C addrspace(4)* addrspace(4)*
  %.addr1 = alloca %struct.C addrspace(4)*, align 8
  %.addr1.ascast = addrspacecast %struct.C addrspace(4)** %.addr1 to %struct.C addrspace(4)* addrspace(4)*
  store %struct.C addrspace(4)* %0, %struct.C addrspace(4)* addrspace(4)* %.addr.ascast, align 8, !tbaa !8
  store %struct.C addrspace(4)* %1, %struct.C addrspace(4)* addrspace(4)* %.addr1.ascast, align 8, !tbaa !8
  %2 = load %struct.C addrspace(4)*, %struct.C addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  %3 = load %struct.C addrspace(4)*, %struct.C addrspace(4)* addrspace(4)* %.addr1.ascast, align 8, !tbaa !8
  call spir_func void @_ZN1CC2ERKS_(%struct.C addrspace(4)* align 4 dereferenceable_or_null(4) %2, %struct.C addrspace(4)* align 4 dereferenceable(4) %3) #3
  ret void
}

; Function Attrs: convergent nounwind
define linkonce_odr hidden spir_func void @_ZN1CC2ERKS_(%struct.C addrspace(4)* align 4 dereferenceable_or_null(4) %this, %struct.C addrspace(4)* align 4 dereferenceable(4) %other) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %struct.C addrspace(4)*, align 8
  %this.addr.ascast = addrspacecast %struct.C addrspace(4)** %this.addr to %struct.C addrspace(4)* addrspace(4)*
  %other.addr = alloca %struct.C addrspace(4)*, align 8
  %other.addr.ascast = addrspacecast %struct.C addrspace(4)** %other.addr to %struct.C addrspace(4)* addrspace(4)*
  store %struct.C addrspace(4)* %this, %struct.C addrspace(4)* addrspace(4)* %this.addr.ascast, align 8, !tbaa !8
  store %struct.C addrspace(4)* %other, %struct.C addrspace(4)* addrspace(4)* %other.addr.ascast, align 8, !tbaa !12
  %this1 = load %struct.C addrspace(4)*, %struct.C addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  %x = getelementptr inbounds %struct.C, %struct.C addrspace(4)* %this1, i32 0, i32 0, !intel-tbaa !14
  %0 = load %struct.C addrspace(4)*, %struct.C addrspace(4)* addrspace(4)* %other.addr.ascast, align 8, !tbaa !12
  %x2 = getelementptr inbounds %struct.C, %struct.C addrspace(4)* %0, i32 0, i32 0, !intel-tbaa !14
  %1 = load i32, i32 addrspace(4)* %x2, align 4, !tbaa !14
  store i32 %1, i32 addrspace(4)* %x, align 4, !tbaa !14
  ret void
}

; Function Attrs: convergent nounwind
define internal spir_func void @_ZTS1C.omp.destr(%struct.C addrspace(4)* %0) #2 section ".text.startup" {
entry:
  %.addr = alloca %struct.C addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %struct.C addrspace(4)** %.addr to %struct.C addrspace(4)* addrspace(4)*
  store %struct.C addrspace(4)* %0, %struct.C addrspace(4)* addrspace(4)* %.addr.ascast, align 8, !tbaa !8
  ret void
}

attributes #0 = { convergent mustprogress noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 2053, i32 13639481, !"_Z3foo1C", i32 7, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSP1C", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"any pointer", !10, i64 0}
!14 = !{!15, !16, i64 0}
!15 = !{!"struct@_ZTS1C", !16, i64 0}
!16 = !{!"int", !10, i64 0}
