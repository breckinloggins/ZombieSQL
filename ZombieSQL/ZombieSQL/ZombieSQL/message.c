//
//  message.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 5/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "message.h"

extern struct _ZdbMessages* ZdbMessages;

struct _ZdbMessageDef
{
    int     messageCode;                    /* The numeric code */
    char*   formatString;                   /* The format string */
};

//static ZdbMessageDef messageDefs[] = 
//{
//    {ZdbMessages->InfoSuccess, "The operation completed successfully"},
    
//};


