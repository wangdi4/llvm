; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Check that the map-type of %this1 is updated to 32 from 0, and it's
; passed to the kernel.
; 32 (PARAM) for %this1, 281474976710674 (MEMBER_OF_1 | PTR_AND_OBJ | FROM) for %y, 288 (PARAM | LITERAL) is for %x.
; CHECK: @.offload_maptypes = {{.*}} [i64 32, i64 281474976710674, i64 288]
; CHECK: call i32 @__tgt_target_teams_mapper(%struct.ident_t* {{[^,]+}}, i64 {{[^,]+}}, i8* @[[KERNEL:[^,.]+]].region_id, {{.*}})
; CHECK: call void @[[KERNEL]](%class.S* %this1, i64 %x{{.*}})

; Test src:
;
; #include <stdio.h>
;
; class S {
; public:
;   int x;
;   int *y;
;   void foo() {
; #pragma omp target teams map(from: y[0:3]) num_teams(x)
;     {}
; //    printf("%p \n", y);
;   }
; };
;
; int main() {
;   int a[10];
;   S s;
;   s.x = 8;
;   s.y = &a[0];
;   a[2] = 111;
;   s.foo();
; }
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

%class.S = type { i32, i32* }

$_ZN1S3fooEv = comdat any

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define linkonce_odr dso_local void @_ZN1S3fooEv(%class.S* nonnull dereferenceable(16) %this) #1 comdat align 2 {
entry:
  %this.addr = alloca %class.S*, align 8
  store %class.S* %this, %class.S** %this.addr, align 8
  %this1 = load %class.S*, %class.S** %this.addr, align 8
  %x = getelementptr inbounds %class.S, %class.S* %this1, i32 0, i32 0
  %y = getelementptr inbounds %class.S, %class.S* %this1, i32 0, i32 1
  %y2 = getelementptr inbounds %class.S, %class.S* %this1, i32 0, i32 1
  %0 = load i32*, i32** %y2, align 8
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 0
  %1 = getelementptr i32*, i32** %y, i32 1
  %2 = bitcast i32** %y to i8*
  %3 = bitcast i32** %1 to i8*
  %4 = ptrtoint i8* %3 to i64
  %5 = ptrtoint i8* %2 to i64
  %6 = sub i64 %4, %5
  %7 = sdiv exact i64 %6, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(%class.S* %this1, i32** %y, i64 %7, i64 0, i8* null, i8* null), "QUAL.OMP.MAP.FROM:CHAIN"(i32** %y, i32* %arrayidx, i64 12, i64 281474976710674, i8* null, i8* null), "QUAL.OMP.FIRSTPRIVATE"(i32* %x) ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32* %x) ]
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline norecurse optnone uwtable mustprogress "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable mustprogress "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 0, i32 66309, i32 47061490, !"_ZN1S3fooEv", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
