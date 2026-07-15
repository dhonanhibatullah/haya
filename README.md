# **haya**

## **A. Prerequisites**

### **1. Python**

#### **1.1. On Windows**

* Follow Python installation guide from [this](https://www.python.org/downloads/).

#### **1.2. On Linux Debian**

* Most Linux distros already have Python inside of it. If not:

    ```bash
    sudo apt update
    sudo apt install python3 python3-pip python3-venv
    ```

### **2. Espressif Installation Manager**

#### **2.1. On Windows**

* Follow installation guide for EIM GUI from [this](https://docs.espressif.com/projects/idf-im-ui/en/latest/#windows-installation).

* From EIM GUI, choose `Custom Installation` > `Select Target: all` > `Select IDF Version: v6.0.2` > all the default. Wait for the installation to complete.

* Use `ESP-IDF PowerShell` or `ESP-IDF Command Prompt` from the start menu when developing.

#### **2.2. On Linux Debian, CLI Recommended**

* Install the EIM-CLI:

    ```bash
    # Add the Espressif APT repository
    echo "deb [trusted=yes] https://dl.espressif.com/dl/eim/apt/ stable main" | sudo tee /etc/apt/sources.list.d/espressif.list

    # Update package lists
    sudo apt update

    # Install CLI only
    sudo apt install eim-cli
    ```

* Install ESP-IDF v6.0.2:

    ```bash
    eim install -i v6.0.2
    ```

* Add this command to `.bashrc` to ease `idf.py` environment setup:

    ```bash
    echo "alias idf-v6.0.2='source \"\$HOME/.espressif/v6.0.2/esp-idf/export.sh\"'" >> ~/.bashrc
    source ~/.bashrc
    ```

* Then activate ESP-IDF with:

    ```bash
    idf-v6.0.2
    ```

### **3. VSCode Setup**

#### **3.1. Clangd Extension**

* Install `clangd` extension on VSCode.

* Go to `Settings` > `Clangd: Arguments` and add:

    - for Linux:

        ```text
        --query-driver=**/xtensa-esp-elf/**/bin/xtensa-esp*-elf-gcc,**/xtensa-esp-elf/**/bin/xtensa-esp*-elf-g++,**/riscv32-esp-elf/**/bin/riscv32-esp-elf-gcc,**/riscv32-esp-elf/**/bin/riscv32-esp-elf-g++
        ```
    
    - for Windows:

        ```text
        --query-driver=C:/Espressif/tools/xtensa-esp-elf/*/xtensa-esp-elf/bin/xtensa-esp32-elf-gcc.exe,C:/Espressif/tools/xtensa-esp-elf/*/xtensa-esp-elf/bin/xtensa-esp32s3-elf-gcc.exe,C:/Espressif/tools/riscv32-esp-elf/*/riscv32-esp-elf/bin/riscv32-esp-elf-gcc.exe
        ```

* After opening a `.c` or `.h` file, you may be prompted to install `Clangd` on your machine. Accept it.

* Right-click when editing the `.c` or `.h` file and go to `Format Document With...` > `Configure Default Formatter...` > `clangd`.

* Go to `Settings` > `Editor: Format On Save` and enable it to format the file everytime the `.c` or `.h` file is saved.

#### **3.2. ESP-IDF Extension**

* Install `ESP-IDF` extension on your VSCode. This helps the development so much if you are not familiar with command lines.
