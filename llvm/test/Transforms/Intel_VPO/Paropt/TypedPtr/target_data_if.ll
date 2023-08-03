; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes="function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt" -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

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

; CHECK: call void @__tgt_target_data_begin(i64 %{{.*}}, i32 2, i8** %{{.*}}, i8** %{{.*}}, i64* %{{.*}}, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes{{.*}}, i32 0, i32 0))
; CHECK-NEXT: call void @test_target_data_if.DIR.OMP.TARGET.DATA.{{.*}}(i32* %map_size.addr, [1024 x i32]* %a)
; CHECK-NEXT: %{{.*}} = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK-NEXT: %{{.*}} = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_ptrs, i32 0, i32 0
; CHECK-NEXT: %{{.*}} = getelementptr inbounds [2 x i64], [2 x i64]* %.offload_sizes, i32 0, i32 0
; CHECK: call void @__tgt_push_code_location({{.*}})
; CHECK-NEXT: call void @__tgt_target_data_end(i64 %{{.*}}, i32 2, i8** %{{.*}}, i8** %{{.*}}, i64* %{{.*}}, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes{{.*}}, i32 0, i32 0))
; CHECK-NEXT:  br label %if.end
; CHECK-EMPTY:
; CHECK-NEXT: if.else:
; CHECK-NEXT: store i32 -1, i32* %.run_host_version
; CHECK-NEXT: call void @test_target_data_if.DIR.OMP.TARGET.DATA.{{.*}}(i32* %map_size.addr, [1024 x i32]* %a)

; Check that the host fallback code is not forced to run with 1 thread
; by calling __kmpc_push_num_teams(LOC, 0, 0, 1) to set thread limit to 1
; CHECK-LABEL: omp_offload.failed:
; CHECK-NOT: call void @__kmpc_push_num_teams({{.*}}, i32 0, i32 0, i32 1)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @test_target_data_if(i32 %map_size) {
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
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.IF"(i32 %conv),
    "QUAL.OMP.MAP.TO"(i32* %map_size.addr, i32* %map_size.addr, i64 4, i64 1, i8* null, i8* null),
    "QUAL.OMP.MAP.TO"([1024 x i32]* %a, i32* %arrayidx2, i64 %7, i64 1, i8* null, i8* null) ]

  %9 = load i32, i32* %map_size.addr, align 4
  %cmp3 = icmp sgt i32 %9, 512
  %conv4 = zext i1 %cmp3 to i32
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.IF"(i32 %conv4),
    "QUAL.OMP.MAP.TOFROM"([1024 x i32]* %a, [1024 x i32]* %a, i64 4096, i64 547, i8* null, i8* null),
    "QUAL.OMP.PRIVATE"(i32* %j),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %map_size.addr) ]

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

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2053, i32 8914597, !"test_target_data_if", i32 16, i32 0, i32 0}
