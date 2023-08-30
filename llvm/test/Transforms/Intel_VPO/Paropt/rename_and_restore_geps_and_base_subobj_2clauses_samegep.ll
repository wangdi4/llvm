; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -early-cse -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=CSE
; RUN: opt -bugpoint-enable-legacy-pm -vpo-restore-operands -S %t2.ll -o %t3.ll && FileCheck --input-file=%t3.ll %s -check-prefix=RESTR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %t3.ll | FileCheck %s -check-prefix=TFORM

; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare)" -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -passes="function(early-cse)" -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=CSE
; RUN: opt -passes="function(vpo-restore-operands)" -S %t2.ll -o %t3.ll && FileCheck --input-file=%t3.ll %s -check-prefix=RESTR
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %t3.ll | FileCheck %s -check-prefix=TFORM

; This is a variation of rename_and_restore_geps_and_base_subobj.ll, with two clauses
; on the same GEP %x on the same directive, to ensure that its renaming/restoring
; doesn't happen twice.
;
; %x, %y and @global get renamed in the prepare pass. %x has two clauses, but it is
; expected to be renamed only once.

; PREPR:      store ptr %x, ptr %x.addr, align 8
; PREPR:      store ptr %y, ptr %y.addr, align 8
; PREPR:      store ptr @global, ptr %global.addr, align 8

; PREPR:      "QUAL.OMP.MAP.TOFROM:SUBOBJ"(ptr %x, ptr %x, i32 4, i32 160, ptr null, ptr null)
; PREPR-SAME: "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr %x, [4 x i8] zeroinitializer, i32 1),
; PREPR-SAME: "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr %y, i32 0, i32 1),
; PREPR-SAME: "QUAL.OMP.LIVEIN"(ptr @global),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR:SUBOBJ"(ptr %x, ptr %x.addr),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR:SUBOBJ"(ptr %y, ptr %y.addr),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr @global, ptr %global.addr)

; PREPR:      [[X_RENAMED:%.*]] = load volatile ptr, ptr %x.addr, align 8
; PREPR:      [[Y_RENAMED:%.*]] = load volatile ptr, ptr %y.addr, align 8
; PREPR:      [[GLOBAL_RENAMED:%.*]] = load volatile ptr, ptr %global.addr, align 8

; PREPR:      call void @bar(ptr [[X_RENAMED]], ptr [[Y_RENAMED]], ptr [[GLOBAL_RENAMED]])


; But early-cse replaces %x with @global and %y with a const-expr GEP on @global.
; This means %x and @global are not distinguishable.

; CSE:        "QUAL.OMP.MAP.TOFROM:SUBOBJ"(ptr @global, ptr @global, i32 4, i32 160, ptr null, ptr null)
; CSE-SAME:   "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr @global, [4 x i8] zeroinitializer, i32 1),
; CSE-SAME:   "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr getelementptr inbounds ([20 x i8], ptr @global, i64 0, i64 4), i32 0, i32 1),
; CSE-SAME:   "QUAL.OMP.LIVEIN"(ptr @global),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR:SUBOBJ"(ptr @global, ptr %x.addr),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR:SUBOBJ"(ptr getelementptr inbounds ([20 x i8], ptr @global, i64 0, i64 4), ptr %y.addr),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR"(ptr @global, ptr %global.addr)

; CSE:        [[X_RENAMED:%.*]] = load volatile ptr, ptr %x.addr, align 8
; CSE:        [[Y_RENAMED:%.*]] = load volatile ptr, ptr %y.addr, align 8
; CSE:        [[GLOBAL_RENAMED:%.*]] = load volatile ptr, ptr %global.addr, align 8

; CSE:        call void @bar(ptr [[X_RENAMED]], ptr [[Y_RENAMED]], ptr [[GLOBAL_RENAMED]])

; But because of the SUBOBJ modifier present on the OPERAND.ADDR and the clauses for %x and %y,
; vpo-restore-operands is able to restore the distinction of %x and %y vs @global, by breaking %x, %y
; back into Instructions.

; RESTR:      [[X_INST:%.+]] = bitcast ptr @global to ptr
; RESTR:      [[Y_INST:%.+]] = getelementptr inbounds [20 x i8], ptr @global, i64 0, i64 4

; RESTR:      "QUAL.OMP.MAP.TOFROM:SUBOBJ"(ptr [[X_INST]], ptr @global, i32 4, i32 160, ptr null, ptr null)
; RESTR-SAME: "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr [[X_INST]], [4 x i8] zeroinitializer, i32 1)
; RESTR-SAME: "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr [[Y_INST]], i32 0, i32 1)
; RESTR-SAME: "QUAL.OMP.LIVEIN"(ptr @global)

; RESTR:      call void @bar(ptr [[X_INST]], ptr [[Y_INST]], ptr @global)

; Note: for map clauses with SUBOBJ modifiers, we only need to update the first operand of the map-chain to
; be the renamed X_INST (as it will be the kernel argument). We don't care about the other fields.

; After vpo-paropt, because of the distinction of the various operands, we are able
; to handle replacement of X_INST, Y_INST and @global as different entities.

; TFORM:      define internal void @__omp_offloading_1_2_wibble_l3(ptr %{{[^ ,]+}})
; TFORM:      [[Y_PRIV:%.+]] = alloca i32, align 4
; TFORM:      [[X_PRIV:%.+]] = alloca [4 x i8], align 1

; TFORM:      call void @bar(ptr [[X_PRIV]], ptr [[Y_PRIV]], ptr @global)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-unknown-linux-gnu"

@global = common unnamed_addr global [20 x i8] zeroinitializer, align 4

declare void @bar(ptr, ptr, ptr)

define void @wibble() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %x = getelementptr inbounds [20 x i8], ptr @global, i32 0, i32 0
  %y = getelementptr inbounds [20 x i8], ptr @global, i32 0, i32 4

  %tmp1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM:SUBOBJ"(ptr %x, ptr %x, i32 4, i32 160, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr %x, [4 x i8] zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr %y, i32 0, i32 1),
    "QUAL.OMP.LIVEIN"(ptr @global) ]

  call void @bar(ptr %x, ptr %y, ptr @global)

  call void @llvm.directive.region.exit(token %tmp1) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 1, i32 2, !"wibble", i32 3, i32 0, i32 0}
