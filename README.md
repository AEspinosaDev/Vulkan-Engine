 <H1 ALIGN="CENTER"> VULKAN 3D LIBRAY 👀 </H1>
<p align="center"> 
A Vulkan based 3D library that acts as a wrapper for the most verbose Vulkan parts that I implemented to learn the API.

![Screenshot (51)](https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/89759b3c-b129-4f54-b177-8b9c7ac3b14c)
![Screenshot (52)](https://github.com/AEspinosaDev/Vulkan-Engine/assets/79087129/32acad6d-4ddb-4952-a877-08e8a20fdf7e)

## Project Structure 🗃️

It consists of two parts:

- The library itself, a place where you can find a lot of abstracted functionality.

- A demo application that links against the library.

The main feautures of the library are:

- PBR, Phong and other types of materials abstractions.
- Scene and scene objects abstractions.
- Texture loading with mipmapping.
- On the fly shader compiling.
- Dynamic renderer states for some graphic features.
- Multipass (depth pass for shadows).
- Vulkan object and functionality abstraction.
- Simple to use user interface (Unfinished).
- Easy to distribute source code.

This project is finished for now, It has a long way until becoming a decent library, there are no fancy features for now, but all the basics are here. I will continue working on it in a couple of months, adding more advance functionality.

## Project building and usage 🛠️

The prequisites for using this code are:

- Windows 10, 11 (Although it should be easy enough to set it up for Linux).
- Vulkan SDK 1.3.* installed.
- Cmake installed.

First, build the project using Cmake, no further configuration needed. The project is configured in such a way that, during the build process, CMake takes care of automatically locating and linking all dependencies on the system, as well as importing them from the internet if they are not available, with exception of the Vulkan SDK, due to its more complex installation. This has been done to facilitate an easy and lightweight distribution of the source code, sparing the user the effort of manual configuration. Although the project has been implemented in Visual Studio Code, a practical file structure has been configured for CMake in case it is opened in Visual Studio.

Once the project is opened in the IDE of choice, compile it in the desired mode, and it would be ready to run. The CMake configuration is set for a 64-bit architecture, but it can be changed. CMake also takes care of automatically configuring the paths for resource files.

The project compiles dependencies, the 3D library, and the demonstration application, which statically links against the 3D library. The library is a STATIC lib, do not try to link dynamically against it.
