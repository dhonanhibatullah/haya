# **haya**

## **A. Prerequisites**

### 1. ESP-IDF with EIM

### 2. Visual Studio Code

### 3. Clangd

- Install `clangd` extension on the vscode

- Add the arguments:

    ```bash
    --query-driver=**/xtensa-esp*-elf-gcc,**/xtensa-esp*-elf-g++,**/riscv32-esp-elf-gcc,**/riscv32-esp-elf-g++
    ```