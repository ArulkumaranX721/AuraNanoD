#ifndef DATABASE_H
#define DATABASE_H

#define DATABASE_NAME_LENGTH 64
#define DATABASE_PATH_LENGTH 260

typedef struct
{
    char name[DATABASE_NAME_LENGTH];
    char storage_path[DATABASE_PATH_LENGTH];
    int initialized;
} DatabaseConfig;

void database_configure(DatabaseConfig *database);
void database_show_config(const DatabaseConfig *database);

#endif