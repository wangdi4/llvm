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
#ifndef TAL_MACROS_H
#define TAL_MACROS_H

#include "tal_opcodes.h"
#include "tal_rdtsc.h"
#include "tal_structs.h"
#include "tal_types.h"

#if TAL_COMPILER == TAL_COMPILER_MSVC || TAL_COMPILER == TAL_COMPILER_ICC
#pragma warning(                                                               \
    disable : 4127) // warning C4127: conditional expression is constant
#endif // TAL_COMPILER == TAL_COMPILER_MSVC || TAL_COMPILER == TAL_COMPILER_ICC

#define TAL_RESERVE(tr, numDWords)                                             \
  if ((tr)->TalTrace_cur + numDWords > (tr)->TalTrace_end) {                   \
    (tr)->TalTrace_pfnFlush((TAL_TRACE *)tr);                                  \
  }

#define TAL_RESERVE_CMDP(cmdP, numDWords)                                      \
  if ((cmdP)->TalTrace_cur + numDWords > (cmdP)->TalTrace_end) {               \
    (cmdP)->CmdPool_pfnFlush((TAL_TRACE *)tr);                                 \
  }

#define TAL_REMAIN_DWORDS(tr) ((tr)->TalTrace_end - (tr)->TalTrace_cur)
#define TAL_REMAIN_BYTES(tr) (TAL_REMAIN_DWORDS(tr) << 2)
#define TAL_REMAIN_WORDS(tr) (TAL_REMAIN_DWORDS(tr) << 1)

#define TAL_COMMAND_OPCODE_SHIFT 21
#define TAL_COMMAND_OPCODE_MASK 0x3FF
#define TAL_COMMAND_NUMDWORDS_SHIFT 11
#define TAL_COMMAND_NUMDWORDS_MASK 0x3FF
#define TAL_COMMAND_ARGA_SHIFT 3
#define TAL_COMMAND_ARGA_MASK 0xFF
#define TAL_COMMAND_ARGB_SHIFT 0
#define TAL_COMMAND_ARGB_MASK 0x7

#define TAL_COMMAND(opcode, numDWords, argA, argB)                             \
  (((opcode) << TAL_COMMAND_OPCODE_SHIFT) |                                    \
   ((numDWords) << TAL_COMMAND_NUMDWORDS_SHIFT) |                              \
   ((argA) << TAL_COMMAND_ARGA_SHIFT) | ((argB) << TAL_COMMAND_ARGB_SHIFT))
#define TAL_ESCAPE_COMMAND(escape_opcode, argB)                                \
  TAL_COMMAND(TAL_OPCODE_ESCAPE, 1, escape_opcode, argB)

#define TAL_GET_OPCODE(cmd)                                                    \
  (((cmd) >> TAL_COMMAND_OPCODE_SHIFT) & TAL_COMMAND_OPCODE_MASK)
#define TAL_GET_NUMDWORDS(cmd)                                                 \
  (((cmd) >> TAL_COMMAND_NUMDWORDS_SHIFT) & TAL_COMMAND_NUMDWORDS_MASK)
#define TAL_GET_ARGA(cmd)                                                      \
  (((cmd) >> TAL_COMMAND_ARGA_SHIFT) & TAL_COMMAND_ARGA_MASK)
#define TAL_GET_ARGB(cmd)                                                      \
  (((cmd) >> TAL_COMMAND_ARGB_SHIFT) & TAL_COMMAND_ARGB_MASK)

#define TAL_SET_ARGA(cmd, v)                                                   \
  (((cmd) & ~(TAL_COMMAND_ARGA_MASK << TAL_COMMAND_ARGA_SHIFT)) |              \
   (((v)&TAL_COMMAND_ARGA_MASK) << TAL_COMMAND_ARGA_SHIFT))
#define TAL_SET_ARGB(cmd, v)                                                   \
  (((cmd) & ~(TAL_COMMAND_ARGB_MASK << TAL_COMMAND_ARGB_SHIFT)) |              \
   (((v)&TAL_COMMAND_ARGB_MASK) << TAL_COMMAND_ARGB_SHIFT))
#define TAL_SET_NUMDWORDS(cmd, v)                                              \
  ((cmd & ~(TAL_COMMAND_NUMDWORDS_MASK << TAL_COMMAND_NUMDWORDS_SHIFT)) |      \
   (((v)&TAL_COMMAND_NUMDWORDS_MASK) << TAL_COMMAND_NUMDWORDS_SHIFT))

#define TAL_IS_ESCAPE_COMMAND(cmd, escape_opcode)                              \
  ((TAL_GET_OPCODE(cmd) == TAL_OPCODE_ESCAPE) &&                               \
   (TAL_GET_ARGA(cmd) == (escape_opcode)))
#define TAL_SET_NUMDWORDS(cmd, v)                                              \
  ((cmd & ~(TAL_COMMAND_NUMDWORDS_MASK << TAL_COMMAND_NUMDWORDS_SHIFT)) |      \
   (((v)&TAL_COMMAND_NUMDWORDS_MASK) << TAL_COMMAND_NUMDWORDS_SHIFT))

#define TAL_BYTES_TO_DWORDS(x) (((TAL_UINT32)x) >> 2)
#define TAL_PRESTORE_BLOB(s) ((s % 4) ? ((s >> 2) + 1) : (s >> 2))

// Pushing helpers
#if defined(TAL_DISABLE)
#define TAL_STORE_FLT(cur, dwordOffset, value) value;
#define TAL_STORE_U32(cur, dwordOffset, value) value;
#define TAL_STORE_U64(cur, dwordOffset, value) value;
#define TAL_STORE_U64_PTR(cur, dwordOffset, value) value;
#define TAL_PRESTORE_STR(str) (0)
#define TAL_SET_CUR(cur, tra)
#define TAL_STORE_STR(cur, dwordOffset, str) str;
#define TAL_COMMIT(trace, numDWords) trace;
#define TAL_PUSH_COMMAND(tra, opcode, numDWords, argA, argB)                   \
  tra;                                                                         \
  opcode;                                                                      \
  numDWords;                                                                   \
  argA;                                                                        \
  argB;
