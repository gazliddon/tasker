/* ==========================================================================
 File:			GTYPES.H
 
 Notes:			Standard Non Machine dependant data types used by LIBGAZ
 
 Author:			Gary Liddon @ 73b
 
 Created:		Saturday 16th March 1996
 
 Copyright (C) 1996 - 1997 Gary Liddon
 All rights reserved. 
 =========================================================================== */

#ifndef	__GTYPES_H__
#define	__GTYPES_H__

/* ---------------------------------------------------------------------------
 Includes
 -------- */


namespace glib
{
	
	/* ---------------------------------------------------------------------------
	 Defines
	 ------- */
#ifndef NULL
	
#ifdef __cplusplus
	
#define NULL 0
	
#else
	
#define NULL ((void *)0)
	
#endif /* __cplusplus */
#endif /* NULL */
	
#ifndef FALSE
#define FALSE 0
#endif
	
#ifndef TRUE
#define TRUE (!FALSE)
#endif



	
	/* ---------------------------------------------------------------------------
	 Typedefs
	 -------- */
	 
typedef signed char			s8;
typedef signed short		s16;
typedef signed long int		s32;
typedef long long			s64;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned long		u32;

typedef unsigned long long	u64;
typedef long long			s64;

typedef float				f32;
typedef double				f64;

	/* Lower case versions are defined in mtypes.h */
	
	typedef unsigned int uint;	
	
	
	/*	GLIB Api Defintion
	 ------------------ */
#ifndef GLIB_API
#define GLIB_API
#endif
	
	
}

/* ---------------------------------------------------------------------------
 Globals
 ------- */
/* --------------------------------------------------------------------------- */
#endif	/* __GTYPES_H__ */
/* ---------------------------------------------------------------------------
 ends */

