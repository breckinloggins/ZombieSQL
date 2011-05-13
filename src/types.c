//
//  types.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

extern struct _ZdbStandardTypes* ZdbStandardTypes;

#define COMPARISON_FN(type) _compare##type
#define DECLARE_COMPARISON_FN(type)                                     \
    int _compare##type(void* value1, void* value2, int* result)   \
    {                                                                   \
        if (*(type*)value1 == *(type*)value2)                           \
            *(type*)result = 0;                                         \
        else if (*(type*)value1 < *(type*)value2)                       \
            *(type*)result = -1;                                        \
        else                                                            \
            *(type*)result = 1;                                         \
                                                                        \
        return ZDB_RESULT_SUCCESS;                                      \
    }

#define SIZEOF_FN(type) _sizeof##type
#define DECLARE_SIZEOF_FN(type)                                         \
    int _sizeof##type(void* value, size_t* result)                \
    {                                                                   \
        if (value != NULL)                                              \
            *result = sizeof(*(type*)value);                            \
        else                                                            \
            *result = sizeof(type);                                     \
                                                                        \
        return ZDB_RESULT_SUCCESS;                                \
    }

#define COPY_FN(type) _copy##type
#define DECLARE_COPY_FN(type)                                           \
    int _copy##type(void* dest, void* src)                        \
    {                                                                   \
        if (dest == NULL || src == NULL)                                \
        {                                                               \
            return ZDB_RESULT_INVALID_NULL;                      \
        }                                                               \
                                                                        \
        *(type*)dest = *(type*)src;                                     \
        return ZDB_RESULT_SUCCESS;                                \
    }

#define FROMSTRING_FN(type) _fromstring##type
#define DECLARE_FROMSTRING_FN(type, fn, default)                        \
    int _fromstring##type(const char* str, void* result)          \
    {                                                                   \
        if (str == NULL)                                                \
        {                                                               \
            *(type*)result = default;                                   \
        }                                                               \
        else                                                            \
        {                                                               \
            *(type*)result = fn(str);                                   \
        }                                                               \
                                                                        \
        return ZDB_RESULT_SUCCESS;                                \
    }

#define TOSTRING_FN(type) _tostring##type
#define DECLARE_TOSTRING_FN(type, fmt)                                  \
    int _tostring##type(void* value, size_t* length, char* result)\
    {                                                                   \
        if (result == NULL)                                             \
        {                                                               \
            /* Just calculate the size */                               \
            *length = snprintf(NULL, 0, fmt, *(type*)value);            \
        }                                                               \
        else                                                            \
        {                                                               \
            /* Actually output the result */                            \
            *length = snprintf(result, (*length)+1, fmt, *(type*)value);\
        }                                                               \
                                                                        \
        return ZDB_RESULT_SUCCESS;                                \
    }

#define NEXTVALUE_FN(type) _nextvalue##type
#define DECLARE_NEXTVALUE_FN(type, first)                               \
    int _nextvalue##type(void* value, void* nextValue)            \
    {                                                                   \
        if (!value)                                                     \
            *(type*)nextValue = first;                                  \
        else                                                            \
            *(type*)nextValue = (*(type*)value)+1;                      \
                                                                        \
        return ZDB_RESULT_SUCCESS;                                \
    }

/* VARCHAR functions are a little different */
int _comparevarchar(void* value1, void* value2, int* result)
{
    *result = strcmp((char*)value1, (char*)value2);
    
    return ZDB_RESULT_SUCCESS;
}

int _sizeofvarchar(void* value, size_t* result)
{
    *result = ZDB_LIMIT_VARCHAR;
    
    return ZDB_RESULT_SUCCESS;
}

int _copyvarchar(void* dest, void* src)
{
    if (dest == NULL || src == NULL)
    {
        return ZDB_RESULT_INVALID_NULL;
    }
    
    strncpy((char*)dest, (char*)src, ZDB_LIMIT_VARCHAR);
    
    return ZDB_RESULT_SUCCESS;
}

int _fromstringvarchar(const char* str, void* result)
{
    if (str == NULL)
    {
        strcpy((char*)result, "");
    }
    else
    {
        strcpy((char*)result, str);
    }
    
    return ZDB_RESULT_SUCCESS;
}

