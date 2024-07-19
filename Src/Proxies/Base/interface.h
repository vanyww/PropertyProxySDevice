#pragma once

#include "../../private.h"

typedef SDevicePropertyStatus (* GetPropertyFunction)(
      ThisHandle                                *handle,
      const void                                *interface,
      void                                      *target,
      const SDeviceGetPartialPropertyParameters *parameters);

typedef SDevicePropertyStatus (* SetPropertyFunction)(
      ThisHandle                                *handle,
      const void                                *interface,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters,
      bool                                      *didChange);

typedef struct
{
   GetPropertyFunction Get;
   SetPropertyFunction Set;
} PropertyProxyInterface;

#define COMPOSE_PROPERTY_PROXY_INTERFACE(type_name) (                                                                  \
   (const PropertyProxyInterface)                                                                                      \
   {                                                                                                                   \
      .Get = PROPERTY_PROXY_GET(type_name),                                                                            \
      .Set = PROPERTY_PROXY_SET(type_name)                                                                             \
   })

#define PROPERTY_PROXY_GET_RETURN_VALUE SDevicePropertyStatus
#define PROPERTY_PROXY_GET_ARGUMENTS(handle_name, interface_name, target_name, parameters_name) (                      \
   ThisHandle                                *handle_name,                                                             \
   const void                                *interface_name,                                                          \
   void                                      *target_name,                                                             \
   const SDeviceGetPartialPropertyParameters *parameters_name)
#define PROPERTY_PROXY_GET_POINTER(pointer_name)                                                                       \
   PROPERTY_PROXY_GET_RETURN_VALUE (* pointer_name) PROPERTY_PROXY_GET_ARGUMENTS(,,,)
#define PROPERTY_PROXY_GET(type_name)                                                                                  \
   _PropertyProxySDeviceGet##type_name##Property
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
   _PropertyProxySDeviceSet##type_name##Property
#define PROPERTY_PROXY_SET_DECLARATION(                                                                                \
      type_name, handle_name, interface_name, target_name, parameters_name, did_change_name)                           \
   PROPERTY_PROXY_SET_RETURN_VALUE                                                                                     \
   PROPERTY_PROXY_SET(type_name)                                                                                       \
   PROPERTY_PROXY_SET_ARGUMENTS(handle_name, interface_name, target_name, parameters_name, did_change_name)
