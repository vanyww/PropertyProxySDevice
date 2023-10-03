#include "private.h"

#include "SDeviceCore/heap.h"
#include "SDeviceCore/common.h"

#include <memory.h>
#include <stdlib.h>

SDEVICE_IDENTITY_BLOCK_DEFINITION(PropertyProxy,
                                  ((const SDeviceUuid)
                                  {
                                     .High = PROPERTY_PROXY_SDEVICE_UUID_HIGH,
                                     .Low  = PROPERTY_PROXY_SDEVICE_UUID_LOW
                                  }),
                                  ((const SDeviceVersion)
                                  {
                                     .Major = PROPERTY_PROXY_SDEVICE_VERSION_MAJOR,
                                     .Minor = PROPERTY_PROXY_SDEVICE_VERSION_MINOR,
                                     .Patch = PROPERTY_PROXY_SDEVICE_VERSION_PATCH
                                  }));

SDEVICE_CREATE_HANDLE_DECLARATION(PropertyProxy, init, owner, identifier, context)
{
   UNUSED_PARAMETER(init);

   ThisHandle *handle = SDeviceAllocHandle(sizeof(ThisInitData), sizeof(ThisRuntimeData));
   handle->Header = (SDeviceHandleHeader)
   {
      .Context       = context,
      .OwnerHandle   = owner,
      .IdentityBlock = &SDEVICE_IDENTITY_BLOCK(PropertyProxy),
      .LatestStatus  = PROPERTY_PROXY_SDEVICE_STATUS_OK,
      .Identifier    = identifier
   };

   return handle;
}

SDEVICE_DISPOSE_HANDLE_DECLARATION(PropertyProxy, handlePointer)
{
   SDeviceAssert(handlePointer != NULL);

   ThisHandle **_handlePointer = handlePointer;
   ThisHandle *handle = *_handlePointer;

   SDeviceAssert(IS_VALID_THIS_HANDLE(handle));

   SDeviceFreeHandle(handle);
   *_handlePointer = NULL;
}

static SDevicePropertyStatus SetCommonPropertyPart(ThisHandle                                *handle,
                                                   const PropertyInternal                    *property,
                                                   void                                      *propertyHandle,
                                                   const SDeviceSetPartialPropertyParameters *parameters,
                                                   bool                                      *wasChanged)
{
   uint8_t *propertyBuffer = alloca(property->Size);
   SDevicePropertyStatus getPropertyStatus =
         property->Interface.AsCommon.Get(propertyHandle, propertyBuffer);

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getPropertyStatus));

   if(getPropertyStatus != SDEVICE_PROPERTY_STATUS_OK)
      return getPropertyStatus;

   bool willPropertyChange;
   bool usePropertyCompare = (wasChanged != NULL);

#if PROPERTY_PROXY_SDEVICE_USE_ROLLBACK
   uint8_t *rollbackBuffer;
   bool usePropertyRollback = property->AllowsRollback;

   if(usePropertyRollback)
   {
      rollbackBuffer = alloca(parameters->Size);
      memcpy(rollbackBuffer, &propertyBuffer[parameters->Offset], parameters->Size);
   }
   else
#endif
   if(usePropertyCompare)
   {
      willPropertyChange =
            (memcmp(&propertyBuffer[parameters->Offset], parameters->Data, parameters->Size) != 0);
   }

   memcpy(&propertyBuffer[parameters->Offset], parameters->Data, parameters->Size);

   SDevicePropertyStatus setPropertyStatus =
         property->Interface.AsCommon.Set(propertyHandle, parameters->Data);

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setPropertyStatus));

   switch(setPropertyStatus)
   {
      case SDEVICE_PROPERTY_STATUS_OK:
         if(usePropertyCompare)
         {
#if PROPERTY_PROXY_SDEVICE_USE_ROLLBACK
            if(usePropertyRollback)
            {
               willPropertyChange =
                     (memcmp(rollbackBuffer, parameters->Data, parameters->Size) != 0);
            }
#endif
            *wasChanged = willPropertyChange;
         }
         break;

#if PROPERTY_PROXY_SDEVICE_USE_ROLLBACK
      case SDEVICE_PROPERTY_STATUS_PROCESSING_ERROR:
         if(usePropertyRollback)
         {
            memcpy(&propertyBuffer[parameters->Offset], rollbackBuffer, parameters->Size);

            SDevicePropertyStatus rollbackPropertyStatus =
                  property->Interface.AsCommon.Set(propertyHandle, propertyBuffer);

            SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(rollbackPropertyStatus));

            LogSettingRollbackStatus(handle, PROPERTY_PROXY_SDEVICE_STATUS_ROLLBACK_OCCURRED, rollbackPropertyStatus);
            return SDEVICE_PROPERTY_STATUS_PROCESSING_ERROR;
         }
         break;
