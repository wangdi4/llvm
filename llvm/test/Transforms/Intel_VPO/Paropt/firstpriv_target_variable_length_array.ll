; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S -vpo-paropt-emit-target-fp-ctor-dtor=true  %s | FileCheck %s --check-prefix=CTORDTOR
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -vpo-paropt-emit-target-fp-ctor-dtor=true %s | FileCheck %s --check-prefix=CTORDTOR
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=NOCTORDTOR
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=NOCTORDTOR

; Test src:
;
; class A {
; public:
; #pragma omp declare target
;   A();
; #pragma omp end declare target
; };
; void fn1(int n) {
;   A e[n];
; #pragma omp target firstprivate(e)
;   for (int d = 0; d < 100; d++)
;     ;
; }

; This test is used to check emitting loop of calling copy constructor for
; variable length array type with target construct + firstprivate clause.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.A = type { i8 }

; Function Attrs: convergent mustprogress noinline nounwind optnone
define protected spir_func void @_Z3fn1i(i32 noundef %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  %d = alloca i32, align 4
  %n.addr.ascast = addrspacecast ptr %n.addr to ptr addrspace(4)
  %saved_stack.ascast = addrspacecast ptr %saved_stack to ptr addrspace(4)
  %__vla_expr0.ascast = addrspacecast ptr %__vla_expr0 to ptr addrspace(4)
  %omp.vla.tmp.ascast = addrspacecast ptr %omp.vla.tmp to ptr addrspace(4)
  %d.ascast = addrspacecast ptr %d to ptr addrspace(4)
  store i32 %n, ptr addrspace(4) %n.addr.ascast, align 4
  %0 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr addrspace(4) %saved_stack.ascast, align 8
  %vla = alloca %class.A, i64 %1, align 1
  %vla.ascast = addrspacecast ptr %vla to ptr addrspace(4)
  store i64 %1, ptr addrspace(4) %__vla_expr0.ascast, align 8
  %isempty = icmp eq i64 %1, 0
  br i1 %isempty, label %arrayctor.cont, label %new.ctorloop

new.ctorloop:                                     ; preds = %entry
  %arrayctor.end = getelementptr inbounds %class.A, ptr addrspace(4) %vla.ascast, i64 %1
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %new.ctorloop
  %arrayctor.cur = phi ptr addrspace(4) [ %vla.ascast, %new.ctorloop ], [ %arrayctor.next, %arrayctor.loop ]
  call spir_func void @_ZN1AC1Ev(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1) %arrayctor.cur) #5
  %arrayctor.next = getelementptr inbounds %class.A, ptr addrspace(4) %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr addrspace(4) %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %entry, %arrayctor.loop
  store i64 %1, ptr addrspace(4) %omp.vla.tmp.ascast, align 8
  %3 = mul nuw i64 %1, 1
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr addrspace(4) %vla.ascast, %class.A zeroinitializer, i64 %1, ptr @_ZTS1A.omp.copy_constr, ptr @_ZTS1A.omp.destr),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %vla.ascast, ptr addrspace(4) %vla.ascast, i64 %3, i64 161, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %d.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.vla.tmp.ascast, i64 0, i32 1) ]
  %5 = load i64, ptr addrspace(4) %omp.vla.tmp.ascast, align 8
  store i32 0, ptr addrspace(4) %d.ascast, align 4
  br label %for.cond

