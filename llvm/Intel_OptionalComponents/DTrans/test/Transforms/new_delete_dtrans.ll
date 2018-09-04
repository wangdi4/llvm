; RUN: sed -e s/^.new64:// %s | \
; RUN:      opt -whole-program-assume -dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-new64 %s
; RUN: sed -e s/^.newa64:// %s | \
; RUN:      opt -whole-program-assume -dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-newa64 %s
; RUN: sed -e s/^.new64nt:// %s | \
; RUN:      opt -whole-program-assume -dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-new64nt %s
; RUN: sed -e s/^.new64nt:// %s | \
; RUN:      opt -whole-program-assume -dtrans-deletefield -S -dtrans-print-types -o - | FileCheck --check-prefix=CHECK-types64nt %s
; RUN: sed -e s/^.newa64nt:// %s | \
; RUN:      opt -whole-program-assume -dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-newa64nt %s
; RUN: sed -e s/^.new64al:// %s | \
; RUN:      opt -whole-program-assume -dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-new64al %s
; RUN: sed -e s/^.newa64al:// %s | \
; RUN:      opt -whole-program-assume -dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-newa64al %s
; RUN: sed -e s/^.new64alnt:// %s | \
; RUN:      opt -whole-program-assume -dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-new64alnt %s
; RUN: sed -e s/^.newa64alnt:// %s | \
; RUN:      opt -whole-program-assume -dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-newa64alnt %s
; RUN: sed -e s/^.newa64alnt:// %s | \
; RUN:      opt -whole-program-assume -dtrans-deletefield -S -dtrans-print-types -o - | FileCheck --check-prefix=CHECK-types64alnt %s

; RUN: sed -e s/^.new64:// %s | \
; RUN:      opt -whole-program-assume -passes=dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-new64 %s
; RUN: sed -e s/^.newa64:// %s | \
; RUN:      opt -whole-program-assume -passes=dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-newa64 %s
; RUN: sed -e s/^.new64nt:// %s | \
; RUN:      opt -whole-program-assume -passes=dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-new64nt %s
; RUN: sed -e s/^.new64nt:// %s | \
; RUN:      opt -whole-program-assume -passes=dtrans-deletefield -dtrans-print-types -S -o - | FileCheck --check-prefix=CHECK-types64nt %s
; RUN: sed -e s/^.newa64nt:// %s | \
; RUN:      opt -whole-program-assume -passes=dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-newa64nt %s
; RUN: sed -e s/^.new64al:// %s | \
; RUN:      opt -whole-program-assume -passes=dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-new64al %s
; RUN: sed -e s/^.newa64al:// %s | \
; RUN:      opt -whole-program-assume -passes=dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-newa64al %s
; RUN: sed -e s/^.new64alnt:// %s | \
; RUN:      opt -whole-program-assume -passes=dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-new64alnt %s
; RUN: sed -e s/^.newa64alnt:// %s | \
; RUN:      opt -whole-program-assume -passes=dtrans-deletefield -S -o - | FileCheck --check-prefix=CHECK-newa64alnt %s
; RUN: sed -e s/^.newa64alnt:// %s | \
; RUN:      opt -whole-program-assume -passes=dtrans-deletefield -dtrans-print-types -S -o - | FileCheck --check-prefix=CHECK-types64alnt %s

; This test verifies that the dtrans delete pass correctly transforms new/delete routines.


; Run through c++filt to check
; new64       -- _Znwm / _ZdlPv // new(unsigned long) / delete(void*)
; newa64      -- _Znam / _ZdaPv // new[](unsigned long) / delete(void*)

; new64nt     -- _ZnwmRKSt9nothrow_t / _ZdlPvRKSt9nothrow_t : new(unsigned int, nothrow) / delete(void*, nothrow)
; newa64nt    -- _ZnamRKSt9nothrow_t / _ZdaPvRKSt9nothrow_t : new[](unsigned int, nothrow) / delete(void*, nothrow)

; new64al     -- _ZnwmSt11align_val_t / _ZdlPvSt11align_val_t : new(unsigned int, align_val_t) / delete(void*, align_val_t)
; newa64al    -- _ZnamSt11align_val_t / _ZdaPvSt11align_val_t : new[](unsigned int, align_val_t) / delete(void*, align_val_t)

; new32alnt   -- _ZnwmSt11align_val_tRKSt9nothrow_t / _ZdaPvSt11align_val_tRKSt9nothrow_t : new(unsigned int, align_val_t, nothrow) / delete(void*, align_val_t, nothrow)
; newa32alnt  -- _ZnamSt11align_val_tRKSt9nothrow_t / _ZdlPvSt11align_val_tRKSt9nothrow_t : new[](unsigned int, align_val_t, nothrow) / delete(void*, align_val_t, nothrow)

%struct.test = type { i32, i64, i32 }

%"struct.std::nothrow_t" = type { i8 }

@al = global i64 0
@nt = global %"struct.std::nothrow_t" zeroinitializer

define i32 @doSomething(%struct.test* %p_test) {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ret i32 %valA
}

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate a structure.

