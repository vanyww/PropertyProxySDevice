#include "../Inc/ParameterTransactionProxySDevice/core.h"

__SDEVICE_INITIALIZE_HANDLE_DECLARATION(ParameterManager, handle)
{
   SDeviceAssert(handle != NULL);
   SDeviceAssert(handle->IsInitialized != true);
   SDeviceAssert(handle->Init.ParameterList != NULL);
   SDeviceAssert(handle->Init.TryWriteToStorage != NULL);
   SDeviceAssert(handle->Init.TryReadFromStorage != NULL);
   SDeviceAssert(handle->Init.ParameterList->Parameters != NULL);

#ifdef __SDEVICE_ASSERT
   for(size_t i = 0; i < handle->Init.ParameterList->Length; i++)
   {
      const ParameterManagerParameter *parameter = &handle->Init.ParameterList->Parameters[i];
      SDeviceAssert(parameter->DefaultValue != NULL);
      SDeviceAssert(parameter->SetFunction != NULL);
      SDeviceAssert(parameter->GetFunction != NULL);
      SDeviceAssert(parameter->Size > 0);
   }
#endif

   handle->IsInitialized = true;
}
