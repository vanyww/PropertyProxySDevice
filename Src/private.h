#pragma once

#include "PropertyProxySDevice/public.h"

#define IS_VALID_THIS_HANDLE(handle) (                                                                                 \
   {                                                                                                                   \
      ThisHandle *_handle = (handle);                                                                                  \
                                                                                                                       \
      _handle &&                                                                                                       \
      SDeviceCompareIdentityBlocks(                                                                                    \
            SDeviceGetHandleIdentityBlock(_handle), &SDEVICE_IDENTITY_BLOCK(PropertyProxy));                           \
   })

SDEVICE_RUNTIME_DATA_FORWARD_DECLARATION(PropertyProxy);

SDEVICE_RUNTIME_DATA_DECLARATION(PropertyProxy) { };

SDEVICE_HANDLE_DECLARATION(PropertyProxy);
SDEVICE_INTERNAL_ALIASES_DECLARATION(PropertyProxy);

typedef PropertyProxySDevicePropertyType ThisPropertyType;
typedef PropertyProxySDeviceSimplePropertyInterface ThisSimplePropertyInterface;
typedef PropertyProxySDevicePartialPropertyInterface ThisPartialPropertyInterface;
typedef PropertyProxySDeviceIndexerPropertyInterface ThisIndexerPropertyInterface;
typedef PropertyProxySDeviceProperty ThisProperty;
