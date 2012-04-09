#define NOT_IN_LIST -1

typedef struct nodeStruct
{
    struct nodeStruct *next, *prev;
    void *item;
}   nodeType;

typedef nodeType *nodePtr;

typedef struct
{
    int nItems;
    nodePtr last, first;
}   listType;

typedef listType *listPtr;

extern struct listStruct
{
    int p;
}   listG;

extern void list_Init (listPtr list);

extern nodePtr list_GetNode(listType list, int i);
extern int list_FindItem (listType list, void *item);
extern int list_AddItem(listPtr list, void *item);
extern void list_RemoveItem(listPtr list, int i, int trashitem);
extern void list_RemoveAllItems(listPtr list, int trashitems);
extern void list_MoveUp(listType list, int i);
extern void list_MoveDown(listType list, int i);
extern void listG_Init(void);

extern void list_InitPanel (char *header, char *button);
