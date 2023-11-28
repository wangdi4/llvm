//===----------------------------------------------------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the XML coverage renderer.
//
//===----------------------------------------------------------------------===//

#include "Intel_CoverageReportXML.h"

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/ProfileData/Coverage/CoverageMapping.h"

using namespace llvm;
using namespace coverage;

// Filter the set of all possible files to report based on the filter of file
// names, and generate the report.
void CoverageReportXML::renderFileReports(
    raw_ostream &OS, const CoverageFilters &IgnoreFilenameFilters) const {
  std::vector<std::string> UniqueSourceFiles;
  for (StringRef SF : Coverage.getUniqueSourceFiles()) {
    // Skip source files that match the name filter.
    if (!IgnoreFilenameFilters.matchesFilename(SF))
      UniqueSourceFiles.emplace_back(SF.str());
  }
  renderFileReports(OS, UniqueSourceFiles);
}

// Generate the report for the list of \p Files.
void CoverageReportXML::renderFileReports(raw_ostream &OS,
                                          ArrayRef<std::string> Files) const {
  renderFileReports(OS, Files, CoverageFiltersMatchAll());
}

// Generate the report for the list of \p Files, but filter out specific
// functions based on \p Filters.
void CoverageReportXML::renderFileReports(
    raw_ostream &OS, ArrayRef<std::string> Files,
    const CoverageFiltersMatchAll &Filters) const {

  XMLTag ProjectTag(ProjectTagName);
  ProjectTag.renderOpen(OS);
  for (StringRef Filename : Files) {
    XMLTag ModuleTag(ModuleTagName, &ProjectTag);
    ModuleTag.addAttribute(NameAttrName, Filename);

    // Collect the functions to report for this file into a vector that can be
    // sorted by the starting line number.
    std::vector<std::pair<unsigned, const coverage::FunctionRecord *>>
        FuncsToReport;
    // An InstantiationGroup represents a function of the file, and may have
    // more than one instantiation due to template instances or inline copies of
    // static functions.
    for (const auto &Group : Coverage.getInstantiationGroups(Filename)) {
      // Walk over each instantiation.
      const auto &Instantiations = Group.getInstantiations();
      for (const coverage::FunctionRecord *F : Instantiations) {
        if (!Filters.matches(Coverage, *F))
          continue;

        assert(!F->CountedRegions.empty() &&
               "Function expected to always have at least 1 region");
        LineColPair FuncStart = F->CountedRegions.front().startLoc();
        FuncsToReport.push_back({FuncStart.first, F});
      }
    }

    if (!FuncsToReport.empty()) {
      ModuleTag.renderOpen(OS);
      std::sort(FuncsToReport.begin(), FuncsToReport.end());
      for (const auto &LineFunc : FuncsToReport) {
        const auto &InstantiationSummary =
            FunctionCoverageSummary::get(Coverage, *LineFunc.second);
        renderFunction(OS, ModuleTag, LineFunc.second, InstantiationSummary);
      }
      ModuleTag.renderClose(OS);
    }
  }
  ProjectTag.renderClose(OS);
}

