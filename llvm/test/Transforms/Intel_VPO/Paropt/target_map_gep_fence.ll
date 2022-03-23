; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
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

source_filename = "target_map_gep_fence.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %local = alloca [1 x i32], align 4
  %0 = bitcast [1 x i32]* %local to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([1 x i32]* %local) ]
  br label %DIR.OMP.TARGET.2

; Make sure that vpo-paropt-prepare captures %local to a temporary location.
; PREPR: store [1 x i32]* %local, [1 x i32]** [[LADDR:%[a-zA-Z._0-9]+]]
; PREPR: call token @llvm.directive.region.entry()
; PREPR-SAME: "QUAL.OMP.MAP.TOFROM"([1 x i32]* %local)
; And the Value where %local is store is added to the directive in a
; "QUAL.OMP.OPERAND.ADDR" clause.
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"([1 x i32]* %local, [1 x i32]** [[LADDR]])
;
; Make sure that the above renaming is removed after vpo-restore-operands pass.
; RESTR: call token @llvm.directive.region.entry()
; RESTR-SAME: "QUAL.OMP.MAP.TOFROM"([1 x i32]* %local)
; RESTR-NOT: store [1 x i32]* %local, [1 x i32]** {{%[a-zA-Z._0-9]+}}
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1

; Check that after prepare, uses of %local are replaced with a load from LADDR.
; PREPR: [[LADDR_LOAD:%[a-zA-Z._0-9]+]] = load volatile [1 x i32]*, [1 x i32]** [[LADDR]]
; PREPR: [[GEP1:%[a-zA-Z._0-9]+]] = getelementptr inbounds [1 x i32], [1 x i32]* [[LADDR_LOAD]], i64 0, i64 0
; PREPR: store i32 800, i32* [[GEP1]]

; Check that after restore, LADDR is removed and %local is used.
; RESTR: [[GEP2:%[a-zA-Z._0-9]+]] = getelementptr inbounds [1 x i32], [1 x i32]* %local, i64 0, i64 0
; RESTR: store i32 800, i32* [[GEP2]]

  %arrayidx1 = getelementptr inbounds [1 x i32], [1 x i32]* %local, i64 0, i64 0, !intel-tbaa !3
  store i32 800, i32* %arrayidx1, align 4, !tbaa !3
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.TARGET.2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.END.TARGET.3
  %2 = bitcast [1 x i32]* %local to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #2
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 64770, i32 1081364312, !"main", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA1_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
