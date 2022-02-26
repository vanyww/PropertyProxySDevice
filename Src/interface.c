#include "../Inc/ParameterTransactionProxySDevice/core.h"

#include <alloca.h>
#include <memory.h>

static SDeviceOperationStatus SetFullParameterValue(const ParameterManagerParameter *parameter, const void *value)
{
   if(parameter->HasPartialInterface == true)
   {
      return parameter->SetFunction(parameter->Handle,
                                    &(SDevicePartialParameterArguments)
                                    {
                                       .Data.Set = value,
                                       .Offset = 0,
                                       .Size = parameter->Size
                                    });
   }

   return parameter->SetFunction(parameter->Handle, value);
}

static SDeviceOperationStatus GetFullParameterValue(const ParameterManagerParameter *parameter, void *value)
{
   if(parameter->HasPartialInterface == true)
   {
      return parameter->GetFunction(parameter->Handle,
                                    &(SDevicePartialParameterArguments)
                                    {
                                       .Data.Get = value,
                                       .Offset = 0,
                                       .Size = parameter->Size
                                    });
   }

   return parameter->GetFunction(parameter->Handle, value);
}

static bool TryInitializeParameterFromStorage(__SDEVICE_HANDLE(ParameterManager) *handle,
                                              const ParameterManagerParameter *parameter)
{
   void *buffer = alloca(parameter->Size);
   if(handle->Init.TryReadFromStorage(handle, parameter->StorageAddress, buffer, parameter->Size) != true)
   {
      SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_READ_FAIL);
      return false;
   }

   if(SetFullParameterValue(parameter, buffer) != SDEVICE_OPERATION_STATUS_OK)
   {
      SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_SET_FAIL);
      return false;
   }

   return true;
}

bool ParameterManagerTryInitializeParameter(__SDEVICE_HANDLE(ParameterManager) *handle, size_t parameterId)
{
   SDeviceAssert(handle != NULL);
   SDeviceAssert(handle->IsInitialized == true);
   SDeviceAssert(parameterId < handle->Init.ParameterList->Length);

   const ParameterManagerParameter *parameter = &handle->Init.ParameterList->Parameters[parameterId];

   /* initially, before first parameters save, it'll show that any entry returned error */
   if(TryInitializeParameterFromStorage(handle, parameter) != true)
   {
      if(SetFullParameterValue(parameter, parameter->DefaultValue) != SDEVICE_OPERATION_STATUS_OK)
      {
         SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_SET_FAIL);
         return false;
      }
   }

   return true;
}

ParameterManagerStatus ParameterManagerRead(__SDEVICE_HANDLE(ParameterManager) *handle,
                                            ParameterManagerArguments *arguments,
                                            void *data)
{
   SDeviceAssert(data != NULL);
   SDeviceAssert(handle != NULL);
   SDeviceAssert(arguments != NULL);
   SDeviceAssert(handle->IsInitialized == true);
   SDeviceAssert(arguments->Id < handle->Init.ParameterList->Length);

   const ParameterManagerParameter *parameter = &handle->Init.ParameterList->Parameters[arguments->Id];
   if(SIZE_MAX - arguments->Size < arguments->Offset || parameter->Size > arguments->Offset + arguments->Size)
   {
      SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_WRONG_OPERATION_SIZE);
      return PARAMETER_MANAGER_STATUS_HANDLED_ERROR;
   }

   SDeviceOperationStatus status;
   if(parameter->HasPartialInterface == true)
   {
      status = parameter->GetFunction(parameter->Handle,
                                      &(SDevicePartialParameterArguments)
                                      {
                                         .Data.Get = data,
                                         .Offset = arguments->Offset,
                                         .Size = arguments->Size
                                      });
   }
   else
   {
      void *valueBuffer = alloca(parameter->Size);
      status = parameter->GetFunction(parameter->Handle, valueBuffer);
      memcpy(data, valueBuffer + arguments->Offset, arguments->Size);
   }

   if(status != SDEVICE_OPERATION_STATUS_OK)
   {
      SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_GET_FAIL);
      return PARAMETER_MANAGER_STATUS_HANDLED_ERROR;
   }

   return PARAMETER_MANAGER_STATUS_OK;
}

