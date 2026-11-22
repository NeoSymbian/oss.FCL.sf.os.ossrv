/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
* © Portions Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *	@(#)setjmp.h	8.2 (Berkeley) 1/21/94
 * $FreeBSD: src/include/setjmp.h,v 1.8 2002/10/05 05:48:50 mike Exp $
 */

#ifndef _SETJMP_H_
#define _SETJMP_H_

//---
#ifdef __cplusplus
#include <e32def.h>

extern "C" {
#endif

//---

#include <sys/cdefs.h>

/* The size of the jmp_buf is machine dependent: */
#include <stdapis/machine/setjmp.h>


__BEGIN_DECLS
#if __BSD_VISIBLE || __POSIX_VISIBLE >= 200112 || __XSI_VISIBLE
void	_longjmp(jmp_buf, int) __dead2;
#endif
IMPORT_C void	longjmp(jmp_buf, int) __dead2;
IMPORT_C int	setjmp(jmp_buf);
#if __BSD_VISIBLE || __POSIX_VISIBLE || __XSI_VISIBLE
void	siglongjmp(sigjmp_buf, int) __dead2;
#endif

#ifdef __SYMBIAN32__
#define _setjmp setjmp
#define _longjmp longjmp
#endif

/* NeoSymbian (2026, docs user-side-smp.md US3): code built with VFP (ARMv8 AArch32, softfp) keeps its callers'
 * values in the callee-saved d8-d15, which this jmp_buf has no room for, so a longjmp would return with them
 * clobbered. Telling GCC that setjmp's return clobbers d8-d15 makes the function calling it save them in its
 * prologue and restore them when it returns (frozen jmp_buf and exports, no new code). */
#if defined(__GNUC__) && defined(__ARM_FP) && !defined(__ASSEMBLER__)
#define setjmp(__neo_b) __extension__ ({ int __neo_r = (setjmp)(__neo_b); \
	__asm__ __volatile__("" ::: "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15"); __neo_r; })
#endif

__END_DECLS

//---
#ifdef __cplusplus
}
#endif
//---

#endif /* !_SETJMP_H_ */
