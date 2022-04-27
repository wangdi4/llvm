; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s

; CHECK: @main()
; CHECK-NOT: load{{.*}} %a
; CHECK-NOT: store{{.*}} %a.priv
; CHECK: @__omp_offloading
; CHECK: store{{.*}} 5, i32* %a
; CHECK: @main.DIR.OMP.TASK
; CHECK-NOT: store{{.*}}%a.gep

; #include <stdio.h>
;
; int main() {
; int a;
; #pragma omp task firstprivate(a)
; #pragma omp target map(tofrom:a)
;   {
;     a = 5;
;   }
;   return 0;
; }

; The %a alloca is firstprivate but undefined outside the region. CE sinks it
; into the task region, and we should not try to copy its value to the task
; object. That slot in the task object will remain uninitialized.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %a = alloca i32, align 4
  %0 = bitcast i32* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %end.dir.temp5 = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %a), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp5) ]
  %temp.load6 = load volatile i1, i1* %end.dir.temp5, align 1
  br i1 %temp.load6, label %DIR.OMP.END.TARGET.7.split, label %DIR.OMP.TASK.3

DIR.OMP.TASK.3:                                   ; preds = %entry
  %end.dir.temp = alloca i1, align 1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32* %a, i32* %a, i64 4, i64 35), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.TARGET.6.split, label %DIR.OMP.TARGET.5

DIR.OMP.TARGET.5:                                 ; preds = %DIR.OMP.TASK.3
  store i32 5, i32* %a, align 4
  br label %DIR.OMP.END.TARGET.6.split

DIR.OMP.END.TARGET.6.split:                       ; preds = %DIR.OMP.TASK.3, %DIR.OMP.TARGET.5
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.7.split

DIR.OMP.END.TARGET.7.split:                       ; preds = %entry, %DIR.OMP.END.TARGET.6.split
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret i32 0
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: uwtable
define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare dso_local void @__tgt_register_requires(i64) local_unnamed_addr

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 0, i32 64773, i32 1078689017, !"_Z4main", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
