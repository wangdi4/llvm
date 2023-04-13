; INTEL_CUSTOMIZATION
; RUN: opt -passes="function(early-cse,vpo-cfg-restructuring),vpo-paropt" -S < %s 2>&1 | FileCheck %s

; CSE should not occur ptrtoint calls inside/outside the OMP region.

; CHECK: define dso_local void @_Z3fooiif
; CHECK:  %add = ptrtoint ptr %a.addr to i32
; CHECK: define internal void @_Z3fooiif.DIR.OMP.PARALLEL{{.*}}
; CHECK:  %aaddr = ptrtoint ptr %a.addr to i32
; CHECK:  %add2 = add i32 %aaddr, %baddr

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"@tid.addr" = external global i32

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z3fooiif(i32 noundef %a, i32 noundef %b, i32 noundef %result) #0 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %result.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  store i32 %result, ptr %result.addr, align 4
  %add = ptrtoint ptr %a.addr to i32
  store i32 %add, ptr %result.addr, align 4
  %g.addr = alloca ptr, align 8
  %a.addr.addr = alloca ptr, align 8
  %b.addr.addr = alloca ptr, align 8
  %result.addr.addr = alloca ptr, align 8
  store ptr %a.addr, ptr %a.addr.addr, align 8
  store ptr %b.addr, ptr %b.addr.addr, align 8
  store ptr %result.addr, ptr %result.addr.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %t3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED:TYPED"(ptr %a.addr, i32 0, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr %b.addr, i32 0, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr %result.addr, i32 0, i32 1), "QUAL.OMP.OPERAND.ADDR"(ptr %a.addr, ptr %a.addr.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %b.addr, ptr %b.addr.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %result.addr, ptr %result.addr.addr), "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  %aaddr = ptrtoint ptr %a.addr to i32
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %entry
  %baddr = ptrtoint ptr %b.addr to i32
  %add2 = add i32 %aaddr, %baddr
  store i32 %add2, ptr %result.addr, align 4
  br label %DIR.OMP.END.PARALLEL.4.split

DIR.OMP.END.PARALLEL.4.split:                     
  call void @llvm.directive.region.exit(token %t3) [ "DIR.OMP.END.PARALLEL"() ]
  %baddr2 = ptrtoint ptr %b.addr to i32
  %aaddr3 = ptrtoint ptr %a.addr to i32
  %add5 = add i32 %aaddr3, %baddr2
  store i32 %add5, ptr %result.addr, align 4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

attributes #0 = { nounwind }
; end INTEL_CUSTOMIZATION
