; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S <%s | FileCheck %s
;
; Test support for the QUAL.OMP.DEPARRAY representation for DEPEND clauses.
;
; // C test with depobj construct and depend clauses with depobj, in, out depend-types
; #include <stdio.h>
; #include <omp.h>
;
; double ccc;
; int main() {
;   int aaa;
;   float bbb;
;
;   // #pragma omp parallel num_threads(8)
;   {
;     // #pragma omp single
;     {
;       omp_depend_t obj1;
;       #pragma omp depobj(obj1) depend(out:aaa)
;
;       #pragma omp task depend(depobj:obj1) depend(out:bbb)
;       {
;         // printf( "task1\n");
;       }
;
;       #pragma omp task depend(in:aaa,bbb) depend(out:ccc)
;       {
;         // printf( "task2\n");
;       }
;
;       #pragma omp depobj(obj1) destroy
;     }// single
;   } // parallel
;
;   return 0;
; }
;
; ModuleID = 'task_deparray.ll'
source_filename = "task_deparray.ll"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, i8* }
%struct.kmp_depend_info = type { i64, i64, i8 }

@ccc = dso_local global double 0.000000e+00, align 8
@0 = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
@1 = private unnamed_addr constant %struct.ident_t { i32 0, i32 2, i32 0, i32 22, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @0, i32 0, i32 0) }, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %aaa = alloca i32, align 4
  %bbb = alloca float, align 4
  %obj1 = alloca i8*, align 8
  %depobj.size.addr = alloca i64, align 8
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %dep.counter.addr = alloca i64, align 8
  %.dep.arr.addr1 = alloca [3 x %struct.kmp_depend_info], align 8
  %dep.counter.addr2 = alloca i64, align 8
  %i = call i32 @__kmpc_global_thread_num(%struct.ident_t* @1)
  store i32 0, i32* %retval, align 4
  %.dep.arr.addr = call i8* @__kmpc_alloc(i32 %i, i64 48, i8* null)
  %i1 = bitcast i8* %.dep.arr.addr to %struct.kmp_depend_info*
  %i2 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i1, i32 0, i32 0
  store i64 1, i64* %i2, align 8
  %i3 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %i1, i64 1
  %i4 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i3, i32 0, i32 0
  %i5 = ptrtoint i32* %aaa to i64
  store i64 %i5, i64* %i4, align 8
  %i6 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i3, i32 0, i32 1
  store i64 4, i64* %i6, align 8
  %i7 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i3, i32 0, i32 2
  store i8 3, i8* %i7, align 8
  %i8 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %i1, i64 1
  %i9 = bitcast %struct.kmp_depend_info* %i8 to i8*
  store i8* %i9, i8** %obj1, align 8
  %i10 = load i8*, i8** %obj1, align 8
  %i11 = bitcast i8* %i10 to %struct.kmp_depend_info*
  %i12 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %i11, i64 -1
  %i13 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i12, i32 0, i32 0
  %i14 = load i64, i64* %i13, align 8
  store i64 0, i64* %depobj.size.addr, align 8
  %i15 = load i64, i64* %depobj.size.addr, align 8
  %i16 = add nuw i64 %i15, %i14
  store i64 %i16, i64* %depobj.size.addr, align 8
  %i17 = load i64, i64* %depobj.size.addr, align 8
  %i18 = add nuw i64 0, %i17
  %i19 = add nuw i64 %i18, 1
  %i20 = call i8* @llvm.stacksave()
  store i8* %i20, i8** %saved_stack, align 8
  %vla = alloca %struct.kmp_depend_info, i64 %i19, align 16
  store i64 %i19, i64* %__vla_expr0, align 8
  %dep.array.size = trunc i64 %i19 to i32
  %i22 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %vla, i64 0
  %i23 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i22, i32 0, i32 0
  %i24 = ptrtoint float* %bbb to i64
  store i64 %i24, i64* %i23, align 16
  %i25 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i22, i32 0, i32 1
  store i64 4, i64* %i25, align 8
  %i26 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i22, i32 0, i32 2
  store i8 3, i8* %i26, align 16
  store i64 1, i64* %dep.counter.addr, align 8
  %i27 = load i8*, i8** %obj1, align 8
  %i28 = bitcast i8* %i27 to %struct.kmp_depend_info*
  %i29 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %i28, i64 -1
  %i30 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i29, i32 0, i32 0
  %i31 = load i64, i64* %i30, align 8
  %i32 = mul nuw i64 24, %i31
  %i33 = load i64, i64* %dep.counter.addr, align 8
  %i34 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %vla, i64 %i33
  %i35 = bitcast %struct.kmp_depend_info* %i34 to i8*
  %i36 = bitcast %struct.kmp_depend_info* %i28 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %i35, i8* align 8 %i36, i64 %i32, i1 false)
  %i37 = add nuw i64 %i33, %i31
  store i64 %i37, i64* %dep.counter.addr, align 8
  %dep.array = bitcast %struct.kmp_depend_info* %vla to i8*
  %i39 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.DEPARRAY"(i32 %dep.array.size, i8* %dep.array) ]

