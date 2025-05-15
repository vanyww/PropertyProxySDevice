#include "partial.h"

#include "SDeviceCore/assert.h"

#include <stdint.h>
#include <memory.h>

static inline SDevicePropertyStatus SetProperty(
      const PropertyProxySDevicePartialPropertyInterface *interface,
      void                                               *target,
      const SDeviceSetPartialPropertyParameters          *parameters)
{
   return interface->Set(target, parameters);
}

static inline SDevicePropertyStatus SetCompareProperty(
      const PropertyProxySDevicePartialPropertyInterface *interface,
      void                                               *target,
      const SDeviceSetPartialPropertyParameters          *parameters,
      bool                                               *didChange)
{
   SDeviceAssert(interface->Get);

   uint8_t valueBuffer[parameters->Size];
   SDevicePropertyStatus getStatus =
         interface->Get(
               target,
               &(const SDeviceGetPartialPropertyParameters)
               {
                  .Data   = valueBuffer,
                  .Offset = parameters->Offset,
                  .Size   = parameters->Size
               });

   if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
      return getStatus;

   bool willChange =
         !!memcmp(
               valueBuffer,
               parameters->Data,
               parameters->Size);

   SDevicePropertyStatus setStatus =
         interface->Set(target, parameters);

   if(setStatus != SDEVICE_PROPERTY_STATUS_OK)
      return setStatus;

   *didChange = willChange;

   return SDEVICE_PROPERTY_STATUS_OK;
}

SET_PROPERTY_DECLARATION(Partial, interface, target, parameters, didChange)
{
   const PropertyProxySDevicePartialPropertyInterface *_interface = interface;

   SDevicePropertyStatus setStatus =
         (didChange) ?
               SetCompareProperty(_interface, target, parameters, didChange) :
               SetProperty(_interface, target, parameters);

   return setStatus;
}

GET_PROPERTY_DECLARATION(Partial, interface, target, parameters)
{
   const PropertyProxySDevicePartialPropertyInterface *_interface = interface;

   return _interface->Get(target, parameters);
}
