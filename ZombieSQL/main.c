//
//  main.c
//  BraindeadSQL
//
//  Created by Benjamin Loggins on 4/20/11.
//  Copyright 2011 GreatFoundry. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LIMIT_VARCHAR       255
#define LIMIT_COLUMNS       32

#define COLTYPE_BOOLEAN         1
#define COLTYPE_INT             2
#define COLTYPE_FLOAT           3
#define COLTYPE_VARCHAR         4
#define COLTYPE_AUTOINCREMENT   5

#define ROW_CHUNKS          128

#define RESULT_SUCCESS             0        /* No error ocurred */
#define RESULT_ERR_AUTOINCREMENT   -1       /* Attempt to update or set an autoincrement value */

typedef int ColumnType;
typedef int Result;

typedef struct ColumnVal_t
{
    int ignored;                        /* Set if the database should ignore whatever the value is (e.g. for autoincrement) */
    union 
    {
        char boolVal;
        int intVal;
        float floatVal;
        char varcharVal[LIMIT_VARCHAR];        
    };
} ColumnVal;

typedef struct Column_t
{
    ColumnType type;
    char name[LIMIT_VARCHAR];
    ColumnVal* lastInsertedValue;       /* Used mainly to track the last autoincrement number */
} Column;

typedef struct Row_t
{
    ColumnVal values[LIMIT_COLUMNS];
} Row;


typedef struct Table_t
{
    char name[LIMIT_VARCHAR];
    int columnCount;
    int rowCount;
    int freeRowsLeft;
    
    Column** columns;
    Row** rows;
} Table;

Column* CreateColumn(char* name, ColumnType type)
{
    Column* c = malloc(sizeof(Column));
    c->type = type;
    strcpy(c->name, name);
    c->lastInsertedValue = NULL;
    
    return c;
}

Table* CreateTable(char* name, int columnCount, Column** columnDefs)
{
    int i;
    Table* t = malloc(sizeof(Table));
    
    strcpy(t->name, name);
    t->columnCount = columnCount;
    
    t->columns = malloc(columnCount * sizeof(Column*));
    for (i = 0; i < columnCount; i++)
    {
        t->columns[i] = columnDefs[i];
    }
    
    t->rows = NULL; 
    t->freeRowsLeft = 0;
    t->rowCount = 0;
    
    return t;
}

void DropTable(Table* table)
{
    int i;
    for (i = 0; i < table->columnCount; i++)
    {
        free(table->columns[i]);
    }
    free(table->columns);
    table->columns = NULL;
    table->columnCount = 0;
    
    for (i = 0; i < table->rowCount; i++)
    {
        free(table->rows[i]);
    }
    free(table->rows);
    table->rows = NULL;
    table->rowCount = 0;
}

Result InsertInto(Table* table, int columnCount, ColumnVal* values, Row** row)
{
    int i;
    Row* r = malloc(sizeof(Row));
    
    for (i = 0; i < columnCount; i++)
    {
        if (table->columns[i]->type == COLTYPE_AUTOINCREMENT && !values[i].ignored)
        {
            /* You can't set an auto increment column */
            free(r);
            return RESULT_ERR_AUTOINCREMENT;
        }
        else if (table->columns[i]->type == COLTYPE_AUTOINCREMENT)
        {
            int lastValue = -1;
            if (table->columns[i]->lastInsertedValue != NULL)
            {
                lastValue = table->columns[i]->lastInsertedValue->intVal;
            }
            
            values[i].intVal = ++lastValue;
            r->values[i] = values[i];
        }
        else if (!values[i].ignored)
        {
            r->values[i] = values[i];
        }
        
        table->columns[i]->lastInsertedValue = &(values[i]);
    }
    
    if (table->freeRowsLeft == 0)
    {
        /* We're out of free rows, so allocate some more */
        table->rows = realloc(table->rows, (table->rowCount + ROW_CHUNKS)*sizeof(Row*));
        table->freeRowsLeft = ROW_CHUNKS;
    }
    
    table->rows[table->rowCount++] = r;
    table->freeRowsLeft--;
    
    *row = r;
    
    return RESULT_SUCCESS;
}

void PrintColumn(Column* column)
{
    printf("%s", column->name);
}

void PrintColumnValue(ColumnType type, ColumnVal* value)
{
    switch(type)
    {
        case COLTYPE_BOOLEAN:
            printf("%s", value->boolVal? "TRUE" : "FALSE");
            break;
        case COLTYPE_INT:
        case COLTYPE_AUTOINCREMENT:
            printf("%d", value->intVal);
            break;
        case COLTYPE_FLOAT:
            printf("%f", value->floatVal);
            break;
        case COLTYPE_VARCHAR:
            printf("%s", value->varcharVal);
            break;
        default:
            printf("ERROR IN PRINTCOLUMNVALUE\n");
            break;
    }
}

void PrintRow(Row* row, Column** columns, int columnCount)
{
    int i;
    for (i = 0; i < columnCount; i++)
    {
        PrintColumnValue(columns[i]->type, &row->values[i]);
        printf("\t");
    }
}

void PrintTable(Table* table)
{
    int i;
    for (i = 0; i < table->columnCount; i++)
    {
        PrintColumn(table->columns[i]);
        printf("\t");
    }
    
    printf("\n\n");
    
    for (i = 0; i < table->rowCount; i++)
    {
        PrintRow(table->rows[i], table->columns, table->columnCount);
        printf("\n");
    }
}

int main (int argc, const char * argv[])
{
    int numEmployees, i;
    
    Column** columns = malloc(5*sizeof(Column*));
    columns[0] = CreateColumn("ID", COLTYPE_AUTOINCREMENT);
    columns[1] = CreateColumn("Name", COLTYPE_VARCHAR);
    columns[2] = CreateColumn("Age", COLTYPE_INT);
    columns[3] = CreateColumn("Salary", COLTYPE_FLOAT);
    columns[4] = CreateColumn("Active", COLTYPE_BOOLEAN);
    
    Table* t = CreateTable("Employees", 5, columns);
    
    printf("Enter number of employess: ");
    scanf("%d", &numEmployees);
    for (i = 0; i < numEmployees; i++)
    {
        ColumnVal* values = malloc(4*sizeof(ColumnVal));
        
        values[0].ignored = 1;
        
        printf("Employee %d Name: ", i);
        scanf("%s", &(values[1].varcharVal));
        
        printf("Employee %d Age: ", i);
        scanf("%d", &(values[2].intVal));
        
        printf("Employee %d Salary: ", i);
        scanf("%f", &(values[3].floatVal));
        
        values[4].boolVal = 1;
        
        Row* r;
        if (InsertInto(t, 4, values, &r) != RESULT_SUCCESS)
        {
            printf("ERROR inserting row\n");
        }
    }
    
    printf("\n\nEmployees\n");
    PrintTable(t);
    
    DropTable(t);
    
    return 0;
}

