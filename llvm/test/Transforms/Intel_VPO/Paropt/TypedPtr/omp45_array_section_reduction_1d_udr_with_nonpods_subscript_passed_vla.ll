; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL

; #include <string>
;
; static const int N = 100;
;
; typedef struct my_struct{int a; int b; std::string c; my_struct(): a(0), b(0), c("0") {}; } TYPE;
; bool operator <(const TYPE& t1, const TYPE& t2) { return (t1.a < t2.a) || (t1.b < t2.b) || (t1.c < t2.c); }
;
; void my_init(TYPE& t) { t.a = 1; t.b = 1; t.c = "1"; }
; void my_add(TYPE& lhs, TYPE const &rhs) { lhs.a += rhs.a; lhs.b += rhs.b; lhs.c = std::to_string(static_cast<long long>(std::stoi(lhs.c) + std::stoi(rhs.c)));}
;
; static TYPE y[N];
; static TYPE x[N];
;
; #pragma omp declare reduction (my_reduction_add : TYPE : my_add(omp_out, omp_in)) initializer(my_init(omp_priv))
;
; // Reduction on:         Element of VLA Array passed to function
; // Reduction type:       UDR on NONPOD (with initializer)
; // Array section layout: 1-D
; // Access in body:       EXPR_SUBSCRIPT
; void cq415166_1d_h_pas_vla(int n, TYPE ypas[n]) {
;
; #pragma omp parallel for reduction(my_reduction_add:ypas[0]) num_threads(N)
;     for (int i = 0; i < N; i++) {
;         my_add(ypas[0], x[i]);
;     }
; }
;
; int main() {
;
;     for (int i = 0; i < N; i++) {
;         x[i].a = i; x[i].b = i * i; x[i].c = std::to_string(static_cast<long long>(i * i * i));
;         y[i].a = 0; y[i].b = 0; y[i].c = "0";
;     }
;     cq415166_1d_h_pas_vla(N, y);
;
;     return 0;
; }

; ALL-NOT: "QUAL.OMP.REDUCTION.UDR"
; ALL: red.init.body{{.*}}:
; ALL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.init.body{{.*}} ]
; ALL: call void @.omp_initializer.(%struct.my_struct* %{{.*}}, %struct.my_struct* %{{.*}})
; ALL: br i1 %red.cpy.done{{.*}}, label %red.init.done{{.*}}, label %red.init.body{{.*}}

; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: red.update.body{{.*}}:
; CRITICAL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; CRITICAL: call void @.omp_combiner.(%struct.my_struct* %{{.*}}, %struct.my_struct* %{{.*}})
; CRITICAL: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; CRITICAL: call void @__kmpc_end_critical({{.*}})
; CRITICAL: call void @_ZTS9my_struct.omp.destr(%struct.my_struct* %red.cpy.dest.ptr{{.*}})

; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: red.update.body{{.*}}:
; FASTRED-DAG: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; FASTRED-DAG: call void @.omp_combiner.(%struct.my_struct* %{{.*}}, %struct.my_struct* %{{.*}})
; FASTRED-DAG: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})
; FASTRED-DAG: call void @_ZTS9my_struct.omp.destr(%struct.my_struct* %{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i32, i32, %"class.std::__cxx11::basic_string" }
%"class.std::__cxx11::basic_string" = type { %"struct.std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", i64, %union.anon }
%"struct.std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" = type { i8* }
%union.anon = type { i64, [8 x i8] }

@_ZL1y_43559e1743a563ad451ba81f4d99c671 = internal global [100 x %struct.my_struct] zeroinitializer, align 16
@_ZL1x_43559e1743a563ad451ba81f4d99c671 = internal global [100 x %struct.my_struct] zeroinitializer, align 16

define dso_local void @_Z21cq415166_1d_h_pas_vlaiP9my_struct(i32 %n, %struct.my_struct* %ypas) {
entry:
  %n.addr = alloca i32, align 4
  %ypas.addr = alloca %struct.my_struct*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %omp.vla.tmp = alloca i64, align 8
  %i = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  store %struct.my_struct* %ypas, %struct.my_struct** %ypas.addr, align 8
  %0 = load i32, i32* %n.addr, align 4
  %1 = zext i32 %0 to i64
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  store i64 %1, i64* %omp.vla.tmp, align 8
  %2 = load i64, i64* %omp.vla.tmp, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:ARRSECT"(%struct.my_struct** %ypas.addr, i64 1, i64 0, i64 1, i64 1, i8* null, void (%struct.my_struct*)* @_ZTS9my_struct.omp.destr, void (%struct.my_struct*, %struct.my_struct*)* @.omp_combiner., void (%struct.my_struct*, %struct.my_struct*)* @.omp_initializer.),
    "QUAL.OMP.NUM_THREADS"(i32 100),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i),
    "QUAL.OMP.SHARED"([100 x %struct.my_struct]* @_ZL1x_43559e1743a563ad451ba81f4d99c671),
    "QUAL.OMP.SHARED"(i64* %omp.vla.tmp) ]

  %4 = load i64, i64* %omp.vla.tmp, align 8
  %5 = load i32, i32* %.omp.lb, align 4
  store i32 %5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32* %.omp.iv, align 4
  %7 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %9 = load %struct.my_struct*, %struct.my_struct** %ypas.addr, align 8
  %arrayidx = getelementptr inbounds %struct.my_struct, %struct.my_struct* %9, i64 0
  %10 = load i32, i32* %i, align 4
  %idxprom = sext i32 %10 to i64
  %arrayidx1 = getelementptr inbounds [100 x %struct.my_struct], [100 x %struct.my_struct]* @_ZL1x_43559e1743a563ad451ba81f4d99c671, i64 0, i64 %idxprom
  call void @_Z6my_addR9my_structRKS_(%struct.my_struct* dereferenceable(40) %arrayidx, %struct.my_struct* dereferenceable(40) %arrayidx1)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %11, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

declare void @_Z6my_addR9my_structRKS_(%struct.my_struct* dereferenceable(40) %lhs, %struct.my_struct* dereferenceable(40) %rhs)
declare void @.omp_combiner.(%struct.my_struct* noalias %0, %struct.my_struct* noalias %1)
declare void @_ZTS9my_struct.omp.destr(%struct.my_struct* %0)
declare void @.omp_initializer.(%struct.my_struct* noalias %0, %struct.my_struct* noalias %1)
