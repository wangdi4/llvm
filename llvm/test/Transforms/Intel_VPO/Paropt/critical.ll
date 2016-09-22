; RUN: opt -vpo-cfg-restructuring -vpo-paropt -mtriple=unknown-unknown-unknown\
; RUN: -S < %s | FileCheck %s -check-prefix=DEFAULT -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt -mtriple=i686-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN32 -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt -mtriple=x86_64-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN64 -check-prefix=ALL

source_filename = "critical_noname.c"
target triple = "unknown-unknown-unknown"

; DEFAULT: @[[LOCK:.gomp_critical_user_.var]] = common global [8 x i32] zeroinitializer
; WIN32: @[[LOCK:"_\$vcomp\$critsect\$.var"]] = common global [8 x i32] zeroinitializer
; WIN64: @[[LOCK:"\$vcomp\$critsect\$.var"]] = common global [8 x i32] zeroinitializer

;-----------------------------------------------------------------------------
; Test critical section generation without any user provided name.
; -----------------------------------------------------------------------------
define void @add_1(i32* nocapture %num) {
entry:
; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.OMP.CRITICAL")
; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
; ALL: call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* @{{[^\s]*}}, i32 %{{[^\s]*}}, [8 x i32]* @[[LOCK]])
  call void @llvm.intel.directive(metadata !"DIR.OMP.CRITICAL")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")

; ALL: %0 = load i32, i32* %num, align 4
; ALL: %add = add nsw i32 %0, 1
; ALL:  store i32 %add, i32* %num, align 4
  %0 = load i32, i32* %num, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, i32* %num, align 4

; ALL: call void @__kmpc_end_critical({ i32, i32, i32, i32, i8* }* @{{[^\s]+}}, i32 %{{[^\s]+}}, [8 x i32]* @[[LOCK]])
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.CRITICAL")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.CRITICAL")
; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

declare void @llvm.intel.directive(metadata) #2
attributes #2 = { argmemonly nounwind }

