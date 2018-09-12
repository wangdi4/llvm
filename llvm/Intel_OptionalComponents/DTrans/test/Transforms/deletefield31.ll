; Test check that delete fields optimization
; does NOT happen with 4 unknown function calls
; DOES happen when unknown funcitons are classified
; as free/malloc using internal switches.
;
; RUN: opt -S < %s \
; RUN:     -dtrans-malloc-functions=struct.A,0 \
; RUN:     -dtrans-malloc-functions=_ZN4testnwEm \
; RUN:     -dtrans-free-functions=struct.A,1 \
; RUN:     -dtrans-free-functions=_ZN4testdlEPv \
; RUN:     -whole-program-assume -dtrans-identify-unused-values=false \
; RUN:     -dtrans-deletefield | FileCheck --check-prefix=CHECK-REPLACED %s

; RUN: opt -S < %s \
; RUN:     -dtrans-malloc-functions=_ZN4testnwEm \
; RUN:     -dtrans-free-functions=struct.A,1 \
; RUN:     -dtrans-free-functions=_ZN4testdlEPv \
; RUN:     -whole-program-assume -dtrans-identify-unused-values=false \
; RUN:     -dtrans-deletefield | FileCheck %s

; RUN: opt -S < %s \
; RUN:     -dtrans-malloc-functions=struct.A,0 \
; RUN:     -dtrans-free-functions=struct.A,1 \
; RUN:     -dtrans-free-functions=_ZN4testdlEPv \
; RUN:     -whole-program-assume -dtrans-identify-unused-values=false \
; RUN:     -dtrans-deletefield | FileCheck %s

; RUN: opt -S < %s \
; RUN:     -dtrans-malloc-functions=struct.A,0 \
; RUN:     -dtrans-malloc-functions=_ZN4testnwEm \
; RUN:     -dtrans-free-functions=_ZN4testdlEPv \
; RUN:     -whole-program-assume -dtrans-identify-unused-values=false \
; RUN:     -dtrans-deletefield | FileCheck %s

; RUN: opt -S < %s \
; RUN:     -dtrans-malloc-functions=struct.A,0 \
; RUN:     -dtrans-malloc-functions=_ZN4testnwEm \
; RUN:     -dtrans-free-functions=struct.A,1 \
; RUN:     -whole-program-assume -dtrans-identify-unused-values=false \
; RUN:     -dtrans-deletefield | FileCheck %s

; RUN: opt -S < %s \
; RUN:     -dtrans-malloc-functions=struct.A,0 \
; RUN:     -dtrans-malloc-functions=_ZN4testnwEm \
; RUN:     -dtrans-free-functions=struct.A,1 \
; RUN:     -dtrans-free-functions=_ZN4testdlEPv \
; RUN:     -whole-program-assume -dtrans-identify-unused-values=false \
; RUN:     -passes=dtrans-deletefield | FileCheck --check-prefix=CHECK-REPLACED %s

; RUN: opt -S < %s \
; RUN:     -dtrans-malloc-functions=_ZN4testnwEm \
; RUN:     -dtrans-free-functions=struct.A,1 \
; RUN:     -dtrans-free-functions=_ZN4testdlEPv \
; RUN:     -whole-program-assume -dtrans-identify-unused-values=false \
; RUN:     -passes=dtrans-deletefield | FileCheck %s

; RUN: opt -S < %s \
; RUN:     -dtrans-malloc-functions=struct.A,0 \
; RUN:     -dtrans-free-functions=struct.A,1 \
; RUN:     -dtrans-free-functions=_ZN4testdlEPv \
; RUN:     -whole-program-assume -dtrans-identify-unused-values=false \
; RUN:     -passes=dtrans-deletefield | FileCheck %s

; RUN: opt -S < %s \
; RUN:     -dtrans-malloc-functions=struct.A,0 \
; RUN:     -dtrans-malloc-functions=_ZN4testnwEm \
; RUN:     -dtrans-free-functions=_ZN4testdlEPv \
; RUN:     -whole-program-assume -dtrans-identify-unused-values=false \
; RUN:     -passes=dtrans-deletefield | FileCheck %s

