; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s

; Make sure WRegion parsing doesn't crash while handling
; [FIRST|LAST]PRIVATE:NONPODs with "i8* null" constructor/destructors.
; An example of when the ctor/dtor can be "i8* null" is found during
; device compilation for PRIVATE:NONPOD that's outside of a TARGET region.

;
; // test's C source
; struct SSS {
;   int aaa;
; };
; int main() {
;     SSS bbb;
;     #pragma omp parallel private(bbb)
;     {
;         #pragma omp target firstprivate(bbb)
;         {
;         }
;     }
;     return 0;
; }

; The WRN dump looks like this:
; BEGIN PARALLEL ID=1 {
;  ...
;  PRIVATE clause (size=1): NONPOD(%struct.SSS addrspace(4)* %bbb.ascast, CTOR: UNSPECIFIED, DTOR: UNSPECIFIED)
;  ...
; }
;
; CHECK: PRIVATE clause (size=1): NONPOD({{.*}}, CTOR: UNSPECIFIED, DTOR: UNSPECIFIED)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.SSS = type { i32 }

; Function Attrs: noinline norecurse nounwind optnone mustprogress
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %bbb = alloca %struct.SSS, align 4
  %bbb.ascast = addrspacecast %struct.SSS* %bbb to %struct.SSS addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE:NONPOD"(%struct.SSS addrspace(4)* %bbb.ascast, i8* null, i8* null) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(%struct.SSS addrspace(4)* %bbb.ascast), "QUAL.OMP.MAP.TO"(%struct.SSS addrspace(4)* %bbb.ascast, %struct.SSS addrspace(4)* %bbb.ascast, i64 4, i64 161, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline norecurse nounwind optnone mustprogress "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}

!0 = !{i32 0, i32 57, i32 -693529316, !"_Z4main", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
