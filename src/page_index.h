#ifndef _PAGE_INTERNAL_H
#define _PAGE_INTERNAL_H

typedef struct {
	void *page;
	int keyname_size;
	char *keyname;
	int left_page;
} redislite_page_index_key;

typedef struct {
	void *db;
	int free_space;
	int number_of_keys;
	int right_page;
	int alloced_keys;
	redislite_page_index_key** keys;
} redislite_page_index;

redislite_page_index *redislite_page_index_create(void* db);
int redislite_insert_key(void *db, unsigned char *key, int length, int left);
int redislite_page_index_add_key(redislite_page_index *page, int pos, int left, unsigned char *key, int length);
void redislite_write_index(unsigned char *data, redislite_page_index *page);
redislite_page_index *redislite_read_index(void *db, unsigned char *data);

#endif