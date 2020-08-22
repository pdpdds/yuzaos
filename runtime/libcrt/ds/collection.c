/* MollenOS
 *
 * Copyright 2011 - 2018, Philip Meulengracht
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ? , either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * MollenOS - Generic Collection Implementation
 *  - Implements Collection and queue functionality
 */

#include <collection.h>
#include <SystemCall_Impl.h>
#include <stringdef.h>
#include <assert.h>
#include <memory.h>
#include <atomic.h>

 /* Helper Function
 * Matches two keys based on the key type
 * returns 0 if they are equal, or -1 if not */
int
dsmatchkey(
	_In_ KeyType_t KeyType,
	_In_ DataKey_t Key1,
	_In_ DataKey_t Key2)
{
	switch (KeyType) {
	case KeyId: {
		if (Key1.Value.Id == Key2.Value.Id) {
			return 0;
		}
	} break;
	case KeyInteger: {
		if (Key1.Value.Integer == Key2.Value.Integer) {
			return 0;
		}
	} break;
	case KeyString: {
		return strcmp(Key1.Value.String.Pointer, Key2.Value.String.Pointer);
	} break;
	}
	return -1;
}

/* CollectionCreate
 * Instantiates a new collection with the specified key-type. */
Collection_t*
CollectionCreate(
    _In_ KeyType_t              KeyType)
{
    // Allocate a new Collection structure
    Collection_t *Collection = (Collection_t*)malloc(sizeof(Collection_t));
    memset(Collection, 0, sizeof(Collection_t));

    // Set initial information
    Collection->KeyType = KeyType;
    return Collection;
}

/* CollectionConstruct
 * Instantiates a new static Collection with the given attribs and keytype */
void
CollectionConstruct(
    _In_ Collection_t*          Collection,
    _In_ KeyType_t              KeyType)
{
    memset(Collection, 0, sizeof(Collection_t));
    Collection->KeyType = KeyType;
}

/* CollectionClear
 * Clears the Collection of members, cleans up nodes. */
int
CollectionClear(
    _In_ Collection_t*          Collection)
{
    CollectionItem_t *Node = NULL;
    ASSERT(Collection != NULL);

    // Get initial node and then
    // just iterate while destroying nodes
    Node = CollectionPopFront(Collection);
    while (Node != NULL) {
        CollectionDestroyNode(Collection, Node);
        Node = CollectionPopFront(Collection);
    }
    return 0;
}

/* CollectionDestroy
 * Destroys the Collection and frees all resources associated
 * does also free all Collection elements and keys */
int
CollectionDestroy(
    _In_ Collection_t*          Collection)
{
    CollectionItem_t *Node = NULL;
    ASSERT(Collection != NULL);

    // Get initial node and then
    // just iterate while destroying nodes
    Node = CollectionPopFront(Collection);
    while (Node != NULL) {
        CollectionDestroyNode(Collection, Node);
        Node = CollectionPopFront(Collection);
    }
    free(Collection);
    return 0;
}

/* CollectionLength
 * Returns the length of the given Collection */
size_t
CollectionLength(
    _In_ Collection_t*          Collection)
{
    ASSERT(Collection != NULL);
	return Collection->Length; //4바이트이기 때문에 단위 연산임
    //return atomic_load(&Collection->Length);
}

/* CollectionBegin
 * Retrieves the starting element of the Collection */
CollectionIterator_t*
CollectionBegin(
    _In_ Collection_t*          Collection)
{
	ASSERT(Collection != NULL);
    return Collection->Head;
}

/* CollectionNext
 * Iterates to the next element in the Collection and returns
 * NULL when the end has been reached */
CollectionIterator_t*
CollectionNext(
    _In_ CollectionIterator_t*  It)
{
    return (It == NULL) ? NULL : It->Link;
}

/* CollectionCreateNode
 * Instantiates a new Collection node that can be appended to the Collection 
 * by CollectionAppend. If using an unsorted Collection set the sortkey == key */
CollectionItem_t*
CollectionCreateNode(
    _In_ DataKey_t              Key,
    _In_ void*                  Data)
{
    // Allocate a new instance of the Collection-node
    CollectionItem_t *Node = (CollectionItem_t*)malloc(sizeof(CollectionItem_t));
    memset(Node, 0, sizeof(CollectionItem_t));
    Node->Key       = Key;
    Node->Data      = Data;
    Node->Dynamic   = true;
    return Node;
}

/* CollectionDestroyNode
 * Cleans up a Collection node and frees all resources it had */
int
CollectionDestroyNode(
    _In_ Collection_t*          Collection,
    _In_ CollectionItem_t*      Node)
{
	ASSERT(Collection != NULL);
	ASSERT(Node != NULL);
    if (Node->Dynamic == false) {
        return 0;
    }

    // Behave different based on the type of key
    switch (Collection->KeyType) {
        case KeyString:
            free((void*)Node->Key.Value.String.Pointer);
            break;

        default:
            break;
    }

    // Cleanup node and return
    free(Node);
    return 0;
}

