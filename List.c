#include <utility.h>
#include <userint.h>
#include <ansi_c.h>
#include "util.h"
#include "list.h"
#include "listu.h"

#define TRUE 1
#define FALSE 0

struct listStruct listG;

void    list_Init (listPtr list);
nodePtr list_GetNode (listType list, int i);
int     list_FindItem (listType list, void *item);
int     list_AddItem (listPtr list, void *item);
void    list_RemoveItem (listPtr list, int i, int trashitem);
void    list_RemoveAllItems (listPtr list, int trashitems);
void    list_MoveUp (listType list, int i);
void    list_MoveDown (listType list, int i);
void    listG_Init(void);

void list_InitPanel (char *header, char *button);

void listG_Init(void)
{
    util_ChangeInitMessage ("List Utilities...");

    listG.p = 0;
}

void list_Init (listPtr list)
{
    list->first = NULL;
    list->last = NULL;
    list->nItems = 0;
}

nodePtr list_GetNode (listType list, int i)
{
    nodePtr p;
    int cnt;

    cnt = 0;
    p = list.first;
    while (cnt < i) {if(p && p->next){ p = p->next; cnt++;}else return NULL;}
    return p;
}

int list_FindItem (listType list, void *item)
{
    int i, cnt = 0;
    nodePtr p;

    i = 0;
    p = list.first;
    while (p && ((p->item != item) && (p->next))) { p = p->next; i++;}

    if (!p || (p->item != item))
    {
        while (cnt < 20) {cnt++;}
        i = NOT_IN_LIST;
    }
    return i;
}

int list_AddItem (listPtr list, void *item)
{
    nodePtr newnode;

    newnode = malloc (sizeof(nodeType));

    if (!newnode) {util_OutofMemory("List Add Item"); return FALSE;}

    newnode->next = NULL;
    newnode->prev = NULL;
    newnode->item = item;

    if (list->nItems == 0)
        list->first = newnode;
    else
    {
        list->last->next = newnode;
        newnode->prev = list->last;
    }

    list->last = newnode;
    list->nItems++;
    return TRUE;
}

void list_RemoveItem (listPtr list, int i, int trashitem)
{
    nodePtr trash;

    trash = list_GetNode (*list, i);
    if (trashitem) free (trash->item);

    if (list->nItems == 1)
    {
        list->first = NULL;
        list->last = NULL;
    }
    else
    {
        if (list->first == trash)
        {
            list->first = list->first->next;
            list->first->prev = NULL;
        }
        else if (list->last == trash)
        {
            list->last = list->last->prev;
            list->last->next = NULL;
        }
        else
        {
            trash->next->prev = trash->prev;
            trash->prev->next = trash->next;
        }
    }

    free (trash);
    list->nItems--;
}

void list_RemoveAllItems (listPtr list, int trashitems)
{
    while (list->first) list_RemoveItem (list, 0, trashitems);
}

void list_MoveUp (listType list, int i)
{
    nodePtr p;
    void *temp;

    if (i != 0)
    {
        p = list_GetNode (list, i);
        temp = p->prev->item;
        p->prev->item = p->item;
        p->item = temp;
    }
}

void list_MoveDown (listType list, int i)
{
    nodePtr p;
    void *temp;

    if (i != (list.nItems-1))
    {
        p = list_GetNode (list, i);
        temp = p->next->item;
        p->next->item = p->item;
        p->item = temp;
    }
}

void list_InitPanel (char *header, char *button)
{
    listG.p = LoadPanel (0, "listu.uir", LIST);
    
    SetPanelPos (listG.p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    util_InitClose (listG.p, LIST_CANCEL, TRUE);

    SetPanelAttribute (listG.p, ATTR_TITLE, header);
    SetCtrlAttribute (listG.p, LIST_CONT, ATTR_LABEL_TEXT, button);
}

int  ListSelectAllCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, n, checked;
    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, control, &checked);
        GetNumListItems (panel, LIST_ITEMS, &n);
        for (i = 0; i < n; i++)
            CheckListItem (panel, LIST_ITEMS, i, checked);
    }
    return 0;
}

