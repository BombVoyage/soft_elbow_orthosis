#include "Definitions.h"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>
#include "../include/driver.h"
#include "../include/listener.h"
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>

void *g_pKeyHandle = 0;
unsigned short g_usNodeId = 1;
string g_deviceName;
string g_protocolStackName;
string g_interfaceName;
string g_portName;
int g_baudrate = 0;
EAppMode g_eAppMode = CURRENT;

const string g_programName = "driver";

void PrintUsage() {
  cout << "Usage: driver" << endl;
  cout << "\t-h : this help" << endl;
  cout << "\t-c : current control mode" << endl;
  cout << "\t-p : position control mode" << endl;
  cout << "\t-v : velocity control mode" << endl;
}

void LogError(string functionName, int p_lResult, unsigned int p_ulErrorCode) {
  cerr << g_programName << ": " << functionName
       << " failed (result=" << p_lResult << ", errorCode=0x" << std::hex
       << p_ulErrorCode << ")" << endl;
}

void LogInfo(string message) { cout << message << endl; }

void SeparatorLine() {
  const int lineLength = 65;
  for (int i = 0; i < lineLength; i++) {
    cout << "-";
  }
  cout << endl;
}

void PrintSettings() {
  stringstream msg;

  msg << "default settings:" << endl;
  msg << "node id             = " << g_usNodeId << endl;
  msg << "device name         = '" << g_deviceName << "'" << endl;
  msg << "protocal stack name = '" << g_protocolStackName << "'" << endl;
  msg << "interface name      = '" << g_interfaceName << "'" << endl;
  msg << "port name           = '" << g_portName << "'" << endl;
  msg << "baudrate            = " << g_baudrate;

  LogInfo(msg.str());

  SeparatorLine();
}

void SetDefaultParameters() {
  // USB
  g_usNodeId = 1;
  g_deviceName = "EPOS4";
  g_protocolStackName = "MAXON SERIAL V2";
  g_interfaceName = "USB";
  g_portName = "USB0";
  g_baudrate = 1000000;
}

int OpenDevice(unsigned int *p_pErrorCode) {
  int lResult = MMC_FAILED;

  char *pDeviceName = new char[255];
  char *pProtocolStackName = new char[255];
  char *pInterfaceName = new char[255];
  char *pPortName = new char[255];

  strcpy(pDeviceName, g_deviceName.c_str());
  strcpy(pProtocolStackName, g_protocolStackName.c_str());
  strcpy(pInterfaceName, g_interfaceName.c_str());
  strcpy(pPortName, g_portName.c_str());

  LogInfo("Open device...");

  g_pKeyHandle = VCS_OpenDevice(pDeviceName, pProtocolStackName, pInterfaceName,
                                pPortName, p_pErrorCode);

  if (g_pKeyHandle != 0 && *p_pErrorCode == 0) {
    unsigned int lBaudrate = 0;
    unsigned int lTimeout = 0;

    if (VCS_GetProtocolStackSettings(g_pKeyHandle, &lBaudrate, &lTimeout,
                                     p_pErrorCode) != 0) {
      if (VCS_SetProtocolStackSettings(g_pKeyHandle, g_baudrate, lTimeout,
                                       p_pErrorCode) != 0) {
        if (VCS_GetProtocolStackSettings(g_pKeyHandle, &lBaudrate, &lTimeout,
                                         p_pErrorCode) != 0) {
          if (g_baudrate == (int)lBaudrate) {
            lResult = MMC_SUCCESS;
          }
        }
      }
    }
  } else {
    g_pKeyHandle = 0;
  }

  delete[] pDeviceName;
  delete[] pProtocolStackName;
  delete[] pInterfaceName;
  delete[] pPortName;

  return lResult;
}

int CloseDevice(unsigned int *p_pErrorCode) {
  int lResult = MMC_FAILED;

  *p_pErrorCode = 0;

  LogInfo("Close device");

  if (VCS_CloseDevice(g_pKeyHandle, p_pErrorCode) != 0 && *p_pErrorCode == 0) {
    lResult = MMC_SUCCESS;
  }

  return lResult;
}

int ParseArguments(int argc, char **argv) {
  int lOption;
  int lResult = MMC_SUCCESS;

  opterr = 0;

  while ((lOption = getopt(argc, argv, "hcpv")) != -1) {
    switch (lOption) {
    case 'c':
      g_eAppMode = CURRENT;
      break;
    case 'p':
      g_eAppMode = POSITION;
      break;
    case 'v':
      g_eAppMode = VELOCITY;
      break;
    case 'h': 
      PrintUsage();
      exit(1);
    case '?': // unknown option...
      stringstream msg;
      msg << "Unknown option: '" << char(optopt) << "'!";
      LogInfo(msg.str());
      PrintUsage();
      lResult = MMC_FAILED;
      break;
    }
  }

  return lResult;
}

