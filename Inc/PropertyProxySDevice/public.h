#pragma once

#include "config.h"
#include "dependencies.h"

#define PROPERTY_PROXY_SDEVICE_VERSION_MAJOR 2
#define PROPERTY_PROXY_SDEVICE_VERSION_MINOR 0
#define PROPERTY_PROXY_SDEVICE_VERSION_PATCH 1

#define PROPERTY_PROXY_SDEVICE_IS_VALID_PROPERTY_TYPE(value) (                                                         \
   (value) < PROPERTY_PROXY_SDEVICE_PROPERTY_TYPES_COUNT)

SDEVICE_HANDLE_FORWARD_DECLARATION(PropertyProxy);
SDEVICE_INIT_DATA_FORWARD_DECLARATION(PropertyProxy);

typedef enum
{
   PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_SIMPLE,
   PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_PARTIAL,

   PROPERTY_PROXY_SDEVICE_PROPERTY_TYPES_COUNT
} PropertyProxySDevicePropertyType;

typedef struct
{
   SDEVICE_GET_SIMPLE_PROPERTY_POINTER(Get);
   SDEVICE_SET_SIMPLE_PROPERTY_POINTER(Set);
   size_t Size;
} PropertyProxySDeviceSimplePropertyInterface;

typedef struct
{
   SDEVICE_GET_PARTIAL_PROPERTY_POINTER(Get);
   SDEVICE_SET_PARTIAL_PROPERTY_POINTER(Set);
   size_t Size;
} PropertyProxySDevicePartialPropertyInterface;

typedef struct
{
   PropertyProxySDevicePropertyType Type;
   const void                      *Interface;
} PropertyProxySDeviceProperty;

SDEVICE_INIT_DATA_DECLARATION(PropertyProxy) { };

SDEVICE_CREATE_HANDLE_DECLARATION(PropertyProxy, init, context);
SDEVICE_DISPOSE_HANDLE_DECLARATION(PropertyProxy, handlePointer);

SDevicePropertyStatus PropertyProxySDeviceGet(
      SDEVICE_HANDLE(PropertyProxy)             *handle,
      const PropertyProxySDeviceProperty        *property,
      void                                      *target,
      const SDeviceGetPartialPropertyParameters *parameters);

SDevicePropertyStatus PropertyProxySDeviceSet(
      SDEVICE_HANDLE(PropertyProxy)             *handle,
      const PropertyProxySDeviceProperty        *property,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters,
      bool                                      *didChange);

size_t PropertyProxySDeviceComputeTotalSize(const PropertyProxySDeviceProperty *property);
