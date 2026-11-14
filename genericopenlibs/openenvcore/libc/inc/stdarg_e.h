/*
* Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
* SPDX-License-Identifier: EPL-2.0 (changed by the NeoSymbian Authors)
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
* The NeoSymbian Authors (2026): GCC's own va_list (the ARM procedure call standard's, as RVCT's).
* Description:
* This is all somewhat tortured because ANSI C says that stdio.h is not supposed 
* to define va_list: it needs to have access to that data type, but must not use that name.
* Instead stdio.h should use an __ name, which is safe because it is reserved for 
* the implementation.  Hence __e32_va_list.
* 
*
*/



#ifndef _STDARG_E_H
#define _STDARG_E_H

/**
EPOC32 definitions, extracted from E32DEFS.H
*/
#if defined(__GNUC__) && !defined(__ARMCC__)
/* NeoSymbian: GCC passes the first arguments in registers, so the address of the last named one says nothing about
   where the rest are: its builtins, whose va_list is the procedure call standard's struct { void *__ap; } -- the same
   as RVCT's, which the libc sources also use directly (ap.__ap in vfscanf) */
typedef __builtin_va_list __e32_va_list;
#define va_start(ap,pn) __builtin_va_start(ap,pn)
#define va_arg(ap,type) __builtin_va_arg(ap,type)
#define va_end(ap) __builtin_va_end(ap)
#define va_copy(d,s) __builtin_va_copy(d,s)
#else
typedef char* __e32_va_list;

#define va_start(ap,pn) (ap=(__e32_va_list)&pn+((sizeof(pn)+sizeof(int)-1)&~(sizeof(int)-1)),(void)0)
#define va_arg(ap,type) (ap+=((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)),(*(type *)(ap-((sizeof(type)+sizeof(int)-1)&~(sizeof(int)-1)))))
#define va_end(ap) (ap=0,(void)0)
#endif

#endif
