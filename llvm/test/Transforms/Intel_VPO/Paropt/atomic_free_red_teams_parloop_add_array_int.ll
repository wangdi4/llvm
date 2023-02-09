; REQUIRES: asserts
; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -switch-to-offload -vpo-paropt -debug-only=vpo-paropt-transform -S <%s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='vpo-paropt' -debug-only=vpo-paropt-transform -S <%s 2>&1 | FileCheck %s
;
; // test source
; void foo () {
;   unsigned result[16] = { 0 };
;   #pragma omp target teams distribute reduction(+:result)
;   for (auto i = 0; i < 10; ++i)
;     result[i] = 1;
; }
;
; CHECK: Atomic-free reduction global update: Parsed array [16 x i32] addrspace(4)* %result.ascast[16 x i32] as an array section [0 : 16] of i32
;
source_filename = "lit.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@"@tid.addr" = external global i32
@red_buf = extern_weak addrspace(1) global [16 x i32] #0
@teams_counter = private addrspace(1) global i32 0 #1

; Function Attrs: convergent mustprogress noinline nounwind optnone
define protected spir_func void @_Z3foov() #2 {
entry:
  %result = alloca [16 x i32], align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %result.ascast = addrspacecast [16 x i32]* %result to [16 x i32] addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %0 = bitcast [16 x i32] addrspace(4)* %result.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %0, i8 0, i64 64, i1 false)
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store volatile i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.TARGET.12

DIR.OMP.TARGET.12:                                ; preds = %DIR.OMP.TARGET.2
  br label %DIR.OMP.TARGET.12.split

DIR.OMP.TARGET.12.split:                          ; preds = %DIR.OMP.TARGET.12
  %end.dir.temp28 = alloca i1, align 1
  br label %DIR.OMP.TARGET.131

DIR.OMP.TARGET.131:                               ; preds = %DIR.OMP.TARGET.12.split
  br label %DIR.OMP.TARGET.232

DIR.OMP.TARGET.232:                               ; preds = %DIR.OMP.TARGET.131
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
 "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
 "QUAL.OMP.MAP.TOFROM"([16 x i32] addrspace(4)* %result.ascast, [16 x i32] addrspace(4)* %result.ascast, i64 64, i64 547, i8* null, i8* null),
 "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %.omp.iv.ascast),
 "QUAL.OMP.FIRSTPRIVATE:WILOCAL"(i32 addrspace(4)* %.omp.lb.ascast),
 "QUAL.OMP.FIRSTPRIVATE:WILOCAL"(i32 addrspace(4)* %.omp.ub.ascast),
 "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %i.ascast),
 "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %tmp.ascast),
 "QUAL.OMP.OFFLOAD.HAS.TEAMS.REDUCTION"(),
 "QUAL.OMP.MAP.TO"([16 x i32] addrspace(1)* @red_buf, [16 x i32] addrspace(1)* @red_buf, i64 65536, i64 128),
 "QUAL.OMP.MAP.TO"(i32 addrspace(1)* @teams_counter, i32 addrspace(1)* @teams_counter, i64 4, i64 129),
 "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp28) ]

  br label %DIR.OMP.TARGET.333

DIR.OMP.TARGET.333:                               ; preds = %DIR.OMP.TARGET.232
  %temp.load29 = load volatile i1, i1* %end.dir.temp28, align 1
  %cmp30 = icmp ne i1 %temp.load29, false
  br i1 %cmp30, label %DIR.OMP.END.TEAMS.8.split, label %2

2:                                                ; preds = %DIR.OMP.TARGET.333
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %2
  br label %DIR.OMP.TEAMS.4

DIR.OMP.TEAMS.4:                                  ; preds = %DIR.OMP.TARGET.3
  br label %DIR.OMP.TEAMS.2

DIR.OMP.TEAMS.2:                                  ; preds = %DIR.OMP.TEAMS.4
  br label %DIR.OMP.TEAMS.2.split

DIR.OMP.TEAMS.2.split:                            ; preds = %DIR.OMP.TEAMS.2
  %end.dir.temp25 = alloca i1, align 1
  br label %DIR.OMP.TEAMS.434

DIR.OMP.TEAMS.434:                                ; preds = %DIR.OMP.TEAMS.2.split
  br label %DIR.OMP.TEAMS.535

