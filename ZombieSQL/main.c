//
//  main.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/20/11.
//  Copyright 2011 GreatFoundry. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "engine.h"


int main (int argc, const char * argv[])
{
    int numEmployees, i;
    
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
    
    printf("Enter number of employess: ");
    scanf("%d", &numEmployees);
    for (i = 0; i < numEmployees; i++)
    {
        ZdbColumnVal* values = malloc(4*sizeof(ZdbColumnVal));
        
        values[0].ignored = 1;
        
        printf("Employee %d Name: ", i);
        scanf("%s", &(values[1].varcharVal));
        
        printf("Employee %d Age: ", i);
        scanf("%d", &(values[2].intVal));
        
        printf("Employee %d Salary: ", i);
        scanf("%f", &(values[3].floatVal));
        
        values[4].boolVal = 1;
        
        ZdbRow* r;
        if (ZdbInsertRow(t, 4, values, &r) != ZDB_RESULT_SUCCESS)
        {
            printf("ERROR inserting row\n");
        }
    }
    
    printf("\n");
    ZdbPrintDatabase(db);
    
    ZdbDropDatabase(db);
    
    return 0;
}

