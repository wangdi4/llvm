/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  cl_device_api.h

\*****************************************************************************/

/*! \file
 *
 *   \brief The interface between the device agent and the OpenCL runtime
 *   \author Evgeny Fiksman evgeny.fiksman@intel.com
 *
 *   \version 1.1
 *   \date November 2010
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "CL/cl.h"

#define IN
#define OUT
#define INOUT

/*! \def MAX_WORK_DIM
    \brief The maximum working dimension size.

    Memory Objects and Kernels should use this define to go over working dimensions.
*/
#define MAX_WORK_DIM		3

//!< ------------------------------------------------------------------------------
//!< Program container definition.
//!< Data types and structures
//!< ------------------------------------------------------------------------------

/*! \enum cl_prog_binary_type
 * Defines possible values for IR/binary types supported by the program container
 */
enum cl_prog_binary_type
{
	CL_PROG_BIN_DASHER,				//!< Containers holds Intel DASHER intermediate
	CL_PROG_BIN_COMPILED_LLVM,		//!< Container holds compiled Apple OCL LLVM intermediate
    CL_PROG_BIN_LINKED_LLVM,        //!< Container holds linked Apple OCL LLVM intermediate
    CL_PROG_BIN_EXECUTABLE_LLVM,    //!< Container holds executable Apple OCL LLVM intermediate
	CL_PROG_DLL_X86,				//!< Container is a dynamically loaded library name
	CL_PROG_OBJ_X86,				//!< Container holds x86 object code (.obj)
	CL_PROG_BIN_X86,				//!< Container holds x86 binary code
	CL_PROG_BIN_PTX,				//!< Container holds NVidia PTX intermediate
	CL_PROG_BIN_CUBIN				//!< Container holds NVidia CUBbinary
};

/*! \enum cl_prog_container_type
 * Defines possible values for containers types supported by the program container
 */
enum cl_prog_container_type
{
	CL_PROG_CNT_PRIVATE,			//!< Containers is private container and its structure is unknown
	CL_PROG_CNT_PROGRAM				//!< Container holds OCL program container as defined by cl_prog_program
};

/*! \struct _cl_work_description_type
 * Defines the OCL work description (dimensions and offsets of the groups)
 */
typedef struct _cl_work_description_type
{
	unsigned int workDimension;
	size_t globalWorkOffset[MAX_WORK_DIM];
	size_t globalWorkSize[MAX_WORK_DIM];
	size_t localWorkSize[MAX_WORK_DIM];
} cl_work_description_type;

/*! \enum cl_kernel_arg_type
 * Defines possible values for kernel argument types
 */
enum cl_kernel_arg_type
{
	CL_KRNL_ARG_INT		= 0,	//!< Argument is a signed integer.
	CL_KRNL_ARG_UINT,			//!< Argument is an unsigned integer.
	CL_KRNL_ARG_FLOAT,			//!< Argument is a float.
	CL_KRNL_ARG_DOUBLE,			//!< Argument is a double.
	CL_KRNL_ARG_VECTOR,			//!< Argument is a vector of basic types, like int8, float4, etc.
	CL_KRNL_ARG_SAMPLER,		//!< Argument is a sampler object
	CL_KRNL_ARG_COMPOSITE,		//!< Argument is a user defined struct
	CL_KRNL_ARG_PTR_LOCAL,		//!< Argument is a pointer to array declared in local memory
								//!< Memory object types bellow this line
	CL_KRNL_ARG_PTR_GLOBAL,		//!< Argument is a pointer to array in global memory of various types
								//!< The array type could be char, short, int, float or double
								//!< User must pass a handle to a memory buffer for this argument type
	CL_KRNL_ARG_PTR_CONST,		//!< Argument is a pointer to buffer declared in constant(global) memory
	CL_KRNL_ARG_PTR_IMG_2D,		//!< Argument is a pointer to 2D image
	CL_KRNL_ARG_PTR_IMG_3D,		//!< Argument is a pointer to 3D image
    CL_KRNL_ARG_PTR_IMG_2D_ARR  //!< Argument is a pointer to a 2D image array
};

/*! \struct cl_kernel_argument
 *  \brief Defines possible values for kernel argument types.
 */
struct cl_kernel_argument
{
	cl_kernel_arg_type			type;			//!< Type of the argument.
	unsigned int			size_in_bytes;		//!< Size of the argument in bytes
												//!< for pointer types the size is 0
};

/*! \struct cl_prog_binary_desc
 *  \brief This structure defines a container description of binary
 */
struct cl_prog_binary_desc
{
	cl_prog_binary_type	bin_type;		//!< Binary/IR type that is held by a container
	cl_ushort			bin_ver_major;	//!< Binary/IR major version identifier
	cl_ushort			bin_ver_minor;	//!< Binary/IR minor version identifier
};

/*! \struct cl_prog_kernel
 *  \brief This structure defines a single kernel container.
 */
struct cl_prog_kernel
{
	const char*					name;			//!< A pointer to kernel name
	int							arg_count;		//!< A number of arguments to the kernel
	const cl_kernel_argument*	args;			//!< An array of kernel argument
	size_t						metadata_size;	//!< Size in bytes of the metadata generated by the back-end compiler.
	const void*					metadata;		//!< A pointer to buffer that holds metadata generated by the back-end compiler
	size_t						imp_local_size;	//!< Size in bytes of the implicitly defined local memory buffers
	size_t						bin_size;		//!< Size in bytes of kernel binary/IR representation
												//!< If 0, bin points to code segment
	const void*					bin;			//!< A pointer to kernel binary representation
};

/*! \struct cl_prog_program
 *  \brief This structure defines a specific container for binaries or IR of OCL programs
 */
struct cl_prog_program
{
	cl_uint			kernel_count;			//!< Number of kernels in a program
	cl_prog_kernel*	kernels;				//!< A pointer to list of cl_prog_kernel
};

/*! \def _CL_CONTAINER_MASK_
    \brief This structure defines a container that holds binaries or IR of OCL compiled programs
*/
#define _CL_CONTAINER_MASK_		"CLPC"

/*! \struct cl_prog_container_header
 *  \brief This structure defines a specific container for binaries or IR of OCL programs
 */
typedef struct _cl_prog_container_header
{
	cl_char					mask[4];		//!< A container identifier mask must be "CLPC"
	cl_prog_binary_desc		description;	//!< Binary/IR description that is held by a container
	cl_prog_container_type	container_type;	//!< Type of container that stores program binary, as defined by cl_prog_container_type
	size_t					container_size; //!< Size in bytes of the container data
} cl_prog_container_header;

// Interface declaration
class IOCLFrameworkCallbacks;
class IOCLDevLogDescriptor;
class IOCLDeviceAgent;
class IOCLDevMemoryObject;

// ------------------------------------------------------------------------------
// Device API data types definition
// ------------------------------------------------------------------------------

/*! \typedef cl_dev_cmd_list
 * Command list handle definition
 */
typedef void*	cl_dev_cmd_list;

/*! \typedef cl_dev_cmd_id
 * Device command id
 */
typedef void*	cl_dev_cmd_id;

/*! \typedef cl_dev_program
 * Program handle definition
 */
typedef void* cl_dev_program;

/*! \typedef cl_dev_kernel
 * Kernel handle definition
 */
typedef void* cl_dev_kernel;

/*! \typedef cl_dev_subdevice_id
 * Sub-device ID definition
 */
typedef void* cl_dev_subdevice_id;

/*! \typedef cl_dev_memobj_handle
 * Device object handle definition
 */
typedef void* cl_dev_memobj_handle;

/*! \enum cl_dev_err_code
 * Defines device return values that are used by the OCL framework.
 */
