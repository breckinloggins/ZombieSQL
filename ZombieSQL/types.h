//
//  types.h
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef TYPES_H
#define TYPES_H

#include "engine.h"

#include <stddef.h>

struct _ZdbStandardTypes
{
    ZdbType* booleanType;
    ZdbType* intType;
    ZdbType* floatType;
    ZdbType* varcharType;
};

struct _ZdbStandardTypes* ZdbStandardTypes;

//
// Interfaces
//

// ZdbTypeCompareFn - Compares two values, returns 0 if equal, a negative number if value 1 is less than value 2, or a positive number if value 1 is greater than value 2.  Returns an error if the values can't be compared
typedef ZdbResult (*ZdbTypeCompareFn)(void* value1, void* value2, int* result);

// ZdbTypeSizeFn - Given a value from a type, determines the amount of bytes are needed to store it.  If passed null, return the nominal or static size and ZDB_RESULT_SUCCESS.  Otherwise return ZDB_RESULT_INVALID_STATE to signal that it is impossible to determine a nominal or static size
typedef ZdbResult (*ZdbTypeSizeFn)(void* value, size_t* result);

// ZdbTypeFromStringFn - Given a string, returns a value for the type from that string representation.  Always returns ZDB_RESULT_SUCCESS unless the value cannot be converted from the string
typedef ZdbResult (*ZdbTypeFromStringFn)(const char* str, void* result);

// ZdbTypeToStringFn - Give a value from a type, determines the string representation for that value.  Length is the length of the string array you are passing in.  Call this function with a null string pointer to determine how much space you will need to allocate for the string.  On return, length will contain the number of characters actually written
typedef ZdbResult (*ZdbTypeToStringFn)(void* value, size_t* length, char* result);

// ZdbTypeNextValueFn - Given a value, returns the next one in the sequence.  This is an optional function.  If set, it allows columns declared to be of this type to have the autoincrement attribute set.  The caller is responsible for allocating space for the next value
typedef ZdbResult (*ZdbTypeNextValueFn)(void* value, void* nextValue);

ZdbResult ZdbTypeInitialize();  /* Sets up the standard types */

ZdbResult ZdbTypeCreate(const char* name, ZdbTypeCompareFn compareFn, ZdbTypeSizeFn sizeFn, ZdbTypeFromStringFn fromStringFn, ZdbTypeToStringFn toStringFn, ZdbTypeNextValueFn nextValueFn, ZdbType** newType);

ZdbResult ZdbTypeCompare(ZdbType* type, void* value1, void* value2, int* result);

ZdbResult ZdbTypeSizeof(ZdbType* type, void* value, size_t* result);

ZdbResult ZdbTypeFromString(ZdbType* type, const char* str, void* result);

ZdbResult ZdbTypeToString(ZdbType* type, void* value, size_t* length, char* result);

ZdbResult ZdbTypeNextValue(ZdbType* type, void* value, void* nextValue);

int ZdbTypeSupportsCompare(ZdbType* type);
int ZdbTypeSupportsSizeof(ZdbType* type);
int ZdbTypeSupportsFromString(ZdbType* type);
int ZdbTypeSupportsToString(ZdbType* type);
int ZdbTypeSupportsNextValue(ZdbType* type);

#endif // TYPES_H