#endif

      default:
         break;
   }

   return setPropertyStatus;
}

static SDevicePropertyStatus SetCommonPropertyFull(ThisHandle                                *handle,
                                                   const PropertyInternal                    *property,
                                                   void                                      *propertyHandle,
                                                   const SDeviceSetPartialPropertyParameters *parameters,
                                                   bool                                      *wasChanged)
{
   uint8_t *propertyBuffer;
   bool usePropertyCompare = (wasChanged != NULL);

#if PROPERTY_PROXY_SDEVICE_USE_ROLLBACK
   bool usePropertyRollback = property->AllowsRollback;

   if(property->Interface.AsPartial.Get != NULL && (usePropertyRollback || usePropertyCompare))
#else
   if(property->Interface.AsPartial.Get != NULL && usePropertyCompare)
#endif
   {
      propertyBuffer = alloca(parameters->Size);
      SDevicePropertyStatus getPropertyStatus =
            property->Interface.AsCommon.Get(propertyHandle, propertyBuffer);

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getPropertyStatus));

      if(getPropertyStatus != SDEVICE_PROPERTY_STATUS_OK)
         return getPropertyStatus;
   }
   else
   {
      propertyBuffer = NULL;
   }

   SDevicePropertyStatus setPropertyStatus =
         property->Interface.AsCommon.Set(propertyHandle, parameters->Data);

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setPropertyStatus));

   switch(setPropertyStatus)
   {
      case SDEVICE_PROPERTY_STATUS_OK:
         if(usePropertyCompare)
         {
            *wasChanged =
                  (propertyBuffer == NULL) ? true : (memcmp(propertyBuffer, parameters->Data, parameters->Size) != 0);
         }
         break;

#if PROPERTY_PROXY_SDEVICE_USE_ROLLBACK
      case SDEVICE_PROPERTY_STATUS_PROCESSING_ERROR:
         if(usePropertyRollback && propertyBuffer != NULL)
         {
            SDevicePropertyStatus rollbackPropertyStatus =
                  property->Interface.AsCommon.Set(propertyHandle, propertyBuffer);

            SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(rollbackPropertyStatus));

            LogSettingRollbackStatus(handle, PROPERTY_PROXY_SDEVICE_STATUS_ROLLBACK_OCCURRED, rollbackPropertyStatus);
            return SDEVICE_PROPERTY_STATUS_PROCESSING_ERROR;
         }
         break;
#endif

      default:
         break;
   }

   return setPropertyStatus;
}

