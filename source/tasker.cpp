/* ===========================================================================
File:		TASKER.CPP

Notes:		Cooperative multitasking

Author:		G Robert Liddon @ 73b

Created:	Wednesday 27th March 1996

Copyright (C) 1996 DCI Ltd All rights reserved. 
============================================================================ */

/* ---------------------------------------------------------------------------
Standard Lib Includes
--------------------- */

/* ---------------------------------------------------------------------------
Glib Includes
------------- */
#include "tasker.h"
#include "gsys.h"
#include "gdebug.h"


/* ---------------------------------------------------------------------------
My Includes
----------- */

/* ---------------------------------------------------------------------------
Defines
------- */

namespace glib
{

/* ---------------------------------------------------------------------------
Function Prototypes
------------------- */
static void		ReturnToSchedulerIfCurrentTask(TASK *T);
static void		ExecuteTask(TASK *T);
static void 	AddToList(TASK **Head,TASK *ThisObj);
static void 	DetachFromList(TASK **Head,TASK *ThisObj);
static void 	LoTskKill(TASK *T);
static void		DoEpi(TASK * T);
static void		DoPro(TASK * T);

/* ---------------------------------------------------------------------------
Defines
------- */

/* ---------------------------------------------------------------------------
Variables
--------- */
static TASK	*	ActiveTasks;
static TASK	*	CurrentTask;

static jmp_buf	SchEnv;			/* The Scheduler's enviroment */
static u32		ExecId;
static u32		ExecMask;
static int		TasksActive;


	
/*	Vars for epi pro stuff
---------------------- */
static TSK_CBACK	EpiFunc;			/* Func to call before execing task of correct class */
static TSK_CBACK	ProFunc;			/* Func to call after execing task of correct class */
static u32			EpiProId;			
static u32			EpiProMask;

static DOTSK_CBACK	DoTasksPrologue;
static DOTSK_CBACK	DoTasksEpilogue;


/*	---------------------------------------------------------------------------
Function:		static void DoEpi(TASK * T)
Purpose:		Do epilogue route if appropriate
--------------------------------------------------------------------------- */
static void DoEpi(TASK * T)
{
	if (EpiFunc)
		if ((T->Id&EpiProMask)==EpiProId)
			EpiFunc(T);
}

/*	---------------------------------------------------------------------------
Function:		static void DoPro(TASK * T)
Purpose:		Do prologue route if appropriate
--------------------------------------------------------------------------- */
static void DoPro(TASK * T)
{
	if (ProFunc)
		if ((T->Id&EpiProMask)==EpiProId)
			ProFunc(T);
}

/*	---------------------------------------------------------------------------
Function:		bool TSK_OpenModule(u32 MemType)

Purpose:		Init the tasker module

Params:			MemType				=	Mem tasker uses for workspace

Returns:		FALSE if an error
--------------------------------------------------------------------------- */
bool TSK_OpenModule(void)
{
	
	TasksActive=0;
	ActiveTasks=NULL;
	CurrentTask=NULL;
	TSK_ClearExecFilter();
	TSK_ClearEpiProFilter();
	TSK_SetDoTasksEpilogue(NULL);
	TSK_SetDoTasksPrologue(NULL);
	
	return TRUE;
}	

/*	---------------------------------------------------------------------------
Function:	TASK *	TSK_AddTask(u32 Id,void (*Main)(TASK *T))

Purpose:	Add a task to the list of those to do

Returns:	-> to task to run
NULL if no tasks available
--------------------------------------------------------------------------- */

static unsigned int alignUp(unsigned int _size, int _rounding)
{
	return _size + (_rounding -1 ) - (_size % _rounding);
}


TASK *	TSK_AddTask(u32 Id,void (*Main)(TASK *T),int StackSize,int DataSize)
{
	int newStackSize = alignUp(StackSize, 16);
	int newDataSize = alignUp(DataSize, 16);
	
	u8 * pData = new u8[DataSize];
	u8 * pStack =  new u8[StackSize];
	TASK * pRetTask = new TASK;

	ASSERT(pRetTask && pStack && pData);
	
	pRetTask->Id = Id;
	
	pRetTask->fToInit=1;
	pRetTask->fToDie=0;
	pRetTask->fKillable=1;
	pRetTask->fActive=1;
	
	pRetTask->Main=Main;
	pRetTask->Stack= pStack;
	pRetTask->StackSize= newStackSize;
	pRetTask->Data = pData;
	
	pRetTask->SleepTime=1;
	
	/* if a task running then add as next in list or at beggining */
	
	if (CurrentTask)
	{
		AddToList(&CurrentTask->Next,pRetTask);
		pRetTask->Prev=CurrentTask;
	}
	else
		AddToList(&ActiveTasks,pRetTask);
	
	TasksActive++;
	
	return pRetTask;
	
}	

/*	---------------------------------------------------------------------------
Function:		void TSK_DoTasks(void)
Purpose:			Run All the tasks
--------------------------------------------------------------------------- */
void TSK_DoTasks(void)
{
	TASK *  T;
	T=ActiveTasks;
	
	if (DoTasksPrologue)
		DoTasksPrologue();
	
	while (T)
	{
		if (T->fActive && (T->Id&ExecMask)==ExecId)
		{
			T->SleepTime--;
			
			if (!T->SleepTime)
			{
				
				if (setjmp(SchEnv) == 0)
				{
					
					/* See if this task needs initing or not */
					CurrentTask=T;
					
					DoPro(T);
					
					if (T->fToInit)
					{
						T->fToInit=0;
						void * stack = (void *)(T->Stack+T->StackSize-32);
						void(*etask)(void *) = (void(*)(void *))(ExecuteTask);
                        stack = (void*) (((u64) stack)&(~0xf));
                        
                        GSYS_SetStackAndJump(stack,etask,T);
					}
					else
						longjmp(T->TskEnv,1);
				}
				else
				{
					
					/*	Back from the previous task	*/
					TASK *		NextT;
					
					
					NextT=T->Next;
					
					/*	Top this task if it was intended to die */
					
					if (T->fToDie)
						LoTskKill(T);
					
					T=NextT;
				}
			}
			else
				T=T->Next;
		}
		else
			T=T->Next;
	}
	
	if (DoTasksEpilogue)
		DoTasksEpilogue();
	
	CurrentTask=NULL;
}	


/*	---------------------------------------------------------------------------
Function:		void TSK_Sleep(int Frames)
Purpose:		This Task to sleep for an amount of frames
Params:			T -> Task
Frames = num of frames to sleep
--------------------------------------------------------------------------- */
void TSK_Sleep(int Frames)
{
	TASK *	T;
	
	T=CurrentTask;	
	
	ASSERT(T);
	
	DoEpi(T);
	
	if (!setjmp(T->TskEnv))
	{
		/* Saving enviro so go back to scheduler */
		T->SleepTime=Frames;
		ReturnToSchedulerIfCurrentTask(CurrentTask);
	}
}

/*	---------------------------------------------------------------------------
Function:		static void ReturnToScheduler(TASK *T)
Purpose:		Return to scheduler
Params:			T -> Current Task
--------------------------------------------------------------------------- */
static void ReturnToSchedulerIfCurrentTask(TASK *T)
{
	longjmp(SchEnv,1);
}	

/*	---------------------------------------------------------------------------
Function:		void TSK_Die(void)
Purpose:		Kill off current task
--------------------------------------------------------------------------- */
void TSK_Die(void)
{
	if (CurrentTask)
		TSK_Kill(CurrentTask);
}

/*	---------------------------------------------------------------------------
Function:		void TSK_Kill(TASK *T)
Purpose:		Kill off a task
If this is the current task then return to scheduler
Params:			T -> Task to kill
--------------------------------------------------------------------------- */
void TSK_Kill(TASK *T)
{
	if (T==CurrentTask && CurrentTask)
	{
		T->fToDie=TRUE;
		ReturnToSchedulerIfCurrentTask(T);
	}
	else
		LoTskKill(T);
}


/*	---------------------------------------------------------------------------
Function:		TASK *	TSK_GetFirstActive(void);

Purpose:		Get the first in the chain of active tasks

Params:			T -> Task block to check

Returns:		True if it is
--------------------------------------------------------------------------- */
TASK *	TSK_GetFirstActive(void)
{
	return(ActiveTasks);
}	


/*	---------------------------------------------------------------------------
Function:		void TSK_JumpAndResetStack(void (*RunFunc)(TASK *))

Purpose:		Current running task stack is reset and jumped off to
another routine

Params:			T -> Task block to check

Returns:		True if it is
--------------------------------------------------------------------------- */
void TSK_JumpAndResetStack(void (*RunFunc)(TASK *))
{
	TASK *	T;
	
	T=CurrentTask;
	
	if (T)
	{
		T->Main=RunFunc;
		GSYS_SetStackAndJump((void *)((u64)T->Stack+T->StackSize-sizeof(void *)*4),(void (*)(void*))ExecuteTask,T);
	}
}


/*	---------------------------------------------------------------------------
Function:		TASK *	TSK_GetCurrentTask(void);

Purpose:		Get the current executing task

Returns:		-> Current execiting task block

--------------------------------------------------------------------------- */
TASK *	TSK_GetCurrentTask(void)
{
	return(CurrentTask);
}	

/*	---------------------------------------------------------------------------
Function:		bool	TSK_IsCurrentTask(TASK *T)

Purpose:		Is this task the currently executing one?

Returns:		TRUE or FALSE

--------------------------------------------------------------------------- */
bool TSK_IsCurrentTask(TASK *T)
{	
	return(T==CurrentTask);
}	

/*	---------------------------------------------------------------------------
Function:		TASK *TSK_Exist(TASK *T,u32 Id,u32 Mask)

Purpose:		Is this task the currently executing one?

Params:			T -> Calling task

Returns:		-> first task found

--------------------------------------------------------------------------- */
TASK *TSK_Exist(TASK *T,u32 Id,u32 Mask)
{	
	TASK *	ptrTask;
	TASK *	RetTask;
	
	ptrTask=ActiveTasks;
	RetTask=NULL;
	
	while (ptrTask && !RetTask)
	{
		if ((ptrTask != T) && (ptrTask->Id&Mask)==Id)
			RetTask=ptrTask;
		else
			ptrTask=ptrTask->Next;
	}
	
	return(RetTask);
}

/*	---------------------------------------------------------------------------
Function:		void TSK_SetExecFilter(u32 Id,u32 Mask)

Purpose:		Set a filter for tasks that are executed
If ((Task.Id&Mask) == Id) then task is run

Params:
u32 =			Id;
Mask =			Task class mask

--------------------------------------------------------------------------- */
void TSK_SetExecFilter(u32 Id,u32 Mask)
{
	ExecId=Id;
	ExecMask=Mask;
}


/*	---------------------------------------------------------------------------
Function:		void TSK_ClearExecFilter(void)

Purpose:		Clears the exec filter, everything is run

--------------------------------------------------------------------------- */
void TSK_ClearExecFilter(void)
{
	TSK_SetExecFilter(0,0);
}


/*	---------------------------------------------------------------------------
Function:		int	TSK_KillTasks(TASK * CallingT,u32 Id,u32 Mask)

Purpose:		Mass task killer. Whacks through task list looking for victims.
If (T->fKillable && (Task.Id&Mask) == Id) then kill it.

Params:			CallingT ->		Calling task
u32 =			Id;
Mask =			Task class mask

Returns:		# of Tasks Killed
--------------------------------------------------------------------------- */
int	TSK_KillTasks(TASK * CallingT,u32 Id,u32 Mask)
{
	int			TasksKilled;
	TASK	*	T;
	bool		WasCurrentTaskKilled;
	
	TasksKilled=0;
	WasCurrentTaskKilled=FALSE;
	T=ActiveTasks;
	
	while (T)
	{
		TASK	*	NextT;
		
		NextT=T->Next;
		
		if (T != CallingT)
		{
			if (T->fKillable && (T->Id&Mask)==Id)
			{
				if (T==CurrentTask)
					WasCurrentTaskKilled=TRUE;
				else
					LoTskKill(T);
				
				TasksKilled++;
			}
		}
		
		T=NextT;
	}
	
	/* If Current task was killed then we go on to next task */
	
	if (WasCurrentTaskKilled)
	{
		CurrentTask->fToDie=TRUE;
		ReturnToSchedulerIfCurrentTask(CurrentTask);
	}
	
	return(TasksKilled);
}	


/*	---------------------------------------------------------------------------
Function:	void TSK_IterateTasks(u32 Id,u32 Mask,void (*CallBack)(TASK *T))
Purpose:	Go through task list and do a callback for all tasks which match.
If (Task->Id&Mask)==Id then callback function is invoked.
Params:		Id = Task Id to match
Mask = Task class mask
--------------------------------------------------------------------------- */
void TSK_IterateTasks(u32 Id,u32 Mask,void (*CallBack)(TASK *T))
{
	TASK	*	T;
	
	T=ActiveTasks;
	
	while (T)
	{
		TASK	*	NextT;
		
		NextT=T->Next;
		
		if ((T->Id&Mask)==Id)
			CallBack(T);
		
		T=NextT;
	}
}


/*	---------------------------------------------------------------------------
Function:	void TSK_MakeTaskInactive(TASK *T)
Purpose:	Make a task temporarily inactive.
Params:		T -> Task to knock out
--------------------------------------------------------------------------- */
void	TSK_MakeTaskInactive(TASK *T)
{
	T->fActive=0;
}


/*	---------------------------------------------------------------------------
Function:	void TSK_MakeTaskActive(TASK *T)
Purpose:	Make a task active again.
Params:		T -> Task to reactivate
--------------------------------------------------------------------------- */
void	TSK_MakeTaskActive(TASK *T)
{
	T->fActive=1;
}


/*	---------------------------------------------------------------------------
Function:	void TSK_MakeTaskImmortal(TASK *T)
Purpose:	Make a task impervious to mass killings.
Note that task can still be killed explicitly with TSK_Kill.
Params:		T -> Task to protect
--------------------------------------------------------------------------- */
void	TSK_MakeTaskImmortal(TASK *T)
{
	T->fKillable=0;
}


/*	---------------------------------------------------------------------------
Function:	void TSK_MakeTaskMortal(TASK *T)
Purpose:	Make a task vulnerable to mass killings.
Params:		T -> Task to expose
--------------------------------------------------------------------------- */
void	TSK_MakeTaskMortal(TASK *T)
{
	T->fKillable=1;
}


/*	---------------------------------------------------------------------------
Function:	bool TSK_IsTaskActive(TASK *T)
Purpose:	Check if a task is active
Params:		T -> Task to eheck
Returns:	0=Inactive, 1=Active
--------------------------------------------------------------------------- */
bool TSK_IsTaskActive(TASK *T)
{
	return (T->fActive);
}


/*	---------------------------------------------------------------------------
Function:	bool TSK_IsTaskMortal(TASK *T)
Purpose:	Check if a task is easy to kill.
Params:		T -> Task to check
Returns:	0=Immortal, 1=Mortal
--------------------------------------------------------------------------- */
bool	TSK_IsTaskMortal(TASK *T)
{
	return (T->fKillable);
}


/*	---------------------------------------------------------------------------
Function:		void DetachFromList(TASK **Head,TASK *ThisObj)
Purpose:		Take an object from an obj list
Params:			Head		->	Head ptr of list
ThisObj	->	Obj to add
--------------------------------------------------------------------------- */
static void DetachFromList(TASK **Head,TASK *ThisObj)
{
	if (ThisObj->Prev)
		ThisObj->Prev->Next=ThisObj->Next;
	else
		*Head=ThisObj->Next;
	
	if (ThisObj->Next)
		ThisObj->Next->Prev=ThisObj->Prev;
}


/*	---------------------------------------------------------------------------
Function:		void AddToList(TASK **Head,TASK *ThisObj)
Purpose:		Add an object to a obj list
Params:			Head		->	Head ptr of list
ThisObj	->	Obj to add
--------------------------------------------------------------------------- */
static void AddToList(TASK **Head,TASK *ThisObj)
{
	ThisObj->Prev=NULL;
	
	if ((ThisObj->Next=*Head))
		ThisObj->Next->Prev=ThisObj;
	
	*Head=ThisObj;
}

/*	---------------------------------------------------------------------------
Function:		static void LoTskKill(TASK *T)
Purpose:		Low level killing of task helper routine
Params:			T -> Task to kill
--------------------------------------------------------------------------- */
static void LoTskKill(TASK *T)
{
	/* Take out of list of active tasks */
	
	DetachFromList(&ActiveTasks,T);
	
	/* Dealloc the task block */
	delete[] T->Stack;
	delete[] T->Data;
	delete[] T;

	TasksActive--;
	
}	


/*	---------------------------------------------------------------------------
Function:		static void ExecuteTask(TASK *T)
Purpose:		Low level task executor, protects from tasks that return
without a TSK_Kill
Params:			T -> Task to execute
--------------------------------------------------------------------------- */
static void ExecuteTask(TASK *T)
{
	T->Main(T);
	T=CurrentTask;
	DoEpi(T);
	T->fToDie=TRUE;
	ReturnToSchedulerIfCurrentTask(T);
}	


/*	---------------------------------------------------------------------------
Epilogue / Prologue Function stuff
--------------------------------------------------------------------------- */
DOTSK_CBACK TSK_SetDoTasksPrologue(DOTSK_CBACK Func)
{
	DOTSK_CBACK		Old;
	
	Old=DoTasksPrologue;
	DoTasksPrologue=Func;
	return(Old);
}

DOTSK_CBACK TSK_SetDoTasksEpilogue(DOTSK_CBACK Func)
{
	DOTSK_CBACK		Old;
	
	Old=DoTasksEpilogue;
	DoTasksEpilogue=Func;
	return(Old);
}


TSK_CBACK 	TSK_SetTaskPrologue(TSK_CBACK Pro)
{
	TSK_CBACK 	Old;
	
	Old=ProFunc;
	ProFunc=Pro;
	return(Old);
}

TSK_CBACK 	TSK_SetTaskEpilogue(TSK_CBACK Epi)
{
	TSK_CBACK 	Old;
	
	Old=EpiFunc;
	EpiFunc=Epi;
	return(Old);
}

void TSK_SetEpiProFilter(u32 Id,u32 Mask)
{
	EpiProId=Id;
	EpiProMask=Id;
}

void TSK_ClearEpiProFilter(void)
{
	TSK_SetEpiProFilter(1,0);
	TSK_SetTaskEpilogue(NULL);
	TSK_SetTaskPrologue(NULL);
}


}

/* ---------------------------------------------------------------------------
ends */


