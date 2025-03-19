# Olex2: Crystallography at your fingertip!
**Unofficial fork with some specific changes so it can be compiled in my machine. Use it at your own risk. I provide no guarantees it will work! I am not affiliated with Olexsys.**
---
This reposityory contains the source code for the Olex2 software by Olexsys. Please refer to the 
[documentation](https://www.olexsys.org/olex2/docs/) for 
instructions on how to use it. It is mainly divided in 2 repositories. This repository contains the core utils necessary
for the software to run. The other subversion repository contains the GUI, written in Python. 

## Building from source
The prefered method to build from source is using the Scoons script. But firstly it is necessary to compile from source 
wx-widgets 3.0.5. This software is currently incompatible with other versions (You're free to try and make it work!). 

It is pretty important to build it using C++17 standard and it is preferable to use the same compiler for the 
library and for this program. 

### Building wx-widgets:
Firstly you should get a copy of wx-widgets: https://www.wxwidgets.org/downloads/#v3.0.5 and follow the following steps:

GNU/Linux:
1. Create a directory for building and cd into it:
```
mkdir build && cd build
```
2. Choose a compiler and use the following command to compile it:
```
../configure --with-gtk CC=clang CXX=clang++ CXXFLAGS="-std=c++17"
```
You should replace clang with gcc and g++ if you want to compile using GCC. It is important to use C++17 for wx-widgets.
3. Compile and install it:
```
sudo make install -j
```
4. Verify the wx-config version:
```
$ wx-config --version
3.0.5
```
### Building Olex2:
The preferred method for building wx-widgets is the scons. Firstly define 
