#include "./../include/processA_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <bmpfile.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

// Declaration variables for bmp file
const int width = 1600;
const int height = 600;
const int depth = 4;

// Define struct for shared memory and variables
struct shared
{
    int m[1600][600];   // Matrix of shared memory
};

// Define semaphores
sem_t *semaphore;   // Semaphore
sem_t *semaphore2;  // Semaphore

// Buffer to store the string to write to the log file
char log_buffer[100];

// File descriptor for the log file
int log_fd;

// Variable to store the value of the write function
int check;

// Function to check the correctness of the operation just done
void CheckCorrectness(int c) 
{
    if(c == -1) 
    {
        close(log_fd);  // Close the log file
        perror("Error in writing function");    // Print the error message
        exit(1);
    }
}

// Function to draw a blue circle
void draw_blue_circle(int radius,int x,int y, bmpfile_t *bmp) {

    rgb_pixel_t pixel = {255, 0, 0, 0}; // Blue color
    
    // Loop through the circle
    for(int i = -radius; i <= radius; i++) {
        for(int j = -radius; j <= radius; j++) {
            // If distance is smaller, point is within the circle
            if(sqrt(i*i + j*j) < radius) {
                /*
                * Color the pixel at the specified (x,y) position
                * with the given pixel values
                */
                bmp_set_pixel(bmp, x*20 + i, y*20 + j, pixel);  // Draw the circle
            }
        }
    }
}

// Function to cancel the blue circle
void cancel_blue_circle(int radius,int x,int y, bmpfile_t *bmp) {

    rgb_pixel_t pixel = {255, 255, 255, 0}; // White color
    for(int i = -radius; i <= radius; i++) {
        for(int j = -radius; j <= radius; j++) {
            // If distance is smaller, point is within the circle
            if(sqrt(i*i + j*j) < radius) {
                /*
                * Color the pixel at the specified (x,y) position
                * with the given pixel values
                */
                bmp_set_pixel(bmp,  x*20+i,y*20+  j, pixel);    // Cancel the circle
            }
        }
    }
}

int main(int argc, char *argv[])
{
    // Open the log file
    if ((log_fd = open("processA.log",O_WRONLY|O_APPEND|O_CREAT, 0666)) == -1)
    {
        // If the file could not be opened, print an error message and exit
        perror("Error opening command file");
        exit(1);
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Delcare circle radius
    int radius = 30;

    // Variable declaration in order to access to shared memory
    key_t ShmKEY;
    int ShmID;

    // Pointer to the struct of shared memory
    struct shared  *ShmPTR;


    ShmKEY = ftok(".", 'x');    // Get the key
    ShmID = shmget(ShmKEY, sizeof(struct shared), IPC_CREAT | 0666);    // Get the ID
    if (ShmID < 0) {
        perror("*** shmget error (server) ***\n");  // If the ID is not correct, print an error message
        exit(1);
    }


    ShmPTR = (struct shared *) shmat(ShmID, NULL, 0);   // Attach the shared memory
    if ((int) ShmPTR == -1) {
        perror("*** shmat error (server) ***\n");   // If the attach is not correct, print an error message
        exit(1);
    }

    bmpfile_t *bmp;
    bmp = bmp_create(width, height, depth); // Create the bmp file
    semaphore = sem_open("/mysem", O_CREAT, 0666, 1);   // Open the semaphore
    semaphore2 = sem_open("/mysem2", O_CREAT, 0666, 0); // Open the semaphore

    if(semaphore == (void*)-1)
    {
        perror("sem_open failure"); // If the semaphore is not correct, print an error message
        exit(1);
    }

    if(semaphore2 == (void*)-1)
    {
        perror("sem_open failure"); // If the semaphore is not correct, print an error message
        exit(1);
    }

    // Variable declaration in order to get the time
    time_t rawtime;
    struct tm *info;

    // Infinite loop
    while (TRUE)
    {
        // Get current time
        time(&rawtime);
        info = localtime(&rawtime);

        // Get input in non-blocking mode
        int cmd = getch();

        int x = circle.x;   // Get the x coordinate of the circle
        int y = circle.y;   // Get the y coordinate of the circle

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;   // Avoid trigger resize event on launch
            }
            else {
                reset_console_ui(); // Re-draw UI
            }
        }

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Print button pressed"); // Print a message on the screen

                    // Write to the log file
                    sprintf(log_buffer, "<Process_A> Print button pressed: %s\n", asctime(info));   // Get the time
                    check = write(log_fd, log_buffer, strlen(log_buffer));  // Write to the log file
                    CheckCorrectness(check);    // Check if the write is correct

                    bmp_save(bmp, "out/image.bmp"); // Save the bmp file

                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' '); // Clear the message on the screen
                    }
                }
            }
        }
        
        // If input is an arrow key, move circle accordingly...
        else if(cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN) 
        {
            
            sem_wait(semaphore);    // Wait for the semaphore
            
            // Write to the log file
            sprintf(log_buffer, "<Process_A> Keyboard button pressed: %s\n", asctime(info));    // Get the time
            check = write(log_fd, log_buffer, strlen(log_buffer));  // Write to the log file
            CheckCorrectness(check);    // Check if the write is correct
            
            move_circle(cmd);   // Move the circle
            draw_circle();  // Draw the circle
    
            cancel_blue_circle(radius,x,y, bmp);    // Cancel the circle
            for (int i = 0; i < 1600; i++) {
                for (int j = 0; j < 600; j++) {
                    ShmPTR->m[i][j] = 0;    // Set the shared memory to 0
                }
            }
            
            draw_blue_circle(radius,circle.x,circle.y, bmp);    // Draw the circle
            
            // Write to the shared memory
            for (int i = 0; i < 1600; i++) {
                for (int j = 0; j < 600; j++) {
                    rgb_pixel_t *pixel = bmp_get_pixel(bmp, i, j);  // Get the pixel
                    
                    // If the pixel is blue, set the shared memory to 1
                    if ((pixel->blue == 255) && (pixel->red == 0) && (pixel->green==0) && (pixel->alpha==0)) {
                        ShmPTR->m[i][j] = 1;
                    }
                }
            }

            sem_post(semaphore2);     // Post the semaphore  
        }
            
    }

        sem_close(semaphore);   // Close the semaphore
        sem_unlink("/mysem");   // Unlink the semaphore
        sem_close(semaphore2);  // Close the semaphore
        sem_unlink("/mysem2");  // Unlink the semaphore
        shmdt((void *) ShmPTR); // Detach the shared memory
        shmctl(ShmID, IPC_RMID, NULL);  // Remove the shared memory
        bmp_destroy(bmp);   // Destroy the bmp file
        endwin();   // End the window

        // Close the log file
        close(log_fd);

        return 0;
}
