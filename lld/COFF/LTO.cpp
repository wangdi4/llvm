//===- LTO.cpp ------------------------------------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LTO.h"
#include "Config.h"
#include "InputFiles.h"
#include "Symbols.h"
#include "lld/Common/Args.h"
#include "lld/Common/CommonLinkerContext.h"
#include "lld/Common/Strings.h"
#include "lld/Common/TargetOptionsCommandFlags.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/LTO/Config.h"
#include "llvm/LTO/LTO.h"
#include "llvm/Object/SymbolicFile.h"
#include "llvm/Support/Caching.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Intel_WP_utils.h" // INTEL
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

using namespace llvm;
using namespace llvm::object;
using namespace lld;
using namespace lld::coff;

// Creates an empty file to and returns a raw_fd_ostream to write to it.
static std::unique_ptr<raw_fd_ostream> openFile(StringRef file) {
  std::error_code ec;
  auto ret =
      std::make_unique<raw_fd_ostream>(file, ec, sys::fs::OpenFlags::OF_None);
  if (ec) {
    error("cannot open " + file + ": " + ec.message());
    return nullptr;
  }
  return ret;
}

static std::string getThinLTOOutputFile(StringRef path) {
  return lto::getThinLTOOutputFile(
      std::string(path), std::string(config->thinLTOPrefixReplace.first),
      std::string(config->thinLTOPrefixReplace.second));
}

static lto::Config createConfig() {
  lto::Config c;
  c.Options = initTargetOptionsFromCodeGenFlags();
#if INTEL_CUSTOMIZATION
  // This change has caused numerous CompFails in CPU2017 LIT testing.
  // Disabling it here
  // TODO: Enable after fixing CMPLRLLVM-28261
  // c.Options.EmitAddrsig = true;
  c.Options.IntelLibIRCAllowed = config->intelLibIRCAllowed;
#endif // INTEL_CUSTOMIZATION
  // Always emit a section per function/datum with LTO. LLVM LTO should get most
  // of the benefit of linker GC, but there are still opportunities for ICF.
  c.Options.FunctionSections = true;
  c.Options.DataSections = true;

  // Use static reloc model on 32-bit x86 because it usually results in more
  // compact code, and because there are also known code generation bugs when
  // using the PIC model (see PR34306).
  if (config->machine == COFF::IMAGE_FILE_MACHINE_I386)
    c.RelocModel = Reloc::Static;
  else
    c.RelocModel = Reloc::PIC_;
#ifndef NDEBUG
  c.DisableVerify = false;
#else
  c.DisableVerify = true;
#endif
  c.DiagHandler = diagnosticHandler;
  c.OptLevel = config->ltoo;
  c.CPU = getCPUStr();
  c.MAttrs = getMAttrs();
  c.CGOptLevel = args::getCGOptLevel(config->ltoo);
  c.AlwaysEmitRegularLTOObj = !config->ltoObjPath.empty();
  c.UseNewPM = config->ltoNewPassManager; // INTEL
  c.DebugPassManager = config->ltoDebugPassManager;
  c.CSIRProfile = std::string(config->ltoCSProfileFile);
  c.RunCSIRInstr = config->ltoCSProfileGenerate;
  c.PGOWarnMismatch = config->ltoPGOWarnMismatch;
#if INTEL_CUSTOMIZATION
  c.PTO.LoopVectorization = c.OptLevel > 1;
  c.PTO.SLPVectorization = c.OptLevel > 1;
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  // Linking for an executable
  // NOTE: Whole program can't be achieved when linking a DLL, it is only
  // for executables. On the other hand, whole program read can be achieved
  // when linking an executable and a DLL is present.
  if (!config->dll)
    c.WPUtils.setLinkingExecutable(true);
#endif // INTEL_CUSTOMIZATION

  if (config->saveTemps)
    checkError(c.addSaveTemps(std::string(config->outputFile) + ".",
                              /*UseInputModulePath*/ true));
  return c;
}

BitcodeCompiler::BitcodeCompiler() {
  // Initialize indexFile.
  if (!config->thinLTOIndexOnlyArg.empty())
    indexFile = openFile(config->thinLTOIndexOnlyArg);

  // Initialize ltoObj.
  lto::ThinBackend backend;
  if (config->thinLTOIndexOnly) {
    auto OnIndexWrite = [&](StringRef S) { thinIndices.erase(S); };
    backend = lto::createWriteIndexesThinBackend(
        std::string(config->thinLTOPrefixReplace.first),
        std::string(config->thinLTOPrefixReplace.second),
        config->thinLTOEmitImportsFiles, indexFile.get(), OnIndexWrite);
  } else {
    backend = lto::createInProcessThinBackend(
        llvm::heavyweight_hardware_concurrency(config->thinLTOJobs));
  }

  ltoObj = std::make_unique<lto::LTO>(createConfig(), backend,
                                       config->ltoPartitions);
}

BitcodeCompiler::~BitcodeCompiler() = default;

static void undefine(Symbol *s) { replaceSymbol<Undefined>(s, s->getName()); }

