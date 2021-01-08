#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

/*
Max Jensen
12/13/20
This program monitors the security of a filesystem by recursing through a directory, comparing the filesystem information of the current files against saved information about those files gathered from a previous run. It notifies the user of all steps as they happen, and sends warning messages when, based on the saved data from the previous run, it detects a file that is new or missing, or a file that has filesystem info different from its saved information. It runs automatically at login (specifically the user jensemax). Currently it can handle a maximum of 50 files or directories, but this can be increased if necessary. The program uses a binary file to store its saved information. See the design doc for detailed implementation information.
*/

int saveData(const char *name, const struct stat *file, int type);
int readData();
int checkSimilar();
int checkSame(struct stat old, struct stat new); 

// Global arrays for fileinfo structs, used to compare saved and new data.
struct stat newFiles[50];
struct stat savedFiles[50];

int globalCounter = 0;

// Counters used to keep track of number of file info structs in the arrays
int newCount = 0;
int savedCount = 0;

int main(int argc, char *argv[]){			
	printf("|||| Reading previously saved data in to security checker ||||\n");
	
	sleep(2);
	
	readData();
	
	fopen("data.bin","wb");

	printf("|||| Saving current data out ||||\n");
	
	sleep(2);
	
	// MODIFY THE PATH BELOW to run this program on a different directory tree
	ftw("/home/jensemax/OpSystems/test", saveData, 10);

	checkSimilar();

	return 0;
}

//Saves current directory info
int saveData(const char *name, const struct stat *file, int type){
	
	FILE *ptr_myfile;
	struct stat statInfo;

	char time[50];
		
	ptr_myfile=fopen("data.bin","ab");
	
	statInfo = *file;
	
	newFiles[globalCounter]=*file;
	
	if (!ptr_myfile){
		printf("Unable to open file!");
		return 1;
	}
	
	fwrite(&statInfo, sizeof(statInfo), 1, ptr_myfile);
	printf("Saving the file or directory with name %s\n", name);
	printf("inode: %ld\n", statInfo.st_ino);
	printf("mode: %d\n", statInfo.st_mode);
	printf("owner user ID: %d\n", statInfo.st_uid);
	printf("owner group ID: %d\n", statInfo.st_gid);
	printf("filesize: %ld\n", statInfo.st_size);

    strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&statInfo.st_atime));
	printf("last accessed: %s\n", time);
	strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&statInfo.st_mtime));
	printf("last modified: %s\n\n", time);
	
	sleep(1);
	globalCounter++;
	newCount++;
	fclose(ptr_myfile);
}


// Reads previously saved directory info
int readData(){
	FILE *tempFPtr;
	struct stat fileToRead;
	char time[50];
	int size;
	int counter;
	
	tempFPtr=fopen("data.bin","rb");
		
	counter = 0;
	
	while (1){		
		fread(&fileToRead,sizeof(fileToRead),1,tempFPtr);		
		
		if(feof(tempFPtr)) {
		  break;
		}
		
		printf("Reading the file or directory with inode: %ld\n", fileToRead.st_ino);
		printf("mode: %d\n", fileToRead.st_mode);
		printf("owner user ID: %d\n", fileToRead.st_uid);
		printf("owner group ID: %d\n", fileToRead.st_gid);
		printf("filesize: %ld\n", fileToRead.st_size);
		strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&fileToRead.st_atime));
		printf("last accessed: %s\n", time);
		strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&fileToRead.st_mtime));
		printf("last modified: %s\n\n", time);
		
		savedFiles[counter] = fileToRead;
		
		sleep(1);
		
		counter++;
		savedCount++;
	}
	fclose(tempFPtr);
}

