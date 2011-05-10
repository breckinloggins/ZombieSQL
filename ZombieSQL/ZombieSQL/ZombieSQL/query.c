//
//  query.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#include "query.h"


struct _ZdbQueryCondition
{
    ZdbQueryConditionType type;
    int columnIndex;
    void* value;
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

int _compareValues(ZdbType* type, void* value1, void* value2, ZdbQueryConditionType conditionType)
{
    int result = 0;
    ZdbTypeCompare(type, value1, value2, &result);
    return !result;
}

int _matchesQuery(ZdbRecordset* recordset)
{
    void* value1 = NULL;
    void* value2 = NULL;
    ZdbType* type;
    
    switch(recordset->query->condition.type)
    {
        case ZDB_QUERY_CONDITION_NONE:
            /* No condition, always match */
            return 1;
        default:
            value1 = recordset->query->condition.value;
            type = recordset->query->table->columns[recordset->query->condition.columnIndex]->type;
            
            ZdbQueryGetValue(recordset, recordset->query->condition.columnIndex, type, &value2);
            return _compareValues(type, value1, value2, recordset->query->condition.type);
    }
}

/*
 * Public functions
 */

ZdbResult ZdbQueryCreate(ZdbDatabase* database, ZdbQuery** query)
{
    ZdbQuery* q = malloc(sizeof(ZdbQuery));
    q->database = database;
    q->table = NULL;
    q->condition.type = ZDB_QUERY_CONDITION_NONE;   /* ALL rows */
    q->condition.value = NULL;
    
    *query = q;
    return ZdbMessages->InfoSuccess;
}

ZdbResult ZdbQueryAddTable(ZdbQuery* query, ZdbTable* table)
{
    if (query->table != NULL)
    {
        // Multiple tables in query is not currently supported
        return ZdbMessages->ErrorInvalidState;
    }
    
    query->table = table;
    return ZdbMessages->InfoSuccess;
}

ZdbResult ZdbQueryAddCondition(ZdbQuery* query, ZdbQueryConditionType type, int column, ZdbType* valueType, const char* str)
{
    
    if (query->database == NULL || query->table == NULL)
    {
        /* The query must be initialized and a table selected before adding a condition */
        return ZdbMessages->ErrorInvalidState;
    }
    
    if (column < 0 || column >= query->table->columnCount)
    {
        /* The column index is out of range for this query */
        return ZdbMessages->ErrorInvalidState;
    }
    
    ZdbType* columnType = query->table->columns[column]->type;
    if (columnType != valueType)
    {
        /* The types do not match */
        return ZdbMessages->ErrorInvalidState;
    }
    
    void* value;
    if (ZdbTypeNewValue(valueType, str, &value) != ZdbMessages->InfoSuccess)
    {
        /* There was an error creating the value from the string */
        return ZdbMessages->ErrorInvalidState;
    }
    
    query->condition.type = type;
    query->condition.columnIndex = column;
    query->condition.value = value;
    
    return ZdbMessages->InfoSuccess;
}


ZdbResult ZdbQueryExecute(ZdbQuery* query, ZdbRecordset** recordset)
{
    ZdbRecordset* rs = malloc(sizeof(recordset));
    rs->query = query;
    rs->rowIndex = -1;
    
    *recordset = rs;
    return ZdbMessages->InfoSuccess;
}

ZdbResult ZdbQueryFree(ZdbQuery* query)
{
    if (query == NULL)
    {
        /* Can't free a NULL query */
        return ZdbMessages->ErrorInvalidState;
    }
    
    if (query->condition.value)
    {
        free(query->condition.value);
    }
    
    free(query);
    
    return ZdbMessages->InfoSuccess;
}

int ZdbQueryNextResult(ZdbRecordset* recordset)
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

ZdbResult ZdbQueryGetValue(ZdbRecordset* recordset, int column, ZdbType* type, void** value)
{
    if (column < 0 || column >= recordset->query->table->columnCount)
    {
        /* Invalid column specified */
        return ZdbMessages->ErrorInvalidState;
    }
    
    if (type != recordset->query->table->columns[column]->type)
    {
        /* Attempt to cast result to an incompatible type */
        return ZDB_MESSAGE_ERR_INVALID_CAST;
    }
    
    ZdbRow* resultRow = recordset->query->table->rows[recordset->rowIndex];
    if (ZdbEngineGetValue(recordset->query->table, resultRow, column, value) != ZdbMessages->InfoSuccess)
    {
        /* Error getting the value for this row */
        return ZdbMessages->ErrorInvalidState;
    }
    
    return ZdbMessages->InfoSuccess;
}

ZdbResult ZdbQueryGetInt(ZdbRecordset* recordset, int column, int* value)
{
    int* v;
    ZdbResult result = ZdbQueryGetValue(recordset, column, ZdbStandardTypes->intType, (void**)&v);
        
    if (result == ZdbMessages->InfoSuccess)
    {
        *value = *v;
    }
    
    return result;
}

ZdbResult ZdbQueryGetBoolean(ZdbRecordset* recordset, int column, int* value)
{
    int* v;
    ZdbResult result = ZdbQueryGetValue(recordset, column, ZdbStandardTypes->booleanType, (void**)&v);
     
    if (result == ZdbMessages->InfoSuccess)
    {
        *value = *v;
    }
    
    return result;
}

ZdbResult ZdbQueryGetString(ZdbRecordset* recordset, int column, char** value)
{
    char* v;
    ZdbResult result = ZdbQueryGetValue(recordset, column, ZdbStandardTypes->varcharType, (void**)&v);
    
    if (result == ZdbMessages->InfoSuccess)
    {
        *value = v;
    }
    
    return result;    
}

ZdbResult ZdbQueryGetFloat(ZdbRecordset* recordset, int column, float* value)
{
    float *v;
    ZdbResult result = ZdbQueryGetValue(recordset, column, ZdbStandardTypes->floatType, (void**)&v);
    
    if (result == ZdbMessages->InfoSuccess)
    {
        *value = *v;
    }
    
    return result;
}




