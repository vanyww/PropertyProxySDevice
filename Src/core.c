#include "private.h"

#include "SDeviceCore/errors.h"
#include "SDeviceCore/common.h"
#include "SDeviceCore/heap.h"

#include <memory.h>

SDEVICE_STRING_NAME_DEFINITION(PropertyProxy);

SDEVICE_CREATE_HANDLE_DECLARATION(PropertyProxy, init, parent, identifier, context)
{
   ThisHandle *handle = SDeviceMalloc(sizeof(ThisHandle));
   handle->Header = (SDeviceHandleHeader)
   {
      .Context           = context,
      .OwnerHandle       = parent,
      .SDeviceStringName = SDEVICE_STRING_NAME(PropertyProxy),
      .LatestStatus      = PROPERTY_PROXY_SDEVICE_STATUS_OK,
      .Identifier        = identifier
   };

   return handle;
}

SDEVICE_DISPOSE_HANDLE_DECLARATION(PropertyProxy, handlePointer)
{
   SDeviceAssert(handlePointer != NULL);

   ThisHandle **_handlePointer = handlePointer;
   ThisHandle *handle = *_handlePointer;

   SDeviceAssert(handle != NULL);

   SDeviceFree(handle);
   *_handlePointer = NULL;
}

static bool TryWriteWithoutRollback(SDEVICE_HANDLE(PropertyProxy)             *handle,
                                    void                                      *propertyHandle,
                                    const PropertyProxySDeviceProperty        *property,
                                    const SDeviceSetPartialPropertyParameters *parameters)
{
   SDeviceDebugAssert(handle != NULL);
   SDeviceDebugAssert(property != NULL);
   SDeviceDebugAssert(parameters != NULL);
   SDeviceDebugAssert(propertyHandle != NULL);
   SDeviceDebugAssert(parameters->Data != NULL);
   SDeviceDebugAssert(property->Interface.Set != NULL);
   SDeviceDebugAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset) &&
                      parameters->Size + parameters->Offset <= property->Size);

   SDevicePropertyOperationStatus status;

   if(property->IsPartial)
   {
      status = property->Interface.AsPartial.Set(propertyHandle, parameters);
   }
   else if(parameters->Size == property->Size)
   {
      status = property->Interface.AsCommon.Set(propertyHandle, parameters->Data);
   }
   else
   {
      SDeviceDebugAssert(property->Interface.AsCommon.Get != NULL);

      uint8_t valueBuffer[property->Size];

      /* read current value */
      status = property->Interface.AsCommon.Get(propertyHandle, valueBuffer);
      if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
         goto ErrorExit;

      /* prepare and write new value, using current value */
      memcpy(&valueBuffer[parameters->Offset], parameters->Data, parameters->Size);
      status = property->Interface.AsCommon.Set(propertyHandle, valueBuffer);
   }

   if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
   {
      ErrorExit:
      SDeviceLogStatus(handle, status);
      return false;
   }

   return true;
}

