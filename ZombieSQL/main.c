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

/* MAJOR HACK - REMOVE ME!!! */
int id;
int age;
char name[255];
float salary;
int active;

void CreateTestRow(ZdbTable* table, char* name, char* age, char* salary, char* active)
{
    ZdbRow* r;
    if (ZdbEngineInsertRow(table, 5, &r) != ZDB_RESULT_SUCCESS)
    {
        printf("ERROR inserting row\n");
    }
    else if (ZdbEngineUpdateRow(table, r, 5, NULL, name, age, salary, active) != ZDB_RESULT_SUCCESS)
    {
        printf("ERROR setting row values\n");
    }
}

ZdbDatabase* CreateTestDatabase()
{
    ZdbDatabase* db = NULL;
    ZdbEngineCreateDB("Company", &db);
    
    ZdbColumn** columns = malloc(5*sizeof(ZdbColumn*));
    ZdbEngineCreateColumn("ID", ZdbStandardTypes->intType, 1, &(columns[0]));
    ZdbEngineCreateColumn("Name", ZdbStandardTypes->varcharType, 0, &(columns[1]));
    ZdbEngineCreateColumn("Age", ZdbStandardTypes->intType, 0, &(columns[2]));
    ZdbEngineCreateColumn("Salary", ZdbStandardTypes->floatType, 0, &(columns[3]));
    ZdbEngineCreateColumn("Active", ZdbStandardTypes->booleanType, 0, &(columns[4]));
    
    ZdbTable* t = NULL;
    ZdbEngineCreateTable(db, "Employees", 5, columns, &t);
    
    CreateTestRow(t, "Breckin", "30", "34000.0", "1");
    CreateTestRow(t, "Bob", "22", "15600.0", "1");
    CreateTestRow(t, "Jane", "45", "45000.0", "1");
    CreateTestRow(t, "John", "35", "95600.0", "0");
    
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

int TestTypeCopyValue()
{
    int i1, i2;
    float f1, f2;
    int b1, b2;
    
    char s1[ZDB_LIMIT_VARCHAR], s2[ZDB_LIMIT_VARCHAR];
    
    printf("Testing Type CopyValue functions...\n");
    
    printf("\tint... ");
    i1 = 10; i2 = 0;
    ZdbTypeCopy(ZdbStandardTypes->intType, &i2, &i1);
    if (i2 != 10)
    {
        printf("FAIL (i2 is %d, should be %d)\n", i2, i1);
        return 0;
    }
    printf("PASS\n");

    printf("\tfloat... ");
    f1 = 11.2; f2 = 0;
    ZdbTypeCopy(ZdbStandardTypes->floatType, &f2, &f1);
    if (abs(f2 - f1) > 0.1)
    {
        printf("FAIL (f2 is %f, should be %f)\n", f2, f1);
        return 0;
    }
    printf("PASS\n");
    
    printf("\tboolean... ");
    b1 = 1; b2 = 0;
    ZdbTypeCopy(ZdbStandardTypes->booleanType, &b2, &b1);
    if (!b2)
    {
        printf("FAIL (b2 is %d, should be %d)\n", b2, b1);
        return 0;
    }
    printf("PASS\n");
    
    printf("\tvarchar... ");
    strcpy(s1, "foo"), strcpy(s2, "");
    ZdbTypeCopy(ZdbStandardTypes->varcharType, &s2, &s1);
    if (strcmp(s2, "foo"))
    {
        printf("FAIL (s2 is %s, should be %s)\n", s2, s1);
        return 0;
    }
    printf("PASS\n");
    
    
    return 1; /* FIXME */
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
    
    if (ZdbTypeSizeof(ZdbStandardTypes->intType, &a, &size) != ZDB_RESULT_ERR_UNSUPPORTED)
    {
        printf("Did not receive UNSUPPORTED error message for passing in explicit type value to sizeof\n");
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
    
    if (!TestTypeCopyValue())
    {
        printf("Type system copy value failed\n");
        return 0;
    }
    
    return 1;
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

void TestOneConditionQuery(ZdbDatabase* db, int column, ZdbQueryConditionType queryType, void* value, ZdbType* valueType)
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
    
    printf("Test...\n");
    PrintQueryResults(rs);
   
}

void UpdateRowTestHelper(const char* testName, ZdbDatabase* db, int table, int queryColumn, void* queryValue, int updateColumn, void* updateValue)
{
    /* Most of this code will likely turn into the Update command code */
    
    ZdbQuery* q;
    ZdbRecordset* rs;
    ZdbQueryCreate(db, &q);
    
    ZdbType* queryColumnType = db->tables[table]->columns[queryColumn]->type;
    ZdbType* updateColumnType = db->tables[table]->columns[updateColumn]->type;
    
    /* Get some names of things to make diagnostic messages less painful */
    char queryValueString[ZDB_LIMIT_VARCHAR], tableName[ZDB_LIMIT_VARCHAR], columnName[ZDB_LIMIT_VARCHAR], originalValueString[ZDB_LIMIT_VARCHAR], updateValueString[ZDB_LIMIT_VARCHAR];
    
    size_t length = ZDB_LIMIT_VARCHAR;
    ZdbTypeToString(queryColumnType, queryValue, &length, queryValueString);
    strcpy(tableName, db->tables[table]->name);
    strcpy(columnName, db->tables[table]->columns[updateColumn]->name);
    length = ZDB_LIMIT_VARCHAR;
    ZdbTypeToString(updateColumnType, updateValue, &length, updateValueString);
    
    printf("%s - Testing update of %s(%s): ", testName, tableName, columnName);
    
    ZdbQueryAddTable(q, db->tables[table]);
    ZdbQueryAddCondition(q, ZDB_QUERY_CONDITION_EQ, queryColumn, queryColumnType, queryValue);
    ZdbQueryExecute(q, &rs);
    if (!ZdbQueryNextResult(rs))
    {
        printf("FAIL (No rows returned)\n");
        return;
    }
    
    /* Get the current value so we can print it*/
    void* originalValue;
    length = ZDB_LIMIT_VARCHAR;
    ZdbQueryGetValue(rs, updateColumn, updateColumnType, &originalValue);
    ZdbTypeToString(updateColumnType, originalValue, &length, originalValueString);
    
    printf("%s => %s ...", originalValueString, updateValueString);
    
    int rowId = -1;
    
    /* Collect the values for this row as a string */
    void** values = calloc(db->tables[table]->columnCount, sizeof(void*));
    
    int i;
    for (i = 0; i < db->tables[table]->columnCount; i++)
    {
        void* value;
        ZdbType* thisType = db->tables[table]->columns[i]->type;
        ZdbQueryGetValue(rs, i, thisType, &value);
        
        if (rowId == -1 && db->tables[table]->columns[i]->autoincrement)
        {
            /* For our test DB, we can assume the first auto increment column is an 
              int row id that matches the row index */
            rowId = *(int*)value;
        }
        
        if (i == updateColumn)
        {
            values[i] = updateValue;
        }
        else
        {
            values[i] = value;
        }
    }
    
    
    ZdbRow* row = db->tables[table]->rows[rowId];
    ZdbEngineUpdateRowValues(db->tables[table], row, db->tables[table]->columnCount, values);
    
    void* updatedValue;
    ZdbQueryGetValue(rs, updateColumn, updateColumnType, &updatedValue);
    int result = 0;
    ZdbTypeCompare(updateColumnType, updateValue, updatedValue, &result);
    if (result != 0)
    {
        printf("FAILED (Updated value does not match)\n");
    }
    
    free(values);
    
    printf("SUCCESS\n");
}

void TestBasicRowUpdate(ZdbDatabase* db)
{
    
    /* Legal wants Bob's official name changed to Robert in the DB */
    int i = 1;
    UpdateRowTestHelper("UPDATE_BOB", db, 0, 0, &i, 1, "Robert");
    
    /* John gets rehired */
    i = 3;
    int active = 1;
    UpdateRowTestHelper("UPDATE_JOHN", db, 0, 0, &i, 4, &active);
    
    /* Jane got older */
    i = 2;
    int age = 46;
    UpdateRowTestHelper("UPDATE_JANE", db, 0, 0, &i, 2, &age);
    
    /* And I got a raise! */
    i = 0;
    float salary = 80000;
    UpdateRowTestHelper("UPDATE_BRECKIN", db, 0, 0, &i, 3, &salary);
    
}


int main (int argc, const char * argv[])
{
    if (ZdbTypeInitialize() != ZDB_RESULT_SUCCESS)
    {
        printf("Could not initialize type system\n");
    }
    
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
    
    
    id = 1;
    TestOneConditionQuery(db, 0, ZDB_QUERY_CONDITION_EQ, &id, ZdbStandardTypes->intType);
    
    strcpy(name, "Jane");
    TestOneConditionQuery(db, 1, ZDB_QUERY_CONDITION_EQ, &name, ZdbStandardTypes->varcharType);
    
    age = 30;
    TestOneConditionQuery(db, 2, ZDB_QUERY_CONDITION_EQ, &age, ZdbStandardTypes->intType);
    
    salary = 34000.00f;
    TestOneConditionQuery(db, 3, ZDB_QUERY_CONDITION_EQ, &salary, ZdbStandardTypes->floatType);
    
    active = 0;
    TestOneConditionQuery(db, 4, ZDB_QUERY_CONDITION_EQ, &active, ZdbStandardTypes->booleanType);
    
    /* Tests multiple results */
    active = 1;
    TestOneConditionQuery(db, 4, ZDB_QUERY_CONDITION_EQ, &active, ZdbStandardTypes->booleanType);
    
    TestBasicRowUpdate(db);
    
    ZdbEngineDropDB(db);
    
    return 0;
}

