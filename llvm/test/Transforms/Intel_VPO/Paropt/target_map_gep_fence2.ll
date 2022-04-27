; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
;
; Below is the original C source.
;
; unsigned g_sbox[256];
; int main () {
;    unsigned * sbox = g_sbox;
;    #pragma omp target map(to: sbox[0:256])
;    {
;      int dummy=123;
;    }
;    return 0;
; }
;
; Beore the fix, the early-cse substitutes use of %arrayidx with its definition.
;
; Before early-cse:
;
; entry:
;   %sbox = alloca i32*, align 8
;   store i32* getelementptr inbounds ([256 x i32], [256 x i32]* @g_sbox, i32 0, i32 0), i32** %sbox
;   %1 = load i32*, i32** %sbox, align 8
;   %arrayidx = getelementptr inbounds i32, i32* %1, i64 0
;   br label %DIR.OMP.TARGET.1
;
; DIR.OMP.TARGET.1:                                 ; preds = %entry
;   %2 = call token @llvm.directive.region.entry() [
;          "DIR.OMP.TARGET"(),
;          "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
;          "QUAL.OMP.MAP.TO:AGGRHEAD"(i32** %sbox, i32** %sbox, i64 8),
;          "QUAL.OMP.MAP.TO:AGGR"(i32** %sbox, i32* %arrayidx, i64 1024),
;          ... ]
;
; Make sure that in prepare pass, %arrayidx and %sbox are renamed, even though %arrayidx has
; no use inside the region.

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@g_sbox = common dso_local global [256 x i32] zeroinitializer, align 16
@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %sbox = alloca i32*, align 8
  %dummy = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast i32** %sbox to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #2
  store i32* getelementptr inbounds ([256 x i32], [256 x i32]* @g_sbox, i32 0, i32 0), i32** %sbox, align 8, !tbaa !3
  %1 = load i32*, i32** %sbox, align 8, !tbaa !3
  %arrayidx = getelementptr inbounds i32, i32* %1, i64 0
;
; Make sure that vpo-paropt-prepare captures %local to a temporary location.
; PREPR: store i32** %sbox, i32*** [[SADDR:%[a-zA-Z._0-9]+]]
; PREPR: store i32* %arrayidx, i32** [[AADDR:%[a-zA-Z._0-9]+]]
; PREPR: call token @llvm.directive.region.entry()
; And the Value where %local is store is added to the directive in a
; "QUAL.OMP.OPERAND.ADDR" clause.
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(i32** %sbox, i32*** [[SADDR]])
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(i32* %arrayidx, i32** [[AADDR]])
;
; Make sure that the above renaming is removed after vpo-restore-operands pass.
; RESTR: call token @llvm.directive.region.entry()
; RESTR-SAME: "QUAL.OMP.MAP.TO:AGGRHEAD"(i32** %sbox, i32** %sbox, i64 8)
; RESTR-SAME: "QUAL.OMP.MAP.TO:AGGR"(i32** %sbox, i32* getelementptr inbounds ([256 x i32], [256 x i32]* @g_sbox{{.*}}){{.*}})
; RESTR-NOT: store i32** %sbox, i32*** {{%[a-zA-Z._0-9]+}}
; RESTR-NOT: store i32* %arrayidx, i32** {{AADDR:%[a-zA-Z._0-9]+}}
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO:AGGRHEAD"(i32** %sbox, i32** %sbox, i64 8), "QUAL.OMP.MAP.TO:AGGR"(i32** %sbox, i32* %arrayidx, i64 1024), "QUAL.OMP.PRIVATE"(i32* %dummy) ]
  %3 = bitcast i32* %dummy to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  store i32 123, i32* %dummy, align 4, !tbaa !7
  %4 = bitcast i32* %dummy to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #2
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  %5 = bitcast i32** %sbox to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %5) #2
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

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 46, i32 -1940912277, !"main", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"pointer@_ZTSPj", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !5, i64 0}
