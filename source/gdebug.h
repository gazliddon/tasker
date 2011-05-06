/* ===========================================================================
	File:		GDEBUG.H

	Notes:		API for machine independant debugging

	Author:		Gary Liddon @ 73b

	Created:	Wednesday 27th March 1996

	Copyright (C) 1996 - 1997 Gary Liddon
	All rights reserved. 

  ============================================================================ */

#ifndef __GDEBUG_H__
#define __GDEBUG_H__

/* ---------------------------------------------------------------------------
	Includes
	-------- */

/*	Standard Lib
	------------ */
#include "stdarg.h"


/*	Glib Includes
	------------- */
#include "gtypes.h"

namespace glib
{
/* ---------------------------------------------------------------------------
	Defines
	------- */

/* ---------------------------------------------------------------------------
	Typedefs
	-------- */

#ifdef __NO_ASSERTS__
#define ASSERT(p)

#else

#ifdef __GL_DEBUG__
#define ASSERT(p) ( (p) ? (void)0 : (void) glib::DBG_Error(#p,__FILE__,__LINE__) )
#else
#define ASSERT(p) ( (p) ? (void)0 : (void) glib::DBG_Error(#p,__FILE__,__LINE__) )
#endif

#endif


/* ---------------------------------------------------------------------------
	Enums
	----- */

/* ---------------------------------------------------------------------------
	Id for each file
	---------------- */

/* ---------------------------------------------------------------------------
	Externs
	------- */

/* ---------------------------------------------------------------------------
	Structures
	---------- */

/* ---------------------------------------------------------------------------
	Supported Functions
	------------------- */

GLIB_API bool	DBG_OpenModule(void);
GLIB_API void	DBG_Halt(void);
GLIB_API void	DBG_PollHost(void);

GLIB_API void	DBG_SendMessage(char const *e,...);
GLIB_API void	DBG_SendErrorMessage(char *e,...);

GLIB_API void	DBG_SetMessageHandler(void (*Func)(char const *e,va_list argptr));
GLIB_API void	DBG_SetErrorMessageHandler(void (*Func)(char *e,va_list argptr));

GLIB_API void	DBG_Error(char const *Text,char const *File,int Line);

GLIB_API void	DBG_SetErrorFunc(void (*EFunc)(char const *Text,char const *File,int Line));
GLIB_API void	DBG_SetPollRoutine(void (*Func)(void));

/* --------------------------------------------------------------------------- */
}
/* --------------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------------
	ends */