enum cl_dev_err_code
{
	CL_DEV_SUCCESS				= 0,			//!< Function call or query call succeeded
	CL_DEV_BUSY,								//!< Device is busy during function execution
	CL_DEV_ERROR_FAIL			= 0x80000000,	//!< Internal unspecified error
	CL_DEV_INVALID_VALUE,						//!< Invalid value was passed to the function.
	CL_DEV_INVALID_PROPERTIES,					//!< Properties might be valid but not supported
	CL_DEV_OUT_OF_MEMORY,						//!< Resource allocation failure
	CL_DEV_INVALID_COMMAND_LIST,				//!< Invalid command list handle
	CL_DEV_INVALID_COMMAND_TYPE,				//!< Invalid command type
	CL_DEV_INVALID_COMMAND_PARAM,				//!< Invalid command parameter
	CL_DEV_INVALID_MEM_OBJECT,					//!< Invalid memory object
	CL_DEV_INVALID_KERNEL,						//!< Invalid kernel identifier
	CL_DEV_INVALID_OPERATION,					//!< Device cannot perform requested operation
	CL_DEV_INVALID_WRK_DIM,						//!< Invalid work dimension (i.e. a value between 1 and 3)
	CL_DEV_INVALID_WG_SIZE,						//!< Invalid work-group size
	CL_DEV_INVALID_GLB_OFFSET,					//!< Invalid global offset, only (0, 0, 0) is supported
	CL_DEV_INVALID_WRK_ITEM_SIZE,				//!< Invalid work-item size
	CL_DEV_INVALID_IMG_FORMAT,					//!< Invalid image format
	CL_DEV_INVALID_IMG_SIZE,					//!< Width or height of the image are 0 or exceed maximum possible values
	CL_DEV_OBJECT_ALLOC_FAIL,					//!< Failed to allocate resources for memory object
	CL_DEV_INVALID_BINARY,						//!< The binary is not supported by the device or program container content is invalid
	CL_DEV_INVALID_BUILD_OPTIONS,				//!< One of more build options specified for the back-end compiler are invalid
	CL_DEV_INVALID_PROGRAM,						//!< Invalid program object handle
	CL_DEV_BUILD_IN_PROGRESS,					//!< Back-end compiler is still in operation
	CL_DEV_BUILD_ALREADY_COMPLETE,				//!< Back-end compiler previously compiled this program
	CL_DEV_BUILD_ERROR,							//!< Error occurred during back-end build process
	CL_DEV_INVALID_KERNEL_NAME,					//!< Kernel name is not found in the program
	CL_DEV_OBJECT_ALREADY_LOCKED,				//!< Memory object is already locked
	CL_DEV_INVALID_OPERATION_MODE,				//!< Invalid operation mode
	CL_DEV_NOT_SUPPORTED						//!< The operation is not supported by the device
};

/**
* CL_FAILED
* Checks whether a return code is failure
*/
#define CL_DEV_FAILED(code)				((int)CL_DEV_SUCCESS > (int)(code))
/**
* CL_DEV_SUCCEEDED
* Checks whether a return code is succeeded
*/
#define CL_DEV_SUCCEEDED(code)			((int)CL_DEV_SUCCESS <= (int)(code))

/*! \enum cl_dev_cmd_list_props
 * Defines possible values of device list properties
 */
enum cl_dev_cmd_list_props
{
	CL_DEV_LIST_NONE		=	0,		//!< Determines a list wherein all items will be executed sequentially.
	CL_DEV_LIST_ENABLE_OOO	=	1,		//!< Determines whether the out-of-order optimization could be applied on items in the command list
	CL_DEV_LIST_SUBDEVICE	=	2,		//!< Determines whether the command list executes on a root-level device or on a sub-device
	CL_DEV_LIST_IN_PLACE    =   4       //!< Determines whether the command list is executed using the calling thread
};

/*! \enum cl_dev_cmd_type
 * Defines possible values of device command identifiers that is passed within cl_dev_cmd_desc structure.
 */
enum cl_dev_cmd_type
{
	CL_DEV_CMD_READ = 0,			//!< Read buffer command
	CL_DEV_CMD_WRITE,				//!< Write buffer command
	CL_DEV_CMD_COPY,				//!< Copy buffer command
	CL_DEV_CMD_MAP,					//!< Map Command
	CL_DEV_CMD_UNMAP,				//!< UnMap command
	CL_DEV_CMD_EXEC_KERNEL,			//!< Execute Kernel Command
	CL_DEV_CMD_EXEC_TASK,			//!< Execute task command
	CL_DEV_CMD_EXEC_NATIVE,			//!< Execute native kernel command
    CL_DEV_CMD_FILL_BUFFER,         //!< Fill buffer
    CL_DEV_CMD_FILL_IMAGE,          //!< Fill image
    //--------------------
	CL_DEV_CMD_MAX_COMMAND_TYPE
};

/*! \enum cl_dev_map_flags
 * Defines possible values of mapped region of a memory object.
 */
/*! \enum cl_dev_binary_prop
 * Defines possible values of binary that is passed to clDevBuildProgram function.
 */
enum cl_dev_binary_prop
{
	CL_DEV_BINARY_COMPILER = 1,		//!< The origin of binary is one of the Front-End compilers
	CL_DEV_BINARY_USER				//!< The origin of binary is a binary loaded by user
};

/*! \enum cl_dev_kernel_info
 * Defines possible values of the kernel information that could be retrieved by clDevGetKernelInfo function.
 */
enum cl_dev_kernel_info
{
	CL_DEV_KERNEL_NAME					= 1,	//!< Specifies NULL terminated function name.
	CL_DEV_KERNEL_PROTOTYPE,					//!< Specifies list of kernel arguments (prototype)
												//!< as defined by cl_kernel_arg_type.
	CL_DEV_KERNEL_MAX_WG_SIZE,					//!< Returns the maximum work-group size that can be used to execute
												//!< a kernel. The device agent uses resource requirements
												//!< of the kernel to determine optimal work-group size.
	CL_DEV_KERNEL_WG_SIZE,						//!< Returns the work-group size that can be used to execute
												//!< a kernel. The device agent uses resource requirements
												//!< of the kernel to determine optimal work-group size.
	CL_DEV_KERNEL_WG_SIZE_REQUIRED,				//!< Specifies the required work-group size as was declared during
												//!< kernel compilation.
	CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE,			//!< Specifies size of implicit local memory buffers defined in kernel.
	CL_DEV_KERNEL_PRIVATE_SIZE					//!< Specifies size of private memory required for
												//!< execution of singe instance of a kernel
};

/*! \enum cl_dev_partition_prop
* Defines a list of possible device partitioning properties. Reflects the list in the Device Fission extension. Not all are supported
*/
enum cl_dev_partition_prop
{
    CL_DEV_PARTITION_EQUALLY = 1,
    CL_DEV_PARTITION_BY_COUNTS,
    CL_DEV_PARTITION_BY_NAMES,
    CL_DEV_PARTITION_AFFINITY_L1,
    CL_DEV_PARTITION_AFFINITY_L2,
    CL_DEV_PARTITION_AFFINITY_L3,
    CL_DEV_PARTITION_AFFINITY_L4,
    CL_DEV_PARTITION_AFFINITY_NUMA,
    CL_DEV_PARTITION_AFFINITY_NEXT
};

/*! \enum cl_dev_bs_flags
* Defines a list of possible when querying Runtime Memory object backing store
*/
enum cl_dev_bs_flags
{
	CL_DEV_BS_GET_IF_AVAILABLE = 1,			//! Return Backing Store only if available,
											//! otherwise returns NULL
	CL_DEV_BS_GET_ALWAYS					//! Returns Backing Store in any case,
											//! if not available runtime shall allocate one
};
// ------------------------------------------------------------------------------
// Device API data structure definition
// ------------------------------------------------------------------------------

// sharing group ids for either buffers or images
enum cl_dev_buffer_sharing_group_id
{
    CL_DEV_CPU_BUFFER_SHARING_GROUP_ID = 0, //! All devices that may use the same Buffer IOCLDevMemoryObject implementation as CPU does
    CL_DEV_MIC_BUFFER_SHARING_GROUP_ID,     //! All devices that may use the same Buffer IOCLDevMemoryObject implementation as MIC does

