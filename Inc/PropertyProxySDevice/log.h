#pragma once

#include "SDeviceCore/core.h"

typedef enum
{
   PROPERTY_PROXY_SDEVICE_STATUS_OK,
   PROPERTY_PROXY_SDEVICE_STATUS_ROLLBACK_OCCURRED
} PropertyProxySDeviceStatus;

typedef struct
{
   SDevicePropertyStatus RollbackStatus;
} PropertyProxySDeviceRollbackStatusLogExtras;
