#ifndef __SPIFFS_DEV_H__
#define __SPIFFS_DEV_H__

#include <stddef.h>
#include "sys/alt_dev.h"
#include "sys/alt_llist.h"
#include "spiffs.h"

struct spiffs_dev_t
{
  alt_dev fs_dev;
  spiffs fss;
  spiffs_config cfg;
  alt_u16 work_buf[(SPIFFS_LOG_PAGE_SIZE)];
  alt_u32 fds[(SPIFFS_MAX_FDS)];
  alt_u32 cache_buf[(SPIFFS_LOG_PAGE_SIZE)+8];
};

extern void spiffs_dev_init(struct spiffs_dev_t *dev);

#define SPIFFS_DEV_INSTANCE(name, dev) \
  static struct spiffs_dev_t dev = \
  {                                \
    {                              \
      ALT_LLIST_ENTRY,             \
      name##_NAME,                 \
    },                             \
  }

#define SPIFFS_DEV_INIT(name, dev) \
  spiffs_dev_init(&dev)

/* vim: set et sts=2 sw=2: */
#endif /* __SPIFFS_DEV_H__ */
