; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
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

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@g_sbox = common dso_local global [256 x i32] zeroinitializer, align 16
@"@tid.addr" = external global i32

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %sbox = alloca i32*, align 8
  %dummy = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast i32** %sbox to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0)
  store i32* getelementptr inbounds ([256 x i32], [256 x i32]* @g_sbox, i32 0, i32 0), i32** %sbox, align 8
  %1 = load i32*, i32** %sbox, align 8
  %arrayidx = getelementptr inbounds i32, i32* %1, i64 0
;
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO:AGGRHEAD"(i32** %sbox, i32** %sbox, i64 8),
    "QUAL.OMP.MAP.TO:AGGR"(i32** %sbox, i32* %arrayidx, i64 1024),
    "QUAL.OMP.PRIVATE"(i32* %dummy) ]
  %3 = bitcast i32* %dummy to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3)
  store i32 123, i32* %dummy, align 4
  %4 = bitcast i32* %dummy to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  %5 = bitcast i32** %sbox to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %5)
  ret i32 0
}

declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 46, i32 -1940912277, !"main", i32 4, i32 0, i32 0}