; RUN: opt -S < %s \
; RUN:     -dtrans-malloc-functions=struct.A,0 \
; RUN:     -dtrans-malloc-functions=_ZN4testnwEm \
; RUN:     -dtrans-free-functions=struct.A,1 \
; RUN:     -whole-program-assume -dtrans-identify-unused-values=false \
; RUN:     -passes=dtrans-deletefield | FileCheck %s

; CHECK-NOT:  %__DFT_struct.test = type {}
; CHECK: %struct.test = type { i32, i64, i32 }

; CHECK-REPLACED:  %__DFT_struct.test = type { i32 }
; CHECK-REPLACED-NOT: %struct.test = type { i32, i64, i32 }

; CHECK-REPLACED-LABEL: define internal void @_Z4foo1P1Ai(%struct.A* %m, i32 %cond)
; CHECK-REPLACED: %call = call i8* %tmp1(%struct.A* %m, i64 4)

; CHECK-REPLACED-LABEL: define internal void @_Z4foo2i(i32 %cond)
; CHECK-REPLACED:   %call = call i8* @_ZN4testnwEm(i64 4)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.exc = type { i8 }
%struct.test = type { i32, i64, i32 }
%struct.A = type { i32 (...)** }

$_ZTS3exc = comdat any

$_ZTI3exc = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS3exc = linkonce_odr constant [5 x i8] c"3exc\00", comdat
@_ZTI3exc = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @_ZTS3exc, i32 0, i32 0) }, comdat

; Function Attrs: noinline uwtable
define void @_Z8tryThrowi(i32 %cond) #0 {
entry:
  %cmp = icmp eq i32 %cond, 2
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %exception = call i8* @__cxa_allocate_exception(i64 1) #2
  %tmp = bitcast i8* %exception to %struct.exc*
  call void @__cxa_throw(i8* %exception, i8* bitcast ({ i8*, i8* }* @_ZTI3exc to i8*), i8* null) #3
  unreachable

if.end:                                           ; preds = %entry
  ret void
}

declare i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

@a = external global i32

define void @_Z4foo1P1Ai(%struct.A* %m, i32 %cond) #0 {
entry:
  %tmp = bitcast %struct.A* %m to i8* (%struct.A*, i64)***
  %vtable = load i8* (%struct.A*, i64)**, i8* (%struct.A*, i64)*** %tmp, align 8
  %vfn = getelementptr inbounds i8* (%struct.A*, i64)*, i8* (%struct.A*, i64)** %vtable, i64 0
  %tmp1 = load i8* (%struct.A*, i64)*, i8* (%struct.A*, i64)** %vfn, align 8
  %call = call i8* %tmp1(%struct.A* %m, i64 24)
  %tmp2 = bitcast i8* %call to %struct.test*
  %tmp3 = bitcast %struct.test* %tmp2 to i8*
  %tmp4 = bitcast %struct.A* %m to void (%struct.A*, i8*)***
  %addr = getelementptr inbounds %struct.test, %struct.test* %tmp2, i32 0, i32 0
  store i32 0, i32* %addr
  %zero = load i32, i32* %addr
  store i32 %zero, i32* @a
  %vtable2 = load void (%struct.A*, i8*)**, void (%struct.A*, i8*)*** %tmp4, align 8
  %vfn3 = getelementptr inbounds void (%struct.A*, i8*)*, void (%struct.A*, i8*)** %vtable2, i64 1
  %tmp5 = load void (%struct.A*, i8*)*, void (%struct.A*, i8*)** %vfn3, align 8
  call void %tmp5(%struct.A* %m, i8* %tmp3)
  ret void
}

define void @_Z4foo2i(i32 %cond) #0 {
entry:
  %call = call i8* @_ZN4testnwEm(i64 24)
  %tmp = bitcast i8* %call to %struct.test*
  %tmp1 = bitcast %struct.test* %tmp to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %tmp1, i8 0, i64 24, i1 false)
  %isnull = icmp eq %struct.test* %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  %tmp2 = bitcast %struct.test* %tmp to i8*
  call void @_ZN4testdlEPv(i8* %tmp2)
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %entry
  ret void
}

declare i8* @_ZN4testnwEm(i64) #1

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #1

declare void @_ZN4testdlEPv(i8*) #1

attributes #0 = { noinline uwtable }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { noreturn }