DIR.OMP.TEAMS.535:                                ; preds = %DIR.OMP.TEAMS.434
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
 "QUAL.OMP.REDUCTION.ADD"([16 x i32] addrspace(4)* %result.ascast),
 "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %.omp.iv.ascast),
 "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast),
 "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast),
 "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %i.ascast),
 "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %tmp.ascast),
 "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp25) ]

  br label %DIR.OMP.TEAMS.6

DIR.OMP.TEAMS.6:                                  ; preds = %DIR.OMP.TEAMS.535
  %temp.load26 = load volatile i1, i1* %end.dir.temp25, align 1
  %cmp27 = icmp ne i1 %temp.load26, false
  br i1 %cmp27, label %DIR.OMP.END.DISTRIBUTE.7.split, label %4

4:                                                ; preds = %DIR.OMP.TEAMS.6
  br label %DIR.OMP.TEAMS.5

DIR.OMP.TEAMS.5:                                  ; preds = %4
  br label %DIR.OMP.TEAMS.5.split

DIR.OMP.TEAMS.5.split:                            ; preds = %DIR.OMP.TEAMS.5
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.DISTRIBUTE.7

DIR.OMP.DISTRIBUTE.7:                             ; preds = %DIR.OMP.TEAMS.5.split
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
 "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast),
 "QUAL.OMP.FIRSTPRIVATE:WILOCAL"(i32 addrspace(4)* %.omp.lb.ascast),
 "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast),
 "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %i.ascast),
 "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]

  br label %DIR.OMP.DISTRIBUTE.8

DIR.OMP.DISTRIBUTE.8:                             ; preds = %DIR.OMP.DISTRIBUTE.7
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  %cmp24 = icmp ne i1 %temp.load, false
  br i1 %cmp24, label %omp.loop.exit.split, label %6

6:                                                ; preds = %DIR.OMP.DISTRIBUTE.8
  br label %DIR.OMP.DISTRIBUTE.6

DIR.OMP.DISTRIBUTE.6:                             ; preds = %6
  %7 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store volatile i32 %7, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.DISTRIBUTE.6
  %8 = load volatile i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %9 = load volatile i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %8, %9
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load volatile i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %11 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds [16 x i32], [16 x i32] addrspace(4)* %result.ascast, i64 0, i64 %idxprom
  store i32 1, i32 addrspace(4)* %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load volatile i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %12, 1
  store volatile i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  br label %omp.loop.exit.split

omp.loop.exit.split:                              ; preds = %DIR.OMP.DISTRIBUTE.8, %omp.loop.exit
  br label %DIR.OMP.END.DISTRIBUTE.9

DIR.OMP.END.DISTRIBUTE.9:                         ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.DISTRIBUTE"() ]
  br label %DIR.OMP.END.DISTRIBUTE.7

DIR.OMP.END.DISTRIBUTE.7:                         ; preds = %DIR.OMP.END.DISTRIBUTE.9
  br label %DIR.OMP.END.DISTRIBUTE.7.split

DIR.OMP.END.DISTRIBUTE.7.split:                   ; preds = %DIR.OMP.TEAMS.6, %DIR.OMP.END.DISTRIBUTE.7
  br label %DIR.OMP.END.TEAMS.10

DIR.OMP.END.TEAMS.10:                             ; preds = %DIR.OMP.END.DISTRIBUTE.7.split
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.8

DIR.OMP.END.TEAMS.8:                              ; preds = %DIR.OMP.END.TEAMS.10
  br label %DIR.OMP.END.TEAMS.8.split

DIR.OMP.END.TEAMS.8.split:                        ; preds = %DIR.OMP.TARGET.333, %DIR.OMP.END.TEAMS.8
  br label %DIR.OMP.END.TARGET.11

DIR.OMP.END.TARGET.11:                            ; preds = %DIR.OMP.END.TEAMS.8.split
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.9

DIR.OMP.END.TARGET.9:                             ; preds = %DIR.OMP.END.TARGET.11
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p4i8.i64(i8 addrspace(4)* nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

attributes #0 = { "paropt_red_globalbuf" }
attributes #1 = { "paropt_red_teamscounter" }
attributes #2 = { convergent mustprogress noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly nofree nounwind willreturn writeonly }
attributes #4 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 58, i32 -696809407, !"_Z3foov", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
