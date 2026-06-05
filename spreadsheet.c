#include "spreadsheet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

void ss_init(Spreadsheet *ss) {
    memset(ss, 0, sizeof(Spreadsheet));
}


int ss_load(Spreadsheet *ss, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    
    char line[8192];
    int row = 0;
    
    while (fgets(line, sizeof(line), f) && row < MAX_ROWS) {
        char *ptr = line;
        int col = 0;
        
        while (*ptr && col < MAX_COLS) {
            
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            
            char *start = ptr;
            
            while (*ptr && *ptr != ',' && *ptr != '\n' && *ptr != '\r') ptr++;
            
            int len = ptr - start;
            if (len > MAX_CELL_LEN - 1) len = MAX_CELL_LEN - 1;
            
            char cell_str[MAX_CELL_LEN];
            strncpy(cell_str, start, len);
            cell_str[len] = '\0';
            
           
            if (len == 0) {
                ss->cells[row][col].type = 0;
                ss->cells[row][col].value = 0;
            } else if (cell_str[0] == '=') {
                ss->cells[row][col].type = 2;
                strcpy(ss->cells[row][col].formula, cell_str);
                ss->cells[row][col].value = 0;
            } else {
                char *endptr;
                double val = strtod(cell_str, &endptr);
                if (*endptr == '\0') {
                    ss->cells[row][col].type = 1;
                    ss->cells[row][col].value = val;
                } else {
                    ss->cells[row][col].type = 0;
                    ss->cells[row][col].value = 0;
                }
            }
            ss->cells[row][col].visited = 0;
            ss->cells[row][col].evaluating = 0;
            
            
            if (*ptr == ',') ptr++;
            col++;
        }
        ss->cols = (col > ss->cols) ? col : ss->cols;
        row++;
    }
    ss->rows = row;
    fclose(f);
    return 0;
}


static double get_cell_value(Spreadsheet *ss, const char *ref) {
    if (!isalpha(ref[0]) || !isdigit(ref[1])) return 0;
    
    int col = toupper(ref[0]) - 'A';
    int row = atoi(ref + 1) - 1;
    
    if (row < 0 || row >= ss->rows || col < 0 || col >= ss->cols) return 0;
    
    return ss->cells[row][col].value;
}


static double parse_expr(const char *str, Spreadsheet *ss, int *pos) {
    
    double result = 0;
    char op = '+';
    
    while (1) {
       
        while (str[*pos] == ' ') (*pos)++;
        
        double val;
        
        
        if (str[*pos] == '(') {
            (*pos)++;
            val = parse_expr(str, ss, pos);
            while (str[*pos] == ' ') (*pos)++;
            if (str[*pos] == ')') (*pos)++;
        }
        
        else if (isalpha(str[*pos]) && isdigit(str[*pos + 1])) {
            char ref[10];
            int i = 0;
            while (isalpha(str[*pos + i]) || isdigit(str[*pos + i])) {
                ref[i] = str[*pos + i];
                i++;
            }
            ref[i] = '\0';
            val = get_cell_value(ss, ref);
            (*pos) += i;
        }
        
        else {
            char *end;
            val = strtod(str + (*pos), &end);
            (*pos) = end - str;
        }
        
        
        if (op == '+') result += val;
        else if (op == '-') result -= val;
        else if (op == '*') result *= val;
        else if (op == '/') result /= val;
        
        
        while (str[*pos] == ' ') (*pos)++;
        
        
        if (str[*pos] == '+' || str[*pos] == '-' || str[*pos] == '*' || str[*pos] == '/') {
            op = str[*pos];
            (*pos)++;
        } else {
            break;
        }
    }
    
    return result;
}


static int eval_cell(Spreadsheet *ss, int row, int col) {
    Cell *cell = &ss->cells[row][col];
    
    if (cell->type != 2) return 0;
    if (cell->evaluating) return -1; 
    if (cell->visited) return 0;
    
    cell->evaluating = 1;
    
    int pos = 1;  
    double result = parse_expr(cell->formula, ss, &pos);
    
    cell->evaluating = 0;
    cell->value = result;
    cell->visited = 1;
    
    return 0;
}


int ss_evaluate(Spreadsheet *ss) {
   
    for (int i = 0; i < ss->rows; i++)
        for (int j = 0; j < ss->cols; j++)
            ss->cells[i][j].visited = ss->cells[i][j].evaluating = 0;
    
    
    for (int i = 0; i < ss->rows; i++) {
        for (int j = 0; j < ss->cols; j++) {
            if (ss->cells[i][j].type == 2 && !ss->cells[i][j].visited) {
                if (eval_cell(ss, i, j) != 0) {
                    fprintf(stderr, "Error: Cyclic dependency detected!\n");
                    return -1;
                }
            }
        }
    }
    return 0;
}


int ss_save(Spreadsheet *ss, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    
    for (int i = 0; i < ss->rows; i++) {
        for (int j = 0; j < ss->cols; j++) {
            fprintf(f, "%.6g", ss->cells[i][j].value);
            if (j < ss->cols - 1) fprintf(f, ",");
        }
        fprintf(f, "\n");
    }
    fclose(f);
    return 0;
}