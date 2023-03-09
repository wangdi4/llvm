; REQUIRES: x86-registered-target

<<<<<<< HEAD
; INTEL_CUSTOMIZATION
; RUN: opt -temporarily-allow-old-pass-syntax /dev/null -disable-output 2>&1 | FileCheck %s --check-prefix=OK --allow-empty
; RUN: opt -temporarily-allow-old-pass-syntax /dev/null -disable-output -passes=instcombine 2>&1 | FileCheck %s --check-prefix=OK --allow-empty
; RUN: opt -temporarily-allow-old-pass-syntax /dev/null -disable-output -instcombine 2>&1 | FileCheck %s --check-prefix=WARN
; RUN: opt -temporarily-allow-old-pass-syntax /dev/null -disable-output -instcombine -globaldce 2>&1 | FileCheck %s --check-prefix=WARN
; RUN: opt -temporarily-allow-old-pass-syntax /dev/null -disable-output -instcombine -enable-new-pm=0 2>&1 | FileCheck %s --check-prefix=OK --allow-empty
; RUN: opt -temporarily-allow-old-pass-syntax /dev/null -disable-output -codegenprepare -mtriple=x86_64-unknown-linux-gnu 2>&1 | FileCheck %s --check-prefix=OK --allow-empty
; end INTEL_CUSTOMIZATION
=======
; RUN: opt /dev/null -disable-output 2>&1 | FileCheck %s --check-prefix=OK --allow-empty
; RUN: opt /dev/null -disable-output -passes=instcombine 2>&1 | FileCheck %s --check-prefix=OK --allow-empty
; RUN: not opt /dev/null -disable-output -instcombine 2>&1 | FileCheck %s --check-prefix=WARN
; RUN: not opt /dev/null -disable-output -instcombine -always-inline 2>&1 | FileCheck %s --check-prefix=WARN
; RUN: opt /dev/null -disable-output -instcombine -enable-new-pm=0 2>&1 | FileCheck %s --check-prefix=OK --allow-empty
; RUN: opt /dev/null -disable-output -codegenprepare -mtriple=x86_64-unknown-linux-gnu 2>&1 | FileCheck %s --check-prefix=OK --allow-empty
>>>>>>> f57ff7346bc6e08317308c15118e4c0372a98658

; OK-NOT: deprecated

; WARN: The `opt -passname` syntax for the new pass manager is deprecated, please use `opt -passes=<pipeline>` (or the `-p` alias for a more concise version). ;INTEL
