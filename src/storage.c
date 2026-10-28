#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "records.h"
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#endif

#include "Storage.h"

#define STORAGE_VERSION 1

static int create_data_directory(const char *path)
{
    struct stat info;

    if (stat(path, &info) == 0)
    {
#ifdef _WIN32
        if (info.st_mode & _S_IFDIR)
#else
        if (S_ISDIR(info.st_mode))
#endif
        {
            return 1;
        }

        printf("Error: Storage path exists but is not a directory.\n");
        return 0;
    }

#ifdef _WIN32
    if (_mkdir(path) == 0)
#else
    if (mkdir(path, 0775) == 0)
#endif
    {
        printf("Storage directory created: %s\n", path);
        return 1;
    }

    printf("Error: Could not create storage directory: %s\n", path);
    printf("System error: %s\n", strerror(errno));

    return 0;
}

int storage_initialize(const DatabaseConfig *database)
{
    char metadata_path[DATABASE_PATH_LENGTH + 32];
    FILE *file;

    if (database == NULL || !database->initialized)
    {
        printf("Error: Database configuration is not initialized.\n");
        return 0;
    }

    if (!create_data_directory(database->storage_path))
    {
        return 0;
    }

    int written = snprintf(
        metadata_path,
        sizeof(metadata_path),
        "%s/luraxdb.meta",
        database->storage_path
    );

    if (written < 0 || (size_t)written >= sizeof(metadata_path))
    {
        printf("Error: Metadata file path is too long.\n");
        return 0;
    }

    file = fopen(metadata_path, "rb");

    if (file != NULL)
    {
        char stored_name[DATABASE_NAME_LENGTH];
        int stored_version;

        int valid = fscanf(
            file,
            "LURAXDB\nNAME=%63[^\n]\nVERSION=%d",
            stored_name,
            &stored_version
        );

        fclose(file);

        if (valid != 2 ||
            strcmp(stored_name, database->name) != 0 ||
            stored_version != STORAGE_VERSION)
        {
            printf("Error: Existing metadata is invalid or incompatible.\n");
            return 0;
        }

        printf("Existing database metadata loaded.\n");
        return 1;
    }

    file = fopen(metadata_path, "wb");

    if (file == NULL)
    {
        printf("Error: Could not create metadata file.\n");
        printf("System error: %s\n", strerror(errno));
        return 0;
    }

    int result = fprintf(
        file,
        "LURAXDB\nNAME=%s\nVERSION=%d\n",
        database->name,
        STORAGE_VERSION
    );

    if (result < 0)
    {
        printf("Error: Could not write database metadata.\n");
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0)
    {
        printf("Error: Could not close metadata file properly.\n");
        return 0;
    }

    printf("Database metadata created successfully.\n");

    return 1;
}

void storage_show_info(const DatabaseConfig *database)
{
    printf("\nStorage Information\n");
    printf("-------------------\n");

    printf("Storage Path    : %s\n", database->storage_path);
    printf("Metadata File   : %s/luraxdb.meta\n",
           database->storage_path);
    printf("Format Version  : %d\n", STORAGE_VERSION);
    printf("Storage Type    : File-based\n");
}