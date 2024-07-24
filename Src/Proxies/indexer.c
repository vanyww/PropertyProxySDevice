#include "indexer.h"

#include "SDeviceCore/common.h"

#include <memory.h>
#include <stdlib.h>

static inline SDevicePropertyStatus SetIndexerProperty(
      ThisHandle                                *handle,
      const ThisIndexerPropertyInterface        *interface,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters)
{
   size_t startIdx          = parameters->Offset / interface->ItemSize,
         firstPartialOffset = parameters->Offset % interface->ItemSize;

   size_t firstPartialSize = (firstPartialOffset) ? interface->ItemSize - firstPartialOffset : 0;

   size_t fullsLength    = (parameters->Size - firstPartialSize) / interface->ItemSize,
         lastPartialSize = (parameters->Size - firstPartialSize) % interface->ItemSize;

   const void *bytesBuffer = parameters->Data;
   const void *itemsBuffers[fullsLength + (firstPartialSize > 0) + (lastPartialSize > 0)];

   if(firstPartialSize > 0)
   {
      void *itemBuffer = alloca(interface->ItemSize);
      SDevicePropertyStatus getStatus =
            interface->Get(
                  target,
                  &(const SDeviceGetIndexerPropertyParameters)
                  {
                     .Items    = &itemBuffer,
                     .StartIdx = startIdx,
                     .Length   = 1
                  });

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getStatus));

      if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
         return getStatus;

      memcpy(itemBuffer + firstPartialOffset, bytesBuffer, firstPartialSize);
      FIRST(itemsBuffers) = itemBuffer;
      bytesBuffer += firstPartialSize;
   }

   for(size_t i = (firstPartialSize > 0); i < LENGTHOF(itemsBuffers) - (lastPartialSize > 0); i++)
   {
      itemsBuffers[i] = bytesBuffer;
      bytesBuffer += interface->ItemSize;
   }

   if(lastPartialSize > 0)
   {
      void *itemBuffer = alloca(interface->ItemSize);
      SDevicePropertyStatus getStatus =
            interface->Get(
                  target,
                  &(const SDeviceGetIndexerPropertyParameters)
                  {
                     .Items    = &itemBuffer,
                     .StartIdx = startIdx + LENGTHOF(itemsBuffers) - 1,
                     .Length   = 1
                  });

      SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getStatus));

      if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
         return getStatus;

      memcpy(itemBuffer, bytesBuffer, lastPartialSize);
      LAST(itemsBuffers) = itemBuffer;
   }

   SDevicePropertyStatus setStatus =
         interface->Set(
               target,
               &(const SDeviceSetIndexerPropertyParameters)
               {
                  .Items    = itemsBuffers,
                  .StartIdx = startIdx,
                  .Length   = LENGTHOF(itemsBuffers)
               });

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setStatus));

   return setStatus;
}

static inline SDevicePropertyStatus SetCompareIndexerProperty(
      ThisHandle                                *handle,
      const ThisIndexerPropertyInterface        *interface,
      void                                      *target,
      const SDeviceSetPartialPropertyParameters *parameters,
      bool                                      *didChange)
{
   size_t startIdx          = parameters->Offset / interface->ItemSize,
         firstPartialOffset = parameters->Offset % interface->ItemSize;

   size_t firstPartialSize = (firstPartialOffset) ? interface->ItemSize - firstPartialOffset : 0;

   size_t fullsLength    = (parameters->Size - firstPartialSize) / interface->ItemSize,
         lastPartialSize = (parameters->Size - firstPartialSize) % interface->ItemSize;

   const void *bytesBuffer = parameters->Data;
   const void *itemsBuffers[fullsLength + (firstPartialSize > 0) + (lastPartialSize > 0)];

   for(size_t i = 0; i < LENGTHOF(itemsBuffers); i++)
      itemsBuffers[i] = alloca(interface->ItemSize);

   SDevicePropertyStatus getStatus =
         interface->Get(
               target,
               &(const SDeviceGetIndexerPropertyParameters)
               {
                  .Items    = (void **)itemsBuffers,
                  .StartIdx = startIdx,
                  .Length   = LENGTHOF(itemsBuffers)
               });

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getStatus));

   if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
      return getStatus;

   bool willChange = false;

   if(firstPartialSize > 0)
   {
      willChange = !!memcmp(FIRST(itemsBuffers) + firstPartialOffset, bytesBuffer, firstPartialSize);

      memcpy((void *)FIRST(itemsBuffers) + firstPartialOffset, bytesBuffer, firstPartialSize);

      bytesBuffer += firstPartialSize;
   }

   for(size_t i = (firstPartialSize > 0); i < LENGTHOF(itemsBuffers) - (lastPartialSize > 0); i++)
   {
      if(!willChange)
         willChange = !!memcmp(itemsBuffers[i], bytesBuffer, interface->ItemSize);

      itemsBuffers[i] = bytesBuffer;
      bytesBuffer += interface->ItemSize;
   }

   if(lastPartialSize > 0)
   {
      if(!willChange)
         willChange = !!memcmp(LAST(itemsBuffers), bytesBuffer, lastPartialSize);

      memcpy((void *)LAST(itemsBuffers), bytesBuffer, lastPartialSize);
   }

   SDevicePropertyStatus setStatus =
         interface->Set(
               target,
               &(const SDeviceSetIndexerPropertyParameters)
               {
                  .Items    = itemsBuffers,
                  .StartIdx = startIdx,
                  .Length   = LENGTHOF(itemsBuffers)
               });

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(setStatus));

   if(setStatus != SDEVICE_PROPERTY_STATUS_OK)
      return setStatus;

   *didChange = willChange;

   return SDEVICE_PROPERTY_STATUS_OK;
}

