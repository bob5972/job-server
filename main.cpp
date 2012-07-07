/*
 * main.cpp --
 *
 *     Main entry point for job-server.
 */
 
#include <stdio.h>
#include <iostream>

#include "MBString.h"

using namespace std;

int main()
{
	MBString line;
	
	while (!cin.eof()) {
		printf("> ");
		MBString_GetLine(cin, line);
		if (!line.isEmpty()) {
			printf("Executing: %s\n", line.cstr());
			system(line.cstr());
		}
	}
	
	return 0;
}

