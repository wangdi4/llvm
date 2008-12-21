///////////////////////////////////////////////////////////
//  enqueue_commands.h
//  Implementation of the Class ReadBufferCommand
//  Created on:      16-Dec-2008 10:11:31 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#if !defined(EA_AD59843B_3AC8_43d2_A19D_A86C45CAAAE9__INCLUDED_)
#define EA_AD59843B_3AC8_43d2_A19D_A86C45CAAAE9__INCLUDED_

//#include "Buffer.h"
//
//
///******************************************************************
// *
// ******************************************************************/
//class ReadBufferCommand : public Command
//{
//public:
//	ReadBufferCommand(
//		MemoryObject* 	buffer, 
//		size_t 			offset, 
//		size_t			cb, 
//		void* 			dst
//		);
//
//private:
//    MemoryObject*   m_buffer;
//	size_t          m_offset;
//	size_t          m_cb;
//    void*           m_dst;
//};
//
//
//
///******************************************************************
// *
// ******************************************************************/
//class WriteBufferCommand : public Command
//{
//
//public:
//	WriteBufferCommand(
//        MemoryObject*   buffer, 
//        size_t          offset, 
//        size_t          cb,
//        const void*     src
//        );
//
//	virtual ~WriteBufferCommand();
//
//private:
//    MemoryObject*   m_buffer;
//	size_t          m_offset;
//	size_t          m_cb;
//    void*           m_src;
//};
//
///******************************************************************
// *
// ******************************************************************/
//class ReadImageCommand
//{
//
//public:
//	ReadImageCommand(
//        MemoryObject*   image, 
//        const size_t*   origin[3], 
//        const size_t*   region[3], 
//        size_t          row_pitch, 
//        size_t          slice_pitch, 
//        void*           dst
//        );
//	virtual ~ReadImageCommand();
//
//private:
//        MemoryObject*   m_image;
//        const size_t*   m_origin[3]; 
//        const size_t*   m_region[3]; 
//        size_t          m_rowPitch;
//        size_t          m_slicePitch; 
//        void*           m_dst;
//};
//
///******************************************************************
// *
// ******************************************************************/
//class WriteImageCommand
//{
//
//public:
//	WriteImageCommand(
//        MemoryObject*   image, 
//        const size_t*   origin[3], 
//        const size_t*   region[3], 
//        size_t          row_pitch, 
//        size_t          slice_pitch, 
//        const void*     dst       
//        );
//	virtual ~WriteImageCommand();
//
//    private:
//        MemoryObject*   m_image;
//        const size_t*   m_origin[3]; 
//        const size_t*   m_region[3]; 
//        size_t          m_rowPitch;
//        size_t          m_slicePitch; 
//        const void*     m_src;
//};
//
///******************************************************************
// *
// ******************************************************************/
//class CopyBufferCommand : public Command
//{
//
//public:
//	CopyBufferCommand(
//		MemoryObject* 	srcBuffer, 
//		MemoryObject* 	dstBuffer, 
//		size_t 			srcOffset, 
//		size_t 			dstOffset,
//		size_t 			cb
//		);		
//	virtual ~CopyBufferCommand();
//
//private:
//    MemoryObject* 	m_srcBuffer;
//    MemoryObject* 	m_dstBuffer;
//	size_t 			m_Offset;
//	size_t 			m_srcOffset;
//	size_t 			m_cb;
//};
//
///******************************************************************
// *
// ******************************************************************/
//class CopyImageCommand
//{
//
//public:
//	CopyImageCommand(
//	 	MemoryObject*   srcImage,
//        MemoryObject*   dstimage, 
//        const size_t*   srcOrigin[3], 
//        const size_t*   dstOrigion[3],
//        const size_t*   region[3]
//	);
//	virtual ~CopyImageCommand();
//
//private: 
//    MemoryObject*   m_srcImage;
//    MemoryObject*   m_dstImage;
//    const size_t*   m_srcOrigin[3];
//    const size_t*   m_dstOrigion[3];
//    const size_t*   m_region[3];   
//};
//
///******************************************************************
// *
// ******************************************************************/
//class CopyImageToBufferCommand
//{
//public:
//	CopyImageToBufferCommand(
//        MemoryObject*   srcImage, 
//        MemoryObject*   dstBuffer, 
//        const size_t*   srcOrigin[3], 
//        const size_t*   region[3],
//        size_t          dstOffset
//    );
//	virtual ~CopyImageToBufferCommand();
//
//private: 
//    MemoryObject*   m_srcImage;
//    MemoryObject*   m_dstBuffer;
//    const size_t*   m_srcOrigin[3];
//    const size_t*   m_region[3];
//    size_t          m_dstOffset; 
//
//};
//
///******************************************************************
// *
// ******************************************************************/
//class CopyBufferToImageCommand
//{
//
//public:
//	CopyBufferToImageCommand(
//        MemoryObject*   srcBuffer, 
//        MemoryObject*   dstImage, 
//        size_t          srcOffset, 
//        const size_t*   dstOrigin[3], 
//        const size_t*   region[3]
//    );
//	virtual ~CopyBufferToImageCommand();
//
//private: 
//    MemoryObject*   m_srcBuffer;
//    MemoryObject*   m_dstImage;
//    size_t          m_srcOffset; 
//    const size_t*   m_dstOrigin[3];
//    const size_t*   m_region[3];
//
//
//
//};
//
///******************************************************************
// *
// ******************************************************************/
//class MapBufferCommand
//{
//
//public:
//	MapBufferCommand(
//		MemoryObject* 	buffer,
//		cl_map_flags 	map_flags, 
//		size_t 			offset, 
//		size_t 			cb		
//	 	);
//	virtual ~MapBufferCommand();
//
//private: 
//    MemoryObject*   m_buffer;
//	cl_map_flags 	m_mapFlags;
//    size_t          m_offset; 
//    size_t		   	m_cb;
//};
//
///******************************************************************
// *
// ******************************************************************/
//class MapImageCommand
//{
//public:
//	MapImageCommand(
//    	MemoryObject*   image,
//    	cl_map_flags 	map_flags,
//    	const size_t*   origion[3],
//    	const size_t*   region[3],  	
//    	size_t* 		image_row_pitch, 
//    	size_t* 		image_slice_pitch
//	);
//	virtual ~MapImageCommand();
//private: 
//	MemoryObject*   m_image;
//    cl_map_flags 	m_map_flags;
//    const size_t*   m_origion[3];
//    const size_t*   m_region[3]; 	
//    size_t* 		m_image_row_pitch;
//    size_t* 		m_image_slice_pitch;
//	
//};
//
///******************************************************************
// *
// ******************************************************************/
//class UnmapMemObjectCommand
//{
//
//public:
//	UnmapMemObjectCommand(
//		MemoryObject*   memObject,
//    	void* 			mapped_ptr
//		);
//	virtual ~UnmapMemObjectCommand();
//
//private: 
//	MemoryObject*   m_memObject;
//    void*			m_mappedPtr;
//};
//
///******************************************************************
// *
// ******************************************************************/
//class NDRangeKernelCommand
//{
//public:
//	NDRangeKernelCommand(
//		Kernel* 		kernel,
//		cl_uint			work_dim,
//		const size_t*	global_work_offset,
//		const size_t*	global_work_size,
//		const size_t*	local_work_size
//		);
//	virtual ~NDRangeKernelCommand();
//
//private: 
//	Kernel* 		m_kernel;
//	cl_uint			m_workDim;
//	const size_t*	m_globalWorkOffset;
//	const size_t*	m_globalWorkSize;
//	const size_t*	m_localWorkSize;
//
//};
//
///******************************************************************
// *
// ******************************************************************/
//class TaskCommand
//{
//
//public:
//	TaskCommand(
//		Kernel* kernel
//		);
//	virtual ~TaskCommand();
//private:
//	Kernel* m_kernel;
//};
//
///******************************************************************
// *
// ******************************************************************/
//class NativeKernelCommand
//{
//
//public:
//	NativeKernelCommand(	
//		void* 				usrfunc,
//		void*				args,
//		size_t 				cbArgs,
//		cl_uint 			numMemObjects,
//		const MemoryObject*	memObjList,
//		const void**		args_mem_loc
//	);
//	virtual ~NativeKernelCommand();
//
//private:
//	void* 				m_usrfunc;
//	void*				m_args;
//	size_t 				m_cbArgs;
//	cl_uint 			m_numMemObjects;
//	const MemoryObject*	m_memObjList;
//	const void**		m_argsMemLoc;
//};
//
///******************************************************************
// *
// ******************************************************************/
//class MarkerCommand
//{
//
//public:
//	MarkerCommand();
//	virtual ~MarkerCommand();
//
//};
//
///******************************************************************
// *
// ******************************************************************/
//class WaitForEventsCommand
//{
//
//public:
//	WaitForEventsCommand();
//	virtual ~WaitForEventsCommand();
//
//};
//
///******************************************************************
// *
// ******************************************************************/
//class BarrierCommand
//{
//
//public:
//	BarrierCommand();
//	virtual ~BarrierCommand();
//
//};
//
//
//
//
//class Command : public OCLObject
//{
//
//public:
//	Command();
//	virtual ~Command();
//	QueueEvent *m_QueueEvent;
//
//	virtual cl_dev_cmd_desc GetDevCommandDesc() =0;
//	virtual cl_command_type GetCommandType() const;
//
//};


#endif // !defined(EA_AD59843B_3AC8_43d2_A19D_A86C45CAAAE9__INCLUDED_)
