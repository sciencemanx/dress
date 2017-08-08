#pragma once

#include <elf.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct sections {
	Elf64_Shdr *hdrs;
	size_t count;
	char *str_tbl;
};

struct segments {
	Elf64_Phdr *hdrs;
	size_t count;
};

typedef struct {
	Elf64_Sym *symbol_tbl;
	size_t num_symbols;
	char *str_tbl;
} symbols;

typedef struct symbol {
	char *name;
	void *addr;
	bool is_function;
	char *section;
	long size;
} symbol_t;

typedef struct {
	Elf64_Ehdr *elf_hdr;
	Elf64_Phdr *program_hdrs;
	size_t num_program_hdrs;
	Elf64_Shdr *section_hdrs;
	size_t num_section_hdrs;
	char *section_str_tbl;
	uint8_t *file;
	size_t file_size;
	size_t sh_info_index;
} elf64;


bool create_elf64(elf64 *elf, int fd);
bool write_elf64(elf64 *elf, int fd);
bool delete_elf64(elf64 *elf);

void print_sections(elf64 *elf);
void print_symbols(elf64 *elf);

bool add_symbols(elf64 *elf, symbol_t **symbols);
