// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
// SPDX-License-Identifier: EPL-2.0 (changed by the NeoSymbian Authors)
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
// The NeoSymbian Authors (2026): acquire/release on the done flag (SMP)
// Description:
// Name        : pthread_once.cpp
// Part of     : pthread
// Interface   : POSIX, pthread
// POSIX implementation of mutexes on Symbian
// Version     :
//



#include "mutextypes.h"

EXPORT_C int pthread_once(pthread_once_t * once_control, 
                          thread_init_routine init_routine)
{
  
#ifndef NOUSE_INTERNALS
    void* tlsPtr;
    // Get the TLS pointer
    tlsPtr = _pthread_getTls();  

    if(!tlsPtr)
    {
        return ENOMEM;
    }
#endif  
  
    if(!once_control || !init_routine)
    {
        return EINVAL;
    }

    if ((int)__e32_atomic_load_acq32(once_control) == _EDone)	// NeoSymbian: acquire, pairs with the release below (SMP)
    {
        return 0;   
    }
  
#ifndef NOUSE_INTERNALS  
    _pthread_atomicMutexLock(tlsPtr);
#endif  
  
  
    if(*once_control == _EDone)
    {

#ifndef  NOUSE_INTERNALS   
        _pthread_atomicMutexUnlock(tlsPtr);
#endif      
        return 0;
    }
  
    *once_control = _EDoing;
    init_routine();
    __e32_atomic_store_rel32(once_control, _EDone);	// NeoSymbian: publishes what init_routine wrote (SMP)
#ifndef NOUSE_INTERNALS    
    _pthread_atomicMutexUnlock(tlsPtr);
#endif   
    return 0;
}

// End of File
