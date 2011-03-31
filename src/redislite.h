#ifndef _REDISLITE_H
#define _REDISLITE_H

#include <stdio.h>
#include "page.h"

#define HEADER_STRING "Redislite format 1"
#define DEFAULT_PAGE_SIZE 512
#define DEFAULT_MODIFIED_PAGE_SIZE 4
#define WRITE_FORMAT_VERSION 1
#define READ_FORMAT_VERSION 1


typedef struct {
	unsigned char *filename;
	FILE *file;
	int page_size;
	int file_change_counter;
	int number_of_pages;
	int first_freelist_page;
	int number_of_freelist_pages;
	void *root;

	int readonly;

	int modified_pages_length;
	int modified_pages_free;
	void **modified_pages;
} redislite;

redislite* redislite_create_database(const unsigned char *filename);
redislite* redislite_open_database(const unsigned char *filename);
void redislite_close_database(redislite *db);
unsigned char *redislite_read_page(redislite *db, int num);
int redislite_add_modified_page(redislite *db, int page_number, char type, void *page_data);

#define REDISLITE_OK 0
#define REDISLITE_ERR 1
#define REDISLITE_OOM 2

#endif
