#pragma once
#include <stdint.h>

// double linked list element
typedef struct dlelement dlelement_t;
struct dlelement
{
	void* data;
	dlelement_t* prev;
	dlelement_t* next;
};

typedef struct
{
	dlelement_t* head;
	dlelement_t* tail;
} list_t;

#define      list_init() {0, 0}
list_t* list_create(void);                                        // Allocates memory for a list, returns a pointer to that list.
void         list_construct(list_t* list);                             // Initializes given list_t instance
void         list_destruct(list_t* list);                              // Destructs given list_t instance
void         list_free(list_t* list);                                  // Deletes everything that has been allocated for this list.
dlelement_t* list_alloc_elem(size_t size, const char* description);    // Allocates memory for a list element with additional space for data
dlelement_t* list_insert(list_t* list, dlelement_t* next, void* data); // Inserts a new element before an element of the list (0 = append). Returns a pointer to the new element.
dlelement_t* list_append(list_t* list, void* data);                    // Inserts a new element at the end of the list and returns a pointer to it.
void         list_append_elem(list_t* list, dlelement_t* elem);        // Inserts given element at the end of the list.
dlelement_t* list_delete(list_t* list, dlelement_t* elem);             // Deletes the element elem and returns a pointer to the element that was behind it.
dlelement_t* list_getElement(list_t* list, uint32_t number);           // Returns the data at the position "number".
dlelement_t* list_find(const list_t* list, void* data);                // Finds an element with data in the list and returns a pointer to it.
size_t       list_getCount(const list_t* list);                        // Returns the number of elements of the list
bool         list_isEmpty(const list_t* list);
void         list_show(const list_t* list);
