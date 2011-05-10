//
//  message.h
//  ZombieSQL
//
//  Created by Benjamin Loggins on 5/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef MESSAGE_H
#define MESSAGE_H

#define DECLARE_MESSAGE(name) ZdbMessageDef* name
#define DEFINE_MESSAGE(name, type, format)                  \
        { name, type, format },                             

#define ZDB_MESSAGE_TYPE_INFO            0       /* Diagnostic only */
#define ZDB_MESSAGE_TYPE_WARN            1       /* Non-critical issue */
#define ZDB_MESSAGE_TYPE_ERROR           2       /* Critical issue */

typedef struct _ZdbMessageDef ZdbMessageDef;

struct _ZdbMessages
{
    DECLARE_MESSAGE(InfoSuccess);
    DECLARE_MESSAGE(ErrorAutoIncrement);
    DECLARE_MESSAGE(ErrorInvalidState);
    DECLARE_MESSAGE(ErrorInvalidCast);
    DECLARE_MESSAGE(ErrorUnsupported);
};

struct _ZdbMessages* ZdbMessages;



#endif // MESSAGE_H
