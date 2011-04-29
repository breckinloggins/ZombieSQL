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
    if (ZdbEngineInsertRow(table, 5, values, &r) != ZDB_RESULT_SUCCESS)
    {
        printf("ERROR inserting row\n");
    }
}

ZdbDatabase* CreateTestDatabase()
{
    ZdbDatabase* db = NULL;
    ZdbEngineCreateDB("Company", &db);
    
    ZdbColumn** columns = malloc(5*sizeof(ZdbColumn*));
    ZdbEngineCreateColumn("ID", ZDB_COLTYPE_AUTOINCREMENT, &(columns[0]));
    ZdbEngineCreateColumn("Name", ZDB_COLTYPE_VARCHAR, &(columns[1]));
    ZdbEngineCreateColumn("Age", ZDB_COLTYPE_INT, &(columns[2]));
    ZdbEngineCreateColumn("Salary", ZDB_COLTYPE_FLOAT, &(columns[3]));
    ZdbEngineCreateColumn("Active", ZDB_COLTYPE_BOOLEAN, &(columns[4]));
    
    ZdbTable* t = NULL;
    ZdbEngineCreateTable(db, "Employees", 5, columns, &t);
    
    CreateTestRow(t, "Breckin", 30, 34000.0, 1);
    CreateTestRow(t, "Bob", 22, 15600.0, 1);
    CreateTestRow(t, "Jane", 45, 45000.0, 1);
    CreateTestRow(t, "John", 35, 95600.0, 0);
    
    return db;
}

void PrintQueryResults(ZdbRecordset* rs)
{
    while(ZdbQueryNextResult(rs))
    {
        int id = 0;
        char* name;
        int age = 0;
        float salary = 0.0f;
        int active = 0;
        
        ZdbQueryGetInt(rs, 0, &id);
        ZdbQueryGetString(rs, 1, &name);
        ZdbQueryGetInt(rs, 2, &age);
        ZdbQueryGetFloat(rs, 3, &salary);
        ZdbQueryGetBoolean(rs, 4, &active);
        
        printf("Employee %d (%s/%d): $%.2f [%s]\n",
               id,
               name,
               age,
               salary,
               active? "ACTIVE" : "TERMINATED");
    }
}

int TestTypeSystem()
{
    if (ZdbTypeInitialize() != ZDB_RESULT_SUCCESS)
    {
        printf("Error initializing type system\n");
        return 0;
    }
    
    ZdbResult result;
    
    /* TODO: Repeat all tests with all builtin types */
    
    // Test comparisons
    int a, b, res;
    
    a = 1, b = 1;
    result = ZdbTypeCompare(ZdbStandardTypes->intType, &a, &b, &res);
    if (result != ZDB_RESULT_SUCCESS)
    {
        printf("Error comparing int types (==)\n");
        return 0;
    }
    else if (res != 0)
    {
        printf("Comparing equal int types did not return equals\n");
        return 0;
    }
  
    a = 1, b = 2;
    result = ZdbTypeCompare(ZdbStandardTypes->intType, &a, &b, &res);
    if (result != ZDB_RESULT_SUCCESS)
    {
        printf("Error comparing int types (<)\n");
        return 0;
    }
    else if (res >= 0)
    {
        printf("Comparing unequal int types did not return less than\n");
        return 0;
    }
 
    a = 2, b = 1;
    result = ZdbTypeCompare(ZdbStandardTypes->intType, &a, &b, &res);
    if (result != ZDB_RESULT_SUCCESS)
    {
        printf("Error comparing int types (>)\n");
        return 0;
    }
    else if (res <= 0)
    {
        printf("Comparing unequal int types did not return greater than\n");
        return 0;
    }
    
    // Test sizeof
    size_t size;
    result = ZdbTypeSizeof(ZdbStandardTypes->intType, NULL, &size);
    if (result != ZDB_RESULT_SUCCESS)
    {
        printf("Error determing size of int type\n");
        return 0;
    }
    else if (size != sizeof(int))
    {
        printf("Sizeof did not return the correct value for int\n");
        return 0;
    }
    
    // Test From String
    result = ZdbTypeFromString(ZdbStandardTypes->intType, "42", &a);
    if (result != ZDB_RESULT_SUCCESS)
    {
        printf("Error converting string to int type\n");
        return 0;
    }
    
    if (a != 42)
    {
        printf("FromString did not convert string to correct value\n");
        return 0;
    }
    
    // Test To String
    char str[7] = {0};
    a = 12345;
    size = 0;
    result = ZdbTypeToString(ZdbStandardTypes->intType, &a, &size, NULL);
    if (result != ZDB_RESULT_SUCCESS)
    {
        printf("Error getting length of int value\n");
        return 0;
    }
    
    if (size != 5)
    {
        printf("ToString returned invalid size\n");
        return 0;
    }
    
    result = ZdbTypeToString(ZdbStandardTypes->intType, &a, &size, str);
    if (result != ZDB_RESULT_SUCCESS)
    {
        printf("ToString returned failure\n");
        return 0;
    }
    
    if (strcmp(str, "12345"))
    {
        printf("ToString returned invalid value\n");
        return 0;
    }

    return 1;
    
    // Test Next Value
    a = 42;
    result = ZdbTypeNextValue(ZdbStandardTypes->intType, &a, &b);
    if (result != ZDB_RESULT_SUCCESS)
    {
        printf("Next value call failed\n");
        return 0;
    }
    
    if (b != 43)
    {
        printf("Next value returned an invalid value\n");
    }
}

void TestBasicQuery(ZdbDatabase* db)
{
    printf("\nTesting Basic Query...\n");
    ZdbQuery* q = NULL;
    if (ZdbQueryCreate(db, &q) != ZDB_RESULT_SUCCESS)
    {
        printf("Error creating query\n");
        return;
    }
    
    if (ZdbQueryAddTable(q, db->tables[0]) != ZDB_RESULT_SUCCESS)
    {
        printf("Error adding table to query\n");
        return;
    }
    
    ZdbRecordset* rs = NULL;
    if (ZdbQueryExecute(q, &rs) != ZDB_RESULT_SUCCESS)
    {
        printf("Error executing query\n");
        return;
    }
        
    PrintQueryResults(rs);
}

void TestOneConditionQuery(ZdbDatabase* db, int column, ZdbQueryConditionType queryType, ZdbColumnVal value, ZdbColumnType valueType)
{
    ZdbQuery* q = NULL;
    if (ZdbQueryCreate(db, &q) != ZDB_RESULT_SUCCESS)
    {
        printf("Error creating query\n");
        return;
    }
    
    if (ZdbQueryAddTable(q, db->tables[0]) != ZDB_RESULT_SUCCESS)
    {
        printf("Error adding table to query\n");
        return;
    }
    
     if (ZdbQueryAddCondition(q, queryType, column, valueType, value) != ZDB_RESULT_SUCCESS)
    {
        printf("Error adding query condition\n");
        return;
    }
    
    ZdbRecordset* rs = NULL;
    if (ZdbQueryExecute(q, &rs) != ZDB_RESULT_SUCCESS)
    {
        printf("Error executing query\n");
        return;
    }
    
    PrintQueryResults(rs);
   
}



int main (int argc, const char * argv[])
{
    if (!TestTypeSystem())
    {
        printf("Type system test failed\n");
    }
    else
    {
        printf("Type system test succeeded\n");
    }
    
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
    
    ZdbEngineDropDB(db);
    
    return 0;
}

