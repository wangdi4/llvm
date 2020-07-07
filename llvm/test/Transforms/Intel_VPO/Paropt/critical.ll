; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -mtriple=unknown-unknown-unknown\
; RUN: -S < %s | FileCheck %s -check-prefix=DEFAULT -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -mtriple=i686-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN32 -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -mtriple=x86_64-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN64 -check-prefix=ALL
; RUN: opt -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -mtriple=unknown-unknown-unknown\
; RUN: -S < %s | FileCheck %s -check-prefix=DEFAULT -check-prefix=ALL
; RUN: opt -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -mtriple=i686-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN32 -check-prefix=ALL
; RUN: opt -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -mtriple=x86_64-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN64 -check-prefix=ALL

source_filename = "critical_noname.c"
target triple = "unknown-unknown-unknown"

; DEFAULT: @[[LOCK:.gomp_critical_user_.AS0.var]] = common global [8 x i32] zeroinitializer
; WIN32: @[[LOCK:"_\$vcomp\$critsect\$.AS0.var"]] = common global [8 x i32] zeroinitializer
; WIN64: @[[LOCK:"\$vcomp\$critsect\$.AS0.var"]] = common global [8 x i32] zeroinitializer

;-----------------------------------------------------------------------------
; Test critical section generation without any user provided name.
; -----------------------------------------------------------------------------
define void @add_1(i32* nocapture %num) {
entry:
; ALL-NOT: %ret = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"() ]
; ALL: call void @__kmpc_critical({{[^,]+}}, i32 %{{[^\s]*}}, [8 x i32]* @[[LOCK]])
  %ret = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"() ]

; ALL: %0 = load i32, i32* %num, align 4
; ALL: %add = add nsw i32 %0, 1
; ALL:  store i32 %add, i32* %num, align 4
  %0 = load i32, i32* %num, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, i32* %num, align 4

; ALL: call void @__kmpc_end_critical({{[^,]+}}, i32 %{{[^\s]+}}, [8 x i32]* @[[LOCK]])
  call void @llvm.directive.region.exit(token %ret) [ "DIR.OMP.END.CRITICAL"() ]
; ALL-NOT: void @llvm.directive.region.exit(token %ret) [ "DIR.OMP.END.CRITICAL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