    CL_DEV_MAX_BUFFER_SHARING_GROUP_ID      //! Last id
};

enum cl_dev_image_sharing_group_id
{
    CL_DEV_CPU_IMAGE_SHARING_GROUP_ID = 0,  //! All devices that may use the same Image IOCLDevMemoryObject implementation as CPU does
    CL_DEV_MIC_IMAGE_SHARING_GROUP_ID,      //! All devices that may use the same Image IOCLDevMemoryObject implementation as MIC does

    CL_DEV_MAX_IMAGE_SHARING_GROUP_ID       //! Last id
};

/*! \struct cl_dev_alloc_prop
 * \brief Description of device memory allocator properties
 * This structure holds a description of properties supported by device memory allocator.
 * The information should be quired per each object type(Buffer, Image2D, Image3D)
 */
struct cl_dev_alloc_prop
{
	cl_dev_buffer_sharing_group_id bufferSharingGroupId; //!< ID of the sharing group used by the device allocator for buffers
	cl_dev_image_sharing_group_id  imageSharingGroupId;  //!< ID of the sharing group used by the device allocator for images

	size_t		alignment;			//!< Specifies the minimum alignment in bytes for the memory object
	size_t		preferred_alignment;//!< Specifies the preferred alignment in bytes for the memory object
	cl_ulong	maxBufferSize;      //!< Specifies the minimum size in bytes for the buffer memory object
	bool		imagesSupported;    //!< Device supports images
	bool		hostUnified;		//!< Memory allocator may allocate memory in unified(accessible) with the host memory
	bool		usedByDMA;			//!< Device may use DMA engine to access memory object
	bool		GLSharing;			//!< Device memory manager can accept GL sub-system memory object handles
	bool		DXSharing;			//!< Device memory manager can accept DX sub-system memory object handles
};

/*! \struct cl_dev_cmd_desc
 * \brief Description of executed command
 * This structure holds a description of a command that is passed to the device for execution.
 */
struct cl_dev_cmd_desc
{
	cl_dev_cmd_type	type;				//!< Command type identifier, defined in cl_dev_cmd_type
	cl_dev_cmd_id	id;					//!< Framework provided command identifier. It will be used
										//!< during command completion notification in clDevCmdStatusChanged()
	void*			data;				//!< A Pointer to user specific information It will be used
										//!< during command completion notification in clDevCmdStatusChanged()
	void*			params;				//!< Pointer to a buffer that holds command specific parameters
	size_t			param_size;			//!< Size of the parameter buffer in bytes
	bool			profiling;			//!< Enable profiling data for this command
} ;

/*! \struct cl_dev_cmd_param_rw
 * \brief Parameters of READ/WRITE commands
 * This structure holds a value of parameters which are passed within cl_dev_cmd_desc
 * and associated with CL_DEV_CMD_READ and CL_DEV_CMD_WRITE identifiers.
 */
struct cl_dev_cmd_param_rw
{
	IOCLDevMemoryObject*	memObj;				//!< Handle to a memory object from where/to the data to be read/written.
											//!< It can be a buffer, image2D or image3D
	cl_uint			dim_count;				//!< A number of dimensions in the memory object.
	size_t			origin[MAX_WORK_DIM];	//!< Multi-dimensional offset in the memory object.
											//!< For 2Dimages origin[2] must be 0.
	size_t			region[MAX_WORK_DIM];	//!< Defines multi-dimensional region of the memory object to be used.
											//!< region[0] is width of region,
											//!< region[1] is height of the region,
											//!< region[2] is depth of the region.
	size_t			memobj_pitch[MAX_WORK_DIM-1];	//!< A pointer to array of dimension pitches in bytes for the read region. The size of the array is dim_count-1.
											//!< memobj_pitch[0] will contain the scan-line pitch in bytes for the read region,
											//!< memobj_pitch[1] will contain the size in bytes of each 2D slice for the mapped region.
	void*			ptr;					//!< Pointer to the host memory where the data to be read/written
	size_t			pitch[MAX_WORK_DIM-1];	//!< A pointer to array of dimension pitches in bytes for the pointer parameter. The size of the array is dim_count-1.
											//!< pitch[0] will contain the scan-line pitch in bytes for the mapped region,
											//!< pitch[1] will contain the size in bytes of each 2D slice for the mapped region.
	size_t			ptr_origin[MAX_WORK_DIM]; //!< Multi-dimensional offset in the ptr.
} ;

/*! \struct cl_dev_cmd_param_copy
 * \brief Description of COPY command
 * This structure holds a value of parameters which are passed within cl_dev_cmd_desc and associated with CL_DEV_CMD_COPY identifier.
 */
struct cl_dev_cmd_param_copy
{
	IOCLDevMemoryObject*	srcMemObj;			//!< Handle to a source memory object from where the data to be read.
	IOCLDevMemoryObject*	dstMemObj;			//!< Handle to a source memory object to where the data to be written.
	cl_uint				src_dim_count;				//!< A number of dimensions in the source memory object.
	cl_uint				dst_dim_count;				//!< A number of dimensions in the destination memory object.
	size_t				src_origin[MAX_WORK_DIM];	//!< Multi-dimensional offset in the source memory object.
	size_t				dst_origin[MAX_WORK_DIM];	//!< Multi-dimensional offset in the destination memory object.
	size_t				region[MAX_WORK_DIM];		//!< Defines multi-dimensional region of the memory object to be copied.
													//!< region[0] is width of region,
													//!< region[1] is height of the region,
													//!< region[2] is depth of the region.
	size_t			src_pitch[MAX_WORK_DIM-1];	//!< src memory object pitch, non-relevant pitch values are filled with zero
	size_t			dst_pitch[MAX_WORK_DIM-1];	//!< dst memory object pitch, non-relevant pitch values are filled with zero

} ;

/*! \struct cl_dev_cmd_param_map
 * \brief Description of MAP command
 * This structure holds a value of parameters which are passed within cl_dev_cmd_desc and associated with CL_DEV_CMD_MAP identifier.
 */
struct	cl_dev_cmd_param_map
{
	IOCLDevMemoryObject*	memObj;				//!< Handle to a memory object to be mapped
	void*					ptr;					//!< A pointer to mapped region associated with the map command [filled by device]
	cl_map_flags			flags;					//!< A bit-field is set to indicate how the mapped region will be used.
													//!< The values are defined by cl_dev_map_flags.
	cl_uint					dim_count;				//!< A number of dimensions in the mapped memory object.
	size_t					origin[MAX_WORK_DIM];	//!< Multi-dimensional offset in the memory object to mapped.
	size_t					region[MAX_WORK_DIM];	//!< Defines multi-dimensional region of the memory object to be mapped.
													//!< region[0] is width of region,
													//!< region[1] is height of the region,
													//!< region[2] is depth of the region.
	size_t					pitch[MAX_WORK_DIM-1];	//!< A pointer to array of dimension pitches in bytes for the mapped region. The size of the array is dim_count-1.
													//!< pitch[0] will contain the scan-line pitch in bytes for the mapped region,
													//!< pitch[1] will contain the size in bytes of each 2D slice for the mapped region.

	void*					map_handle;     		//!< Device-specific handle associated with the map command.	[filled/used by device]

};

/*! \struct cl_dev_cmd_param_kernel
 * \brief Description of NDRange and TASK commands
 * This structure holds a value of parameters which are passed within cl_dev_cmd_desc and associated with CL_DEV_CMD_KERNEL
 * and CL_DEV_CMD_TASK identifiers.
 */
