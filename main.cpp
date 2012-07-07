/*
 * main.cpp --
 *
 *     Main entry point for job-server.
 */
 
#include <stdio.h>
#include <iostream>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "MBString.h"
#include "MBVector.h"

using namespace std;

#define PID_INVALID (-1)

typedef struct MainData {
	MBVector<pid_t> children;
	int numJobs;
} MainData;

static MainData mainData;

static void
InsertChild(pid_t pid)
{
	for (int x = 0; x < mainData.children.size(); x++) {
		if (mainData.children[x] == PID_INVALID) {
			mainData.children[x] = pid;
			return;
		}
	}
	
	NOT_REACHED();
}

static void
WaitForChild(void)
{
	int status;
	int result;
	pid_t pid;
	
	for (int x = 0; x < mainData.children.size(); x++) {
		if (mainData.children[x] == PID_INVALID) {
			return;
		}
	}
	
	//No free slot
	for (int x = 0; x < mainData.children.size(); x++) {
		pid = mainData.children[x];
		ASSERT(pid != PID_INVALID);
		
		result = waitpid(pid, &status, WNOHANG);
		if (result == -1) {			
			Warning("%s: Error occured while waiting for child.\n");
		} else if (WIFEXITED(status)) {
			mainData.children[x] = PID_INVALID;
			return;
		}
	}
	
	// All children are still running...
	Warning("%s: Waiting for any child...\n", __FUNCTION__);
	pid = wait(&status);	
	if (pid != -1) {
		for (int x = 0; x < mainData.children.size(); x++) {
			if (mainData.children[x] == pid) {
				mainData.children[x] = PID_INVALID;
				return;
			}
		}
		ASSERT(FALSE); // We didn't find our child??
	}
	
	Warning("%s", "Didn't find a free child... Trying again.\n");
	WaitForChild();
}

int main()
{
	MBString line;
	pid_t pid;
	
	mainData.numJobs = get_nprocs();
	mainData.children.resize(mainData.numJobs);
	
	for (int x = 0; x < mainData.numJobs; x++) {
		mainData.children[x] = PID_INVALID;
	}
	
	while (!cin.eof()) {
		printf("> ");
		MBString_GetLine(cin, line);
		if (!line.isEmpty()) {			
			WaitForChild();
				
			pid = fork();
			if (!pid) {
				// We are the child
				system(line.cstr());
				exit(0);
			} else {
				// We are the parent
				Warning("%s: Launched [%d] %s\n", __FUNCTION__, pid, line.cstr());//banackm
				InsertChild(pid);
			}
		}
	}
	
	// Wait for any remaining children.
	for (int x = 0; x < mainData.children.size(); x++) {
		pid = mainData.children[x];
		if (pid != PID_INVALID) {
			printf("%s: Waiting for %d\n", __FUNCTION__, pid);//banackm
			waitpid(pid, NULL, 0);
		}
	}
	
	return 0;
}

