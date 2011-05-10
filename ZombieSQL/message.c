//
//  message.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 5/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "message.h"

#define MESSAGE_MAX_LENGTH          1024

#define DEFINE_MESSAGE(code, name, type, format)                         \
        static ZdbMessageDef name = { code, "name", type, format }; \
        ZdbMessages->name = &name;

extern struct _ZdbMessages* ZdbMessages;

struct _ZdbMessageDef
{
    int     code;                    /* The numeric code */
    char*   name;                    /* The symbolic name */
    int     type;                    /* The type of the message */
    char*   formatString;            /* The format string */
};


void ZdbMessageInitialize()
{
    ZdbMessages = calloc(1, sizeof(struct _ZdbMessages));
    DEFINE_MESSAGE(0,
                   InfoSuccess, 
                   ZDB_MESSAGE_TYPE_INFO, 
                   "The operation completed successfully"
                   );
    
    DEFINE_MESSAGE(1,
                   ErrorAutoIncrement, 
                   ZDB_MESSAGE_TYPE_ERROR,
                   "Cannot set a value for column '%s' because it has the autoincrement flag set"
                   );
    
    DEFINE_MESSAGE(2, 
                   ErrorInvalidState,
                   ZDB_MESSAGE_TYPE_ERROR,
                   "The state of the operation is invalid: %s"
                   );
    
    DEFINE_MESSAGE(3,
                   ErrorInvalidCast,
                   ZDB_MESSAGE_TYPE_ERROR,
                   "Cannot cast %s to %s"
                   );
    
    DEFINE_MESSAGE(4,
                   ErrorUnsupported, 
                   ZDB_MESSAGE_TYPE_ERROR,
                   "The operation is unsupported: %s"
                   );
                   
}

void ZdbMessagePrint(ZdbMessageDef* msg, ...)
{
    char format[MESSAGE_MAX_LENGTH] = {0};
    char* messageType;
    switch(msg->type)
    {
        case ZDB_MESSAGE_TYPE_INFO:
            messageType = "INFO";
            break;
        case ZDB_MESSAGE_TYPE_WARN:
            messageType = "WARN";
            break;
        case ZDB_MESSAGE_TYPE_ERROR:
            messageType = "ERROR";
            break;
        default:
            messageType = "UNKNOWN";
            break;
    }
    
    snprintf(format, MESSAGE_MAX_LENGTH, "(%s)\t%s\n", messageType, msg->formatString);
    
    va_list argp;
	va_start(argp,  msg);
    
    vprintf(format, argp);
    
    va_end(argp);
}