struct	cl_dev_cmd_param_kernel
{
	cl_dev_kernel		kernel; 					//!< Handle to a kernel object to be executed
	cl_uint				work_dim;					//!< The number of dimensions used to specify the global work-items and work-items in the work-group.
													//!< Work_dim must be greater than 0 and less than or equal to 3.
													//!< When executing a task, this value must be equal to 1.
	size_t				glb_wrk_offs[MAX_WORK_DIM];	//!< Currently must be (0, 0, 0). In a future revision of OpenCL,
													//!< glb_work_offs can be used to specify an array of work_dim unsigned
													//!< values that describe the offset used to calculate the global ID of a work-item.
	size_t				glb_wrk_size[MAX_WORK_DIM];	//!< An array of work_dim unsigned values that describe the number of global work-items
													//!< in work_dim dimensions that will execute the kernel function. The total number of
													//!< global work-items is computed as glb_wrk_size[0] * � *glb_wrk_size[work_dim � 1].
													//!< When executing a task, this value must be equal to 1.
	size_t				lcl_wrk_size[MAX_WORK_DIM];	//!< An array of work_dim unsigned values that describe the number of work-items
													//!< that make up a work-group (also referred to as the size of the work-group)
													//!< that will execute the kernel specified by kernel. The total number of work-items in a work-group
													//!< is computed as lcl_wrk_size[0] * � * lcl_wrk_size[work_dim � 1].
													//!< When executing a task, this value must be equal to 1. When the values are 0, and hint or required
													//!< work-group size is defined for the kernel, the agent will use these values for execution.
													//!< When the values are 0, and neither hint nor required work-group sizes is not defined,
													//!< the agent will use optimal work-group size.
	const void*			arg_values;					//!< An array of argument values of the specific kernel.
													//!< An order of the values must be the same as the order of parameters in the kernel prototype.
													//!< If an argument is a memory object, a relevant value contains its handle (dev_mem_obj).
	size_t				arg_size;					//!< Size in bytes of the arg_values array.
} ;


/**
 * Used for fill image, and fill buffer commands.
 */
struct cl_dev_cmd_param_fill
{
    IOCLDevMemoryObject*    memObj;         //!< Handle to a memory object from where/to the data to be read/written.
    //!< It can be a buffer, image2D or image3D
    cl_uint         dim_count;              //!< A number of dimensions in the memory object.
    size_t          offset[MAX_WORK_DIM];   //!< Multi-dimensional offset in the memory object.
    //!< For 2Dimages origin[2] must be 0.
    size_t          region[MAX_WORK_DIM];   //!< Defines multi-dimensional region of the memory object to be used.
    //!< region[0] is width of region ; region[1] is height of the region ;
    //!< region[2] is depth of the region.
    const void*     pattern;                //!< Pointer to the host fill buffer
    size_t          pattern_size;           //!< Size of fill buffer (in bytes)
} ;

/**
* \typedef fn_clNativeKernel
* native function prototype
*/
typedef void (CL_CALLBACK fn_clNativeKernel)(void* INOUT param);

/*! \struct cl_dev_cmd_param_native
 * \brief Description of Native Function command
 * This structure holds a value of parameters which are passed within cl_dev_cmd_desc and associated with CL_DEV_CMD_NATIVE identifier.
 */
struct cl_dev_cmd_param_native
{
	fn_clNativeKernel*					func_ptr;			//!< A pointer to a host callable user function.
	size_t								args;				//!< The size in bytes of the argument list that argv points to.
	void*								argv;				//!< A pointer to the args list that user function should be called with. Some items contain handles to device memory objects. Before the user function is executed, the memory object handles are replaced by pointers to global memory.
	unsigned int						mem_num;			//!< The number of buffer objects that are passed in argv. These values will be updated by the device agent.
	size_t*								mem_offset;			//!< An offset to appropriate locations that argv points to where memory object handles (void* values) are stored.
};

// ------------------------------------------------------------------------------
// Device API functions definition
// ------------------------------------------------------------------------------

class IOCLFrameworkCallbacks;
class IOCLDevLogDescriptor;
class IOCLDevice;

//
//Device Initialization functions
//
/* clDevInitDevice
   Description
		Initialize internal state of the device driver, returns a set of device driver entry points.
   Input
		dev_id			Device identifier as it appears in framework. This value is used in cl_dev_mem object identification.
		pDevCallBacks	A pointer to an interface for callback functions provided to the device by the framework
		pLogDesc		A pointer to an interface for logger functions provided to the device by the framework
   Output
		pDevice			A pointer to an interface to the device
   Returns
		CL_DEV_SUCCESS		The device was successfully created. pDevEntry holds updated pointers
		CL_DEV_ERROR_FAIL	Internal error
*/
typedef cl_dev_err_code (fn_clDevCreateDeviceInstance)(
								   unsigned int		dev_id,
								   IOCLFrameworkCallbacks	*pDevCallBacks,
								   IOCLDevLogDescriptor		*pLogDesc,
								   IOCLDeviceAgent*				*pDevice
								   );

//! This function return device specific information defined by cl_device_info enumeration as specified in OCL spec. table 4.3.
/*!
	\param[in]	param					An enumeration that identifies the device information being queried. It can be one of
										the following values as specified in OCL spec. table 4.3
	\param[in]	valSize					Specifies the size in bytes of memory pointed to by paramValue. This size in
										bytes must be >= size of return type
	\param[out]	paramVal				A pointer to memory location where appropriate values for a given param as specified
										in OCL spec. table 4.3 will be returned. If paramVal is NULL, it is ignored
	\param[out]	paramValSizeRet			Returns the actual size in bytes of data being queried by paramVal. If paramValSize_ret is NULL,
										it is ignored.
	\retval		CL_DEV_SUCCESS			If functions is executed successfully.
	\retval		CL_DEV_INVALID_VALUE	If param_name is not one of the supported values or if size in bytes specified by
										paramValSize is < size of return type as specified in OCL spec. table 4.3 and paramVal is not a NULL value.
*/
typedef cl_dev_err_code (fn_clDevGetDeviceInfo)(
                        cl_device_info  IN  param,
                        size_t          IN  valSize,
                        void*           OUT paramVal,
                        size_t*         OUT paramValSizeRet
						);

/*!
 \interface IOCLFrameworkCallbacks
 \brief Device Callback interface prototype.

  This interface is provided by the runtime to the device agent.
  Thought this interface a device agent notifies runtime on command execution status change and
  back-end build completion.
*/
class IOCLFrameworkCallbacks
{
public:
	//! This function is called when previously enqueued command changes its state to RUNNING or COMPLETED.
	/*!
		\param[in]	cmd_id				Identifier of the enqueued command that changes its status
		\param[in]	data				A pointer to buffer that was passed within cl_dev_cmd_desc within clDevCommandListExecute().
		\param[in]	cmd_status			New command status
		\param[in]	completion_result	Indicates completion result. CL_DEV_SUCCESS the command was successfully completed.
										CL_DEV_ERROR_FAILED indicated that command completed with an error.
		\param[in]	timer				Time in nano seconds of the device timer when the status changed occurred,
										available only when profiling is enabled.
	*/
	virtual void clDevCmdStatusChanged(	cl_dev_cmd_id	IN cmd_id,
											void*			IN	data,
											cl_int			IN cmd_status,
											cl_int			IN completion_result,
											cl_ulong		IN timer
											) = 0;
};

/*!
 \interface IOCLDevBackingStore
 \brief Runtime Backing Store interface

  This interface represents Backing Store provided by the runtime and serves device
  memory object.
*/
class IOCLDevBackingStore
{
public:
	/*! \enum cl_dev_bs_description
	 * Defines possible values for backing store raw data origin
	 */
	enum cl_dev_bs_description
	{
		CL_DEV_BS_RT_ALLOCATED = 0,		//!<	The backing store is allocated by the runtime or by a device
		CL_DEV_BS_USER_ALLOCATED,		//!<	The backing store is based on a pointer provided by CL_USE_HOST_PTR
		CL_DEV_BS_USER_COPY,			//!<	The backing store contains data to be copied by the runtime (CL_COPY_HOST_PTR)
		CL_DEV_BS_RT_MAPPED             //!<    The backing store contains data that is mapped by the runtime from other object (such as GL object)
	};

