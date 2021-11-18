//REQUIRES: system-windows

#import <filenotdfound.dll> implementation_only

//RUN: not %clang_cc1 -fintel-ms-compatibility -Imystuff -show-import-processing %s > %t-check.txt 2>&1
//RUN: FileCheck --input-file=%t-check.txt %s
//CHECK: warning: wrapper file generated: {{.*}}import{{.*}}cpp [-Wintel-compat]
//CHECK: note: added to wrapper file: #import <filenotdfound.dll> implementation_only
//CHECK: warning: argument file generated: {{.*}}filenotdfound{{.*}}arg [-Wintel-compat]
//CHECK: note: added to argument file: /I
//CHECK-NEXT: note: added to argument file: /I"mystuff"
//CHECK-NEXT: note: added to argument file: /I
//CHECK-NEXT: note: added to argument file: {{.*}}import{{.*}}cpp
//CHECK: error: could not process #import directive with

//RUN: not %clang_cc1 -fintel-ms-compatibility -internal-isystem "d:\foo\bar\something/include" -show-import-processing %s > %t-nobase.txt 2>&1
//RUN: FileCheck --check-prefix NOBASE --input-file=%t-nobase.txt %s
//NOBASE: warning: wrapper file generated: {{.*}}import{{.*}}cpp [-Wintel-compat]
//NOBASE: note: added to wrapper file: #import <filenotdfound.dll> implementation_only
//NOBASE: warning: argument file generated: {{.*}}filenotdfound{{.*}}arg [-Wintel-compat]
//NOBASE: note: added to argument file: /I
//NOBASE-NEXT: note: added to argument file: /I
//NOBASE-NEXT: note: added to argument file: /I"d:\foo\bar\something/include"
//NOBASE-NEXT: note: added to argument file: {{.*}}import{{.*}}cpp
//NOBASE: error: could not process #import directive with

//RUN: not %clang_cc1 -fintel-ms-compatibility -header-base-path "d:\\foo\\bar\\" -internal-isystem "d:\\foo\\bar\\something/include" -show-import-processing %s > %t-base.txt 2>&1
//RUN: FileCheck --check-prefix BASE --input-file=%t-base.txt %s
//BASE: warning: wrapper file generated: {{.*}}import{{.*}}cpp [-Wintel-compat]
//BASE: note: added to wrapper file: #import <filenotdfound.dll> implementation_only
//BASE: warning: argument file generated: {{.*}}filenotdfound{{.*}}arg [-Wintel-compat]
//BASE: note: added to argument file: /I
//BASE-NEXT: note: added to argument file: /I
//BASE-NEXT: note: added to argument file: {{.*}}import{{.*}}cpp
//BASE: error: could not process #import directive with
