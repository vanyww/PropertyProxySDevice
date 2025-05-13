#include "private.h"
#include "Maps/proxy_map.h"

#include "SDeviceCore/assert.h"

#define IS_VALID_PROPERTY_TYPE_IDX(value) (                                                                            \
   (value) < PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_IDXS_COUNT)

SDevicePropertyStatus PropertyProxySDeviceGet(
      const PropertyProxySDeviceProperty        *property,
      void                                      *target,
      const SDeviceGetPartialPropertyParameters *parameters)
{
   SDeviceAssert(property);
   SDeviceAssert(target);
   SDeviceAssert(parameters);
   SDeviceAssert(parameters->Data);
   SDeviceAssert(property->Interface.AsAny.Get);

   SDeviceAssert(
         !WILL_INT_ADD_OVERFLOW(
               parameters->Size, parameters->Offset));

   SDeviceAssert(
         parameters->Size + parameters->Offset <=
         property->Interface.AsAny.Size);

   SDeviceAssert(
         IS_VALID_PROPERTY_TYPE_IDX(
               property->TypeIdx));

   const Proxy *proxy =
         &ProxyMap[property->TypeIdx];

   return proxy->Get(&property->Interface, target, parameters);
}

SDevicePropertyStatus PropertyProxySDeviceSet(
      const PropertyProxySDeviceProperty        *property,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters,
      bool                                      *didChange)
{
   SDeviceAssert(property);
   SDeviceAssert(target);
   SDeviceAssert(parameters);
   SDeviceAssert(parameters->Data);
   SDeviceAssert(property->Interface.AsAny.Set);

   SDeviceAssert(
         !WILL_INT_ADD_OVERFLOW(
               parameters->Size, parameters->Offset));

   SDeviceAssert(
         parameters->Size + parameters->Offset <=
         property->Interface.AsAny.Size);

   SDeviceAssert(
         IS_VALID_PROPERTY_TYPE_IDX(
               property->TypeIdx));

   const Proxy *proxy =
         &ProxyMap[property->TypeIdx];

   return proxy->Set(&property->Interface, target, parameters, didChange);
}