void BitcodeCompiler::add(BitcodeFile &f) {
  lto::InputFile &obj = *f.obj;
  unsigned symNum = 0;
  std::vector<Symbol *> symBodies = f.getSymbols();
  std::vector<lto::SymbolResolution> resols(symBodies.size());

  if (config->thinLTOIndexOnly)
    thinIndices.insert(obj.getName());

  // Provide a resolution to the LTO API for each symbol.
  for (const lto::InputFile::Symbol &objSym : obj.symbols()) {
    Symbol *sym = symBodies[symNum];
    lto::SymbolResolution &r = resols[symNum];
    ++symNum;

    // Ideally we shouldn't check for SF_Undefined but currently IRObjectFile
    // reports two symbols for module ASM defined. Without this check, lld
    // flags an undefined in IR with a definition in ASM as prevailing.
    // Once IRObjectFile is fixed to report only one symbol this hack can
    // be removed.
    r.Prevailing = !objSym.isUndefined() && sym->getFile() == &f;
#if INTEL_CUSTOMIZATION
    bool definitionFound = false;

    // If a symbol is undefined then we need to check if it a weak symbol. A
    // weak symbol might not have a definition but it will be replaced with
    // the strong symbol that it is mapped to. This is important in Windows
    // because lib CTR replaces ISO C++ functions with MS version (e.g.
    // putenv - > _putenv).
    if (Undefined *currSym = dyn_cast<Undefined>(sym)) {
      Defined *aliasSym = currSym->getWeakAlias();
      if (aliasSym)
        definitionFound = true;
    }
    // Check if the symbol was defined or was marked as lazy symbol. A lazy
    // symbol is a symbol whose definition was found in a library but it
    // isn't used.
    else {
      definitionFound = isa<Defined>(sym) || sym->isLazy();
    }

    // Mark that the symbol was resolved by the Linker
    r.ResolvedByLinker = definitionFound;
#endif // INTEL_CUSTOMIZATION
    r.VisibleToRegularObj = sym->isUsedInRegularObj;
    if (r.Prevailing)
      undefine(sym);

    // We tell LTO to not apply interprocedural optimization for wrapped
    // (with -wrap) symbols because otherwise LTO would inline them while
    // their values are still not final.
    r.LinkerRedefined = !sym->canInline;
  }
  checkError(ltoObj->add(std::move(f.obj), resols));
}

// Merge all the bitcode files we have seen, codegen the result
// and return the resulting objects.
#if INTEL_CUSTOMIZATION
std::vector<InputFile *>
BitcodeCompiler::compile(COFFLinkerContext &ctx,
                         std::vector<StringRef> *buffersOut) {
#endif // INTEL_CUSTOMIZATION
  unsigned maxTasks = ltoObj->getMaxTasks();
  buf.resize(maxTasks);
  files.resize(maxTasks);

  // The /lldltocache option specifies the path to a directory in which to cache
  // native object files for ThinLTO incremental builds. If a path was
  // specified, configure LTO to use it as the cache directory.
  FileCache cache;
  if (!config->ltoCache.empty())
    cache =
        check(localCache("ThinLTO", "Thin", config->ltoCache,
                         [&](size_t task, std::unique_ptr<MemoryBuffer> mb) {
                           files[task] = std::move(mb);
                         }));

  checkError(ltoObj->run(
      [&](size_t task) {
        return std::make_unique<CachedFileStream>(
            std::make_unique<raw_svector_ostream>(buf[task]));
      },
      cache));

  // Emit empty index files for non-indexed files
  for (StringRef s : thinIndices) {
    std::string path = getThinLTOOutputFile(s);
    openFile(path + ".thinlto.bc");
    if (config->thinLTOEmitImportsFiles)
      openFile(path + ".imports");
  }

  // ThinLTO with index only option is required to generate only the index
  // files. After that, we exit from linker and ThinLTO backend runs in a
  // distributed environment.
  if (config->thinLTOIndexOnly) {
    if (!config->ltoObjPath.empty())
      saveBuffer(buf[0], config->ltoObjPath);
    if (indexFile)
      indexFile->close();
    return {};
  }

  if (!config->ltoCache.empty())
    pruneCache(config->ltoCache, config->ltoCachePolicy);

  std::vector<InputFile *> ret;
  for (unsigned i = 0; i != maxTasks; ++i) {
    // Assign unique names to LTO objects. This ensures they have unique names
    // in the PDB if one is produced. The names should look like:
    // - foo.exe.lto.obj
    // - foo.exe.lto.1.obj
    // - ...
    StringRef ltoObjName =
        saver().save(Twine(config->outputFile) + ".lto" +
                     (i == 0 ? Twine("") : Twine('.') + Twine(i)) + ".obj");

    // Get the native object contents either from the cache or from memory.  Do
    // not use the cached MemoryBuffer directly, or the PDB will not be
    // deterministic.
    StringRef objBuf;
    if (files[i])
      objBuf = files[i]->getBuffer();
    else
      objBuf = buf[i];
    if (objBuf.empty())
      continue;

    if (config->saveTemps)
      saveBuffer(buf[i], ltoObjName);
    ret.push_back(make<ObjFile>(ctx, MemoryBufferRef(objBuf, ltoObjName)));
#if INTEL_CUSTOMIZATION
    if (buffersOut)
      buffersOut->push_back(objBuf);
#endif // INTEL_CUSTOMIZATION
  }

  return ret;
}
