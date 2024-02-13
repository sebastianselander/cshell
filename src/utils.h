#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct ExitInfo ExitInfo;

void normalize_status(int *status);

ExitInfo exit_info_init();

struct ExitInfo {
    int exit_code;
    bool terminate;
}; 

void print_str(const char *text, size_t text_len);
void println_str(const char* text, size_t text_len);

#endif
