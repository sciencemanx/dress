#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

#include "libelf.h"
#include "logging.h"

#define MAX_LINE 1024

void usage(char *prog_name) {
	printf("Usage: %s <in-file> <out-file> <symbol-file>\n", prog_name);
	exit(1);
}

char *get_name(char *line) {
	char *start, *end, *at, *paren, *space, *name;
	size_t len;

	start = line;
	while (start[0] == ' ') start++;

	at = strchr(start, '@');
	paren = strchr(start, '(');
	space = strchr(start, ' ');

	end = at;
	if (end == NULL || (size_t) end > (size_t) paren) end = paren;
	if (end == NULL || (size_t) end > (size_t) space) end = space;

	if (end == NULL) return NULL;

	len = (size_t) end - (size_t) start;
	name = calloc(len + 1, sizeof(char));
	if (name == NULL) return NULL;

	strncpy(name, start, len);

	return name;
}

void *get_addr(char *line) {
	char *start;
	int base;

	start = strchr(line, '*');
	if (start == NULL) return NULL;

	start++;
	base = 10;
	if (start[0] == '0' && start[1] == 'x') {
		base = 16;
		start += 2;
	}

	return (void *) strtoul(start, NULL, base);
}

bool is_function_sym(char *line) {
	return strchr(line, '(') != NULL && strchr(line, ')') != NULL;
}

symbol_t **read_symbol_file(FILE *fp) {
	char line[MAX_LINE];
	symbol_t **symbols;
	symbol_t *symbol;
	int num_symbols;

	symbols = NULL;
	num_symbols = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		symbol = malloc(sizeof(*symbol));

		symbol->name = get_name(line);
		symbol->addr = get_addr(line);
		symbol->section = NULL;

		symbol->is_function = is_function_sym(line);

		if (symbol->name == NULL) {
			free(symbol->name);
			free(symbol);
			continue;
		}
		if (symbol->addr == NULL) {
			free(symbol);
			continue;
		}

		num_symbols++;
		symbols = realloc(symbols, num_symbols * sizeof(*symbols));

		symbols[num_symbols - 1] = symbol;
	}
	
	symbols = realloc(symbols, (num_symbols + 1) * sizeof(*symbols));
	symbols[num_symbols] = NULL;
	return symbols;
}

void print_syms(symbol_t **symbols) {
	while (*symbols != NULL) {
		if ((*symbols)->is_function) printf("function: ");
		printf("%s @ *%p\n", (*symbols)->name, (*symbols)->addr);
		symbols++;
	}
}

int main(int argc, char **argv) {
	int in_fd, out_fd;
	FILE *sym_fp;
	elf64 elf;
	char *in_file, *out_file, *symbol_file;
	symbol_t **symbols;
	struct stat st;

	// if (argc != 4) usage(argv[0]);

	in_file = "b.out";
	if (argc > 1) in_file = argv[1];
	in_fd = open(in_file, O_RDWR);

	out_file = "c.out";
	if (argc > 2) out_file = argv[2];
	out_fd = open(out_file, O_WRONLY | O_CREAT);

	symbol_file = "a.syms";
	if (argc > 3) symbol_file = argv[3];
	sym_fp = fopen(symbol_file, "r");

	info("parsing symbol file");
	symbols = read_symbol_file(sym_fp);
	print_syms(symbols);

	info("creating elf");
	create_elf64(&elf, in_fd);

	info("adding symbols to elf");
	if (add_symbols(&elf, symbols)) {
		info("writing elf to %d", out_fd);
		fstat(in_fd, &st);
		write_elf64(&elf, out_fd);
		fchmod(out_fd, st.st_mode & 07777);
	} else {
		warn("[-] failed to add symbols");
	}

	close(in_fd);
	close(out_fd);
	fclose(sym_fp);
	delete_elf64(&elf);

	return 0;
}
