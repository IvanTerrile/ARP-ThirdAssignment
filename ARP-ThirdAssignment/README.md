# Third Assignment for the course "Advanced and Robot Programming" - UniGe
### Developed by  [@Miryru](https://github.com/Miryru), [@alemuraa](https://github.com/alemuraa), [@IvanTerrile](https://github.com/IvanTerrile)
Assignment given by the professor Renato Zaccaria for the course "Advanced and Robot Programming" - Robotics Engineering, at the University of Genova.

The project uses as a basis the implementation of the Second Assignment. But in this implementation you can choose between three modes: 1) Normal Mode, which replicates the functionality of the second assignment; 2) Server Mode, which displays on a remote machine in the windows ncurses of process A and B, respectively the movement and trajectory performed through the client; 3) Client Mode, which via keyboard input moves the spot in the ncurses window of process A and the trajectory in the ncurses window of process B both in the local windows of the client and remote windows of the server. In addition, the client can take a picture of the spot and save it in the client and also in the server.

### ```Read the [info.txt] to know the info about the project.```
-----

## Library installation and usage guide 
To execute the program you need to install the *libbitmap* library. This library is used to read and write bitmap images. It is a shared library, which means that it is not linked to the executable at compile time, but rather at runtime. This allows the library to be updated without having to recompile the programs which use it.

To work with the bitmap library, you need to follow these steps:
1. Download the source code from [this GitHub repo](https://github.com/draekko/libbitmap.git) in your file system.
2. Navigate to the root directory of the downloaded repo and run the configuration through command ```./configure```. Configuration might take a while.  While running, it prints some messages telling which features it is checking for.
3. Type ```make``` to compile the package.
4. Run ```make install``` to install the programs and any data files and documentation.
5. Upon completing the installation, check that the files have been properly installed by navigating to ```/usr/local/lib```, where you should find the ```libbmp.so``` shared library ready for use.
6. In order to properly compile programs which use the *libbitmap* library, you first need to notify the **linker** about the location of the shared library. To do that, you can simply add the following line at the end of your ```.bashrc``` file:      
```export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"```

### Use of the *libbitmap* library in our programs
To use it in our programs:
1. include the library in your programs via ```#include <bmpfile.h>```. If you want to check the content of ```bmpfile.h``` to glimpse the functionalities of the library, navigate to ```/usr/local/include```, where the header file should be located.
2. compile programs which use the *libbitmap* library by linking the shared library with the ```-lbmp``` command.     