#define TAL_IS_LOGGABLE(tra, lvl, cat) ((lvl) != (lvl) && (cat) != (cat))
#define TAL_ROUND_TO_DWORDS(len) (0)
#define TAL_DWORDS_TO_BYTES(len) (0)
#else // !defined(TAL_DISABLE)

#define TAL_STORE_FLT(cur, dwordOffset, value)                                 \
  ((float *)&(cur)[dwordOffset])[0] = value;
#define TAL_STORE_U32(cur, dwordOffset, value) (cur)[dwordOffset] = value;
#if TAL_PLATFORM == TAL_PLATFORM_WINDOWS
#define TAL_STORE_U64(cur, dwordOffset, value)                                 \
  ((TAL_UINT64 *)&(cur)[dwordOffset])[0] = value;
#elif TAL_PLATFORM == TAL_PLATFORM_NIX
#define TAL_STORE_U64(cur, dwordOffset, value)                                 \
  {                                                                            \
    TAL_UINT64 __value = (value);                                              \
    ((TAL_UINT32 *)&(cur)[dwordOffset])[0] = __value;                          \
    ((TAL_UINT32 *)&(cur)[dwordOffset])[1] = (__value >> 32);                  \
  }
#else // TAL_PLATFORM
#error "Not defined for current platform"
#endif // TAL_PLATFORM == TAL_PLATFORM_WINDOWS

#ifdef TAL_32 // need to cast to int32 before going to u64 to fool some
              // compilers... :~!
#define TAL_STORE_U64_PTR(cur, dwordOffset, value)                             \
  ((TAL_UINT64 *)&(cur)[dwordOffset])[0] = (TAL_UINT64)((TAL_UINT32)value);
#else // TAL_64
#define TAL_STORE_U64_PTR(cur, dwordOffset, value)                             \
  ((TAL_UINT64 *)&(cur)[dwordOffset])[0] = (TAL_UINT64)value;
#endif // TAL_ bits
#define TAL_PRESTORE_STR(str) (((TAL_UINT32)strlen(str) >> 2) + 1)
#define TAL_STORE_STR(cur, dwordOffset, str)                                   \
  strcpy((char *)&(cur[dwordOffset]), str);
#define TAL_COMMIT(trace, numDWords)                                           \
  ((trace)->TalTrace_cur += (1 + (numDWords)));

#define TAL_PRESTORE_STR_EX(cnt) (((TAL_UINT32)cnt >> 2) + 1)
#define TAL_STORE_STR_A(cur, dwordOffset, remain, str)                         \
  strcpy_s((char *)&(cur[dwordOffset]), remain, (const char *)str);
#define TAL_STORE_STR_A_EX(cur, dwordOffset, remain, str, count)               \
  memcpy_s((void *)&(cur[dwordOffset]), remain, (const void *)str,             \
           count * sizeof(char));
#define TAL_STORE_STR_W(cur, dwordOffset, remain, str)                         \
  wcscpy_s((wchar_t *)&(cur[dwordOffset]), remain, (const wchar_t *)str);
#define TAL_STORE_STR_W_EX(cur, dwordOffset, remain, str, count)               \
  memcpy_s((void *)&(cur[dwordOffset]), remain, (const void *)str,             \
           count * sizeof(wchar_t));

#if TAL_PLATFORM == TAL_PLATFORM_WINDOWS
#define TAL_STORE_I64(cur, dwordOffset, value)                                 \
  ((TAL_INT64 *)&(cur)[dwordOffset])[0] = value;
#elif TAL_PLATFORM == TAL_PLATFORM_NIX
#define TAL_STORE_I64(cur, dwordOffset, value)                                 \
  {                                                                            \
    TAL_INT64 __value = (value);                                               \
    ((TAL_INT32 *)&(cur)[dwordOffset])[0] = __value;                           \
    ((TAL_INT32 *)&(cur)[dwordOffset])[1] = (__value >> 32);                   \
  }
#else // TAL_PLATFORM
#error "Not defined for current platform"
#endif // TAL_PLATFORM == TAL_PLATFORM_WINDOWS

#define TAL_STORE_I32(cur, dwordOffset, value)                                 \
  ((TAL_INT32 *)&(cur)[dwordOffset])[0] = value;
#define TAL_STORE_DBL(cur, dwordOffset, value)                                 \
  ((double *)&(cur)[dwordOffset])[0] = value;
#define TAL_STORE_I16(cur, dwordOffset, value)                                 \
  ((TAL_INT16 *)&(cur)[dwordOffset])[0] = value;
#define TAL_STORE_U16(cur, dwordOffset, value)                                 \
  ((TAL_UINT16 *)&(cur)[dwordOffset])[0] = value;
#define TAL_STORE_I16_EX(cur, dwordOffset, remain, data, count)                \
  memcpy_s((void *)&(cur[dwordOffset]), remain, (const void *)data,            \
           count * sizeof(TAL_INT16));
#define TAL_STORE_U16_EX(cur, dwordOffset, remain, data, count)                \
  memcpy_s((void *)&(cur[dwordOffset]), remain, (const void *)data,            \
           count * sizeof(TAL_UINT16));
#define TAL_STORE_UDT(cur, dwordOffset, remain, data, size)                    \
  memcpy_s((void *)&(cur[dwordOffset]), remain, (const void *)data, size);

#define TAL_SET_CUR(var, tra) TAL_UINT32 *var = (tra)->TalTrace_cur;
#define TAL_PUSH_COMMAND(tra, opcode, numDWords, argA, argB)                   \
  TAL_RESERVE(tra, 1 + (numDWords));                                           \
  TAL_STORE_U32((tra)->TalTrace_cur, 0,                                        \
                TAL_COMMAND(opcode, numDWords, argA, argB));

#define TAL_PUSH_CMDP(cmdP, opcode, numDWords, argA, argB)                     \
  TAL_RESERVE_CMDP(cmdP, 1 + (numDWords));                                     \
  TAL_STORE_U32((cmdP)->TalTrace_cur, 0,                                       \
                TAL_COMMAND(opcode, numDWords, argA, argB));

