/* ===========================================================================
 File:		GDEBUG.C
 
 Notes:		Mac OS Implementation of glib debug api
 
 Author:		G Robert Liddon @ BA 283 mid atlantic
 
 Created:	Saturday 22nd May 2004
 
 Copyright (C) 1996 SuperExcellent All rights reserved. 
 ============================================================================ */


/* ---------------------------------------------------------------------------
 Standard Lib Includes
 --------------------- */
#include <stdio.h>
#include <stdlib.h>

/* ---------------------------------------------------------------------------
 Glib Includes
 ------------- */
#include "gdebug.h"

/* ---------------------------------------------------------------------------
 Game Includes
 ------------- */

/* ---------------------------------------------------------------------------
 Function Prototypes
 ------------------- */
namespace glib
{
	/* ---------------------------------------------------------------------------
	 Vars
	 ---- */
	void (*MsgFunc)(char const *e,va_list argptr);
	void (*ErrorFunc)(char const *Text,char const *File,int Line);
	
	/*	---------------------------------------------------------------------------
	 Function:	bool DBG_OpenModule(void);
	 
	 Purpose:	Initialise the debug module
	 
	 Returns:	FALSE if unable to init
	 
	 --------------------------------------------------------------------------- */
	GLIB_API bool DBG_OpenModule(void)
	{
		return(TRUE);
	}	
	
	
	/*	---------------------------------------------------------------------------
	 Function:	void	DBG_PollHost(void)
	 
	 Purpose:	Poll the host to enable debugging
	 
	 --------------------------------------------------------------------------- */
	GLIB_API void DBG_PollHost(void)
	{
	}	
	
	/*	---------------------------------------------------------------------------
	 Function:	void	DBG_Halt(void)
	 
	 Purpose:	Stop where I am
	 
	 --------------------------------------------------------------------------- */
	GLIB_API void DBG_Halt(void)
	{
		exit(0);
	}	
	
	
	/*	---------------------------------------------------------------------------
	 Function:	void DBG_SendMessage(char *e,...)
	 
	 Purpose:	Send a diagnostic messgae
	 
	 --------------------------------------------------------------------------- */
	
	GLIB_API void DBG_SendMessage(char const *e,...)
	{
		char buffer[2048];
		
		va_list ap;
		va_start(ap,e);
		vsprintf(buffer,e,ap);
		va_end(ap);
		
		printf("%s\n",buffer);
		
	}	
	
	/*	---------------------------------------------------------------------------
	 Function:	void DBG_SetMessageHandler(void (*Func)(char *e,va_list argptr))
	 
	 Purpose:	Set the message handler
	 
	 --------------------------------------------------------------------------- */
	GLIB_API void DBG_SetMessageHandler(void (*Func)(char const *e,va_list argptr))
	{
		MsgFunc=Func;
	}	
	/*	---------------------------------------------------------------------------
	 Function:	void DBG_Error(char const *Text,char *File,int Line);
	 
	 Purpose:	Send a msg to psyq host
	 
	 --------------------------------------------------------------------------- */
	
	GLIB_API void DBG_Error(char const *Text,char const *File,int Line)
	{
		if (ErrorFunc)
		{
			ErrorFunc(Text, File, Line);
		}
		else
		{
			DBG_SendMessage("Error:\n%s\nline %d in %s\n", Text, Line, File);
		}
		
		DBG_Halt();
	}
	
	
	GLIB_API void DBG_SetErrorFunc(void (*EFunc)(char const *Text,char const *File,int Line))
	{
		ErrorFunc=EFunc;
	}
	
	
}

/* ---------------------------------------------------------------------------
 ends */
