/* ************************************************************************* *\
                INTEL CORPORATION PROPRIETARY INFORMATION
     This software is supplied under the terms of a license agreement or 
     nondisclosure agreement with Intel Corporation and may not be copied 
     or disclosed except in accordance with the terms of that agreement. 
        Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef COIRESULT_COMMON_H
#define COIRESULT_COMMON_H
/** @ingroup COIResult
 *  @addtogroup COIResultCommon
@{

* @file common/COIResult_common.h 
* Result codes and definitions. */
#ifdef __cplusplus
extern "C" {
#endif 

typedef enum COIRESULT
{
    COI_SUCCESS = 0,                  ///< The function succeeded without error.
    COI_ERROR,                        ///< Unspecified error.
    COI_NOT_INITIALIZED,              ///< The function was called before the 
                                      ///< system was initialized.
    COI_ALREADY_INITIALIZED,          ///< The function was called after the 
                                      ///< system was initialized.
    COI_ALREADY_EXISTS,               ///< Cannot complete the request due to
                                      ///< the existence of a similar object.
    COI_DOES_NOT_EXIST,               ///< The specified object was not found.
    COI_INVALID_POINTER,              ///< One of the provided addresses was not
                                      ///< valid.
    COI_OUT_OF_RANGE,                 ///< One of the arguments contains a value
                                      ///< that is invalid.
    COI_NOT_SUPPORTED,                ///< This function is not currently 
                                      ///< supported as used.
    COI_TIME_OUT_REACHED,             ///< The specified time out caused the 
                                      ///< function to abort.
    COI_DUPLICATE_OBJECT,             ///< All objects must be unique.
    COI_ARGUMENT_MISMATCH,            ///< The specified arguments are not 
                                      ///< compatible.
    COI_SIZE_MISMATCH,                ///< The specified size does not match the
                                      ///< expected size.
    COI_OUT_OF_MEMORY,                ///< The function was unable to allocate 
                                      ///< the required memory.
    COI_INVALID_HANDLE,               ///< One of the provided handles was not 
                                      ///< valid.
    COI_RETRY,                        ///< This function currently can't 
                                      ///< complete, but might be able to later.
    COI_RESOURCE_EXHAUSTED,           ///< The resource was not large enough.
    COI_ALREADY_LOCKED,               ///< The object was expected to be 
                                      ///< unlocked, but was locked.
    COI_NOT_LOCKED,                   ///< The object was expected to be locked,
                                      ///< but was unlocked.
    COI_MISSING_DEPENDENCY,           ///< One or more dependent components
                                      ///< could not be found.
    COI_UNDEFINED_SYMBOL,             ///< One or more symbols the component 
                                      ///< required was not defined in any 
                                      ///< library.
    COI_PENDING,                      ///< Operation is not finished
    COI_BINARY_AND_HARDWARE_MISMATCH, ///< A specified binary will not run on
                                      ///< the specified hardware.
    COI_PROCESS_DIED,
    COI_INVALID_FILE,                 ///< The file is invalid for its intended
                                      ///< usage in the function.
    COI_BARRIER_CANCELED,             ///< Barrier wait on a user barrier that
                                      ///< was unregistered or is being
                                      ///< unregistered returns 
                                      ///< COI_BARRIER_CANCELED.

    COI_NUM_RESULTS                   ///< Reserved, do not use.
}
COIRESULT;

//////////////////////////////////////////////////////////////////////////////
///
/// Returns the string version of the passed in COIRESULT. Thus if
/// COI_RETRY is passed in, this function returns the string "COI_RETRY". If
/// the error code passed ins is not valid then "COI_ERROR" will be returned.
///
/// @param in_ResultCode
///        [in] COIRESULT code to return the string version of.
///
/// @return String version of the passed in COIRESULT code.
///
const char* 
COIResultGetName(
            COIRESULT       in_ResultCode);

#ifdef __cplusplus
}
#endif 

#endif /* COIRESULT_COMMON_H */

/*! @} */
