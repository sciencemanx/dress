#include "libelf.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ENTRY(x) case x: return #x + 4;
char *sh_type_str(uint32_t sh_type) {
	switch (sh_type) {
		ENTRY(SHT_NULL);
		ENTRY(SHT_PROGBITS);
		ENTRY(SHT_SYMTAB);
		ENTRY(SHT_STRTAB);
		ENTRY(SHT_RELA);
		ENTRY(SHT_HASH);
		ENTRY(SHT_NOTE);
		ENTRY(SHT_NOBITS);
		ENTRY(SHT_REL);
		ENTRY(SHT_SHLIB);
		ENTRY(SHT_DYNSYM);
		ENTRY(SHT_LOPROC);
		ENTRY(SHT_HIPROC);
		ENTRY(SHT_LOUSER);
		ENTRY(SHT_HIUSER);
		default:
			return "UNKNOWN";
	}
}

bool update_elf64(elf64 *elf) {
	int i;
	Elf64_Shdr section_str_tbl_hdr;
	Elf64_Shdr section_header;

	printf("updating elf\n");

	elf->elf_hdr = (Elf64_Ehdr *) elf->file;

	elf->program_hdrs = (Elf64_Phdr *) &(elf->file)[elf->elf_hdr->e_phoff];
	elf->num_program_hdrs = elf->elf_hdr->e_phnum;

	elf->section_hdrs = (Elf64_Shdr *) &(elf->file)[elf->elf_hdr->e_shoff];
	elf->num_section_hdrs = elf->elf_hdr->e_shnum;

	section_str_tbl_hdr = elf->section_hdrs[elf->elf_hdr->e_shstrndx];
	elf->section_str_tbl = &(elf->file)[section_str_tbl_hdr.sh_offset];

	printf("section name index: %d\n", elf->elf_hdr->e_shstrndx);
	printf("shstr hdr located at %p\n", &elf->section_hdrs[elf->elf_hdr->e_shstrndx]);
	printf("shstrtbl offset: 0x%lx\n", section_str_tbl_hdr.sh_offset);
	printf("file start: %p, end: %p\n", elf->file, elf->file + elf->file_size);
}

bool create_elf64(elf64 *elf, int fd) {
	struct stat st;

	fstat(fd, &st);

	// elf->file = mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	elf->file = malloc(st.st_size);
	read(fd, elf->file, st.st_size);
	elf->file_size = st.st_size;

	update_elf64(elf);

	return true;
}

bool write_elf64(elf64 *elf, int fd) {
	int written, n;

	// n = elf->file_size;
	// while (written > 0) {
	// 	written = write(fd, elf->file, n);
	// 	if (written <= 0) return false;
	// 	n -= written;
	// }

	write(fd, elf->file, elf->file_size);

	return true;
}

bool delete_elf64(elf64 *elf) {
	// munmap(elf->file, elf->file_size);
	free(elf->file);

	return true;
}

Elf64_Shdr *get_symbol_tbl_hdr(elf64 *elf) {
	int i;

	for (i = 0; i < elf->num_section_hdrs; i++) {
		if (elf->section_hdrs[i].sh_type == SHT_SYMTAB) 
			return &elf->section_hdrs[i];
	}

	return NULL;
}

Elf64_Shdr *get_linked_hdr(elf64 *elf, Elf64_Shdr *section_hdr) {
	uint32_t link_num;

	link_num = section_hdr->sh_link;

	if (link_num == 0) return NULL;
	else return &elf->section_hdrs[link_num];
}

void print_sections(elf64 *elf) {
	int i;
	Elf64_Shdr sh_hdr;

	printf("section_headers: %p\n", elf->section_hdrs);

	printf("Sections:\n");
	printf(" NUM | %20s | %15s | LINK\n", "NAME", "TYPE");

	for (i = 0; i < elf->num_section_hdrs; i++) {
		sh_hdr = elf->section_hdrs[i];
		printf("  %2d | %20s | %15s | %3d\n",
			i, &(elf->section_str_tbl)[sh_hdr.sh_name], sh_type_str(sh_hdr.sh_type), sh_hdr.sh_link);
	}
}

void print_symbols(elf64 *elf) {
	int i, n;
	Elf64_Sym symbol;
	Elf64_Shdr *symbol_tbl_hdr, *symbol_str_tbl_hdr;
	Elf64_Sym *symbol_tbl;
	char *symbol_str_tbl;

	symbol_tbl_hdr = get_symbol_tbl_hdr(elf);

	printf("Symbols:\n");

	if (symbol_tbl_hdr == NULL) {
		printf("  no symbols\n");
		return;
	}

	symbol_tbl = (Elf64_Sym *) &elf->file[symbol_tbl_hdr->sh_offset];

	symbol_str_tbl_hdr = get_linked_hdr(elf, symbol_tbl_hdr);

	if (symbol_str_tbl_hdr == NULL) {
		printf("  error: no strings for symbols");
		return;
	}

	symbol_str_tbl = (char *) &elf->file[symbol_str_tbl_hdr->sh_offset];

	printf("%lu / %lu\n", symbol_tbl_hdr->sh_size, symbol_tbl_hdr->sh_entsize);

	n = symbol_tbl_hdr->sh_size / symbol_tbl_hdr->sh_entsize;
	for (i = 0; i < n; i++) {
		symbol = symbol_tbl[i];
		printf("  name: %40s | value: 0x%08lx | size: %3lu\n", 
			&symbol_str_tbl[symbol.st_name], symbol.st_value, symbol.st_size);
	}
}