#define TAL_ROUND_BYTES_TO_DWORDS(len) ((((TAL_UINT32)(len)) >> 2) + 1)
#define TAL_DWORDS_TO_BYTES(len) ((((TAL_UINT32)(len)) << 2))

#if TAL_PLATFORM != TAL_PLATFORM_NIX
#define TAL_IS_COMPILED_IN(lvl, cat)                                           \
  ((((TAL_UINT32)lvl) <= (TAL_MAX_COMPILED_IN_LOG_LEVEL)) &&                   \
   ((cat) & (TAL_COMPILED_IN_CATEGORIES)))
#define TAL_IS_LOGGABLE(tra, lvl, cat)                                         \
  (TAL_IS_COMPILED_IN(lvl, cat) &&                                             \
   (((TAL_UINT32)lvl) <= (tra)->captureLevel) &&                               \
   ((cat) & (tra)->captureCategory))
#else // TAL_PLATFORM == TAL_PLATFORM_NIX
#define TAL_IS_COMPILED_IN(lvl, cat)                                           \
  ((((TAL_UINT32)lvl) <= ((TAL_UINT32)TAL_MAX_COMPILED_IN_LOG_LEVEL)) &&       \
   (((TAL_UINT64)cat) & ((TAL_UINT64)TAL_COMPILED_IN_CATEGORIES)))
#define TAL_IS_LOGGABLE(tra, lvl, cat)                                         \
  (TAL_IS_COMPILED_IN(lvl, cat) &&                                             \
   (((TAL_UINT32)lvl) <= (tra)->captureLevel) &&                               \
   (((TAL_UINT64)cat) & (tra)->captureCategory))
#endif // TAL_PLATFORM

#endif // TAL_DISABLE

#define TAL_PUSH_ANNOTE_TID_PARENT_PID(tra, lvl, cat, tid, parentNameId)       \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 2;                                    \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_ANNOTE_TID_PARENT_PID, nDWords, 0, 0);       \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, tid);                                          \
          TAL_STORE_U64(cur, 3, parentNameId);                                 \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_PID_NAME(tra, lvl, cat, pid, processName, otherProc)   \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + TAL_PRESTORE_STR(processName);        \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_ANNOTE_PID_NAME, nDWords, otherProc, 0);     \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, pid);                                          \
          TAL_STORE_STR(cur, 3, processName);                                  \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }
#define TAL_PUSH_ANNOTE_TID_NAME(tra, lvl, cat, pid, tid, threadName)          \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 2 + TAL_PRESTORE_STR(threadName);     \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_ANNOTE_TID_NAME, nDWords, 0, 0);             \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, pid);                                          \
          TAL_STORE_U64(cur, 3, tid);                                          \
          TAL_STORE_STR(cur, 5, threadName);                                   \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }
