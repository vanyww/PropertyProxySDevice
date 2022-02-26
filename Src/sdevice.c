#include "../Inc/ParameterTransactionProxySDevice/core.h"

__SDEVICE_INITIALIZE_HANDLE_DECLARATION(ParameterTransactionProxy, handle)
{
   SDeviceAssert(handle != NULL);
   SDeviceAssert(handle->IsInitialized != true);

   handle->IsInitialized = true;
}
