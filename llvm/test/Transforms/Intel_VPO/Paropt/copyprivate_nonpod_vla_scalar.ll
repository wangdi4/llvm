; RUN: opt -opaque-pointers=1 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S -verify %s | FileCheck %s
; RUN: opt -opaque-pointers=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,verify)' -S %s | FileCheck %s

; Test src:
;
; #include <stdlib.h>
;
; class goo {
; public:
;   int x;
;   goo() { x = 10; }
; };
;
; void foo(int64_t n) {
;   goo a[n], b[20], c;
; // #pragma omp single copyprivate(b, c)
; #pragma omp single copyprivate(a, c)
;   ;
; }

; Since the frontend does not allow VLA in the copyprivate clause, the IR is modified to include VLA manually.

; Check the different copyprivate structs for array A and scalar C.
; CHECK: %__struct.kmp_copy_privates_t = type { %[[ARRAY_INFO:__struct.kmp_copy_privates_t.array.*]], ptr }
; CHECK: %[[ARRAY_INFO]] = type { ptr, i64 }

; Check storing the array A's starting addr and size
; CHECK: %[[CP_STRUCT:copyprivate.agg.*]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[CP_ARRAY_INFO:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[CP_STRUCT]], i32 0, i32 0
; CHECK: %[[CP_A:.*]] = getelementptr inbounds %[[ARRAY_INFO]], ptr %[[CP_ARRAY_INFO]], i32 0, i32 0
; CHECK: store ptr %vla, ptr %[[CP_A]], align 8
; CHECK: %[[CP_A_SIZE:.*]] = getelementptr inbounds %[[ARRAY_INFO]], ptr %[[CP_ARRAY_INFO]], i32 0, i32 1
; CHECK: store i64 %n, ptr %[[CP_A_SIZE]], align 8

; Check storing the scalar C's addr
; CHECK: %[[CP_C:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[CP_STRUCT]], i32 0, i32 1
; CHECK: store ptr %c, ptr %[[CP_C]], align 8
; CHECK: call void @__kmpc_copyprivate(ptr @{{.*}}, i32 %{{.*}}, i32 24, ptr %[[CP_STRUCT]], ptr @[[CPRIV_CALLBACK:_Z3fool_copy_priv.*]], i32 %{{.*}})

; Check loading the array A's starting addr and size
; CHECK: define internal void @[[CPRIV_CALLBACK]](ptr %[[STRUCT_DST:.*]], ptr %[[STRUCT_SRC:.*]])
; CHECK: %[[A_SRC_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_SRC]], i32 0, i32 0
; CHECK: %[[A_DST_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_DST]], i32 0, i32 0
; CHECK: %[[A_SRC_GEP:.*]] = getelementptr inbounds %[[ARRAY_INFO]], ptr %[[A_SRC_INFO_GEP]], i32 0, i32 0
; CHECK: %[[A_SRC:.*]] = load ptr, ptr %[[A_SRC_GEP]], align 8
; CHECK: %[[A_DST_GEP:.*]] = getelementptr inbounds %[[ARRAY_INFO]], ptr %[[A_DST_INFO_GEP]], i32 0, i32 0
; CHECK: %[[A_DST:.*]] = load ptr, ptr %[[A_DST_GEP]], align 8
; CHECK: %[[A_SRC_SIZE_GEP:.*]] = getelementptr inbounds %[[ARRAY_INFO]], ptr %[[A_SRC_INFO_GEP]], i32 0, i32 1
; CHECK: %[[A_SRC_SIZE:.*]] = load i64, ptr %[[A_SRC_SIZE_GEP]], align 8
; CHECK: %[[A_DST_BEGIN:.*]] = getelementptr inbounds %class.goo, ptr %[[A_DST]], i32 0
; CHECK: %[[A_SRC_BEGIN:.*]] = getelementptr inbounds %class.goo, ptr %[[A_SRC]], i32 0
; CHECK: %[[A_CP_DST_PTR:.*]] = phi ptr {{.*}}[ %[[A_DST_BEGIN]], %{{.*}} ]
; CHECK: %[[A_CP_SRC_PTR:.*]] = phi ptr {{.*}}[ %[[A_SRC_BEGIN]], %{{.*}} ]
; CHECK: call void @_ZTS3goo.omp.copy_assign(ptr %[[A_CP_DST_PTR]], ptr %[[A_CP_SRC_PTR]])

; Check loading the scalar C's addr
; CHECK: %[[C_SRC_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_SRC]], i32 0, i32 1
; CHECK: %[[C_DST_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_DST]], i32 0, i32 1
; CHECK: %[[C_SRC:.*]] = load ptr, ptr %[[C_SRC_GEP]], align 8
; CHECK: %[[C_DST:.*]] = load ptr, ptr %[[C_DST_GEP]], align 8
; CHECK: call void @_ZTS3goo.omp.copy_assign(ptr %[[C_DST]], ptr %[[C_SRC]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.goo = type { i32 }

$_ZN3gooC2Ev = comdat any

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z3fool(i64 noundef %n) #0 {
entry:
  %n.addr = alloca i64, align 8
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %b = alloca [20 x %class.goo], align 16
  %c = alloca %class.goo, align 4
  store i64 %n, ptr %n.addr, align 8
  %0 = load i64, ptr %n.addr, align 8
  %1 = call ptr @llvm.stacksave()
  store ptr %1, ptr %saved_stack, align 8
  %vla = alloca %class.goo, i64 %0, align 16
  store i64 %0, ptr %__vla_expr0, align 8
  %isempty = icmp eq i64 %0, 0
  br i1 %isempty, label %arrayctor.cont, label %new.ctorloop

new.ctorloop:                                     ; preds = %entry
  %arrayctor.end = getelementptr inbounds %class.goo, ptr %vla, i64 %0
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %new.ctorloop
  %arrayctor.cur = phi ptr [ %vla, %new.ctorloop ], [ %arrayctor.next, %arrayctor.loop ]
  call void @_ZN3gooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.goo, ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %entry, %arrayctor.loop
  %array.begin = getelementptr inbounds [20 x %class.goo], ptr %b, i32 0, i32 0
  %arrayctor.end1 = getelementptr inbounds %class.goo, ptr %array.begin, i64 20
  br label %arrayctor.loop2

arrayctor.loop2:                                  ; preds = %arrayctor.loop2, %arrayctor.cont
  %arrayctor.cur3 = phi ptr [ %array.begin, %arrayctor.cont ], [ %arrayctor.next4, %arrayctor.loop2 ]
  call void @_ZN3gooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %arrayctor.cur3)
  %arrayctor.next4 = getelementptr inbounds %class.goo, ptr %arrayctor.cur3, i64 1
  %arrayctor.done5 = icmp eq ptr %arrayctor.next4, %arrayctor.end1
  br i1 %arrayctor.done5, label %arrayctor.cont6, label %arrayctor.loop2

arrayctor.cont6:                                  ; preds = %arrayctor.loop2
  call void @_ZN3gooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %c)
  %array.begin7 = getelementptr inbounds [20 x %class.goo], ptr %b, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE:NONPOD.TYPED"(ptr %vla, %class.goo zeroinitializer, i64 %n, ptr @_ZTS3goo.omp.copy_assign),
    "QUAL.OMP.COPYPRIVATE:NONPOD.TYPED"(ptr %c, %class.goo zeroinitializer, i32 1, ptr @_ZTS3goo.omp.copy_assign) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]
  %3 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %3)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #1

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN3gooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %x = getelementptr inbounds %class.goo, ptr %this1, i32 0, i32 0
  store i32 10, ptr %x, align 4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: noinline uwtable
define internal void @_ZTS3goo.omp.copy_assign(ptr noundef %0, ptr noundef %1) #4 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr, align 8
  %3 = load ptr, ptr %.addr1, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %2, ptr align 4 %3, i64 4, i1 false)
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #5

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #1

attributes #0 = { mustprogress noinline optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }
attributes #4 = { noinline uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #5 = { argmemonly nofree nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