#define TAL_PUSH_ANNOTE_CLOCK_JITTER(tra, lvl, cat, pid, jitter)               \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 2;                                    \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_ANNOTE_CLOCK_JITTER, nDWords, 0, 0);         \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, pid);                                          \
          TAL_STORE_U64(cur, 3, jitter);                                       \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_TASK(tra, lvl, cat, fn)                                 \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + 2;                                  \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_TASK, nDWords, 0, 0);           \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_U64_PTR(cur, 3, fn);                                       \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_TASK_EX(tra, lvl, cat, fn, ts)                          \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 2;                                    \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_TASK, nDWords, 0, 0);           \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_U64_PTR(cur, 3, fn);                                       \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_NAMED_TASK(tra, lvl, cat, name)                         \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + TAL_PRESTORE_STR(name);             \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_NAMED_TASK, nDWords, 0, 0);     \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_STR(cur, 3, name);                                         \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_NAMED_TASK_H(tra, lvl, cat, name_handle)                \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + 1;                                  \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_NAMED_TASK_H, nDWords, 0, 0);   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_U32(cur, 3, name_handle);                                  \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_NAMED_TASK_EX(tra, lvl, cat, name, ts)                  \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + TAL_PRESTORE_STR(name);               \
      TAL_UNUSED(nDWords);                                                     \
      TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_NAMED_TASK, nDWords, 0, 0);       \
      {                                                                        \
        TAL_SET_CUR(cur, (tra));                                               \
        TAL_STORE_U64(cur, 1, ts);                                             \
        TAL_STORE_STR(cur, 3, name);                                           \
        TAL_COMMIT(tra, nDWords);                                              \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_NAMED_TASK_H_EX(tra, lvl, cat, name_handle, ts)         \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 1;                                    \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_NAMED_TASK_H, nDWords, 0, 0);   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_U32(cur, 3, name_handle);                                  \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_TASK_WITH_ID(tra, lvl, cat, fn, id_ns, id_hi, id_lo)    \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 * 4;                                  \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_TASK_WITH_ID, nDWords, id_ns,   \
                         0);                                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_U64_PTR(cur, 3, fn);                                       \
          TAL_STORE_U64(cur, 5, id_hi);                                        \
          TAL_STORE_U64(cur, 7, id_lo);                                        \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_TASK_WITH_ID_EX(tra, lvl, cat, fn, id_ns, id_hi, id_lo, \
                                       ts)                                     \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 * 4;                                    \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_TASK_WITH_ID, nDWords, id_ns,   \
                         0);                                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_U64_PTR(cur, 3, fn);                                       \
          TAL_STORE_U64(cur, 5, id_hi);                                        \
          TAL_STORE_U64(cur, 7, id_lo);                                        \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_NAMED_TASK_WITH_ID(tra, lvl, cat, name, id_ns, id_hi,   \
                                          id_lo)                               \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 strDwords = TAL_PRESTORE_STR(name);               \
        TAL_CONST TAL_UINT32 nDWords = 2 + strDwords + 4;                      \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_NAMED_TASK_WITH_ID, nDWords,    \
                         id_ns, 0);                                            \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_STR(cur, 3, name);                                         \
          TAL_STORE_U64(cur, 3 + strDwords, id_hi);                            \
          TAL_STORE_U64(cur, 3 + strDwords + 2, id_lo);                        \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_NAMED_TASK_H_WITH_ID(tra, lvl, cat, name_handle, id_ns, \
                                            id_hi, id_lo)                      \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + 1 + 4;                              \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_NAMED_TASK_H_WITH_ID, nDWords,  \
                         id_ns, 0);                                            \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_U32(cur, 3, name_handle);                                  \
          TAL_STORE_U64(cur, 3 + 1, id_hi);                                    \
          TAL_STORE_U64(cur, 3 + 1 + 2, id_lo);                                \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_NAMED_TASK_WITH_ID_EX(tra, lvl, cat, name, id_ns,       \
                                             id_hi, id_lo, ts)                 \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      unsigned strDwords = TAL_PRESTORE_STR(name);                             \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + strDwords + 4;                      \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_NAMED_TASK_WITH_ID, nDWords,    \
                         id_ns, 0);                                            \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_STR(cur, 3, name);                                         \
          TAL_STORE_U64(cur, 3 + strDwords, id_hi);                            \
          TAL_STORE_U64(cur, 3 + strDwords + 2, id_lo);                        \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_NAMED_TASK_H_WITH_ID_EX(tra, lvl, cat, name_handle,     \
                                               id_ns, id_hi, id_lo, ts)        \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 1 + 4;                                \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_NAMED_TASK_H_WITH_ID, nDWords,  \
                         id_ns, 0);                                            \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_U32(cur, 3, name_handle);                                  \
          TAL_STORE_U64(cur, 3 + 1, id_hi);                                    \
          TAL_STORE_U64(cur, 3 + 1 + 2, id_lo);                                \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_END_TASK(tra, lvl, cat)                                       \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2;                                      \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_END_TASK, nDWords, 0, 0);             \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_END_TASK_EX(tra, lvl, cat, ts)                                \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2;                                        \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_END_TASK, nDWords, 0, 0);             \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_NAMED_VIRTUAL_TASK_WITH_ID(tra, lvl, cat, name, id_ns,  \
                                                  id_hi, id_lo)                \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      unsigned strDwords = TAL_PRESTORE_STR(name);                             \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + strDwords + 4;                      \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_NAMED_VIRTUAL_TASK_WITH_ID,     \
                         nDWords, id_ns, 0);                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_STR(cur, 3, name);                                         \
          TAL_STORE_U64(cur, 3 + strDwords, id_hi);                            \
          TAL_STORE_U64(cur, 3 + strDwords + 2, id_lo);                        \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_BEGIN_NAMED_VIRTUAL_TASK_H_WITH_ID(                           \
    tra, lvl, cat, name_handle, id_ns, id_hi, id_lo)                           \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + 1 + 4;                              \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_BEGIN_NAMED_VIRTUAL_TASK_H_WITH_ID,   \
                         nDWords, id_ns, 0);                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_U32(cur, 3, name_handle);                                  \
          TAL_STORE_U64(cur, 4, id_hi);                                        \
          TAL_STORE_U64(cur, 6, id_lo);                                        \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_END_VIRTUAL_TASK(tra, lvl, cat)                               \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2;                                      \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_END_VIRTUAL_TASK, nDWords, 0, 0);     \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ID_CREATED(tra, lvl, cat, id_ns, id_hi, id_lo, global)        \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + 4;                                  \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_ID_CREATED, nDWords, id_ns, global);  \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_U64(cur, 3, id_hi);                                        \
          TAL_STORE_U64(cur, 5, id_lo);                                        \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ADD_RELATION(tra, lvl, cat, relation, this_id_ns, this_id_hi, \
                              this_id_lo, that_id_ns, that_id_hi, that_id_lo)  \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + 1 + 4 + 4;                          \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_ADD_RELATION, nDWords, relation, 0);  \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_U32(cur, 3, ((this_id_ns << 8) | that_id_ns));             \
          TAL_STORE_U64(cur, 4, this_id_hi);                                   \
          TAL_STORE_U64(cur, 6, this_id_lo);                                   \
          TAL_STORE_U64(cur, 8, that_id_hi);                                   \
          TAL_STORE_U64(cur, 10, that_id_lo);                                  \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

// ASKME: Deprecate this?
#define TAL_PUSH_ADD_RELATION_TO_CURRENT(tra, lvl, cat, relation, that_id_ns,  \
                                         that_id_hi, that_id_lo)               \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 1 + 4;                                  \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_ADD_RELATION_TO_CURRENT, nDWords,     \
                         relation, 0);                                         \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, that_id_ns);                                   \
          TAL_STORE_U64(cur, 2, that_id_hi);                                   \
          TAL_STORE_U64(cur, 4, that_id_lo);                                   \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

// Should work the same as ADD_RELATION_TO_CURRENT, but just in case we need to
// treat it differently, we'll use a different opcode.
#define TAL_PUSH_ADD_RELATION_THIS(tra, lvl, cat, relation, that_id_ns,        \
                                   that_id_hi, that_id_lo)                     \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 1 + 4;                                  \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_ADD_RELATION_THIS, nDWords, relation, \
                         0);                                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, that_id_ns);                                   \
          TAL_STORE_U64(cur, 2, that_id_hi);                                   \
          TAL_STORE_U64(cur, 4, that_id_lo);                                   \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_TASK_STOLEN(tra, lvl, cat, id_ns, id_hi, id_lo)               \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + 4;                                  \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_TASK_STOLEN, nDWords, id_ns, 0);      \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_U64(cur, 3, id_hi);                                        \
          TAL_STORE_U64(cur, 5, id_lo);                                        \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ID_RETIRED(tra, lvl, cat, id_ns, id_hi, id_lo)                \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + 4;                                  \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_ID_RETIRED, nDWords, id_ns, 0);       \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_U64(cur, 3, id_hi);                                        \
          TAL_STORE_U64(cur, 5, id_lo);                                        \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

