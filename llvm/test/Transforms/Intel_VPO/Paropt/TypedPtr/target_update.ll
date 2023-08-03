; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; This file tests the support of omp target update construct.
; void device_side_scan(int arg) {
;   #pragma omp target update from(arg) if(arg) device(4)
;   {++arg;}
; }
;

; CHECK:  call void @__tgt_target_data_update({{.*}})

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-mic"

@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr constant { i32, i32, i32, i32, i8* } { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0, i32 0, i32 0) }

define void @_Z16device_side_scani(i32 %arg) {
entry:
  %tid.val = tail call i32 @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0)
  %arg.addr = alloca i32, align 4
  store i32 %arg, i32* %arg.addr, align 4
  %tobool = icmp ne i32 %arg, 0
  br label %DIR.OMP.TARGET.UPDATE.1

DIR.OMP.TARGET.UPDATE.1:                          ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"(),
    "QUAL.OMP.IF"(i1 %tobool),
    "QUAL.OMP.DEVICE"(i32 4),
    "QUAL.OMP.MAP.FROM"(i32* %arg.addr, i32* %arg.addr, i64 4, i64 2, i8* null, i8* null) ]

  br label %DIR.OMP.END.TARGET.UPDATE.3

DIR.OMP.END.TARGET.UPDATE.3:                      ; preds = %DIR.OMP.TARGET.UPDATE.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.UPDATE"() ]
  %1 = load i32, i32* %arg.addr, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* %arg.addr, align 4
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare i32 @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }*)