void CoverageReportXML::renderFunction(
    raw_ostream &OS, XMLTag &ModuleTag, const coverage::FunctionRecord *F,
    const FunctionCoverageSummary &InstantiationSummary) const {
  XMLTag FunctionTag(FunctionTagName, &ModuleTag);
  FunctionTag.addAttribute(NameAttrName,
                           DemangleName(InstantiationSummary.Name));
  FunctionTag.addAttribute(FreqAttrName, InstantiationSummary.ExecutionCount);
  FunctionTag.renderOpen(OS);
  const auto &RegionCoverage = InstantiationSummary.RegionCoverage;
  XMLTag BlocksTag(BlocksTagName, &FunctionTag);
  BlocksTag.addAttribute(TotalBlocksAttrName, RegionCoverage.getNumRegions());
  BlocksTag.addAttribute(CoveredBlockCountAttrName,
                         RegionCoverage.getCovered());
  BlocksTag.addAttribute(CoveragePercentageAttrName,
                         std::to_string(RegionCoverage.getPercentCovered()) +
                             "%");
  BlocksTag.renderOpen(OS);

  if (Options.ShowRegionsInXML) {
    // There are two types of regions we are concerned with for generating
    // the detailed report. Code regions which have a source location, and
    // a file id. The file id is an index into F->Filenames map. The file id
    // also serves as a way to associate a macro expansion region that describes
    // where a macro is used.
    //
    // When macros are used, an ExpansionRegion is used which contains
    //   1. The code location where the macro is used
    //   2. The source file id of the location where the macro is used.
    //      - This may be another macro.
    //   3. An expanded file id that associates the source location with a code
    //   region that specifies the source locatoin of the macro.
    //
    // For example:
    // Code region:      Start line:col - End line:col   File Id
    //   Case 1:              687:36   -     687:71        0
    //   Case 2:              430:44   -     430:66        1
    //   Case 3:              193:30   -     193:124       4
    //
    // Expansion region: Start line:col - End line:col   File Id    Expanded
    //                                                              File Id
    //                         688:13   -    688:43        0          1
    //                         689:13   -    689:43        0          2
    //                         430:44   -    430:64        2          4
    //
    // Case 1: Direct mapping
    //   The source code line:column location is 687:36 to 687:71
    //
    // Case 2: Simple macro usage
    //   A source code region from a macro defined at the location 430:44 -
    //   430:66 is used with an expansion of the macro at line 688:13 - 688:43.
    //   This is identified by matching the 'File Id' = 1 in the code region
    //   table, with the element in the 'Expansion region' table with the
    //   'Expanded File Id' = 1.
    //
    // Case 3: A macro that made use of another macro
    //   A source code region from a macro defined at the location 193:30 -
    //   193:124 is used in a macro at 430:44 - 430:64, which is used in the
    //   function via a macro expansion in the region 689:13 - 689:43. This is
    //   found from multiple iterations of the expansion region table to reach
    //   an entry with a File id that is not a member of the expansion table.
    //

    // Print the code region and expansion regions when -dump option is used.
    if (Options.Debug)
      dbgs() << "Collecting region infor for function: " << F->Name << "\n";

    // Map 'expansion id' to its corresponding 'file id', start & end locations
    // to enable lookups when processing the CodeRegion elements.
    DenseMap<uint32_t, std::tuple<uint32_t, LineColPair, LineColPair>>
        MacroUseMapping;
    for (const auto &CR : F->CountedRegions) {
      if (CR.Kind != coverage::CounterMappingRegion::ExpansionRegion)
        continue;

      LineColPair StartLoc = CR.startLoc();
      LineColPair EndLoc = CR.endLoc();
      MacroUseMapping.insert(
          {CR.ExpandedFileID, {CR.FileID, StartLoc, EndLoc}});

      if (Options.Debug)
        dbgs() << "  ER: " << StartLoc.first << ":" << StartLoc.second << " - "
               << EndLoc.first << ":" << EndLoc.second << "@" << CR.FileID
               << "^" << CR.ExpandedFileID << " "
               << F->Filenames[CR.ExpandedFileID] << "\n";
    }

    // Collect the information to be reported, so that instances that map to the
    // same code region can be grouped in the output.
    DenseMap<std::pair<LineColPair, LineColPair>, OutputRecord> InstanceMapping;
    for (const auto &CR : F->CountedRegions) {
      if (CR.Kind != coverage::CounterMappingRegion::CodeRegion)
        continue;

      uint32_t FileId = CR.FileID;
      LineColPair StartLoc = CR.startLoc();
      LineColPair EndLoc = CR.endLoc();

      if (Options.Debug)
        dbgs() << "  CR: " << CR.startLoc().first << ":" << CR.startLoc().second
               << " - " << CR.endLoc().first << ":" << CR.endLoc().second << "@"
               << CR.FileID << "\n";

      auto It = MacroUseMapping.find(FileId);
      if (It == MacroUseMapping.end()) {
        auto Exists = InstanceMapping.find({StartLoc, EndLoc});
        if (Exists == InstanceMapping.end())
          InstanceMapping.insert(
              {{CR.startLoc(), CR.endLoc()},
               OutputRecord(CR.startLoc(), CR.endLoc(), CR.ExecutionCount)});
        else
          Exists->second.addInstance(CR.ExecutionCount);
      } else {
        uint32_t MacroUseLocFileId = std::get<0>(It->second);
        SmallSet<uint32_t, 4> Visited;
        auto NestIt = MacroUseMapping.find(MacroUseLocFileId);
        Visited.insert(MacroUseLocFileId);
        while (NestIt != MacroUseMapping.end()) {
          It = NestIt;
          MacroUseLocFileId = std::get<0>(NestIt->second);
          NestIt = MacroUseMapping.find(MacroUseLocFileId);
          [[maybe_unused]] auto KV = Visited.insert(MacroUseLocFileId);
          assert(KV.second && "Cycle detected in macro map");
        }

        // Save the code region's location and file information in the instance
        // mapping to report where the macro is defined, but use the location
        // where the macro is used as the sorting key for outputting locations
        // within the current function.
        LineColPair UseStart = std::get<1>(It->second);
        LineColPair UseEnd = std::get<2>(It->second);
        auto Exists = InstanceMapping.find({UseStart, UseEnd});
        if (Exists == InstanceMapping.end())
          InstanceMapping.insert(
              {{UseStart, UseEnd},
               OutputRecord(UseStart, UseEnd, CR.ExecutionCount, FileId,
                            StartLoc, EndLoc)});
        else
          Exists->second.addInstance(CR.ExecutionCount, FileId, StartLoc,
                                     EndLoc);
      }
    }

    SmallVector<OutputRecord, 32> SortedInstanceMapping;
    SortedInstanceMapping.reserve(InstanceMapping.size());
    std::transform(InstanceMapping.begin(), InstanceMapping.end(),
                   std::back_inserter(SortedInstanceMapping),
                   [](auto &KV) { return KV.second; });
    std::sort(SortedInstanceMapping.begin(), SortedInstanceMapping.end());
    for (const auto &Elem : SortedInstanceMapping) {
      XMLTag BlockTag(BlockTagName, &BlocksTag);
      BlockTag.addAttribute(LineStartAttrName, Elem.StartLoc.first);
      BlockTag.addAttribute(ColumnStartAttrName, Elem.StartLoc.second);
      BlockTag.addAttribute(LineEndAttrName, Elem.EndLoc.first);
      BlockTag.addAttribute(ColumnEndAttrName, Elem.EndLoc.second);

      BlockTag.renderOpen(OS);
      for (const auto &Inst : enumerate(Elem.Instances)) {
        XMLTag InstanceTag(InstanceTagName, &BlockTag);
        InstanceTag.addAttribute(InstIdAttrName, Inst.index() + 1);
        InstanceTag.addAttribute(FreqAttrName, Inst.value().ExecCount);
        if (Inst.value().MacroFile) {
          // Report the macro definition location
          uint32_t MacroFileId = Inst.value().MacroFile.value();
          LineColPair MacroStart = Inst.value().MacroDefStartLoc.value();
          LineColPair MacroEnd = Inst.value().MacroDefEndLoc.value();
          InstanceTag.addAttribute(MacroExpFileAttrName,
                                   F->Filenames[MacroFileId]);
          InstanceTag.addAttribute(MacroExpLineStartAttrName, MacroStart.first);
          InstanceTag.addAttribute(MacroExpColStartAttrName, MacroStart.second);
          InstanceTag.addAttribute(MacroExpLineEndAttrName, MacroEnd.first);
          InstanceTag.addAttribute(MacroExpColEndAttrName, MacroEnd.second);
        }
        InstanceTag.renderOpen(OS);
        InstanceTag.renderClose(OS);
      }
      BlockTag.renderClose(OS);
    }
  }
  BlocksTag.renderClose(OS);
  FunctionTag.renderClose(OS);
}