// ueber lowlevel pushers
//////////////////////////////////////////////////////////////////////////
#define TAL_PUSH_ANNOTE_PROCESSID(tra, lvl, cat, pid)                          \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2;                                        \
      TAL_UNUSED(nDWords);                                                     \
      TAL_PUSH_COMMAND(tra, TAL_ANNOTE_PROCESSID, nDWords, 0, 0);              \
      {                                                                        \
        TAL_SET_CUR(cur, (tra));                                               \
        TAL_STORE_U64(cur, 1, pid);                                            \
        TAL_COMMIT(tra, nDWords);                                              \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_THREADID(tra, lvl, cat, tid)                           \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2;                                        \
      TAL_UNUSED(nDWords);                                                     \
      TAL_PUSH_COMMAND(tra, TAL_ANNOTE_THREADID, nDWords, 0, 0);               \
      {                                                                        \
        TAL_SET_CUR(cur, (tra));                                               \
        TAL_STORE_U64(cur, 1, tid);                                            \
        TAL_COMMIT(tra, nDWords);                                              \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_CLOCK_PARAMS_ID(tra, lvl, cat, freq, base, pid)        \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 4 + 2;                                    \
      TAL_UNUSED(nDWords);                                                     \
      TAL_PUSH_COMMAND(tra, TAL_ANNOTE_CLOCKPARAMS_ID, nDWords, 0, 0);         \
      {                                                                        \
        TAL_SET_CUR(cur, (tra));                                               \
        TAL_STORE_U64(cur, 1, freq);                                           \
        TAL_STORE_U64(cur, 3, base);                                           \
        TAL_STORE_U64(cur, 5, pid);                                            \
        TAL_COMMIT(tra, nDWords);                                              \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_MODULEINFO_ID(tra, lvl, cat, lo, hi, pid, moduleName)  \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 4 + 2 + TAL_PRESTORE_STR(moduleName);     \
      TAL_UNUSED(nDWords);                                                     \
      TAL_PUSH_COMMAND(tra, TAL_ANNOTE_MODULEINFO_ID, nDWords, 0, 0);          \
      {                                                                        \
        TAL_SET_CUR(cur, (tra));                                               \
        TAL_STORE_U64(cur, 1, lo);                                             \
        TAL_STORE_U64(cur, 3, hi);                                             \
        TAL_STORE_U64(cur, 5, pid);                                            \
        TAL_STORE_STR(cur, 7, moduleName);                                     \
        TAL_COMMIT(tra, nDWords);                                              \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_MASTERPROCESS_ID(tra, lvl, cat, pid)                   \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 0 + 2;                                    \
      TAL_UNUSED(nDWords);                                                     \
      TAL_PUSH_COMMAND(tra, TAL_ANNOTE_MASTERPROCESS_ID, nDWords, 0, 0);       \
      {                                                                        \
        TAL_SET_CUR(cur, (tra));                                               \
        TAL_STORE_U64(cur, 1, pid);                                            \
        TAL_COMMIT(tra, nDWords);                                              \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_PSEUDO_PROCESS(tra, lvl, cat, real_pid, fake_pid)      \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 4;                                        \
      TAL_UNUSED(nDWords);                                                     \
      TAL_PUSH_COMMAND(tra, TAL_ANNOTE_PSEUDO_PROCESS, nDWords, 0, 0);         \
      {                                                                        \
        TAL_SET_CUR(cur, (tra));                                               \
        TAL_STORE_U64(cur, 1, real_pid);                                       \
        TAL_STORE_U64(cur, 3, fake_pid);                                       \
        TAL_COMMIT(tra, nDWords);                                              \
      }                                                                        \
    }                                                                          \
  }

// FIXME Optimize by removing 2nd stlen
#define TAL_PUSH_ANNOTE_VERSION(tra, lvl, cat, version)                        \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_PUSH_COMMAND(tra, TAL_ANNOTE_VERSION, 0, version, 0);                \
      TAL_COMMIT(tra, 0);                                                      \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_BUILD_VERSION(tra, lvl, cat, talversion,               \
                                      captureversion)                          \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWordsForName1 =                                   \
          TAL_PRESTORE_STR((talversion)->BuildVersion);                        \
      nDWordsForName1;                                                         \
      TAL_CONST TAL_UINT32 nDWordsForName2 =                                   \
          TAL_PRESTORE_STR((captureversion)->BuildVersion);                    \
      nDWordsForName2;                                                         \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords =                                         \
            nDWordsForName1 + 4 + nDWordsForName2 + 4;                         \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_ANNOTE_BUILD_VERSION, nDWords, 0, 0);        \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          int off = 1;                                                         \
          TAL_STORE_STR(cur, off, (talversion)->BuildVersion);                 \
          TAL_STORE_U32(cur, off += nDWordsForName1, (talversion)->ApiMajor);  \
          TAL_STORE_U32(cur, off += 1, (talversion)->ApiMinor);                \
          TAL_STORE_U32(cur, off += 1, (talversion)->FileMajor);               \
          TAL_STORE_U32(cur, off += 1, (talversion)->FileMinor);               \
          TAL_STORE_STR(cur, off += 1, (captureversion)->BuildVersion);        \
          TAL_STORE_U32(cur, off += nDWordsForName2,                           \
                        (captureversion)->ApiMajor);                           \
          TAL_STORE_U32(cur, off += 1, (captureversion)->ApiMinor);            \
          TAL_STORE_U32(cur, off += 1, (captureversion)->FileMajor);           \
          TAL_STORE_U32(cur, off += 1, (captureversion)->FileMinor);           \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_TASK_COLOR(tra, lvl, cat, func, color)                 \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 1;                                    \
      TAL_UNUSED(nDWords);                                                     \
      TAL_PUSH_COMMAND(tra, TAL_ANNOTE_TASK_COLOR, nDWords, 0, 0);             \
      {                                                                        \
        TAL_SET_CUR(cur, (tra));                                               \
        TAL_STORE_U64(cur, 1, (TAL_UINT64)func);                               \
        TAL_STORE_U32(cur, 3, color);                                          \
        TAL_COMMIT(tra, nDWords);                                              \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_NAMED_TASK_COLOR(tra, lvl, cat, name, color)           \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWordsForName = TAL_PRESTORE_STR(name);            \
      nDWordsForName;                                                          \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = nDWordsForName + 1;                     \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_ANNOTE_NAMED_TASK_COLOR, nDWords, 0, 0);     \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_STR(cur, 1, name);                                         \
          TAL_STORE_U32(cur, 1 + nDWordsForName, color);                       \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_STRING_HANDLE(tra, lvl, cat, handle, str_val)          \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 1 + TAL_PRESTORE_STR(str_val);            \
      TAL_UNUSED(nDWords);                                                     \
      TAL_PUSH_COMMAND(tra, TAL_ANNOTE_STRING_HANDLE, nDWords, 0, 0);          \
      {                                                                        \
        TAL_SET_CUR(cur, (tra));                                               \
        TAL_STORE_U32(cur, 1, handle);                                         \
        TAL_STORE_STR(cur, 2, str_val);                                        \
        TAL_COMMIT(tra, nDWords);                                              \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ANNOTE_COUNTER_SAMPLE_TYPE(tra, lvl, cat, name, sample_type)  \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWordsForName = TAL_PRESTORE_STR(name);            \
      TAL_UNUSED(nDWordsForName);                                              \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = nDWordsForName + 1;                     \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_ANNOTE_COUNTER_SAMPLE_TYPE, nDWords, 0, 0);  \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_STR(cur, 1, name);                                         \
          TAL_STORE_U32(cur, 1 + nDWordsForName, (TAL_UINT32)sample_type);     \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_MARKER(tra, lvl, cat, name)                                   \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_UINT64 ts = TAL_GetCurrentTime();                                  \
        TAL_CONST TAL_UINT32 nDWords = 2 + TAL_PRESTORE_STR(name);             \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_NAMED_MARKER, nDWords, 0, 0);         \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_STR(cur, 3, name);                                         \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_MARKER_EX(tra, lvl, cat, name, ts)                            \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + TAL_PRESTORE_STR(name);               \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_NAMED_MARKER, nDWords, 0, 0);         \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_STR(cur, 3, name);                                         \
          TAL_COMMIT(tra, nDWords + 1);                                        \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_EVENT(tra, lvl, cat, name)                                    \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = 2 + TAL_PRESTORE_STR(name);             \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_NAMED_EVENT, nDWords, 0, 0);          \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, TAL_GetCurrentTime());                         \
          TAL_STORE_STR(cur, 3, name);                                         \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_EVENT_EX(tra, lvl, cat, name, ts)                             \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + TAL_PRESTORE_STR(name);               \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_NAMED_EVENT, nDWords, 0, 0);          \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_STR(cur, 3, name);                                         \
          TAL_COMMIT(tra, nDWords + 1);                                        \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_PARAM_I32V(tra, lvl, cat, name, nelems, valptr)               \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_UINT32 i = 0;                                                        \
      TAL_CONST TAL_UINT32 nDWords = TAL_PRESTORE_STR(name) + 1 + nelems;      \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_PARAM_I32V, nDWords, 0, 0);           \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, nelems);                                       \
          for (i = 0; i < nelems; ++i) {                                       \
            TAL_STORE_U32(cur, 2 + i, (valptr)[i]);                            \
          }                                                                    \
          TAL_STORE_STR(cur, 2 + nelems, name);                                \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_PARAM_I64V(tra, lvl, cat, name, nelems, valptr)               \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_UINT32 i = 0;                                                        \
      TAL_CONST TAL_UINT32 nDWords =                                           \
          TAL_PRESTORE_STR(name) + 1 + (2 * nelems);                           \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_PARAM_I64V, nDWords, 0, 0);           \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, nelems);                                       \
          for (i = 0; i < nelems; ++i) {                                       \
            TAL_STORE_U64(cur, 2 + (2 * i), (valptr)[i]);                      \
          }                                                                    \
          TAL_STORE_STR(cur, 2 + (2 * nelems), name);                          \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_PARAM_FV(tra, lvl, cat, name, nelems, valptr)                 \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_UINT32 i = 0;                                                        \
      TAL_CONST TAL_UINT32 nDWords = TAL_PRESTORE_STR(name) + 1 + nelems;      \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_PARAM_FV, nDWords, 0, 0);             \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, nelems);                                       \
          for (i = 0; i < nelems; ++i) {                                       \
            TAL_STORE_FLT(cur, 2 + i, (valptr)[i]);                            \
          }                                                                    \
          TAL_STORE_STR(cur, 2 + nelems, name);                                \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_PARAM_S(tra, lvl, cat, name, val)                             \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords_Name = TAL_PRESTORE_STR(name);              \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = nDWords_Name + TAL_PRESTORE_STR(val);   \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_PARAM_S, nDWords, 0, 0);              \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_STR(cur, 1, name);                                         \
          TAL_STORE_STR(cur, 1 + nDWords_Name, val);                           \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_PARAM_I32V_H(tra, lvl, cat, name, nelems, valptr)             \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_UINT32 i = 0;                                                        \
      TAL_CONST TAL_UINT32 nDWords = 1 + 1 + nelems;                           \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_PARAM_I32V_H, nDWords, 0, 0);         \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, nelems);                                       \
          for (i = 0; i < nelems; ++i) {                                       \
            TAL_STORE_U32(cur, 2 + i, (valptr)[i]);                            \
          }                                                                    \
          TAL_STORE_U32(cur, 2 + nelems, name);                                \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_PARAM_I64V_H(tra, lvl, cat, name, nelems, valptr)             \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_UINT32 i = 0;                                                        \
      TAL_CONST TAL_UINT32 nDWords = 1 + 1 + (2 * nelems);                     \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_PARAM_I64V_H, nDWords, 0, 0);         \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, nelems);                                       \
          for (i = 0; i < nelems; ++i) {                                       \
            TAL_STORE_U64(cur, 2 + (2 * i), (valptr)[i]);                      \
          }                                                                    \
          TAL_STORE_U32(cur, 2 + (2 * nelems), name);                          \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_PARAM_FV_H(tra, lvl, cat, name, nelems, valptr)               \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_UINT32 i = 0;                                                        \
      TAL_CONST TAL_UINT32 nDWords = 1 + 1 + nelems;                           \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_PARAM_FV_H, nDWords, 0, 0);           \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, nelems);                                       \
          for (i = 0; i < nelems; ++i) {                                       \
            TAL_STORE_FLT(cur, 2 + i, (valptr)[i]);                            \
          }                                                                    \
          TAL_STORE_U32(cur, 2 + nelems, name);                                \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_PARAM_S_H(tra, lvl, cat, name, val)                           \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords_Name = 1;                                   \
      {                                                                        \
        TAL_CONST TAL_UINT32 nDWords = nDWords_Name + TAL_PRESTORE_STR(val);   \
        TAL_UNUSED(nDWords);                                                   \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_PARAM_S_H, nDWords, 0, 0);            \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, name);                                         \
          TAL_STORE_STR(cur, 1 + nDWords_Name, val);                           \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ADD_COUNTER(tra, lvl, cat, name, val)                         \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + TAL_PRESTORE_STR(name);               \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_ADD_COUNTER, nDWords, 0, 0);          \
        TAL_UNUSED(nDWords);                                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_STR(cur, 1, name);                                         \
          TAL_STORE_U64(cur, nDWords - 1, (TAL_UINT64)val);                    \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_ADD_COUNTER_H(tra, lvl, cat, name, val)                       \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 1;                                    \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_ADD_COUNTER_H, nDWords, 0, 0);        \
        TAL_UNUSED(nDWords);                                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, name);                                         \
          TAL_STORE_U64(cur, 1 + 1, (TAL_UINT64)val);                          \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_SUB_COUNTER(tra, lvl, cat, name, val)                         \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + TAL_PRESTORE_STR(name);               \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_SUB_COUNTER, nDWords, 0, 0);          \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_STR(cur, 1, name);                                         \
          TAL_STORE_U64(cur, nDWords - 1, (TAL_UINT64)val);                    \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_SUB_COUNTER_H(tra, lvl, cat, name, val)                       \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 1;                                    \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_SUB_COUNTER, nDWords, 0, 0);          \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, name);                                         \
          TAL_STORE_U64(cur, 1 + 1, (TAL_UINT64)val);                          \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_SAMPLE_COUNTER(tra, lvl, cat, name, val)                      \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 2 + TAL_PRESTORE_STR(name);           \
      {                                                                        \
        TAL_UINT64 ts = TAL_GetCurrentTime();                                  \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_SAMPLE_COUNTER, nDWords, 0, 0);       \
        TAL_UNUSED(nDWords);                                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_STR(cur, 3, name);                                         \
          TAL_STORE_U64(cur, nDWords - 1, (TAL_UINT64)val);                    \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_SAMPLE_COUNTER_H(tra, lvl, cat, name, val)                    \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 2 + 1;                                \
      {                                                                        \
        TAL_UINT64 ts = TAL_GetCurrentTime();                                  \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_SAMPLE_COUNTER_H, nDWords, 0, 0);     \
        TAL_UNUSED(nDWords);                                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_U32(cur, 3, name);                                         \
          TAL_STORE_U64(cur, 3 + 1, (TAL_UINT64)val);                          \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_SAMPLE_COUNTER_EX(tra, lvl, cat, name, val, ts)               \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 2 + TAL_PRESTORE_STR(name);           \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_SAMPLE_COUNTER, nDWords, 0, 0);       \
        TAL_UNUSED(nDWords);                                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_STR(cur, 3, name);                                         \
          TAL_STORE_U64(cur, nDWords - 1, (TAL_UINT64)val);                    \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_SAMPLE_COUNTER_EX_H(tra, lvl, cat, name, val, ts)             \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 2 + 2 + 1;                                \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_SAMPLE_COUNTER_H, nDWords, 0, 0);     \
        TAL_UNUSED(nDWords);                                                   \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ts);                                           \
          TAL_STORE_U32(cur, 3, name);                                         \
          TAL_STORE_U64(cur, 3 + 1, (TAL_UINT64)val);                          \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_STACK_TRACEV(tra, lvl, cat, nelems, valptr)                   \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 1 + (2 * nelems);                         \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_STACK_TRACEV, nDWords, 0, 0);         \
        {                                                                      \
          TAL_UINT32 i = 0;                                                    \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U32(cur, 1, nelems);                                       \
          for (i = 0; i < nelems; ++i) {                                       \
            TAL_STORE_U64(cur, 2 + (2 * i), (valptr)[i]);                      \
          }                                                                    \
          TAL_COMMIT(tra, nDWords);                                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define TAL_PUSH_PMU_COUNTERS(tra, lvl, cat, ir, llc_references, llc_misses,   \
                              unhalted_core_cycles)                            \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_CONST TAL_UINT32 nDWords = 8;                                        \
      TAL_UNUSED(nDWords);                                                     \
      {                                                                        \
        TAL_PUSH_COMMAND(tra, TAL_OPCODE_PMU_COUNTERS, nDWords, 0, 0);         \
        {                                                                      \
          TAL_SET_CUR(cur, (tra));                                             \
          TAL_STORE_U64(cur, 1, ir);                                           \
          TAL_STORE_U64(cur, 3, llc_references);                               \
          TAL_STORE_U64(cur, 5, llc_misses);                                   \
          TAL_STORE_U64(cur, 7, unhalted_core_cycles);                         \
        }                                                                      \
        TAL_COMMIT(tra, nDWords);                                              \
      }                                                                        \
    }                                                                          \
  }