int _tostringvarchar(void* value, size_t* length, char* result)
{
    if (result == NULL)
    {
        /* Just calculate the length */
        *length = strlen((char*)value);
    }
    else
    {
        *length = snprintf(result, (*length) + 1, "%s", (char*)value );
    }
    
    return ZDB_RESULT_SUCCESS;
}
/* End VARCHAR functions */

struct _ZdbType
{
    char name[ZDB_LIMIT_VARCHAR];
    ZdbTypeCompareFn compare;
    ZdbTypeSizeFn size;
    ZdbTypeCopyFn copy;
    ZdbTypeFromStringFn fromString;
    ZdbTypeToStringFn toString;
    ZdbTypeNextValueFn nextValue;
};

DECLARE_COMPARISON_FN(int)
DECLARE_SIZEOF_FN(int)
DECLARE_COPY_FN(int)
DECLARE_FROMSTRING_FN(int, atoi, 0)
DECLARE_TOSTRING_FN(int, "%d")
DECLARE_NEXTVALUE_FN(int, 0)

DECLARE_COMPARISON_FN(float)
DECLARE_SIZEOF_FN(float)
DECLARE_COPY_FN(float)
DECLARE_FROMSTRING_FN(float, atof, 0.0f)
DECLARE_TOSTRING_FN(float, "%f")
DECLARE_NEXTVALUE_FN(float, 0.0f)



int ZdbTypeInitialize()
{
    int result = ZDB_RESULT_SUCCESS;
    
    ZdbStandardTypes = malloc(sizeof(struct _ZdbStandardTypes));
    
    ZdbTypeCreate("int", COMPARISON_FN(int), SIZEOF_FN(int), COPY_FN(int), FROMSTRING_FN(int), TOSTRING_FN(int), NEXTVALUE_FN(int), &ZdbStandardTypes->intType);
    
    ZdbTypeCreate("float", COMPARISON_FN(float), SIZEOF_FN(float), COPY_FN(float), FROMSTRING_FN(float), TOSTRING_FN(float), NEXTVALUE_FN(float), &ZdbStandardTypes->floatType);
    
    ZdbTypeCreate("boolean", COMPARISON_FN(int), SIZEOF_FN(int), COPY_FN(int), FROMSTRING_FN(int), TOSTRING_FN(int), NULL, &ZdbStandardTypes->booleanType);
    
    ZdbTypeCreate("varchar", _comparevarchar, _sizeofvarchar, _copyvarchar, _fromstringvarchar, _tostringvarchar, NULL, &ZdbStandardTypes->varcharType);
    
    return result;
}

