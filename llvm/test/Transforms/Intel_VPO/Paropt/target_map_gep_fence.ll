; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR

; Below is the original C source.
;
; Problem: %arrayidx and %arrayidx1 are GEP for local[0] outside and inside of
;          the TARGET region, respectively. Because they are the same expr,
;          early-cse optimizes away %arrayidx1, replacing it with %arrayidx.
;          This makes %arrayidx an unexpected live-in to the TARGET region.
;
; Solution: VPO Prepare uses stores %local to %local.addr. %local.addr is added
;          to the @llvm.directive.region.entry directive in an operand bundle with
;          tag "QUAL.OMP.OPERAND.ADDR", and then all uses of %local inside the
;          region are replaced with a load from %local.addr.
;          This is later undone in a pass called -vpo-remove-operands, before
;          VPO Transform, restoring to the original form.
;
; int main() {
;   int local[1];
;   local[0] = 123;
;   #pragma omp target  map(tofrom: local)
;   {
;      local[0] = 800;
;   }
; }

; Make sure that vpo-paropt-prepare captures %local to a temporary location.
; PREPR: store ptr %local, ptr [[LADDR:%[a-zA-Z._0-9]+]]
; PREPR: call token @llvm.directive.region.entry()

; PREPR-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %local)
; And the Value where %local is store is added to the directive in a
; "QUAL.OMP.OPERAND.ADDR" clause.
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %local, ptr [[LADDR]])

; Check that after prepare, uses of %local are replaced with a load from LADDR.
; PREPR: [[LADDR_LOAD:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[LADDR]]
; PREPR: store i32 800, ptr [[LADDR_LOAD]]

; Make sure that the above renaming is removed after vpo-restore-operands pass.
; RESTR: call token @llvm.directive.region.entry()
; RESTR-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %local)
; RESTR-NOT: store ptr %local, ptr {{%[a-zA-Z._0-9]+}}
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"

; Check that after restore, LADDR is removed and %local is used.
; RESTR: store i32 800, ptr %local

source_filename = "target_map_gep_fence.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() {
entry:
  %local = alloca [1 x i32], align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %local)
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %local) ]
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  store i32 800, ptr %local, align 4
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.TARGET.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.END.TARGET.3
  call void @llvm.lifetime.end.p0(i64 4, ptr %local)
  ret i32 0
}

declare void @llvm.lifetime.start.p0(i64, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0(i64, ptr nocapture)


!omp_offload.info = !{!0}
!0 = !{i32 0, i32 64770, i32 1081364312, !"main", i32 6, i32 0, i32 0}
