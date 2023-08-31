; RUN: opt -vpo-paropt-fast-reduction-ctrl=3 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=FRCTRL3
; RUN: opt -vpo-paropt-fast-reduction-ctrl=3 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=FRCTRL3
; RUN: opt -vpo-paropt-fast-reduction-ctrl=7 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=FRCTRL7
; RUN: opt -vpo-paropt-fast-reduction-ctrl=7 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=FRCTRL7

; // C++ source:
;class I {
; int l;
; int m;
; int n;
;
;public:
; void operator+= (I &v ) { this->l+=v.l; }
;};
;
;int main() {
; I s;
; I& rs = s;
;
;#pragma omp declare reduction(merge:class I: omp_out += omp_in)
;#pragma omp parallel reduction(merge:rs)
; {
; }
;}

; With -vpo-paropt-fast-reduction-ctrl=3 no extra copy for NONPOD reduction is created and instead the member of
; the fast_reduction struct is used directly inside the region
;FRCTRL3-NOT: %{{.*}}.red = alloca %class.{{.*}}, align {{.*}}
;FRCTRL3-NOT: %{{.*}}.red.ref = alloca ptr, align {{.*}}
;FRCTRL3-NOT: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %{{.*}}.fast_red, ptr align 4 %{{.*}}.red, i64 12, i1 false)
;FRCTRL3: call void @_ZTS1I.omp.destr(ptr %{{.*}}s.fast_red)

; With -vpo-paropt-fast-reduction-ctrl=7 an extra copy for NONPOD reduction is created and used inside the region.
; The value of the extra copy is copied with memcpy after the region to the member of the fast_reduction struct.
;FRCTRL7: %fast_red_struct = alloca %struct.fast_red_t, align 8
;FRCTRL7-NEXT: %{{.*}}.red = alloca %class.{{.*}}, align {{.*}}
;FRCTRL7-NEXT: %{{.*}}.red.ref = alloca ptr, align {{.*}}
;FRCTRL7: %[[PTR:[^ ]+]] = load ptr, ptr %{{.*}}.fast_red.ref, align 8
;FRCTRL7: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %[[PTR]], ptr align 4 %{{.*}}.red, i64 12, i1 false)
;FRCTRL7: call void @_ZTS1I.omp.destr(ptr %{{.*}}.fast_red)

%class.I = type { i32, i32, i32 }

define dso_local noundef i32 @main() {
entry:
  %s = alloca %class.I, align 4
  %rs = alloca ptr, align 8
  store ptr %s, ptr %rs, align 8
  %0 = load ptr, ptr %rs, align 8

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.UDR:BYREF.TYPED"(ptr %rs, %class.I zeroinitializer, i32 1, ptr @_ZTS1I.omp.def_constr, ptr @_ZTS1I.omp.destr, ptr @.omp_combiner., ptr null) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry() 
declare void @llvm.directive.region.exit(token) 
declare void @.omp_combiner.(ptr noalias noundef %0, ptr noalias noundef %1) 
declare noundef ptr @_ZTS1I.omp.def_constr(ptr noundef %0) 
declare void @_ZTS1I.omp.destr(ptr noundef %0) 

