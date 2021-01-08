# OpSysProj3
This program monitors the security of a filesystem by recursing through a directory, comparing the filesystem information of the current files against
saved information about those files gathered from a previous run.
It notifies the user of all steps as they happen, and sends warning messages when, based on the saved data from the previous run, it detects a file that is new or missing,
or a file that has filesystem info different from its saved information. It runs automatically at login (specifically the user jensemax).
Currently it can handle a maximum of 50 files or directories, but this can be increased if necessary. The program uses a binary file to store its saved information.
