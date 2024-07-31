#pragma once

#include "PropertyProxySDevice/public.h"

#define IS_VALID_THIS_HANDLE(handle) SDEVICE_IS_VALID_HANDLE(PropertyProxy, handle)

SDEVICE_RUNTIME_DATA_FORWARD_DECLARATION(PropertyProxy);

SDEVICE_RUNTIME_DATA_DECLARATION(PropertyProxy) { };

SDEVICE_HANDLE_DECLARATION(PropertyProxy);
SDEVICE_INTERNAL_ALIASES_DECLARATION(PropertyProxy);

typedef PropertyProxySDeviceProperty ThisProperty;
typedef PropertyProxySDevicePropertyType ThisPropertyType;

typedef PropertyProxySDeviceSimplePropertyInterface ThisSimplePropertyInterface;
typedef PropertyProxySDevicePartialPropertyInterface ThisPartialPropertyInterface;
