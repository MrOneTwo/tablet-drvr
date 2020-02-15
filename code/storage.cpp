#define ASSETS_MAX_COUNT 128

typedef struct Memory {
  bool32 isInitialized;

  uint64 transientMemorySize;
  void* transientMemory;
  void* transientTail;

  uint64 persistentMemorySize;
  void* persistentMemory;
  void* persistentTail;
} Memory;

