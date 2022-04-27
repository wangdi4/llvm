; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S -vpo-paropt-atomic-free-reduction=false %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -vpo-paropt-atomic-free-reduction=false %s | FileCheck %s


; This test is to check the ref.tmp.ascast and ref.tmp1.ascast in private
; clauses with distribute + simd construct are privatized to a global variable
; with local address space.
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

; CHECK: @[[RED_LOCAL_PRIV:[^,]+]] = internal addrspace(3) global [4 x i32] zeroinitializer, align 1
; CHECK: [[TEAM_NUM:[%a-z]+]] = call spir_func i32 @omp_get_team_num()
; CHECK: [[IDX_EXT:[%a-z]+]] = sext i32 [[TEAM_NUM]] to i64
; CHECK: {{.*}} = getelementptr inbounds [4 x i32], [4 x i32] addrspace(3)* @[[RED_LOCAL_PRIV]], i64 0, i64 [[IDX_EXT]]
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)


; ModuleID = 'test0.cpp'
source_filename = "test0.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline norecurse nounwind optnone uwtable mustprogress
define hidden i32 @main(i32 %argc, i8 addrspace(4)* addrspace(4)* %argv) #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %argc.addr = alloca i32, align 4
  %argc.addr.ascast = addrspacecast i32* %argc.addr to i32 addrspace(4)*
  %argv.addr = alloca i8 addrspace(4)* addrspace(4)*, align 8
  %argv.addr.ascast = addrspacecast i8 addrspace(4)* addrspace(4)** %argv.addr to i8 addrspace(4)* addrspace(4)* addrspace(4)*
  %r = alloca [4 x i32], align 4
  %r.ascast = addrspacecast [4 x i32]* %r to [4 x i32] addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 %argc, i32 addrspace(4)* %argc.addr.ascast, align 4
  store i8 addrspace(4)* addrspace(4)* %argv, i8 addrspace(4)* addrspace(4)* addrspace(4)* %argv.addr.ascast, align 8
  %0 = bitcast [4 x i32] addrspace(4)* %r.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %0, i8 0, i64 16, i1 false)
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([4 x i32] addrspace(4)* %r.ascast, [4 x i32] addrspace(4)* %r.ascast, i64 16, i64 547, i8* null, i8* null) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 4), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([4 x i32] addrspace(4)* %r.ascast, i64 1, i64 0, i64 4, i64 1) ]
  %call = call spir_func i32 @omp_get_team_num() #4
  %idxprom = sext i32 %call to i64
  %arrayidx = getelementptr inbounds [4 x i32], [4 x i32] addrspace(4)* %r.ascast, i64 0, i64 %idxprom
  store i32 1, i32 addrspace(4)* %arrayidx, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}


; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly mustprogress
declare void @llvm.memset.p4i8.i64(i8 addrspace(4)* nocapture writeonly %0, i8 %1, i64 %2, i1 immarg %3) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #2

; Function Attrs: convergent nofree nounwind readonly
declare spir_func i32 @omp_get_team_num() local_unnamed_addr #3

attributes #0 = { convergent noinline norecurse nounwind mustprogress "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn writeonly mustprogress }
attributes #2 = { nounwind }
attributes #3 = { convergent nofree nounwind readonly "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #4 = { convergent nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 66311, i32 42092804, !"_Z4main", i32 13, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.0"}
