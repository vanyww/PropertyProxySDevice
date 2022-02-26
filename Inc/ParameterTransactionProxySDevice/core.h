#pragma once

#include "SDeviceCore/interface.h"

typedef struct
{
   void *Handle;
   __SDEVICE_SET_PARAMETER_POINTER(SetFunction);
   __SDEVICE_GET_PARAMETER_POINTER(GetFunction);
   size_t Size;
   bool HasPartialInterface;
} ParameterTransactionProxyParameter;

typedef enum
{
   PARAMETER_TRANSACTION_PROXY_STATUS_OK,
   PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR,
   PARAMETER_TRANSACTION_PROXY_STATUS_UNHANDLED_ERROR
} ParameterTransactionProxyStatus;

typedef struct
{
   const ParameterTransactionProxyParameter *Parameter;
   size_t Size;
   size_t Offset;
} ParameterTransactionProxyArguments;

/* Satty's interface start */

__SDEVICE_HANDLE_FORWARD_DECLARATION(ParameterTransactionProxy);

typedef struct { } __SDEVICE_INIT_DATA(ParameterTransactionProxy);

typedef struct { } __SDEVICE_RUNTIME_DATA(ParameterTransactionProxy);

__SDEVICE_HANDLE_DEFINITION(ParameterTransactionProxy);

__SDEVICE_INITIALIZE_HANDLE_DECLARATION(ParameterTransactionProxy,);

typedef enum
{
   PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_SET_FAIL             = 0x01,
   PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_GET_FAIL             = 0x02,
   PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_WRONG_OPERATION_TYPE = 0x03,
   PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_WRONG_OPERATION_SIZE = 0x04,
   PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_ROLLBACK_FAIL        = 0x05
} ParameterTransactionProxyRuntimeError;

/* Satty's interface end */

ParameterTransactionProxyStatus ParameterTransactionProxyRead(__SDEVICE_HANDLE(ParameterTransactionProxy) *,
                                                              ParameterTransactionProxyArguments,
                                                              void *);
ParameterTransactionProxyStatus ParameterTransactionProxyWrite(__SDEVICE_HANDLE(ParameterTransactionProxy) *,
                                                               ParameterTransactionProxyArguments,
                                                               const void *);
