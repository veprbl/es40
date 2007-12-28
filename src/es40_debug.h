/*  ES40 emulator.
 *
 *  This file is based upon GXemul.
 *
 *  Copyright (C) 2004-2007  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 */

/** 
 * \file
 * Contains macro's and prototypes for debugging.
 *
 * X-1.3        Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.2        Camiel Vanderhoeven                             15-NOV-2007
 *      Included stdarg.h for Linux.
 *
 * X-1.1        Camiel Vanderhoeven                             14-NOV-2007
 *      Initial creation.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#include <stdarg.h>

#if !defined(INCLUDED_DEBUG_H)
#define INCLUDED_DEBUG_H

#define	DEBUG_BUFSIZE		1024
#define	DEBUG_INDENTATION	4


#ifdef HAVE___FUNCTION__

#define	FAILURE(error_msg)					{	\
		char where_msg[800];					\
		sprintf(where_msg,			\
		    "%s, line %i, function '%s'\n",			\
		    __FILE__, __LINE__, __FUNCTION__);			\
        	fprintf(stderr, "%s: %s\n", error_msg, where_msg);	\
		exit(1);						\
	}

#else

#define	FAILURE(error_msg)					{	\
		char where_msg[800];					\
		sprintf(where_msg,			\
		    "%s, line %i\n", __FILE__, __LINE__);		\
        	fprintf(stderr, "%s: %s\n", error_msg, where_msg);	\
		exit(1);						\
	}

#endif	/*  !HAVE___FUNCTION__  */


#define	CHECK_ALLOCATION(ptr)					{	\
		if ((ptr) == NULL)					\
			FAILURE("Out of memory");			\
	}

#define	CHECK_REALLOCATION(dst,src,type)					{	\
	    type * rea_x;											\
		rea_x = (type *)src;									\
		if ((rea_x) == NULL) {									\
			FAILURE("Out of memory");							\
		} else {												\
			dst = rea_x;										\
		}														\
	}

static void va_debug(va_list argp, char *fmt);
void debug_indentation(int diff);
void debug(char *fmt, ...);
void fatal(char *fmt, ...);


#endif
