This project is a glfw and vulkan based game that will hopefully be crossplatform.
Right now it is built and ships to only macos. 
Hopefully the libraries and C code written will allow for compatibility with other platforms down the line.

The main game and tests are written in C. 

# try the build
1. Navigate to root directory
2. Click run.sh


# building the project
You must have clang and the ability to use make. Right now the libraries used for this project is dynamically linked.

1. xcode-select --install
2. make test
3. Enjoy!



TODOs: 
- Implment a simple solution for shaders linking for testing and shipping
- 