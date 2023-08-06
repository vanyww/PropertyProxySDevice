#pragma once

#include "config.h"
#include "dependencies.h"

#include <stdbool.h>

#define PROPERTY_PROXY_SDEVICE_VERSION_MAJOR 2
#define PROPERTY_PROXY_SDEVICE_VERSION_MINOR 0
#define PROPERTY_PROXY_SDEVICE_VERSION_PATCH 0
#define PROPERTY_PROXY_SDEVICE_VERSION (                                                                               \
   (SDeviceVersion)                                                                                                    \
   {                                                                                                                   \
      .Major = PROPERTY_PROXY_SDEVICE_VERSION_MAJOR,                                                                   \
      .Minor = PROPERTY_PROXY_SDEVICE_VERSION_MINOR,                                                                   \
      .Patch = PROPERTY_PROXY_SDEVICE_VERSION_PATCH                                                                    \
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
#if defined(PROPERTY_PROXY_SDEVICE_USE_ROLLBACK)
   bool AllowsRollback;
#endif
} PropertyProxySDeviceProperty;

SDEVICE_HANDLE_FORWARD_DECLARATION(PropertyProxy);
SDEVICE_INIT_DATA_FORWARD_DECLARATION(PropertyProxy);

typedef enum
{
   PROPERTY_PROXY_SDEVICE_STATUS_OK              = SDEVICE_PROPERTY_OPERATION_STATUS_OK,
   PROPERTY_PROXY_SDEVICE_STATUS_VALIDATION_FAIL = SDEVICE_PROPERTY_OPERATION_STATUS_VALIDATION_ERROR,
   PROPERTY_PROXY_SDEVICE_STATUS_OPERATION_FAIL  = SDEVICE_PROPERTY_OPERATION_STATUS_PROCESSING_ERROR,
   PROPERTY_PROXY_SDEVICE_STATUS_ROLLBACK_SUCCESS,
   PROPERTY_PROXY_SDEVICE_STATUS_ROLLBACK_FAIL
} PropertyProxySDeviceStatus;

SDEVICE_INIT_DATA_DECLARATION(PropertyProxy) { };

SDEVICE_CREATE_HANDLE_DECLARATION(PropertyProxy, init, parent, identifier, context);
SDEVICE_DISPOSE_HANDLE_DECLARATION(PropertyProxy, handlePointer);

bool PropertyProxySDeviceTryRead(SDEVICE_HANDLE(PropertyProxy)             *handle,
                                 void                                      *propertyHandle,
                                 const PropertyProxySDeviceProperty        *property,
                                 const SDeviceGetPartialPropertyParameters *parameters);
bool PropertyProxySDeviceTryWrite(SDEVICE_HANDLE(PropertyProxy)             *handle,
                                  void                                      *propertyHandle,
                                  const PropertyProxySDeviceProperty        *property,
                                  const SDeviceSetPartialPropertyParameters *parameters);