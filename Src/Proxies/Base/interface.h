#pragma once

#include "../../private.h"

#define PROPERTY_PROXY_GET_RETURN_VALUE SDevicePropertyStatus
#define PROPERTY_PROXY_GET_ARGUMENTS(handle_name, interface_name, target_name, parameters_name) (                      \
   ThisHandle                                *handle_name,                                                             \
   const void                                *interface_name,                                                          \
   void                                      *target_name,                                                             \
   const SDeviceGetPartialPropertyParameters *parameters_name)
#define PROPERTY_PROXY_GET_POINTER(pointer_name)                                                                       \
   PROPERTY_PROXY_GET_RETURN_VALUE (* pointer_name) PROPERTY_PROXY_GET_ARGUMENTS(,,,)
#define PROPERTY_PROXY_GET(type_name)                                                                                  \
   PropertyProxySDeviceInternalGet##type_name##Property
#define PROPERTY_PROXY_GET_DECLARATION(                                                                                \
      type_name, handle_name, interface_name, target_name, parameters_name)                                            \
   PROPERTY_PROXY_GET_RETURN_VALUE                                                                                     \
   PROPERTY_PROXY_GET(type_name)                                                                                       \
   PROPERTY_PROXY_GET_ARGUMENTS(handle_name, interface_name, target_name, parameters_name)

#define PROPERTY_PROXY_SET_RETURN_VALUE SDevicePropertyStatus
#define PROPERTY_PROXY_SET_ARGUMENTS(handle_name, interface_name, target_name, parameters_name, did_change_name) (     \
   ThisHandle                                *handle_name,                                                             \
   const void                                *interface_name,                                                          \
   void                                      *target_name,                                                             \
   const SDeviceSetPartialPropertyParameters *parameters_name,                                                         \
   bool                                      *did_change_name)
#define PROPERTY_PROXY_SET_POINTER(pointer_name)                                                                       \
   PROPERTY_PROXY_SET_RETURN_VALUE (* pointer_name) PROPERTY_PROXY_SET_ARGUMENTS(,,,,)
#define PROPERTY_PROXY_SET(type_name)                                                                                  \
   PropertyProxySDeviceInternalSet##type_name##Property
#define PROPERTY_PROXY_SET_DECLARATION(                                                                                \
      type_name, handle_name, interface_name, target_name, parameters_name, did_change_name)                           \
   PROPERTY_PROXY_SET_RETURN_VALUE                                                                                     \
   PROPERTY_PROXY_SET(type_name)                                                                                       \
   PROPERTY_PROXY_SET_ARGUMENTS(handle_name, interface_name, target_name, parameters_name, did_change_name)

#define PROPERTY_PROXY_COMPUTE_TOTAL_SIZE_RETURN_VALUE size_t
#define PROPERTY_PROXY_COMPUTE_TOTAL_SIZE_ARGUMENTS(interface_name) (const void *interface_name)
#define PROPERTY_PROXY_COMPUTE_TOTAL_SIZE_POINTER(pointer_name)                                                        \
   PROPERTY_PROXY_COMPUTE_TOTAL_SIZE_RETURN_VALUE (* pointer_name) PROPERTY_PROXY_COMPUTE_TOTAL_SIZE_ARGUMENTS()
#define PROPERTY_PROXY_COMPUTE_TOTAL_SIZE(type_name)                                                                   \
   PropertyProxySDeviceInternalGetTotalSize##type_name##Property
#define PROPERTY_PROXY_COMPUTE_TOTAL_SIZE_DECLARATION(type_name, interface_name)                                       \
   PROPERTY_PROXY_COMPUTE_TOTAL_SIZE_RETURN_VALUE                                                                      \
   PROPERTY_PROXY_COMPUTE_TOTAL_SIZE(type_name)                                                                        \
   PROPERTY_PROXY_COMPUTE_TOTAL_SIZE_ARGUMENTS(interface_name)

typedef struct
{
   PROPERTY_PROXY_GET_POINTER(Get);
   PROPERTY_PROXY_SET_POINTER(Set);
   PROPERTY_PROXY_COMPUTE_TOTAL_SIZE_POINTER(ComputeTotalSize);
} PropertyProxyInterface;

#define COMPOSE_PROPERTY_PROXY_INTERFACE(type_name) (                                                                  \
   (const PropertyProxyInterface)                                                                                      \
   {                                                                                                                   \
      .Get              = PROPERTY_PROXY_GET(type_name),                                                               \
      .Set              = PROPERTY_PROXY_SET(type_name),                                                               \
      .ComputeTotalSize = PROPERTY_PROXY_COMPUTE_TOTAL_SIZE(type_name),                                                \
   })
