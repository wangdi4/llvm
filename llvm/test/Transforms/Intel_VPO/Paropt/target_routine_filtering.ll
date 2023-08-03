; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -switch-to-offload -S %s | FileCheck %s
;
; When compiling the C++ program below with
;   icpx -O0 -fiopenmp -fopenmp-targets=spir64
; the routine foo() should not be emitted for the target device.
; Only bar() and the offload routine corresponding to the
; outlined target region in main() should be emitted.
;
; #include <iostream>
; void foo() { std::cout << "Error\n"; }
; void bar() {}
; int main() {
;   foo();
;   #pragma omp target
;     bar();
;   return 0;
; }
;

; CHECK-NOT: foo
; CHECK-DAG: define dso_local spir_func void @_Z3barv()
; CHECK-NOT: foo
; CHECK-DAG: define weak dso_local spir_kernel void @__omp_offloading_{{.*}}_main
; CHECK-NOT: foo

; ModuleID = 'target_routine_filtering.cpp'
source_filename = "target_routine_filtering.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

%"class.std::basic_ostream" = type { ptr, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", ptr, i8, i8, ptr, ptr, ptr, ptr }
%"class.std::ios_base" = type { ptr, i64, i64, i32, i32, i32, ptr, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, ptr, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { ptr, ptr, i32, i32 }
%"struct.std::ios_base::_Words" = type { ptr, i64 }
%"class.std::locale" = type { ptr }
%"class.std::locale::_Impl" = type { i32, ptr, i64, ptr, ptr }
%"class.std::locale::facet" = type <{ ptr, i32, [4 x i8] }>
%"class.std::basic_streambuf" = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], ptr, i8, [7 x i8], ptr, ptr, ptr, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ ptr, i32 }>
%struct.__locale_struct = type { [13 x ptr], ptr, ptr, ptr, [13 x ptr] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::num_get" = type { %"class.std::locale::facet.base", [4 x i8] }

$__clang_call_terminate = comdat any

@_ZSt4cout = external dso_local global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [7 x i8] c"Error\0A\00", align 1
@"@tid.addr" = external global i32

; Function Attrs: noinline norecurse optnone uwtable
define dso_local i32 @main() #0 personality ptr bitcast (ptr @__gxx_personality_v0 to ptr) {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call spir_func void @_Z3foov()
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  br label %DIR.OMP.TARGET.11

DIR.OMP.TARGET.11:                                ; preds = %DIR.OMP.TARGET.1
  call spir_func void @_Z3barv()
  br label %invoke.cont

invoke.cont:                                      ; preds = %DIR.OMP.TARGET.11
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  br label %DIR.OMP.END.TARGET.2

DIR.OMP.END.TARGET.2:                             ; preds = %invoke.cont
  ret i32 0
}

; Function Attrs: noinline optnone uwtable
define dso_local spir_func void @_Z3foov() #1 {
entry:
  %call = call spir_func dereferenceable(272) ptr @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(ptr dereferenceable(272) @_ZSt4cout, ptr getelementptr inbounds ([7 x i8], ptr @.str, i64 0, i64 0))
  ret void
}

declare dso_local spir_func dereferenceable(272) ptr @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(ptr dereferenceable(272), ptr) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @_Z3barv() #4 {
entry:
  ret void
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr) #5 comdat {
  %2 = call ptr @__cxa_begin_catch(ptr %0) #3
  call void @_ZSt9terminatev() #6
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr)

declare dso_local void @_ZSt9terminatev()

attributes #0 = { noinline norecurse optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { noinline noreturn nounwind }
attributes #6 = { noreturn nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 0, i32 58, i32 -671282012, !"main", i32 7, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