; Copy constructor
; CTORDTOR:  %[[TO:[^,]+]] = getelementptr inbounds %class.A, ptr %vla.ascast.fpriv, i32 0
; CTORDTOR-NEXT:  %[[FROM:[^,]+]] = getelementptr inbounds %class.A, ptr addrspace(1) %vla.ascast, i32 0
; CTORDTOR-NEXT:  %[[END:[^,]+]] = getelementptr %class.A, ptr %[[TO]], i64 %[[VLA_LEN:[^,]+]]
; CTORDTOR-NEXT:  %priv.cpyctor.isempty = icmp eq ptr %[[TO]], %[[END]]
; CTORDTOR-NEXT:  br i1 %priv.cpyctor.isempty, label %priv.cpyctor.done, label %priv.cpyctor.body
; CTORDTOR-LABEL: priv.cpyctor.body:
; CTORDTOR-NEXT:  %priv.cpy.dest.ptr = phi ptr [ %[[TO]], %{{.*}} ], [ %priv.cpy.dest.inc, %{{.*}} ]
; CTORDTOR-NEXT:  %priv.cpy.src.ptr = phi ptr addrspace(1) [ %[[FROM]], %{{.*}} ], [ %priv.cpy.src.inc, %{{.*}} ]
; CTORDTOR:  call spir_func void @_ZTS1A.omp.copy_constr(ptr addrspace(4) %{{.*}}, ptr addrspace(4) %{{.*}})
; CTORDTOR:  %priv.cpy.dest.inc = getelementptr %class.A, ptr %priv.cpy.dest.ptr, i32 1
; CTORDTOR-NEXT:  %priv.cpy.src.inc = getelementptr %class.A, ptr addrspace(1) %priv.cpy.src.ptr, i32 1
; CTORDTOR-NEXT:  %priv.cpy.done = icmp eq ptr %priv.cpy.dest.inc, %[[END]]
; CTORDTOR-NEXT:  br i1 %priv.cpy.done, label %priv.cpyctor.done, label %priv.cpyctor.body
; CTORDTOR-LABEL: priv.cpyctor.done:
; CTORDTOR-NEXT:  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CTORDTOR-NEXT:  br i1 %is.master.thread, label %master.thread.code{{[0-9]*}}, label %master.thread.fallthru{{[0-9]*}}

; Make sure we don't call the ctor/dtor unless requested
; NOCTORDTOR-NOT: call {{.*}}@_ZTS1A.omp.copy_constr

for.cond:                                         ; preds = %for.inc, %arrayctor.cont
  %6 = load i32, ptr addrspace(4) %d.ascast, align 4
  %cmp = icmp slt i32 %6, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %7 = load i32, ptr addrspace(4) %d.ascast, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, ptr addrspace(4) %d.ascast, align 4
  br label %for.cond, !llvm.loop !8

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  %8 = load ptr, ptr addrspace(4) %saved_stack.ascast, align 8
  call void @llvm.stackrestore(ptr %8)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #1

; Function Attrs: convergent
declare spir_func void @_ZN1AC1Ev(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1)) unnamed_addr #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: convergent noinline nounwind
define internal void @_ZTS1A.omp.copy_constr(ptr addrspace(4) noundef %0, ptr addrspace(4) noundef %1) #4 {
entry:
  %.addr = alloca ptr addrspace(4), align 8
  %.addr1 = alloca ptr addrspace(4), align 8
  %.addr.ascast = addrspacecast ptr %.addr to ptr addrspace(4)
  %.addr1.ascast = addrspacecast ptr %.addr1 to ptr addrspace(4)
  store ptr addrspace(4) %0, ptr addrspace(4) %.addr.ascast, align 8
  store ptr addrspace(4) %1, ptr addrspace(4) %.addr1.ascast, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) %.addr.ascast, align 8
  %3 = load ptr addrspace(4), ptr addrspace(4) %.addr1.ascast, align 8
  ret void
}

; Function Attrs: convergent noinline nounwind
define internal spir_func void @_ZTS1A.omp.destr(ptr addrspace(4) noundef %0) #4 section ".text.startup" {
entry:
  %.addr = alloca ptr addrspace(4), align 8
  %.addr.ascast = addrspacecast ptr %.addr to ptr addrspace(4)
  store ptr addrspace(4) %0, ptr addrspace(4) %.addr.ascast, align 8
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #1

attributes #0 = { convergent mustprogress noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }
attributes #4 = { convergent noinline nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #5 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1923835737, !"_Z3fn1i", i32 9, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
