#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>

cJSON *load_json(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(file_size + 1);

    if (!data)
    {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    fread(data, 1, file_size, file);
    data[file_size] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(data);
    free(data);
    return json;
}