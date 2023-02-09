; RUN: opt -enable-new-pm=0 -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -S %s | FileCheck %s
;
; // Evaluate decimal value representation of kmpc_loc.flags for correct flags bit representing type
; // of work sharing construct
;
; // Loop construct flag check ( bit 0x200 )
; CHECK-DAG:   call void @__kmpc_for_static_init_4(%struct.ident_t* @.kmpc_loc.0.0, i32 %{{.*}}, i32 34, i32* %{{.*}}, i32* %{{.*}}, i32* %{{.*}}, i32* %{{.*}}, i32 1, i32 1)
; CHECK-DAG:   @.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838861314, i32 0, i32 0, i8* {{.*}}, {{.*}}, i32 0, i32 0) }
;
; // Sections construct check ( bit 0x400 )
; CHECK-DAG:   call void @__kmpc_for_static_init_4(%struct.ident_t* @.kmpc_loc.0.0.8, i32 %{{.*}}, i32 34, i32* %{{.*}}, i32* %{{.*}}, i32* %{{.*}}, i32* %{{.*}}, i32 1, i32 1)
; CHECK-DAG:   @.kmpc_loc.0.0.8 = private unnamed_addr global %struct.ident_t { i32 0, i32 838861826, i32 0, i32 0, i8* {{.*}}, {{.*}}, i32 0, i32 0) }
;
; // Distribute construct check ( bit 0x800 )
; CHECK-DAG:   call void @__kmpc_dist_for_static_init_4(%struct.ident_t* @.kmpc_loc.0.0.16, i32 %{{.*}}, i32 34, i32* %{{.*}}, i32* %{{.*}}, i32* %{{.*}}, i32* %{{.*}}, i32* %stride, i32 1, i32 1)
; CHECK-DAG:   @.kmpc_loc.0.0.16 = private unnamed_addr global %struct.ident_t { i32 0, i32 838862850, i32 0, i32 0, i8* {{.*}}, {{.*}}, i32 0, i32 0) }
;
; Original code
; #include <stdio.h>
;
; int main(int argc, char **argv) {
;
;  // Test loop detection
;  #pragma omp parallel for num_threads(1)
;  for (int i = 0; i < 3; ++i) {}
;
;  // Test sections detection
;  #pragma omp parallel
;  {
;    #pragma omp sections
;    {
;      #pragma omp section
;      puts("Section 1\n");
;      #pragma omp section
;      puts("Section 2\n");
;    }
;  }
;
;  #pragma omp teams distribute parallel for num_threads(1)
;  for (int i = 0; i < 3; ++i) {}
;
;  return 0;
; }

; ModuleID = 'kmpc_static_loop_flgs.cpp'
source_filename = "kmpc_static_loop_flgs.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [11 x i8] c"Section 1\0A\00", align 1
@.str.1 = private unnamed_addr constant [11 x i8] c"Section 2\0A\00", align 1
@"@tid.addr" = external global i32

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp3 = alloca i32, align 4
  %.omp.iv4 = alloca i32, align 4
  %.omp.lb5 = alloca i32, align 4
  %.omp.ub6 = alloca i32, align 4
  %i10 = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 0, i32* %.omp.lb, align 4
  store volatile i32 2, i32* %.omp.ub, align 4
  %num.sects = alloca i32, align 4
  store volatile i32 1, i32* %num.sects, align 4
  %.sloop.iv.1 = alloca i32, align 4
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %entry
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:                          ; preds = %DIR.OMP.PARALLEL.LOOP.1
  br label %DIR.OMP.PARALLEL.LOOP.118

DIR.OMP.PARALLEL.LOOP.118:                        ; preds = %DIR.OMP.PARALLEL.LOOP.2
  br label %DIR.OMP.PARALLEL.LOOP.118.split

DIR.OMP.PARALLEL.LOOP.118.split:                  ; preds = %DIR.OMP.PARALLEL.LOOP.118
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.LOOP.143

DIR.OMP.PARALLEL.LOOP.143:                        ; preds = %DIR.OMP.PARALLEL.LOOP.118.split
  br label %DIR.OMP.PARALLEL.LOOP.244

DIR.OMP.PARALLEL.LOOP.244:                        ; preds = %DIR.OMP.PARALLEL.LOOP.143
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  br label %DIR.OMP.PARALLEL.LOOP.345

