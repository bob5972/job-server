#!/usr/bin/perl


open JOBSERVER, "| build/job-server";

print JOBSERVER "./work.sh 5\n";
print JOBSERVER "./work.sh 3\n";
print JOBSERVER "./work.sh 6\n";
print JOBSERVER "./work.sh 7\n";
print JOBSERVER "./work.sh 8\n";
print JOBSERVER "./work.sh 9\n";
print JOBSERVER "./work.sh 10\n";
print JOBSERVER "./work.sh 11\n";

close JOBSERVER