#ifdef TAL_DISABLE

#define TAL_ReadHardwareCounter32(ctr) 0xFFFFFFFF

#define TAL_PUSH_SAMPLE_HW_COUNTERS_32x4(tra, lvl, cat)                        \
  {                                                                            \
    TAL_UNUSED(tra);                                                           \
    TAL_UNUSED(lvl);                                                           \
    TAL_UNUSED(cat);                                                           \
  }

#else

#ifndef TAL_DOXYGEN
TAL_INLINE TAL_UINT32 TAL_ReadHardwareCounter32(int counter) {
#if TAL_PLATFORM == TAL_PLATFORM_LARRYSIM
#warning not supported
  counter;
  return 0xFFFFFFFF;
#elif TAL_PLATFORM == TAL_PLATFORM_WINDOWS
  counter;
  return 0xFFFFFFFF;
#elif TAL_PLATFORM == TAL_PLATFORM_NIX
#if defined(TAL_FEATURE_LARRABEE)
  TAL_UINT32 result;
  __asm {
      mov ecx, counter;
      rdpmc;
      mov result, eax;
  }
  return result;
#else  // !defined(TAL_FEATURE_LARRABEE)
  (void)counter;
  return 0xFFFFFFFF;
#endif // !defined(TAL_FEATURE_LARRABEE)
#endif // TAL_PLATFORM
}
#endif // ndef TAL_DOXYGEN

