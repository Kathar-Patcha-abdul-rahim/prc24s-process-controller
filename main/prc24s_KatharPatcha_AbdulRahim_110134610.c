//Kathar Patcha Abdul Rahim
//110134610
//Assignment 2


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>

//global variables to keep track
int descendants[100];
int size = 0;
pid_t targetId;
pid_t rootId;


//Fetch the Parent Id of the given process
int get_ppid(int pid) {
    char path[256];
    //construct the process id path along with proc
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    //open the proc flie in read mode
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Could not open process status file");
        return -1;
    }

    char line[256];
    int ppid = -1;
    //read the parent id
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line, "PPid: %d", &ppid);
            break;
        }
    }

    //close the file
    fclose(file);
    //return the parent id
    return ppid;
}


//Function to check if the process is a descendant of root_process
int isDescendantOfRoot(int target_pid, int root_pid) {
    int current_pid = target_pid;

    //Validate the parent id recursively in descending
    while (current_pid != root_pid && current_pid != 1) {
        int ppid = get_ppid(current_pid);
        if (ppid == -1) {
            // Process does not exist
            return 0;
        }
        current_pid = ppid;
    }
    return current_pid == root_pid;
}


//Find the descendants of the given process id
void findDescendants(pid_t pid) {
    DIR *dir;
    struct dirent *entry;
    char path[256];

    //Open proc directory
    if ((dir = opendir("/proc")) == NULL) {
        perror("Failed to open /proc directory");
        return;
    }

    //Traverse all entries in proc
    while ((entry = readdir(dir)) != NULL) {
        //Check if entry is a directory and its name is a number (process ID)
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            pid_t child_pid = atoi(entry->d_name);

            //Construct path to status file of the process
            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);

            //Open status file
            FILE *file = fopen(path, "r");
            if (file) {
                char line[256];
                while (fgets(line, sizeof(line), file)) {
                    //Find PPid line
                    if (strncmp(line, "PPid:", 5) == 0) {
                        pid_t parent_pid = atoi(line + 6);

                        //If parent PID matches the given PID, recursively send SIGKILL to this child
                        if (parent_pid == pid) {
                            //Recursively kill descendants
                            findDescendants(child_pid);
                            break; //No need to continue searching in this process's status
                        }
                    }
                }
                fclose(file);
            }
        }
    }
    //Close the directory
    closedir(dir);
    //Add the process id to the global descendants array
    descendants[size++] = pid;
}

//Fetch the status of the process and perform action based on that
int fetchStatusAndPerformAction(pid_t pid, char *flag)
{
    //Larger buffer to hold state and newline
    char state[3];
    char cmd[50];
    sprintf(cmd, "ps -o state= -p %d", (int)pid);
    FILE* fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    if (fgets(state, sizeof(state), fp) == NULL) {
        pclose(fp);
        return 0; // Process not found or no state information available
    }
    pclose(fp);
    //If the status of the process is paused send SIGCONT
    if(strcmp(flag, "paused") == 0 && state[0] == 'T')
     {
        printf("\nSending %d signal to process id %d", SIGCONT,pid);
        kill(pid, SIGCONT);
     }

    //If the status of the process is Zombie, print the process id
    else if(strcmp(flag, "zombie") == 0 && state[0] == 'Z')
     {
        printf("\n%d", pid);
     }
    //Verify whether a process is a defunct or funct
    else if(strcmp(flag, "status") == 0)
     {
      if(state[0] != 'Z')
        printf("\nThe status of the %d is not Defunct", pid);
      else
        printf("\nThe status of the %d is defunct", pid);
     }
    //Verify whether a process is zombie or not
    else if(strcmp(flag, "isZombie") == 0 && state[0] == 'Z')
     {
        return 1;
     }
    return 0;
}


//Function to send the corresponding signal and perform further actions
int sendSignal(int signal, int pauseFlag)
{
 for(int i=0; i<size; i++)
 {

   //skip the loop if the pid is the root id
   if(rootId == descendants[i])
   {
        //printf("\nskipping .... %d", descendants[i]);
        continue;
   }
   //printf("\nProcess ID: %d", descendants[i]);
   //Verify the process is paused or not if the pauseflag is on
   if(pauseFlag != 0)
        fetchStatusAndPerformAction(descendants[i], "paused");
   else {
        printf("\nSending %d signal to the process id: %d", signal, descendants[i]);
        kill(descendants[i], signal);
  }
 }
}

//Print the descendants based on several actions
int listDescendants(pid_t pid, int flag)
{

 if(flag == 0)
  {
     for(int i=0; i<size; i++)
     {
        //print all the descendants except the immediate child of the given process id
        if(pid == descendants[i] || pid == get_ppid(descendants[i]))
                continue;
        printf("\n%d", descendants[i]);
     }
  }

 if(flag == 1 || flag == 2)
 {
    for(int i=0; i<size; i++)
    {
        //print the immediate child of the process id
        if(pid != descendants[i] && pid == get_ppid(descendants[i]) &&
                targetId != descendants[i])
                if(flag == 1)
                        printf("\n%d", descendants[i]);
                else if(flag == 2)
                        //verify whether a process is zombie or not and print it
                        fetchStatusAndPerformAction(descendants[i], "zombie");
    }
 }
}


