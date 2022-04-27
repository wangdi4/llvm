; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-apply-config -simplifycfg -vpo-paropt-config=%S/Inputs/Intel_paropt_apply_config.yaml -S -debug-only=vpo-paropt-apply-config -o /dev/null %s 2>&1 | FileCheck %s
; RUN: opt -passes='require<vpo-paropt-config-analysis>,function(vpo-cfg-restructuring,vpo-paropt-apply-config,simplifycfg)' -vpo-paropt-config=%S/Inputs/Intel_paropt_apply_config.yaml -S -debug-only=vpo-paropt-apply-config -o /dev/null %s 2>&1 | FileCheck %s

; Original code:
; Clauses in test_typed are hand-modified to use the TYPED form.
;void test(int n, int m) {
;#pragma omp target
;  ;
;#pragma omp target teams
;  ;
;#pragma omp target teams num_teams(1)
;  ;
;#pragma omp target teams thread_limit(2)
;  ;
;#pragma omp target teams num_teams(3) thread_limit(4)
;  ;
;#pragma omp target teams num_teams(n)
;  ;
;#pragma omp target teams thread_limit(m)
;  ;
;#pragma omp target teams num_teams(n) thread_limit(m)
;  ;
;}
;
;void test_typed(int n, int m) {
;#pragma omp target
;  ;
;#pragma omp target teams
;  ;
;#pragma omp target teams num_teams(1)
;  ;
;#pragma omp target teams thread_limit(2)
;  ;
;#pragma omp target teams num_teams(3) thread_limit(4)
;  ;
;#pragma omp target teams num_teams(n)
;  ;
;#pragma omp target teams thread_limit(m)
;  ;
;#pragma omp target teams num_teams(n) thread_limit(m)
;  ;
;}

