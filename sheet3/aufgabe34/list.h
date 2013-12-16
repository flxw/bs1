
#ifndef _LIST_H_
#define _LIST_H_

#include <windows.h>

VOID InitializeListHead(IN PLIST_ENTRY ListHead);

BOOLEAN IsListEmpty(IN PLIST_ENTRY ListHead);

BOOLEAN RemoveEntryList(IN PLIST_ENTRY Entry);

PLIST_ENTRY RemoveHeadList(IN PLIST_ENTRY ListHead);
PLIST_ENTRY RemoveTailList(IN PLIST_ENTRY ListHead);

VOID InsertTailList(IN PLIST_ENTRY ListHead, IN PLIST_ENTRY Entry);
VOID InsertHeadList(IN PLIST_ENTRY ListHead, IN PLIST_ENTRY Entry);

#endif // _LIST_H_