//Starting point of the program
int main(int argc, char *argv[])
{
 //Verify the number of the arguments passed
 if(argc < 3 || argc > 4)
 {
        printf("\nInvalid arguments... Exiting");
        return 0;
 }

 //Declare and assign necessary variables
 char *option = NULL;


 //Verify whether there are 4 arguments and perform variable assignation
 if(argc == 4)
 {
        option = argv[1];
        rootId = atoi(argv[2]);
        targetId = atoi(argv[3]);
 }
 //perform variable assignation if the argc is not 4
 else
 {
        rootId = atoi(argv[1]);
        targetId = atoi(argv[2]);
 }

 //Find whether the given process id is a descendant of the root
 if(isDescendantOfRoot(targetId, rootId))
 {
   printf("\nProcess Id %d belongs to the root Id %d\n",
        targetId, rootId);
   pid_t ppid = get_ppid(targetId);
   if(ppid != -1) {
        //If it is a decendant print the pid and parent id
        printf("\nPID: %d, PPID: %d\n", targetId, ppid);
 }}
 else
 {
  printf("\nProcess Id %d does not belongs to the root Id %d\n",
        targetId, rootId);
  return 0;
 }

//if the argc is not 4 terminate the program
if(argc != 4)
        return 0;

//kill all the descendants of the root process
if(strcmp(option, "-dx") == 0)
{
 //find all the descendants of the given processId
 findDescendants(rootId);
 //Send the corresponding signal
 sendSignal(SIGKILL, 0);
}

//send SIGSTOP to all the descendants of the root
else if(strcmp(option, "-dt") == 0)
{
 findDescendants(rootId);
 sendSignal(SIGSTOP, 0);
}

//send SIGCONT to all the descendants of the root who are paused
else if(strcmp(option, "-dc") == 0)
{
 findDescendants(targetId);
 sendSignal(SIGCONT, 1);
}

//kill the process id given
else if(strcmp(option, "-rp") == 0)
{
 printf("\nSending %d signal to the process id: %d", SIGTERM, targetId);
 //send kill signal
 kill(targetId, SIGTERM);
}

//print all the non direct descendants of the process id
else if(strcmp(option, "-nd") == 0)
{
 printf("\nNon direct descendants of %d :", targetId);
 findDescendants(targetId);
 //print all the non direct descendants
 listDescendants(targetId, 0);
}

//print all the immediate descendants of the process id
else if(strcmp(option, "-dd") == 0)
{
 printf("\nDirect descendants of %d :", targetId);
 findDescendants(targetId);
 //print all the direct descendants
 listDescendants(targetId, 1);
}

//print all the sibling processes of the process id
else if(strcmp(option, "-sb") == 0)
{
 printf("\nSibling processes of %d :", targetId);
 //find all the descendants of the parent of the process id
 findDescendants(get_ppid(targetId));
 //Now print all the child of the parent of the process id
 //which are not equal to the process id
 listDescendants(get_ppid(targetId), 1);
}

//print all the sibling processes of the process id that are defunct
else if(strcmp(option, "-bz") == 0)
{
 printf("\nSibling process of %d which are defunct", targetId);
 findDescendants(get_ppid(targetId));
 //print all the zombie child of the parent of the process id
 //which are not equal to the process id
 listDescendants(get_ppid(targetId), 2);
}

//print all the descendants of the process id who are defunct
else if(strcmp(option, "-zd") == 0)
{
 printf("\nDescendant processes of %d that are defunct :", targetId);
 findDescendants(targetId);
 for(int i=0; i<size; i++)
        //verify whether the process is zombie or not and print if so
        fetchStatusAndPerformAction(descendants[i], "zombie");
}

//list the grandchildren of the process id
else if(strcmp(option, "-gc") == 0)
{
 printf("\nGrand Children of the process id %d :", targetId);
 findDescendants(targetId);
 for(int i=0; i<size; i++)
 {
    //List the grandchildren of the process id
    if(targetId == get_ppid(get_ppid(descendants[i])))
        printf("\n%d", descendants[i]);
 }
}

//print the status of the process whether defunct or not
else if(strcmp(option, "-sz") == 0)
{
 //verify whether the given process is zombie or not
 fetchStatusAndPerformAction(targetId, "status");
}

//kill parents of all zombie processes that are descendants of the process id
else if(strcmp(option, "-kz") == 0)
{
 findDescendants(targetId);
 for(int i=0; i<size; i++)
 {
    //verify whether a process is zombie or not
    if(fetchStatusAndPerformAction(descendants[i], "isZombie"))
        //kill the parent of the process id
        kill(get_ppid(descendants[i]), SIGTERM);
 }
}
//print invalid option
else
{
 printf("\nInvalid option...");
}

 printf("\n\n");
 return 0;
}

