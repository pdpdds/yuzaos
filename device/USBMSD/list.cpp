#include "list.h"
#include <stdio.h>


list_t* list_create(void)
{
    list_t* list = (list_t *)malloc(sizeof(list_t));
    if (list)
    {
        list->head = 0;
        list->tail = 0;
    }
    return (list);
}

void list_construct(list_t* list)
{
    list->head = 0;
    list->tail = 0;
}

dlelement_t* list_alloc_elem(size_t size, const char* description)
{
    dlelement_t* newElement = (dlelement_t*)malloc(sizeof(dlelement_t) + size);
    if (newElement)
        newElement->data = newElement + 1;
    return newElement;
}

dlelement_t* list_append(list_t* list, void* data)
{
    dlelement_t* newElement = (dlelement_t*)malloc(sizeof(dlelement_t));
    if (newElement)
    {
        newElement->data = data;
        list_append_elem(list, newElement);
    }
    return newElement;
}

void list_append_elem(list_t* list, dlelement_t* elem)
{
    elem->next = 0;
    elem->prev = list->tail;

    if (list->head == 0)
    {
        list->head = elem;
    }
    else
    {
        list->tail->next = elem;
    }
    list->tail = elem;
}

dlelement_t* list_insert(list_t* list, dlelement_t* next, void* data)
{
    if (next == 0)
    {
        return (list_append(list, data));
    }

    dlelement_t* newElement = (dlelement_t*)malloc(sizeof(dlelement_t));
    if (newElement)
    {
        newElement->data = data;

        if (next == list->head)
        {
            newElement->next = list->head;
            newElement->prev = 0;
            list->head->prev = newElement;
            list->head       = newElement;
        }
        else
        {
            newElement->prev = next->prev;
            newElement->next = next;
            next->prev->next = newElement;
            next->prev       = newElement;
        }

        return newElement;
    }

    return (0);
}

dlelement_t* list_delete(list_t* list, dlelement_t* elem)
{
    if (list->head == 0)
    {
        return (0);
    }

    if (list->head == list->tail)
    {
        free(elem);
        list->head = list->tail = 0;
        return (0);
    }

    dlelement_t* temp = elem->next;

    if (elem == list->head)
    {
        list->head       = elem->next;
        list->head->prev = 0;
    }
    else if (elem == list->tail)
    {
        list->tail       = elem->prev;
        list->tail->next = 0;
    }
    else
    {
        elem->prev->next = elem->next;
        elem->next->prev = elem->prev;
    }

    free(elem);

    return temp;
}

void list_destruct(list_t* list)
{
    dlelement_t* cur = list->head;

    while (cur)
    {
        dlelement_t* nex = cur->next;
        free(cur);
        cur = nex;
    }
    list->head = list->tail = 0;
}

void list_free(list_t* list)
{
    list_destruct(list);
    free(list);
}

dlelement_t* list_getElement(list_t* list, uint32_t number)
{
    dlelement_t* cur = list->head;
    while (true)
    {
        if (number == 0 || cur == 0)
        {
            return (cur);
        }

        --number;
        cur = cur->next;
    }
}

dlelement_t* list_find(const list_t* list, void* data)
{
    dlelement_t* cur = list->head;
    while (cur && cur->data != data)
    {
        cur = cur->next;
    }

    return (cur);
}

size_t list_getCount(const list_t* list)
{
    size_t count = 0;
    for (dlelement_t* e = list->head; e; e = e->next)
    {
        count++;
    }
    return (count);
}

bool list_isEmpty(const list_t* list)
{
    return (list->head == 0);
}

void list_show(const list_t* list)
{
    printf("list dump\n");

    for (dlelement_t* e = list->head; e; e = e->next)
    {
        printf("data: %x\n", e->data);
    }
}