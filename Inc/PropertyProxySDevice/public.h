#pragma once

#include "dependencies.h"
#include "config.h"
#include "log.h"

/* e62b0f56-4db6-11ee-bd0b-2bd074ce77da */
#define PROPERTY_PROXY_SDEVICE_UUID_HIGH 0xe62b0f564db611ee
#define PROPERTY_PROXY_SDEVICE_UUID_LOW  0xbd0b2bd074ce77da

#define PROPERTY_PROXY_SDEVICE_VERSION_MAJOR 2
#define PROPERTY_PROXY_SDEVICE_VERSION_MINOR 0
#define PROPERTY_PROXY_SDEVICE_VERSION_PATCH 0

SDEVICE_HANDLE_FORWARD_DECLARATION(PropertyProxy);
SDEVICE_INIT_DATA_FORWARD_DECLARATION(PropertyProxy);

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
   bool   IsPartial;
#if PROPERTY_PROXY_SDEVICE_USE_ROLLBACK
   bool   AllowsRollback;
#endif
} PropertyProxySDeviceProperty;

SDEVICE_INIT_DATA_DECLARATION(PropertyProxy) { };

SDEVICE_IDENTITY_BLOCK_DECLARATION(PropertyProxy);

SDEVICE_CREATE_HANDLE_DECLARATION(PropertyProxy, init, owner, identifier, context);
SDEVICE_DISPOSE_HANDLE_DECLARATION(PropertyProxy, handlePointer);

SDevicePropertyStatus PropertyProxySDeviceGet(SDEVICE_HANDLE(PropertyProxy)             *handle,
                                              const PropertyProxySDeviceProperty        *property,
                                              void                                      *propertyHandle,
                                              const SDeviceGetPartialPropertyParameters *parameters);
SDevicePropertyStatus PropertyProxySDeviceSet(SDEVICE_HANDLE(PropertyProxy)             *handle,
                                              const PropertyProxySDeviceProperty        *property,
                                              void                                      *propertyHandle,
                                              const SDeviceSetPartialPropertyParameters *parameters,
                                              bool                                      *wasChanged);
