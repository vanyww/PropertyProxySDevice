#pragma once

#include "PropertyProxySDevice/public.h"

#include "SDeviceCore/errors.h"

#define IS_VALID_THIS_HANDLE(handle) (                                                                                 \
{                                                                                                                      \
   ThisHandle *_handle = (handle);                                                                                     \
   _handle != NULL &&                                                                                                  \
   SDeviceCompareIdentityBlocks(SDeviceGetHandleIdentityBlock(_handle),                                                \
                                 &SDEVICE_IDENTITY_BLOCK(PropertyProxy));                                              \
})

#define LogSettingRollbackStatus(handle, status, rollbackStatus) (                                                     \
{                                                                                                                      \
   PropertyProxySDeviceRollbackStatusLogExtras _extras =                                                               \
   {                                                                                                                   \
      .RollbackStatus = (rollbackStatus)                                                                               \
   };                                                                                                                  \
   SDeviceLogStatusWithExtras(handle, status, &_extras, sizeof(_extras));                                              \
})                                                                                                                     \

SDEVICE_RUNTIME_DATA_FORWARD_DECLARATION(PropertyProxy);

SDEVICE_RUNTIME_DATA_DECLARATION(PropertyProxy) { };

SDEVICE_HANDLE_DECLARATION(PropertyProxy);
SDEVICE_INTERNAL_ALIASES_DECLARATION(PropertyProxy);

typedef PropertyProxySDeviceProperty PropertyInternal;

typedef SDevicePropertyStatus (* GetPropertyFunction)(ThisHandle                                *handle,
                                                      const PropertyInternal                    *property,
                                                      void                                      *propertyHandle,
                                                      const SDeviceGetPartialPropertyParameters *parameters);
typedef SDevicePropertyStatus (* SetPropertyFunction)(ThisHandle                                *handle,
                                                      const PropertyInternal                    *property,
                                                      void                                      *propertyHandle,
                                                      const SDeviceSetPartialPropertyParameters *parameters,
                                                      bool                                      *wasChanged);
