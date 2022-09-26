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
; #pragma omp single copyprivate(a)
;   ;
; }

; CHECK: %[[CP_STRUCT:copyprivate.agg.*]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[CP_A:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[CP_STRUCT]], i32 0, i32 0
; CHECK: store [20 x %class.goo]* %a, [20 x %class.goo]** %[[CP_A]], align 8
; CHECK: %[[CP_STRUCT_BITCAST:.*]] = bitcast %__struct.kmp_copy_privates_t* %[[CP_STRUCT]] to i8*
; CHECK: call void @__kmpc_copyprivate(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 8, i8* %[[CP_STRUCT_BITCAST]], i8* bitcast (void (%__struct.kmp_copy_privates_t*, %__struct.kmp_copy_privates_t*)* @[[CPRIV_CALLBACK:_Z3foov_copy_priv.*]] to i8*), i32 %{{.*}})
; CHECK: define internal void @[[CPRIV_CALLBACK]](%__struct.kmp_copy_privates_t* %[[STRUCT_DST:.*]], %__struct.kmp_copy_privates_t* %[[STRUCT_SRC:.*]])
; CHECK: %[[SRC_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[STRUCT_SRC]], i32 0, i32 0
; CHECK: %[[DST_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[STRUCT_DST]], i32 0, i32 0
; CHECK: %[[SRC:.*]] = load [20 x %class.goo]*, [20 x %class.goo]** %[[SRC_GEP]], align 8
; CHECK: %[[DST:.*]] = load [20 x %class.goo]*, [20 x %class.goo]** %[[DST_GEP]], align 8
; CHECK: %[[DST_BEGIN:.*]] = getelementptr inbounds [20 x %class.goo], [20 x %class.goo]* %[[DST]], i32 0, i32 0
; CHECK: %[[SRC_BEGIN:.*]] = getelementptr inbounds [20 x %class.goo], [20 x %class.goo]* %[[SRC]], i32 0, i32 0
; CHECK: %[[INDEX:.*]] = getelementptr %class.goo, %class.goo* %[[DST_BEGIN]], i32 20
; CHECK: %priv.cpyassn.isempty = icmp eq %class.goo* %[[DST_BEGIN]], %[[INDEX]]
; CHECK: br i1 %priv.cpyassn.isempty, label %priv.cpyassn.done, label %priv.cpyassn.body
; CHECK: %[[CP_DST_PTR:.*]] = phi %class.goo* {{.*}}[ %[[DST_BEGIN]], %{{.*}} ]
; CHECK: %[[CP_SRC_PTR:.*]] = phi %class.goo* {{.*}}[ %[[SRC_BEGIN]], %{{.*}} ]
; CHECK: call void @_ZTS3goo.omp.copy_assign(%class.goo* %[[CP_DST_PTR]], %class.goo* %[[CP_SRC_PTR]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.goo = type { i32 }

$_ZN3gooC2Ev = comdat any

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %a = alloca [20 x %class.goo], align 16
  %array.begin = getelementptr inbounds [20 x %class.goo], [20 x %class.goo]* %a, i32 0, i32 0
  %arrayctor.end = getelementptr inbounds %class.goo, %class.goo* %array.begin, i64 20
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %entry
  %arrayctor.cur = phi %class.goo* [ %array.begin, %entry ], [ %arrayctor.next, %arrayctor.loop ]
  call void @_ZN3gooC2Ev(%class.goo* noundef nonnull align 4 dereferenceable(4) %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.goo, %class.goo* %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq %class.goo* %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %arrayctor.loop
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE:NONPOD"([20 x %class.goo]* %a, void (%class.goo*, %class.goo*)* @_ZTS3goo.omp.copy_assign) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SINGLE"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN3gooC2Ev(%class.goo* noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca %class.goo*, align 8
  store %class.goo* %this, %class.goo** %this.addr, align 8
  %this1 = load %class.goo*, %class.goo** %this.addr, align 8
  %x = getelementptr inbounds %class.goo, %class.goo* %this1, i32 0, i32 0
  store i32 10, i32* %x, align 4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline uwtable
define internal void @_ZTS3goo.omp.copy_assign(%class.goo* noundef %0, %class.goo* noundef %1) #3 {
entry:
  %.addr = alloca %class.goo*, align 8
  %.addr1 = alloca %class.goo*, align 8
  store %class.goo* %0, %class.goo** %.addr, align 8
  store %class.goo* %1, %class.goo** %.addr1, align 8
  %2 = load %class.goo*, %class.goo** %.addr, align 8
  %3 = load %class.goo*, %class.goo** %.addr1, align 8
  %4 = bitcast %class.goo* %2 to i8*
  %5 = bitcast %class.goo* %3 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %4, i8* align 4 %5, i64 4, i1 false)
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #4

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
