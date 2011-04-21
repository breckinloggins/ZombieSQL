//
//  main.c
//  BraindeadSQL
//
//  Created by Benjamin Loggins on 4/20/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LIMIT_VARCHAR       255
#define LIMIT_COLUMNS       32

#define COLTYPE_BOOLEAN     1
#define COLTYPE_INT         2
#define COLTYPE_FLOAT       3
#define COLTYPE_VARCHAR     4

#define ROW_CHUNKS          128

typedef int ColumnType;

typedef struct Column_t
{
    ColumnType type;
    char name[LIMIT_VARCHAR];
} Column;

typedef union ColumnVal_t
{
    char boolVal;
    int intVal;
    float floatVal;
    char varcharVal[LIMIT_VARCHAR];    
} ColumnVal;

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

Row* InsertInto(Table* table, int columnCount, ColumnVal* values)
{
    int i;
    Row* r = malloc(sizeof(Row));
    
    for (i = 0; i < columnCount; i++)
    {
        r->values[i] = values[i];
    }
    
    if (table->freeRowsLeft == 0)
    {
        /* We're out of free rows, so allocate some more */
        table->rows = realloc(table->rows, (table->rowCount + ROW_CHUNKS)*sizeof(Row*));
        table->freeRowsLeft = ROW_CHUNKS;
    }
    
    table->rows[table->rowCount++] = r;
    table->freeRowsLeft--;
    
    return r;
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
    
    Column** columns = malloc(4*sizeof(Column*));
    columns[0] = CreateColumn("ID", COLTYPE_INT);
    columns[1] = CreateColumn("Name", COLTYPE_VARCHAR);
    columns[2] = CreateColumn("Salary", COLTYPE_FLOAT);
    columns[3] = CreateColumn("Active", COLTYPE_BOOLEAN);
    
    Table* t = CreateTable("Employees", 4, columns);
    
    printf("Enter number of employess: ");
    scanf("%d", &numEmployees);
    for (i = 0; i < numEmployees; i++)
    {
        ColumnVal* values = malloc(4*sizeof(ColumnVal));
        printf("Employee %d ID: ", i);
        scanf("%d", &(values[0].intVal));
        
        printf("Employee %d Name: ", i);
        scanf("%s", &(values[1].varcharVal));
        
        printf("Employee %d Salary: ", i);
        scanf("%f", &(values[2].floatVal));
        
        values[3].boolVal = 1;
    
        InsertInto(t, 4, values);
    }
    
    
    // insert code here...
    printf("\n\nEmployees\n");
    PrintTable(t);
    
    DropTable(t);
    
    return 0;
}

