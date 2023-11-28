; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -early-cse -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=CSE
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-restore-operands -S %t2.ll -o %t3.ll && FileCheck --input-file=%t3.ll %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %t3.ll | FileCheck %s -check-prefix=TFORM

; RUN: opt -opaque-pointers=0 -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare)" -S -o %t1.ll <%s &&  FileCheck --input-file=%t1.ll %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -passes="function(early-cse)" -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s -check-prefix=CSE
; RUN: opt -opaque-pointers=0 -passes="function(vpo-restore-operands)" -S %t2.ll -o %t3.ll && FileCheck --input-file=%t3.ll %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %t3.ll | FileCheck %s -check-prefix=TFORM

; This test is to show how renaming/restoring of operands that are GEP
; Instructions (%x, %y) into a global array (@global), works when the base
; global is also a clause operand on the same directive.


; %x, %y and @global get renamed in the prepare pass.

; PREPR:      store [4 x i8]* %x, [4 x i8]** %x.addr, align 8
; PREPR:      store i32* %y, i32** %y.addr, align 8
; PREPR:      store [20 x i8]* @global, [20 x i8]** %global.addr, align 8

; PREPR:      "QUAL.OMP.PRIVATE:TYPED"([4 x i8]* %x, [4 x i8] zeroinitializer, i32 1),
; PREPR-SAME: "QUAL.OMP.PRIVATE:TYPED"(i32* %y, i32 0, i32 1),
; PREPR-SAME: "QUAL.OMP.SHARED"([20 x i8]* @global),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"([4 x i8]* %x, [4 x i8]** %x.addr),
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(i32* %y, i32** %y.addr),
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

; CSE:        "QUAL.OMP.PRIVATE:TYPED"([4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*), [4 x i8] zeroinitializer, i32 1),
; CSE-SAME:   "QUAL.OMP.PRIVATE:TYPED"(i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*), i32 0, i32 1),
; CSE-SAME:   "QUAL.OMP.SHARED"([20 x i8]* @global),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR"([4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*), [4 x i8]** %x.addr),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR"(i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*), i32** %y.addr),
; CSE-SAME:   "QUAL.OMP.OPERAND.ADDR"([20 x i8]* @global, [20 x i8]** %global.addr)

; CSE:        [[X_RENAMED:%.*]] = load volatile [4 x i8]*, [4 x i8]** %x.addr, align 8
; CSE:        [[Y_RENAMED:%.*]] = load volatile i32*, i32** %y.addr, align 8
; CSE:        [[GLOBAL_RENAMED:%.*]] = load volatile [20 x i8]*, [20 x i8]** %global.addr, align 8

; CSE:        %arg1 = bitcast [4 x i8]* [[X_RENAMED]] to i8*
; CSE:        %arg2 = bitcast i32* [[Y_RENAMED]] to i8*
; CSE:        %arg3 = bitcast [20 x i8]* [[GLOBAL_RENAMED]] to i8*
; CSE:        call void @bar(i8* %arg1, i8* %arg2, i8* %arg3)


; After vpo-restore-operands, the constant-expression representing %x and the
; base @global are still distinguishable in this case.

; RESTR:      "QUAL.OMP.PRIVATE:TYPED"([4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*), [4 x i8] zeroinitializer, i32 1),
; RESTR-SAME: "QUAL.OMP.PRIVATE:TYPED"(i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*), i32 0, i32 1),
; RESTR-SAME: "QUAL.OMP.SHARED"([20 x i8]* @global)

; RESTR:      %arg1 = bitcast [4 x i8]* bitcast ([20 x i8]* @global to [4 x i8]*) to i8*
; RESTR:      %arg2 = bitcast i32* bitcast (i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*) to i8*
; RESTR:      %arg3 = bitcast [20 x i8]* @global to i8*
; RESTR:      call void @bar(i8* %arg1, i8* %arg2, i8* %arg3)


; After vpo-paropt, because of the distinction of the various pointer types, we are able
; to handle replacement of the constexpr representing %x, constexpr representing %y
; and @global as different entities.

; TFORM:      define internal void @wibble.DIR.OMP.PARALLEL{{.*}}(i32* %tid, i32* %bid)
; TFORM:      [[Y_PRIV:%.*]] = alloca i32, align 4
; TFORM:      [[X_PRIV:%.*]] = alloca [4 x i8], align 1

; TFORM:      %arg1 = bitcast [4 x i8]* [[X_PRIV]] to i8*
; TFORM:      %arg2 = bitcast i32* [[Y_PRIV]] to i8*
; TFORM:      %arg3 = bitcast [20 x i8]* @global to i8*
; TFORM:      call void @bar(i8* %arg1, i8* %arg2, i8* %arg3)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global = common unnamed_addr global [20 x i8] zeroinitializer, align 4

declare void @bar(i8*, i8*, i8*)

define void @wibble() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %x = bitcast i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 0) to [4 x i8]*
  %y = bitcast i8* getelementptr inbounds ([20 x i8], [20 x i8]* @global, i32 0, i32 4) to i32*

  %tmp1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"([4 x i8]* %x, [4 x i8] zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %y, i32 0, i32 1),
    "QUAL.OMP.SHARED"([20 x i8]* @global) ]

  %arg1 = bitcast [4 x i8]* %x to i8*
  %arg2 = bitcast i32* %y to i8*
  %arg3 = bitcast [20 x i8]* @global to i8*
  call void @bar(i8* %arg1, i8* %arg2, i8* %arg3)

  call void @llvm.directive.region.exit(token %tmp1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