	//!	Returns pointer to a backing store data
	/*!
		\retval	A pointer to backing store raw data area
	*/
	virtual void* GetRawData() const = 0;

	//!	Returns a description of the Raw Data origin
	/*!
		\retval	A pointer to backing store raw data area
	*/
	virtual cl_dev_bs_description GetRawDataDecription() const = 0;

	//!	Returns if there is a valid data stored in backing store
	/*!
		\retval	true	The backing store holding the valid data
		\retval	false	No valid data in the backing store
	*/

	//!	Returns a size of the raw data in bytes
    virtual size_t GetRawDataSize() const = 0;

	//!	Returns an offset in bytes to the element identified at origin.
	//!    origin must be an array with MAX_WORK_DIM elements
    virtual size_t GetRawDataOffset( const size_t* origin ) const = 0;

	virtual bool IsDataValid() const = 0;

	//!	Returns a number of dimensions used by data in the Backing Store
	virtual size_t GetDimCount() const = 0;

	//!	Returns a pointer to array of dimensions in bytes of the backing store The size of the array is dim_count.
	//!		pitch[0] will contain the scan-line pitch in bytes of the mapped region,
	//!		pitch[1] will contain the size in bytes of each 2D slice of the mapped region.
	//!		The NULL value is valid only for 1D buffers.
	virtual const size_t* GetDimentions() const = 0;

	//!	Returns a pointer to array of dimension pitches in bytes for the mapped region. The size of the array is dim_count-1.
	//!		pitch[0] will contain the scan-line pitch in bytes of the mapped region,
	//!		pitch[1] will contain the size in bytes of each 2D slice of the mapped region.
	virtual const size_t* GetPitch() const = 0;

	//!	Returns a reference to the image format. If Backing Store does not represent image the value of format is undefined.
    virtual const cl_image_format&  GetFormat() const = 0;

	//!	Returns a size of the image element or 1 if Backing Store represents buffer.
    virtual size_t GetElementSize() const = 0;

	//!	Add pendency on backing store instance and returns the new value
	/*!
		\retval The new pendency count
	*/
	virtual int AddPendency() = 0;

	//!	Remove pendency on backing store instance, when pendency is 0 the backing store is released
	/*!
		\retval	The new pendency count
	*/
	virtual int RemovePendency() = 0;

    //! Returns the pointer where the user should expect the data (NULL if used did not provide this pointer)
    virtual void* GetUserProvidedHostMapPtr(void) const = 0;
};

/*!
 \interface IOCLDevBackingStoreService
 \brief Runtime Memory Object Backing Store service interface

  This interface represents Backing Store service provided by the runtime memory object
*/
class IOCLDevRTMemObjectService
{
public:
	//!	Retrieves current memory object backing store.
	/*!
		\param[in]	flags	A flag represents backing store access flags
		\paran[out] ppBS	Runtime memory object backing store,
							may return NULL if BS doesn't exists

		\retval	CL_DEV_SUCCESS		The function is executed successfully.
		\retval	CL_DEV_ERROR_FAIL	When error occured during Backing store retrivals
	*/
	virtual cl_dev_err_code GetBackingStore(cl_dev_bs_flags flags, IOCLDevBackingStore* *ppBS) = 0;

    virtual cl_dev_err_code GetBackingStore(cl_dev_bs_flags flags, const IOCLDevBackingStore** ppBS) const = 0;

	//!	Updates current memory object backing store.
	/*!
		\paran[out] pBS		Updated memory object backing store to be used by the runtime

		\retval	CL_DEV_SUCCESS		The function is executed successfully.
		\retval	CL_DEV_ERROR_FAIL	When error occured during backing store update
	*/
	virtual cl_dev_err_code SetBackingStore(IOCLDevBackingStore* pBS) = 0;

	//!	Returns the number of Device Agent sharing the runtime memory object
	virtual size_t GetDeviceAgentListSize() const = 0;

	//!	Returns the list Device Agent sharing the runtime memory object
	virtual const IOCLDeviceAgent* const *GetDeviceAgentList() const = 0;
};

/*!
 \interface IOCLDevMemoryObject
 \brief Device Agent Memory object interface.

  This interface represents the OCL device agent memory object.
*/
class IOCLDevMemoryObject
{
public:
	//!	Creates host mapped memory region for further buffer map operation.
	/*!
		\param[in,out] pMapParams	A valid pointer to descriptor of memory mapped region.

		\retval	CL_DEV_SUCCESS		The function is executed successfully.
		\retval	CL_DEV_ERROR_FAIL	When error occured during mapped region creation
	*/
	virtual cl_dev_err_code clDevMemObjCreateMappedRegion( cl_dev_cmd_param_map*	pMapParams ) = 0;

	//!		Releases previously created host mapped memory region.
	/*
		\param[in]	pMapParams				A valid pointer to descriptor of memory mapped region.


		\retval	CL_DEV_SUCCESS				The function was executed successfully
		\retval	CL_INVALID_VALUE			If mapped memory pointer is not associated with memory object.
	*/
	virtual cl_dev_err_code clDevMemObjReleaseMappedRegion( cl_dev_cmd_param_map* pMapParams ) = 0;

	//! Retrieves a handle object data as represented by specific device type and device unique node id (NUMA)
	/*
		\param[in]	dev_type		Device type to which handle should be returned
		\param[in]	node_id			Unique ID of the required node
		\param[out]	handle			Pointer to where the device specific handle will be returned

		\retval	CL_DEV_SUCCESS				The function was executed successfully
		\retval	CL_INVALID_VALUE			If device type or node_id are not supported
	*/
	virtual cl_dev_err_code clDevMemObjGetDescriptor(cl_device_type dev_type,
									cl_dev_subdevice_id node_id,
									cl_dev_memobj_handle *handle) = 0;

	//!		This function creates sub-object for given memory object
	/*
		\param[in]	mem_flags				Memory access flags for required sub-buffer, the consistency of the
											flags with buffer flags are performed by the runtime
		\param[in]	origin					Location of sub region in the original object
		\param[in]	size					Size of required sub region
		\param[out] ppSubBuffer				A pointer to returned sub-buffer object

		\retval	CL_DEV_SUCCESS				The function is executed successfully.
		\retval	CL_DEV_INVALID_OPERATION	If sub-buffer can't be created
	*/
	virtual cl_dev_err_code clDevMemObjCreateSubObject( cl_mem_flags mem_flags,
		const size_t *origin, const size_t *size, IOCLDevMemoryObject** ppSubObject ) = 0;

	//!		This function deletes previously created memory object.
	/*
		\retval	CL_DEV_SUCCESS				The function is executed successfully.
		\retval	CL_DEV_INVALID_OPERATION	If memObj has one or mapped regions.
	*/
	virtual cl_dev_err_code clDevMemObjRelease( ) = 0;

};

/*!
 \interface IOCLDeviceFECompilerDescription
 \brief Provides description of the Front End compiler properties.

  This interface represents the Front-End compiler module and required
  properties to be able perform front-end compilation for specific device
*/
class IOCLDeviceFECompilerDescription
{
public:
	/* clDevFEModuleName
		Description
			This function returns the front compiler module name
		Returns
			Pointer to a string which holds front-end compiler module name
	*/
	virtual const char* clDevFEModuleName() const = 0;
	/* clDevFEConfiguration
		Description
			This function returns the front compiler configuration block
		Returns
			Pointer the front compiler configuration block
	*/
	virtual const void* clDevFEDeviceInfo() const = 0;
	/* clDevFEConfigurationSize
		Description
			This function returns the front compiler configuration block size
		Returns
			The front compiler configuration block size
	*/
	virtual size_t		clDevFEDeviceInfoSize() const = 0;
};

/*!
 \interface IOCLDeviceAgent
 \brief Device Agent interface.

  This interface represents the OCL device agent implementation.
  OCL runtime communicates with device agent thought this interface
*/
class IOCLDeviceAgent
{
public:

