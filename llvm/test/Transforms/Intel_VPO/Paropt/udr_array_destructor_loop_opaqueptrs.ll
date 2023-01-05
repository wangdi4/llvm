; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S %s | FileCheck %s

; Test src:
;
;#include <stdio.h>
;
; typedef struct my_struct{
;     int a; int b;
; } TYPE;
; 
; void my_add(TYPE& lhs, TYPE const &rhs) { lhs.a += rhs.a; lhs.b += rhs.b;}
; #pragma omp declare reduction (my_reduction_add : TYPE : my_add(omp_out, omp_in))
; 
; TYPE yarr[2];
; int main() {
;     yarr[1].a = 11; yarr[1].b = 11;
; 
; #pragma omp parallel reduction(my_reduction_add: yarr) num_threads(1)
;     { yarr[1].a += 11; yarr[1].b += 11; }
; 
;     return 0;
; }

; Check that the destructor is called on each element of the array
;CHECK: red.destr.body:                                   ; preds = %red.destr.body, %DIR.OMP.END.PARALLEL{{.*}}
;CHECK-NEXT:   %red.cpy.dest.ptr{{.*}} = phi ptr [ %array{{.*}}, %DIR.OMP.END.PARALLEL{{.*}}], [ %red.cpy.dest.inc{{.*}}, %red.destr.body ]
;CHECK-NEXT:   call void @_ZTS9my_struct.omp.destr(ptr %red.cpy.dest.ptr{{.*}}) 
;CHECK-NEXT:   %red.cpy.dest.inc{{.*}} = getelementptr %struct.my_struct, ptr %red.cpy.dest.ptr{{.*}}, i32 1
;CHECK-NEXT:   %red.cpy.done{{.*}} = icmp eq ptr %red.cpy.dest.inc{{.*}}, %{{.*}}
;CHECK-NEXT:   br i1 %red.cpy.done{{.*}}, label %red.destr.done, label %red.destr.body

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i32, i32 }

@yarr = dso_local global [2 x %struct.my_struct] zeroinitializer, align 16

declare void @_Z6my_addR9my_structRKS_(ptr noundef nonnull align 4 dereferenceable(8) %lhs, ptr noundef nonnull align 4 dereferenceable(8) %rhs)

define dso_local noundef i32 @main() #1 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 11, ptr getelementptr inbounds ([2 x %struct.my_struct], ptr @yarr, i64 0, i64 1), align 8
  store i32 11, ptr getelementptr inbounds ([2 x %struct.my_struct], ptr @yarr, i64 0, i64 1, i32 1), align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr @yarr, %struct.my_struct zeroinitializer, i64 2, ptr @_ZTS9my_struct.omp.def_constr, ptr @_ZTS9my_struct.omp.destr, ptr @.omp_combiner., ptr null),
     "QUAL.OMP.NUM_THREADS"(i32 1) ]

  %1 = load i32, ptr getelementptr inbounds ([2 x %struct.my_struct], ptr @yarr, i64 0, i64 1), align 8
  %add = add nsw i32 %1, 11
  store i32 %add, ptr getelementptr inbounds ([2 x %struct.my_struct], ptr @yarr, i64 0, i64 1), align 8
  %2 = load i32, ptr getelementptr inbounds ([2 x %struct.my_struct], ptr @yarr, i64 0, i64 1, i32 1), align 4
  %add1 = add nsw i32 %2, 11
  store i32 %add1, ptr getelementptr inbounds ([2 x %struct.my_struct], ptr @yarr, i64 0, i64 1, i32 1), align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry() #2

declare void @llvm.directive.region.exit(token) #2

define internal void @.omp_combiner.(ptr noalias noundef %0, ptr noalias noundef %1) #3 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  call void @_Z6my_addR9my_structRKS_(ptr noundef nonnull align 4 dereferenceable(8) %3, ptr noundef nonnull align 4 dereferenceable(8) %2)
  ret void
}

define internal noundef ptr @_ZTS9my_struct.omp.def_constr(ptr noundef %0) #3 section ".text.startup" {
entry:
  %retval = alloca ptr, align 8
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  %1 = load ptr, ptr %retval, align 8
  ret ptr %1
}

define internal void @_ZTS9my_struct.omp.destr(ptr noundef %0) #3 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  ret void
}
