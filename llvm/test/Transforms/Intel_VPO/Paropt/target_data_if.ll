; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S | FileCheck %s

; This test checks that after paropt pass, the runtime calls for "target data
; begin" and "target data end", outlined function and use of offload_ptrs,
; offload_sizes and offload_baseptrs are in consecutive lines
;
; #define SIZE 1024
; #define SIZE_THRESHOLD 512
;
; void test_target_data_if(int map_size){
;   int a[SIZE];
;   int i = 0;
;
;   for (i = 0; i < SIZE; i++) {
;     a[i] = SIZE - i;
;   }
;
; #pragma omp target data if(map_size > SIZE_THRESHOLD) map(to: map_size)  \
;   map(to: a[0:map_size])
;   {
; #pragma omp target if (map_size > SIZE_THRESHOLD)
;     {
;       int j = 0;
;       for (j = 0; j < map_size; j++) {
;         a[j] = -1;
;       }
;     } // end target
;
;   }//end-target data
;
; }
;


source_filename = "target_data_if.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test_target_data_if(i32 %map_size) #0 {
entry:
  %map_size.addr = alloca i32, align 4
  %a = alloca [1024 x i32], align 16
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %map_size, i32* %map_size.addr, align 4
  store i32 0, i32* %i, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 1024
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  %sub = sub nsw i32 1024, %1
  %2 = load i32, i32* %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %a, i64 0, i64 %idxprom
  store i32 %sub, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, i32* %i, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %4 = load i32, i32* %map_size.addr, align 4
  %cmp1 = icmp sgt i32 %4, 512
  %conv = zext i1 %cmp1 to i32
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* %a, i64 0, i64 0
  %5 = load i32, i32* %map_size.addr, align 4
  %6 = zext i32 %5 to i64
  %7 = mul nuw i64 %6, 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.IF"(i32 %conv), "QUAL.OMP.MAP.TO"(i32* %map_size.addr), "QUAL.OMP.MAP.TO:AGGRHEAD"([1024 x i32]* %a, i32* %arrayidx2, i64 %7) ]

; CHECK: call void @__tgt_target_data_begin(i64 -1, i32 2, i8** %{{.*}}, i8** %{{.*}}, i64* %{{.*}}, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes.1, i32 0, i32 0))
; CHECK-NEXT: call void @test_target_data_if.DIR.OMP.TARGET.DATA.{{.*}}(i32* %map_size.addr, [1024 x i32]* %a)
; CHECK-NEXT: %{{.*}} = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK-NEXT: %{{.*}} = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_ptrs, i32 0, i32 0
; CHECK-NEXT: %{{.*}} = getelementptr inbounds [2 x i64], [2 x i64]* %.offload_sizes, i32 0, i32 0
; CHECK-NEXT: call void @__tgt_target_data_end(i64 -1, i32 2, i8** %{{.*}}, i8** %{{.*}}, i64* %{{.*}}, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes.1, i32 0, i32 0))
; CHECK-NEXT:  br label %if.end
; CHECK-EMPTY:
; CHECK-NEXT: if.else:
; CHECK-NEXT: store i32 -1, i32* %.run_host_version
; CHECK-NEXT: call void @test_target_data_if.DIR.OMP.TARGET.DATA.{{.*}}(i32* %map_size.addr, [1024 x i32]* %a)

  %9 = load i32, i32* %map_size.addr, align 4
  %cmp3 = icmp sgt i32 %9, 512
  %conv4 = zext i1 %cmp3 to i32
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.IF"(i32 %conv4), "QUAL.OMP.MAP.TOFROM"([1024 x i32]* %a), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.FIRSTPRIVATE"(i32* %map_size.addr) ]
  store i32 0, i32* %j, align 4
  store i32 0, i32* %j, align 4
  br label %for.cond5

for.cond5:                                        ; preds = %for.inc11, %for.end
  %11 = load i32, i32* %j, align 4
  %12 = load i32, i32* %map_size.addr, align 4
  %cmp6 = icmp slt i32 %11, %12
  br i1 %cmp6, label %for.body8, label %for.end13

for.body8:                                        ; preds = %for.cond5
  %13 = load i32, i32* %j, align 4
  %idxprom9 = sext i32 %13 to i64
  %arrayidx10 = getelementptr inbounds [1024 x i32], [1024 x i32]* %a, i64 0, i64 %idxprom9
  store i32 -1, i32* %arrayidx10, align 4
  br label %for.inc11

for.inc11:                                        ; preds = %for.body8
  %14 = load i32, i32* %j, align 4
  %inc12 = add nsw i32 %14, 1
  store i32 %inc12, i32* %j, align 4
  br label %for.cond5

for.end13:                                        ; preds = %for.cond5
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_offloading.requires_reg() #2 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare dso_local void @__tgt_register_requires(i64)

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2053, i32 8914597, !"test_target_data_if", i32 16, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