int ZdbTypeCreate(const char* name, ZdbTypeCompareFn compareFn, ZdbTypeSizeFn sizeFn, ZdbTypeCopyFn copyFn, ZdbTypeFromStringFn fromStringFn, ZdbTypeToStringFn toStringFn, ZdbTypeNextValueFn nextValueFn, ZdbType** newType)
{
    if (name == NULL || !strlen(name))
    {
        /* Types must have a name */
        /* TODO: Inforce uniqueness */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    if (compareFn == NULL || sizeFn == NULL || toStringFn == NULL)
    {
        /* Cannot declare a type without these critical functions */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    ZdbType* t = malloc(sizeof(ZdbType));
    strncpy(t->name, name, ZDB_LIMIT_VARCHAR);
    t->compare = compareFn;
    t->size = sizeFn;
    t->copy = copyFn;
    t->fromString = fromStringFn;
    t->toString = toStringFn;
    t->nextValue = nextValueFn;
    
    *newType = t;
    return ZDB_RESULT_SUCCESS;
}

int ZdbTypeNewValue(ZdbType* type, const char* str, void** result)
{
    if (type == NULL)
    {
        /* Can't create a new value without a type */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    size_t size = 0;
    int r = ZdbTypeSizeof(type, NULL, &size);
    if (r != ZDB_RESULT_SUCCESS)
    {
        /* Problem getting the size information we need */
        return r;
    }
    
    if (size <= 0)
    {
        /* Invalid size */
        return ZDB_RESULT_INVALID_OPERATION;
    }
    
    void* value = calloc(1, size);
    r = ZdbTypeFromString(type, str, value);
    if (r != ZDB_RESULT_SUCCESS)
    {
        /* Problem creating the type from the given string */
        free(value);
        return r;
    }
    
    *result = value;
    return ZDB_RESULT_SUCCESS;
}

int ZdbTypeGetName(ZdbType* type, const char* result)
{
    if (type == NULL)
    {
        /* Must pass a valid type */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    result = type->name;
    
    return ZDB_RESULT_SUCCESS;
}

int ZdbTypeSupportsCompare(ZdbType* type) { return type->compare != NULL; }
int ZdbTypeSupportsSizeof(ZdbType* type) { return type->size != NULL; }
int ZdbTypeSupportsFromString(ZdbType* type) { return type->fromString != NULL; }
int ZdbTypeSupportsToString(ZdbType* type) { return type->toString != NULL; }
int ZdbTypeSupportsNextValue(ZdbType* type) { return type->nextValue != NULL; }

int ZdbTypeCompare(ZdbType* type, void* value1, void* value2, int* result)
{
    if (type == NULL)
    {
        /* Can't compare values without a type definition */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    if (!ZdbTypeSupportsCompare(type))
    {
        /* Types of this type can't be compared */
        return ZDB_RESULT_UNSUPPORTED;
    }
    
    if (result == NULL)
    {
        /* Can't pass a null value to result */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    /* Type object performs the actual comparison */
    return type->compare(value1, value2, result);
}

int ZdbTypeSizeof(ZdbType* type, void* value, size_t* result)
{
    if (type == NULL)
    {   
        /* Can't determine size without a type definition */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    if (!ZdbTypeSupportsSizeof(type))
    {
        /* Type does not support sizeof operation */
        /* NOTE: Should never happen */
        return ZDB_RESULT_UNSUPPORTED;
    }
    
    if (value != NULL)
    {
        /* Per-value sizes are not yet supported */
        /* TODO: Support this */
        return ZDB_RESULT_UNSUPPORTED;
    }
    
    if (result == NULL)
    {
        /* Can't pass a null value to result */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    /* Type object performs the actual size computation */
    return type->size(value, result);
}

int ZdbTypeCopy(ZdbType* type, void* dest, void* src)
{
    if (type == NULL)
    {
        /* Can't perform copy without a type definition */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    if (dest == NULL || src == NULL)
    {
        /* Dest and src arguments can't be null */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    /* Type object performs the actual copy */
    return type->copy(dest, src);
}

int ZdbTypeFromString(ZdbType* type, const char* str, void* result)
{
    if (type == NULL)
    {
        /* Can't perform from string without a type definition */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    if (!ZdbTypeSupportsFromString(type))
    {
        /* Type does not support fromString operation */
        return ZDB_RESULT_UNSUPPORTED;
    }
    
    if (result == NULL)
    {
        /* Can't pass a null value to result */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    /* Type object performs the actual conversion */
    return type->fromString(str, result);
}

int ZdbTypeToString(ZdbType* type, void* value, size_t* length, char* result)
{
    if (type == NULL)
    {
        /* Can't perform to string without a type definition */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    if (!ZdbTypeSupportsToString(type))
    {
        /* Type does not support toString operation */
        return ZDB_RESULT_UNSUPPORTED;
    }
    
    if (length == NULL)
    {
        /* Can't call without a valid length parameter */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    /* Type object performs the actual conversion */
    return type->toString(value, length, result);
}

int ZdbTypeNextValue(ZdbType* type, void* value, void* nextValue)
{
    if (type == NULL)
    {
        /* Can't perform next value without a type definition */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    if (!ZdbTypeSupportsNextValue(type))
    {
        /* Type does not support nextValue operation */
        return ZDB_RESULT_UNSUPPORTED;
    }
    
    if (nextValue == NULL)
    {
        /* nextValue parameters must be valid */
        return ZDB_RESULT_INVALID_NULL;
    }
    
    /* Type object performs the actual incrementing */
    int result = type->nextValue(value, nextValue);
    
    return result;
}