ParameterManagerStatus ParameterManagerWrite(__SDEVICE_HANDLE(ParameterManager) *handle,
                                             ParameterManagerArguments *arguments,
                                             const void *data)
{
   SDeviceAssert(data != NULL);
   SDeviceAssert(handle != NULL);
   SDeviceAssert(arguments != NULL);
   SDeviceAssert(handle->IsInitialized == true);
   SDeviceAssert(arguments->Id < handle->Init.ParameterList->Length);

   const ParameterManagerParameter *parameter = &handle->Init.ParameterList->Parameters[arguments->Id];
   if(SIZE_MAX - arguments->Size < arguments->Offset || parameter->Size > arguments->Offset + arguments->Size)
   {
      SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_WRONG_OPERATION_SIZE);
      return PARAMETER_MANAGER_STATUS_HANDLED_ERROR;
   }

   bool writeRollbackFlag = false;
   SDeviceOperationStatus status;
   void *oldValuePartBuffer = alloca(arguments->Size);
   if(parameter->HasPartialInterface == true)
   {
      /* save old value part that will be updated to be able to revert changes */
      status = parameter->GetFunction(parameter->Handle,
                                      &(SDevicePartialParameterArguments)
                                      {
                                         .Data.Get = oldValuePartBuffer,
                                         .Offset = arguments->Offset,
                                         .Size = arguments->Size
                                      });

      if(status != SDEVICE_OPERATION_STATUS_OK)
      {
         SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_GET_FAIL);
         return PARAMETER_MANAGER_STATUS_HANDLED_ERROR;
      }

      /* try set new value part */
      status = parameter->SetFunction(parameter->Handle,
                                      &(SDevicePartialParameterArguments)
                                      {
                                         .Data.Set = data,
                                         .Offset = arguments->Offset,
                                         .Size = arguments->Size
                                      });

      /* in case of processing error during write procedure, rollback all changes */
      if(status == SDEVICE_OPERATION_STATUS_PROCESSING_ERROR)
      {
         writeRollbackFlag = true;
         status = parameter->SetFunction(parameter->Handle,
                                         &(SDevicePartialParameterArguments)
                                         {
                                            .Data.Set = oldValuePartBuffer,
                                            .Offset = arguments->Offset,
                                            .Size = arguments->Size
                                         });
      }
   }
   else
   {
      /* read full current parameter value */
      void *valueBuffer = alloca(parameter->Size);
      if(parameter->GetFunction(parameter->Handle, valueBuffer) != SDEVICE_OPERATION_STATUS_OK)
      {
         SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_GET_FAIL);
         return PARAMETER_MANAGER_STATUS_HANDLED_ERROR;
      }

      /* check if parameter alreay has this value */
      if(memcmp(valueBuffer + arguments->Offset, data, arguments->Size) == 0)
         return PARAMETER_MANAGER_STATUS_OK;

      /* save old value part that will be updated to be able to revert changes */
      memcpy(oldValuePartBuffer, valueBuffer + arguments->Offset, arguments->Size);

      /* create full new value */
      memcpy(valueBuffer + arguments->Offset, data, arguments->Size);

      /* try set new value */
      status = parameter->SetFunction(parameter->Handle, valueBuffer);

      /* in case of processing error during write procedure, rollback all changes */
      if(status == SDEVICE_OPERATION_STATUS_PROCESSING_ERROR)
      {
         writeRollbackFlag = true;
         memcpy(valueBuffer + arguments->Offset, oldValuePartBuffer, arguments->Size);
         status = parameter->SetFunction(parameter->Handle, valueBuffer);
      }
   }

   /* process state in case of write rollback due to processing error */
   if(writeRollbackFlag == true)
   {
      /* reversion succesfull, return "handled" error flag */
      if(status == SDEVICE_OPERATION_STATUS_OK)
      {
         SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_SET_FAIL);
         return PARAMETER_MANAGER_STATUS_HANDLED_ERROR;
      }

      /* reversion failed, return "unhandled" error flag */
      SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_VALUE_REVERSION_FAIL);
      return PARAMETER_MANAGER_STATUS_UNHANDLED_ERROR;
   }

   /* process common states */
   switch(status)
   {
      case SDEVICE_OPERATION_STATUS_OK:
         return PARAMETER_MANAGER_STATUS_OK;
         break;

      case SDEVICE_OPERATION_STATUS_VALIDATION_ERROR:
         SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_SET_FAIL);
         return PARAMETER_MANAGER_STATUS_HANDLED_ERROR;

      default:
         SDeviceAssert(false);
         break;
   }

   return PARAMETER_MANAGER_STATUS_UNHANDLED_ERROR;
}

ParameterManagerStatus ParameterManagerSaveSetting(__SDEVICE_HANDLE(ParameterManager) *handle, size_t parameterId)
{
   SDeviceAssert(handle != NULL);
   SDeviceAssert(handle->IsInitialized == true);
   SDeviceAssert(parameterId < handle->Init.ParameterList->Length);

   const ParameterManagerParameter *parameter = &handle->Init.ParameterList->Parameters[parameterId];
   void *valueBuffer = alloca(parameter->Size);
   if(GetFullParameterValue(parameter, valueBuffer) != SDEVICE_OPERATION_STATUS_OK) /* FULL */
   {
      SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_GET_FAIL);
      return PARAMETER_MANAGER_STATUS_HANDLED_ERROR;
   }

   if(handle->Init.TryWriteToStorage(handle, parameter->StorageAddress, valueBuffer, parameter->Size) != true)
   {
      SDeviceRuntimeErrorRaised(handle, PARAMETER_MANAGER_RUNTIME_ERROR_WRITE_FAIL);
      return PARAMETER_MANAGER_STATUS_UNHANDLED_ERROR;
   }

   return PARAMETER_MANAGER_STATUS_OK;
}
