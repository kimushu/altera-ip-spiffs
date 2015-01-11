/*
 * Storage device wrapper for SPIFFS (SPI Flash File System)
 *
 * - SPIFFS is made by Peter Andersson
 *   https://github.com/pellepl/spiffs
 *
 * - spiffs_dev wrapper is made by kimu_shu
 *   https://github.com/kimushu/peridot
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include "altera_avalon_spi.h"
#include "spiffs_dev.h"

#define SPIFFS_HAL_TRACE(fmt, ...) \
  (printf(fmt "\n", ##__VA_ARGS__))

static int spiffs_convert_to_errno(int fs_errno)
{
  return ENOTSUP;
}

static int spiffs_dev_open(alt_fd *fd, const char *name, int flags, int mode)
{
  spiffs_flags fs_flags = 0;
  struct spiffs_dev_t *dev = (struct spiffs_dev_t *)fd->dev;
  spiffs_file fs_fh;

  name += strlen(dev->fs_dev.name) + 1;

  if (flags & O_WRONLY)
    fs_flags |= SPIFFS_WRONLY;
  else if (flags & O_RDWR)
    fs_flags |= SPIFFS_RDWR;
  else
    fs_flags |= SPIFFS_RDONLY;

  if (flags & O_APPEND)
    fs_flags |= SPIFFS_APPEND;

  if (flags & O_CREAT)
    fs_flags |= SPIFFS_CREAT;

  if (flags & O_TRUNC)
    fs_flags |= SPIFFS_TRUNC;

  fs_fh = SPIFFS_open(&dev->fss, name, fs_flags, mode);
  if (fs_fh == -1) {
    return -spiffs_convert_to_errno(SPIFFS_errno(&dev->fss));
  }
  fd->priv = (alt_u8 *)(uintptr_t)fs_fh;
  return 0;
}

static int spiffs_dev_close(alt_fd *fd)
{
  struct spiffs_dev_t *dev = (struct spiffs_dev_t *)fd->dev;
  spiffs_file fs_fh = (uintptr_t)fd->priv;

  SPIFFS_close(&dev->fss, fs_fh);
  return 0;
}

static int spiffs_dev_read(alt_fd *fd, char *ptr, int len)
{
  struct spiffs_dev_t *dev = (struct spiffs_dev_t *)fd->dev;
  spiffs_file fs_fh = (uintptr_t)fd->priv;
  int result;

  result = SPIFFS_read(&dev->fss, fs_fh, ptr, len);

  if (result == -1) {
    return -spiffs_convert_to_errno(SPIFFS_errno(&dev->fss));
  }

  return result;
}

static int spiffs_dev_write(alt_fd *fd, const char *ptr, int len)
{
  struct spiffs_dev_t *dev = (struct spiffs_dev_t *)fd->dev;
  spiffs_file fs_fh = (uintptr_t)fd->priv;
  int result;

  result = SPIFFS_write(&dev->fss, fs_fh, (char *)ptr, len);

  if (result == -1) {
    return -spiffs_convert_to_errno(SPIFFS_errno(&dev->fss));
  }
  return result;
}

static int spiffs_dev_lseek(alt_fd *fd, int ptr, int dir)
{
  struct spiffs_dev_t *dev = (struct spiffs_dev_t *)fd->dev;
  spiffs_file fs_fh = (uintptr_t)fd->priv;
  int result;

  result = SPIFFS_lseek(&dev->fss, fs_fh, ptr, dir);
  /* dir: SEEK_xxx == SPIFFS_SEEK_xxx */

  if (result == -1) {
    return -spiffs_convert_to_errno(SPIFFS_errno(&dev->fss));
  }
  return result;
}

static int spiffs_dev_fstat(alt_fd *fd, struct stat *buf)
{
  struct spiffs_dev_t *dev = (struct spiffs_dev_t *)fd->dev;
  spiffs_file fs_fh = (uintptr_t)fd->priv;
  int result;
  spiffs_stat fs_stat;

  result = SPIFFS_fstat(&dev->fss, fs_fh, &fs_stat);
  if (result == -1) {
    return -spiffs_convert_to_errno(SPIFFS_errno(&dev->fss));
  }

  memset(buf, 0, sizeof(*buf));
  buf->st_ino = fs_stat.obj_id;
  buf->st_size = fs_stat.size;
  return 0;
}

static int spiffs_dev_ioctl(alt_fd *fd, int req, void *arg)
{
  return -ENOSYS;
}

static void spiffs_hal_set_addr(alt_u8 *buf, alt_u32 addr)
{
  if ((SPIFFS_ADDR_WIDTH) == 4) {
    buf[0] = (addr >> 24) & 0xff;
    buf[1] = (addr >> 16) & 0xff;
    buf[2] = (addr >>  8) & 0xff;
    buf[3] = (addr >>  0) & 0xff;
  }
  else if ((SPIFFS_ADDR_WIDTH) == 3) {
    buf[0] = (addr >> 16) & 0xff;
    buf[1] = (addr >>  8) & 0xff;
    buf[2] = (addr >>  0) & 0xff;
  }
}

static void spiffs_hal_wait_ready(void)
{
  alt_u8 cmd[1];
  alt_u8 res[1];

  cmd[0] = SPIFFS_OPCODE_RDSTS;
  do {
    alt_avalon_spi_command(
        SPIFFS_SPIC_BASE,
        SPIFFS_SLAVE_NUMBER,
        sizeof(cmd), cmd,
        sizeof(res), res,
        0);
  } while(res[0] & 0x01);
}

