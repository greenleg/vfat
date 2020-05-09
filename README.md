# vfat
The virtual file system based on FAT (file allocation table). The whole file system completely resides in a single file.
## Build
In order to build a binary, just run `build-release.sh`:
```
./build-release.sh
```
The output executable `vfat` will be placed into `../build-vfat-Release` by default.
## Usage
First, it needs to create and format a file system using the following command:
```
./vfat -f -dev:<file-name> -size:<volume-size>
```
E.g. `./vfat -f -dev:db0 -size:50` creates a new volume 'db0' of maximum 50MB size.
After displaying the command line prompt `$` one can enter and execute various commands.
The existing file system can be opened by just specifying the path to the virtual disk:
```
./vfat -dev:<file-name>
```
E.g. `./vfat -dev:db0` opens the virtual volume 'db0'.
## Command Line Interface
The following basic commands are supported:
```
ls     - Lists all files in the given directory;
mkdir  - Attempts to create a directory;
cd     - Changes the current directory (the directory in which the user is currently working);
touch  - Creates an empty file;
cat    - Displays text file on screen;
import - Imports a file or a directory from native Linux file system;
mv     - Moves a file or a directory;
cp     - Copies a file or a directory;
rm     - Deletes a file or a directrory;
tree   - Displays the hierarchical structure of the current directory;
exit   - Stops the process and goes back to Linux;
```
