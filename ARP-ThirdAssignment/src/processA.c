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
        exit(1);
    }
    else if(number_mode == 3){
        perror(msg);
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

    // Control to choose the execution mode
    printf("Choose the execution mode: \n");
    printf("Write 1 for Normal mode: \n");
    printf("Write 2 for Server mode: \n");
    printf("Write 3 Client mode: \n");

    int mode;
    printf("Enter a number: ");
    while (scanf("%d", &mode) != 1 || mode < 1 || mode > 3) {
        printf("Invalid input. Please enter a modality between 1 and 3: ");
        while (getchar() != '\n');
    }

    // vecchio controllo fatto da ale, vediamo se quello sopra è migliore dato che è in poche righe
    
    // int mode;
    // char input[100];
    // int r;
    
    // while (1) {
    //     printf("Enter a number: ");
    //     scanf("%s", input);
    //     int len = strlen(input); 
    //     int valid = 1;
    //     for (int i = 0; i < len; i++) {
    //         if (!isdigit(input[i])) {
    //             valid = 0;
    //             break;
    //         }
    //     }
        
    //     if (valid) {
    //         mode = atoi(input);
    //         if (mode >= 1 && mode <= 3) {
    //             break; 
    //         } 
    //         else {
    //             printf("Invalid input! The number is not 1 or 2 or 3. Try again.\n");
    //         }
    //     } 
        
    //     else {
    //         printf("Invalid input! The number is not 1 or 2 or 3. Try again.\n");
    //     }
    // }

    // Variable declaration in order to get the time
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);

    // Write to the log file
    sprintf(log_buffer, "<Process_A> Modality: %d %s\n", mode, asctime(info));   // Get the time
    check = write(log_fd, log_buffer, strlen(log_buffer));  // Write to the log file
    CheckCorrectness(check);    // Check if the write is correct

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

    if (mode == 1){
        // Utility variable to avoid trigger resize event on launch
        int first_resize = TRUE;

        // Initialize UI
        init_console_ui();

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
    }
    else if (mode == 2){
        // Variable initialization for a server socket connection
        int sockfd, newsockfd;  // File descriptors for the socket and the new socket
        int portno, clilen, n;  // Port number on wich the server accept the connecgtion,
                                // the size of the address of the client, 
                                // and return value for read and write calls

        char buffer[256];   // Buffer to store the message

        struct sockaddr_in serv_addr, cli_addr; // Structure containing an internet address (server and client address)

        // Create a new socket with address domain AF_INET, type SOCK_STREAM, protocol 0
        sockfd = socket(AF_INET, SOCK_STREAM, 0);   
        if (sockfd < 0)
            error("ERROR opening socket", mode); 

        // Initialize the socket structure
        bzero((char *) &serv_addr, sizeof(serv_addr));  // Initialize serv_addr to 0

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // TODO: Check if the port number is valid, i.e. between 2000 and 65535, not float or string or char
        // TODO: se ti viene più comodo puoi anche rifare tutta questa parte di controllo, basta che funzioni ahahahah
        // TODO: solo due cose: 1) input è il buffer che avevi creato tu ieri per leggere la stringa da tastiera e salvarla,
        //       quindi è inizializzato in cima nel tuo codice di ieri. 2) portno è la variabile che contiene il numero della porta
        //       quindi il numero che prenderai da tastiera, e che deve rispettare le condizioni che ti ho scritto sopra, va salvato in questa variabile

        // Get the port number on wich the server will listen from the command line
        printf("\nEnter the port number where the Server is still listening: ");
        while (scanf("%d", &portno) != 1 || portno < 2500 || portno > 65535) {
            printf("\nInvalid input. Port number must be a number between 2000 and 65535! \nInsert a new port number:: ");
            while (getchar() != '\n');
        }

        // quello seguente è il tuo codice, quello prima è il mio.

        // do {
        //     scanf("%s", input);
        //     portno = (int)strtol(input , ( char **) NULL , 10);
        //     if (portno < 2000 || portno > 65535) {
        //         printf("Port number must be a number between 2000 and 65535! Insert a new port number: ");
        //     }
        // } while (portno < 2000 || portno > 65535);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        serv_addr.sin_family = AF_INET; // A short integer value wich contains a code for the address family
        serv_addr.sin_port = htons(portno); // A short integer value wich contains the port number
                                            // The function htons converts a port number in host byte order 
                                            // to a port number in network byte order
        serv_addr.sin_addr.s_addr = INADDR_ANY; // A structure of type struct in_addr which contains only a single field, 
                                                // unsigned long s_addr, wich contains the IP address of the host
        
        // Bind the socket to the address and port number specified in serv_addr
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR on binding", mode);
        else
            printf("\nServer is listening on port %d\n...", portno);

        listen(sockfd,5);   // Listen for connections on a socket

        // Accept a connection on a socket
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept", mode);

        // Read from the socket (from the client to the server)
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n < 0) 
            error("ERROR reading from socket", mode);
        printf("Here is the message: %s\n",buffer); 

        // Write to the socket (from the server to the client)
        n = write(newsockfd,"I got your message",18);
        if (n < 0) 
            error("ERROR writing to socket", mode);
    }
    else if (mode == 3){
        // Variable initialization for a server socket connection
        int sockfd;  // File descriptors for the socket 
        int portno, n;  // Port number on wich the server accept the connecgtion,
                        // and return value for read and write calls

        struct sockaddr_in serv_addr; // Structure containing an internet address (the server address)
        struct hostent *server; // A pointer to a structure of type hostent

        char buffer[256];   // Buffer to store the message

        char address[100]; 
        printf("\nEnter the address of the Server where the Client send the information: ");
        scanf("%s", address);
        printf("\nEnter the port of the Server where the Client send the information: ");
        scanf("%d", &portno);
        
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
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(portno); 

        if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0)
            error("ERROR connecting", mode);

        printf("Please enter the message: ");

        //Write to the socket (from the client to the server)
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket", mode);

        //Read from the socket (from the server to the client)
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0)
            error("ERROR reading from socket", mode);
        printf("%s\n",buffer); 
    }

    return 0;
}