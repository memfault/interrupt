typedef enum {
  kSettingsFileRead,
  kSettingsFileWrite,
  kSettingsFileDelete,
  // Other items to instrument
} eAnalyticsKey;

void analytics_inc(eAnalyticsKey key);