#if defined(PROPERTY_PROXY_SDEVICE_USE_ROLLBACK)
static bool TryWriteWithRollback(SDEVICE_HANDLE(PropertyProxy)             *handle,
                                 void                                      *propertyHandle,
                                 const PropertyProxySDeviceProperty        *property,
                                 const SDeviceSetPartialPropertyParameters *parameters)
{
   SDeviceDebugAssert(handle != NULL);
   SDeviceDebugAssert(property != NULL);
   SDeviceDebugAssert(parameters != NULL);
   SDeviceDebugAssert(propertyHandle != NULL);
   SDeviceDebugAssert(parameters->Data != NULL);
   SDeviceDebugAssert(property->AllowsRollback);
   SDeviceDebugAssert(property->Interface.Set != NULL);
   SDeviceDebugAssert(property->Interface.Get != NULL);
   SDeviceDebugAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset) &&
                      parameters->Size + parameters->Offset <= property->Size);

   bool hasRollbackOccurred = false;
   SDevicePropertyOperationStatus status;
   uint8_t rollbackValueBuffer[parameters->Size];

   if(property->IsPartial)
   {
      /* try read current value part, that will be written (will be used in case rollback is needed) */
      status = property->Interface.AsPartial.Get(propertyHandle,
                                                 &(const SDeviceGetPartialPropertyParameters)
                                                 {
                                                    .Offset = parameters->Offset,
                                                    .Size   = parameters->Size,
                                                    .Data   = rollbackValueBuffer
                                                 });
      if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
         goto ErrorExit;

      /* try write new value part */
      status = property->Interface.AsPartial.Set(propertyHandle, parameters);

      /* in case of processing error during write procedure, try to rollback all the changes */
      if(status == SDEVICE_PROPERTY_OPERATION_STATUS_PROCESSING_ERROR)
      {
         hasRollbackOccurred = true;
         status = property->Interface.AsPartial.Set(propertyHandle,
                                                   &(const SDeviceSetPartialPropertyParameters)
                                                   {
                                                      .Offset = parameters->Offset,
                                                      .Size   = parameters->Size,
                                                      .Data   = rollbackValueBuffer
                                                   });
      }
   }
   else if(parameters->Size == property->Size)
   {
      /* try read current value (will be used in case rollback is needed) */
      status = property->Interface.AsCommon.Get(propertyHandle, rollbackValueBuffer);
      if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
         goto ErrorExit;

      /* try write new value */
      status = property->Interface.AsCommon.Set(propertyHandle, parameters->Data);

      /* in case of processing error during write procedure, try to rollback all the changes */
      if(status == SDEVICE_PROPERTY_OPERATION_STATUS_PROCESSING_ERROR)
      {
         hasRollbackOccurred = true;
         status = property->Interface.AsCommon.Set(propertyHandle, rollbackValueBuffer);
      }
   }
   else
   {
      uint8_t valueBuffer[property->Size];

      /* try read current value to compose new value */
      status = property->Interface.AsCommon.Get(propertyHandle, valueBuffer);
      if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
         goto ErrorExit;

      /* save old value part, that will be written (will be used in case rollback is needed) */
      memcpy(rollbackValueBuffer, &valueBuffer[parameters->Offset], parameters->Size);

      /* compose new value */
      memcpy(&valueBuffer[parameters->Offset], parameters->Data, parameters->Size);

      /* try write new value */
      status = property->Interface.AsCommon.Set(propertyHandle, valueBuffer);

      /* in case of processing error during write procedure, try to rollback all the changes */
      if(status == SDEVICE_PROPERTY_OPERATION_STATUS_PROCESSING_ERROR)
      {
         hasRollbackOccurred = true;
         memcpy(&valueBuffer[parameters->Offset], rollbackValueBuffer, parameters->Size);
         status = property->Interface.AsCommon.Set(propertyHandle, valueBuffer);
      }
   }

   if(hasRollbackOccurred)
   {
      SDeviceLogStatus(handle, status == SDEVICE_PROPERTY_OPERATION_STATUS_OK ?
                               PROPERTY_PROXY_SDEVICE_STATUS_ROLLBACK_SUCCESS :
                               PROPERTY_PROXY_SDEVICE_STATUS_ROLLBACK_FAIL);
      return false;
   }

   if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
   {
      ErrorExit:
      SDeviceLogStatus(handle, status);
      return false;
   }

   return true;
}
#endif

bool PropertyProxySDeviceTryRead(SDEVICE_HANDLE(PropertyProxy)             *handle,
                                 void                                      *propertyHandle,
                                 const PropertyProxySDeviceProperty        *property,
                                 const SDeviceGetPartialPropertyParameters *parameters)
{
   SDeviceAssert(handle != NULL);
   SDeviceAssert(property != NULL);
   SDeviceAssert(parameters != NULL);
   SDeviceAssert(propertyHandle != NULL);
   SDeviceAssert(parameters->Data != NULL);
   SDeviceAssert(property->Interface.Get != NULL);
   SDeviceAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset) &&
                 parameters->Size + parameters->Offset <= property->Size);

   SDevicePropertyOperationStatus status;

   if(property->IsPartial)
   {
      status = property->Interface.AsPartial.Get(propertyHandle, parameters);
   }
   else if(parameters->Size == property->Size)
   {
      status = property->Interface.AsCommon.Get(propertyHandle, parameters->Data);
   }
   else
   {
      /* use proxy buffer to get full value and then return only requested part */
      uint8_t valueBuffer[property->Size];

      status = property->Interface.AsCommon.Get(propertyHandle, valueBuffer);
      if(status == SDEVICE_PROPERTY_OPERATION_STATUS_OK)
      {
         memcpy(parameters->Data, &valueBuffer[parameters->Offset], parameters->Size);
         return true;
      }
   }

   if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
   {
      SDeviceLogStatus(handle, status);
      return false;
   }

   return true;
}

bool PropertyProxySDeviceTryWrite(SDEVICE_HANDLE(PropertyProxy)             *handle,
                                  void                                      *propertyHandle,
                                  const PropertyProxySDeviceProperty        *property,
                                  const SDeviceSetPartialPropertyParameters *parameters)
{
   SDeviceAssert(handle != NULL);
   SDeviceAssert(property != NULL);
   SDeviceAssert(parameters != NULL);
   SDeviceAssert(propertyHandle != NULL);
   SDeviceAssert(parameters->Data != NULL);
   SDeviceAssert(property->Interface.Set != NULL);
   SDeviceAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset) &&
                 parameters->Size + parameters->Offset <= property->Size);

#if defined(PROPERTY_PROXY_SDEVICE_USE_ROLLBACK)
   if(property->Interface.Get != NULL && property->AllowsRollback)
      return TryWriteWithRollback(handle, propertyHandle, property, parameters);
#endif

   SDeviceAssert(property->IsPartial || parameters->Size == property->Size || property->Interface.Get != NULL);

   return TryWriteWithoutRollback(handle, propertyHandle, property, parameters);
}