; Check debug messages:
; CHECK: ====== Enter VPO Paropt Apply Config ======
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z4test_l2', index: 0
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '11'
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '22'
; CHECK: VPO Paropt Apply Config: there is no child teams region.
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z4test_l4', index: 1
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '33'
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '44'
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z4test_l6', index: 2
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '55'
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '66'
; CHECK: VPO Paropt Apply Config: config overrides user-specified num_teams: i32 1
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z4test_l8', index: 3
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '77'
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '88'
; CHECK: VPO Paropt Apply Config: config overrides user-specified thread_limit: i32 2
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z4test_l10', index: 4
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '99'
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '111'
; CHECK: VPO Paropt Apply Config: config overrides user-specified thread_limit: i32 4
; CHECK: VPO Paropt Apply Config: config overrides user-specified num_teams: i32 3
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z4test_l12', index: 5
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '222'
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '333'
; CHECK: VPO Paropt Apply Config: config overrides user-specified num_teams:
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z4test_l14', index: 6
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '444'
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '555'
; CHECK: VPO Paropt Apply Config: config overrides user-specified thread_limit:
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z4test_l16', index: 7
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '666'
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '777'
; CHECK: VPO Paropt Apply Config: config overrides user-specified thread_limit:
; CHECK: VPO Paropt Apply Config: config overrides user-specified num_teams:
; CHECK: ====== Exit  VPO Paropt Apply Config ======
; CHECK: ====== Enter VPO Paropt Apply Config ======
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z10test_typed_l21', index: 8
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '11'
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '22'
; CHECK: VPO Paropt Apply Config: there is no child teams region.
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z10test_typed_l23', index: 9
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '44'
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z10test_typed_l25', index: 10
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '66'
; CHECK: VPO Paropt Apply Config: config overrides user-specified num_teams: i32 1
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z10test_typed_l27', index: 11
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '77'
; CHECK: VPO Paropt Apply Config: config overrides user-specified thread_limit: i32 2
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z10test_typed_l29', index: 12
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '99'
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '111'
; CHECK: VPO Paropt Apply Config: config overrides user-specified thread_limit: i32 4
; CHECK: VPO Paropt Apply Config: config overrides user-specified num_teams: i32 3
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z10test_typed_l31', index: 13
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '222'
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z10test_typed_l33', index: 14
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '555'
; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_805_2c801eb__Z10test_typed_l35', index: 15
; CHECK: VPO Paropt Apply Config: config specifies NumTeams: '777'
; CHECK: VPO Paropt Apply Config: config overrides user-specified num_teams:
; CHECK: ====== Exit  VPO Paropt Apply Config ======

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
define hidden spir_func void @test(i32 noundef %n, i32 noundef %m) #0 {
entry:
  %n.addr = alloca i32, align 4
  %m.addr = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %n.addr.ascast = addrspacecast i32* %n.addr to i32 addrspace(4)*
  %m.addr.ascast = addrspacecast i32* %m.addr to i32 addrspace(4)*
  %.capture_expr.0.ascast = addrspacecast i32* %.capture_expr.0 to i32 addrspace(4)*
  %.capture_expr.1.ascast = addrspacecast i32* %.capture_expr.1 to i32 addrspace(4)*
  store i32 %n, i32 addrspace(4)* %n.addr.ascast, align 4, !tbaa !23
  store i32 %m, i32 addrspace(4)* %m.addr.ascast, align 4, !tbaa !23
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 1) ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3) ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.THREAD_LIMIT"(i32 2) ]
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 4) ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 3), "QUAL.OMP.THREAD_LIMIT"(i32 4) ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET"() ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 5), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %n.addr.ascast) ]
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 addrspace(4)* %n.addr.ascast) ]
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TARGET"() ]
  %11 = load i32, i32 addrspace(4)* %m.addr.ascast, align 4, !tbaa !23
  store i32 %11, i32 addrspace(4)* %.capture_expr.0.ascast, align 4, !tbaa !23
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 6), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr.0.ascast) ]
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.THREAD_LIMIT"(i32 addrspace(4)* %.capture_expr.0.ascast) ]
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.TARGET"() ]
  %14 = load i32, i32 addrspace(4)* %m.addr.ascast, align 4, !tbaa !23
  store i32 %14, i32 addrspace(4)* %.capture_expr.1.ascast, align 4, !tbaa !23
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 7), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %n.addr.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr.1.ascast) ]
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 addrspace(4)* %n.addr.ascast), "QUAL.OMP.THREAD_LIMIT"(i32 addrspace(4)* %.capture_expr.1.ascast) ]
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: convergent noinline nounwind
define hidden spir_func void @test_typed(i32 noundef %n, i32 noundef %m) #0 {
entry:
  %n.addr = alloca i32, align 4
  %m.addr = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %n.addr.ascast = addrspacecast i32* %n.addr to i32 addrspace(4)*
  %m.addr.ascast = addrspacecast i32* %m.addr to i32 addrspace(4)*
  %.capture_expr.2.ascast = addrspacecast i32* %.capture_expr.2 to i32 addrspace(4)*
  %.capture_expr.3.ascast = addrspacecast i32* %.capture_expr.3 to i32 addrspace(4)*
  store i32 %n, i32 addrspace(4)* %n.addr.ascast, align 4, !tbaa !23
  store i32 %m, i32 addrspace(4)* %m.addr.ascast, align 4, !tbaa !23
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 8) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 9) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 10) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS:TYPED"(i32 1, i32 0) ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 11) ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.THREAD_LIMIT:TYPED"(i32 2, i32 0) ]
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 12) ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS:TYPED"(i32 3, i32 0), "QUAL.OMP.THREAD_LIMIT:TYPED"(i32 4, i32 0) ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET"() ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 13), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %n.addr.ascast, i32 0, i32 1) ]
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS:TYPED"(i32 addrspace(4)* %n.addr.ascast, i32 0) ]
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TARGET"() ]
  %11 = load i32, i32 addrspace(4)* %m.addr.ascast, align 4, !tbaa !23
  store i32 %11, i32 addrspace(4)* %.capture_expr.2.ascast, align 4, !tbaa !23
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 14), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.capture_expr.2.ascast, i32 0, i32 1) ]
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.THREAD_LIMIT:TYPED"(i32 addrspace(4)* %.capture_expr.2.ascast, i32 0) ]
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.TARGET"() ]
  %14 = load i32, i32 addrspace(4)* %m.addr.ascast, align 4, !tbaa !23
  store i32 %14, i32 addrspace(4)* %.capture_expr.3.ascast, align 4, !tbaa !23
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 15), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %n.addr.ascast, i32 0, i32 1), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.capture_expr.3.ascast, i32 0, i32 1) ]
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS:TYPED"(i32 addrspace(4)* %n.addr.ascast, i32 0), "QUAL.OMP.THREAD_LIMIT:TYPED"(i32 addrspace(4)* %.capture_expr.3.ascast, i32 0) ]
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0, !1, !2, !3, !4, !5, !6, !7, !8, !9, !10, !11, !12, !13, !14, !15}
!llvm.module.flags = !{!16, !17, !18, !19, !20}
!opencl.used.extensions = !{!21}
!opencl.used.optional.core.features = !{!21}
!opencl.compiler.options = !{!21}
!llvm.ident = !{!22}

