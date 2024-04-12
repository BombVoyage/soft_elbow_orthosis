# Soft elbow orthosis control

This repository constitutes a method of controlling a soft active elbow orthotic 
device.

Follow this guide step by step to quickly reproduce the results from the report.  

## Overview
The project details can be found in `report` and `presentation`.  
The `arduino` directory contains the microcontroller code to be flashed.  
The `python` directory contains multiple useful files:  

- `data_sender.py` is used to manually specify command values to send to the motor  
- `main.py` is the primary program used to connect the sensor system to the motor system  
- `torque_estimation.ipynb` is used for to create and test the torque estimation neural network models  

The `src` directory mainly contains `driver.cpp` which interfaces the user PC with the EPOS4 driver.  

## Setup
### Environment setup

First, navigate to the `EPOS_Linux_Library` directory and execute the `install.sh` script 
to install the EPOS Driver libraries to your PC.  

```bash
cd EPOS_Linux_Library
chmod +x install.sh
sudo ./install.sh
```

Now you may go back to the repository root and build the `bin/driver` executable.

```bash
cd ..
make
```

For python setup, we recommend using pyenv to create and manage a virtual 
environment.  
Install pyenv and create a virtual environment `venv`.  

```bash
curl https://pyenv.run | bash
pyenv install 3.11.7
pyenv virtualenv 3.11.7 venv
pyenv activate venv
```

Add the following to your `.bashrc` to automatically initialise pyenv whenever
a shell is started:

```bash
export PYENV_ROOT="$HOME/.pyenv"
[[ -d $PYENV_ROOT/bin ]] && export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init -)"
```

Install python package requirements.

```bash
pip install -r python/requirements.txt
```

### Hardware setup

For hardware connections, please refer to `report/report.pdf`.
You may need to flash either the `main_ser.ino` or `main_ble.ino` from the `arduino`
directory to the microcontroller depending on your chosen use case.  

## Usage examples
Once you have set up the environment and hardware, you may conduct multiple 
tests.

### Manual control
The `python/data_sender.py` program can be used to send commands to the driver. 
Launch the driver in current control mode.
```bash
driver -c
```

In a new terminal, launch the data\_sender and send a command of 400mA.
```bash
python python/data_sender.py
```

### sEMG control
The `python/main.py` function allows to interface the sensor system with the driver.  
The `-d` option allows for testing without driver connection.  
The `-o` option is used to specify the name for the generated plot and data files.  
The `-t` option lets the user set the test time.  
The `-b` option is used to set the communication to BLE. The default operation mode is Serial.  

Launch the driver in current control mode.
```bash
driver -c
```

Launch a basic test.
```bash
python python/main.py -bv
```
