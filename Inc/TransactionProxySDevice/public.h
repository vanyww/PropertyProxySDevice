#pragma once

#include "SDeviceCore/core.h"

#include <stdbool.h>

#if (SDEVICE_CORE_VERSION_MAJOR != 7) || (SDEVICE_CORE_VERSION_MINOR < 0)
#error SDevice core version is incorrect.
#endif

#define TRANSACTION_PROXY_SDEVICE_VERSION_MAJOR 1
#define TRANSACTION_PROXY_SDEVICE_VERSION_MINOR 0
#define TRANSACTION_PROXY_SDEVICE_VERSION_PATCH 0
#define TRANSACTION_PROXY_SDEVICE_CORE_VERSION ((SDeviceVersion)                                                       \
{                                                                                                                      \
   .Major = TRANSACTION_PROXY_SDEVICE_VERSION_MAJOR,                                                                   \
   .Minor = TRANSACTION_PROXY_SDEVICE_VERSION_MINOR,                                                                   \
   .Patch = TRANSACTION_PROXY_SDEVICE_VERSION_PATCH                                                                    \
})

typedef struct
{
   union
   {
      struct
      {
         SDEVICE_SET_PROPERTY_POINTER(Set);
         SDEVICE_GET_PROPERTY_POINTER(Get);
      } AsCommon;
      struct
      {
         SDEVICE_SET_PARTIAL_PROPERTY_POINTER(Set);
         SDEVICE_GET_PARTIAL_PROPERTY_POINTER(Get);
      } AsPartial;
      struct
      {
         void *Set;
         void *Get;
      };
   } Interface;
   void *Handle;
   size_t Size;
   bool AllowsRollback;
   bool HasPartialInterface;
} TransactionProxySDeviceProperty;

typedef enum
{
   PARAMETER_TRANSACTION_PROXY_STATUS_OK,
   PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR,
   PARAMETER_TRANSACTION_PROXY_STATUS_UNHANDLED_ERROR
} ParameterTransactionProxyStatus;

SDEVICE_HANDLE_FORWARD_DECLARATION(TransactionProxy);
SDEVICE_INIT_DATA_FORWARD_DECLARATION(TransactionProxy);

typedef enum
{
   TRANSACTION_PROXY_SDEVICE_STATUS_OK              = SDEVICE_PROPERTY_OPERATION_STATUS_OK,
   TRANSACTION_PROXY_SDEVICE_STATUS_VALIDATION_FAIL = SDEVICE_PROPERTY_OPERATION_STATUS_VALIDATION_ERROR,
   TRANSACTION_PROXY_SDEVICE_STATUS_OPERATION_FAIL  = SDEVICE_PROPERTY_OPERATION_STATUS_PROCESSING_ERROR,

   TRANSACTION_PROXY_SDEVICE_STATUS_ROLLBACK_SUCCESS,
   TRANSACTION_PROXY_SDEVICE_STATUS_ROLLBACK_FAIL
} TransactionProxySDeviceStatus;

SDEVICE_INIT_DATA_DECLARATION(TransactionProxy)
{

};

SDEVICE_CREATE_HANDLE_DECLARATION(TransactionProxy, init, parent, identifier, context);
SDEVICE_DISPOSE_HANDLE_DECLARATION(TransactionProxy, handlePointer);

bool TransactionProxySDeviceTryRead(SDEVICE_HANDLE(TransactionProxy) *handle,
                                    const TransactionProxySDeviceProperty *property,
                                    const SDeviceGetPartialPropertyParameters *parameters);
bool TransactionProxySDeviceTryWrite(SDEVICE_HANDLE(TransactionProxy) *handle,
                                     const TransactionProxySDeviceProperty *property,
                                     const SDeviceSetPartialPropertyParameters *parameters);
