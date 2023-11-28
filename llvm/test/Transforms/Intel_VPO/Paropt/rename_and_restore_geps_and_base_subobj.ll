; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -early-cse -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=CSE
; RUN: opt -bugpoint-enable-legacy-pm -vpo-restore-operands -S %t2.ll -o %t3.ll && FileCheck --input-file=%t3.ll %s -check-prefix=RESTR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %t3.ll | FileCheck %s -check-prefix=TFORM

; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare)" -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -passes="function(early-cse)" -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=CSE
; RUN: opt -passes="function(vpo-restore-operands)" -S %t2.ll -o %t3.ll && FileCheck --input-file=%t3.ll %s -check-prefix=RESTR
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %t3.ll | FileCheck %s -check-prefix=TFORM

; This is a version of rename_and_restore_geps_and_base.ll, which adds
; SUBOBJ modifiers to the clauses on GEPs, to ensure that their individuality
; vs the base global is retained after renaming/restoring.

; %x, %y and @global get renamed in the prepare pass.

; PREPR:      store ptr %x, ptr %x.addr, align 8
; PREPR:      store ptr %y, ptr %y.addr, align 8
; PREPR:      store ptr @global, ptr %global.addr, align 8

; PREPR:      "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr %x, [4 x i8] zeroinitializer, i32 1),
; PREPR-SAME: "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr %y, i32 0, i32 1),
; PREPR-SAME: "QUAL.OMP.SHARED"(ptr @global),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR:SUBOBJ"(ptr %x, ptr %x.addr),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR:SUBOBJ"(ptr %y, ptr %y.addr),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr @global, ptr %global.addr)

; PREPR:      [[X_RENAMED:%.*]] = load volatile ptr, ptr %x.addr, align 8
; PREPR:      [[Y_RENAMED:%.*]] = load volatile ptr, ptr %y.addr, align 8
; PREPR:      [[GLOBAL_RENAMED:%.*]] = load volatile ptr, ptr %global.addr, align 8

; PREPR:      call void @bar(ptr [[X_RENAMED]], ptr [[Y_RENAMED]], ptr [[GLOBAL_RENAMED]])


; But early-cse replaces %x with @global and %y with a const-expr GEP on @global.
; This means %x and @global are not distinguishable.

; CSE:        "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr @global, [4 x i8] zeroinitializer, i32 1),
; CSE-SAME:   "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr getelementptr inbounds ([20 x i8], ptr @global, i64 0, i64 4), i32 0, i32 1),
; CSE-SAME:   "QUAL.OMP.SHARED"(ptr @global),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR:SUBOBJ"(ptr @global, ptr %x.addr),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR:SUBOBJ"(ptr getelementptr inbounds ([20 x i8], ptr @global, i64 0, i64 4), ptr %y.addr),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR"(ptr @global, ptr %global.addr)

; CSE:        [[X_RENAMED:%.*]] = load volatile ptr, ptr %x.addr, align 8
; CSE:        [[Y_RENAMED:%.*]] = load volatile ptr, ptr %y.addr, align 8
; CSE:        [[GLOBAL_RENAMED:%.*]] = load volatile ptr, ptr %global.addr, align 8

; CSE:        call void @bar(ptr [[X_RENAMED]], ptr [[Y_RENAMED]], ptr [[GLOBAL_RENAMED]])

; But because of the SUBOBJ modifier present on the OPERAND.ADDR and the PRIVATE clauses for %x and %y,
; vpo-restore-operands is able to restore the distinction of %x and %y vs @global, by breaking %x, %y
; back into Instructions.

; RESTR:      [[X_INST:%.+]] = bitcast ptr @global to ptr
; RESTR:      [[Y_INST:%.+]] = getelementptr inbounds [20 x i8], ptr @global, i64 0, i64 4
;
; RESTR:      "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr [[X_INST]], [4 x i8] zeroinitializer, i32 1)
; RESTR-SAME: "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr [[Y_INST]], i32 0, i32 1)
; RESTR-SAME: "QUAL.OMP.SHARED"(ptr @global)

; RESTR:      call void @bar(ptr [[X_INST]], ptr [[Y_INST]], ptr @global)


; This means that the privatization/replacement %x, %y and @global inside the region happens correctly.

; TFORM:      define internal void @wibble.DIR.OMP.PARALLEL{{.*}}(ptr %tid, ptr %bid)
; TFORM:      [[Y_PRIV:%.+]] = alloca i32, align 4
; TFORM:      [[X_PRIV:%.+]] = alloca [4 x i8], align 1

; TFORM:      call void @bar(ptr [[X_PRIV]], ptr [[Y_PRIV]], ptr @global)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global = common unnamed_addr global [20 x i8] zeroinitializer, align 4

declare void @bar(ptr, ptr, ptr)

define void @wibble() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %x = getelementptr inbounds [20 x i8], ptr @global, i32 0, i32 0
  %y = getelementptr inbounds [20 x i8], ptr @global, i32 0, i32 4

  %tmp1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr %x, [4 x i8] zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr %y, i32 0, i32 1),
    "QUAL.OMP.SHARED"(ptr @global) ]

  call void @bar(ptr %x, ptr %y, ptr @global)

  call void @llvm.directive.region.exit(token %tmp1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
