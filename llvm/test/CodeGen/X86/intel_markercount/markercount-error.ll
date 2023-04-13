; REQUIRES: intel_feature_markercount
; RUN: not llc < %s -mtriple=x86_64-- -override-marker-count-file=./no-such-file -o /dev/null 2>&1 | FileCheck %s --check-prefix=FILE
; RUN: echo '[(]' > %t.json
; RUN: not llc < %s -mtriple=x86_64-- -override-marker-count-file=%t.json -o /dev/null 2>&1 | FileCheck %s --check-prefix=JSON
; RUN: echo '{"name":"f","function":"never"}' > %t.json
; RUN: not llc < %s -mtriple=x86_64-- -override-marker-count-file=%t.json -o /dev/null 2>&1 | FileCheck %s --check-prefix=ARRAY
; RUN: echo '["f", "be"]' > %t.json
; RUN: not llc < %s -mtriple=x86_64-- -override-marker-count-file=%t.json -o /dev/null 2>&1 | FileCheck %s --check-prefix=FORMAT
; RUN: echo '[{"name":"f"}]' > %t.json
; RUN: not llc < %s -mtriple=x86_64-- -override-marker-count-file=%t.json -o /dev/null 2>&1 | FileCheck %s --check-prefix=FORMAT
; RUN: echo '[{"name":"f","function":"mbe"}]' > %t.json
; RUN: not llc < %s -mtriple=x86_64-- -override-marker-count-file=%t.json -o /dev/null 2>&1 | FileCheck %s --check-prefix=FORMAT
; RUN: echo '[{"name":"f","loop":"mbe"}]' > %t.json
; RUN: not llc < %s -mtriple=x86_64-- -override-marker-count-file=%t.json -o /dev/null 2>&1 | FileCheck %s --check-prefix=FORMAT
; RUN: echo '[{"name":"f","function":"be"}, {"name":"f","loop":"be"}]' > %t.json
; RUN: not llc < %s -mtriple=x86_64-- -override-marker-count-file=%t.json -o /dev/null 2>&1 | FileCheck %s --check-prefix=DUPLICATE

; FILE: Error opening marker count file: ./no-such-file
; JSON: Override marker count file is not a valid JSON: {{.*.json}}
; ARRAY: Expected a top-level array: {{.*.json}}
; FORMAT: Expected an array of {name:<function_name>, function:<never|me|be>, loop:<never|me|be>, comment:<comment>}. name must exits, at least one of function and loop exist and comment is optional: {{.*.json}}
; DUPLICATE: Duplicate object for function f: {{.*.json}}

define i32 @f() {
entry:
  ret i32 0
}
