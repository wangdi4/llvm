; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Check that the map-type for %1 is not changed from 0 to something else
; (like 0x23) by Paropt.
; CHECK: [[MAPTYPES:@.offload_maptypes]] = {{.*}} [1 x i64] zeroinitializer
; CHECK: call void @__tgt_target_data_begin_mapper({{.*}}i64* getelementptr inbounds ([1 x i64], [1 x i64]* [[MAPTYPES]], i32 0, i32 0){{.*}})
; CHECK: call void @__tgt_target_data_end_mapper({{.*}}i64* getelementptr inbounds ([1 x i64], [1 x i64]* [[MAPTYPES]], i32 0, i32 0){{.*}})

; Test src:
;
; #include <stdio.h>
;
; void foo(int *x, int n) {
;
;   printf("%p\n", &x[0]);
; #pragma omp target data map(alloc : x [0:n])
;   { printf("%p\n", &x[0]); }
; }
;
; int main() {
;   int y[2];
;   foo(&y[0], 2);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32* %x, i32 %n) #0 {
entry:
  %x.addr = alloca i32*, align 8
  %n.addr = alloca i32, align 4
  store i32* %x, i32** %x.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32*, i32** %x.addr, align 8
  %ptridx = getelementptr inbounds i32, i32* %0, i64 0
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* %ptridx)
  %1 = load i32*, i32** %x.addr, align 8
  %2 = load i32*, i32** %x.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %2, i64 0
  %3 = load i32, i32* %n.addr, align 4
  %conv = sext i32 %3 to i64
  %4 = mul nuw i64 %conv, 4

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TOFROM"(i32* %1, i32* %arrayidx, i64 %4, i64 0, i8* null, i8* null) ]

  %6 = load i32*, i32** %x.addr, align 8
  %ptridx1 = getelementptr inbounds i32, i32* %6, i64 0
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* %ptridx1) #2

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET.DATA"() ]

  ret void
}

declare dso_local i32 @printf(i8*, ...) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
