; RUN: sycl-post-link --ir-output-only -ompoffload-link-entries %s -S -o - | \
; RUN:   FileCheck %s -check-prefixes=LINK
; RUN: sycl-post-link --ir-output-only -ompoffload-link-entries \
; RUN:   --ompoffload-make-globals-static %s -S -o - | \
; RUN:   FileCheck %s -check-prefixes=LINK,STATIC
; RUN: sycl-post-link --ir-output-only -ompoffload-link-entries \
; RUN:   --ompoffload-sort-entries %s -S -o - | \
; RUN:   FileCheck %s -check-prefixes=SORTED

; LINK-NOT: @.omp_offloading.entry._ZN1B1XE
; LINK-NOT: @.omp_offloading.entry._ZN1A1XE
; LINK-NOT: @.omp_offloading.entry._Z10SomeGlobal
; LINK-NOT: @.omp_offloading.entry._Z13AnotherGlobal
; LINK-NOT: @.omp_offloading.entry.__omp_offloading_806_6ee1866__Z3foov_l10
; LINK-DAG: @__omp_offloading_entries_table = addrspace(2) constant [5 x %struct.__tgt_offload_entry]
; LINK-DAG: @__omp_offloading_entries_table_size = addrspace(2) constant i64 200
; STATIC-DAG: @_ZN1B1XE ={{.*}}internal
; STATIC-DAG: @_ZN1A1XE ={{.*}}internal
; STATIC-DAG: @SomeGlobal ={{.*}}internal
; STATIC-DAG: @AnotherGlobal ={{.*}}internal

; SORTED-DAG: @.omp_offloading.entry_name.2 ={{.*}}c"_Z10SomeGlobal\00"
; SORTED-DAG: @.omp_offloading.entry_name.3 ={{.*}}c"_Z13AnotherGlobal\00"
; SORTED-DAG: @.omp_offloading.entry_name.1 ={{.*}}c"_ZN1A1XE\00"
; SORTED-DAG: @.omp_offloading.entry_name = {{.*}}c"_ZN1B1XE\00"
; SORTED-DAG: @.omp_offloading.entry_name.4 ={{.*}}c"__omp_offloading_806_6ee1866__Z3foov_l10\00"

; SORTED: @__omp_offloading_entries_table = addrspace(2) constant [5 x %struct.__tgt_offload_entry]
; SORTED: @SomeGlobal{{.*}}@.omp_offloading.entry_name.2
; SORTED: @AnotherGlobal{{.*}}@.omp_offloading.entry_name.3
; SORTED: @_ZN1A1XE{{.*}}@.omp_offloading.entry_name.1
; SORTED: @_ZN1B1XE{{.*}}@.omp_offloading.entry_name{{[^.]}}
; SORTED: null{{.*}}@.omp_offloading.entry_name.4
; SORTED: @__omp_offloading_entries_table_size = addrspace(2) constant i64 200

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

%struct.__tgt_offload_entry = type { i8 addrspace(4)*, i8 addrspace(2)*, i64, i32, i32, i64 }

@_ZN1B1XE = hidden target_declare addrspace(1) global i32 0, align 4
@_ZN1A1XE = hidden target_declare addrspace(1) global i32 0, align 4
@SomeGlobal = hidden target_declare addrspace(1) global i32 0, align 4
@AnotherGlobal = hidden target_declare addrspace(1) global i32 0, align 4
@.omp_offloading.entry_name = internal target_declare unnamed_addr addrspace(2) constant [9 x i8] c"_ZN1B1XE\00"
@.omp_offloading.entry._ZN1B1XE = weak target_declare addrspace(2) constant %struct.__tgt_offload_entry { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @_ZN1B1XE to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(2)* @.omp_offloading.entry_name, i32 0, i32 0), i64 4, i32 0, i32 0, i64 9 }, section "omp_offloading_entries"
@.omp_offloading.entry_name.1 = internal target_declare unnamed_addr addrspace(2) constant [9 x i8] c"_ZN1A1XE\00"
@.omp_offloading.entry._ZN1A1XE = weak target_declare addrspace(2) constant %struct.__tgt_offload_entry { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @_ZN1A1XE to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(2)* @.omp_offloading.entry_name.1, i32 0, i32 0), i64 4, i32 0, i32 0, i64 9 }, section "omp_offloading_entries"
@.omp_offloading.entry_name.2 = internal target_declare unnamed_addr addrspace(2) constant [15 x i8] c"_Z10SomeGlobal\00"
@.omp_offloading.entry._Z10SomeGlobal = weak target_declare addrspace(2) constant %struct.__tgt_offload_entry { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @SomeGlobal to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([15 x i8], [15 x i8] addrspace(2)* @.omp_offloading.entry_name.2, i32 0, i32 0), i64 4, i32 0, i32 0, i64 15 }, section "omp_offloading_entries"
@.omp_offloading.entry_name.3 = internal target_declare unnamed_addr addrspace(2) constant [18 x i8] c"_Z13AnotherGlobal\00"
@.omp_offloading.entry._Z13AnotherGlobal = weak target_declare addrspace(2) constant %struct.__tgt_offload_entry { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @AnotherGlobal to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([18 x i8], [18 x i8] addrspace(2)* @.omp_offloading.entry_name.3, i32 0, i32 0), i64 4, i32 0, i32 0, i64 18 }, section "omp_offloading_entries"
@.omp_offloading.entry_name.4 = internal target_declare unnamed_addr addrspace(2) constant [41 x i8] c"__omp_offloading_806_6ee1866__Z3foov_l10\00"
@.omp_offloading.entry.__omp_offloading_806_6ee1866__Z3foov_l10 = weak target_declare addrspace(2) constant %struct.__tgt_offload_entry { i8 addrspace(4)* null, i8 addrspace(2)* getelementptr inbounds ([41 x i8], [41 x i8] addrspace(2)* @.omp_offloading.entry_name.4, i32 0, i32 0), i64 0, i32 0, i32 0, i64 41 }, section "omp_offloading_entries"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_kernel void @__omp_offloading_806_6ee1866__Z3foov_l10() #0 {
newFuncRoot:
  br label %DIR.OMP.TARGET.2.split

DIR.OMP.END.TARGET.4.exitStub:                    ; preds = %DIR.OMP.TARGET.3.split
  ret void

DIR.OMP.TARGET.2.split:                           ; preds = %newFuncRoot
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2.split
  br label %DIR.OMP.TARGET.3.split

DIR.OMP.TARGET.3.split:                           ; preds = %DIR.OMP.TARGET.3
  br label %DIR.OMP.END.TARGET.4.exitStub
}

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target.declare"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!spirv.Source = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{}
!2 = !{!"clang version 9.0.0"}
!3 = !{i32 4, i32 200000}
