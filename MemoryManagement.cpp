#include "headers.h"


/*----------Global variables-----------*/
queue <struct processData> allProcesses;
queue <struct processData> roundRobinQ;
vector <struct processBlock> processTable;
//vector <vector <struct memoryData>> memory;
vector <struct memNode*> freeNodes;
memoryTree Memory;
int quantum = 0, switchingTime = 0, finishedCount = 0, processesCount = 0, currentTime = 0;
struct processData runningProcess, arrivedProcess;
bool processAllocated = false;

/*-----------Output Files-------------*/
ofstream out("Output.txt");
ofstream logout("log.txt");

/*---------Functions' Headers-----*/
void receiveProcesses();
void loadInputFile();
int getHighestPowerofTwo(int size);
void deallocate(struct processData process);
void InsertinProcessTable(struct processData&);
bool allocate(struct processData &process);
bool Split(memNode* currentNode, int requiredSize, struct processData&);
//void mergeEmpty(vector <struct memoryData> currentVector, int mergeIndex);
void RR_Algorithm();
void runProcess();
void switchProcess();
void print_queue(queue<struct processData> q);
void outFile();

int main()
{
	memNode firstFree(maxSize);
	firstFree.startingAdd = 0;
	freeNodes.push_back(&firstFree);

	loadInputFile();

	while (roundRobinQ.size() < 1) {
		receiveProcesses();
		currentTime++;
	}
	RR_Algorithm();
	outFile();
	system("pause");
	return 0;
}

//Round Robin Algorithm
void RR_Algorithm()
{
	currentTime--;
	while (finishedCount < processesCount) {
		if (!roundRobinQ.empty())
		{
			print_queue(roundRobinQ);
			runningProcess = roundRobinQ.front();
			roundRobinQ.pop();
			runProcess();
		}
		else
		{
			currentTime++;
		}
	}
	return;
}

//This function is used to run process on top of readyQueue
void runProcess()
{	
	int startTime;
	if (runningProcess.remainingTime == runningProcess.runningTime) {
		//allocate in memory 
		if (allocate(runningProcess)) {
			processTable[runningProcess.id - 1].memStart = runningProcess.allocatedMemory->startingAdd;
			processTable[runningProcess.id - 1].memEnd = runningProcess.allocatedMemory->startingAdd + pow(2, runningProcess.allocatedMemory->size) - 1;
			processTable[runningProcess.id - 1].startTime = currentTime;
		}
	}
	else {
		switchProcess();
	}
	startTime = currentTime;
	if (runningProcess.allocatedMemory != NULL) {
		if (runningProcess.remainingTime>quantum)  // The process will not finish this loop
		{
			//Updating process data and process block
			runningProcess.remainingTime -= quantum;
			currentTime += quantum;
			logout << "Executing process T" << runningProcess.id << " : started at " << startTime << ", stopped at " << currentTime << ", " << runningProcess.remainingTime << " remaining. Memory starts at " << processTable[runningProcess.id - 1].memStart << " and ends at " << processTable[runningProcess.id - 1].memEnd << endl;
			currentTime++;
			receiveProcesses();
			switchProcess();
			roundRobinQ.push(runningProcess);

		}
		else
		{
			currentTime += runningProcess.remainingTime;
			logout << "Executing process T" << runningProcess.id << " : started at " << startTime << ", finished at " << currentTime << ". Memory starts at " << processTable[runningProcess.id - 1].memStart << " and ends at " << processTable[runningProcess.id - 1].memEnd << endl;

			receiveProcesses();
			processTable[runningProcess.id - 1].finishTime = currentTime;
			// deallocate Memory 
			deallocate(runningProcess);
			currentTime++;
			finishedCount++;
			// update process block --> log
		}
	}
	else {

		logout << "Could not allocate process" << endl;
		runningProcess.postponedCount++;
		if (runningProcess.postponedCount == 5) {
			logout << "Removing process from ready queue." << endl;
			processesCount--;
		}
		else roundRobinQ.push(runningProcess);
	}
	
}

void InsertinProcessTable(struct processData &process) {
		struct processBlock pB;
		pB.id = process.id;
		pB.arrivalTime = process.arrivalTime;
		pB.runningTime = process.runningTime;
		pB.finishTime = pB.arrivalTime;
		pB.memSize = process.size;
		pB.memStart = 0;
		pB.memEnd = 0;
		processTable.push_back(pB);
	
}