int Prepare(unsigned int *p_pErrorCode) {
  int lResult = MMC_SUCCESS;
  BOOL oIsFault = 0;

  if (VCS_GetFaultState(g_pKeyHandle, g_usNodeId, &oIsFault, p_pErrorCode) ==
      0) {
    LogError("VCS_GetFaultState", lResult, *p_pErrorCode);
    lResult = MMC_FAILED;
  }

  if (lResult == 0) {
    if (oIsFault) {
      stringstream msg;
      msg << "clear fault, node = '" << g_usNodeId << "'";
      LogInfo(msg.str());

      if (VCS_ClearFault(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0) {
        LogError("VCS_ClearFault", lResult, *p_pErrorCode);
        lResult = MMC_FAILED;
      }
    }

    if (lResult == 0) {
      BOOL oIsEnabled = 0;

      if (VCS_GetEnableState(g_pKeyHandle, g_usNodeId, &oIsEnabled,
                             p_pErrorCode) == 0) {
        LogError("VCS_GetEnableState", lResult, *p_pErrorCode);
        lResult = MMC_FAILED;
      }

      if (lResult == 0) {
        if (!oIsEnabled) {
          if (VCS_SetEnableState(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0) {
            LogError("VCS_SetEnableState", lResult, *p_pErrorCode);
            lResult = MMC_FAILED;
          }
        }
      }
    }
  }
  return lResult;
}

void signal_handler(int signal, siginfo_t *si, void *unused) {
  unsigned int lErrorCode = 0;
  int cmd = si->si_value.sival_int;
  switch (g_eAppMode) {
  case CURRENT: {
    // Command the motor
    cout << "Current: " << cmd << "mA" << endl;
    if (VCS_SetCurrentMust(g_pKeyHandle, g_usNodeId, cmd, &lErrorCode) ==
        0)
      printf("Set current failed\n");
    break;
  }
  case POSITION: {
    // Command the motor
    cout << "Position: " << cmd << endl;
    if (VCS_MoveToPosition(g_pKeyHandle, g_usNodeId, cmd, 1, 0, &lErrorCode) ==
        0)
      printf("Move to position failed\n");
    break;
  }
  case VELOCITY: {
    // Command the motor
    cout << "Velocity: " << cmd << "rpm" << endl;
    if (VCS_MoveWithVelocity(g_pKeyHandle, g_usNodeId, cmd, &lErrorCode) ==
        0)
      printf("Move with velocity failed\n");
    break;
  }
  case UNKNOWN:
  break;
  }
}

void Activate(unsigned int *p_pErrorCode, int *p_Result) {
  switch (g_eAppMode) {
  case CURRENT: {
    printf("Launching in current control mode.\n");
    if (VCS_ActivateCurrentMode(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0) {
      LogError("VCS_ActivateCurrentMode", *p_Result, *p_pErrorCode);
      return;
    }
  } break;
  case POSITION: {
    printf("Launching in position control mode.\n");
    if (VCS_ActivateProfilePositionMode(g_pKeyHandle, g_usNodeId,
                                        p_pErrorCode) == 0) {
      LogError("VCS_ActivateProfilePositionMode", *p_Result, *p_pErrorCode);
      return;
    }
  } break;
  case VELOCITY: {
    printf("Launching in velocity control mode.\n");
    if (VCS_ActivateProfileVelocityMode(g_pKeyHandle, g_usNodeId,
                                        p_pErrorCode) == 0) {
      LogError("VCS_ActivateProfileVelocityMode", *p_Result, *p_pErrorCode);
      return;
    }
  }
  case UNKNOWN:
  break;
  }
}

void Stop(unsigned int *p_pErrorCode, int *p_Result) {
  switch (g_eAppMode) {
  case CURRENT: {
    return;
  } break;
  case POSITION: {
    if (*p_Result == MMC_SUCCESS) {
      LogInfo("halt position movement");
      if (VCS_HaltPositionMovement(g_pKeyHandle, g_usNodeId, p_pErrorCode) ==
          0) {
        LogError("VCS_HaltPositionMovement", *p_Result, *p_pErrorCode);
        *p_Result = MMC_FAILED;
      }
    }
  } break;
  case VELOCITY: {
    if (*p_Result == MMC_SUCCESS) {
      LogInfo("halt velocity movement");
    }
    if (VCS_HaltVelocityMovement(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0) {
      *p_Result = MMC_FAILED;
      LogError("VCS_HaltVelocityMovement", *p_Result, *p_pErrorCode);
    }
  } break;
  case UNKNOWN:
  break;
  }
}

int main(int argc, char **argv) {
  int lResult = MMC_FAILED;
  unsigned int ulErrorCode = 0;
  int target = 0;
  pthread_t listener;

  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = signal_handler;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGRTMIN, &sa, NULL) == -1) {
    cerr << "Error: sigaction" << endl;
    return EXIT_FAILURE;
  }

  SetDefaultParameters();
  if ((lResult = ParseArguments(argc, argv)) != MMC_SUCCESS) {
    return lResult;
  }
  PrintSettings();

  if ((lResult = OpenDevice(&ulErrorCode)) != MMC_SUCCESS) {
    LogError("OpenDevice", lResult, ulErrorCode);
    return lResult;
  }
  if ((lResult = Prepare(&ulErrorCode)) != MMC_SUCCESS) {
    LogError("PrepareDemo", lResult, ulErrorCode);
    return lResult;
  }

  Activate(&ulErrorCode, &lResult);


  pthread_create(&listener, NULL, get_cmd_thread, (void *)&target);

  while (true) {
    // TODO: Write Velocity, Position, and Current data to a file
    usleep(50);
  }
  Stop(&ulErrorCode, &lResult);

  if ((lResult = CloseDevice(&ulErrorCode)) != MMC_SUCCESS) {
    LogError("CloseDevice", lResult, ulErrorCode);
    return lResult;
  }
  void *threadResult;
  pthread_join(listener, &threadResult);
  // Check the thread result
  if (threadResult == (void *)EXIT_SUCCESS) {
    cout << "Thread exited successfully\n";
  } else {
    cout << "Thread exited with failure\n";
  }
  return EXIT_SUCCESS;
}
