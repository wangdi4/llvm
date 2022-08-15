; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; class goo {
; public:
;   int x;
;   goo() { x = 10; }
; };
;
; void foo() {
;   goo a[20];
;   goo (&b)[20] = a;
; #pragma omp single copyprivate(b)
;   ;
; }

; CHECK: store ptr %a, ptr %b, align 8
; CHECK-NEXT: %[[BREF:.*]] = load ptr, ptr %b, align 8
; CHECK: %[[CP_STRUCT:copyprivate.agg.*]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[CP_BREF:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[CP_STRUCT]], i32 0, i32 0
; CHECK: store ptr %[[BREF]], ptr %[[CP_BREF]], align 8
; CHECK: call void @__kmpc_copyprivate(ptr @{{.*}}, i32 %{{.*}}, i32 8, ptr %[[CP_STRUCT]], ptr @[[CPRIV_CALLBACK:_Z3foov_copy_priv.*]], i32 %{{.*}})
; CHECK: define internal void @[[CPRIV_CALLBACK]](ptr %[[STRUCT_DST:.*]], ptr %[[STRUCT_SRC:.*]])
; CHECK: %[[SRC_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_SRC]], i32 0, i32 0
; CHECK: %[[DST_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_DST]], i32 0, i32 0
; CHECK: %[[SRC:.*]] = load ptr, ptr %[[SRC_GEP]], align 8
; CHECK: %[[DST:.*]] = load ptr, ptr %[[DST_GEP]], align 8
; CHECK: %[[DST_BEGIN:.*]] = getelementptr inbounds [20 x %class.goo], ptr %[[DST]], i32 0
; CHECK: %[[SRC_BEGIN:.*]] = getelementptr inbounds [20 x %class.goo], ptr %[[SRC]], i32 0
; CHECK: %[[INDEX:.*]] = getelementptr %class.goo, ptr %[[DST_BEGIN]], i32 20
; CHECK: %priv.cpyassn.isempty = icmp eq ptr %[[DST_BEGIN]], %[[INDEX]]
; CHECK: br i1 %priv.cpyassn.isempty, label %priv.cpyassn.done, label %priv.cpyassn.body
; CHECK: %[[CP_DST_PTR:.*]] = phi ptr {{.*}}[ %[[DST_BEGIN]], %{{.*}} ]
; CHECK: %[[CP_SRC_PTR:.*]] = phi ptr {{.*}}[ %[[SRC_BEGIN]], %{{.*}} ]
; CHECK: call void @_ZTS3goo.omp.copy_assign(ptr %[[CP_DST_PTR]], ptr %[[CP_SRC_PTR]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.goo = type { i32 }

$_ZN3gooC2Ev = comdat any

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %a = alloca [20 x %class.goo], align 16
  %b = alloca ptr, align 8
  %array.begin = getelementptr inbounds [20 x %class.goo], ptr %a, i32 0, i32 0
  %arrayctor.end = getelementptr inbounds %class.goo, ptr %array.begin, i64 20
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %entry
  %arrayctor.cur = phi ptr [ %array.begin, %entry ], [ %arrayctor.next, %arrayctor.loop ]
  call void @_ZN3gooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.goo, ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %arrayctor.loop
  store ptr %a, ptr %b, align 8
  %0 = load ptr, ptr %b, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE:NONPOD.TYPED"(ptr %0, [20 x %class.goo] zeroinitializer, i32 1, ptr @_ZTS3goo.omp.copy_assign) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN3gooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %x = getelementptr inbounds %class.goo, ptr %this1, i32 0, i32 0
  store i32 10, ptr %x, align 4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline uwtable
define internal void @_ZTS3goo.omp.copy_assign(ptr noundef %0, ptr noundef %1) #3 {
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
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #4

attributes #0 = { mustprogress noinline optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { argmemonly nofree nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