	/* clDevPartition
		Description
			This function partitions the device into sub-devices allowing execution on parts of a device's compute units.
		Input
			props						The desired partitioning criterion
            num_requested_subdevices    An upper bound on the amount of sub-devices to be created
            num_subdevices              The number of sub-devices to be generated if using CL_DEV_PARTITION_BY_COUNTS or BY_NAMES
            parent_device_id            The ID of the parent device. NULL if the parent device is the root device. 
            param                       An optional param: the partition size in case of PARTITION_EQUALLY, the count list if BY_COUNTS, the name list if BY_NAMES etc
		Output
            num_subdevices              The number of sub-devices to be generated if partitioning equally or by affinity
			cl_dev_subdevice_id 		A list of ids, one for each partition created
		Returns
			CL_DEV_SUCCESS				The command queue successfully created
			CL_DEV_INVALID_VALUE		If values specified in properties are not valid, num_subdevices is NULL, *num_subdevices is > 0 but subdevice_ids is NULL
			CL_DEV_INVALID_PROPERTIES	If values specified in properties are valid but are not supported by the device
			CL_DEV_OUT_OF_MEMORY		If there is a failure to allocate resources required by the OCL device driver
	*/
    virtual cl_dev_err_code clDevPartition(  cl_dev_partition_prop   IN     props,
                                               cl_uint               IN     num_requested_subdevices,
											   cl_dev_subdevice_id   IN     parent_device_id,
                                               cl_uint*              INOUT  num_subdevices,
                                               void*                 IN     param,
                                               cl_dev_subdevice_id*  OUT    subdevice_ids
                                            ) = 0;

	/* clDevReleaseSubdevice
		Description
			This function releases the resources associated with the given sub-device ID.
            Behaviour is undefined if command lists still exist that reference that sub-device.
            It is the framework's responsibility to ensure this is not called prematurely.
		Input
			subdevice_id				The sub-device to release
		Returns
			CL_DEV_SUCCESS				The command queue successfully created
			CL_DEV_INVALID_VALUE		If the subdevice_id is invalid
	*/
    virtual cl_dev_err_code clDevReleaseSubdevice(  cl_dev_subdevice_id IN subdevice_id
                                                   ) = 0;

	//
	// Command List function prototypes
	//
	//! 	This function creates a dependent command list on a device. This function performs an implicit retain of the command list.
	/*!
		\param[in]	props		Determines whether the out-of-order optimization could be applied on items in the command list.
		\param[out]	list		A valid (non zero) handle to device command list.
		\retval		CL_DEV_SUCCESS				The command queue successfully created
		\retval		CL_DEV_INVALID_VALUE		If values specified in properties are not valid
		\retval		CL_DEV_INVALID_PROPERTIES	If values specified in properties are valid but are not supported by the device
		\retval		CL_DEV_OUT_OF_MEMORY		If there is a failure to allocate resources required by the OCL device driver
	*/
	virtual cl_dev_err_code clDevCreateCommandList(	cl_dev_cmd_list_props IN props,
                                                        cl_dev_subdevice_id   IN subdevice_id,
										   cl_dev_cmd_list* OUT list
										   ) = 0;

	//! 	This function flushes the content of a list, all waiting commands are sent to execution.
	/*!
		\param[in]	list		A valid (non zero) handle to device command list.
		\retval		CL_DEV_SUCCESS					If the function was successfully executed
		\retval		CL_DEV_INVALID_COMMAND_LIST		If command list is not a valid command list
	*/
	virtual cl_dev_err_code clDevFlushCommandList(	cl_dev_cmd_list IN list
										  ) = 0;

	//!	After the all commands of the command list have completed (eg. Kernel executions, memory object updates etc.),
	//!	the command queue is deleted.
	/*!
		\param[in]	list		A valid (non zero) handle to device command list.
		\retval		CL_DEV_SUCCESS					The command queue successfully created
		\retval		CL_DEV_INVALID_COMMAND_LIST		If command list is not a valid command list
	*/
	virtual cl_dev_err_code clDevReleaseCommandList( cl_dev_cmd_list IN list
											 ) = 0;

	//!Passes a list of dependent commands into a specified command list for execution.
	//!If the list was created without OOO optimizations the commands are depended by the list index: item[n] depends on item[n-1].
	//!First item (item[0]) is dependent on the last item that was passed within previous  call for the same list identifier.
	/*!
		\param[in]	list		A valid handle to device command list, where to add list of commands. If value is NULL,
								the new independent list is created for given commands
		\param[in]	cmds		A pointer to a vector of command list to execute, each entry contains a pointer to cl_dev_cmd_desc structure
		\param[in]	count		Number of entries in cmds parameter

		\retval CL_DEV_SUCCESS					The function was executed successfully.
		\retval CL_DEV_INVALID_COMMAND_LIST		If command list is not a valid command list
		\retval CL_DEV_INVALID_COMMAND_TYPE		If command type specified in one of the cmds entries is not a valid command.
		\retval CL_DEV_INVALID_MEM_OBJECT		If one or more memory objects specified in parameters in one or more of cmds entries
												are not valid or are not buffer objects.
		\retval CL_DEV_INVALID_KERNEL			If kernel identifier specified in execution parameters is not valid.
		\retval CL_DEV_INVALID_OPERATION		If specific device cannot execute native kernel.
		\retval CL_DEV_OUT_OF_RESOURCES			Is a failure to queue the execution instance of kernel because of insufficient resources
												needed to execute the kernel.
		\retval CL_DEV_INVALID_WRK_DIM			If work_dim is not a valid value (i.e. a value between 1 and 3).
		\retval CL_DEV_INVALID_WG_SIZE			If lcl_wrk_size is specified and number of workitems specified by glb_wrk_size is
												not evenly divisable by size of work-group given by lcl_wrk_size or does not match
												the work-group size specified for kernel using the __attribute__((reqd_work_group_size(X, Y, Z)))
												qualifier in program source.
		\retval CL_DEV_INVALID_GLB_OFFSET		If glb_wrk_offset is not (0, 0, 0).
		\retval CL_DEV_INVALID_WRK_ITEM_SIZE	If the number of work-items specified in any of lcl_wrk_size[] is greater than the corresponding
												values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[]
	*/
	virtual cl_dev_err_code clDevCommandListExecute( cl_dev_cmd_list IN list,
												 cl_dev_cmd_desc* IN *cmds,
												 cl_uint IN count
												 ) = 0;

	//!	Add the calling thread to execution pool and wait for all submitted commands to completed.
	/*!
		\param[in]	list		A valid handle to device command list, where to add list of commands. If value is NULL,
								the new independent list is created for given commands
		\retval		CL_DEV_SUCCESS					The command queue successfully created
		\retval		CL_DEV_INVALID_COMMAND_LIST		If command list is not a valid command list
		\retval		CL_DEV_NOT_SUPPORTED			The operation is not supported by device. The runtime should handle wait by itself
	*/
	virtual cl_dev_err_code clDevCommandListWaitCompletion( cl_dev_cmd_list IN list
												 ) = 0;

	//!This function returns the list of image formats supported by an OCL implementation when the information about
	//! an image memory object is specified and device supports image objects.
	/*!
		\param[in]	flags			A bit-field that is used to specify allocation and usage information such as the memory arena
									that should be used to allocate the image object and how it will be used.
		\param[in]	image_type		Describes the image type as described in (cl_dev_mem_object_type).Only image formats are supported.
		\param[in]	num_entries		Specifies the number of entries that can be returned in the memory location given by formats.
									If value is 0 and formats is NULL, the num_entries_ret returns actual number of supported formats.
		\param[out]	formats			A pointer to array of structures that describes format properties of the image to be allocated.
									Refer to OCL spec section 5.2.4.1 for a detailed description of the image format descriptor.
		\param[out]	num_entries_ret	The actual number of supported image formats for a specific context and values specified by flags.
									If the value is NULL, it is ignored.
		\retval		CL_DEV_SUCCESS			The function is executed successfully.
		\retval		CL_DEV_INVALID_VALUE	If values specified in parameters is not valid or if num_entries is 0 and formats is not NULL.
	*/
	virtual cl_dev_err_code clDevGetSupportedImageFormats( cl_mem_flags IN flags,
									   cl_mem_object_type IN image_type,
									   cl_uint IN num_entries,
									   cl_image_format* OUT formats,
									   cl_uint* OUT num_entries_ret
									   ) const = 0;


