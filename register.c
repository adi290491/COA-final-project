#include <stdio.h>
#include <string.h>
#include "register.h"

void init_registers()
{
    for (int i = 0; i < 24; i++)
    {
        physical_register_file[i].value = 0;
        physical_register_file[i].flag = 0;
    }
    memset(register_access_table, -1, sizeof(int) * 16);

    memset(architectural_register, 0, sizeof(int) * 16);
}

void allocate_physical_register(int rdest)
{
    for (int i = 0; i < 24; i++)
    {
        if (physical_register_file[i].status == 0)
        {

            if (register_access_table[rdest] != -1)
            {
                free_physical_register(rdest); //register freed
            }
            register_access_table[rdest] = i;
            physical_register_file[i].status = 1; //allocated

            break;
        }
    }
}

void free_physical_register(int rdest)
{
    physical_register_file[register_access_table[rdest]].status = 0;
    register_access_table[rdest] = -1;
}

void display_physical_register()
{
    printf("======================Physical Register File========================\n");
    printf("Register\tContent\tStatus\tFlag\n");

    for (int i = 0; i < 24; i++)
    {
        printf("P[%d]\t%d\t%d\t%d", i, physical_register_file[i].value, physical_register_file[i].status, physical_register_file[i].flag);
    }
}

void display_rename_table()
{
    printf("======================Rename Table========================\n");
    printf("Architectural Register\tPhysical Register\\n");

    for (int i = 0; i < 16; i++)
    {
        printf("R[%d]\tP%d", i, register_access_table[i]);
    }
}