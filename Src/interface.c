#include "../Inc/ParameterTransactionProxySDevice/core.h"

#include <alloca.h>
#include <memory.h>

static inline ParameterTransactionProxyStatus WriteWithoutRollback(__SDEVICE_HANDLE(ParameterTransactionProxy) *handle,
                                                                   ParameterTransactionProxyArguments arguments,
                                                                   const void *data)
{
   SDeviceOperationStatus status;
   if(arguments.Parameter->HasPartialInterface == true)
   {
      status = arguments.Parameter->SetFunction(arguments.Parameter->Handle,
                                                &(SDevicePartialParameterArguments)
                                                {
                                                   .Data.Set = data,
                                                   .Offset = arguments.Offset,
                                                   .Size = arguments.Size
                                                });
   }
   else
   {
      if(SIZE_MAX - arguments.Size < arguments.Offset || arguments.Parameter->Size > arguments.Offset + arguments.Size)
      {
         SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_WRONG_OPERATION_SIZE);
         return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;
      }

      if(arguments.Size != arguments.Parameter->Size)
      {
         SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_WRONG_OPERATION_TYPE);
         return PARAMETER_TRANSACTION_PROXY_STATUS_UNHANDLED_ERROR;
      }

      status = arguments.Parameter->SetFunction(arguments.Parameter->Handle, data);
   }

   switch(status)
   {
      case SDEVICE_OPERATION_STATUS_OK:
         return PARAMETER_TRANSACTION_PROXY_STATUS_OK;

      case SDEVICE_OPERATION_STATUS_VALIDATION_ERROR:
         SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_SET_FAIL);
         return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;

      case SDEVICE_OPERATION_STATUS_PROCESSING_ERROR:
         SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_SET_FAIL);
         return PARAMETER_TRANSACTION_PROXY_STATUS_UNHANDLED_ERROR;

      default:
         SDeviceAssert(false);
         return PARAMETER_TRANSACTION_PROXY_STATUS_UNHANDLED_ERROR;
   }
}

static inline ParameterTransactionProxyStatus WriteWithRollback(__SDEVICE_HANDLE(ParameterTransactionProxy) *handle,
                                                                ParameterTransactionProxyArguments arguments,
                                                                const void *data)
{
   bool rollbackFlag = false;
   SDeviceOperationStatus status;
   void *rollbackValue = alloca(arguments.Size);
   if(arguments.Parameter->HasPartialInterface == true)
   {
      /* save old value part that will be updated to be able to rollback */
      status = arguments.Parameter->GetFunction(arguments.Parameter->Handle,
                                                &(SDevicePartialParameterArguments)
                                                {
                                                   .Data.Get = rollbackValue,
                                                   .Offset = arguments.Offset,
                                                   .Size = arguments.Size
                                                });

      if(status != SDEVICE_OPERATION_STATUS_OK)
      {
         SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_GET_FAIL);
         return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;
      }

      /* set new value part */
      status = arguments.Parameter->SetFunction(arguments.Parameter->Handle,
                                                &(SDevicePartialParameterArguments)
                                                {
                                                   .Data.Set = data,
                                                   .Offset = arguments.Offset,
                                                   .Size = arguments.Size
                                                });

      /* in case of processing error during write procedure, rollback all changes */
      if(status == SDEVICE_OPERATION_STATUS_PROCESSING_ERROR)
      {
         rollbackFlag = true;
         status = arguments.Parameter->SetFunction(arguments.Parameter->Handle,
                                                   &(SDevicePartialParameterArguments)
                                                   {
                                                      .Data.Set = rollbackValue,
                                                      .Offset = arguments.Offset,
                                                      .Size = arguments.Size
                                                   });
      }
   }
   else
   {
      if(SIZE_MAX - arguments.Size < arguments.Offset || arguments.Parameter->Size > arguments.Offset + arguments.Size)
      {
         SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_WRONG_OPERATION_SIZE);
         return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;
      }

      if(arguments.Size == arguments.Parameter->Size)
      {
         /* save current value be able to revert changes */
         if(arguments.Parameter->GetFunction(arguments.Parameter->Handle, rollbackValue) != SDEVICE_OPERATION_STATUS_OK)
         {
            SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_GET_FAIL);
            return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;
         }

         /* check if parameter alreay has this value */
         if(memcmp(rollbackValue, data, arguments.Parameter->Size) == 0)
            return PARAMETER_TRANSACTION_PROXY_STATUS_OK;

         /* try set new value */
         status = arguments.Parameter->SetFunction(arguments.Parameter->Handle, data);

         /* in case of processing error during write procedure, rollback all changes */
         if(status == SDEVICE_OPERATION_STATUS_PROCESSING_ERROR)
         {
            rollbackFlag = true;
            status = arguments.Parameter->SetFunction(arguments.Parameter->Handle, rollbackValue);
         }
      }
      else
      {
         void *newValue = alloca(arguments.Size);

         /* read full current value */
         if(arguments.Parameter->GetFunction(arguments.Parameter->Handle, newValue) != SDEVICE_OPERATION_STATUS_OK)
         {
            SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_GET_FAIL);
            return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;
         }

         /* check if parameter alreay has this value */
         if(memcmp(newValue + arguments.Offset, data, arguments.Size) == 0)
            return PARAMETER_TRANSACTION_PROXY_STATUS_OK;

         /* save old value part that will be updated to be able to revert changes */
         memcpy(rollbackValue, newValue + arguments.Offset, arguments.Size);

         /* create full new value */
         memcpy(newValue + arguments.Offset, data, arguments.Size);

         /* try set new value */
         status = arguments.Parameter->SetFunction(arguments.Parameter->Handle, newValue);

         /* in case of processing error during write procedure, rollback all changes */
         if(status == SDEVICE_OPERATION_STATUS_PROCESSING_ERROR)
         {
            rollbackFlag = true;
            memcpy(newValue + arguments.Offset, rollbackValue, arguments.Size);
            status = arguments.Parameter->SetFunction(arguments.Parameter->Handle, newValue);
         }
      }
   }

   /* process state in case of write rollback due to processing error */
   if(rollbackFlag == true)
   {
      /* reversion succesfull, return "handled" error flag */
      if(status == SDEVICE_OPERATION_STATUS_OK)
      {
         SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_SET_FAIL);
         return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;
      }

      /* reversion failed, return "unhandled" error flag */
      SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_ROLLBACK_FAIL);
      return PARAMETER_TRANSACTION_PROXY_STATUS_UNHANDLED_ERROR;
   }

   switch(status)
   {
      case SDEVICE_OPERATION_STATUS_OK:
         return PARAMETER_TRANSACTION_PROXY_STATUS_OK;
         break;

      case SDEVICE_OPERATION_STATUS_VALIDATION_ERROR:
         SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_SET_FAIL);
         return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;

      default:
         SDeviceAssert(false);
         break;
   }

   return PARAMETER_TRANSACTION_PROXY_STATUS_UNHANDLED_ERROR;
}

