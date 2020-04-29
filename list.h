/*****************************************************************************
 * File name: list.h
 *
 *  Author: Vasco Candeias nº 84196
 *
 *  Release date: 29/04/2020
 *
 *  Description: Headers file for the list.c functions
 *
 ****************************************************************************/

#ifndef LIST_H
#define LIST_H

typedef void* Item;
typedef struct List* List;
typedef struct Node* ListNode;

List put(List, Item);
Item pop(List);
void delete_list(List);
ListNode get_head(List);
Item get_data(ListNode);
ListNode next(ListNode);
void printlist(List);

#endif