#pragma once

#include "SDeviceCore/config.h"

#if !defined(PROPERTY_PROXY_SDEVICE_USE_EXTERNAL_CONFIG) || defined(DOXYGEN)
   #define PROPERTY_PROXY_SDEVICE_USE_EXTERNAL_CONFIG false
#endif

#if PROPERTY_PROXY_SDEVICE_USE_EXTERNAL_CONFIG
   #include "property_proxy_sdevice_config.h"
#endif

#if !defined(PROPERTY_PROXY_SDEVICE_USE_ROLLBACK) || defined(DOXYGEN)
   #define PROPERTY_PROXY_SDEVICE_USE_ROLLBACK false
#endif
