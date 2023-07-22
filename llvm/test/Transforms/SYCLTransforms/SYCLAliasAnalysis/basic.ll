; RUN: opt -passes=aa-eval -aa-pipeline=sycl-kernel-aa -print-all-alias-modref-info -disable-output %s 2>&1 | FileCheck %s

@global = global i8 undef
@global_g = addrspace(1) global i8 undef, align 8
@global_c = addrspace(2) global i8 undef, align 8
@global_l = addrspace(3) global i8 undef, align 8

; CHECK-LABEL: Function: test
; CHECK-DAG: NoAlias:      i8* %gep, i8* %p1
; CHECK-DAG: NoAlias:      i8 addrspace(3)* %local1, i8* %p1
; CHECK-DAG: MayAlias:     i8* %gep, i8 addrspace(3)* %local1
; CHECK-DAG: MayAlias:     i8* %p, i8* %p1
; CHECK-DAG: NoAlias:      i8* %gep, i8* %p
; CHECK-DAG: NoAlias:      i8 addrspace(3)* %local1, i8* %p
; CHECK-DAG: NoAlias:      i8 addrspace(1)* %g, i8* %p1
; CHECK-DAG: NoAlias:      i8 addrspace(1)* %g, i8* %gep
; CHECK-DAG: NoAlias:      i8 addrspace(1)* %g, i8 addrspace(3)* %local1
; CHECK-DAG: NoAlias:      i8 addrspace(1)* %g, i8* %p
; CHECK-DAG: NoAlias:      i8 addrspace(2)* %c, i8* %p1
; CHECK-DAG: NoAlias:      i8 addrspace(2)* %c, i8* %gep
; CHECK-DAG: NoAlias:      i8 addrspace(2)* %c, i8 addrspace(3)* %local1
; CHECK-DAG: NoAlias:      i8 addrspace(2)* %c, i8* %p
; CHECK-DAG: NoAlias:      i8 addrspace(2)* %c, i8 addrspace(1)* %g
; CHECK-DAG: NoAlias:      i8 addrspace(3)* %l, i8* %p1
; CHECK-DAG: MayAlias:     i8* %gep, i8 addrspace(3)* %l
; CHECK-DAG: MayAlias:     i8 addrspace(3)* %l, i8 addrspace(3)* %local1
; CHECK-DAG: NoAlias:      i8 addrspace(3)* %l, i8* %p
; CHECK-DAG: NoAlias:      i8 addrspace(1)* %g, i8 addrspace(3)* %l
; CHECK-DAG: NoAlias:      i8 addrspace(2)* %c, i8 addrspace(3)* %l
; CHECK-DAG: MayAlias:     i8* %p1, i8* @global
; CHECK-DAG: NoAlias:      i8* %gep, i8* @global
; CHECK-DAG: NoAlias:      i8 addrspace(3)* %local1, i8* @global
; CHECK-DAG: MayAlias:     i8* %p, i8* @global
; CHECK-DAG: NoAlias:      i8 addrspace(1)* %g, i8* @global
; CHECK-DAG: NoAlias:      i8 addrspace(2)* %c, i8* @global
; CHECK-DAG: NoAlias:      i8 addrspace(3)* %l, i8* @global
; CHECK-DAG: NoAlias:      i8* %p1, i8 addrspace(1)* @global_g
; CHECK-DAG: NoAlias:      i8* %gep, i8 addrspace(1)* @global_g
; CHECK-DAG: NoAlias:      i8 addrspace(3)* %local1, i8 addrspace(1)* @global_g
; CHECK-DAG: NoAlias:      i8* %p, i8 addrspace(1)* @global_g
; CHECK-DAG: MayAlias:     i8 addrspace(1)* %g, i8 addrspace(1)* @global_g
; CHECK-DAG: NoAlias:      i8 addrspace(2)* %c, i8 addrspace(1)* @global_g
; CHECK-DAG: NoAlias:      i8 addrspace(3)* %l, i8 addrspace(1)* @global_g
; CHECK-DAG: NoAlias:      i8* @global, i8 addrspace(1)* @global_g
; CHECK-DAG: NoAlias:      i8* %p1, i8 addrspace(2)* @global_c
; CHECK-DAG: NoAlias:      i8* %gep, i8 addrspace(2)* @global_c
; CHECK-DAG: NoAlias:      i8 addrspace(3)* %local1, i8 addrspace(2)* @global_c
; CHECK-DAG: NoAlias:      i8* %p, i8 addrspace(2)* @global_c
; CHECK-DAG: NoAlias:      i8 addrspace(1)* %g, i8 addrspace(2)* @global_c
; CHECK-DAG: MayAlias:     i8 addrspace(2)* %c, i8 addrspace(2)* @global_c
; CHECK-DAG: NoAlias:      i8 addrspace(3)* %l, i8 addrspace(2)* @global_c
; CHECK-DAG: NoAlias:      i8* @global, i8 addrspace(2)* @global_c
; CHECK-DAG: NoAlias:      i8 addrspace(2)* @global_c, i8 addrspace(1)* @global_g
; CHECK-DAG: NoAlias:      i8* %p1, i8 addrspace(3)* @global_l
; CHECK-DAG: MayAlias:     i8* %gep, i8 addrspace(3)* @global_l
; CHECK-DAG: MayAlias:     i8 addrspace(3)* %local1, i8 addrspace(3)* @global_l
; CHECK-DAG: NoAlias:      i8* %p, i8 addrspace(3)* @global_l
; CHECK-DAG: NoAlias:      i8 addrspace(1)* %g, i8 addrspace(3)* @global_l
; CHECK-DAG: NoAlias:      i8 addrspace(2)* %c, i8 addrspace(3)* @global_l
; CHECK-DAG: MayAlias:     i8 addrspace(3)* %l, i8 addrspace(3)* @global_l
; CHECK-DAG: NoAlias:      i8* @global, i8 addrspace(3)* @global_l
; CHECK-DAG: NoAlias:      i8 addrspace(1)* @global_g, i8 addrspace(3)* @global_l
; CHECK-DAG: NoAlias:      i8 addrspace(2)* @global_c, i8 addrspace(3)* @global_l

