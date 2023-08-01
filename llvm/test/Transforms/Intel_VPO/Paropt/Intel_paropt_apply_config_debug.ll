; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-apply-config -simplifycfg -vpo-paropt-config=%S/Inputs/Intel_paropt_apply_config.yaml -S -debug-only=vpo-paropt-apply-config -o /dev/null %s 2>&1 | FileCheck %s
; RUN: opt -passes='require<vpo-paropt-config-analysis>,function(vpo-cfg-restructuring,vpo-paropt-apply-config,simplifycfg)' -vpo-paropt-config=%S/Inputs/Intel_paropt_apply_config.yaml -S -debug-only=vpo-paropt-apply-config -o /dev/null %s 2>&1 | FileCheck %s

; Original code:
; Clauses are hand-modified.
; test() contains IR for untyped ConstantInt clause operands.
; test_typed() contains IR for constructs with no clauses, or typed clauses.

;void test() {
;#pragma omp target teams num_teams(1)
;  ;
;#pragma omp target teams thread_limit(2)
;  ;
;#pragma omp target teams num_teams(3) thread_limit(4)
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

define hidden spir_func void @test() {
entry:
  %i0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2) ]
  %i1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS"(i32 1) ]
  call void @llvm.directive.region.exit(token %i1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %i0) [ "DIR.OMP.END.TARGET"() ]

  %i2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3) ]
  %i3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.THREAD_LIMIT"(i32 2) ]
  call void @llvm.directive.region.exit(token %i3) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %i2) [ "DIR.OMP.END.TARGET"() ]

  %i4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 4) ]
  %i5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS"(i32 3),
    "QUAL.OMP.THREAD_LIMIT"(i32 4) ]
  call void @llvm.directive.region.exit(token %i5) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %i4) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

define hidden spir_func void @test_typed(i32 noundef %n, i32 noundef %m) {
entry:
  %n.addr = alloca i32, align 4
  %m.addr = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %n.addr.ascast = addrspacecast ptr %n.addr to ptr addrspace(4)
  %m.addr.ascast = addrspacecast ptr %m.addr to ptr addrspace(4)
  %.capture_expr.2.ascast = addrspacecast ptr %.capture_expr.2 to ptr addrspace(4)
  %.capture_expr.3.ascast = addrspacecast ptr %.capture_expr.3 to ptr addrspace(4)
  store i32 %n, ptr addrspace(4) %n.addr.ascast, align 4, !tbaa !23
  store i32 %m, ptr addrspace(4) %m.addr.ascast, align 4, !tbaa !23

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 8) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 9) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 10) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(i32 1, i32 0) ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 11) ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(i32 2, i32 0) ]
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]

  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 12) ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(i32 3, i32 0),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(i32 4, i32 0) ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET"() ]

  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 13),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %n.addr.ascast, i32 0, i32 1) ]
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(ptr addrspace(4) %n.addr.ascast, i32 0) ]
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TARGET"() ]

  %11 = load i32, ptr addrspace(4) %m.addr.ascast, align 4, !tbaa !23
  store i32 %11, ptr addrspace(4) %.capture_expr.2.ascast, align 4, !tbaa !23

  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 14),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1) ]
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0) ]
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.TARGET"() ]

  %14 = load i32, ptr addrspace(4) %m.addr.ascast, align 4, !tbaa !23
  store i32 %14, ptr addrspace(4) %.capture_expr.3.ascast, align 4, !tbaa !23

  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 15),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %n.addr.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0, i32 1) ]
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(ptr addrspace(4) %n.addr.ascast, i32 0),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0) ]
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)


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
