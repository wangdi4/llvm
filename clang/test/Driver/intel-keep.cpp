// UNSUPPORTED: system-windows
// Arg files created with -i_keep and /Q_keep option
// RUN: %clang -save-temps -i_keep -target x86_64-unknown-linux-gnu %s -###
// RUN: %clang_cl -save-temps /Q_keep /Qsave-temps --target=x86_64-pc-windows-msvc %s -###
// RUN: ls clang*.assembler.x86_64.arg
// RUN: ls clang*.compiler.x86_64.arg
// RUN: ls clang*.backend.x86_64.arg
// RUN: ls clang*.preprocessor.x86_64.arg
// RUN: ls ld.linker.x86_64.arg

// RUN: %clang -save-temps -i_keep -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -###
// RUN: %clang_cl -Qsave-temps /Q_keep -Qiopenmp -Qopenmp-targets=spir64 --target=x86_64-pc-windows-msvc %s -###
// RUN: ls clang*.backend.spir64.arg
// RUN: ls clang*.backend.x86_64.arg
// RUN: ls clang*.compiler.spir64.arg
// RUN: ls clang*.compiler.x86_64.arg
// RUN: ls clang*.assembler.x86_64.arg
// RUN: ls clang*.preprocessor.spir64.arg
// RUN: ls clang*.preprocessor.x86_64.arg
// RUN: ls clang-offload-bundler.clang-offload-unbundler.spir64.arg
// RUN: ls clang-offload-wrapper.clang-offload-wrapper.x86_64.arg
// RUN: ls ld.linker.x86_64.arg
// RUN: ls llvm-link.linker.spir64.arg
// RUN: ls llvm-spirv.llvm-spirv.spir64.arg
// RUN: ls sycl-post-link.sycl-post-link.spir64.arg