; TODO there should be no alias between following pairs:
;   (%gep, %local1), (%p, %p1), (%gep, %l), (%l, %local1),
;   (%p1, @global), (%gep, @global_l), (%local1, @global_l)

define void @test(ptr %p, ptr addrspace(1) %g, ptr addrspace(2) %c, ptr addrspace(3) %l) {
  %p1 = alloca i8, align 1
  %p2 = alloca <16 x i8>, align 16
  %gep = getelementptr inbounds <16 x i8>, ptr %p2, i64 0, i64 0
  %gep1 = getelementptr inbounds <16 x i8>, ptr %p2, i64 0, i64 8
  %local1 = addrspacecast ptr %gep1 to ptr addrspace(3)
  %1 = load i8, ptr %p1, align 1
  %2 = load i8, ptr %gep, align 1
  %3 = load i8, ptr addrspace(3) %local1, align 1
  %4 = load i8, ptr %p, align 1
  %5 = load i8, ptr addrspace(1) %g, align 1
  %6 = load i8, ptr addrspace(2) %c, align 1
  %7 = load i8, ptr addrspace(3) %l, align 1
  %8 = load i8, ptr @global, align 1
  %9 = load i8, ptr addrspace(1) @global_g, align 1
  %10 = load i8, ptr addrspace(2) @global_c, align 1
  %11 = load i8, ptr addrspace(3) @global_l, align 1
  ret void
}

!0 = !{!"char*", !"char*", !"char*", !"char*"}
!1 = !{ptr addrspace(0) null, ptr addrspace(1) null, ptr addrspace(2) null, ptr addrspace(3) null}
