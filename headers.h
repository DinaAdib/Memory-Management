#pragma once
#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <string>
#include <math.h>
#include <vector>
#include <iomanip>
#include <cmath>
#include <forward_list>
#include <list>
#include<algorithm>

#define maxSize 10
using namespace std;


//Defining Structs
//Struct process data for all processes loaded from input file and processes in ready queue

struct memNode {
	int size;
	int startingAdd;
	bool free;
	memNode *left;
	memNode *right;
	memNode *parent;

	memNode(int size) {
		this->size = size;
		startingAdd = 0;
		left = NULL;
		right = NULL; 
		free = true;
		this->parent = this;
	}

	memNode()
	{
		size = 0; 
		startingAdd = 0; 
		left = NULL;
		right = NULL;
		free = true;
		this->parent = this;
	}
	memNode(int size, int startingAdd, memNode* Parent) {
		this->size = size;
		this->startingAdd = startingAdd;
		this->parent = Parent;
		left = NULL;
		right = NULL;
		free = true;
	}

	bool operator<(const memNode* other) const { 
	
		if (this->size == other->size) {
			return (this->startingAdd < other->startingAdd);
		}
				
		 return  (this->size < other->size);
	}

	bool operator>(const memNode* other) const {
		if (this->size == other->size) {
			return (this->startingAdd > other->startingAdd);
		}

		return  (this->size > other->size);
	}

};


struct processData {
	int id;
	int arrivalTime;
	int runningTime;
	int remainingTime;
	int size;
	int postponedCount;
	memNode* allocatedMemory;

	processData() {
		allocatedMemory = NULL;
		postponedCount = 0;
	}
	processData(int i, int a, int r, int p, int s)
	{
		id = i; arrivalTime = a; runningTime = r; size = s;
		postponedCount = 0;
		allocatedMemory = NULL;
	}

	void print() {
//		printf("\n Process Data: id %d \t arrivaltime %d runningtime %d priority %d", id, arrivalTime, runningTime);
	}


}processData;



class memoryTree {

private:
	memNode *root;

public:
	/*Creates and empty Binary Tree.
	Children are set to NULL
	*/
	memoryTree();
	memoryTree(const int &element);
	memNode* memoryTree::getSibling(memNode* myNode) {
		if (myNode == root) return NULL;
		if (myNode == myNode->parent->left) return myNode->parent->right;
		else return myNode->parent->left;
	}
};

memoryTree::memoryTree() {
	root = NULL;
}

memoryTree::memoryTree(const int & element) {
	struct memNode *temp;
	temp = new struct memNode;
	temp->size = element;
	temp->left = NULL;
	temp->right = NULL;
	root = temp;
}


bool compareMemoryNodes(memNode* a, memNode* b) {
	if (a->size == b->size) {
		return (a->startingAdd < b->startingAdd);
	}

	return  (a->size < b->size);
}

//Struct for process data in process control block
struct processBlock {
	int id;
	int arrivalTime;
	int runningTime;
	int remainingTime;
	int startTime;
	int finishTime;
	int waitTime; //derived from other attributes?
	int memSize;
	int memStart;
	int memEnd;
	string state;

	processBlock() { id = -1; }
	processBlock(struct processData data) {
		id = data.id;
		arrivalTime = data.arrivalTime;
		runningTime = data.runningTime;
		remainingTime = data.remainingTime;
		memSize = data.size;

	}

}processBlock;
