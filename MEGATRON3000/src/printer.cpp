#include "printer.h"

void printSeparator(int numCols, const int* widths) {
    for (int i = 0; i < numCols; ++i) {
        printf("+");
        for (int j = 0; j < widths[i] + 2; ++j)
            printf("-");
    }
    printf("+\n");
}

void computeColumnWidths(const char headers[][MAX_FIELD_LEN],
                         const char rows[][MAX_FIELDS][MAX_FIELD_LEN],
                         int totalRows,
                         const int* selectedIndices,
                         int numSelected,
                         int* widths) {
    for (int i = 0; i < numSelected; ++i) {
        int col = selectedIndices[i];
        widths[i] = strlen(headers[col]);
    }

    for (int r = 0; r < totalRows; ++r) {
        for (int i = 0; i < numSelected; ++i) {
            int col = selectedIndices[i];
            int len = strlen(rows[r][col]);
            if (len > widths[i]) widths[i] = len;
        }
    }
}

void printTableSelected(const char headers[][MAX_FIELD_LEN],
                        const char rows[][MAX_FIELDS][MAX_FIELD_LEN],
                        int totalRows,
                        const int* selectedIndices,
                        int numSelected) {
    int widths[MAX_FIELDS] = {0};
    computeColumnWidths(headers, rows, totalRows, selectedIndices, numSelected, widths);

    printSeparator(numSelected, widths);

    for (int i = 0; i < numSelected; ++i) {
        int col = selectedIndices[i];
        printf("| %-*s ", widths[i], headers[col]);
    }
    printf("|\n");

    printSeparator(numSelected, widths);

    for (int r = 0; r < totalRows; ++r) {
        for (int i = 0; i < numSelected; ++i) {
            int col = selectedIndices[i];
            printf("| %-*s ", widths[i], rows[r][col]);
        }
        printf("|\n");
    }

    printSeparator(numSelected, widths);
}

void printTableAll(const char headers[][MAX_FIELD_LEN],
                   const char rows[][MAX_FIELDS][MAX_FIELD_LEN],
                   int totalRows,
                   int totalCols) {
    int selectedIndices[MAX_FIELDS];
    for (int i = 0; i < totalCols; ++i) selectedIndices[i] = i;

    printTableSelected(headers, rows, totalRows, selectedIndices, totalCols);
}

void printTableField(const char headers[][MAX_FIELD_LEN],
                     const char rows[][MAX_FIELDS][MAX_FIELD_LEN],
                     int totalRows,
                     int totalCols,
                     int fieldIndex) {
    if (fieldIndex < 0 || fieldIndex >= totalCols) {
        printf("Índice fuera de rango.\n");
        return;
    }

    int width = strlen(headers[fieldIndex]);
    for (int r = 0; r < totalRows; ++r) {
        int len = strlen(rows[r][fieldIndex]);
        if (len > width) width = len;
    }

    printf("+-%.*s-+\n", width, "--------------------------------");
    printf("| %-*s |\n", width, headers[fieldIndex]);
    printf("+-%.*s-+\n", width, "--------------------------------");

    for (int r = 0; r < totalRows; ++r)
        printf("| %-*s |\n", width, rows[r][fieldIndex]);

    printf("+-%.*s-+\n", width, "--------------------------------");
}

