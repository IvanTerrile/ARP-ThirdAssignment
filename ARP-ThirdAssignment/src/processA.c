#include "./../include/processA_utilities.h"
#include <ctype.h>
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

// Include libraries for socket connection
#include <sys/types.h>  // Includes many data types used in system calls
#include <sys/socket.h> // Includes a number of definitions of structures need for a socket
#include <netinet/in.h> // Includes constants and structures needed for internet domain addresses   //
#include <netdb.h> 

// Function called when a system call for the socket fails
void error(char *msg, int number_mode)
{
    if(number_mode == 2){
        perror(msg);
        sleep(5);
        exit(1);
    }
    else if(number_mode == 3){
        perror(msg);
        sleep(5);
        exit(0);
    }
} 

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

int main(int argc, char *argv[]){
    

    // Control to choose the execution mode
    printf("Select execution mode: \n 1. Normal mode \n 2. Server mode \n 3. Client mode \n");

    int mode;
    printf("Enter mode number: ");
    while (scanf("%d", &mode) != 1 || mode < 1 || mode > 3) {
        printf("Invalid input. Please enter a modality between 1 and 3: ");
        while (getchar() != '\n');
    }
   
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

///////////////////////////////////////////////START/////////////////////////////////////////////////

    /*Socket descriptor for client or server mode*/
    int sockfd, newsockfd;
    int portno;//, clilen;

    switch(mode){
        case 2:{
            int clilen;   // Length of the address of the client

            struct sockaddr_in serv_addr, cli_addr; // Structure containing an internet address (server and client address)

            // Create a new socket with address domain AF_INET, type SOCK_STREAM, protocol 0
            sockfd = socket(AF_INET, SOCK_STREAM, 0);   
            if (sockfd < 0)
                error("ERROR opening socket", mode); 

            // Initialize the socket structure
            bzero((char *) &serv_addr, sizeof(serv_addr));  // Initialize serv_addr to 0

            // Get the port number on which the server will listen from the command line
            printf("\nEnter the port number where the Server will be listening to: ");
            while (scanf("%d", &portno) != 1 || portno < 2500 || portno > 65535) {
                printf("\nInvalid input. Port number must be a number between 2000 and 65535! \nInsert a new port number:: ");
                while (getchar() != '\n');
            }

            serv_addr.sin_family = AF_INET; // A short integer value wich contains a code for the address family
            serv_addr.sin_addr.s_addr = INADDR_ANY; // A structure of type struct in_addr which contains only a single field, 
                                                    // unsigned long s_addr, wich contains the IP address of the host
            serv_addr.sin_port = htons(portno); // A short integer value wich contains the port number
                                                // The function htons converts a port number in host byte order 
                                                // to a port number in network byte order
            
            
            // Bind the socket to the address and port number specified in serv_addr
            if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                error("ERROR on binding", mode);
            else
                printf("\nServer is listening to the port %d\n...", portno);

            listen(sockfd,5);   // Listen for connections on a socket

            // Accept a connection on a socket
            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if (newsockfd < 0)
                error("ERROR on accept", mode);
            else
                printf("\nConnection accepted!\n");
        } break;

        case 3:{
            struct sockaddr_in serv_addr; // Structure containing an internet address (the server address)
            struct hostent *server; // A pointer to a structure of type hostent

            char buffer[256];   // Buffer to store the message

            char address[100]; 
            printf("\nEnter the address of the Server where the Client will send the information: ");
            scanf("%s", address);
            printf("\nEnter the port of the Server where the Client will send the information: ");
            while (scanf("%d", &portno) != 1 || portno < 2500 || portno > 65535) {
                printf("\nInvalid input. Port number must be a number between 2000 and 65535! \nInsert a new port number:: ");
                while (getchar() != '\n');
            }
            
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
                error("ERROR opening socket", mode);

            server = gethostbyname(address);
            if (server == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
            } 

            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;

            bcopy((char *)server->h_addr, 
            (char *)&serv_addr.sin_addr.s_addr, 
            server->h_length);

            serv_addr.sin_port = htons(portno); 

            if (connect(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
                error("ERROR connecting", mode);
            else 
                printf("\nConnection established!\n");

        } break;
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Infinite loop
    while (TRUE)
    {
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

        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if (mode == 1 || mode == 3){
                    if(check_button_pressed(print_btn, &event)) {
                        mvprintw(LINES - 1, 1, "Print button pressed"); // Print a message on the screen
                        
                        
                        if (mode == 3){
                        char str_cmd[5];
                        snprintf(str_cmd, 5, "%d",cmd);
                                        
                        if(write(sockfd, str_cmd, 5) < 0){
                            error("ERROR writing to socket", mode);
                        }
                        } // Save the bmp file
                        bmp_save(bmp, "out/image.bmp");
                        refresh();
                        sleep(1);
                        for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                            mvaddch(LINES - 1, j, ' '); // Clear the message on the screen
                        }
                    }
                }
            }
        }
        else if (mode == 2)
        {
            char input_string[5];
           
            //Read from the socket//   
            if(read(newsockfd,input_string,5) < 0){
                error("ERROR reading from socket", mode);
            }
           
            int com = atoi(input_string);

            if(com == KEY_LEFT || com == KEY_RIGHT || com == KEY_UP || com == KEY_DOWN)
            {
                sem_wait(semaphore);    // Wait for the semaphore
        
                move_circle(com);   // Move the circle
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
            else if (com == KEY_MOUSE) 
            {
                mvprintw(LINES - 1, 1, "Screen taken!");
                bmp_save(bmp, "out/image.bmp");
                refresh();
                sleep(1);
                for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                    mvaddch(LINES - 1, j, ' '); // Clear the message on the screen
                }
            }
        }

        
        else if (mode == 1 || mode == 3){
            // If input is an arrow key, move circle accordingly...
            if(cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN) 
            {
                if (mode == 3){
                    char str_cmd[5];
                    snprintf(str_cmd, 5, "%d",cmd);
                                        
                    if(write(sockfd, str_cmd, 5) < 0){
                        error("ERROR writing to socket", mode);
                    }
                }
                sem_wait(semaphore);    // Wait for the semaphore

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
    }

    sem_close(semaphore);   // Close the semaphore
    sem_unlink("/mysem");   // Unlink the semaphore
    sem_close(semaphore2);  // Close the semaphore
    sem_unlink("/mysem2");  // Unlink the semaphore
    shmdt((void *) ShmPTR); // Detach the shared memory
    shmctl(ShmID, IPC_RMID, NULL);  // Remove the shared memory
    bmp_destroy(bmp);   // Destroy the bmp file
    endwin();   // End the window

    return 0;
}
