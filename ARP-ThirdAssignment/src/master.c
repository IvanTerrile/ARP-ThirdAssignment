#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

// Variables to store the PIDs
pid_t pid_procA;  // PID of process A
pid_t pid_procB;  // PID of process B

// Function to spawn a child process
int spawn(const char * program, char * arg_list[]) {
  pid_t child_pid = fork();   // Fork the process and store the PID of the child process

  if(child_pid < 0) {
    perror("Error while forking...");   // Print the error
    return 1;  
  }

  else if(child_pid != 0) {
    return child_pid;   // Return the PID of the child process
  }

  else {
    if(execvp (program, arg_list) == 0);    
    perror("Exec failed");  
    return 1;
  }
}


// Function to kill all the processes
void kill_all(int sig) {
  // Kill all the processes
  kill(pid_procA, SIGKILL);
  kill(pid_procB, SIGKILL);
  
}



int main() {

  

  // Process A
  char * arg_list_A[] = { "/usr/bin/konsole", "-e", "./bin/processA", NULL };
  pid_procA = spawn("/usr/bin/konsole", arg_list_A);

  // Process B
  char * arg_list_B[] = { "/usr/bin/konsole", "-e", "./bin/processB", NULL };
  pid_procB = spawn("/usr/bin/konsole", arg_list_B);

  // Kill all the processes when Ctrl+C is pressed
  signal(SIGINT, kill_all); 

  int status;  // Variable to store the status of the child processes

  // Check PIDs
  waitpid(pid_procA, &status, 0); // Wait for the process A to finish
  waitpid(pid_procB, &status, 0); // Wait for the process B to finish

 
  
  // Print the status of the child processes
  printf ("Main program exiting with status %d\n", status);
  return 0;
}

