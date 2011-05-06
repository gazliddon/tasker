#include "taskable.h"
#include "tasker.h"

#include <stdio.h>
#include "gdebug.h"

namespace glib
{
	
	void doTask(TASK * t);
	
	Taskable::Taskable(int _id) :  mPleaseDie(false)
	{			   
		m_taskPtr->Id = _id;
	}
    
	Taskable::~Taskable(void)
	{
	}
	
	void doTask(TASK * t)
	{
		Taskable * object = (Taskable *) t->Data;
		object->main();
		delete object;
	}
	
	void * Taskable::operator new( size_t stAllocateBlock)
	{
		TASK *  t;
		
		t=(TASK *) TSK_AddTask(0,doTask,40*1024,stAllocateBlock);
		
		ASSERT(t);
		
		Taskable * object = (Taskable *) t->Data;
		object->m_taskPtr=t;
		
		return(object);
	}
	
	void Taskable::operator delete( void * block)
	{
		Taskable *	object;
		TASK *	t;
		object =(Taskable *) block;
		
		t=object->m_taskPtr;
		TSK_Kill(t);
	}
	
	static int sCount;
	
	static void countCallBack(TASK *T)
	{
		sCount+=1;
	}
	
	static void askToDieCallback(TASK *T)
	{
		Taskable * t = (Taskable*)(T->Data);
		t->pleaseDie();
	}
	
	int Taskable::countTasks(u32 _id)
	{
		sCount =0;
		TSK_IterateTasks(_id, 0xffffffff, countCallBack);
		int count = sCount;

		return count;
	}
	
	void Taskable::askTasksToDieByIDandWait(u32 _id)
	{
		TSK_IterateTasks(_id, 0xffffffff, askToDieCallback);
	
		while (countTasks(_id) != 0)
			glib::TSK_Sleep(1);
	}
	
	
	void Taskable::pleaseDie(void)
	{
		mPleaseDie = true;
	}
	
	
	void Taskable::sleepWhileNotDead(int _frames)
	{
		for (int f=0; f<_frames && !mPleaseDie; f++)
			sleep();
	}
	
	void Taskable::sleepWhileFalse(volatile bool & flag)
	{
		while (!flag)
		{
			sleep();
		}
	}

	
	bool Taskable::sleepWhileFalseMinFrames(volatile bool & flag, int _frames)
	{
		
		while (!flag && _frames != 0)
		{
			_frames--;
			sleep();
		}
		
		return _frames == 0;
	}
	
	
	void Taskable::sleepWhileTrue(volatile bool & flag)
	{
		while (flag)
		{
			sleep();
		}
	}
	
	void Taskable::sleep(unsigned int _val)
	{
		glib::TSK_Sleep(_val);
	}
	
	
}