/* CollectionInsertAt
 * Insert the node into a specific position in the Collection, if position is invalid it is
 * inserted at the back. This function is not available for sorted Collections, it will simply 
 * call CollectionInsert instead */
int
CollectionInsertAt(
    _In_ Collection_t*          Collection, 
    _In_ CollectionItem_t*      Node, 
    _In_ int                    Position)
{
    // Sanitize parameters
    if (Collection == NULL || Node == NULL) {
        return -1;
    }

    // We need to make this implementation
    _CRT_UNUSED(Position);

    // todo
    return 0;
}

/* CollectionInsert 
 * Inserts the node into the front of the Collection. This should be used for sorted
 * Collections, but is available for unsorted Collections aswell */
int
CollectionInsert(
    _In_ Collection_t*          Collection, 
    _In_ CollectionItem_t*      Node)
{
	ASSERT(Collection != NULL);
	ASSERT(Node != NULL);

    // Set previous
    Node->Prev = NULL;

    // In case the Collection is empty - no processing needed
	Syscall_DSLock(&Collection->SyncObject);
    //dslock(&Collection->SyncObject);
    if (Collection->Head == NULL || Collection->Tail == NULL) {
        Node->Link          = NULL;
        Collection->Tail    = Node;
        Collection->Head    = Node;
    }
    else {
        // Make the node point to head
        Node->Link              = Collection->Head;
        Collection->Head->Prev  = Node;
        Collection->Head        = Node;
    }
    //dsunlock(&Collection->SyncObject);
	Syscall_DSUnlock(&Collection->SyncObject);
	AtomicInterlockedAdd(&Collection->Length, 1);
    return 0;
}

/* CollectionAppend
 * Inserts the node into the the back of the Collection. This function is not
 * available for sorted Collections, it will simply redirect to CollectionInsert */
int
CollectionAppend(
    _In_ Collection_t*          Collection,
    _In_ CollectionItem_t*      Node)
{
	ASSERT(Collection != NULL);
	ASSERT(Node != NULL);

    // Set eol
    Node->Link = NULL;

	Syscall_DSLock(&Collection->SyncObject);
    if (Collection->Head == NULL || Collection->Tail == NULL) {
        Node->Prev          = NULL;
        Collection->Tail    = Node;
        Collection->Head    = Node;
    }
    else {
        // Append to tail
        Node->Prev              = Collection->Tail;
        Collection->Tail->Link  = Node;
        Collection->Tail        = Node;
    }
    
	Syscall_DSUnlock(&Collection->SyncObject);
    AtomicInterlockedAdd(&Collection->Length, 1);
    return 0;
}

/* CollectionPopFront
 * Removes and returns the first element in the collection. */
CollectionItem_t*
CollectionPopFront(
    _In_ Collection_t*          Collection)
{
    // Variables
    CollectionItem_t *Current = NULL;

    // Do some sanity checks on the state of the collection
	ASSERT(Collection != NULL);
    if (Collection->Head == NULL) {
        return NULL;
    }

    // Manipulate the Collection to find the next pointer of the
    // node that comes before the one to be removed.
    //dslock(&Collection->SyncObject);
	Syscall_DSLock(&Collection->SyncObject);
    Current             = Collection->Head;
    Collection->Head    = Current->Link;

    // Set previous to null
    if (Collection->Head != NULL) {
        Collection->Head->Prev = NULL;
    }

    // Update tail if necessary
    if (Collection->Tail == Current) {
        Collection->Head = Collection->Tail = NULL;
    }
    //dsunlock(&Collection->SyncObject);
	Syscall_DSLock(&Collection->SyncObject);
	AtomicInterlockedDecrement(&Collection->Length);

    // Reset its link (remove any Collection traces!)
    Current->Link = NULL;
    Current->Prev = NULL;
    return Current;
}

/* CollectionPopBack
 * Removes and returns the last element in the collection. */
CollectionItem_t*
CollectionPopBack(
    _In_ Collection_t*          Collection)
{
    _CRT_UNUSED(Collection);
    return NULL;
}

/* CollectionGetNodeByKey
 * These are the node-retriever functions 
 * they return the Collection-node by either key data or index */
CollectionItem_t*
CollectionGetNodeByKey(
    _In_ Collection_t*          Collection,
    _In_ DataKey_t              Key, 
    _In_ int                    n)
{
    // Variables
    CollectionItem_t *i     = NULL;
    int Counter             = n;

    // Do some sanity checks on the state of the collection
	ASSERT(Collection != NULL);
    if (Collection->Head == NULL) {
        return NULL;
    }

    // Iterate each member in the given Collection and
    // match on the key
    _foreach(i, Collection) {
        if (!dsmatchkey(Collection->KeyType, i->Key, Key)) {
            if (Counter == 0) {
                break;
            }
            Counter--;
        }
    }
    return Counter == 0 ? i : NULL;
}

/* CollectionGetDataByKey
 * Finds the n-occurence of an element with the given key and returns
 * the associated data with it */
