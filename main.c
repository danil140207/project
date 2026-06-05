#include "spreadsheet.h"
#include <stdio.h>
#include <string.h>

void print_help(const char *prog) {
    printf("Usage: %s -i INPUT [-o OUTPUT] [-h]\n", prog);
    printf("  -i, --input FILE   Input CSV file (required)\n");
    printf("  -o, --output FILE  Output CSV file (default: output.csv)\n");
    printf("  -h, --help         Show this help\n");
    printf("\n");
    printf("Formulas start with '=', support + - * / and cell references like A1, B2\n");
    printf("Example: =A1+B2*3\n");
    printf("Limits: max %d rows, %d columns\n", MAX_ROWS, MAX_COLS);
}

int main(int argc, char *argv[]) {
    const char *input = NULL;
    const char *output = "output.csv";
    
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            if (++i < argc) input = argv[i];
            else {
                fprintf(stderr, "Error: Missing argument for %s\n", argv[i-1]);
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (++i < argc) output = argv[i];
            else {
                fprintf(stderr, "Error: Missing argument for %s\n", argv[i-1]);
                return 1;
            }
        } else {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            fprintf(stderr, "Try '%s --help'\n", argv[0]);
            return 1;
        }
    }
    
    if (!input) {
        fprintf(stderr, "Error: Input file required. Use -i FILE\n");
        fprintf(stderr, "Try '%s --help'\n", argv[0]);
        return 1;
    }
    
    Spreadsheet ss;
    ss_init(&ss);
    
    printf("Loading %s...\n", input);
    if (ss_load(&ss, input) != 0) {
        fprintf(stderr, "Error: Cannot read %s\n", input);
        return 1;
    }
    
    printf("Table: %d rows, %d cols\n", ss.rows, ss.cols);
    printf("Evaluating formulas...\n");
    
    if (ss_evaluate(&ss) != 0) {
        return 1;
    }
    
    printf("Saving to %s...\n", output);
    if (ss_save(&ss, output) != 0) {
        fprintf(stderr, "Error: Cannot write to %s\n", output);
        return 1;
    }
    
    printf("Done!\n");
    return 0;
}