//
//  main.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/20/11.
//  Copyright 2011 GreatFoundry. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zdb.h"

void CreateTestRow(ZdbTable* table, char* name, int age, float salary, int active)
{
    ZdbColumnVal* values = malloc(4*sizeof(ZdbColumnVal));
    values[0].ignored = 1;
    strcpy(values[1].varcharVal, name);
    values[2].intVal = age;
    values[3].floatVal = salary;
    values[4].boolVal = active;
    
    ZdbRow* r;
    if (ZdbInsertRow(table, 5, values, &r) != ZDB_RESULT_SUCCESS)
    {
        printf("ERROR inserting row\n");
    }
}

ZdbDatabase* CreateTestDatabase()
{
    ZdbDatabase* db = NULL;
    ZdbCreateDatabase("Company", &db);
    
    ZdbColumn** columns = malloc(5*sizeof(ZdbColumn*));
    ZdbCreateColumn("ID", ZDB_COLTYPE_AUTOINCREMENT, &(columns[0]));
    ZdbCreateColumn("Name", ZDB_COLTYPE_VARCHAR, &(columns[1]));
    ZdbCreateColumn("Age", ZDB_COLTYPE_INT, &(columns[2]));
    ZdbCreateColumn("Salary", ZDB_COLTYPE_FLOAT, &(columns[3]));
    ZdbCreateColumn("Active", ZDB_COLTYPE_BOOLEAN, &(columns[4]));
    
    ZdbTable* t = NULL;
    ZdbCreateTable(db, "Employees", 5, columns, &t);
    
    CreateTestRow(t, "Breckin", 30, 34000.0, 1);
    CreateTestRow(t, "Bob", 22, 15600.0, 1);
    CreateTestRow(t, "Jane", 45, 45000.0, 1);
    CreateTestRow(t, "John", 35, 95600.0, 0);
    
    return db;
}

void PrintQueryResults(ZdbRecordset* rs)
{
    while(ZdbNextResult(rs))
    {
        int id = 0;
        char* name;
        int age = 0;
        float salary = 0.0f;
        int active = 0;
        
        ZdbGetIntValue(rs, 0, &id);
        ZdbGetStringValue(rs, 1, &name);
        ZdbGetIntValue(rs, 2, &age);
        ZdbGetFloatValue(rs, 3, &salary);
        ZdbGetBooleanValue(rs, 4, &active);
        
        printf("Employee %d (%s/%d): $%.2f [%s]\n",
               id,
               name,
               age,
               salary,
               active? "ACTIVE" : "TERMINATED");
    }
}

void TestBasicQuery(ZdbDatabase* db)
{
    printf("\nTesting Basic Query...\n");
    ZdbQuery* q = NULL;
    if (ZdbCreateQuery(db, &q) != ZDB_RESULT_SUCCESS)
    {
        printf("Error creating query\n");
        return;
    }
    
    if (ZdbAddQueryTable(q, db->tables[0]) != ZDB_RESULT_SUCCESS)
    {
        printf("Error adding table to query\n");
        return;
    }
    
    ZdbRecordset* rs = NULL;
    if (ZdbExecuteQuery(q, &rs) != ZDB_RESULT_SUCCESS)
    {
        printf("Error executing query\n");
        return;
    }
        
    PrintQueryResults(rs);
}

void TestOneConditionQuery(ZdbDatabase* db, int column, ZdbQueryConditionType queryType, ZdbColumnVal value, ZdbColumnType valueType)
{
    ZdbQuery* q = NULL;
    if (ZdbCreateQuery(db, &q) != ZDB_RESULT_SUCCESS)
    {
        printf("Error creating query\n");
        return;
    }
    
    if (ZdbAddQueryTable(q, db->tables[0]) != ZDB_RESULT_SUCCESS)
    {
        printf("Error adding table to query\n");
        return;
    }
    
     if (ZdbAddQueryCondition(q, queryType, column, valueType, value) != ZDB_RESULT_SUCCESS)
    {
        printf("Error adding query condition\n");
        return;
    }
    
    ZdbRecordset* rs = NULL;
    if (ZdbExecuteQuery(q, &rs) != ZDB_RESULT_SUCCESS)
    {
        printf("Error executing query\n");
        return;
    }
    
    PrintQueryResults(rs);
   
}



int main (int argc, const char * argv[])
{
    ZdbDatabase* db = CreateTestDatabase();
    
    printf("\n");
    ZdbPrintDatabase(db);
    
    printf("\n\n");
    printf("Query results:\n");
    TestBasicQuery(db);
    
    printf("\nTesting Equality Condition...\n");
    
    ZdbColumnVal value;
    value.ignored = 0;
    
    value.intVal = 1;
    TestOneConditionQuery(db, 0, ZDB_QUERY_CONDITION_EQ, value, ZDB_COLTYPE_AUTOINCREMENT);
    
    strcpy(value.varcharVal, "Jane");
    TestOneConditionQuery(db, 1, ZDB_QUERY_CONDITION_EQ, value, ZDB_COLTYPE_VARCHAR);
    
    value.intVal = 30;
    TestOneConditionQuery(db, 2, ZDB_QUERY_CONDITION_EQ, value, ZDB_COLTYPE_INT);
    
    value.floatVal = 34000.00f;
    TestOneConditionQuery(db, 3, ZDB_QUERY_CONDITION_EQ, value, ZDB_COLTYPE_FLOAT);
    
    value.boolVal = 0;
    TestOneConditionQuery(db, 4, ZDB_QUERY_CONDITION_EQ, value, ZDB_COLTYPE_BOOLEAN);
    
    /* Tests multiple results */
    value.boolVal = 1;
    TestOneConditionQuery(db, 4, ZDB_QUERY_CONDITION_EQ, value, ZDB_COLTYPE_BOOLEAN);
    
    ZdbDropDatabase(db);
    
    return 0;
}

