/* ==========================================================================
	File:		GSYS.H

	Notes:		Machine Independant API to target for low level system info
				and manipulation
	
	Author:		Gary Liddon

	Copyright (C) 1995 - 1997 Gary Liddon
	All rights reserved. 
  =========================================================================== */

#ifndef __GSYS_H__
#define __GSYS_H__

/* ---------------------------------------------------------------------------
	Includes
	-------- */
#include "gtypes.h"

namespace glib
{

/* ---------------------------------------------------------------------------
	Defines
	------- */

/* ---------------------------------------------------------------------------
	Typedefs
	-------- */

/* ---------------------------------------------------------------------------
	Enums
	----- */

/* ---------------------------------------------------------------------------
	Externs
	------- */

/* ---------------------------------------------------------------------------
	Structures
	---------- */


/* ---------------------------------------------------------------------------
	Supported Functions
	------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/*	System Initialisation stuff
	--------------------------- */
GLIB_API bool	GSYS_InitMachine(void);

/*	Stack handling functions
	------------------------ */
GLIB_API void	GSYS_SetStackAndJump(void *Stack,void(*Func)(void *),void *Param);

/*	Machine Info Functions
	---------------------- */

#ifdef __cplusplus
};
#endif


}

/* --------------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------------
	ends */