	//!This function returns the memory allocation properties required by the device.
	/*!
		\param[in]	memObjType	Object type (Buffer, Image2D, Image3D) for which the quary is performed
		\param[out]	pAllocProp	A pointer to a structure where allocator properties are returned.

		\retval		CL_DEV_SUCCESS			The function was executed successfully
		\retval		CL_DEV_INVALID_VALUE	Invalid object type was specified
	*/
	virtual cl_dev_err_code clDevGetMemoryAllocProperties( cl_mem_object_type memObjType,
									   cl_dev_alloc_prop* pAllocProp
									   ) = 0;

	//!	This function creates a reference and allocates resources required by different memory objects, like buffer,
	//!	2D image and new 3D image. Its not mandatory that actual physical memory will be allocated.
	/*
		\param[in]	node_id		Sub-device ID where the memory object should be created (required for NUMA)
		\param[in]	flags		A bit-field that is used to specify allocation and usage information such as the memory arena
								that should be used to allocate the image object and how it will be used.
		\param[in]	format		A pointer to a structure that describes format properties of the image to be allocated.
								Refer to OCL spec section 5.2.4.1 for a detailed description of the image format descriptor.
								When creating buffers this parameter must be NULL.
		\param[in]	dim_count	A number of dimensions in the required memory object.
		\param[in]	dim_size	A pointer to a buffer that defines object dimensions.
								When creating a buffer, dim points size of the buffer in bytes.
									dim[0] specifies width of the image in pixel,
									dim[1] specifies height of the image in pixel,
									dim[2] specifies depth of the image in pixel,
									dim[dim_count-1] specifies size of the dim_count dimension.
									All values must be greater or equal to 1.
		\param[in]	pBSService	A pointer to the backing store service provided by the runtime memory object.
		\param[out]	pMemObj		A valid (non zero) pointer to allocated memory object.

		\retval	CL_DEV_SUCCESS					The function is executed successfully.
		\retval	CL_DEV_INVALID_VALUE			If values specified in properties are not valid.
		\retval	CL_DEV_INVALID_IMG_FORMAT		If values specified in format are not valid or if it is NULL and width or height not equal 1.
		\retval	CL_DEV_INVALID_IMG_SIZE			If width or height are 0 or if they exceed maximum possible values.
		\retval	CL_DEV_IMG_FORMAT_NOT_SUPPORTED	If the format is not supported.
		\retval	CL_DEV_OBJECT_ALLOC_FAIL		If there is a failure to allocate resources for memory object.
		\retval	CL_DEV_INVALID_OPERATION		If the device does not support images.
	*/
	virtual cl_dev_err_code clDevCreateMemoryObject( cl_dev_subdevice_id IN node_id,
									cl_mem_flags					IN flags,
									const cl_image_format*			IN format,
									size_t							IN dim_count,
									const size_t*					IN dim_size,
									IOCLDevRTMemObjectService*		IN pBSService,
									IOCLDevMemoryObject*			OUT *pMemObj
									) = 0;


	//
	// Program build function prototypes
	//
	//!	This function performs syntax validation of the intermediate or binary to be built by the device during later stages.
	/*
		\param[in]	bin_size			Size of the binary buffer
		\param[in]	bin					A pointer to binary buffer that holds program container defined by cl_prog_container.

		\retval CL_DEV_SUCCESS			The function is executed successfully.
		\retval CL_DEV_INVALID_VALUE	If bin_size is 0 or bin is NULL.
		\retval CL_DEV_INVALID_BINARY	If the binary is not supported by the device or program container content is invalid.
	*/
	virtual cl_dev_err_code clDevCheckProgramBinary( size_t IN bin_size,
												 const void* IN bin
												 ) = 0;

	//!	Creates a device specific program entity (no build is performed).
	/*!
		\param[in]	bin_size				Size of the binary buffer
		\param[in]	bin						A pointer to binary buffer that holds program container defined by cl_prog_container.
		\param[in]	prop					Specifies the origin of the input binary. The values is defined by cl_dev_binary_prop.
		\param[out]	prog							A valid (non zero) handle to created program object.
		\retval		CL_DEV_SUCCESS					The function was executed successfully.
		\retval		CL_DEV_INVALID_BINARY			If the back-end compiler failed to process binary.
		\retval		CL_DEV_OUT_OF_MEMORY			If the device failed to allocate memory for the program.
	*/
	virtual cl_dev_err_code clDevCreateProgram( size_t IN bin_size,
										   const void* IN bin,
										   cl_dev_binary_prop IN prop,
										   cl_dev_program* OUT prog
										   ) = 0;

	//!	Builds (compiles & links) a program executable from the program intermediate or binary.
	/*!
		\param[in]	prog							Program handle to be built
		\param[in]	options							A pointer to a string that describes the build options to be used for building the program executable.
													The list of supported options is described in section 5.4.3 in OCL spec. document.
		\param[out]	buildStatus						This value will be updated with the build status after build is done
		\retval		CL_DEV_SUCCESS					The function was executed successfully.
		\retval		CL_DEV_INVALID_PROGRAM			Invalid program object was specified.
		\retval		CL_DEV_INVALID_BUILD_OPTIONS	If build options for back-end compiler specified by options are invalid.
		\retval		CL_DEV_INVALID_BINARY			If the back-end compiler failed to process binary.
		\retval		CL_DEV_OUT_OF_MEMORY			If the device failed to allocate memory for the program.
	*/
	virtual cl_dev_err_code clDevBuildProgram( cl_dev_program IN prog,
										   const char* IN options,
										   cl_build_status* OUT buildStatus
										   ) = 0;

	//!	Deletes previously created program object and releases all related resources.
	/*!
		\param[in]	prog							Program handle to be built
		\retval		CL_DEV_SUCCESS					The function was executed successfully.
		\retval		CL_DEV_INVALID_PROGRAM			Invalid program object was specified.
	*/
	virtual cl_dev_err_code clDevReleaseProgram(cl_dev_program IN prog) = 0;

	//!	Allows the framework to release the resources allocated by the back-end compiler.
	//!	This is a hint from the framework and does not guarantee that the compiler will not be used in the future
	//!	or that the compiler will actually be unloaded by the device.
	/*!
		\retval		CL_DEV_SUCCESS					The function was executed successfully.
	*/
	virtual cl_dev_err_code clDevUnloadCompiler() = 0;

	//!	Returns the compiled program binary. The output buffer contains program container as defined cl_prog_binary_desc.
	/*!
		\param[in]	prog		A handle to created program object.
		\param[in]	size		Size in bytes of the buffer passed to the function.
		\param[out]	binary		A pointer to buffer wherein program binary will be stored.
		\param[out]	size_ret	The actual size in bytes of the returned buffer. When size is equal to 0 and binary is NULL,
								returns size in bytes of a program binary. If NULL the parameter is ignored.
		\retval		CL_DEV_SUCCESS			The function was executed successfully.
		\retval		CL_DEV_INVALID_BINARY	If the back-end compiler failed to process binary.
		\retval		CL_DEV_INVALID_VALUE	If size is not enough to store the binary or binary is NULL and size is not 0.
	*/
	virtual cl_dev_err_code clDevGetProgramBinary( cl_dev_program IN prog,
											 size_t	IN size,
											 void* OUT binary,
											 size_t* OUT size_ret
											 ) = 0;

