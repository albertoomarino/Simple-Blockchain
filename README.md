# OS - Operating Systems

## Table of Contents

- [Introduction](#introduction)
- [Files](#files)
- [Usage](#usage)
- [License](#license)

## Introduction

The Operating Systems project served as a valuable opportunity to enhance proficiency in C programming, explore core data structures, delve into process management, and delve into inter-process communication. As previously highlighted, a diverse range of resources were employed to gain a profound insight into the intricacies of operating within a system environment.

## Files

The repository consists of two folders:
- Folder **project**: contains the code of the project;
- Document **project_specifications.pdf**: contains the project track.

## Usage

Once you have installed the project and launched it, you are ready to execute the code

The commands that will be used, as you can see from the `makefile`, are as follows:
- command `make all`: this command is used to compile the project. When executed, three binaries will be generated: master, users, and nodes. Each binary corresponds to a specific part of our system. This command is useful for compiling the entire project in a single operation;
- command `make run`: this command is used to run the main application, in our case master. When you execute make run, the master programme will be started with the argument "macro.txt", which can be a specific input file or a desired configuration. This command simplifies the execution of the master application with custom parameters;
- command `make clean`: this command is useful for cleaning up the project. Running make clean will remove all object (.o) files generated during compilation and the master, users, and nodes binaries. This step is important to keep the project folder clean and remove temporary or unnecessary files.

## License

This project is licensed under the [MIT License](https://github.com/albertoomarino/operating-systems/blob/main/LICENSE).

© [Alberto Marino]
