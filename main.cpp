/*
 * main.cpp -- part of job-server
 *    Main entry point for job-server.
 *
 * Copyright (c) 2012-2020 Michael Banack <github@banack.net>
 *
 * MIT License
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include <stdio.h>
#include <iostream>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "MBString.hpp"
#include "MBVector.hpp"

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
	
	Warning("%s: Didn't find a free child... Trying again.\n", __FUNCTION__);
	WaitForChild();
}

int main(int argc, char *argv[])
{
	MBString line;
	pid_t pid;
	
	if (argc > 1) {
		mainData.numJobs = atoi(argv[1]);
	} else {
		mainData.numJobs = get_nprocs();
	}
			
	if (mainData.numJobs <= 0) {
		Warning("Invalid job count specified\n");
		exit(1);
	}
	
	mainData.children.resize(mainData.numJobs);
	
	for (int x = 0; x < mainData.numJobs; x++) {
		mainData.children[x] = PID_INVALID;
	}
	
	while (!cin.eof()) {
		MBString_GetLine(cin, line);
		if (!line.isEmpty()) {			
			WaitForChild();
				
			pid = fork();
			if (!pid) {
				// We are the child
				system(line.CStr());
				exit(0);
			} else {
				// We are the parent
				Warning("%s: Launched [%d] %s\n", __FUNCTION__,
				        pid, line.CStr());
				InsertChild(pid);
			}
		}
	}
	
	// Wait for any remaining children.
	Warning("%s: Waiting for all children...\n", __FUNCTION__);
	for (int x = 0; x < mainData.children.size(); x++) {
		pid = mainData.children[x];
		if (pid != PID_INVALID) {
			waitpid(pid, NULL, 0);
		}
	}
	
	return 0;
}