;new64: %p = call i8* @_Znwm(i64 16)
;newa64: %p = call i8* @_Znam(i64 16)
;new64nt: %p = call i8* @_ZnwmRKSt9nothrow_t(i64 16, %"struct.std::nothrow_t"* @nt)
;newa64nt: %p = call i8* @_ZnamRKSt9nothrow_t(i64 16, %"struct.std::nothrow_t"* @nt)
;new64al: %a = load i64, i64* @al
;new64al: %p = call i8* @_ZnwmSt11align_val_t(i64 16, i64 %a)
;newa64al: %a = load i64, i64* @al
;newa64al: %p = call i8* @_ZnamSt11align_val_t(i64 16, i64 %a)
;new64alnt: %a = load i64, i64* @al
;new64alnt: %p = call i8* @_ZnwmSt11align_val_tRKSt9nothrow_t(i64 16, i64 %a, %"struct.std::nothrow_t"* @nt)
;newa64alnt: %a = load i64, i64* @al
;newa64alnt: %p = call i8* @_ZnamSt11align_val_tRKSt9nothrow_t(i64 16, i64 %a, %"struct.std::nothrow_t"* @nt)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
;new64: call void @_ZdlPv(i8* %p)
;newa64: call void @_ZdaPv(i8* %p)
;new64nt: call void @_ZdlPvRKSt9nothrow_t(i8* %p, %"struct.std::nothrow_t"* @nt)
;newa64nt: call void @_ZdaPvRKSt9nothrow_t(i8* %p, %"struct.std::nothrow_t"* @nt)
;new64al: %a1 = load i64, i64* @al
;new64al: call void @_ZdlPvSt11align_val_t(i8* %p, i64 %a1)
;newa64al: %a1 = load i64, i64* @al
;newa64al: call void @_ZdaPvSt11align_val_t(i8* %p, i64 %a1)
;new64alnt: %a1 = load i64, i64* @al
;new64alnt:  call void @_ZdlPvSt11align_val_tRKSt9nothrow_t(i8* %p, i64 %a1, %"struct.std::nothrow_t"* @nt)
;newa64alnt: %a1 = load i64, i64* @al
;newa64alnt:  call void @_ZdaPvSt11align_val_tRKSt9nothrow_t(i8* %p, i64 %a1, %"struct.std::nothrow_t"* @nt)
  ret i32 %val
}

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)

; CHECK-new64: %p = call i8* @_Znwm(i64 8)
; CHECK-newa64: %p = call i8* @_Znam(i64 8)
; CHECK-new64nt: %p = call i8* @_ZnwmRKSt9nothrow_t(i64 8, %"struct.std::nothrow_t"* @nt)
; CHECK-newa64nt: %p = call i8* @_ZnamRKSt9nothrow_t(i64 8, %"struct.std::nothrow_t"* @nt)
; CHECK-new64al: %p = call i8* @_ZnwmSt11align_val_t(i64 8, i64 %a)
; CHECK-newa64al: %p = call i8* @_ZnamSt11align_val_t(i64 8, i64 %a)
; CHECK-new64alnt: %p = call i8* @_ZnwmSt11align_val_tRKSt9nothrow_t(i64 8, i64 %a, %"struct.std::nothrow_t"* @nt)
; CHECK-newa64alnt: %p = call i8* @_ZnamSt11align_val_tRKSt9nothrow_t(i64 8, i64 %a, %"struct.std::nothrow_t"* @nt)

; CMPLRS-51358
; CHECK-types64nt-LABEL: LLVMType: %"struct.std::nothrow_t" = type { i8 }
; CHECK-types64nt: Safety data: {{.*}}Address taken
; CHECK-types64alnt-LABEL: LLVMType: %"struct.std::nothrow_t" = type { i8 }
; CHECK-types64alnt: Safety data: {{.*}}Address taken


declare void @_ZdlPv(i8*)
declare void @_ZdaPv(i8*)
declare void @_ZdlPvRKSt9nothrow_t(i8*, %"struct.std::nothrow_t"*)
declare void @_ZdaPvRKSt9nothrow_t(i8*, %"struct.std::nothrow_t"*)
declare void @_ZdlPvSt11align_val_t(i8*, i64)
declare void @_ZdaPvSt11align_val_t(i8*, i64)
declare void @_ZdlPvSt11align_val_tRKSt9nothrow_t(i8*, i64, %"struct.std::nothrow_t"*)
declare void @_ZdaPvSt11align_val_tRKSt9nothrow_t(i8*, i64, %"struct.std::nothrow_t"*)

declare noalias i8* @_Znwm(i64)
declare noalias i8* @_Znam(i64)
declare noalias i8* @_ZnwmRKSt9nothrow_t(i64, %"struct.std::nothrow_t"*)
declare noalias i8* @_ZnamRKSt9nothrow_t(i64, %"struct.std::nothrow_t"*)
declare noalias i8* @_ZnwmSt11align_val_t(i64, i64)
declare noalias i8* @_ZnamSt11align_val_t(i64, i64)
declare noalias i8* @_ZnwmSt11align_val_tRKSt9nothrow_t(i64, i64, %"struct.std::nothrow_t"*)
declare noalias i8* @_ZnamSt11align_val_tRKSt9nothrow_t(i64, i64, %"struct.std::nothrow_t"*)