; CHECK: call void @__kmpc_omp_task_with_deps(%struct.ident_t* @.kmpc_loc{{.*}}, i32 %{{.*}}, i8* %.task.alloc, i32 %dep.array.size, i8* %dep.array, i32 0, i8* null)

  call void @llvm.directive.region.exit(token %i39) [ "DIR.OMP.END.TASK"() ]
  %i40 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %i40)
  %i41 = getelementptr inbounds [3 x %struct.kmp_depend_info], [3 x %struct.kmp_depend_info]* %.dep.arr.addr1, i64 0, i64 0
  %i42 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %i41, i64 0
  %i43 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i42, i32 0, i32 0
  %i44 = ptrtoint i32* %aaa to i64
  store i64 %i44, i64* %i43, align 8
  %i45 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i42, i32 0, i32 1
  store i64 4, i64* %i45, align 8
  %i46 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i42, i32 0, i32 2
  store i8 1, i8* %i46, align 8
  %i47 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %i41, i64 1
  %i48 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i47, i32 0, i32 0
  %i49 = ptrtoint float* %bbb to i64
  store i64 %i49, i64* %i48, align 8
  %i50 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i47, i32 0, i32 1
  store i64 4, i64* %i50, align 8
  %i51 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i47, i32 0, i32 2
  store i8 1, i8* %i51, align 8
  %i52 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %i41, i64 2
  %i53 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i52, i32 0, i32 0
  store i64 ptrtoint (double* @ccc to i64), i64* %i53, align 8
  %i54 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i52, i32 0, i32 1
  store i64 8, i64* %i54, align 8
  %i55 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %i52, i32 0, i32 2
  store i8 3, i8* %i55, align 8
  store i64 3, i64* %dep.counter.addr2, align 8
  %dep.array.2 = bitcast %struct.kmp_depend_info* %i41 to i8*
  %i57 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.DEPARRAY"(i32 3, i8* %dep.array.2) ]

; CHECK: call void @__kmpc_omp_task_with_deps(%struct.ident_t* @.kmpc_loc{{.*}}, i32 %{{.*}}, i8* %.task.alloc{{.*}}, i32 3, i8* %dep.array.2, i32 0, i8* null)

  call void @llvm.directive.region.exit(token %i57) [ "DIR.OMP.END.TASK"() ]
  %i58 = load i8*, i8** %obj1, align 8
  %i59 = bitcast i8* %i58 to %struct.kmp_depend_info*
  %i60 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %i59, i64 -1
  %i61 = bitcast %struct.kmp_depend_info* %i60 to i8*
  call void @__kmpc_free(i32 %i, i8* %i61, i8* null)
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @__kmpc_global_thread_num(%struct.ident_t* nocapture readonly) #1

; Function Attrs: nounwind
declare i8* @__kmpc_alloc(i32, i64, i8*) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #2

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #3

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8*) #2

; Function Attrs: nounwind
declare void @__kmpc_free(i32, i8*, i8*) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nofree nosync nounwind willreturn }
attributes #3 = { argmemonly nofree nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
