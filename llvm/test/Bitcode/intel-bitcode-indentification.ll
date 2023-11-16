; Check that a block "IDENTIFICATION_BLOCK_ID" is emitted
;   with Intel-specific identifiers.
;RUN: llvm-as < %s | llvm-bcanalyzer -dump | FileCheck %s
;CHECK: <IDENTIFICATION_BLOCK_ID
;CHECK-NEXT: <STRING
;CHECK-SAME: record string = 'Intel.oneAPI.DPCPP.Compiler_202{{[2-9]}}.{{[0-9]}}.{{[0-9]}}'
;CHECK-NEXT: <EPOCH
;CHECK-SAME: op0=0
;CHECK-NEXT: </IDENTIFICATION_BLOCK_ID