void*
CollectionGetDataByKey(
    _In_ Collection_t*          Collection, 
    _In_ DataKey_t              Key, 
    _In_ int                    n)
{
    CollectionItem_t *Node = CollectionGetNodeByKey(Collection, Key, n);
    return (Node == NULL) ? NULL : Node->Data;
}

/* CollectionExecute(s)
 * These functions execute a given function on all relevant nodes (see names) */
void
CollectionExecuteOnKey(
    _In_ Collection_t*          Collection, 
    _In_ void                   (*Function)(void*, int, void*), 
    _In_ DataKey_t              Key, 
    _In_ void*                  UserData)
{
    // Variables
    CollectionItem_t *Node  = NULL;
    int i                   = 0;
	ASSERT(Collection != NULL);

    // Iterate the Collection and match key
    _foreach(Node, Collection) {
        if (!dsmatchkey(Collection->KeyType, Node->Key, Key)) {
            Function(Node->Data, i++, UserData);
        }
    }
}

/* CollectionExecute(s)
 * These functions execute a given function on all relevant nodes (see names) */
void
CollectionExecuteAll(
    _In_ Collection_t*          Collection, 
    _In_ void                   (*Function)(void*, int, void*), 
    _In_ void*                  UserData)
{
    // Variables
    CollectionItem_t *Node  = NULL;
    int i                   = 0;
	ASSERT(Collection != NULL);

    // Iteate and execute function given
    _foreach(Node, Collection) {
        Function(Node->Data, i++, UserData);
    }
}

/* CollectionUnlinkNode
 * This functions unlinks a node and returns the next node for usage */
CollectionItem_t*
CollectionUnlinkNode(
    _In_ Collection_t*          Collection, 
    _In_ CollectionItem_t*      Node)
{
	ASSERT(Collection != NULL);
	ASSERT(Node != NULL);

    // There are a few cases we need to handle
    // in order for this to be O(1)
    //dslock(&Collection->SyncObject);
	Syscall_DSLock(&Collection->SyncObject);
    if (Node->Prev == NULL) {
        // Ok, so this means we are the
        // first node in the Collection. Do we have a link?
        if (Node->Link == NULL) {
            // We're the only link
            // but lets stil validate we're from this Collection
            if (Collection->Head == Node) {
                Collection->Head = Collection->Tail = NULL;
                AtomicInterlockedDecrement(&Collection->Length);
            }
        }
        else {
            // We have a link this means we set headp to next
            if (Collection->Head == Node) {
                Collection->Head = Node->Link;
                Collection->Head->Prev = NULL;
                AtomicInterlockedDecrement(&Collection->Length);
            }
        }
    }
    else {
        // We have a previous,
        // Special case 1: we are last element
        // which means we should update pointer
        if (Node->Link == NULL) {
            // Ok, we are last element 
            // Update tail pointer to previous
            if (Collection->Tail == Node) {
                Collection->Tail = Node->Prev;
                Collection->Tail->Link = NULL;
                AtomicInterlockedDecrement(&Collection->Length);
            }
        }
        else {
            // Normal case, we just skip this
            // element without interfering with the Collection
            // pointers
            CollectionItem_t *Prev = Node->Prev;
            Prev->Link = Node->Link;
            Prev->Link->Prev = Prev;
            AtomicInterlockedDecrement(&Collection->Length);
        }
    }
    //dsunlock(&Collection->SyncObject);
	Syscall_DSUnlock(&Collection->SyncObject);
    return (Node->Prev == NULL) ? Collection->Head : Node->Link;
}

/* CollectionRemove
 * These are the deletion functions and remove based on either node index or key */
int
CollectionRemoveByNode(
    _In_ Collection_t*          Collection,
    _In_ CollectionItem_t*      Node)
{
	ASSERT(Collection != NULL);
	ASSERT(Node != NULL);
    CollectionUnlinkNode(Collection, Node);

    // Update links
    Node->Link = NULL;
    Node->Prev = NULL;
    return 0;
}

/* CollectionRemove
 * These are the deletion functions and remove based on either node index or key */
int
CollectionRemoveByIndex(
    _In_ Collection_t*          Collection, 
    _In_ int                    Index)
{
    _CRT_UNUSED(Collection);
    _CRT_UNUSED(Index);
    return 0;
}

/* CollectionRemove
 * These are the deletion functions and remove based on either node index or key */
int
CollectionRemoveByKey(
    _In_ Collection_t*          Collection, 
    _In_ DataKey_t              Key)
{
    // Variables    
    CollectionItem_t *Node = NULL;
	ASSERT(Collection != NULL);

    // Lookup node
    Node = CollectionGetNodeByKey(Collection, Key, 0);
    if (Node != NULL) {
        if (CollectionRemoveByNode(Collection, Node) != 0
            || CollectionDestroyNode(Collection, Node) != 0) {
            return -1;
        }
        else {
            return 0;
        }
    }
    return -1;
}
