#pragma once

#include "config.h"
#include "dependencies.h"

#include <stdbool.h>

#define TRANSACTION_PROXY_SDEVICE_VERSION_MAJOR 2
#define TRANSACTION_PROXY_SDEVICE_VERSION_MINOR 0
#define TRANSACTION_PROXY_SDEVICE_VERSION_PATCH 0
#define TRANSACTION_PROXY_SDEVICE_VERSION (                                                                            \
   (SDeviceVersion)                                                                                                    \
   {                                                                                                                   \
      .Major = TRANSACTION_PROXY_SDEVICE_VERSION_MAJOR,                                                                \
      .Minor = TRANSACTION_PROXY_SDEVICE_VERSION_MINOR,                                                                \
      .Patch = TRANSACTION_PROXY_SDEVICE_VERSION_PATCH                                                                 \
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

   size_t Size;
   bool IsPartial;
#if defined(TRANSACTION_PROXY_SDEVICE_USE_ROLLBACK)
   bool AllowsRollback;
#endif
} TransactionProxySDeviceProperty;

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

SDEVICE_INIT_DATA_DECLARATION(TransactionProxy) { };

SDEVICE_CREATE_HANDLE_DECLARATION(TransactionProxy, init, parent, identifier, context);
SDEVICE_DISPOSE_HANDLE_DECLARATION(TransactionProxy, handlePointer);

bool TransactionProxySDeviceTryRead(SDEVICE_HANDLE(TransactionProxy)          *handle,
                                    void                                      *propertyHandle,
                                    const TransactionProxySDeviceProperty     *property,
                                    const SDeviceGetPartialPropertyParameters *parameters);
bool TransactionProxySDeviceTryWrite(SDEVICE_HANDLE(TransactionProxy)          *handle,
                                     void                                      *propertyHandle,
                                     const TransactionProxySDeviceProperty     *property,
                                     const SDeviceSetPartialPropertyParameters *parameters);
