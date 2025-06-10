#include "query.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

char g_schemaPath[512] = {0};

void init_query(const char* schemaPath) {
    strncpy(g_schemaPath, schemaPath, sizeof(g_schemaPath) - 1);
    g_schemaPath[sizeof(g_schemaPath) - 1] = '\0';
}

int split(const char* str, char delim, char output[MAX_FIELDS][MAX_FIELD_LEN]) {
    int count = 0;
    const char* start = str;
    const char* ptr = str;
    while (*ptr != '\0' && count < MAX_FIELDS) {
        if (*ptr == delim) {
            int len = ptr - start;
            if (len >= MAX_FIELD_LEN) len = MAX_FIELD_LEN - 1;
            strncpy(output[count], start, len);
            output[count][len] = '\0';
            count++;
            start = ptr + 1;
        }
        ptr++;
    }
    if (count < MAX_FIELDS && *start != '\0') {
        int len = ptr - start;
        if (len >= MAX_FIELD_LEN) len = MAX_FIELD_LEN - 1;
        strncpy(output[count], start, len);
        output[count][len] = '\0';
        count++;
    }
    return count;
}

void toLower(char* str) {
    while (*str) {
        *str = (char)tolower(*str);
        str++;
    }
}

bool evalCondition(const char* fieldValue, const char* op, const char* condValue, char* errorOutput) {
    if (fieldValue[0] == '\0') return false;

    double valNum, condNum;
    int isValNum = sscanf(fieldValue, "%lf", &valNum) == 1;
    int isCondNum = sscanf(condValue, "%lf", &condNum) == 1;

    if (isValNum && isCondNum) {
        if (strcmp(op, "=") == 0) return valNum == condNum;
        if (strcmp(op, ">") == 0) return valNum > condNum;
        if (strcmp(op, "<") == 0) return valNum < condNum;
        if (strcmp(op, ">=") == 0) return valNum >= condNum;
        if (strcmp(op, "<=") == 0) return valNum <= condNum;
        if (strcmp(op, "!=") == 0) return valNum != condNum;
    }

    if (strcmp(op, "=") == 0) return strcmp(fieldValue, condValue) == 0;
    if (strcmp(op, "!=") == 0) return strcmp(fieldValue, condValue) != 0;

    snprintf(errorOutput, 128, "Error: No se permiten comparaciones como '%s' entre cadenas.", op);
    return false;
}

void mostrarTamanoArchivo(const char* rutaArchivo) {
    struct stat st;
    if (stat(rutaArchivo, &st) == 0) {
        printf("Tama\u00f1o del archivo '%s': %ld bytes\n", rutaArchivo, st.st_size);
    } else {
        perror("Error al obtener el tama\u00f1o del archivo");
    }
}