#define TAL_PUSH_SAMPLE_HW_COUNTERS_32x4(tra, lvl, cat)                        \
  {                                                                            \
    if (TAL_IS_LOGGABLE(tra, lvl, cat)) {                                      \
      TAL_UINT32 c;                                                            \
      TAL_UINT64 ts = TAL_GetCurrentTime();                                    \
      TAL_UNUSED(ts);                                                          \
      if ((tra)->hwPerfCounterHandles[0] != 0xFFFFFFFF) {                      \
        c = TAL_ReadHardwareCounter32(0);                                      \
        TAL_PUSH_SAMPLE_COUNTER_EX_H(tra, lvl, cat,                            \
                                     (tra)->hwPerfCounterHandles[0], c, ts);   \
      }                                                                        \
      if ((tra)->hwPerfCounterHandles[1] != 0xFFFFFFFF) {                      \
        c = TAL_ReadHardwareCounter32(1);                                      \
        TAL_PUSH_SAMPLE_COUNTER_EX_H(tra, lvl, cat,                            \
                                     (tra)->hwPerfCounterHandles[1], c, ts);   \
      }                                                                        \
      if ((tra)->hwPerfCounterHandles[2] != 0xFFFFFFFF) {                      \
        c = TAL_ReadHardwareCounter32(2);                                      \
        TAL_PUSH_SAMPLE_COUNTER_EX_H(tra, lvl, cat,                            \
                                     (tra)->hwPerfCounterHandles[2], c, ts);   \
      }                                                                        \
      if ((tra)->hwPerfCounterHandles[3] != 0xFFFFFFFF) {                      \
        c = TAL_ReadHardwareCounter32(3);                                      \
        TAL_PUSH_SAMPLE_COUNTER_EX_H(tra, lvl, cat,                            \
                                     (tra)->hwPerfCounterHandles[3], c, ts);   \
      }                                                                        \
    }                                                                          \
  }

