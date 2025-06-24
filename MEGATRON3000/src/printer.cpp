#include <stdio.h>
#include <string.h>
#include "printer.h"
int lastColumnWidths[MAX_FIELDS];
int lastNumCols = 0;

void printSeparator(int numCols, const int* widths) {
    for (int i = 0; i < numCols; ++i) {
        printf("+");
        for (int j = 0; j < widths[i] + 2; ++j)
            printf("-");
    }
    printf("+\n");
}

void printHeaders(const char headers[][MAX_FIELD_LEN], int numCols) {
    int widths[MAX_FIELDS] = {0};
    for (int i = 0; i < numCols; ++i)
        widths[i] = strlen(headers[i]);

    printSeparator(numCols, widths);
    for (int i = 0; i < numCols; ++i)
        printf("| %-*s ", widths[i], headers[i]);
    printf("|\n");
    printSeparator(numCols, widths);

    // Guardar las longitudes para impresión de filas
    memcpy(lastColumnWidths, widths, sizeof(int) * numCols);
    lastNumCols = numCols;
}

void printRow(const char campos[][MAX_FIELD_LEN], int numCols, const int* indices, bool selectAll) {
    for (int i = 0; i < numCols; ++i) {
        int idx = selectAll ? i : indices[i];
        printf("| %-*s ", lastColumnWidths[i], campos[idx]);
    }
    printf("|\n");
}

