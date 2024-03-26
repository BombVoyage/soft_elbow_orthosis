#ifndef DRIVER_H
#define DRIVER_H

#include <sstream>
#ifndef MMC_SUCCESS
#define MMC_SUCCESS 0
#endif

#ifndef MMC_FAILED
#define MMC_FAILED 1
#endif

#ifndef MMC_MAX_LOG_MSG_SIZE
#define MMC_MAX_LOG_MSG_SIZE 512
#endif

typedef void *HANDLE;
typedef int BOOL;

enum EAppMode {
  CURRENT,
  POSITION,
  VELOCITY,
  UNKNOWN,
};

using namespace std;

void LogError(string functionName, int p_lResult, unsigned int p_ulErrorCode);
void LogInfo(string message);
void PrintUsage();
void PrintHeader();
void PrintSettings();
int OpenDevice(unsigned int *p_pErrorCode);
int CloseDevice(unsigned int *p_pErrorCode);
void SetDefaultParameters();
int ParseArguments(int argc, char **argv);
int Prepare(unsigned int *p_pErrorCode);
void Activate(unsigned int *p_pErrorCode, int* result);
void Stop(unsigned int *p_pErrorCode, int* p_Result);

#endif
