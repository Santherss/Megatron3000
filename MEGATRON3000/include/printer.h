#pragma once
#include "query.h"

extern int lastColumnWidths[MAX_FIELDS];
extern int lastNumCols;

void printHeaders(const char headers[][MAX_FIELD_LEN], int numCols);
void printRow(const char campos[][MAX_FIELD_LEN], int numCols, const int* indices, bool selectAll);

