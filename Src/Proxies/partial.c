#include "partial.h"

#include "SDeviceCore/common.h"

#include <memory.h>

static inline SDevicePropertyStatus SetPartialProperty(
      ThisHandle                                *handle,
      const ThisPartialPropertyInterface        *interface,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters)
{
   SDevicePropertyStatus setStatus = interface->Set(target, parameters);

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setStatus));

   return setStatus;
}

static inline SDevicePropertyStatus SetComparePartialProperty(
      ThisHandle                                *handle,
      const ThisPartialPropertyInterface        *interface,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters,
      bool                                      *didChange)
{
   uint8_t valueBuffer[parameters->Size];
   SDevicePropertyStatus getStatus = interface->Get(
         target,
         &(const SDeviceGetPartialPropertyParameters)
         {
            .Data   = valueBuffer,
            .Offset = parameters->Offset,
            .Size   = parameters->Size
         });

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getStatus));

   if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
      return getStatus;

   bool willChange = !!memcmp(valueBuffer, parameters->Data, parameters->Size);
   SDevicePropertyStatus setStatus = interface->Set(target, parameters);

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setStatus));

   if(setStatus != SDEVICE_PROPERTY_STATUS_OK)
      return setStatus;

   *didChange = willChange;

   return SDEVICE_PROPERTY_STATUS_OK;
}

PROPERTY_PROXY_SET_DECLARATION(Partial, handle, interface, target, parameters, didChange)
{
   const ThisPartialPropertyInterface *_interface = interface;

   SDeviceAssert(_interface->Set);
   SDeviceAssert(_interface->Get);

   SDeviceAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset));
   SDeviceAssert(parameters->Size + parameters->Offset <= _interface->Size);

   return
         (didChange) ?
               SetComparePartialProperty(handle, _interface, target, parameters, didChange) :
               SetPartialProperty(handle, _interface, target, parameters);
}

PROPERTY_PROXY_GET_DECLARATION(Partial, handle, interface, target, parameters)
{
   const ThisPartialPropertyInterface *_interface = interface;

   SDeviceAssert(_interface->Get);

   SDeviceAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset));
   SDeviceAssert(parameters->Size + parameters->Offset <= _interface->Size);

   SDevicePropertyStatus getStatus = _interface->Get(target, parameters);

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getStatus));

   return getStatus;
}
