#include "simple.h"

#include "SDeviceCore/common.h"

#include <memory.h>

static inline SDevicePropertyStatus SetSimpleProperty(
      ThisHandle                                *handle,
      const ThisSimplePropertyInterface         *interface,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters)
{
   SDevicePropertyStatus setStatus;

   if(interface->Size == parameters->Size)
   {
      setStatus = interface->Set(target, parameters->Data);

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setStatus));
   }
   else
   {
      char valueBuffer[interface->Size];
      SDevicePropertyStatus getStatus = interface->Get(target, valueBuffer);

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getStatus));

      if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
         return getStatus;

      memcpy(&valueBuffer[parameters->Offset], parameters->Data, parameters->Size);
      setStatus = interface->Set(target, valueBuffer);

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setStatus));
   }

   return setStatus;
}

static inline SDevicePropertyStatus SetCompareSimpleProperty(
      ThisHandle                                *handle,
      const ThisSimplePropertyInterface         *interface,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters,
      bool                                      *didChange)
{
   char valueBuffer[interface->Size];
   SDevicePropertyStatus getStatus = interface->Get(target, valueBuffer);

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getStatus));

   if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
      return getStatus;

   bool willChange;
   SDevicePropertyStatus setStatus;

   if(interface->Size == parameters->Size)
   {
      willChange = !!memcmp(valueBuffer, parameters->Data, parameters->Size);
      setStatus = interface->Set(target, parameters->Data);

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setStatus));
   }
   else
   {
      willChange = !!memcmp(&valueBuffer[parameters->Offset], parameters->Data, parameters->Size);

      memcpy(&valueBuffer[parameters->Offset], parameters->Data, parameters->Size);
      setStatus = interface->Set(target, valueBuffer);

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setStatus));
   }

   if(setStatus != SDEVICE_PROPERTY_STATUS_OK)
      return setStatus;

   *didChange = willChange;

   return SDEVICE_PROPERTY_STATUS_OK;
}

PROPERTY_PROXY_SET_DECLARATION(Simple, handle, interface, target, parameters, didChange)
{
   const ThisSimplePropertyInterface *_interface = interface;

   SDeviceAssert(_interface->Set);
   SDeviceAssert(_interface->Get);

   SDeviceAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset));
   SDeviceAssert(parameters->Size + parameters->Offset <= _interface->Size);

   return (didChange) ?
         SetCompareSimpleProperty(handle, _interface, target, parameters, didChange) :
         SetSimpleProperty(handle, _interface, target, parameters);
}

PROPERTY_PROXY_GET_DECLARATION(Simple, handle, interface, target, parameters)
{
   const ThisSimplePropertyInterface *_interface = interface;

   SDeviceAssert(_interface->Get);

   SDeviceAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset));
   SDeviceAssert(parameters->Size + parameters->Offset <= _interface->Size);

   if(_interface->Size == parameters->Size)
   {
      SDevicePropertyStatus getStatus = _interface->Get(target, parameters->Data);

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getStatus));

      return getStatus;
   }

   char valueBuffer[_interface->Size];
   SDevicePropertyStatus getStatus = _interface->Get(target, valueBuffer);

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getStatus));

   if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
      return getStatus;

   memcpy(parameters->Data, &valueBuffer[parameters->Offset], parameters->Size);

   return SDEVICE_PROPERTY_STATUS_OK;
}

PROPERTY_PROXY_COMPUTE_TOTAL_SIZE_DECLARATION(Simple, interface)
{
   const ThisSimplePropertyInterface *_interface = interface;

   return _interface->Size;
}
