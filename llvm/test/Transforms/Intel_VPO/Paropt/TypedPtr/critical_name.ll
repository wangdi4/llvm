; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -mtriple=unknown-unknown-unknown\
; RUN: -S < %s | FileCheck %s -check-prefix=DEFAULT -check-prefix=ALL
; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -mtriple=i686-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN32 -check-prefix=ALL
; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -mtriple=x86_64-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN64 -check-prefix=ALL
; RUN: opt -opaque-pointers=0 -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -mtriple=unknown-unknown-unknown\
; RUN: -S < %s | FileCheck %s -check-prefix=DEFAULT -check-prefix=ALL
; RUN: opt -opaque-pointers=0 -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -mtriple=i686-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN32 -check-prefix=ALL
; RUN: opt -opaque-pointers=0 -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -mtriple=x86_64-unknown-windows\
; RUN: -S < %s | FileCheck %s -check-prefix=WIN64 -check-prefix=ALL

target triple = "unknown-unknown-unknown"

; DEFAULT: @[[LOCK:.gomp_critical_user_my_name.AS0.var]] = common global [8 x i32] zeroinitializer
; WIN32: @[[LOCK:"_\$vcomp\$critsect\$my_name.AS0.var"]] = common global [8 x i32] zeroinitializer
; WIN64: @[[LOCK:"\$vcomp\$critsect\$my_name.AS0.var"]] = common global [8 x i32] zeroinitializer

;-----------------------------------------------------------------------------
; Test critical section generation with name as a regular string.
; -----------------------------------------------------------------------------
define void @add_1(i32* nocapture %num) {
; ALL-LABEL: @add_1(
entry:
; ALL-NOT: %ret = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"(), "QUAL.OMP.NAME"([7 x i8] c"my_name") ]
  %ret = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"(), "QUAL.OMP.NAME"([7 x i8] c"my_name") ]
; ALL: call void @__kmpc_critical({{[^,]+}}, i32 %{{[^\s]+}}, [8 x i32]* @[[LOCK]])

; ALL: %0 = load i32, i32* %num, align 4
; ALL: %add = add nsw i32 %0, 1
; ALL:  store i32 %add, i32* %num, align 4
  %0 = load i32, i32* %num, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, i32* %num, align 4

; ALL: call void @__kmpc_end_critical({{[^,]+}}, i32 %{{[^\s]+}}, [8 x i32]* @[[LOCK]])
; ALL-NOT: void @llvm.directive.region.exit(token %ret) [ "DIR.OMP.END.CRITICAL"() ]
  call void @llvm.directive.region.exit(token %ret) [ "DIR.OMP.END.CRITICAL"() ]
  ret void
}
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; Test critical section generation with name as a C string (ends with nul)
;-----------------------------------------------------------------------------
define void @add_2(i32* nocapture %num) {
; ALL-LABEL: @add_2(
entry:
; ALL-NOT: %ret = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"(), "QUAL.OMP.NAME"([8 x i8] c"my_name\00") ]
  %ret = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"(), "QUAL.OMP.NAME"([8 x i8] c"my_name\00") ]
; ALL: call void @__kmpc_critical({{[^,]+}}, i32 %{{[^\s]+}}, [8 x i32]* @[[LOCK]])

; ALL: %0 = load i32, i32* %num, align 4
; ALL: %add = add nsw i32 %0, 2
; ALL:  store i32 %add, i32* %num, align 4
  %0 = load i32, i32* %num, align 4
  %add = add nsw i32 %0, 2
  store i32 %add, i32* %num, align 4

; ALL: call void @__kmpc_end_critical({{[^,]+}}, i32 %{{[^\s]+}}, [8 x i32]* @[[LOCK]])
; ALL-NOT: void @llvm.directive.region.exit(token %ret) [ "DIR.OMP.END.CRITICAL"() ]
  call void @llvm.directive.region.exit(token %ret) [ "DIR.OMP.END.CRITICAL"() ]
  ret void
}
;-----------------------------------------------------------------------------


declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

