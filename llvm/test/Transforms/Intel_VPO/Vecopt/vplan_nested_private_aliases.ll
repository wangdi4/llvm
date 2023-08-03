; The test checks that if nested aliases of a private are outside the SIMD loop,
; and the user of a nested alias is inside the SIMD loop, then VPlan will
; correctly widen the private and update uses of its alias.

; RUN: opt -vplan-enable-soa=false -S -passes=vplan-vec -vplan-force-vf=4 %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() local_unnamed_addr {
; CHECK-LABEL: @foo()
; CHECK-LABEL:  simd.loop.preheader:

; CHECK-LABEL: vector.body:
; CHECK:    [[WIDE_LOAD0:%.*]] = load <4 x i32>, ptr [[TMP1:%.*]], align 4
; CHECK:    store <4 x i32> [[WIDE_LOAD0]], ptr [[TMP3:%.*]], align 4
;

entry:
  %private.src = alloca i32
  %private.dst = alloca i32
  br label %simd.begin.region


simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %private.src, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %private.dst, i32 0, i32 1), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %simd.loop.preheader

simd.loop.preheader:
  ; nested aliases
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
  %load = load i32, ptr %private.src
  store i32 %load, ptr %private.dst
  br label %simd.loop.exit

simd.loop.exit:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define dso_local void @test_alias_store_outside_loop() local_unnamed_addr {
; CHECK: @test_alias_store_outside_loop
; CHECK-NOT: {{.*}} = alloca <4 x i32>
entry:
  %x = alloca i32
  %y = alloca ptr
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %x, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %y, ptr null, i32 1)]
  br label %simd.loop.preheader

simd.loop.preheader:
  store ptr %x, ptr %y
  store ptr %x, ptr %y
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop]
  %ld = load ptr, ptr %y
  store float 7.0, ptr %x
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Test to make sure that the legality is not tripped and a store within the loop does not prevent vectorization.
define dso_local void @test_legality_safe_store(ptr %y) {
; CHECK: @test_legality_safe_store
; CHECK: {{.*}} = alloca <4 x i32>
entry:
  %x = alloca i32
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %x, i32 0, i32 1)]
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.begin.region], [ %indvar, %simd.loop]
  store ptr %x, ptr %y
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
