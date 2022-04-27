; INTEL_CUSTOMIZATION
; RUN: opt -vpo-paropt -S %s | FileCheck %s -check-prefix=DEFAULT
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s -check-prefix=DEFAULT

; RUN: opt -vpo-paropt -S -vpo-paropt-emit-kmpc-begin-end-only-for-windows=false -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -vpo-paropt-emit-kmpc-begin-end-only-for-windows=false -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END
; RUN: opt -vpo-paropt -S -mtriple=i686-unknown-windows -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -mtriple=i686-unknown-windows -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END
; RUN: opt -vpo-paropt -S -mtriple=x86_64-unknown-windows -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -mtriple=x86_64-unknown-windows -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END

; RUN: opt -vpo-paropt -S -vpo-paropt-emit-kmpc-begin-end-only-for-windows=false %s | FileCheck %s -check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -vpo-paropt-emit-kmpc-begin-end-only-for-windows=false %s | FileCheck %s -check-prefix=WITH_END
; RUN: opt -vpo-paropt -S -mtriple=i686-unknown-windows %s | FileCheck %s -check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -mtriple=i686-unknown-windows %s | FileCheck %s -check-prefix=WITH_END
; RUN: opt -vpo-paropt -S -mtriple=x86_64-unknown-windows %s | FileCheck %s -check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -mtriple=x86_64-unknown-windows %s | FileCheck %s -check-prefix=WITH_END

; program test
;
;   call foo()
;
;   contains
;     subroutine foo
; !      !$omp parallel
;        call bar()
; !      !$omp end parallel
;     end subroutine
;     subroutine bar
;     end subroutine
; end program

; By default kmpc_begin/end emission is disabled. Only kmpc_end is enabled for
; Windows on the request of libomp team as it is needed for proper invocation
; of OMPT finalization routines.

; DEFAULT-NOT: call void @__kmpc_begin
; DEFAULT-NOT: call void @__kmpc_end

; WITH_BEGIN:    call void @__kmpc_begin(%struct.ident_t* @{{.*}}, i32 0)
; WITH_END:      call void @test_IP_foo_()
; WITH_END-NEXT: call void @__kmpc_end(%struct.ident_t* @{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "unknown-unknown-unknown"

@0 = internal unnamed_addr constant i32 2

; Function Attrs: noinline nounwind optnone uwtable
define void @MAIN__() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %func_result = call i32 @for_set_reentrancy(i32* @0)
  call void @test_IP_foo_()
  ret void
}

declare void @test_IP_bar_() #0
declare void @test_IP_foo_() #0

declare i32 @for_set_reentrancy(i32* nocapture readonly %0)

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
; end INTEL_CUSTOMIZATION