DIR.OMP.PARALLEL.LOOP.345:                        ; preds = %DIR.OMP.PARALLEL.LOOP.244
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  %cmp30 = icmp ne i1 %temp.load, false
  br i1 %cmp30, label %omp.loop.exit.split, label %1

1:                                                ; preds = %DIR.OMP.PARALLEL.LOOP.345
  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %1
  %2 = load i32, i32* %.omp.lb, align 4
  store volatile i32 %2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.PARALLEL.LOOP.3
  %3 = load volatile i32, i32* %.omp.iv, align 4
  %4 = load volatile i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load volatile i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load volatile i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %6, 1
  store volatile i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  br label %omp.loop.exit.split

omp.loop.exit.split:                              ; preds = %DIR.OMP.PARALLEL.LOOP.345, %omp.loop.exit
  br label %DIR.OMP.END.PARALLEL.LOOP.446

DIR.OMP.END.PARALLEL.LOOP.446:                    ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %DIR.OMP.END.PARALLEL.LOOP.446
  br label %DIR.OMP.PARALLEL.5

DIR.OMP.PARALLEL.5:                               ; preds = %DIR.OMP.END.PARALLEL.LOOP.4
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.5
  br label %DIR.OMP.PARALLEL.2.split

DIR.OMP.PARALLEL.2.split:                         ; preds = %DIR.OMP.PARALLEL.2
  %end.dir.temp34 = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.547

DIR.OMP.PARALLEL.547:                             ; preds = %DIR.OMP.PARALLEL.2.split
  br label %DIR.OMP.PARALLEL.648

DIR.OMP.PARALLEL.648:                             ; preds = %DIR.OMP.PARALLEL.547
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* %num.sects), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp34) ]
  br label %DIR.OMP.PARALLEL.7

DIR.OMP.PARALLEL.7:                               ; preds = %DIR.OMP.PARALLEL.648
  %temp.load35 = load volatile i1, i1* %end.dir.temp34, align 1
  %cmp36 = icmp ne i1 %temp.load35, false
  br i1 %cmp36, label %DIR.OMP.END.SECTIONS.14.split, label %8

8:                                                ; preds = %DIR.OMP.PARALLEL.7
  br label %DIR.OMP.PARALLEL.6

DIR.OMP.PARALLEL.6:                               ; preds = %8
  br label %DIR.OMP.PARALLEL.6.split

DIR.OMP.PARALLEL.6.split:                         ; preds = %DIR.OMP.PARALLEL.6
  %end.dir.temp31 = alloca i1, align 1
  br label %DIR.OMP.SECTIONS.8

DIR.OMP.SECTIONS.8:                               ; preds = %DIR.OMP.PARALLEL.6.split
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"(), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.sloop.iv.1, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %num.sects, i32 0), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp31) ]
  br label %DIR.OMP.SECTIONS.9

DIR.OMP.SECTIONS.9:                               ; preds = %DIR.OMP.SECTIONS.8
  %temp.load32 = load volatile i1, i1* %end.dir.temp31, align 1
  %cmp33 = icmp ne i1 %temp.load32, false
  br i1 %cmp33, label %DIR.OMP.END.SECTION.13.split, label %10

10:                                               ; preds = %DIR.OMP.SECTIONS.9
  br label %.sloop.preheader.1

.sloop.header.1:                                  ; preds = %main.sw.succBB.1, %.sloop.preheader.1
  br label %.sloop.body.1

.sloop.body.1:                                    ; preds = %.sloop.header.1
  %11 = load volatile i32, i32* %.sloop.iv.1, align 4
  switch i32 %11, label %main.sw.case0.1 [
    i32 1, label %main.sw.case1.1
  ]

main.sw.succBB.1:                                 ; preds = %main.sw.epilog.1
  %12 = load volatile i32, i32* %.sloop.iv.1, align 4
  %.sloop.inc.1 = add nuw nsw i32 %12, 1
  store volatile i32 %.sloop.inc.1, i32* %.sloop.iv.1, align 4
  %13 = load volatile i32, i32* %.sloop.iv.1, align 4
  %main.sloop.cond.1 = icmp sle i32 %13, %sloop.ub
  br i1 %main.sloop.cond.1, label %.sloop.header.1, label %main.sloop.latch.1

