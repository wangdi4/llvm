// RUN: %clang_cc1 -debug-info-kind=limited -dwarf-version=4 -femit-class-debug-always -triple x86_64-linux-gnu -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

namespace abi = __cxxabiv1; //this namespace might be known for xmain without any includes

//CHECK: {{![0-9]+}} = !DIImportedEntity(tag: DW_TAG_imported_declaration, name: "abi", {{.+}}entity: [[N:![0-9]+]]
//CHECK: [[N]] = !DINamespace(name: "__cxxabiv1"



