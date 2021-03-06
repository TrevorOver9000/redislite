#include <stdlib.h>
#include "page.h"
#include "core.h"
#include "page_index.h"
#include "page_freelist.h"

void *redislite_page_get(void *_db, void *_cs, int num, char type)
{
	changeset *cs = (changeset *)_cs;
	redislite *db = (redislite *)_db;
	if (cs) {
		redislite_page *page = redislite_modified_page(cs, num);
		if (page != NULL) {
			return page->data;
		}
		size_t i;
		for (i = 0; i < cs->opened_pages_length; i++) {
			redislite_page *page = cs->opened_pages[i];
			if (page->number == num) {
				return page->data;
			}
		}
	}

	unsigned char *data = redislite_read_page(db, _cs, num);
	if (data == NULL) {
		return NULL;
	}
	void *result = NULL;
	redislite_page_type *page_type = redislite_page_get_type(db, type);
	if (page_type) {
		result = page_type->read_function(db, data);
		if (cs) {
			redislite_add_opened_page(cs, num, type, result);
		}
	}
	redislite_free(data);
	return result;
}

void *redislite_page_get_by_keyname(void *_db, void *_cs, char *key_name, size_t length, char *type)
{
	redislite *db = (redislite *)_db;
	int num = redislite_value_page_for_key(_db, _cs, db->root, key_name, length, type);
	if (num < 0) {
		return NULL;
	}
	return redislite_page_get(_db, _cs, num, *type);
}

int redislite_page_delete(void *_cs, int num, char type)
{
	changeset *cs = (changeset *)_cs;
	redislite *db = cs->db;

	void *data = redislite_page_get(db, cs, num, type);
	redislite_page_type *page_type = redislite_page_get_type(db, type);
	if (page_type->delete_function) {
		page_type->delete_function(cs, data);
	}

	redislite_page_freelist *page = redislite_malloc(sizeof(redislite_page_freelist));
	if (page == NULL) {
		return REDISLITE_OOM;
	}
	page->right_page = db->first_freelist_page;
	db->first_freelist_page = num; // TODO: multithread safeness
	int status = redislite_add_modified_page(cs, num, REDISLITE_PAGE_TYPE_FREELIST, page);
	if (status < 0) {
		redislite_free(page);
	}
	else {
		status = redislite_add_modified_page(cs, 0, REDISLITE_PAGE_TYPE_FIRST, db->root);
		if (status < 0) {
			redislite_free(page);
		}
	}
	return status;
}

int redislite_page_register_type(void *_db, redislite_page_type *type)
{
	redislite *db = (redislite *)_db;
	if (db->types == NULL) {
		db->types = redislite_malloc(sizeof(redislite_page_type *) * 256);
		if (db->types == NULL) {
			redislite_close_database(db);
			return REDISLITE_OOM;
		}
		int i;
		for (i = 0; i < 256; i++) {
			db->types[i] = NULL;
		}
	}
	db->types[(int)type->identifier] = type;
	return REDISLITE_OK;
}

redislite_page_type *redislite_page_get_type(void *_db, char identifier)
{
	redislite *db = (redislite *)_db;
	if (db->types == NULL) {
		return NULL;
	}
	if (db->types[(int)identifier] == NULL) {
		printf("Unknown identifier: '%c'\n", identifier);
	}
	return db->types[(int)identifier];
}
