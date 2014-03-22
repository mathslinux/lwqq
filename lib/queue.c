#include "queue.h"
#include <stdlib.h>

typedef _TAILQ_HEAD(tailq_head, void, ) tailq_head;
typedef struct {
	_TAILQ_ENTRY(void,) entries;
}tailq_entry;

void tailq_init(tailq_head* head)
{
	TAILQ_INIT(head);
}

void tailq_insert_tail(tailq_head* head,void* elem,tailq_entry* entry)
{
	void** last = head->tqh_last;
	TAILQ_INSERT_TAIL(head, entry, entries);
	*last = elem;
}