// Iterated through the struct arrays and checks the fileinfo similarities between two fileinfo structs
int checkSimilar(){
	int checkCounter1 = 0;
	int checkCounter2 = 0;
	
	bool fileIsMiss = true;
	bool fileIsNew = true;	
	
	// Loop that runs through old files first, comparing them to all new files
	while (checkCounter1<savedCount){
		
		/*printf("Comparing ino: %ld and ino: %ld\n", savedFiles[checkCounter1].st_ino, newFiles[checkCounter2].st_ino);*/
		fileIsMiss = true;
		while(checkCounter2<newCount){
			// Check if a saved file is no longer in the scanned files
			if (savedFiles[checkCounter1].st_ino == newFiles[checkCounter2].st_ino){
				fileIsMiss = false;
			}
			checkCounter2++;
		}
		checkCounter2=0;
		//Prints a warning if the file is missing
		if(fileIsMiss==true){
			printf("!WARNING! The file with inode: **%ld** is no longer in the monitored directory!\n", savedFiles[checkCounter1].st_ino);
		}
		checkCounter1++;
	}
	
	// Loop that runs through new files first, comparing them to all old files
	checkCounter1=0;
	checkCounter2=0;
	

	while (checkCounter2<newCount){
		
		/*printf("Comparing ino: %ld and ino: %ld\n", newFiles[checkCounter2].st_ino, savedFiles[checkCounter1].st_ino);*/
		fileIsNew = true;
		while(checkCounter1<savedCount){

			// Checks if a new file is completely new and not in the original
			if (newFiles[checkCounter2].st_ino == savedFiles[checkCounter1].st_ino){
				fileIsNew = false;
			}
			checkCounter1++;
		}
		checkCounter1=0;
		// Prints a warning if the file is new
		if(fileIsNew==true){
			printf("!WARNING! The file with inode: **%ld** is a NEW file that was not present in the monitored directory previously!\n", newFiles[checkCounter2].st_ino);
		}
		checkCounter2++;
	}
	
	// Loop that checks old files against new files for fileinfo changes
	checkCounter1=0;
	checkCounter2=0;
	while (checkCounter1<savedCount){
		while(checkCounter2<newCount){
			checkSame(savedFiles[checkCounter1], newFiles[checkCounter2]);
			checkCounter2++;
		}
		checkCounter2=0;
		checkCounter1++;
	}	
}

// Checks the fileinfo similarities between two fileinfo structs with the same inode
int checkSame(struct stat old, struct stat new){
	char time[50];
	if(old.st_ino == new.st_ino){
		if(old.st_mode != new.st_mode ||
		old.st_uid != new.st_uid ||
		old.st_gid != new.st_gid ||
		old.st_size != new.st_size ||
		(difftime(old.st_atim.tv_sec, new.st_atim.tv_sec) != 0) ||
		(difftime(old.st_mtim.tv_sec, new.st_mtim.tv_sec) != 0)){
			printf("!!!WARNING!!! Discrepancy detected in file with inode: %ld\n", old.st_ino);
			printf("This file has likely been modified since the last run of the security checker!\n");
			printf("Please see the diagnostic output below to check if the modifications are a true security risk.\n");
			printf("^^^^Saved file info^^^^\n");
			printf("inode: %ld\n", old.st_ino);
			printf("mode: %d\n", old.st_mode);
			printf("filesize: %ld\n", old.st_size);
			printf("owner user ID: %d\n", old.st_uid);
			printf("owner group ID: %d\n", old.st_gid);

			strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&old.st_atime));
			printf("last accessed: %s\n", time);
			strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&old.st_mtime));
			printf("last modified: %s\n\n", time);
			
			
			
			printf("^^^^New file info^^^^\n");
			printf("inode: %ld\n", new.st_ino);
			printf("mode: %d\n", new.st_mode);
			printf("filesize: %ld\n", new.st_size);
			printf("owner user ID: %d\n", new.st_uid);
			printf("owner group ID: %d\n", new.st_gid);

			strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&new.st_atime));
			printf("last accessed: %s\n", time);
			strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&new.st_mtime));
			printf("last modified: %s\n\n", time);			
			return 1;
		}
	}
		return 0;
}