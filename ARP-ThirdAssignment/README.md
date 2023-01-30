# Second Assignment for the course "Advanced and Robot Programming" - UniGe
### Developed by  [@Miryru](https://github.com/Miryru), [@alemuraa](https://github.com/alemuraa), [@Ivanterry00](https://github.com/Ivanterry00)
Assignment given by the professor Renato Zaccaria for the course "Advanced and Robot Programming" - Robotics Engineering, at the University of Genova.

The project provides you with a base infrastructure for the implementation of the simulated vision system through shared memory, according to the requirements specified in the PDF file of the assignment.

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
