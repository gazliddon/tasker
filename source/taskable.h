
#ifndef __TASKABLE_H__
#define __TASKABLE_H__

#include <stdlib.h>

#include "tasker.h"

namespace glib
{
	class Taskable
	{
	public:
    
		Taskable(int _id = 0);
		virtual ~Taskable(void);
		
		virtual void main(void) = 0;
		
		void pleaseDie(void);
		
		void * operator new( size_t stAllocateBlock);
		void operator delete( void * );
		

		static int countTasks(u32 _id);
		static void askTasksToDieByIDandWait(u32 _id);
		
		
		
	protected:
		void sleepWhileNotDead(int _frames);
		
		void sleepWhileFalse(volatile bool & flag);
		
		bool sleepWhileFalseMinFrames(volatile bool & flag, int _frames);

		void sleepWhileTrue(volatile bool & flag);
		
		void sleep(unsigned int _val = 1);
		
		TASK * m_taskPtr;
		volatile bool mPleaseDie;
	};

}

#endif
