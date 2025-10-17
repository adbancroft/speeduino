#include "storage_api.h"

bool update(const storage_api_t &api, uint16_t address, byte value) {
  if (api.read(address)!=value) {
    api.write(address, value); 
    return true;
  }
  return false;    
}

__attribute__((noinline)) void updateBlock(const storage_api_t &api, uint16_t address, const byte* pFirst, const byte* pLast) {
  for (; pFirst != pLast; ++address, (void)++pFirst) {
    (void)update(api, address, *pFirst);
  }
}

__attribute__((noinline)) uint16_t updateBlockLimitWriteOps(const storage_api_t &api, uint16_t address, const byte* pFirst, const byte* pLast, uint16_t maxWrites) {
  while (pFirst!=pLast && maxWrites>0U) {
    if (update(api, address, *pFirst)) {
      --maxWrites;
    }
    ++address;
    ++pFirst;
  }

  return maxWrites;
}

__attribute__((noinline)) uint16_t loadBlock(const storage_api_t &api, int16_t address, byte *pFirst, const byte *pLast)
{
  for (; pFirst != pLast; ++address, (void)++pFirst) {
    *pFirst = api.read(address);
  }
  return address;
}

__attribute__((noinline)) void fillBlock(const storage_api_t &api, uint16_t address, uint16_t length, byte value) {
  for (uint16_t end=address+length; address<end; ++address) {
    (void)update(api, address, value);
  }
}

__attribute__((noinline)) void clearStorage(const storage_api_t &api) {
#if defined(STORAGE_API_CUSTOM_CLEAR)
  if (api.clear!=nullptr) {
    api.clear();
  } else {
    fillBlock(api, 0, api.length(), UINT8_MAX);
  }
#else
  fillBlock(api, 0, api.length(), UINT8_MAX);
#endif
}

 __attribute__((noinline)) void moveBlock(const storage_api_t &api, uint16_t dest, uint16_t source, uint16_t size) {
  // Implementation is modelled after memmove.
  if (source<dest) {
    // Source is before dest - in other words we are moving the block *up* the address space
    // Must copy in reverse to handle overlapping blocks.
    dest += size;
    source += size;
    while(size!=0) {
      --dest;
      --source;
      update(api, dest, api.read(source));
      --size;
    }
  } else {
    // Source is after dest - in other words we are moving the block *down* the address space
    // Must use in-order copy to handle overlapping blocks.
    while(size!=0) {
      update(api, dest, api.read(source));
      ++dest;
      ++source;
      --size;
    }
  }
}
