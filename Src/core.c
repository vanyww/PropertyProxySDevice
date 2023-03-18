#include "private.h"

#include "SDeviceCore/errors.h"
#include "SDeviceCore/heap.h"

#include <memory.h>

SDEVICE_CREATE_HANDLE_DECLARATION(TransactionProxy, init, parent, identifier, context)
{
   SDeviceAssert(init != NULL);

   const ThisInitData *_init = init;
   ThisHandle *handle = SDeviceMalloc(sizeof(ThisHandle));

   handle->Init = *_init;
   handle->Header = (SDeviceHandleHeader)
   {
      .Context = context,
      .ParentHandle = parent,
      .Identifier = identifier,
      .LatestStatus = TRANSACTION_PROXY_SDEVICE_STATUS_OK
   };

   return handle;
}

SDEVICE_DISPOSE_HANDLE_DECLARATION(TransactionProxy, handlePointer)
{
   SDeviceAssert(handlePointer != NULL);

   ThisHandle **_handlePointer = handlePointer;
   ThisHandle *handle = *_handlePointer;

   SDeviceAssert(handle != NULL);

   SDeviceFree(handle);
   *_handlePointer = NULL;
}

static bool WriteWithoutRollback(SDEVICE_HANDLE(TransactionProxy) *handle,
                                 const TransactionProxySDeviceProperty *property,
                                 const SDeviceSetPartialPropertyParameters *parameters)
{
   SDeviceDebugAssert(handle != NULL);
   SDeviceDebugAssert(property != NULL);
   SDeviceDebugAssert(parameters != NULL);
   SDeviceDebugAssert(parameters->Data != NULL);
   SDeviceDebugAssert(property->Interface.Set != NULL);
   SDeviceDebugAssert(parameters->Size <= SIZE_MAX - parameters->Offset);
   SDeviceDebugAssert(parameters->Size + parameters->Offset <= property->Size);

   SDevicePropertyOperationStatus status;

   /* partial property */
   if(property->HasPartialInterface)
   {
      status = property->Interface.AsPartial.Set(property->Handle, parameters);
      goto Exit;
   }

   /* common property: exact value size */
   if(parameters->Size == property->Size)
   {
      status = property->Interface.AsCommon.Set(property->Handle, parameters->Data);
      goto Exit;
   }

   /* common property: smaller value size */
   {
      SDeviceDebugAssert(property->Interface.AsCommon.Get != NULL);

      char valueBuffer[property->Size];

      /* read current value */
      status = property->Interface.AsCommon.Get(property->Handle, valueBuffer);
      if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
         goto ErrorExit;

      /* prepare and write new value, using current value */
      memcpy(&valueBuffer[parameters->Offset], parameters->Data, parameters->Size);
      status = property->Interface.AsCommon.Set(property->Handle, valueBuffer);
      goto Exit;
   }

Exit:
   if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
   {
ErrorExit:
      SDeviceLogStatus(handle, status);
      return false;
   }

   return true;
}

