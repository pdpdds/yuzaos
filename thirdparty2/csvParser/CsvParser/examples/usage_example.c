#include <stdio.h>

#include "csvparser.h"

int main() {
    int i =  0;
    //                                   file, delimiter, first_line_is_header?
    CsvParser *csvparser = CsvParser_new("example_file.csv", ",", 0);
    CsvRow *row;

    while ((row = CsvParser_getRow(csvparser)) ) {
    	printf("==NEW LINE==\n");
        const char **rowFields = CsvParser_getFields(row);
        for (i = 0 ; i < CsvParser_getNumFields(row) ; i++) {
            printf("FIELD: %s\n", rowFields[i]);
        }
		printf("\n");
        CsvParser_destroy_row(row);
    }
    CsvParser_destroy(csvparser);
	
    return 0;
}