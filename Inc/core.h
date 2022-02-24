#pragma once

#include "SDeviceCore/interface.h"

typedef struct
{
   void *Handle;
   const void *DefaultValue;
   __SDEVICE_SET_PARAMETER_POINTER(SetFunction);
   __SDEVICE_GET_PARAMETER_POINTER(GetFunction);
   intptr_t StorageAddress;
   size_t Size;
   bool HasPartialInterface;
} ParameterManagerParameter;

typedef enum
{
   PARAMETER_MANAGER_STATUS_OK,
   PARAMETER_MANAGER_STATUS_HANDLED_ERROR,
   PARAMETER_MANAGER_STATUS_UNHANDLED_ERROR
} ParameterManagerStatus;

typedef struct
{
   size_t Id;
   size_t Size;
   size_t Offset;
} ParameterManagerArguments;

typedef struct
{
   const ParameterManagerParameter *Parameters;
   size_t Length;
} ParameterManagerParameterList;

/* Satty's interface start */

__SDEVICE_HANDLE_FORWARD_DECLARATION(ParameterManager);

typedef struct
{
   bool (* TryWriteToStorage)(__SDEVICE_HANDLE(ParameterManager) *, intptr_t, const void *, size_t);
   bool (* TryReadFromStorage)(__SDEVICE_HANDLE(ParameterManager) *, intptr_t, void *, size_t);
   const ParameterManagerParameterList *ParameterList;
} __SDEVICE_INIT_DATA(ParameterManager);

typedef struct { } __SDEVICE_RUNTIME_DATA(ParameterManager);

__SDEVICE_HANDLE_DEFINITION(ParameterManager);

__SDEVICE_INITIALIZE_HANDLE_DECLARATION(ParameterManager,);

typedef enum
{
   PARAMETER_MANAGER_RUNTIME_ERROR_READ_FAIL            = 0x01,
   PARAMETER_MANAGER_RUNTIME_ERROR_WRITE_FAIL           = 0x02,
   PARAMETER_MANAGER_RUNTIME_ERROR_SET_FAIL             = 0x03,
   PARAMETER_MANAGER_RUNTIME_ERROR_GET_FAIL             = 0x04,
   PARAMETER_MANAGER_RUNTIME_ERROR_WRONG_OPERATION_SIZE = 0x05,
   PARAMETER_MANAGER_RUNTIME_ERROR_VALUE_REVERSION_FAIL = 0x06
} ParameterManagerRuntimeError;

/* Satty's interface end */

bool ParameterManagerTryInitializeParameter(__SDEVICE_HANDLE(ParameterManager) *, size_t);
ParameterManagerStatus ParameterManagerSaveSetting(__SDEVICE_HANDLE(ParameterManager) *, size_t);
ParameterManagerStatus ParameterManagerRead(__SDEVICE_HANDLE(ParameterManager) *, ParameterManagerArguments *, void *);
ParameterManagerStatus ParameterManagerWrite(__SDEVICE_HANDLE(ParameterManager) *,
                                             ParameterManagerArguments *,
                                             const void *);
