/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
          Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */

#ifndef _COIPROCESS_SINK_H
#define _COIPROCESS_SINK_H
/** @ingroup COIProcess
 *  @addtogroup COIProcessSink
@{
* @file sink/COIProcess_sink.h
*/
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <common/COITypes_common.h>
#include <common/COIResult_common.h>

#ifdef __cplusplus
extern "C" {
#endif 
#endif // DOXYGEN_SHOULD_SKIP_THIS

//////////////////////////////////////////////////////////////////////////////
///
/// This call will block while waiting for the source to send a process
/// destroy message. This provides the sink side application with a barrier
/// to keep the main() function from exiting until it is directed to by the
/// source. When the shutdown message is received this function will stop
/// any future run functions from executing but will wait for any current
/// run functions to complete. All COI resources will be cleaned up and no
/// additional COI APIs should be called after this function returns.
/// This function does not invoke exit() so the application can perform any
/// of its own cleanup once this call returns.
///
/// @return COI_SUCCESS once the process receives the shutdown message.
///
COIRESULT
COIProcessWaitForShutdown();

#ifdef __cplusplus
} // extern "C"
#endif
#endif //_COIPROCESS_SINK_H

/*! @} */