char *get_section_name(elf64 *elf, Elf64_Shdr *section_hdr) {
	// printf("in get_section_name\n");
	// printf("name index: 0x%x\n", section_hdr->sh_name);
	// printf("name: %s\n", &elf->section_str_tbl[section_hdr->sh_name]);
	// printf("address of file %p, address of string %p\n",
		// elf->file, &elf->section_str_tbl[section_hdr->sh_name]);
	return &elf->section_str_tbl[section_hdr->sh_name];
}

bool increase_file_size(elf64 *elf, size_t inc) {
	size_t new_file_size;
	uint8_t *new_file;

	printf("[+] increasing file size from %lu ", elf->file_size);

	new_file_size = elf->file_size + inc;

	printf("to %lu (by %lu bytes)\n", new_file_size, inc);

	new_file = realloc(elf->file, new_file_size);
	if (new_file == NULL) {
		printf("[-] failed to reallocate file\n");
		return false;
	}

	if (elf->file != new_file) {
		printf("[+] file now in new location\n");
	}

	elf->file = new_file;
	elf->file_size = new_file_size;

	update_elf64(elf); // probably dont need this

	return true;
}

int get_section_hdr_index(elf64 *elf, char *name) {
	int i;

	printf("in get_section_hdr_index (%lu hdrs)\n", elf->num_section_hdrs);


	for (i = 0; i < elf->num_section_hdrs; i++) {
		printf("get %d, name: %s\n", i, get_section_name(elf, &elf->section_hdrs[i]));
		if (strcmp(get_section_name(elf, &elf->section_hdrs[i]), name) == 0) {
			return i;
		}
	}

	return -1;
}

Elf64_Shdr *get_section_hdr(elf64 *elf, char *name) {
	int i;

	// printf("in get_section_hdr\n");

	i = get_section_hdr_index(elf, name);

	// printf("got section_hdr index\n");
	if (i == -1) return NULL;
	else return &elf->section_hdrs[i];
}

// expands a nonallocated section
bool expand_section(elf64 *elf, char *name, size_t increment) {
	int i;
	Elf64_Shdr *section_hdr, *expanded_section_hdr;
	Elf64_Phdr *program_hdr;
	uint8_t *expanded_section_end;
	size_t displaced;

	printf("[+] expanding %s\n", name);

	expanded_section_hdr = get_section_hdr(elf, name);
	if (expanded_section_hdr == NULL) {
		printf("[-] unable to find section header %s\n", name);
		return false;
	}

	displaced = elf->file_size - expanded_section_hdr->sh_offset - expanded_section_hdr->sh_size;

	if (!increase_file_size(elf, increment)) {
		printf("[-] unable to increase file size\n");
		return false;
	}

	if (elf->elf_hdr->e_shoff >= expanded_section_hdr->sh_offset) {
		printf("[+] increasing shoff 0x%lx to ", elf->elf_hdr->e_shoff);
		elf->elf_hdr->e_shoff += increment;
		printf("0x%lx\n", elf->elf_hdr->e_shoff);
	}

	for (i = 0; i < elf->num_section_hdrs; i++) {
		section_hdr = &elf->section_hdrs[i];
		if (section_hdr->sh_offset > expanded_section_hdr->sh_offset) {
			printf("[+] expanding section hdr (i: %d)\n", i);
			section_hdr->sh_offset += increment;
		}
	}

	for (i = 0; i < elf->num_program_hdrs; i++) { //todo implement alignment checks
		program_hdr = &elf->program_hdrs[i];
		if (program_hdr->p_offset >= expanded_section_hdr->sh_offset) {
			printf("[+] expanding segment hdr (i: %d)\n", i);
			program_hdr->p_offset += increment;
		}
	}

	expanded_section_end = &elf->file[expanded_section_hdr->sh_offset] + expanded_section_hdr->sh_size;

	memmove(expanded_section_end + increment, expanded_section_end, displaced);
	printf("moving %p to %p (0x%lx bytes)\n", 
		expanded_section_end, expanded_section_end + increment, displaced);
	printf("       %p    %p\n", expanded_section_end + displaced, expanded_section_end + increment + displaced);
	memset(expanded_section_end, 0, increment);

	update_elf64(elf);

	get_section_hdr(elf, name)->sh_size += increment;

	return true;
}

