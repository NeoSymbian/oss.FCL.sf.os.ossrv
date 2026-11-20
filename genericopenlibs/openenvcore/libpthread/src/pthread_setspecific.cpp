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
// The NeoSymbian Authors (2026): a key with no node in this thread gets one (it used to fail EINVAL).
// Description:
// Name     : pthread_setspecific.cpp
// Part of  : PThread library
// pthread_setspecific call is implemented.
// Version:
//



#include <pthread.h>
#include <errno.h>
#include "threadglobals.h"
#include "pthreadmisc.h"

extern int _insertKeyNodeInList(_pkey_node_t *newTlsPtr,_pthread_node_t *temp);

EXPORT_C int pthread_setspecific(pthread_key_t key, const void *value)
{

    _global_data_t *glbPtr;
    _pthread_node_t *selfNodePtr;
    _pkey_node_t *tlsPtr;
    
    THR_PRINTF("[pthread] Begin pthread_setspecific\n");
    
    if (key >= PTHREAD_KEYS_MAX)
    {
        THR_PRINTF("[pthread] End pthread_setspecific\n");
        return EINVAL;
    }
        
    //Get the TLS value (self node pointer)
    selfNodePtr = (_pthread_node_t*) _pthread_getTls();
    if (NULL == selfNodePtr)    // This should never happen
    {
        return ENOMEM;
    }
    
    glbPtr = selfNodePtr->glbDataPtr;  // point to global struct
    glbPtr->lockThreadTable.Wait();    // Acquire the thread table lock

    int idx;
    idx = key / 32;
    
    int bitPos;
    bitPos = key % 32;
    
    if (((glbPtr->statusflag[idx]) & (0x1<<bitPos)) == _KEY_UNUSED)
    {
        THR_PRINTF("[pthread] End pthread_setspecific\n");
        glbPtr->lockThreadTable.Signal();  // release thread table lock
        return EINVAL;
    }
    
    // Lock the TCB
    selfNodePtr->lockNode.Wait();
    
    for (tlsPtr = selfNodePtr->tlsHead; tlsPtr != NULL; tlsPtr = tlsPtr->next)
    {
        if (tlsPtr->keyNumber == key)
        {
                tlsPtr->tls = (void*)value;
                break; 
        }
    }

    if (tlsPtr == NULL) // First node
    {
        // NeoSymbian: no node for this key in this thread (the key was created while the thread was starting, after
        // _createKeyList): add it now
        tlsPtr = new _pkey_node_t;
        if (tlsPtr == NULL || (tlsPtr->next = NULL, tlsPtr->tls = (void*)value, tlsPtr->keyNumber = key,
                _insertKeyNodeInList(tlsPtr, selfNodePtr) == -1))
        {
            delete tlsPtr;
            selfNodePtr->lockNode.Signal();
            glbPtr->lockThreadTable.Signal();  // release thread table lock
            return ENOMEM;
        }
    }

    // Unlock TCB node
    selfNodePtr->lockNode.Signal();
        
    glbPtr->lockThreadTable.Signal();  // release thread table lock
    
    THR_PRINTF("[pthread] End pthread_setspecific\n");
    return 0;
}

// End of file
