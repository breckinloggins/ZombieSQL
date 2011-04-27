//
//  query.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>
#include <string.h>

#include "query.h"


struct _ZdbQueryCondition
{
    ZdbQueryConditionType type;
    int columnIndex;
    ZdbColumnVal value;
};

struct _ZdbQuery 
{
    ZdbDatabase* database;          /* The database this query will operate on */
    ZdbTable* table;                /* Query subject table */
    ZdbQueryCondition condition;    /* The condition we will evaluate for each row */
};

struct _ZdbRecordset 
{
    ZdbQuery* query;            /* The query that created this recordset */
    int rowIndex;
};

/*
 * Internal functions
 */


ZdbResult _getValue(ZdbRecordset* recordset, int column, ZdbColumnType type, void* value)
{
    if (column < 0 || column >= recordset->query->table->columnCount)
    {
        /* Invalid column specified */
        return ZDB_RESULT_ERR_INVALID_STATE;
    }
    
    if (!ZdbTypesCompatible(type, recordset->query->table->columns[column]->type))
    {
        /* Attempt to cast result to an incompatible type */
        return ZDB_RESULT_ERR_INVALID_CAST;
    }
    
    ZdbRow* resultRow = recordset->query->table->rows[recordset->rowIndex];
    ZdbColumnVal *val = &resultRow->values[column];
    
    switch (ZdbGetCanonicalType(type))
    {
        case ZDB_COLTYPE_BOOLEAN:
            *(int*)value = val->boolVal;
            break;
        case ZDB_COLTYPE_INT:
            *(int*)value = val->intVal;
            break;
        case ZDB_COLTYPE_VARCHAR:
            *(char**)value = val->varcharVal;
            break;
        case ZDB_COLTYPE_FLOAT:
            *(float*)value = val->floatVal;
            break;
        default:
            /* Did you forget to add a type case here? */
            return ZDB_RESULT_ERR_INVALID_STATE;
            break;
    }
    
    return ZDB_RESULT_SUCCESS;
}

int _compareValues(ZdbColumnType type, ZdbColumnVal* value1, ZdbColumnVal* value2, ZdbQueryConditionType conditionType)
{
    switch(ZdbGetCanonicalType(type))
    {
        case ZDB_COLTYPE_BOOLEAN:
            return value1->boolVal == value2->boolVal;
        case ZDB_COLTYPE_INT:
            return value1->intVal == value2->intVal;
        case ZDB_COLTYPE_FLOAT:
            return value1->floatVal == value2->floatVal;
        case ZDB_COLTYPE_VARCHAR:
            return !strcmp(value1->varcharVal, value2->varcharVal);
        default:
            /* Did you forget to add a type here? */
            return 0;
    }
}

int _matchesQuery(ZdbRecordset* recordset)
{
    ZdbColumnVal* value1 = NULL;
    ZdbColumnVal* value2 = NULL;
    ZdbColumnType type;
    
    switch(recordset->query->condition.type)
    {
        case ZDB_QUERY_CONDITION_NONE:
            /* No condition, always match */
            return 1;
        default:
            value1 = &(recordset->query->condition.value);
            value2 = &(recordset->query->table->rows[recordset->rowIndex]->values[recordset->query->condition.columnIndex]);
            
            type = recordset->query->table->columns[recordset->query->condition.columnIndex]->type;
            
            return _compareValues(type, value1, value2, recordset->query->condition.type);
    }
}

/*
 * Public functions
 */

ZdbResult ZdbCreateQuery(ZdbDatabase* database, ZdbQuery** query)
{
    ZdbQuery* q = malloc(sizeof(ZdbQuery));
    q->database = database;
    q->table = NULL;
    q->condition.type = ZDB_QUERY_CONDITION_NONE;   /* ALL rows */
    
    *query = q;
    return ZDB_RESULT_SUCCESS;
}

ZdbResult ZdbAddQueryTable(ZdbQuery* query, ZdbTable* table)
{
    if (query->table != NULL)
    {
        // Multiple tables in query is not currently supported
        return ZDB_RESULT_ERR_INVALID_STATE;
    }
    
    query->table = table;
    return ZDB_RESULT_SUCCESS;
}

ZdbResult ZdbAddQueryCondition(ZdbQuery* query, ZdbQueryConditionType type, int column, ZdbColumnType valueType, ZdbColumnVal value)
{
    if (value.ignored)
    {
        /* Can't ignore a condition value */
        return ZDB_RESULT_ERR_INVALID_STATE;
    }
    
    if (query->database == NULL || query->table == NULL)
    {
        /* The query must be initialized and a table selected before adding a condition */
        return ZDB_RESULT_ERR_INVALID_STATE;
    }
    
    if (column < 0 || column >= query->table->columnCount)
    {
        /* The column index is out of range for this query */
        return ZDB_RESULT_ERR_INVALID_STATE;
    }
    
    ZdbColumnType columnType = query->table->columns[column]->type;
    if (!ZdbTypesCompatible(columnType, valueType))
    {
        /* The types do not match */
        return ZDB_RESULT_ERR_INVALID_STATE;
    }
        
    
    query->condition.type = type;
    query->condition.columnIndex = column;
    query->condition.value = value;
    
    return ZDB_RESULT_SUCCESS;
}


ZdbResult ZdbExecuteQuery(ZdbQuery* query, ZdbRecordset** recordset)
{
    ZdbRecordset* rs = malloc(sizeof(recordset));
    rs->query = query;
    rs->rowIndex = -1;
    
    *recordset = rs;
    return ZDB_RESULT_SUCCESS;
}

int ZdbNextResult(ZdbRecordset* recordset)
{
    while (1)
    {
        ++recordset->rowIndex;
        if (recordset->rowIndex >= recordset->query->table->rowCount)
        {
            /* No more rows */
            return 0;
        }
        
        if (_matchesQuery(recordset))
        {
            break;
        }
    }
    
    /* There are more rows available */
    return 1;
}

ZdbResult ZdbGetIntValue(ZdbRecordset* recordset, int column, int* value)
{
    int v;
    ZdbResult result = _getValue(recordset, column, ZDB_COLTYPE_INT, &v);
        
    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = v;
    }
    
    return result;
}

ZdbResult ZdbGetBooleanValue(ZdbRecordset* recordset, int column, int* value)
{
    int v;
    ZdbResult result = _getValue(recordset, column, ZDB_COLTYPE_BOOLEAN, &v);
     
    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = v;
    }
    
    return result;
}

ZdbResult ZdbGetStringValue(ZdbRecordset* recordset, int column, char** value)
{
    char* v;
    ZdbResult result = _getValue(recordset, column, ZDB_COLTYPE_VARCHAR, &v);
    
    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = v;
    }
    
    return result;    
}

ZdbResult ZdbGetFloatValue(ZdbRecordset* recordset, int column, float* value)
{
    float v;
    ZdbResult result = _getValue(recordset, column, ZDB_COLTYPE_FLOAT, &v);
    
    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = v;
    }
    
    return result;
}




