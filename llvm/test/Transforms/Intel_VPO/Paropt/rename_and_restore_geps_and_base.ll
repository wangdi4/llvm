; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -early-cse -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=CSE
; RUN: opt -bugpoint-enable-legacy-pm -vpo-restore-operands -S %t2.ll -o %t3.ll && FileCheck --input-file=%t3.ll %s -check-prefix=RESTR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %t3.ll | FileCheck %s -check-prefix=TFORM

; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare)" -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -passes="function(early-cse)" -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=CSE
; RUN: opt -passes="function(vpo-restore-operands)" -S %t2.ll -o %t3.ll && FileCheck --input-file=%t3.ll %s -check-prefix=RESTR
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %t3.ll | FileCheck %s -check-prefix=TFORM

; This test is to show how renaming/restoring of operands that are GEP
; Instructions (%x, %y) into a global array (@global), works when the base
; global is also a clause operand on the same directive.


; %x, %y and @global get renamed in the prepare pass.

; PREPR:      store ptr %y, ptr %y.addr, align 8
; PREPR:      store ptr %x, ptr %x.addr, align 8
; PREPR:      store ptr @global, ptr %global.addr, align 8

; PREPR:      "QUAL.OMP.PRIVATE:TYPED"(ptr %y, i32 0, i32 1),
; PREPR-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %x, [4 x i8] zeroinitializer, i32 1),
; PREPR-SAME: "QUAL.OMP.SHARED"(ptr @global),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %y, ptr %y.addr),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %x, ptr %x.addr),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr @global, ptr %global.addr)

; PREPR:      [[Y_RENAMED:%y.*]] = load volatile ptr, ptr %y.addr, align 8
; PREPR:      [[X_RENAMED:%x.*]] = load volatile ptr, ptr %x.addr, align 8
; PREPR:      [[GLOBAL_RENAMED:%global.*]] = load volatile ptr, ptr %global.addr, align 8

; PREPR:      call void @bar(ptr [[X_RENAMED]], ptr [[Y_RENAMED]], ptr [[GLOBAL_RENAMED]])


; But early-cse replaces %x with @global and %y with a const-expr GEP on @global.
; This means %x and @global are not distinguishable.

; CSE:        "QUAL.OMP.PRIVATE:TYPED"(ptr getelementptr inbounds ([20 x i8], ptr @global, i64 0, i64 4), i32 0, i32 1),
; CSE-SAME:   "QUAL.OMP.PRIVATE:TYPED"(ptr @global, [4 x i8] zeroinitializer, i32 1),
; CSE-SAME:   "QUAL.OMP.SHARED"(ptr @global),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR"(ptr getelementptr inbounds ([20 x i8], ptr @global, i64 0, i64 4), ptr %y.addr),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR"(ptr @global, ptr %x.addr),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR"(ptr @global, ptr %global.addr)

; CSE:        [[Y_RENAMED:%y.*]] = load volatile ptr, ptr %y.addr, align 8
; CSE:        [[X_RENAMED:%x.*]] = load volatile ptr, ptr %x.addr, align 8
; CSE:        [[GLOBAL_RENAMED:%global.*]] = load volatile ptr, ptr %global.addr, align 8

; CSE:        call void @bar(ptr [[X_RENAMED]], ptr [[Y_RENAMED]], ptr [[GLOBAL_RENAMED]])


; This causes on %x and @global to be indistinguishable after vpo-restore-operands.

; RESTR:      "QUAL.OMP.PRIVATE:TYPED"(ptr getelementptr inbounds ([20 x i8], ptr @global, i64 0, i64 4), i32 0, i32 1),
; RESTR-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr @global, [4 x i8] zeroinitializer, i32 1),
; RESTR-SAME: "QUAL.OMP.SHARED"(ptr @global)

; RESTR:      call void @bar(ptr @global, ptr getelementptr inbounds ([20 x i8], ptr @global, i64 0, i64 4), ptr @global)

; This would cause the privatization of x to replace all uses of @global inside the
; region. Privatization of %y happens before %x because of the ordering of the clauses.
;
; TFORM:      define internal void @wibble.DIR.OMP.PARALLEL{{.*}}(ptr %tid, ptr %bid)
; TFORM:      [[X_PRIV:%.+]] = alloca [4 x i8], align 4
; TFORM:      [[Y_PRIV:%.+]] = alloca i32, align 4
; TFORM:      call void @bar(ptr [[X_PRIV]], ptr [[Y_PRIV]], ptr [[X_PRIV]])

; Note:
; Had the private clause on %x been before %y, it would be a worse problem. It would
; cause a crash in TFORM pass, as privatization of @global as part of the first clause
; would replace it in the GEP expression for the second clause, before Paropt did
; privatization for it:
;
;   Before:
;    "PRIVATE"(@global), "PRIVATE"(gep(@global, 0, 4), "SHARED"(@global)
;
;   After privatization of first operand:
;    %gep.4 = gep(@global.priv, 0, 4)
;    "PRIVATE"(%global.priv), "PRIVATE"(%gep.4), "SHARED"(%global.priv)
;
; making the original operand of the second private clause, i.e. "gep(@global, 0, 4)" a
; dangling "Value*" pointer, since the original GEP no longer exists in the IR, while
; the WRegion still holds the Value* for that GEP.

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
    "QUAL.OMP.PRIVATE:TYPED"(ptr %y, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x, [4 x i8] zeroinitializer, i32 1),
    "QUAL.OMP.SHARED"(ptr @global) ]

  call void @bar(ptr %x, ptr %y, ptr @global)

  call void @llvm.directive.region.exit(token %tmp1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
