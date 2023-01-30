#include "./../include/processB_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <bmpfile.h>
#include <math.h>
#include <semaphore.h>
#include <errno.h>

// Declaration variables for bmp file
const int width = 1600;
const int height = 600;
const int depth = 4;

// Define struct for shared memory and variables
struct shared
{
    int m[1600][600];   // Matrix for the shared memory
};

// Define semapthores
sem_t *semaphore;
sem_t *semaphore2;

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

int main(int argc, char const *argv[])
{
    // Open the log file
    if ((log_fd = open("processB.log",O_WRONLY|O_APPEND|O_CREAT, 0666)) == -1)
    {
        // If the file could not be opened, print an error message and exit
        perror("Error opening command file");   // Print the error message
        exit(1);
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Variable declaration in order to access to shared memory
    key_t          ShmKEY;
    int            ShmID;

    // Pointer to the struct of shared memory
    struct shared  *ShmPTR;



    ShmKEY = ftok(".", 'x');    // Get the key of the shared memory
    ShmID = shmget(ShmKEY, sizeof(struct shared), 0666);    // Get the ID of the shared memory

    if (ShmID < 0) {
        perror("*** shmget error (server) ***\n");  // If the ID is not correct, print an error message
        exit(1);
    }

    ShmPTR = (struct shared *) shmat(ShmID, NULL, 0);   // Attach the shared memory to the pointer

    if ((int) ShmPTR == -1) {
        perror("*** shmat error (server) ***\n");   // If the pointer is not correct, print an error message
        exit(1);
    }

    bmpfile_t *bmp;
    bmp = bmp_create(width, height, depth);

    int center_cord = 0;    // Variable to store the center of the circle
    int y_cord[600];    // Array to store the y coordinates of the circle
    int x_cord[600];    // Array to store the x coordinates of the circle
    int y;  
    int flag ;
    int old_x;
    int old_y;

    int radius = 30;    // Radius of the circle

    semaphore = sem_open("/mysem", 0);      // Open the semaphore
    semaphore2 = sem_open("/mysem2", 0);    // Open the semaphore


    if(semaphore == (void*) -1)
    {
        perror("sem_open failure"); // If the semaphore is not correct, print an error message
        exit(1);
    }

    if(semaphore2 == (void*) -1)
    {
        perror("sem_open failure"); // If the semaphore is not correct, print an error message
        exit(1);
    }

    // Variable declaration in order to get the time
    time_t rawtime;
    struct tm *info;

    // Infinite loop
    while (TRUE) {

        // Get current time
        time(&rawtime);
        info = localtime(&rawtime);

        // Get input in non-blocking mode
        int cmd = getch();
       
        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;   // Avoid trigger resize event on launch
            }
            else {
                reset_console_ui(); // Re-draw UI
            }
        }
        else {
            mvaddch(LINES/2, COLS/2, '0');  // Draw the center of the circle

            refresh();  // Refresh the screen

            sem_wait(semaphore2);   // Wait the semaphore

            //old center
            if(y_cord[y-1] != 0 && x_cord[y-1] != 0)
            {
                old_x = x_cord[y-1];    // Get the old x coordinate
                old_y = y_cord[y-1];    // Get the old y coordinate
            }
            
            for (int i=0;i<600;i++)
            {
                if(y_cord[i] != 0)
                {
                    y_cord[i]=0;    // Reset the y coordinates
                }
                else if(x_cord[i] != 0)
                {
                    x_cord[i]=0;    // Reset the x coordinates
                }
            
            }

            center_cord = 0;    // Reset the center of the circle
        
            int i, j;
            y = 0;
            flag = 0;
            
            //algorithm to find the center of the circle
            for (i = 0; i < 1600; i++) {
                
                if (flag == 1) {
                    break;
                }

                for (j = 0; j < 600; j++) {
                    if (ShmPTR->m[i][j] == 1)
                    {
                        y_cord[y] = j;  // Store the y coordinate
                        x_cord[y] = i;  // Store the x coordinate

                        // If the y coordinate is greater than the previous one, break the loop
                        if (y_cord[y] > y_cord[y-1]) {
                            flag = 1;
                            break;
                        }

                        y++;    // Increment the y index
                        break;
                    }
                }
            }

            sprintf(log_buffer, "<Process_B> Position of center updated: %s\n", asctime(info));
            check = write(log_fd, log_buffer, strlen(log_buffer));  // Write the log message
            CheckCorrectness(check);    // Check if the write is correct

            mvaddch(floor((int)((y_cord[y-1]+radius)/20)),floor((int)(x_cord[y-1]/20)), '0');   

            refresh();

            //increment the semaphore
            sem_post(semaphore);

            
            //update the position of the blue circle
            cancel_blue_circle(radius,old_x,old_y,bmp); // Cancel the old blue circle

            draw_blue_circle(radius,x_cord[y-1],y_cord[y-1],bmp);  // Draw the new blue circle
        }
    }

    sem_close(semaphore);   // Close the semaphore
    sem_close(semaphore2);  // Close the semaphore2

    shmdt((void *) ShmPTR); // Detach the shared memory
    bmp_destroy(bmp);       // Destroy the bitmap

    endwin();   // End the window

    // Close the log file
    close(log_fd);

    return 0;
}
