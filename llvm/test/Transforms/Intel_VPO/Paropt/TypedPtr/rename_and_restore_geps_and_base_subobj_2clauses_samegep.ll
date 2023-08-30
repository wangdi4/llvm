; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -early-cse -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=CSE
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-restore-operands -S %t2.ll -o %t3.ll && FileCheck --input-file=%t3.ll %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %t3.ll | FileCheck %s -check-prefix=TFORM

; RUN: opt -opaque-pointers=0 -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare)" -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -passes="function(early-cse)" -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=CSE
; RUN: opt -opaque-pointers=0 -passes="function(vpo-restore-operands)" -S %t2.ll -o %t3.ll && FileCheck --input-file=%t3.ll %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %t3.ll | FileCheck %s -check-prefix=TFORM

; This is a variation of rename_and_restore_geps_and_base_subobj.ll, with two clauses
; on the same GEP %x on the same directive, to ensure that its renaming/restoring
; doesn't happen twice.
;
; %x, %y and @global get renamed in the prepare pass. %x has two clauses, but it is
; expected to be renamed only once.

; PREPR:      store [4 x i8]* %x, [4 x i8]** %x.addr, align 8
; PREPR:      store i32* %y, i32** %y.addr, align 8
; PREPR:      store [20 x i8]* @global, [20 x i8]** %global.addr, align 8

; PREPR:      "QUAL.OMP.MAP.TOFROM:SUBOBJ"([4 x i8]* %x, [4 x i8]* %x, i32 4, i32 160, i8* null, i8* null)
; PREPR-SAME: "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"([4 x i8]* %x, [4 x i8] zeroinitializer, i32 1),
; PREPR-SAME: "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(i32* %y, i32 0, i32 1),
; PREPR-SAME: "QUAL.OMP.LIVEIN"([20 x i8]* @global),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR:SUBOBJ"([4 x i8]* %x, [4 x i8]** %x.addr),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR:SUBOBJ"(i32* %y, i32** %y.addr),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"([20 x i8]* @global, [20 x i8]** %global.addr)

; PREPR:      [[X_RENAMED:%.*]] = load volatile [4 x i8]*, [4 x i8]** %x.addr, align 8
; PREPR:      [[Y_RENAMED:%.*]] = load volatile i32*, i32** %y.addr, align 8
; PREPR:      [[GLOBAL_RENAMED:%.*]] = load volatile [20 x i8]*, [20 x i8]** %global.addr, align 8

; PREPR:      %arg1 = bitcast [4 x i8]* [[X_RENAMED]] to i8*
; PREPR:      %arg2 = bitcast i32* [[Y_RENAMED]] to i8*
; PREPR:      %arg3 = bitcast [20 x i8]* [[GLOBAL_RENAMED]] to i8*
; PREPR:      call void @bar(i8* %arg1, i8* %arg2, i8* %arg3)


; Early-cse replaces %x and %y with constexpr casts/geps with @global
; as the base. But because of typed pointers, the clauses for %x and the
; original @global are still distinguishable.

; CSE:        "QUAL.OMP.MAP.TOFROM:SUBOBJ"([4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*), [4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*), i32 4, i32 160, i8* null, i8* null)
; CSE-SAME:   "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"([4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*), [4 x i8] zeroinitializer, i32 1),
; CSE-SAME:   "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*), i32 0, i32 1),
; CSE-SAME:   "QUAL.OMP.LIVEIN"([20 x i8]* @global),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR:SUBOBJ"([4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*), [4 x i8]** %x.addr),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR:SUBOBJ"(i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*), i32** %y.addr),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR"([20 x i8]* @global, [20 x i8]** %global.addr)

; CSE:        [[X_RENAMED:%.*]] = load volatile [4 x i8]*, [4 x i8]** %x.addr, align 8
; CSE:        [[Y_RENAMED:%.*]] = load volatile i32*, i32** %y.addr, align 8
; CSE:        [[GLOBAL_RENAMED:%.*]] = load volatile [20 x i8]*, [20 x i8]** %global.addr, align 8

; CSE:        %arg1 = bitcast [4 x i8]* [[X_RENAMED]] to i8*
; CSE:        %arg2 = bitcast i32* [[Y_RENAMED]] to i8*
; CSE:        %arg3 = bitcast [20 x i8]* [[GLOBAL_RENAMED]] to i8*
; CSE:        call void @bar(i8* %arg1, i8* %arg2, i8* %arg3)

; After vpo-restore-operands, the SUBOBJ constant-exprs are broken into individual instructions for %x and %y.
; Doing so isn't necessary for typed pointers because the clause operands for %x and the base @global
; were distinguishable even without this.

; RESTR:      [[X_INST:%.+]] = bitcast [20 x i8]* @global to [4 x i8]*
; RESTR:      [[Y_INST:%.+]] = bitcast i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*

; RESTR:      "QUAL.OMP.MAP.TOFROM:SUBOBJ"([4 x i8]* [[X_INST]], [4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*), i32 4, i32 160, i8* null, i8* null)
; RESTR-SAME: "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"([4 x i8]* [[X_INST]], [4 x i8] zeroinitializer, i32 1)
; RESTR-SAME: "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(i32* [[Y_INST]], i32 0, i32 1)
; RESTR-SAME: "QUAL.OMP.LIVEIN"([20 x i8]* @global)

; RESTR:      %arg1 = bitcast [4 x i8]* [[X_INST]] to i8*
; RESTR:      %arg2 = bitcast i32* [[Y_INST]] to i8*
; RESTR:      %arg3 = bitcast [20 x i8]* @global to i8*
; RESTR:      call void @bar(i8* %arg1, i8* %arg2, i8* %arg3)

; Note: for map clauses with SUBOBJ modifiers, we only need to update the first operand of the map-chain to
; be the renamed X_INST (as it will be the kernel argument). We don't care about the other fields.

; After vpo-paropt, because of the distinction of the various operands, we are able
; to handle replacement of X_INST, Y_INST and @global as different entities.

; TFORM:      define internal void @__omp_offloading_1_2_wibble_l3([4 x i8]* %{{[^ ,]+}})
; TFORM:      [[Y_PRIV:%.*]] = alloca i32, align 4
; TFORM:      [[X_PRIV:%.*]] = alloca [4 x i8], align 1

; TFORM:      %arg1 = bitcast [4 x i8]* [[X_PRIV]] to i8*
; TFORM:      %arg2 = bitcast i32* [[Y_PRIV]] to i8*
; TFORM:      %arg3 = bitcast [20 x i8]* @global to i8*
; TFORM:      call void @bar(i8* %arg1, i8* %arg2, i8* %arg3)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-unknown-linux-gnu"

@global = common unnamed_addr global [20 x i8] zeroinitializer, align 4

declare void @bar(i8*, i8*, i8*)

define void @wibble() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %x = bitcast i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 0) to [4 x i8]*
  %y = bitcast i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*

  %tmp1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM:SUBOBJ"([4 x i8]* %x, [4 x i8]* %x, i32 4, i32 160, i8* null, i8* null),
    "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"([4 x i8]* %x, [4 x i8] zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(i32* %y, i32 0, i32 1),
    "QUAL.OMP.LIVEIN"([20 x i8]* @global) ]

  %arg1 = bitcast [4 x i8]* %x to i8*
  %arg2 = bitcast i32* %y to i8*
  %arg3 = bitcast [20 x i8]* @global to i8*
  call void @bar(i8* %arg1, i8* %arg2, i8* %arg3)

  call void @llvm.directive.region.exit(token %tmp1) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 1, i32 2, !"wibble", i32 3, i32 0, i32 0}
