; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -mtriple=unknown-unknown-unknown\
; RUN: -S < %s | FileCheck %s -check-prefix=DEFAULT -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -mtriple=i686-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN32 -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -mtriple=x86_64-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN64 -check-prefix=ALL

target triple = "unknown-unknown-unknown"

; DEFAULT: @[[LOCK:.gomp_critical_user_my_name.var]] = common global [8 x i32] zeroinitializer
; WIN32: @[[LOCK:"_\$vcomp\$critsect\$my_name.var"]] = common global [8 x i32] zeroinitializer
; WIN64: @[[LOCK:"\$vcomp\$critsect\$my_name.var"]] = common global [8 x i32] zeroinitializer

;-----------------------------------------------------------------------------
; Test critical section generation with name as a regular string.
; -----------------------------------------------------------------------------
define void @add_1(i32* nocapture %num) {
; ALL-LABEL: @add_1(
entry:
; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.OMP.CRITICAL")
; ALL-NOT: call void @llvm.intel.directive.qual.opnd.a7i8(metadata !"QUAL.OMP.NAME", [7 x i8] c"my_name")
; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  call void @llvm.intel.directive(metadata !"DIR.OMP.CRITICAL")
  call void @llvm.intel.directive.qual.opnd.a7i8(metadata !"QUAL.OMP.NAME", [7 x i8] c"my_name")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")

; ALL: call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* @{{[^\s]+}}, i32 %{{[^\s]+}}, [8 x i32]* @[[LOCK]])

; ALL: %0 = load i32, i32* %num, align 4
; ALL: %add = add nsw i32 %0, 1
; ALL:  store i32 %add, i32* %num, align 4
  %0 = load i32, i32* %num, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, i32* %num, align 4

; ALL: call void @__kmpc_end_critical({ i32, i32, i32, i32, i8* }* @{{[^\s]+}}, i32 %{{[^\s]+}}, [8 x i32]* @[[LOCK]])

; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.CRITICAL")
; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.CRITICAL")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; Test critical section generation with name as a C string (ends with nul)
;-----------------------------------------------------------------------------
define void @add_2(i32* nocapture %num) {
; ALL-LABEL: @add_2(
entry:
; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.OMP.CRITICAL")
; ALL-NOT: call void @llvm.intel.directive.qual.opnd.a8i8(metadata !"QUAL.OMP.NAME", [8 x i8] c"my_name\00")
; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  call void @llvm.intel.directive(metadata !"DIR.OMP.CRITICAL")
  call void @llvm.intel.directive.qual.opnd.a8i8(metadata !"QUAL.OMP.NAME", [8 x i8] c"my_name\00")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")

; ALL: call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* @{{[^\s]+}}, i32 %{{[^\s]+}}, [8 x i32]* @[[LOCK]])

; ALL: %0 = load i32, i32* %num, align 4
; ALL: %add = add nsw i32 %0, 2
; ALL:  store i32 %add, i32* %num, align 4
  %0 = load i32, i32* %num, align 4
  %add = add nsw i32 %0, 2
  store i32 %add, i32* %num, align 4

; ALL: call void @__kmpc_end_critical({ i32, i32, i32, i32, i8* }* @{{[^\s]+}}, i32 %{{[^\s]+}}, [8 x i32]* @[[LOCK]])

; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.CRITICAL")
; ALL-NOT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.CRITICAL")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}
;-----------------------------------------------------------------------------


declare void @llvm.intel.directive(metadata) #2
declare void @llvm.intel.directive.qual.opnd.a7i8(metadata, [7 x i8]) #2
declare void @llvm.intel.directive.qual.opnd.a8i8(metadata, [8 x i8]) #2

attributes #2 = { argmemonly nounwind }

