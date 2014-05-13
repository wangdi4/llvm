/******************************************************************
 //
 //  OpenCL Conformance Tests
 // 
 //  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/

#ifndef _action_classes_h
#define _action_classes_h

#include "testBase.h"

// This is a base class from which all actions are born
// Note: No actions should actually feed I/O to each other, because then
// it would potentially be possible for an implementation to make actions
// wait on one another based on their shared I/O, not because of their
// wait lists!
class Action
{
	public:
		Action() {}
		virtual ~Action() {}
	
		virtual cl_int		Setup( cl_device_id device, cl_context context, cl_command_queue queue ) = 0;
		virtual cl_int		Execute( cl_command_queue queue, cl_uint numWaits, cl_event *waits, cl_event *outEvent ) = 0;
		
		virtual const char * GetName( void ) const = 0;

	protected:
	
};

// Base action for buffer actions
class BufferAction : public Action
{
	public:
		clMemWrapper		mBuffer;
		size_t				mSize;
		void				*mOutBuffer;
		
		BufferAction() { mOutBuffer = NULL; }
		virtual ~BufferAction() { free( mOutBuffer ); }
		
		virtual cl_int Setup( cl_device_id device, cl_context context, cl_command_queue queue, bool allocate );
};

class ReadBufferAction : public BufferAction
{
	public:
		ReadBufferAction() {}
		virtual ~ReadBufferAction() {}
		
		virtual cl_int Setup( cl_device_id device, cl_context context, cl_command_queue queue );
		virtual cl_int	Execute( cl_command_queue queue, cl_uint numWaits, cl_event *waits, cl_event *outEvent );

		virtual const char * GetName( void ) const { return "ReadBuffer"; }
};

#endif // _action_classes_h