ParameterTransactionProxyStatus ParameterTransactionProxyRead(__SDEVICE_HANDLE(ParameterTransactionProxy) *handle,
                                                              ParameterTransactionProxyArguments arguments,
                                                              void *data)
{
   SDeviceAssert(data != NULL);
   SDeviceAssert(handle != NULL);
   SDeviceAssert(handle->IsInitialized == true);

   if(arguments.Parameter->GetFunction == NULL)
   {
      SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_WRONG_OPERATION_TYPE);
      return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;
   }

   SDeviceOperationStatus status;

   if(arguments.Parameter->HasPartialInterface == true)
   {
      status = arguments.Parameter->GetFunction(arguments.Parameter->Handle,
                                                &(SDevicePartialParameterArguments)
                                                {
                                                   .Data.Get = data,
                                                   .Offset = arguments.Offset,
                                                   .Size = arguments.Size
                                                });
   }
   else
   {
      if(SIZE_MAX - arguments.Size < arguments.Offset || arguments.Parameter->Size > arguments.Offset + arguments.Size)
      {
         SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_WRONG_OPERATION_SIZE);
         return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;
      }

      /* check if read has to be done for full value, if so, proxy buffer is unnecessary */
      if(arguments.Size == arguments.Parameter->Size)
      {
         status = arguments.Parameter->GetFunction(arguments.Parameter->Handle, data);
      }
      else
      {
         /* use proxy buffer to get full value and then copy only requested part */
         void *valueBuffer = alloca(arguments.Parameter->Size);
         status = arguments.Parameter->GetFunction(arguments.Parameter->Handle, valueBuffer);
         memcpy(data, valueBuffer + arguments.Offset, arguments.Size);
      }
   }

   if(status != SDEVICE_OPERATION_STATUS_OK)
   {
      SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_GET_FAIL);
      return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;
   }

   return PARAMETER_TRANSACTION_PROXY_STATUS_OK;
}

ParameterTransactionProxyStatus ParameterTransactionProxyWrite(__SDEVICE_HANDLE(ParameterTransactionProxy) *handle,
                                                               ParameterTransactionProxyArguments arguments,
                                                               const void *data)
{
   SDeviceAssert(data != NULL);
   SDeviceAssert(handle != NULL);
   SDeviceAssert(handle->IsInitialized == true);

   if(arguments.Parameter->SetFunction == NULL)
   {
      SDeviceRuntimeErrorRaised(handle, PARAMETER_TRANSACTION_PROXY_RUNTIME_ERROR_WRONG_OPERATION_TYPE);
      return PARAMETER_TRANSACTION_PROXY_STATUS_HANDLED_ERROR;
   }

   return (arguments.Parameter->GetFunction == NULL) ?
          WriteWithoutRollback(handle, arguments, data) :
          WriteWithRollback(handle, arguments, data);
}
