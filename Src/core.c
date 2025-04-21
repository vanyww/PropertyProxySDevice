#include "Proxies/simple.h"
#include "Proxies/partial.h"

#include "SDeviceCore/errors.h"
#include "SDeviceCore/common.h"
#include "SDeviceCore/heap.h"

static const PropertyProxyInterface ProxyInterfaces[] =
{
   [PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_SIMPLE]  = COMPOSE_PROPERTY_PROXY_INTERFACE(Simple),
   [PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_PARTIAL] = COMPOSE_PROPERTY_PROXY_INTERFACE(Partial)
};

static_assert(LENGTHOF(ProxyInterfaces) == PROPERTY_PROXY_SDEVICE_PROPERTY_TYPES_COUNT);

SDEVICE_CREATE_HANDLE_DECLARATION(PropertyProxy, init, context)
{
   UNUSED_PARAMETER(init);

   ThisHandle *instance = SDeviceAllocateHandle(sizeof(*instance->Init), sizeof(*instance->Runtime));

   instance->Context = context;

   return instance;
}

SDEVICE_DISPOSE_HANDLE_DECLARATION(PropertyProxy, handlePointer)
{
   SDeviceAssert(handlePointer);

   ThisHandle **_handlePointer = handlePointer;
   ThisHandle *handle = *_handlePointer;

   SDeviceAssert(handle);

   SDeviceFreeHandle(handle);

   *_handlePointer = NULL;
}

SDevicePropertyStatus PropertyProxySDeviceGet(
      ThisHandle                                *handle,
      const ThisProperty                        *property,
      void                                      *target,
      const SDeviceGetPartialPropertyParameters *parameters)
{
   SDeviceAssert(handle);

   SDeviceAssert(target);
   SDeviceAssert(property);
   SDeviceAssert(parameters);
   SDeviceAssert(parameters->Data);
   SDeviceAssert(property->Interface);

   SDeviceAssert(PROPERTY_PROXY_SDEVICE_IS_VALID_PROPERTY_TYPE(property->Type));

   return ProxyInterfaces[property->Type].Get(handle, property->Interface, target, parameters);
}

SDevicePropertyStatus PropertyProxySDeviceSet(
      ThisHandle                                *handle,
      const ThisProperty                        *property,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters,
      bool                                      *didChange)
{
   SDeviceAssert(handle);

   SDeviceAssert(target);
   SDeviceAssert(property);
   SDeviceAssert(parameters);
   SDeviceAssert(parameters->Data);
   SDeviceAssert(property->Interface);

   SDeviceAssert(PROPERTY_PROXY_SDEVICE_IS_VALID_PROPERTY_TYPE(property->Type));

   return ProxyInterfaces[property->Type].Set(handle, property->Interface, target, parameters, didChange);
}

size_t PropertyProxySDeviceComputeTotalSize(const ThisProperty *property)
{
   SDeviceAssert(property);
   SDeviceAssert(property->Interface);

   SDeviceAssert(PROPERTY_PROXY_SDEVICE_IS_VALID_PROPERTY_TYPE(property->Type));

   return ProxyInterfaces[property->Type].ComputeTotalSize(property->Interface);
}