main.sloop.latch.1:                               ; preds = %main.sw.succBB.1
  br label %DIR.OMP.END.SECTION.13

main.sw.case0.1:                                  ; preds = %.sloop.body.1
  br label %DIR.OMP.SECTION.8

DIR.OMP.SECTION.8:                                ; preds = %main.sw.case0.1
  %call = call i32 @puts(i8* noundef getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0)) #1
  br label %DIR.OMP.END.SECTION.9

DIR.OMP.END.SECTION.9:                            ; preds = %DIR.OMP.SECTION.8
  br label %main.sw.epilog.1

main.sw.case1.1:                                  ; preds = %.sloop.body.1
  br label %DIR.OMP.SECTION.11

DIR.OMP.SECTION.11:                               ; preds = %main.sw.case1.1
  %call2 = call i32 @puts(i8* noundef getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i64 0, i64 0)) #1
  br label %DIR.OMP.END.SECTION.12

DIR.OMP.END.SECTION.12:                           ; preds = %DIR.OMP.SECTION.11
  br label %main.sw.epilog.1

DIR.OMP.END.SECTION.13:                           ; preds = %main.sloop.latch.1
  br label %DIR.OMP.END.SECTION.13.split

DIR.OMP.END.SECTION.13.split:                     ; preds = %DIR.OMP.SECTIONS.9, %DIR.OMP.END.SECTION.13
  br label %DIR.OMP.END.SECTIONS.10

DIR.OMP.END.SECTIONS.10:                          ; preds = %DIR.OMP.END.SECTION.13.split
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.SECTIONS"() ]
  br label %DIR.OMP.END.SECTIONS.14

DIR.OMP.END.SECTIONS.14:                          ; preds = %DIR.OMP.END.SECTIONS.10
  br label %DIR.OMP.END.SECTIONS.14.split

DIR.OMP.END.SECTIONS.14.split:                    ; preds = %DIR.OMP.PARALLEL.7, %DIR.OMP.END.SECTIONS.14
  br label %DIR.OMP.END.PARALLEL.11

DIR.OMP.END.PARALLEL.11:                          ; preds = %DIR.OMP.END.SECTIONS.14.split
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.15

DIR.OMP.END.PARALLEL.15:                          ; preds = %DIR.OMP.END.PARALLEL.11
  br label %DIR.OMP.TEAMS.16

DIR.OMP.TEAMS.16:                                 ; preds = %DIR.OMP.END.PARALLEL.15
  br label %DIR.OMP.TEAMS.3

DIR.OMP.TEAMS.3:                                  ; preds = %DIR.OMP.TEAMS.16
  br label %DIR.OMP.TEAMS.3.split

DIR.OMP.TEAMS.3.split:                            ; preds = %DIR.OMP.TEAMS.3
  %end.dir.temp40 = alloca i1, align 1
  br label %DIR.OMP.TEAMS.12

DIR.OMP.TEAMS.12:                                 ; preds = %DIR.OMP.TEAMS.3.split
  br label %DIR.OMP.TEAMS.13

DIR.OMP.TEAMS.13:                                 ; preds = %DIR.OMP.TEAMS.12
  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE:WILOCAL"(i32* %.omp.iv4), "QUAL.OMP.PRIVATE:WILOCAL"(i32* %.omp.lb5), "QUAL.OMP.PRIVATE:WILOCAL"(i32* %.omp.ub6), "QUAL.OMP.PRIVATE:WILOCAL"(i32* %i10), "QUAL.OMP.PRIVATE:WILOCAL"(i32* %tmp3), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp40) ]
  br label %DIR.OMP.TEAMS.14

DIR.OMP.TEAMS.14:                                 ; preds = %DIR.OMP.TEAMS.13
  %temp.load41 = load volatile i1, i1* %end.dir.temp40, align 1
  %cmp42 = icmp ne i1 %temp.load41, false
  br i1 %cmp42, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.21.split, label %15

15:                                               ; preds = %DIR.OMP.TEAMS.14
  br label %DIR.OMP.TEAMS.17