static alt_32 spiffs_hal_read(alt_u32 addr, alt_u32 size, alt_u8 *ptr)
{
  alt_u8 cmd[1 + (SPIFFS_ADDR_WIDTH)];
  spiffs_hal_wait_ready();

  SPIFFS_HAL_TRACE("R 0x%x,0x%x,0x%x", addr, size, (alt_u32)ptr);

  cmd[0] = SPIFFS_OPCODE_READ;
  spiffs_hal_set_addr(&cmd[1], addr);
  alt_avalon_spi_command(
      SPIFFS_SPIC_BASE,
      SPIFFS_SLAVE_NUMBER,
      sizeof(cmd), cmd,
      size, ptr,
      0);

  return SPIFFS_OK;
}

static alt_32 spiffs_hal_write(alt_u32 addr, alt_u32 size, alt_u8 *ptr)
{
  alt_u8 cmd[1 + (SPIFFS_ADDR_WIDTH)];
  alt_u32 page_size;

  spiffs_hal_wait_ready();

  while (size > 0) {
    SPIFFS_HAL_TRACE("W 0x%x,0x%x,0x%x", addr, size, (alt_u32)ptr);

    /* Write enable (automatically disabled when write completion) */
    cmd[0] = SPIFFS_OPCODE_WREN;
    alt_avalon_spi_command(
        SPIFFS_SPIC_BASE,
        SPIFFS_SLAVE_NUMBER,
        1, cmd,
        0, NULL,
        0);

    /* Write bytes */
    cmd[0] = SPIFFS_OPCODE_WRITE;
    spiffs_hal_set_addr(&cmd[1], addr);
    page_size = (SPIFFS_WRITE_PAGE_SIZE) - (addr & ((SPIFFS_WRITE_PAGE_SIZE) - 1));
    alt_avalon_spi_command(
        SPIFFS_SPIC_BASE,
        SPIFFS_SLAVE_NUMBER,
        sizeof(cmd), cmd,
        0, NULL,
        ALT_AVALON_SPI_COMMAND_MERGE);
    alt_avalon_spi_command(
        SPIFFS_SPIC_BASE,
        SPIFFS_SLAVE_NUMBER,
        page_size, ptr,
        0, NULL,
        0);

    spiffs_hal_wait_ready();

    size -= page_size;
    addr += page_size;
    ptr += page_size;
  }

  return SPIFFS_OK;
}

static alt_32 spiffs_hal_erase(alt_u32 addr, alt_u32 size)
{
  alt_u8 cmd[1 + (SPIFFS_ADDR_WIDTH)];

  spiffs_hal_wait_ready();

  while (size >= SPIFFS_ERASE_SIZE) {
    SPIFFS_HAL_TRACE("E 0x%x", addr);

    /* Write enable (automatically disabled when write completion) */
    cmd[0] = SPIFFS_OPCODE_WREN;
    alt_avalon_spi_command(
        SPIFFS_SPIC_BASE,
        SPIFFS_SLAVE_NUMBER,
        1, cmd,
        0, NULL,
        0);

    /* Erase block */
    cmd[0] = SPIFFS_OPCODE_ERASE;
    spiffs_hal_set_addr(&cmd[1], addr);
    alt_avalon_spi_command(
        SPIFFS_SPIC_BASE,
        SPIFFS_SLAVE_NUMBER,
        sizeof(cmd), cmd,
        0, NULL,
        0);

    spiffs_hal_wait_ready();

    size -= SPIFFS_ERASE_SIZE;
    addr += SPIFFS_ERASE_SIZE;
  }

  return SPIFFS_OK;
}

void spiffs_dev_init(struct spiffs_dev_t *dev)
{
  static struct spiffs_dev_t *cdev = 0;
  if(dev != NULL)
  {
    cdev = dev;
    return;
  }
  dev = cdev;
  int result;
  dev->cfg.phys_addr = SPIFFS_START_ADDR;
  dev->cfg.phys_size = SPIFFS_END_ADDR - SPIFFS_START_ADDR;
  dev->cfg.phys_erase_block = SPIFFS_ERASE_SIZE;
  dev->cfg.log_block_size = SPIFFS_LOG_BLOCK_SIZE;
  dev->cfg.log_page_size = SPIFFS_LOG_PAGE_SIZE;
  dev->cfg.hal_read_f = spiffs_hal_read;
  dev->cfg.hal_write_f = spiffs_hal_write;
  dev->cfg.hal_erase_f = spiffs_hal_erase;
  result = SPIFFS_mount(
      &dev->fss,
      &dev->cfg,
      (alt_u8 *)dev->work_buf,
      (alt_u8 *)dev->fds, sizeof(dev->fds),
      dev->cache_buf, sizeof(dev->cache_buf),
      0);

  if(result == SPIFFS_OK) {
    dev->fs_dev.open  = spiffs_dev_open;
    dev->fs_dev.close = spiffs_dev_close;
    dev->fs_dev.read  = spiffs_dev_read;
    dev->fs_dev.write = spiffs_dev_write;
    dev->fs_dev.lseek = spiffs_dev_lseek;
    dev->fs_dev.fstat = spiffs_dev_fstat;
    dev->fs_dev.ioctl = spiffs_dev_ioctl;
    alt_fs_reg(&dev->fs_dev);
  }
}

/* vim: set et sts=2 sw=2: */
