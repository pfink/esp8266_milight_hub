#include <GroupStatePersistence.h>
#ifdef SPIFFS
  #include <FS.h>
#else
  #include <stdio.h>
#endif

static const char FILE_PREFIX[] = "group_states/";

void GroupStatePersistence::get(const BulbId &id, GroupState& state) {  
  char path[30];
  memset(path, 0, 30);
  buildFilename(id, path);
  
  #ifdef SPIFFS
  if (SPIFFS.exists(path)) {
    File f = SPIFFS.open(path, "r");
    state.load(f);
    f.close();
  }
  #endif
}

void GroupStatePersistence::set(const BulbId &id, const GroupState& state) {
    char path[30];
  memset(path, 0, 30);
  buildFilename(id, path);

  #ifdef SPIFFS
  File f = SPIFFS.open(path, "w");
  state.dump(f);
  f.close();
  #endif
}

void GroupStatePersistence::clear(const BulbId &id) {  
  char path[30];
  buildFilename(id, path);

  #ifdef SPIFFS
  if (SPIFFS.exists(path)) {
    SPIFFS.remove(path);
  }
  #endif
}

char* GroupStatePersistence::buildFilename(const BulbId &id, char *buffer) {
  uint32_t compactId = id.getCompactId();
  return buffer + sprintf(buffer, "%s%x", FILE_PREFIX, compactId);
}
