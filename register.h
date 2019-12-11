#include <stdio.h>

struct Physical_Register
{
    int value;
    int flag;
    int status;
};

struct Physical_Register physical_register_file[24];

int architectural_register[16];

int register_access_table[16];

void init_registers();

void allocate_physical_register(int);

void free_physical_register(int);

void display_physical_register();

void display_rename_table();