// appends to a nonallocated section
bool append_to_section(elf64 *elf, char *name, uint8_t *buf, size_t len) {
	Elf64_Shdr *section_hdr;
	uint64_t section_end_offset;
	uint8_t *section_end;

	section_hdr = get_section_hdr(elf, name);
	if (section_hdr == NULL) {
		printf("[-] unable to find section header\n");
		return false;
	}

	printf("asdfasdfa\n");

	section_end_offset = section_hdr->sh_offset + section_hdr->sh_size;

	if (!expand_section(elf, name, len)) {
		printf("[-] unable to expand section\n");
		return false;
	}

	section_end = &elf->file[section_end_offset];

	memcpy(section_end, buf, len);

	return true;
}

// creates a nonallocated section -- choses the last space in the file before
// the section header table to insert it
bool create_section(elf64 *elf, char *name) {
	Elf64_Shdr new_hdr;
	uint8_t *new_file;
	size_t new_file_size;
	Elf64_Shdr *shstrtab_hdr;
	uint32_t name_offset;

	shstrtab_hdr = get_section_hdr(elf, ".shstrtab");

	if (shstrtab_hdr == NULL) {
		printf("[-] unable to find .shstrtab\n");
		return false;
	}

	name_offset = shstrtab_hdr->sh_size;

	if (!append_to_section(elf, ".shstrtab", name, strlen(name) + 1)) {
		printf("[-] failed to add section name to shstrtab\n");
		return false;
	}

	new_hdr.sh_name = name_offset;
	new_hdr.sh_type = SHT_NULL; // change this after creation
	new_hdr.sh_flags = 0;
	new_hdr.sh_addr = 0;
	new_hdr.sh_offset = elf->elf_hdr->e_shoff; // inserts before section header
	new_hdr.sh_size = 0;
	new_hdr.sh_link = 0; // change this after creation
	new_hdr.sh_info = 0; // section type dependent
	new_hdr.sh_addralign = 0;
	new_hdr.sh_entsize = 0; // change this after creation;

	if (!increase_file_size(elf, sizeof(new_hdr))) {
		printf("[-] failed to increase file size\n");
		return false;
	}
	
	elf->elf_hdr->e_shnum++;
	elf->section_hdrs[elf->num_section_hdrs] = new_hdr;
	elf->num_section_hdrs++;

	return true;
}

bool add_symbols(elf64 *elf, symbol_t **symbols) {
	size_t total_len, num_symbols;
	symbol_t **tracer;
	char *sym_str_tbl, *str_tbl_tracer;
	Elf64_Shdr *symtab, *strtab;
	Elf64_Sym *sym_tab;
	int i;

	symtab = get_section_hdr(elf, ".symtab");
	if (symtab != NULL) strtab = get_linked_hdr(elf, symtab);
	strtab = get_section_hdr(elf, ".strtab");

	if (symtab == NULL && !create_section(elf, ".symtab")) {
		printf("[-] failed to find/create new symtab\n");
		return false;
	}
	if (symtab == NULL && !create_section(elf, ".strtab")) {
		printf("[-] failed to find/create new strtab\n");
		return false;
	}

	symtab = get_section_hdr(elf, ".symtab");
	strtab = get_section_hdr(elf, ".strtab");

	printf("symtab %p strtab %p\n", symtab, strtab);

	symtab->sh_type = SHT_SYMTAB;
	strtab->sh_type = SHT_STRTAB;

	symtab->sh_link = get_section_hdr_index(elf, ".strtab");

	print_sections(elf);

	total_len = 0;
	num_symbols = 0;
	for (tracer = symbols; *tracer != NULL; tracer++) {
		total_len += strlen((*tracer)->name) + 1;
		num_symbols++;
	}

	sym_str_tbl = calloc(total_len, sizeof(*sym_str_tbl));
	sym_tab = calloc(num_symbols, sizeof(*sym_tab));

	str_tbl_tracer = sym_str_tbl;
	for (tracer = symbols, i = 0; *tracer != NULL; tracer++, i++) {
		sym_tab[i].st_name = (size_t) str_tbl_tracer - (size_t) sym_str_tbl;
		sym_tab[i].st_value = (Elf64_Addr) (*tracer)->addr;
		sym_tab[i].st_info = (*tracer)->is_function ? STT_FUNC : STT_OBJECT;
		// sym_tab[i]->st_shndx;
		strcpy(str_tbl_tracer, (*tracer)->name);
		str_tbl_tracer += strlen((*tracer)->name) + 1;
	}

	append_to_section(elf, ".strtab", sym_str_tbl, total_len);
	append_to_section(elf, ".symtab", (char *) sym_tab, num_symbols * sizeof(*sym_tab));

	printf("appended to section\n");

	symtab = get_section_hdr(elf, ".symtab");
	symtab->sh_size = sizeof(Elf64_Sym) * num_symbols;
	symtab->sh_entsize = sizeof(Elf64_Sym);

	printf("symtab shoff: 0x%lx, strtab shoff: 0x%lx\n",
		symtab->sh_offset, strtab->sh_offset);

	printf("section hdrs offset 0x%lx\n",elf->elf_hdr->e_shoff);

	free(sym_str_tbl);

	print_symbols(elf);

	return true;
}