#endif // TAL_DISABLE

#if 0 // LEW_IPC
inline  getCurrentClientContextSwitch(IgpaIPCClient* ctxClient, (TAL_UINT32*) &pCtxBuffer, (TAL_UINT32*) &pCtxBufferEnd)
{
  if(NULL == pCtxBuffer)
  {
    pIPCBlockData = pIPCClientContextSwitch->GetBlockForWriting();
    pIPCBuffer = (TAL_UINT32*)pIPCBlockData;
    pIPCBufferEnd = &pIPCBuffer[pIPCClientContextSwitch->GetBlockSize() / 4];


    if(pCtxBuffer)
    {
      *pIPCBuffer++ = TAL_ESCAPE_COMMAND(TAL_ESCAPE_OPCODE_CONTEXT_SWITCH_DATA,0);
      *pIPCBuffer++ = 0; // will be replaced by payload size
      *pIPCBuffer++ = TAL_COMMAND(TAL_ANNOTE_TRACE_BUFFER_FLUSH_TIME2, 4, 0, 0);
      *(TAL_UINT64*)(pIPCBuffer) = 0; // will be replaced @ flush
      pIPCBuffer += 2;
      *(TAL_UINT64*)(pIPCBuffer) = talGetPID();
      pIPCBuffer += 2;
    }
  }
}
      if(pIPCBuffer)
      {
            TAL_UINT32* pTids = (TAL_UINT32*) pEvent->UserData;

        TAL_STORE_U32(pIPCBuffer, 0, TAL_COMMAND(TAL_OPCODE_CONTEXT_SWITCH_V2, (nDwords-1), 0, 0));  // Opcode
        TAL_STORE_U64(pIPCBuffer, 1, (TAL_UINT64) pEvent->EventHeader.TimeStamp.QuadPart);  // Timestamp
        //TAL_STORE_U64(pCur, 1, TAL_GetCurrentTime());  // Timestamp
        TAL_STORE_U32(pIPCBuffer, 3, pTids[0]);   // new tid
        TAL_STORE_U32(pIPCBuffer, 4, (TAL_UINT32) pEvent->BufferContext.ProcessorNumber); // cpu

        pIPCBuffer += nDwords;

        // if next one won't fit then full, so return the block ("flush")
        if(pIPCBuffer + nDwords > pIPCBufferEnd)
        {
          int bufSizeBytes = 4 * (int)(pIPCBuffer-(TAL_UINT32*)pIPCBlockData);

          pIPCBuffer = (TAL_UINT32*)pIPCBlockData;    // <-- @ escape
          pIPCBuffer++;                  // <-- @ payload size
          *pIPCBuffer++ = bufSizeBytes - 8;        // <-- @ 
          pIPCBuffer++;                  // <-- @ time size, skipped
              *(TAL_UINT64*)(pIPCBuffer) = TAL_GetCurrentTime(); // replace flush time with correct value

          pIPCClientContextSwitch->ReturnBlock(pIPCBlockData, bufSizeBytes);
          pIPCBuffer = NULL;
        }
      }
#endif LEW_IPC

#endif // !defined(TAL_MACROS_H)

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