bool allocate(struct processData &process) {
	int requiredSize = getHighestPowerofTwo(process.size);
	memNode* foundNode = NULL;

	int i = 0;
	bool found = false;
	std::sort(freeNodes.begin(), freeNodes.end(), compareMemoryNodes);
	while (i < freeNodes.size() && !found) {
		if (requiredSize <= freeNodes[i]->size) {
				foundNode = freeNodes[i];
				freeNodes.erase(freeNodes.begin() + i);
				found = true;
		}
		i++;
	}
	if (!found) {
		return false;
	}
	//function to split this free space till required size
	return Split(foundNode, requiredSize, process);
}

bool Split(memNode* currentNode, int requiredSize, struct processData &process) {

	if (currentNode == NULL) return false;
	if (currentNode->size == requiredSize) {
		process.allocatedMemory = currentNode;
		currentNode->free = false;
		return true;
	}
	memNode *left = new memNode(currentNode->size - 1, currentNode->startingAdd, currentNode);
	memNode *right = new memNode(currentNode->size - 1, currentNode->startingAdd + pow(2, currentNode->size - 1), currentNode);
	currentNode->left = left;
	currentNode->right = right;
	currentNode->free = false;
	currentNode->right->free = true;
	freeNodes.push_back(currentNode->right);
	return Split(currentNode->left, requiredSize, process);
}

void deallocate(struct processData process) {
	memNode* currentNode = process.allocatedMemory;
	memNode* parent = currentNode->parent;
	process.allocatedMemory->free = true;
	while (Memory.getSibling(currentNode) && Memory.getSibling(currentNode)->free == true) {
		// search for sibling and delete from free Nodes 
		freeNodes.erase(std::remove(freeNodes.begin(), freeNodes.end(), Memory.getSibling(currentNode)), freeNodes.end());
		delete Memory.getSibling(currentNode);
		delete currentNode;
		currentNode = parent;
		parent = currentNode->parent;
	}
	currentNode->free = true;
	freeNodes.push_back(currentNode);
}

void receiveProcesses() {
	while (!allProcesses.empty() && currentTime >= allProcesses.front().arrivalTime) {
		arrivedProcess = allProcesses.front();
		allProcesses.pop();
		roundRobinQ.push(arrivedProcess);
		InsertinProcessTable(arrivedProcess);
	}
	return;
}

void switchProcess() {

	logout << "Process switching    : started at " << currentTime << " finished at " << currentTime + switchingTime << endl;
	currentTime += switchingTime+1;
}


void print_queue(queue<struct processData> q)
{
	struct processData currentProcess;
	int size = q.size();
	int i = 0;
	logout << "Queue: ";
	while (i<size)
	{
		currentProcess = q.front();
		logout << "T" << q.front().id;
		if (i != size - 1) logout << ", ";
		q.pop();
		q.push(currentProcess);
		i++;
	}
	logout << std::endl;
}

// This function loads the input file into a queue
void loadInputFile()
{
	ifstream inp("processes.txt");
	string Data;
	int runTime, arrivalTime, memSize;
	string pid;
	struct processData currentProcess;
	int pidInt;
	while (getline(inp, Data))
	{
		if (Data[0] != 'T') {
			istringstream iss(Data);
			string x;
			int y;
			iss >> x >> y;
			if (x == "Quantum") {
				quantum = y;
			}
			if (x == "Switch") {
				switchingTime = y;
			}
		}
		else {
			stringstream ss(Data);
			ss >> pid >> currentProcess.runningTime >> currentProcess.arrivalTime >> currentProcess.size;
			pidInt = int(pid[1]) - 48;
			currentProcess.id = pidInt;
			currentProcess.remainingTime = currentProcess.runningTime;
			allProcesses.push(currentProcess);
			processesCount++;
		}

	}
	cout << "Quantum is " << quantum << endl;
	cout << "Switching time is " << switchingTime << endl;
	inp.close();
	return;
}

void outFile() {
	out << "process_id\trun_time\tarrival_time\tfinish_time\tmem_size\tmem_start\tmem_end" << endl;
	for (int i = 0; i < processTable.size(); i++) {
		out << "T" << processTable[i].id << "\t\t" << processTable[i].runningTime << "\t\t" << processTable[i].arrivalTime << "\t\t" << processTable[i].finishTime << "\t\t" << processTable[i].memSize << "\t\t" << processTable[i].memStart << "\t\t" << processTable[i].memEnd << endl;
	}
}
int getHighestPowerofTwo(int size) {
	return ceil(log2(size));
}
