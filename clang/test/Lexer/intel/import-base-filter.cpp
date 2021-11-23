//REQUIRES: system-windows

// Test that both forward and back slash base-path headers are filtered.
#import <filenotdfound.dll> implementation_only

//RUN: not %clang_cc1 -fintel-ms-compatibility \
//RUN:  -header-base-path "d:\\foo\\bar\\" \
//RUN:  -internal-isystem "d:\\foo\\bar\\something/include" \
//RUN:  -internal-isystem "d:/ok/keepit" -Ikeepme \
//RUN:  -internal-isystem "d:/foo/bar/another/include" \
//RUN:  -I"d:/foo/bar/yetanother/include" \
//RUN:  -show-import-processing %s > %t-base.txt 2>&1
//RUN: FileCheck --check-prefix BASE --input-file=%t-base.txt %s

//BASE: warning: argument file generated: {{.*}}filenotdfound{{.*}}arg
//BASE-NOT: {{foo|bar}}

//RUN: not %clang_cc1 -fintel-ms-compatibility \
//RUN:  -internal-isystem "d:\\foo\\bar\\something/include" \
//RUN:  -internal-isystem "d:/ok/keepit" -Ikeepme \
//RUN:  -internal-isystem "d:/foo/bar/another/include" \
//RUN:  -I"d:/foo/bar/yetanother/include" \
//RUN:  -show-import-processing %s > %t-base.txt 2>&1
//RUN: FileCheck --check-prefix NOBASE --input-file=%t-base.txt %s

//NOBASE: warning: argument file generated: {{.*}}filenotdfound{{.*}}arg
//NOBASE: added to argument file: {{.*}}keepme
//NOBASE: added to argument file: {{.*}}yetanother
//NOBASE: added to argument file: {{.*}}something
//NOBASE: added to argument file: {{.*}}keepit
//NOBASE: added to argument file: {{.*}}another
