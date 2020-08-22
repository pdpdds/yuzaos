#include "AVLTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <kassert.h>
#include <math.h>

void AVLTree::Print() const
{
	printf("\n");
	if (fDummyHead.fRight)
		Print(fDummyHead.fRight, 0, false);
	printf("\n");
}

void AVLTree::Print(AVLNode *entry, int level, bool left) const
{
	for (int i = 0; i < level; i++)
		printf("   ");

	printf("   %s (%u - %u) max child depth %d\n", left ? "L" : "R", 
		entry->GetLowKey(), entry->GetHighKey(), entry->fMaxChildDepth);
	if (entry->fLeft)
		Print(entry->fLeft, level + 1, true);
	
	if (entry->fRight)
		Print(entry->fRight, level + 1, false);
}

void __assert_failed(int line, const char *file, const char *expr)
{
    printf("ASSERT FAILED (%s:%d): %s\n", file, line, expr);
    exit(1);
}

void __panic(int line, const char *file, const char *expr)
{
    printf("PANIC (%s:%d): %s\n", file, line, expr);
    exit(1);
}

const int numElements = 512; 

int avl_tests()
{
	AVLNode tstr[numElements];
	AVLTree tree;

	// Case #1: Linear insertion and deletion
	printf("Insert in order\n");
	for (int i = 0; i < numElements; i++)
		tree.Add(&tstr[i], i * 5, i * 5 + 4);

	printf("Find in order\n");
	for (int i = 0; i < numElements; i++)
		SKY_ASSERT2(tree.Find(i * 5) == &tstr[i]);

	printf("Remove in order\n");
	for (int j = 0; j < numElements; j++) {
		tree.Remove(&tstr[j]);
		for (int i = 0; i < numElements; i++) {
			if (i > j) {
				SKY_ASSERT2(tree.Find(i * 5) == &tstr[i]);
			} else {
				SKY_ASSERT2(tree.Find(i * 5) == 0);
			}
		}
	}

	printf("Reverse\n");
	// Case #2: Reverse linear insertion and deletion
	for (int i = numElements - 1; i >= 0; i--)
		tree.Add(&tstr[i], i * 5, i * 5 + 4);

	for (int i = 0; i < numElements; i++)
		SKY_ASSERT2(tree.Find(i * 5) == &tstr[i]);

	for (int j = numElements - 1; j >= 0; j--) {
		tree.Remove(&tstr[j]);
		for (int i = 0; i < numElements; i++) {
			if (i < j) {
				SKY_ASSERT2(tree.Find(i * 5) == &tstr[i]);
			} else {
				SKY_ASSERT2(tree.Find(i * 5) == 0);
			}
		}
	}

	printf("Overlaps\n");
	
	// Case #3: test for overlaps
	SKY_ASSERT2(tree.IsRangeFree(15, 17) == true);
	SKY_ASSERT2(tree.Add(&tstr[0], 10, 20) == &tstr[0]);
	SKY_ASSERT2(tree.IsRangeFree(15, 17) == false);
	SKY_ASSERT2(tree.Add(&tstr[1], 15, 17) == 0);
	SKY_ASSERT2(tree.IsRangeFree(5, 25) == false);
	SKY_ASSERT2(tree.Add(&tstr[1], 5, 25) == 0);
	SKY_ASSERT2(tree.IsRangeFree(5, 17) == false);
	SKY_ASSERT2(tree.Add(&tstr[1], 5, 17) == 0);
	SKY_ASSERT2(tree.IsRangeFree(17, 25) == false);
	SKY_ASSERT2(tree.Add(&tstr[1], 17, 25) == 0);
	tree.Remove(&tstr[0]);	
	SKY_ASSERT2(tree.IsRangeFree(15, 17) == true);


	// Case #4: Random insertion and deletion
	bool isInserted[numElements] = { false, };
	
	for (int tries = 0; tries < 20; tries++) 
	{
		// insert a bunch of stuff
		for (int i = 0; i < numElements; i++) 
		{
			int j = rand() % numElements;
			while (isInserted[j])
				j = (j + 1) % numElements;

			isInserted[j] = true;
			tree.Add(&tstr[j], j * 5, j * 5 + 4);
			for (int k = 0; k < numElements; k++) {
				if (isInserted[k]) {
					SKY_ASSERT2(tree.Find(k * 5) == &tstr[k]);
				} else {
					SKY_ASSERT2(tree.Find(k * 5) == 0);
				}
			}
		}
		
		//tree.Print();
		
		int i = numElements - 1;
		for (AVLTreeIterator iterator(tree, false); iterator.GetCurrent(); iterator.GoToNext()) 
		{
			SKY_ASSERT2(iterator.GetCurrent() == &tstr[i--]);
		}

		i = 0;
		for (AVLTreeIterator iterator(tree, true); iterator.GetCurrent();
			iterator.GoToNext()) {
			SKY_ASSERT2(iterator.GetCurrent() == &tstr[i++]);
		}

		// delete a bunch of stuff
		for (int i = 0; i < numElements; i++) 
		{
			int j = rand() % numElements;
			while (!isInserted[j])
				j = (j + 1) % numElements;

			isInserted[j] = false;
			tree.Remove(&tstr[j]);
			for (int k = 0; k < numElements; k++) {
				if (isInserted[k]) {
					SKY_ASSERT2(tree.Find(k * 5) == &tstr[k]);
				} else {
					SKY_ASSERT2(tree.Find(k * 5) == 0);
				}
			}
		}

		printf("Ok...\n");
	}

	printf("Tree tests passed\n");
	return 1;
}