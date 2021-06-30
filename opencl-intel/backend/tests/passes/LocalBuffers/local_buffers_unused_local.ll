; RUN: %oclopt -dpcpp-kernel-add-implicit-args -debugify -local-buffers -check-debugify -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -dpcpp-kernel-add-implicit-args -local-buffers -S < %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; not mapping unused local variables to local buffer

; Used local variables
@foo.localInt = internal addrspace(3) global i32 0, align 4
@bar.localLong16 = internal addrspace(3) global <16 x i64> zeroinitializer, align 128

; Unused local variables
@foo.localChar = internal addrspace(3) global i8 0, align 1
@foo.localFloat = internal addrspace(3) global float 0.000000e+00, align 4
@bar.localInt4 = internal addrspace(3) global <4 x i32> zeroinitializer, align 16

; Used in multiple functions
@local.used.by.multiple = internal addrspace(3) global i32 0, align 4

; CHECK-LABEL: define void @foo
define void @foo(i32 addrspace(1)* %pInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat) {
entry:
; @foo.localInt in local buffer
; CHECK: [[GEP0:%.*]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT: [[BC0:%.*]] = bitcast i8 addrspace(3)* [[GEP0]] to i32 addrspace(3)*

; @local.used.by.multiple in local buffer
; CHECK: [[GEP1:%.*]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 128
; CHECK-NEXT: [[BC1:%.*]] = bitcast i8 addrspace(3)* [[GEP1]] to i32 addrspace(3)*

; CHECK: %dummyInt = load i32, i32 addrspace(3)* [[BC0]]
  %dummyInt = load i32, i32 addrspace(3)* @foo.localInt, align 4
  store i32 %dummyInt, i32 addrspace(1)* %pInt

; Normal use, will be mapped to local buffer
; CHECK: %normal.use = load i32, i32 addrspace(3)* [[BC1]]
  %normal.use = load i32, i32 addrspace(3)* @local.used.by.multiple, align 4

  ret void
}

; CHECK-LABEL: define void @bar
define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16) {
entry:
; @bar.localLong16 in local buffer
; CHECK: [[GEP2:%.*]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT: [[BC2:%.*]] = bitcast i8 addrspace(3)* [[GEP2]] to <16 x i64> addrspace(3)*

; @ local.used.by.multiple not in local buffer
; CHECK-NOT: getelementptr i8, i8 addrspace(3)* %pLocalMemBase

; CHECK: %dummyLong16 = load <16 x i64>, <16 x i64> addrspace(3)* [[BC2]]
  %dummyLong16 = load <16 x i64>, <16 x i64> addrspace(3)* @bar.localLong16, align 128
  store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16

; Used by a !dbg_declare_inst marked instruction, won't be mapped to local buffer
; CHECK: %var_addr = addrspacecast i32 addrspace(3)* @local.used.by.multiple to i8*, !dbg_declare_inst
  %var_addr = addrspacecast i32 addrspace(3)* @local.used.by.multiple to i8*, !dbg_declare_inst !0

  ret void
}

; CHECK-LABEL: define void @baz
define void @baz() {
entry:
; @local.used.by.multiple in local buffer
; CHECK: [[GEP3:%.*]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT: [[BC3:%.*]] = bitcast i8 addrspace(3)* [[GEP3]] to i32 addrspace(3)*

; A normal use and a !dbg_declare_inst use, then it will be mapped to local buffer
; CHECK: %normal.use = load i32, i32 addrspace(3)* [[BC3]]
; CHECK: %var_addr = addrspacecast i32 addrspace(3)* [[BC3]] to i8*, !dbg_declare_inst
  %normal.use = load i32, i32 addrspace(3)* @local.used.by.multiple, align 4
  %var_addr = addrspacecast i32 addrspace(3)* @local.used.by.multiple to i8*, !dbg_declare_inst !0

  ret void
}

!0 = !{i1 true}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-8: WARNING: Instruction with empty DebugLoc in function {{foo|bar|baz}} -- {{.*}} {{getelementptr|bitcast}}
; DEBUGIFY-NOT: WARNING