static SDevicePropertyStatus SetPartialProperty(ThisHandle                                *handle,
                                                const PropertyInternal                    *property,
                                                void                                      *propertyHandle,
                                                const SDeviceSetPartialPropertyParameters *parameters,
                                                bool                                      *wasChanged)
{
   uint8_t *propertyBuffer;
   bool usePropertyCompare = (wasChanged != NULL);

#if PROPERTY_PROXY_SDEVICE_USE_ROLLBACK
   bool usePropertyRollback = property->AllowsRollback;

   if(property->Interface.AsPartial.Get != NULL && (usePropertyRollback || usePropertyCompare))
#else
   if(property->Interface.AsPartial.Get != NULL && usePropertyCompare)
#endif
   {
      propertyBuffer = alloca(parameters->Size);
      SDevicePropertyStatus getPropertyStatus =
            property->Interface.AsPartial.Get(propertyHandle,
                                              &(const SDeviceGetPartialPropertyParameters)
                                              {
                                                 .Offset = parameters->Offset,
                                                 .Size   = parameters->Size,
                                                 .Data   = propertyBuffer
                                              });

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getPropertyStatus));

      if(getPropertyStatus != SDEVICE_PROPERTY_STATUS_OK)
         return getPropertyStatus;
   }
   else
   {
      propertyBuffer = NULL;
   }

   SDevicePropertyStatus setPropertyStatus =
         property->Interface.AsPartial.Set(propertyHandle, parameters);

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setPropertyStatus));

   switch(setPropertyStatus)
   {
      case SDEVICE_PROPERTY_STATUS_OK:
         if(usePropertyCompare)
         {
            *wasChanged =
                  (propertyBuffer == NULL) ? true : (memcmp(propertyBuffer, parameters->Data, parameters->Size) != 0);
         }
         break;

#if PROPERTY_PROXY_SDEVICE_USE_ROLLBACK
      case SDEVICE_PROPERTY_STATUS_PROCESSING_ERROR:
         if(usePropertyRollback && propertyBuffer != NULL)
         {
            SDevicePropertyStatus rollbackPropertyStatus =
                  property->Interface.AsPartial.Set(propertyHandle,
                                                    &(const SDeviceSetPartialPropertyParameters)
                                                    {
                                                       .Offset = parameters->Offset,
                                                       .Size   = parameters->Size,
                                                       .Data   = propertyBuffer
                                                    });

            SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(rollbackPropertyStatus));

            LogSettingRollbackStatus(handle, PROPERTY_PROXY_SDEVICE_STATUS_ROLLBACK_OCCURRED, rollbackPropertyStatus);
            return SDEVICE_PROPERTY_STATUS_PROCESSING_ERROR;
         }
         break;
#endif

      default:
         break;
   }

   return setPropertyStatus;
}

SDevicePropertyStatus PropertyProxySDeviceGet(ThisHandle                                *handle,
                                              const PropertyInternal                    *property,
                                              void                                      *propertyHandle,
                                              const SDeviceGetPartialPropertyParameters *parameters)
{
   SDeviceAssert(IS_VALID_THIS_HANDLE(handle));

   SDeviceAssert(property != NULL);
   SDeviceAssert(parameters != NULL);
   SDeviceAssert(propertyHandle != NULL);
   SDeviceAssert(parameters->Data != NULL);
   SDeviceAssert(property->Interface.Get != NULL);

   SDeviceAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset) &&
                 parameters->Size + parameters->Offset <= property->Size);

   SDevicePropertyStatus getPropertyStatus;

   if(property->IsPartial)
   {
      getPropertyStatus = property->Interface.AsPartial.Get(propertyHandle, parameters);

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getPropertyStatus));
   }
   else if(parameters->Size == property->Size)
   {
      getPropertyStatus = property->Interface.AsCommon.Get(propertyHandle, parameters->Data);

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getPropertyStatus));
   }
   else
   {
      uint8_t *valueBuffer = alloca(property->Size);
      getPropertyStatus = property->Interface.AsCommon.Get(propertyHandle, valueBuffer);

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getPropertyStatus));

      if(getPropertyStatus == SDEVICE_PROPERTY_STATUS_OK)
         memcpy(parameters->Data, &valueBuffer[parameters->Offset], parameters->Size);
   }

   return getPropertyStatus;
}

SDevicePropertyStatus PropertyProxySDeviceSet(ThisHandle                                *handle,
                                              const PropertyInternal                    *property,
                                              void                                      *propertyHandle,
                                              const SDeviceSetPartialPropertyParameters *parameters,
                                              bool                                      *wasChanged)
{
   SDeviceAssert(IS_VALID_THIS_HANDLE(handle));

   SDeviceAssert(property != NULL);
   SDeviceAssert(parameters != NULL);
   SDeviceAssert(propertyHandle != NULL);
   SDeviceAssert(parameters->Data != NULL);
   SDeviceAssert(property->Interface.Set != NULL);

   SDeviceAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset) &&
                 parameters->Size + parameters->Offset <= property->Size);

   SetPropertyFunction setPropertyFunction;

   if(property->IsPartial)
   {
      setPropertyFunction = SetPartialProperty;
   }
   else if(parameters->Size == property->Size)
   {
      setPropertyFunction = SetCommonPropertyFull;
   }
   else
   {
      SDeviceAssert(property->Interface.Get != NULL);

      setPropertyFunction = SetCommonPropertyPart;
   }

   return setPropertyFunction(handle, property, propertyHandle, parameters, wasChanged);
}
