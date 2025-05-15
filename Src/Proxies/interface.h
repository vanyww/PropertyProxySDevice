#pragma once

#include "../private.h"

#define GET_PROPERTY_RETURN_VALUE SDevicePropertyStatus
#define GET_PROPERTY_ARGUMENTS(interface_name, target_name, parameters_name) (                                         \
   const void                                *interface_name,                                                          \
   void                                      *target_name,                                                             \
   const SDeviceGetPartialPropertyParameters *parameters_name)
#define GET_PROPERTY_POINTER(pointer_name)                                                                             \
   GET_PROPERTY_RETURN_VALUE (* pointer_name) GET_PROPERTY_ARGUMENTS(,,)
#define GET_PROPERTY(type_name)                                                                                        \
   PropertyProxySDeviceInternalGet##type_name##Property
#define GET_PROPERTY_DECLARATION(type_name, interface_name, target_name, parameters_name)                              \
   GET_PROPERTY_RETURN_VALUE                                                                                           \
   GET_PROPERTY(type_name)                                                                                             \
   GET_PROPERTY_ARGUMENTS(interface_name, target_name, parameters_name)

#define SET_PROPERTY_RETURN_VALUE SDevicePropertyStatus
#define SET_PROPERTY_ARGUMENTS(interface_name, target_name, parameters_name, did_change_name) (                        \
   const void                                *interface_name,                                                          \
   void                                      *target_name,                                                             \
   const SDeviceSetPartialPropertyParameters *parameters_name,                                                         \
   bool                                      *did_change_name)
#define SET_PROPERTY_POINTER(pointer_name)                                                                             \
   SET_PROPERTY_RETURN_VALUE (* pointer_name) SET_PROPERTY_ARGUMENTS(,,,)
#define SET_PROPERTY(type_name)                                                                                        \
   PropertyProxySDeviceInternalSet##type_name##Property
#define SET_PROPERTY_DECLARATION(type_name, interface_name, target_name, parameters_name, did_change_name)             \
   SET_PROPERTY_RETURN_VALUE                                                                                           \
   SET_PROPERTY(type_name)                                                                                             \
   SET_PROPERTY_ARGUMENTS(interface_name, target_name, parameters_name, did_change_name)
