; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S -vpo-paropt-atomic-free-reduction=false %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -vpo-paropt-atomic-free-reduction=false %s | FileCheck %s

; Test src:
;
; #include <cstdio>
; #include <omp.h>
; int main() {
;   int r[4] = {0, 0, 0, 0};
; #pragma omp target teams num_teams(4) reduction(+: r[:4])
;   {
;     r[ omp_get_team_num() ] = 1;
;   }
;   return 0;
; }

; The test IR was hand-modified to use a constant section size/offset for
; reduction. CFE currently generates IR instructions to compute them.

; CHECK: @[[RED_LOCAL_PRIV:[^,]+]] = internal addrspace(3) global [4 x i32] zeroinitializer
; CHECK: [[TEAM_NUM:[%a-z]+]] = call spir_func i32 @omp_get_team_num()
; CHECK: [[IDX_EXT:[%a-z]+]] = sext i32 [[TEAM_NUM]] to i64
; CHECK: {{.*}} = getelementptr inbounds [4 x i32], ptr addrspace(3) @[[RED_LOCAL_PRIV]], i64 0, i64 [[IDX_EXT]]
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %r = alloca [4 x i32], align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %r.ascast = addrspacecast ptr %r to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  call void @llvm.memset.p4.i64(ptr addrspace(4) align 4 %r.ascast, i8 0, i64 16, i1 false)
  %arrayidx = getelementptr inbounds [4 x i32], ptr addrspace(4) %r.ascast, i64 0, i64 0

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r.ascast, ptr addrspace(4) %arrayidx, i64 16, i64 547, ptr null, ptr null) ] ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS"(i32 4),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr addrspace(4) %r.ascast, i32 0, i64 4, i64 0) ]

  %call = call spir_func i32 @omp_get_team_num() #4
  %idxprom = sext i32 %call to i64
  %arrayidx1 = getelementptr inbounds [4 x i32], ptr addrspace(4) %r.ascast, i64 0, i64 %idxprom
  store i32 1, ptr addrspace(4) %arrayidx1, align 4

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly mustprogress
declare void @llvm.memset.p4.i64(ptr addrspace(4) nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: convergent nofree nounwind readonly
declare spir_func i32 @omp_get_team_num() local_unnamed_addr #3

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn writeonly mustprogress }
attributes #2 = { nounwind }
attributes #3 = { convergent nofree nounwind readonly "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #4 = { convergent nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66313, i32 186593301, !"_Z4main", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
