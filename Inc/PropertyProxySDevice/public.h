#pragma once

#include "config.h"
#include "dependencies.h"

#define PROPERTY_PROXY_SDEVICE_VERSION_MAJOR 3
#define PROPERTY_PROXY_SDEVICE_VERSION_MINOR 0
#define PROPERTY_PROXY_SDEVICE_VERSION_PATCH 0

typedef enum
{
   PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_IDX_SIMPLE,
   PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_IDX_PARTIAL,

   PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_IDXS_COUNT
} PropertyProxySDevicePropertyTypeIdx;

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
   union
   {
      PropertyProxySDeviceSimplePropertyInterface  AsSimple;
      PropertyProxySDevicePartialPropertyInterface AsPartial;

      struct
      {
         void (* Get)(void);
         void (* Set)(void);

         size_t Size;
      } AsAny;
   } Interface;

   PropertyProxySDevicePropertyTypeIdx TypeIdx;
} PropertyProxySDeviceProperty;

SDevicePropertyStatus PropertyProxySDeviceGet(
      const PropertyProxySDeviceProperty        *property,
      void                                      *target,
      const SDeviceGetPartialPropertyParameters *parameters);

SDevicePropertyStatus PropertyProxySDeviceSet(
      const PropertyProxySDeviceProperty        *property,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters,
      bool                                      *didChange);
