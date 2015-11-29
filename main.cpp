/*
 * This file is part of job-server.
 * Copyright (c) 2015 Michael Banack <bob5972@banack.net>
 *
 * job-server is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * job-server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with job-server.  If not, see <http://www.gnu.org/licenses/>.
 */


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

