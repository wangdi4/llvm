<<<<<<< HEAD
; RUN: opt < %s -globals-aa -function-attrs | \
; RUN:   opt -S -strip -strip-dead-prototypes -strip-named-metadata > %t.no_dbg

; RUN: opt < %s -debugify-each -globals-aa -function-attrs | \
; RUN:   opt -S -strip -strip-dead-prototypes -strip-named-metadata > %t.with_dbg
=======
; RUN: opt -temporarily-allow-old-pass-syntax < %s -passes='require<globals-aa>,function-attrs' | \
; RUN:   opt -temporarily-allow-old-pass-syntax -S -strip -strip-dead-prototypes -strip-named-metadata > %t.no_dbg

; RUN: opt -temporarily-allow-old-pass-syntax < %s -debugify-each -passes='require<globals-aa>,function-attrs' | \
; RUN:   opt -temporarily-allow-old-pass-syntax -S -strip -strip-dead-prototypes -strip-named-metadata > %t.with_dbg
>>>>>>> de787f5994022a5cf24c286963975a4a6542a779

; RUN: diff %t.no_dbg %t.with_dbg

define i32 @f_1(i32 %x) {
  %tmp = call i32 @f_1(i32 0) [ "deopt"() ]
  ret i32 0
}