	//!	Returns the build log after clDevBuildProgram was called and finished.
	/*!
		\param[in]	prog		A handle to created program object.
		\param[in]	size		Size in bytes of the buffer passed to the function.
		\param[out]	log			A pointer to buffer wherein program build log will be stored.
		\param[out]	size_ret	The actual size in bytes of the returned buffer. When size is equal to 0 and binary is NULL,
								returns size in bytes of the log. If NULL the parameter is ignored.
		\retval		CL_DEV_SUCCESS			The function was executed successfully.
		\retval		CL_DEV_INVALID_PROGRAM	If program is not valid program object.
		\retval		CL_DEV_INVALID_VALUE	If size is not enough to store the binary or binary is NULL and size is not 0.
		\retval		CL_DEV_STILL_BUILDING	If program is under build process.
	*/
	virtual cl_dev_err_code clDevGetBuildLog( cl_dev_program IN prog,
										  size_t IN size,
										  char* OUT log,
										  size_t* OUT size_ret
										  ) = 0;

	//!	Returns the list of supported binaries.
	/*!
		\param[in]	size		Size of the buffer passed to the function in terms of cl_prog_binary_desc.
		\param[out] types		A pointer to buffer wherein binary types will be stored.
		\param[out] size_ret	The actual size of the buffer returned by the function in terms of cl_prog_binary_desc.
								When count is equal to 0 and types is NULL, function returns a size of the list.
								If NULL the parameter is ignored.
		\retval	CL_DEV_SUCCESS			The function was executed successfully.
		\retval	CL_DEV_INVALID_PROGRAM	If program is not valid program object.
		\retval	CL_DEV_INVALID_VALUE	If count is not enough to store the binary or types is NULL and count is not 0.
	*/
	virtual cl_dev_err_code clDevGetSupportedBinaries( size_t IN size,
											   cl_prog_binary_desc* OUT types,
											   size_t* OUT size_ret
											   ) = 0;

	//!	Returns an internal identifier of the kernel.
	/*!
		\param[in]	prog		A handle to created program object.
		\param[in]	name		A pointer to NULL terminated string that contains kernel name.
		\param[out] kernel_id	A valid (non zero) kernel internal identifier.
		\retval		CL_DEV_SUCCESS			The function was executed successfully.
		\retval		CL_DEV_INVALID_VALUE	If name is NULL.
		\retval		CL_DEV_INVALID_PROGRAM	If prog is not a valid program object.
		\retval		CL_INVALID_KERNEL_NAME	If name is not found in the program
	*/
	virtual cl_dev_err_code clDevGetKernelId( cl_dev_program IN prog,
										  const char* IN name,
										  cl_dev_kernel* OUT kernel_id
										  ) = 0;

	//!	Returns identifiers of all kernels belong to specific program object.
	/*!
		\param[in]	prog				A handle to created program object.
		\param[in]	num_kernels			Size of array pointed to by kernels specified as the number of cl_dev_kernel entries.
		\param[out] kernels				The buffer where the kernel objects for kernels in program will be returned.
		\param[out] num_kernels_ret		The number of kernels in program. When num_kernels is equal to 0 and kernels is NULL,
										returns only the number of kernels. If NULL the parameter is ignored.

		\retval		CL_DEV_SUCCESS			The function was executed successfully.
		\retval		CL_DEV_INVALID_PROGRAM	If program is not valid program object.
		\retval		CL_DEV_INVALID_VALUE	If num_kernels is not enough to store the identifiers or kernels is NULL and num_kernels is not 0.
	*/
	virtual cl_dev_err_code clDevGetProgramKernels( cl_dev_program IN prog,
											 cl_uint IN num_kernels,
											 cl_dev_kernel* OUT kernels,
											 cl_uint* OUT num_kernels_ret
											 ) = 0;

	//!	Returns information about the kernel in program object.
	/*!
		\param[in]	kernel			An identifier of kernel in program object
		\param[in]	param			Specifies the information to query. The list is defined by cl_dev_kernel_info.
		\param[in]	value_size		Size in bytes of memory buffer pointed to by value.
		\param[out] value			A pointer to memory where the appropriate result being queried is returned.
		\param[out] value_size_ret	The actual size in bytes of data copied to value. When value_size is equal to 0 and value is NULL,
									returns only the size of data. If NULL the parameter is ignored.
		\retval		CL_DEV_SUCCESS			The function was executed successfully.
		\retval		CL_DEV_INVALID_KERNEL	If kernel identifier is not valid.
		\retval		CL_DEV_INVALID_VALUE	If value_size is not enough to store the identifiers or value is NULL and value_size is not 0.
	*/
	virtual cl_dev_err_code clDevGetKernelInfo( cl_dev_kernel IN kernel,
											cl_dev_kernel_info IN param,
											size_t IN value_size,
											void* OUT value,
											size_t* OUT value_size_ret
											) = 0;

	//!	Retrieves current performance counter from the device.
	/*!
		\return		Current performance counter value
	*/
	virtual cl_ulong clDevGetPerformanceCounter() = 0;

	//!	This function sets framework logger for the device.
	/*!
		\param[in]	pLogger						A pointer to a structure that hold framework logger entry points.
		\retval		CL_DEV_SUCCESS				The function was executed successfully.
		\retval		CL_DEV_INVALID_OPERATION	Device doesn't supports logging operations.
	*/
	virtual cl_dev_err_code clDevSetLogger(IOCLDevLogDescriptor* IN pLogger) = 0;

	virtual const IOCLDeviceFECompilerDescription& clDevGetFECompilerDecription() const = 0;

	//!	De-initialize internal state of the device agent and releases all allocated data.
	virtual void clDevCloseDevice() = 0;

};

/*!
 \interface IOCLDevLogDescriptor
 \brief Logger client interface.

  This interface represents the OCL logger client. Thought this interface a device agent may create
  a logger client instance and submit with it log events or messages
*/
class IOCLDevLogDescriptor
{
public:
	//!	This function creates a logger client for the device. Each device may create multiple logger clients,
	//	e.g. client per each internal component.
	/*!
		\param[in]	dev_id				A pointer to a structure that hold framework logger entry points.
		\param[in]	client_name			Name associated with logger client.
		\param[out] client_id			An client logger identifier to be used in further calls to clLogAddLine().
		\retval		CL_DEV_SUCCESS		The function was executed successfully.
		\retval		CL_DEV_FAILED		Logger internal error occurred.
	*/
	virtual cl_int clLogCreateClient( cl_int IN dev_id,
							  const wchar_t* IN client_name,
							  cl_int* OUT client_id
							  ) = 0;

	//!	This function releases previously created logger client.
	/*!
		\param[in]	client_id				A logger client identifier that created with clLogCreateClient().
		\retval		CL_DEV_SUCCESS		The function was executed successfully.
		\retval		CL_DEV_INVALID_VALUE	Invalid client identifier.
	*/
	virtual cl_int clLogReleaseClient(cl_int IN client_id) = 0;

	//!	This function adds a logger line to logger client. The logged line will appear in the logger only if logger line level
	//	is equal or above of the current client logger level.
	/*!
		\param[in]	client_id		Logger client identifier that was created with clLogCreateClient()
		\param[in]	log_level		Logging level of the message.
		\param[in]	source_file		Name of the source file that has added the log message.
		\param[in]	function_name	Name of the function from where the log line was added.
		\param[in]	line_num		Line number from where the log line was added.
		\param[in]	message			The used message to be added into the log line. The message format string is identical to printf() format.
		\retval		CL_DEV_SUCCESS		The function was executed successfully.
		\retval		CL_DEV_INVALID_VALUE	Logger client id is not valid.
	*/
	virtual cl_int clLogAddLine( cl_int IN client_id,
									 cl_int IN log_level,
									 const wchar_t* IN source_file,
									 const wchar_t* IN function_name,
									 cl_int IN line_num,
									 const wchar_t* IN message, ...) = 0;
	virtual cl_int clLogAddLine( cl_int IN client_id,
									 cl_int IN log_level,
									 const char* IN source_file,
									 const char* IN function_name,
									 cl_int IN line_num,
									 const wchar_t* IN message, ...) = 0;

};

#ifdef __cplusplus
}
#endif