static bool WriteWithRollback(SDEVICE_HANDLE(TransactionProxy) *handle,
                              const TransactionProxySDeviceProperty *property,
                              const SDeviceSetPartialPropertyParameters *parameters)
{
   SDeviceDebugAssert(handle != NULL);
   SDeviceDebugAssert(property != NULL);
   SDeviceDebugAssert(parameters != NULL);
   SDeviceDebugAssert(parameters->Data != NULL);
   SDeviceDebugAssert(property->AllowsRollback);
   SDeviceDebugAssert(property->Interface.Set != NULL);
   SDeviceDebugAssert(property->Interface.Get != NULL);
   SDeviceDebugAssert(parameters->Size <= SIZE_MAX - parameters->Offset);
   SDeviceDebugAssert(parameters->Size + parameters->Offset <= property->Size);

   bool rollbackAttemptFlag = false;
   char rollbackValueBuffer[parameters->Size];
   SDevicePropertyOperationStatus status;

   /* partial property */
   if(property->HasPartialInterface)
   {
      /* try read current value part, that will be written (will be used in case rollback is needed) */
      status = property->Interface.AsPartial.Get(property->Handle,
                                                 &(const SDeviceGetPartialPropertyParameters)
                                                 {
                                                    .Offset = parameters->Offset,
                                                    .Size = parameters->Size,
                                                    .Data = rollbackValueBuffer
                                                 });
      if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
         goto ErrorExit;

      /* try write new value part */
      status = property->Interface.AsPartial.Set(property->Handle, parameters);

      /* in case of processing error during write procedure, try to rollback all the changes */
      if(status == SDEVICE_PROPERTY_OPERATION_STATUS_PROCESSING_ERROR)
      {
         rollbackAttemptFlag = true;
         status = property->Interface.AsPartial.Set(property->Handle,
                                                   &(const SDeviceSetPartialPropertyParameters)
                                                   {
                                                      .Offset = parameters->Offset,
                                                      .Size = parameters->Size,
                                                      .Data = rollbackValueBuffer
                                                   });
      }
      goto Exit;
   }

   /* common property: exact value size */
   if(parameters->Size == property->Size)
   {
      /* try read current value (will be used in case rollback is needed) */
      status = property->Interface.AsCommon.Get(property->Handle, rollbackValueBuffer);
      if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
         goto ErrorExit;

      /* try write new value */
      status = property->Interface.AsCommon.Set(property->Handle, parameters->Data);

      /* in case of processing error during write procedure, try to rollback all the changes */
      if(status == SDEVICE_PROPERTY_OPERATION_STATUS_PROCESSING_ERROR)
      {
         rollbackAttemptFlag = true;
         status = property->Interface.AsCommon.Set(property->Handle, rollbackValueBuffer);
      }
      goto Exit;
   }

   /* common property: smaller value size */
   {
      char valueBuffer[property->Size];

      /* try read current value to compose new value */
      status = property->Interface.AsCommon.Get(property->Handle, valueBuffer);
      if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
         goto ErrorExit;

      /* save old value part, that will be written (will be used in case rollback is needed) */
      memcpy(rollbackValueBuffer, &valueBuffer[parameters->Offset], parameters->Size);

      /* compose new value */
      memcpy(&valueBuffer[parameters->Offset], parameters->Data, parameters->Size);

      /* try write new value */
      status = property->Interface.AsCommon.Set(property->Handle, valueBuffer);

      /* in case of processing error during write procedure, try to rollback all the changes */
      if(status == SDEVICE_PROPERTY_OPERATION_STATUS_PROCESSING_ERROR)
      {
         rollbackAttemptFlag = true;
         memcpy(&valueBuffer[parameters->Offset], rollbackValueBuffer, parameters->Size);
         status = property->Interface.AsCommon.Set(property->Handle, valueBuffer);
      }
      goto Exit;
   }

Exit:
   if(rollbackAttemptFlag)
   {

      SDeviceLogStatus(handle, (status == SDEVICE_PROPERTY_OPERATION_STATUS_OK) ?
                               TRANSACTION_PROXY_SDEVICE_STATUS_ROLLBACK_SUCCESS :
                               TRANSACTION_PROXY_SDEVICE_STATUS_ROLLBACK_FAIL);
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

bool TransactionProxySDeviceTryRead(SDEVICE_HANDLE(TransactionProxy) *handle,
                                    const TransactionProxySDeviceProperty *property,
                                    const SDeviceGetPartialPropertyParameters *parameters)
{
   SDeviceAssert(handle != NULL);
   SDeviceAssert(property != NULL);
   SDeviceAssert(parameters != NULL);
   SDeviceAssert(parameters->Data != NULL);
   SDeviceAssert(property->Interface.Get != NULL);
   SDeviceAssert(parameters->Size <= SIZE_MAX - parameters->Offset);
   SDeviceAssert(parameters->Size + parameters->Offset <= property->Size);

   SDevicePropertyOperationStatus status;

   /* partial property */
   if(property->HasPartialInterface)
   {
      status = property->Interface.AsPartial.Get(property->Handle, parameters);
      goto Exit;
   }

   /* common property: exact value size */
   if(parameters->Size == property->Size)
   {
      status = property->Interface.AsCommon.Get(property->Handle, parameters->Data);
      goto Exit;
   }

   /* common property: smaller value size */
   {
      /* use proxy buffer to get full value and then copy only requested part */
      char valueBuffer[property->Size];

      status = property->Interface.AsCommon.Get(property->Handle, valueBuffer);
      if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
         goto ErrorExit;

      memcpy(parameters->Data, &valueBuffer[parameters->Offset], parameters->Size);
   }

Exit:
   if(status != SDEVICE_PROPERTY_OPERATION_STATUS_OK)
   {
ErrorExit:
      SDeviceLogStatus(handle, status);
      return false;
   }

   return true;
}

bool TransactionProxySDeviceTryWrite(SDEVICE_HANDLE(TransactionProxy) *handle,
                                     const TransactionProxySDeviceProperty *property,
                                     const SDeviceSetPartialPropertyParameters *parameters)
{
   SDeviceAssert(handle != NULL);
   SDeviceAssert(property != NULL);
   SDeviceAssert(parameters != NULL);
   SDeviceAssert(parameters->Data != NULL);
   SDeviceAssert(property->Interface.Set != NULL);
   SDeviceAssert(parameters->Size <= SIZE_MAX - parameters->Offset);
   SDeviceAssert(parameters->Size + parameters->Offset <= property->Size);

   if(property->Interface.Get != NULL && property->AllowsRollback)
      return WriteWithRollback(handle, property, parameters);

   SDeviceAssert(property->HasPartialInterface      ||
                 parameters->Size == property->Size ||
                 property->Interface.Get != NULL);

   return WriteWithoutRollback(handle, property, parameters);
}
