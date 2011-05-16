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
    return result;
}

int _matchesQuery(ZdbRecordset* recordset)
{
    void* value1 = NULL;
    void* value2 = NULL;
    ZdbType* type;

    if (recordset->query->condition.type == ZDB_QUERY_CONDITION_NONE)
    {
        /* No condition, always match */
        return 1;
    }

    value1 = recordset->query->condition.value;
    type = recordset->query->table->columns[recordset->query->condition.columnIndex]->type;

    ZdbQueryGetValue(recordset, recordset->query->condition.columnIndex, type, &value2);
    int result = _compareValues(type, value1, value2, recordset->query->condition.type);

    printf("RESULT!! %d %d %d\n", *(int*)value1, *(int*)value2, result);
    switch(recordset->query->condition.type)
    {
        case ZDB_QUERY_CONDITION_EQ:
            return result == 0;
        case ZDB_QUERY_CONDITION_NE:
            return result != 0;
        case ZDB_QUERY_CONDITION_LT:
            return result > 0;
        case ZDB_QUERY_CONDITION_GT:
            return result < 0;
        case ZDB_QUERY_CONDITION_LTE:
            return result >= 0;
        case ZDB_QUERY_CONDITION_GTE:
            return result <= 0;
    }

    return result;
}

/*
 * Public functions
 */

int ZdbQueryCreate(ZdbDatabase* database, ZdbQuery** query)
{
    ZdbQuery* q = malloc(sizeof(ZdbQuery));
    q->database = database;
    q->table = NULL;
    q->condition.type = ZDB_QUERY_CONDITION_NONE;   /* ALL rows */
    q->condition.value = NULL;

    *query = q;
    return ZDB_RESULT_SUCCESS;
}

int ZdbQueryAddTable(ZdbQuery* query, ZdbTable* table)
{
    if (query->table != NULL)
    {
        // Multiple tables in query is not currently supported
        return ZDB_RESULT_UNSUPPORTED;
    }

    query->table = table;
    return ZDB_RESULT_SUCCESS;
}

int ZdbQueryAddCondition(ZdbQuery* query, ZdbQueryConditionType type, int column, ZdbType* valueType, const char* str)
{

    if (query->database == NULL || query->table == NULL)
    {
        /* The query must be initialized and a table selected before adding a condition */
        return ZDB_RESULT_INVALID_NULL;
    }

    if (column < 0 || column >= query->table->columnCount)
    {
        /* The column index is out of range for this query */
        return ZDB_RESULT_INVALID_OPERATION;
    }

    ZdbType* columnType = query->table->columns[column]->type;
    if (columnType != valueType)
    {
        /* The types do not match */
        return ZDB_RESULT_INVALID_CAST;
    }

    void* value;
    if (ZdbTypeNewValue(valueType, str, &value) != ZDB_RESULT_SUCCESS)
    {
        /* There was an error creating the value from the string */
        return ZDB_RESULT_INVALID_OPERATION;
    }

    query->condition.type = type;
    query->condition.columnIndex = column;
    query->condition.value = value;

    return ZDB_RESULT_SUCCESS;
}


int ZdbQueryExecute(ZdbQuery* query, ZdbRecordset** recordset)
{
    ZdbRecordset* rs = malloc(sizeof(recordset));
    rs->query = query;
    rs->rowIndex = -1;

    *recordset = rs;
    return ZDB_RESULT_SUCCESS;
}

int ZdbQueryFree(ZdbQuery* query)
{
    if (query == NULL)
    {
        /* Can't free a NULL query */
        return ZDB_RESULT_INVALID_NULL;
    }

    if (query->condition.value)
    {
        free(query->condition.value);
    }

    free(query);

    return ZDB_RESULT_SUCCESS;
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

int ZdbQueryGetValue(ZdbRecordset* recordset, int column, ZdbType* type, void** value)
{
    if (column < 0 || column >= recordset->query->table->columnCount)
    {
        /* Invalid column specified */
        return ZDB_RESULT_INVALID_OPERATION;
    }

    if (type != recordset->query->table->columns[column]->type)
    {
        /* Attempt to cast result to an incompatible type */
        return ZDB_RESULT_INVALID_CAST;
    }

    ZdbRow* resultRow = recordset->query->table->rows[recordset->rowIndex];
    if (ZdbEngineGetValue(recordset->query->table, resultRow, column, value) != ZDB_RESULT_SUCCESS)
    {
        /* Error getting the value for this row */
        return ZDB_RESULT_INVALID_OPERATION;
    }

    return ZDB_RESULT_SUCCESS;
}

int ZdbQueryGetInt(ZdbRecordset* recordset, int column, int* value)
{
    int* v;
    int result = ZdbQueryGetValue(recordset, column, ZdbStandardTypes->intType, (void**)&v);

    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = *v;
    }

    return result;
}

int ZdbQueryGetBoolean(ZdbRecordset* recordset, int column, int* value)
{
    int* v;
    int result = ZdbQueryGetValue(recordset, column, ZdbStandardTypes->booleanType, (void**)&v);

    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = *v;
    }

    return result;
}

int ZdbQueryGetString(ZdbRecordset* recordset, int column, char** value)
{
    char* v;
    int result = ZdbQueryGetValue(recordset, column, ZdbStandardTypes->varcharType, (void**)&v);

    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = v;
    }

    return result;
}

int ZdbQueryGetFloat(ZdbRecordset* recordset, int column, float* value)
{
    float *v;
    int result = ZdbQueryGetValue(recordset, column, ZdbStandardTypes->floatType, (void**)&v);

    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = *v;
    }

    return result;
}




