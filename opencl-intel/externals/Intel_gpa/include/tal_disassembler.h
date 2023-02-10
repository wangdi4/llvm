/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) 2010, Intel Corporation. All rights reserved.
**
** INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
*LICENSED
** ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
** INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT PROVIDE ANY
*UPDATES,
** ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY DISCLAIMS ANY WARRANTY OF
** MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY PARTICULAR PURPOSE, OR ANY
** OTHER WARRANTY.  Intel disclaims all liability, including liability for
** infringement of any proprietary rights, relating to use of the code. No
*license,
** express or implied, by estoppel or otherwise, to any intellectual property
** rights is granted herein.
**
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef TAL_DISASSEMBLER_H
#define TAL_DISASSEMBLER_H

// This file is intentionally not included in the main taskalyzer header.

#include <public/tal_macros.h>
#include <public/tal_types.h>
#include <stdio.h>
typedef void (*TAL_OutputCallback)(const char *text);
typedef TAL_BOOL (*TAL_TraceDataFilter)(TAL_UINT32 *begin,
                                        TAL_UINT32 sizeInBytes);

#ifndef DLL_API
#define DLL_API
#ifndef TAL_STATIC
#ifdef _WIN32
#undef DLL_API
#ifdef TALDECODE_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif
#endif
#endif
#endif

DLL_API int TAL_GetNumOpcodes();
DLL_API TAL_OPCODEINFO *TAL_GetOpcode(int index);
DLL_API const char *TAL_GetOpcodeName(TAL_OPCODE opc);
DLL_API TAL_OPCODEINFO *TAL_GetOpcodeInfo(TAL_OPCODE opc);

DLL_API int TAL_GetNumRelations();
DLL_API TAL_RELATIONINFO *TAL_GetRelation(int index);
DLL_API TAL_RELATIONINFO *TAL_GetRelationInfo(TAL_RELATION rel);

DLL_API TAL_FILE_VERSION TAL_DetermineFileVersion(TAL_UINT32 dw0,
                                                  TAL_UINT32 dw1,
                                                  TAL_UINT32 dw2);

DLL_API void TAL_DisassembleTraceData(TAL_UINT32 *begin,
                                      TAL_UINT32 numDWords); // to stdout
DLL_API void TAL_DisassembleTraceDataToFile(FILE *f, TAL_UINT32 *begin,
                                            TAL_UINT32 numDWords);
DLL_API void TAL_DisassembleTraceDataToCallback(TAL_OutputCallback cb,
                                                TAL_UINT32 *begin,
                                                TAL_UINT32 numDWords);

DLL_API void TAL_DisassembleEscape(TAL_UINT32 opcode, void *begin,
                                   TAL_UINT32 sizeInBytes); // to stdout
DLL_API void TAL_DisassembleEscapeToFile(FILE *f, TAL_UINT32 opcode,
                                         void *begin, TAL_UINT32 sizeInBytes);
DLL_API void TAL_DisassembleEscapeToCallback(TAL_OutputCallback cb,
                                             TAL_UINT32 opcode, void *begin,
                                             TAL_UINT32 sizeInBytes);

DLL_API void TAL_DisassembleTrace(TAL_TRACE *trace); // to stdout
DLL_API void TAL_DisassembleTraceToFile(FILE *f, TAL_TRACE *trace);
DLL_API void TAL_DisassembleTraceToCallback(TAL_OutputCallback cb,
                                            TAL_TRACE *trace);

DLL_API void TAL_SetDisassemblerVirtualStart(TAL_UINT32 vstart);
DLL_API void TAL_SetDisassemblerTraceDataFilter(TAL_TraceDataFilter filter);

DLL_API void TAL_DisassembleCommandArgmentsToCallback(TAL_OutputCallback cb,
                                                      TAL_UINT32 *command);

DLL_API TAL_BOOL TAL_DisassembleProcessAndThreadNames(TAL_UINT32 *begin,
                                                      TAL_UINT32 numDWords,
                                                      char *out_processname,
                                                      char *out_threadname);

#endif // TAL_DISASSEMBLER_H
