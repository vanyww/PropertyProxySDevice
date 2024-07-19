#include "Proxies/simple.h"
#include "Proxies/partial.h"
#include "Proxies/indexer.h"

#include "SDeviceCore/common.h"
#include "SDeviceCore/heap.h"

SDEVICE_IDENTITY_BLOCK_DEFINITION(
      PropertyProxy,
      ((const SDeviceUuid)
      {
         .High = PROPERTY_PROXY_SDEVICE_UUID_HIGH,
         .Low  = PROPERTY_PROXY_SDEVICE_UUID_LOW
      }),
      ((const SDeviceVersion)
      {
         .Major = PROPERTY_PROXY_SDEVICE_VERSION_MAJOR,
         .Minor = PROPERTY_PROXY_SDEVICE_VERSION_MINOR,
         .Patch = PROPERTY_PROXY_SDEVICE_VERSION_PATCH
      }));

static const PropertyProxyInterface ProxyInterfaces[] =
{
   [PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_SIMPLE]  = COMPOSE_PROPERTY_PROXY_INTERFACE(Simple),
   [PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_PARTIAL] = COMPOSE_PROPERTY_PROXY_INTERFACE(Partial),
   [PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_INDEXER] = COMPOSE_PROPERTY_PROXY_INTERFACE(Indexer)
};

static_assert(LENGTHOF(ProxyInterfaces) == PROPERTY_PROXY_SDEVICE_PROPERTY_TYPES_COUNT);

SDEVICE_CREATE_HANDLE_DECLARATION(PropertyProxy, init, owner, identifier, context)
{
   UNUSED_PARAMETER(init);

   ThisHandle *instance = SDeviceAllocateHandle(sizeof(*instance->Init), sizeof(*instance->Runtime));

   instance->Header = (SDeviceHandleHeader)
   {
      .Context       = context,
      .OwnerHandle   = owner,
      .IdentityBlock = &SDEVICE_IDENTITY_BLOCK(PropertyProxy),
      .LatestStatus  = PROPERTY_PROXY_SDEVICE_STATUS_OK,
      .Identifier    = identifier
   };

   return instance;
}

SDEVICE_DISPOSE_HANDLE_DECLARATION(PropertyProxy, handlePointer)
{
   SDeviceAssert(handlePointer);

   ThisHandle **_handlePointer = handlePointer;
   ThisHandle *handle = *_handlePointer;

   SDeviceAssert(IS_VALID_THIS_HANDLE(handle));

   SDeviceFreeHandle(handle);

   *_handlePointer = NULL;
}

SDevicePropertyStatus PropertyProxySDeviceGet(
      SDEVICE_HANDLE(PropertyProxy)             *handle,
      const PropertyProxySDeviceProperty        *property,
      void                                      *target,
      const SDeviceGetPartialPropertyParameters *parameters)
{
   SDeviceAssert(IS_VALID_THIS_HANDLE(handle));

   SDeviceAssert(target);
   SDeviceAssert(property);
   SDeviceAssert(parameters);
   SDeviceAssert(parameters->Data);
   SDeviceAssert(property->Interface);

   SDeviceAssert(PROPERTY_PROXY_SDEVICE_IS_VALID_PROPERTY_TYPE(property->Type));

   return ProxyInterfaces[property->Type].Get(handle, property->Interface, target, parameters);
}

SDevicePropertyStatus PropertyProxySDeviceSet(
      SDEVICE_HANDLE(PropertyProxy)             *handle,
      const PropertyProxySDeviceProperty        *property,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters,
      bool                                      *didChange)
{
   SDeviceAssert(IS_VALID_THIS_HANDLE(handle));

   SDeviceAssert(target);
   SDeviceAssert(property);
   SDeviceAssert(parameters);
   SDeviceAssert(parameters->Data);
   SDeviceAssert(property->Interface);

   SDeviceAssert(PROPERTY_PROXY_SDEVICE_IS_VALID_PROPERTY_TYPE(property->Type));

   return ProxyInterfaces[property->Type].Set(handle, property->Interface, target, parameters, didChange);
}
