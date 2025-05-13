#pragma once

#include "../Proxies/Simple/simple.h"
#include "../Proxies/Partial/partial.h"

#include "SDeviceCore/common.h"

#define COMPOSE_PROXY(type_name) (                                                                                     \
   (const Proxy)                                                                                                       \
   {                                                                                                                   \
      .Get = GET_PROPERTY(type_name),                                                                                  \
      .Set = SET_PROPERTY(type_name),                                                                                  \
   })

typedef struct
{
   GET_PROPERTY_POINTER(Get);
   SET_PROPERTY_POINTER(Set);
} Proxy;

static const Proxy ProxyMap[] =
{
   [PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_IDX_SIMPLE]  = COMPOSE_PROXY(Simple),
   [PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_IDX_PARTIAL] = COMPOSE_PROXY(Partial)
};

static_assert(LENGTHOF(ProxyMap) == PROPERTY_PROXY_SDEVICE_PROPERTY_TYPE_IDXS_COUNT);
