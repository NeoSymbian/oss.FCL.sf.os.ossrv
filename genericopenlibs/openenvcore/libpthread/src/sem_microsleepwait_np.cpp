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
// The NeoSymbian Authors (2026): a post racing the timeout is taken, not left over (SMP); mutex released on error
// Description:
// Name     : sem_microsleepwait_np.cpp
// Part of  : PThread library
// sem_microsleepwait_np call is implemented.
// Version:
//



#include <e32def.h>
#include <errno.h>

#include "semaphore.h"
#include "semaphoretypes.h"
#include "pthread.h"


int sem_microsleepwait_np (sem_t* sem, const long aSleepTime)
{
    if (!sem || !(*sem) )
    {
        errno = EINVAL;
        return -1;
    }

    _sem_t* lsem = *sem;
    if(lsem->iState != _sem_t::EInitialized)
    {
        errno = EINVAL;
        return -1;
    }

    if(_cancelable_mutex_wait(lsem) != 0)
    {
        return -1;
    }

    int count = --(lsem->iCount);
    lsem->iMutex.Signal();

    int retval = 0;
    if(count <0)
    {
    //push to cleanup

    retval = lsem->iSemaphore.Wait(aSleepTime);
    if(retval != KErrNone)
    {
        //timeout
        //the semaphore count goes up
        lsem->iMutex.Wait();
        if(lsem->iState != _sem_t::EInitialized)
        {
            lsem->iMutex.Signal();	// NeoSymbian: don't keep the mutex
            errno = EINVAL;
            return -1;
        }
        if(lsem->iCount >= 0)
        {
            // NeoSymbian: a sem_post counted this waiter in and signalled iSemaphore just as the wait timed out:
            // take that signal (it is there, so this returns at once) instead of leaving a token too many (SMP)
            lsem->iMutex.Signal();
            lsem->iSemaphore.Wait();
            return 0;
        }
        ++(lsem->iCount);
        lsem->iMutex.Signal();
        errno =  ETIMEDOUT;
        return -1;
    }

    //pop from cleanup
    }
    return 0;

}//sem_microsleepwait_np()

//End of File