PROPERTY_PROXY_SET_DECLARATION(Indexer, handle, interface, target, parameters, didChange)
{
   const ThisIndexerPropertyInterface *_interface = interface;

   SDeviceAssert(_interface->Get);
   SDeviceAssert(_interface->Set);

   SDeviceAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset));
   SDeviceAssert(!WILL_INT_MUL_OVERFLOW(_interface->Length, _interface->ItemSize));
   SDeviceAssert(parameters->Size + parameters->Offset <= _interface->Length * _interface->ItemSize);

   return
         (didChange) ?
               SetCompareIndexerProperty(handle, _interface, target, parameters, didChange) :
               SetIndexerProperty(handle, _interface, target, parameters);
}

PROPERTY_PROXY_GET_DECLARATION(Indexer, handle, interface, target, parameters)
{
   const ThisIndexerPropertyInterface *_interface = interface;

   SDeviceAssert(_interface->Get);

   SDeviceAssert(!WILL_INT_ADD_OVERFLOW(parameters->Size, parameters->Offset));
   SDeviceAssert(!WILL_INT_MUL_OVERFLOW(_interface->Length, _interface->ItemSize));
   SDeviceAssert(parameters->Size + parameters->Offset <= _interface->Length * _interface->ItemSize);

   size_t startIdx          = parameters->Offset / _interface->ItemSize,
         firstPartialOffset = parameters->Offset % _interface->ItemSize;

   size_t firstPartialSize = (firstPartialOffset) ? _interface->ItemSize - firstPartialOffset : 0;

   size_t fullsLength    = (parameters->Size - firstPartialSize) / _interface->ItemSize,
         lastPartialSize = (parameters->Size - firstPartialSize) % _interface->ItemSize;

   void *bytesBuffer = parameters->Data + firstPartialSize;
   void *itemsBuffers[fullsLength + (firstPartialSize > 0) + (lastPartialSize > 0)];

   if(firstPartialSize > 0)
      FIRST(itemsBuffers) = alloca(_interface->ItemSize);

   for(size_t idx = (firstPartialSize > 0); idx < LENGTHOF(itemsBuffers) - (lastPartialSize > 0); idx++)
   {
      itemsBuffers[idx] = bytesBuffer;
      bytesBuffer += _interface->ItemSize;
   }

   if(lastPartialSize > 0)
      LAST(itemsBuffers) = alloca(_interface->ItemSize);

   SDevicePropertyStatus getStatus =
         _interface->Get(
               target,
               &(const SDeviceGetIndexerPropertyParameters)
               {
                  .Items    = itemsBuffers,
                  .StartIdx = startIdx,
                  .Length   = LENGTHOF(itemsBuffers)
               });

   SDeviceAssert(SDEVICE_IS_VALID_PROPERTY_OPERATION_STATUS(getStatus));

   if(getStatus != SDEVICE_PROPERTY_STATUS_OK)
      return getStatus;

   if(firstPartialSize > 0)
      memcpy(parameters->Data, FIRST(itemsBuffers) + firstPartialOffset, firstPartialSize);

   if(lastPartialSize > 0)
      memcpy(parameters->Data + (parameters->Size - lastPartialSize), LAST(itemsBuffers), lastPartialSize);

   return SDEVICE_PROPERTY_STATUS_OK;
}