DIR.OMP.TEAMS.17:                                 ; preds = %15
  store i32 0, i32* %.omp.lb5, align 4
  store volatile i32 2, i32* %.omp.ub6, align 4
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.18

DIR.OMP.DISTRIBUTE.PARLOOP.18:                    ; preds = %DIR.OMP.TEAMS.17
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.19

DIR.OMP.DISTRIBUTE.PARLOOP.19:                    ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.18
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.4

DIR.OMP.DISTRIBUTE.PARLOOP.4:                     ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.19
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.4.split

DIR.OMP.DISTRIBUTE.PARLOOP.4.split:               ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.4
  %end.dir.temp37 = alloca i1, align 1
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.15

DIR.OMP.DISTRIBUTE.PARLOOP.15:                    ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.4.split
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.16

DIR.OMP.DISTRIBUTE.PARLOOP.16:                    ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.15
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv4), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb5), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub6), "QUAL.OMP.PRIVATE"(i32* %i10), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp37) ]
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.17

DIR.OMP.DISTRIBUTE.PARLOOP.17:                    ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.16
  %temp.load38 = load volatile i1, i1* %end.dir.temp37, align 1
  %cmp39 = icmp ne i1 %temp.load38, false
  br i1 %cmp39, label %omp.loop.exit17.split, label %17

17:                                               ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.17
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.20

DIR.OMP.DISTRIBUTE.PARLOOP.20:                    ; preds = %17
  %18 = load i32, i32* %.omp.lb5, align 4
  store volatile i32 %18, i32* %.omp.iv4, align 4
  br label %omp.inner.for.cond7

omp.inner.for.cond7:                              ; preds = %omp.inner.for.inc14, %DIR.OMP.DISTRIBUTE.PARLOOP.20
  %19 = load volatile i32, i32* %.omp.iv4, align 4
  %20 = load volatile i32, i32* %.omp.ub6, align 4
  %cmp8 = icmp sle i32 %19, %20
  br i1 %cmp8, label %omp.inner.for.body9, label %omp.inner.for.end16

omp.inner.for.body9:                              ; preds = %omp.inner.for.cond7
  %21 = load volatile i32, i32* %.omp.iv4, align 4
  %mul11 = mul nsw i32 %21, 1
  %add12 = add nsw i32 0, %mul11
  store i32 %add12, i32* %i10, align 4
  br label %omp.body.continue13

omp.body.continue13:                              ; preds = %omp.inner.for.body9
  br label %omp.inner.for.inc14

omp.inner.for.inc14:                              ; preds = %omp.body.continue13
  %22 = load volatile i32, i32* %.omp.iv4, align 4
  %add15 = add nsw i32 %22, 1
  store volatile i32 %add15, i32* %.omp.iv4, align 4
  br label %omp.inner.for.cond7

omp.inner.for.end16:                              ; preds = %omp.inner.for.cond7
  br label %omp.loop.exit17

omp.loop.exit17:                                  ; preds = %omp.inner.for.end16
  br label %omp.loop.exit17.split

omp.loop.exit17.split:                            ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.17, %omp.loop.exit17
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.18

DIR.OMP.END.DISTRIBUTE.PARLOOP.18:                ; preds = %omp.loop.exit17.split
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.21

DIR.OMP.END.DISTRIBUTE.PARLOOP.21:                ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.18
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.21.split

DIR.OMP.END.DISTRIBUTE.PARLOOP.21.split:          ; preds = %DIR.OMP.TEAMS.14, %DIR.OMP.END.DISTRIBUTE.PARLOOP.21
  br label %DIR.OMP.END.TEAMS.19

DIR.OMP.END.TEAMS.19:                             ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.21.split
  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.22

DIR.OMP.END.TEAMS.22:                             ; preds = %DIR.OMP.END.TEAMS.19
  ret i32 0

.sloop.preheader.1:                               ; preds = %10
  %sloop.ub = load volatile i32, i32* %num.sects, align 4
  store volatile i32 0, i32* %.sloop.iv.1, align 4
  br label %.sloop.header.1

main.sw.epilog.1:                                 ; preds = %DIR.OMP.END.SECTION.12, %DIR.OMP.END.SECTION.9
  br label %main.sw.succBB.1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @puts(i8* noundef) #2

attributes #0 = { mustprogress noinline norecurse nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
