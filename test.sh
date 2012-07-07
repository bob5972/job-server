#!/usr/bin/perl


open JOBSERVER, "| build/job-server";

print JOBSERVER "./work.sh 8\n";
print JOBSERVER "./work.sh 4\n";
print JOBSERVER "./work.sh 3\n";
print JOBSERVER "./work.sh 2\n";
print JOBSERVER "./work.sh 1\n";
print JOBSERVER "./work.sh 6\n";
print JOBSERVER "./work.sh 7\n";
print JOBSERVER "./work.sh 5\n";

close JOBSERVER