std::string CoverageReportXML::DemangleName(StringRef Name) const {
  // Names for static functions are of the form 'filename:name',
  // just pass the function name to the demangler. Because the
  // functions are being grouped by the module already, we will
  // strip off the filename portion for the returned string.
  size_t Demimiter = Name.find_last_of(':');
  std::string Filename;
  if (Demimiter != std::string::npos)
    Name = Name.substr(Demimiter + 1);

  return ::demangle(std::string(Name));
}

CoverageReportXML::XMLTag::XMLTag(StringRef Tag, XMLTag *Parent)
    : TagName(Tag), Parent(Parent) {
  if (Parent)
    Level = Parent->getLevel() + 1;
}

CoverageReportXML::XMLTag::~XMLTag() { assert(!IsOpen && "XMLTag not closed"); }

void CoverageReportXML::XMLTag::renderOpen(raw_ostream &OS) {
  OS.indent(Level * 2);
  OS << "<" << TagName;
  for (const auto &KV : Attributes)
    OS << " " << KV.first << "=\"" << KV.second << "\"";
  OS << ">\n";
  IsOpen = true;
}
void CoverageReportXML::XMLTag::renderClose(raw_ostream &OS) {
  if (Parent)
    assert(Parent->isOpen() && "Incorrect close order");

  assert(isOpen() && "Attempt to close non-open tag");
  OS.indent(Level * 2);
  OS << "</" << TagName << ">\n";
  IsOpen = false;
}

void CoverageReportXML::XMLTag::addAttribute(StringRef Name, StringRef Value) {
  Attributes.push_back({Name, replaceSpecialCharacters(Value)});
}

void CoverageReportXML::XMLTag::addAttribute(StringRef Name, uint64_t Value) {
  Attributes.push_back({Name, std::to_string(Value)});
}

std::string
CoverageReportXML::XMLTag::replaceSpecialCharacters(StringRef Value) {
  size_t From = 0;
  size_t Index;
  std::string Result;
  while ((Index = Value.find_first_of("&<>", From)) != StringRef::npos) {
    Result += Value.substr(From, Index - From);
    switch (Value[Index]) {
    case '&':
      Result += "&amp;";
      break;
    case '<':
      Result += "&lt;";
      break;
    case '>':
      Result += "&gt;";
      break;
    default:
      llvm_unreachable("Unexpected character");
    }
    From = Index + 1;
  }
  Result += Value.substr(From);
  return Result;
}
