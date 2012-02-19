// Copyright (c) 2011 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////
//  conversion_rules.h
//  Rules for image format conversions.
//  Original author: shohaml
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Intel { namespace OpenCL { namespace Framework {
	/**
	 * Convert RGBA floating point color to image format requested.
	 *
	 * @param color RGBA float format
	 * @param channelOrder target channel order
	 * @param channelType target channel type
	 * @param trgtColor allocated space for target color
	 * @param trgtLength size (bytes) of target color
	 *
	 * @return
	 * - CL_SUCCESS if OK
	 * - CL_IMAGE_FORMAT_NOT_SUPPORTED if cannot convert to the specified format.
	 * - CL_INVALID_ARG_SIZE if trgtLength is inappropriate for channel type.
	 * 	@
	 */
	cl_int norm_float_to_image(
			const cl_float4 *color,
			const cl_channel_order channelOrder,
			const cl_channel_type channelType,
			void* trgtColor,
			const size_t trgtLength
			);


	/**
	 * Convert non normalized signed int point color to image format requested.
	 *
	 * @param color non normalized signed int.
	 * @param channelOrder target channel order
	 * @param channelType target channel type
	 * @param trgtColor allocated space for target color
	 * @param trgtLength size (bytes) of target color
	 *
	 * @return
	 * - CL_SUCCESS if OK
	 * - CL_IMAGE_FORMAT_NOT_SUPPORTED if cannot convert to the specified format.
	 * - CL_INVALID_ARG_SIZE if trgtLength is inappropriate for channel type.
	 * 	@
	 */
	cl_int non_norm_signed_to_image(
			const cl_int4 *color,
			const cl_channel_order channelOrder,
			const cl_channel_type channelType,
			void* trgtColor,
			const size_t trgtLength
			);


	/**
	 * Convert non normalized unsigned int point color to image format requested.
	 *
	 * @param color non normalized unsigned int.
	 * @param channelOrder target channel order
	 * @param channelType target channel type
	 * @param trgtColor allocated space for target color
	 * @param trgtLength size (bytes) of target color
	 *
	 * @return
	 * - CL_SUCCESS if OK
	 * - CL_IMAGE_FORMAT_NOT_SUPPORTED if cannot convert to the specified format.
	 * - CL_INVALID_ARG_SIZE if trgtLength is inappropriate for channel type.
	 * 	@
	 */
	cl_int non_norm_unsigned_to_image(
			const cl_uint4 *color,
			const cl_channel_order channelOrder,
            const cl_channel_type channelType,
            void* trgtColor,
            const size_t trgtLength
            );
}}}


