; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

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
;   goo a[n], b[20];
; // #pragma omp single copyprivate(b)
; #pragma omp single copyprivate(a)
;   ;
; }

; Since the frontend does not allow VLA in the copyprivate clause, the IR is modified to include VLA manually.

; CHECK: %[[CP_STRUCT:copyprivate.agg.*]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[CP_ARRAY_INFO:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[CP_STRUCT]], i32 0, i32 0
; CHECK: %[[CP_A:.*]] = getelementptr inbounds %[[ARRAY_INFO:__struct.kmp_copy_privates_t.array.*]], %[[ARRAY_INFO]]* %[[CP_ARRAY_INFO]], i32 0, i32 0
; CHECK: store %class.goo* %vla, %class.goo** %[[CP_A]], align 8
; CHECK: %[[CP_A_SIZE:.*]] = getelementptr inbounds %[[ARRAY_INFO]], %[[ARRAY_INFO]]* %[[CP_ARRAY_INFO]], i32 0, i32 1
; CHECK: store i64 %0, i64* %[[CP_A_SIZE]], align 8
; CHECK: %[[CP_STRUCT_BITCAST:.*]] = bitcast %__struct.kmp_copy_privates_t* %[[CP_STRUCT]] to i8*
; CHECK: call void @__kmpc_copyprivate(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 16, i8* %[[CP_STRUCT_BITCAST]], i8* bitcast (void (%__struct.kmp_copy_privates_t*, %__struct.kmp_copy_privates_t*)* @[[CPRIV_CALLBACK:_Z3fool_copy_priv.*]] to i8*), i32 %{{.*}})

; CHECK: define internal void @[[CPRIV_CALLBACK]](%__struct.kmp_copy_privates_t* %[[STRUCT_DST:.*]], %__struct.kmp_copy_privates_t* %[[STRUCT_SRC:.*]])
; CHECK: %[[SRC_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[STRUCT_SRC]], i32 0, i32 0
; CHECK: %[[DST_INFO_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[STRUCT_DST]], i32 0, i32 0
; CHECK: %[[SRC_GEP:.*]] = getelementptr inbounds %[[ARRAY_INFO]], %[[ARRAY_INFO]]* %[[SRC_INFO_GEP]], i32 0, i32 0
; CHECK: %[[SRC:.*]] = load %class.goo*, %class.goo** %[[SRC_GEP]], align 8
; CHECK: %[[DST_GEP:.*]] = getelementptr inbounds %[[ARRAY_INFO]], %[[ARRAY_INFO]]* %[[DST_INFO_GEP]], i32 0, i32 0
; CHECK: %[[DST:.*]] = load %class.goo*, %class.goo** %[[DST_GEP]], align 8
; CHECK: %[[SRC_SIZE_GEP:.*]] = getelementptr inbounds %[[ARRAY_INFO]], %[[ARRAY_INFO]]* %[[SRC_INFO_GEP]], i32 0, i32 1
; CHECK: %[[SRC_SIZE:.*]] = load i64, i64* %[[SRC_SIZE_GEP]], align 8
; CHECK: %[[DST_BEGIN:.*]] = getelementptr inbounds %class.goo, %class.goo* %[[DST]], i32 0
; CHECK: %[[SRC_BEGIN:.*]] = getelementptr inbounds %class.goo, %class.goo* %[[SRC]], i32 0
; CHECK: %[[INDEX:.*]] = getelementptr %class.goo, %class.goo* %[[DST_BEGIN]], i64 %[[SRC_SIZE]]
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
define dso_local void @_Z3fool(i64 noundef %n) #0 {
entry:
  %n.addr = alloca i64, align 8
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %b = alloca [20 x %class.goo], align 16
  store i64 %n, i64* %n.addr, align 8
  %0 = load i64, i64* %n.addr, align 8
  %1 = call i8* @llvm.stacksave()
  store i8* %1, i8** %saved_stack, align 8
  %vla = alloca %class.goo, i64 %0, align 16
  store i64 %0, i64* %__vla_expr0, align 8
  %isempty = icmp eq i64 %0, 0
  br i1 %isempty, label %arrayctor.cont, label %new.ctorloop

new.ctorloop:                                     ; preds = %entry
  %arrayctor.end = getelementptr inbounds %class.goo, %class.goo* %vla, i64 %0
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %new.ctorloop
  %arrayctor.cur = phi %class.goo* [ %vla, %new.ctorloop ], [ %arrayctor.next, %arrayctor.loop ]
  call void @_ZN3gooC2Ev(%class.goo* noundef nonnull align 4 dereferenceable(4) %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.goo, %class.goo* %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq %class.goo* %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %entry, %arrayctor.loop
  %array.begin = getelementptr inbounds [20 x %class.goo], [20 x %class.goo]* %b, i32 0, i32 0
  %arrayctor.end1 = getelementptr inbounds %class.goo, %class.goo* %array.begin, i64 20
  br label %arrayctor.loop2

arrayctor.loop2:                                  ; preds = %arrayctor.loop2, %arrayctor.cont
  %arrayctor.cur3 = phi %class.goo* [ %array.begin, %arrayctor.cont ], [ %arrayctor.next4, %arrayctor.loop2 ]
  call void @_ZN3gooC2Ev(%class.goo* noundef nonnull align 4 dereferenceable(4) %arrayctor.cur3)
  %arrayctor.next4 = getelementptr inbounds %class.goo, %class.goo* %arrayctor.cur3, i64 1
  %arrayctor.done5 = icmp eq %class.goo* %arrayctor.next4, %arrayctor.end1
  br i1 %arrayctor.done5, label %arrayctor.cont6, label %arrayctor.loop2

arrayctor.cont6:                                  ; preds = %arrayctor.loop2
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE:NONPOD"(%class.goo* %vla, void (%class.goo*, %class.goo*)* @_ZTS3goo.omp.copy_assign) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]
  %3 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %3)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #1

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN3gooC2Ev(%class.goo* noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %class.goo*, align 8
  store %class.goo* %this, %class.goo** %this.addr, align 8
  %this1 = load %class.goo*, %class.goo** %this.addr, align 8
  %x = getelementptr inbounds %class.goo, %class.goo* %this1, i32 0, i32 0
  store i32 10, i32* %x, align 4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: noinline uwtable
define internal void @_ZTS3goo.omp.copy_assign(%class.goo* noundef %0, %class.goo* noundef %1) #4 {
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
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #5

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8*) #1

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
