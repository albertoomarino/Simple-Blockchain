# OS - Operating Systems

We intend to simulate a ledger containing the data of monetary transactions between different users.

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

Copyright (c) [2023] [Alberto Marino]

All rights reserved. This software is protected by copyright laws and international license agreements.

### License Agreement

This License Agreement (hereinafter referred to as the "Agreement") is entered into between the Author or the Right Holder (hereinafter referred to as the "Licensor") and any individual or legal entity (hereinafter referred to as the "Licensee") intending to use the software called [Web-Technologies] (hereinafter referred to as the "Software").

#### 1. GRANT OF LICENSE

Licensor grants Licensee the non-exclusive, non-transferable, revocable right to use the Software in accordance with the terms and conditions of this Agreement.

#### 2. LIMITATIONS

Licensee may not:

a) Copy, distribute, or disclose the Software or any part thereof to any third party, whether free of charge or for any form of compensation.
b) Modify, adapt, reverse engineer, decompile, or disassemble the Software or parts thereof.
c) Remove or alter any copyright notices, proprietary legends, or other identifying symbols in the Software.

#### 3. DISCLAIMER

THE SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED. LICENSOR DISCLAIMS ALL WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT. THE LICENSOR SHALL NOT BE LIABLE FOR ANY DAMAGES ARISING OUT OF THE USE OF THE SOFTWARE.

#### 4. TERMINATION

This Agreement shall remain in force until terminated by either party. The Licensor may terminate this Agreement at any time if the Licensee breaches any of the terms set forth herein. Upon termination, Licensee shall cease all use of the Software and destroy all copies in its possession.

#### 5. APPLICABLE LAW

This Agreement shall be governed by and construed in accordance with the laws of [Jurisdiction]. Any disputes arising out of this Agreement shall be submitted to the exclusive jurisdiction of the courts of [Jurisdiction].

By using the Software, Licensee agrees to be bound by the terms and conditions of this Agreement.

