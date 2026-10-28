#ifndef STORAGE_H
#define STORAGE_H

#include "Database.h"

int storage_initialize(const DatabaseConfig *database);
void storage_show_info(const DatabaseConfig *database);

#endif