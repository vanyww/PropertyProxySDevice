#include "simple.h"

#include "SDeviceCore/assert.h"

#include <stdint.h>
#include <memory.h>

static inline SDevicePropertyStatus SetProperty(
      const PropertyProxySDeviceSimplePropertyInterface *interface,
      void                                              *target,
      const SDeviceSetPartialPropertyParameters         *parameters)
{
   if(interface->Size == parameters->Size)
      return interface->Set(target, parameters->Data);

   SDeviceAssert(interface->Get);

   uint8_t valueBuffer[interface->Size];
   SDevicePropertyStatus getStatus =
         interface->Get(target, valueBuffer);

   if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
      return getStatus;

   memcpy(
         &valueBuffer[parameters->Offset],
         parameters->Data,
         parameters->Size);

   return interface->Set(target, valueBuffer);
}

static inline SDevicePropertyStatus SetCompareProperty(
      const PropertyProxySDeviceSimplePropertyInterface *interface,
      void                                              *target,
      const SDeviceSetPartialPropertyParameters         *parameters,
      bool                                              *didChange)
{
   SDeviceAssert(interface->Get);

   uint8_t valueBuffer[interface->Size];
   SDevicePropertyStatus getStatus =
         interface->Get(target, valueBuffer);

   if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
      return getStatus;

   bool willChange;
   SDevicePropertyStatus setStatus;
   if(interface->Size == parameters->Size)
   {
      willChange =
            !!memcmp(
                  valueBuffer,
                  parameters->Data,
                  parameters->Size);

      setStatus = interface->Set(target, parameters->Data);
   }
   else
   {
      willChange =
            !!memcmp(
                  &valueBuffer[parameters->Offset],
                  parameters->Data,
                  parameters->Size);

      memcpy(
            &valueBuffer[parameters->Offset],
            parameters->Data,
            parameters->Size);

      setStatus = interface->Set(target, valueBuffer);
   }

   if(setStatus != SDEVICE_PROPERTY_STATUS_OK)
      return setStatus;

   *didChange = willChange;

   return SDEVICE_PROPERTY_STATUS_OK;
}

SET_PROPERTY_DECLARATION(Simple, interface, target, parameters, didChange)
{
   const PropertyProxySDeviceSimplePropertyInterface *_interface = interface;

   SDevicePropertyStatus setStatus =
         (didChange) ?
               SetCompareProperty(_interface, target, parameters, didChange) :
               SetProperty(_interface, target, parameters);

   return setStatus;
}

GET_PROPERTY_DECLARATION(Simple, interface, target, parameters)
{
   const PropertyProxySDeviceSimplePropertyInterface *_interface = interface;

   if(_interface->Size == parameters->Size)
      return _interface->Get(target, parameters->Data);

   uint8_t valueBuffer[_interface->Size];
   SDevicePropertyStatus getStatus =
         _interface->Get(target, valueBuffer);

   if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
      return getStatus;

   memcpy(
         parameters->Data,
         &valueBuffer[parameters->Offset],
         parameters->Size);

   return SDEVICE_PROPERTY_STATUS_OK;
}
