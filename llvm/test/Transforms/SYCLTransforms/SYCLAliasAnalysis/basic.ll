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

define void @test(i8 addrspace(0)* %p, i8 addrspace(1)* %g, i8 addrspace(2)* %c, i8 addrspace(3)* %l) {
  %p1 = alloca i8, align 1, addrspace(0)
  %p2 = alloca <16 x i8>, align 16
  %gep = getelementptr inbounds <16 x i8>, <16 x i8>* %p2, i64 0, i64 0
  %gep1 = getelementptr inbounds <16 x i8>, <16 x i8>* %p2, i64 0, i64 8
  %local1 = addrspacecast i8* %gep1 to i8 addrspace(3)*
  load i8, i8 addrspace(0)* %p1
  load i8, i8* %gep
  load i8, i8 addrspace(3)* %local1
  load i8, i8 addrspace(0)* %p
  load i8, i8 addrspace(1)* %g
  load i8, i8 addrspace(2)* %c
  load i8, i8 addrspace(3)* %l
  load i8, i8* @global
  load i8, i8 addrspace(1)* @global_g
  load i8, i8 addrspace(2)* @global_c
  load i8, i8 addrspace(3)* @global_l
  ret void
}
