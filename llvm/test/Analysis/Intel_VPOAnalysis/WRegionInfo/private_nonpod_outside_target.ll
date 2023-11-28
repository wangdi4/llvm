; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s

; Make sure WRegion parsing doesn't crash while handling
; [FIRST|LAST]PRIVATE:NONPODs with "ptr null" constructor/destructors.
; An example of when the ctor/dtor can be "ptr null" is found during
; device compilation for PRIVATE:NONPOD that's outside of a TARGET region.

; Test src:
;
; class SSS {
;   int aaa;
; };
; int main() {
;   SSS bbb;
; #pragma omp parallel private(bbb)
;   {
; #pragma omp target firstprivate(bbb)
;     {}
;   }
;   return 0;
; }

; The WRN dump looks like this:
; BEGIN PARALLEL ID=1 {
;  ...
;  PRIVATE clause (size=1): NONPOD(ptr addrspace(4) %bbb.ascast, TYPED (TYPE: %class.SSS = type { i32 }, NUM_ELEMENTS: i32 1), CTOR: UNSPECIFIED, DTOR: UNSPECIFIED)
;  ...
; }

; CHECK: PRIVATE clause (size=1): NONPOD({{.*}}, CTOR: UNSPECIFIED, DTOR: UNSPECIFIED)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.SSS = type { i32 }

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %bbb = alloca %class.SSS, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %bbb.ascast = addrspacecast ptr %bbb to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr addrspace(4) %bbb.ascast, %class.SSS zeroinitializer, i32 1, ptr null, ptr null) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %bbb.ascast, %class.SSS zeroinitializer, i32 1),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %bbb.ascast, ptr addrspace(4) %bbb.ascast, i64 4, i64 161, ptr null, ptr null) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1926011756, !"_Z4main", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
