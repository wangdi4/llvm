; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt -opaque-pointers=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck --check-prefix=OPQPTR %s
; RUN: opt -opaque-pointers=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck --check-prefix=OPQPTR %s

; Original code:
;int main() {
;  int x = 100;
;#pragma omp teams distribute parallel for num_teams(x) thread_limit(x) num_threads(x)
;  for (int i = 0; i < 100; ++i);
;  return 0;
;}
; NUM_TEAMS and THREAD_LIMIT clauses were hand modified to take
; pointer argument with the type specifier.

; CHECK: [[NUMTEAMS:%[0-9A-Za-z._]+]] = load i32, i32* %x
; CHECK: [[THREADLIMIT:%[0-9A-Za-z._]+]] = load i32, i32* %x
; CHECK: call void @__kmpc_push_num_teams(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 [[NUMTEAMS]], i32 [[THREADLIMIT]])

; OPQPTR: [[NUMTEAMS:%[0-9A-Za-z._]+]] = load i32, ptr %x
; OPQPTR: [[THREADLIMIT:%[0-9A-Za-z._]+]] = load i32, ptr %x
; OPQPTR: call void @__kmpc_push_num_teams(ptr @{{.*}}, i32 %{{.*}}, i32 [[NUMTEAMS]], i32 [[THREADLIMIT]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 100, i32* %x, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(i32* %x, i32 0),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(i32* %x, i32 0) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