!0 = !{i32 0, i32 2053, i32 46662123, !"_Z4test", i32 14, i32 6, i32 0}
!1 = !{i32 0, i32 2053, i32 46662123, !"_Z4test", i32 2, i32 0, i32 0}
!2 = !{i32 0, i32 2053, i32 46662123, !"_Z4test", i32 16, i32 7, i32 0}
!3 = !{i32 0, i32 2053, i32 46662123, !"_Z4test", i32 4, i32 1, i32 0}
!4 = !{i32 0, i32 2053, i32 46662123, !"_Z4test", i32 6, i32 2, i32 0}
!5 = !{i32 0, i32 2053, i32 46662123, !"_Z4test", i32 8, i32 3, i32 0}
!6 = !{i32 0, i32 2053, i32 46662123, !"_Z4test", i32 10, i32 4, i32 0}
!7 = !{i32 0, i32 2053, i32 46662123, !"_Z4test", i32 12, i32 5, i32 0}
!8 = !{i32 0, i32 2053, i32 46662123, !"_Z10test_typed", i32 33, i32 14, i32 0}
!9 = !{i32 0, i32 2053, i32 46662123, !"_Z10test_typed", i32 21, i32 8, i32 0}
!10 = !{i32 0, i32 2053, i32 46662123, !"_Z10test_typed", i32 35, i32 15, i32 0}
!11 = !{i32 0, i32 2053, i32 46662123, !"_Z10test_typed", i32 23, i32 9, i32 0}
!12 = !{i32 0, i32 2053, i32 46662123, !"_Z10test_typed", i32 25, i32 10, i32 0}
!13 = !{i32 0, i32 2053, i32 46662123, !"_Z10test_typed", i32 27, i32 11, i32 0}
!14 = !{i32 0, i32 2053, i32 46662123, !"_Z10test_typed", i32 29, i32 12, i32 0}
!15 = !{i32 0, i32 2053, i32 46662123, !"_Z10test_typed", i32 31, i32 13, i32 0}
!16 = !{i32 1, !"wchar_size", i32 4}
!17 = !{i32 7, !"openmp", i32 50}
!18 = !{i32 7, !"openmp-device", i32 50}
!19 = !{i32 7, !"PIC Level", i32 2}
!20 = !{i32 7, !"frame-pointer", i32 2}
!21 = !{}
!22 = !{!"clang version 13.0.0"}
!23 = !{!24, !24, i64 0}
!24 = !{!"int", !25, i64 0}
!25 = !{!"omnipotent char", !26, i64 0}
!26 = !{!"Simple C/C++ TBAA"}
