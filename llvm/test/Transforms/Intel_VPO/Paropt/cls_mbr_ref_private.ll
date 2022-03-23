; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; typedef short TYPE;
;
; static TYPE x[10];
; static TYPE y;
;
; typedef struct my_struct{TYPE &yref; void work(); my_struct(TYPE &y) : yref(y) {}; } MY_STRUCT_TYPE;
;
; void MY_STRUCT_TYPE::work() {
;
; #pragma omp parallel private(yref)
;   {
;     yref = x[1];
;   }
; }
;
; ModuleID = 'cls_mbr_ref_private.cpp'
source_filename = "cls_mbr_ref_private.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i16* }

@_ZL1x = internal global [10 x i16] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_ZN9my_struct4workEv(%struct.my_struct* %this) #0 align 2 {
entry:
  %this.addr = alloca %struct.my_struct*, align 8
  store %struct.my_struct* %this, %struct.my_struct** %this.addr, align 8
  %this1 = load %struct.my_struct*, %struct.my_struct** %this.addr, align 8
  %yref = getelementptr inbounds %struct.my_struct, %struct.my_struct* %this1, i32 0, i32 0
  %0 = load i16*, i16** %yref, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i16* %0), "QUAL.OMP.SHARED"([10 x i16]* @_ZL1x) ]

; Check that a local copy is created for %0, and the store of the gep on
; @_ZL1x happens to that local copy.
; CHECK: [[PRIV_COPY:%[^ ]+]] = alloca i16
; CHECK: [[X_GEP_LOAD:%[^ ]+]] = load i16, i16* getelementptr inbounds ([10 x i16], [10 x i16]* @_ZL1x, i64 0, i64 1)
; CHECK: store i16 [[X_GEP_LOAD]], i16* [[PRIV_COPY]]
  %2 = load i16, i16* getelementptr inbounds ([10 x i16], [10 x i16]* @_ZL1x, i64 0, i64 1), align 2
  store i16 %2, i16* %0, align 2


  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
