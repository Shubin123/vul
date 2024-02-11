This project is a glfw and vulkan based game that will hopefully be crossplatform.
Right now it is built and ships to only macos (x86-64/AMD64). 
Hopefully the libraries and C code written will allow for compatibility with other platforms down the line.

The main game and tests are written in C. 

# try the build
1. Navigate to root directory
2. Click run.sh


# building the project
You must have clang and the ability to use make. Right now the libraries used for this project is dynamically linked.

1. xcode-select --install
2. install vulkan sdk, brew install glfw, brew install cglm
3. make test

# TODOs
- spawn multiple objects
- bundle the app for macos
- manage more data with structs
- use said structs in functions for multiple returns (ex createIndexBuffer no longer needs to be void)
- refactor into multiple files
- verify shipping/building instructions on seperate machine/vm

