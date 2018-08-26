/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "../../gfx.h"

#if GFX_USE_GFILE && GFILE_NEED_FATFS && GFX_USE_OS_CHIBIOS && !GFILE_FATFS_EXTERNAL_LIB

#include "gfile_fatfs_wrapper.h"

#if HAL_USE_MMC_SPI && HAL_USE_SDC
	#error "cannot specify both MMC_SPI and SDC drivers"
#endif

#if HAL_USE_MMC_SPI
	extern MMCDriver MMCD1;
#elif HAL_USE_SDC
	extern SDCDriver SDCD1;
#else
	#error "MMC_SPI or SDC driver must be specified"
#endif

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
#define MMC     0
#define SDC     0
/*-----------------------------------------------------------------------*/

// WOW - Bugs galore!!! (in ChibiOS)
//	Bugs:
//		1. ChibiOS DMA operations do not do the appropriate cache flushing or invalidating
//			on cpu's that require it eg STM32F7 series.
//			Instead they provide explicit dmaBufferInvalidate and dmaBufferFlush calls
//			and rely on the user to explicitly flush the cache.
//			Solution: We explicitly flush the cache after any possible DMA operation.
//		2. Unfortunately these explicit routines also have a bug. They assume that the
//			specified data structure is aligned on a cache line boundary - not a good assumption.
//			Solution: We increase the size provided to ChibiOS so that it does it properly.
//						This assumes of course that we know the size of the cpu cache line.
#if CH_KERNEL_MAJOR > 2
	#define CPU_CACHE_LINE_SIZE			32
	#define CACHE_FLUSH(buf, sz)		dmaBufferFlush((buf), (sz)+(CPU_CACHE_LINE_SIZE-1))
	#define CACHE_INVALIDATE(buf, sz)	dmaBufferInvalidate((buf), (sz)+(CPU_CACHE_LINE_SIZE-1))
#else
	#define CACHE_FLUSH(buf, sz)
	#define CACHE_INVALIDATE(buf, sz)
#endif

/* Initialize a Drive                                                    */
DSTATUS disk_initialize (
    BYTE drv                /* Physical drive nmuber (0..) */
)
{
  DSTATUS stat;

  switch (drv) {
#if HAL_USE_MMC_SPI
  case MMC:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&MMCD1) != BLK_READY)
      stat |= STA_NOINIT;
    if (mmcIsWriteProtected(&MMCD1))
      stat |=  STA_PROTECT;
    return stat;
#else
  case SDC:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&SDCD1) != BLK_READY)
      stat |= STA_NOINIT;
    if (sdcIsWriteProtected(&SDCD1))
      stat |=  STA_PROTECT;
    return stat;
#endif
  }
  return STA_NODISK;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
    BYTE drv        /* Physical drive nmuber (0..) */
)
{
  DSTATUS stat;

  switch (drv) {
#if HAL_USE_MMC_SPI
  case MMC:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&MMCD1) != BLK_READY)
      stat |= STA_NOINIT;
    if (mmcIsWriteProtected(&MMCD1))
      stat |= STA_PROTECT;
    return stat;
#else
  case SDC:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&SDCD1) != BLK_READY)
      stat |= STA_NOINIT;
    if (sdcIsWriteProtected(&SDCD1))
      stat |= STA_PROTECT;
    return stat;
#endif
  }
  return STA_NODISK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
    BYTE drv,        /* Physical drive nmuber (0..) */
    BYTE *buff,        /* Data buffer to store read data */
    DWORD sector,    /* Sector address (LBA) */
    UINT count        /* Number of sectors to read (1..255) */
)
{
  switch (drv) {
#if HAL_USE_MMC_SPI
  case MMC:
    if (blkGetDriverState(&MMCD1) != BLK_READY)
      return RES_NOTRDY;
    if (mmcStartSequentialRead(&MMCD1, sector))
      return RES_ERROR;
    CACHE_FLUSH(buff, MMCSD_BLOCK_SIZE*count);
    while (count > 0) {
      if (mmcSequentialRead(&MMCD1, buff))
        return RES_ERROR;
      buff += MMCSD_BLOCK_SIZE;
      count--;
    }
    if (mmcStopSequentialRead(&MMCD1))
        return RES_ERROR;
    CACHE_INVALIDATE(buff, MMCSD_BLOCK_SIZE*count);
    return RES_OK;
#else
  case SDC:
    if (blkGetDriverState(&SDCD1) != BLK_READY)
      return RES_NOTRDY;
    CACHE_FLUSH(buff, MMCSD_BLOCK_SIZE*count);
    if (sdcRead(&SDCD1, sector, buff, count))
      return RES_ERROR;
    CACHE_INVALIDATE(buff, MMCSD_BLOCK_SIZE*count);
    return RES_OK;
#endif
  }
  return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
    BYTE drv,            /* Physical drive nmuber (0..) */
    const BYTE *buff,    /* Data to be written */
    DWORD sector,        /* Sector address (LBA) */
    UINT count            /* Number of sectors to write (1..255) */
)
{
  switch (drv) {
#if HAL_USE_MMC_SPI
  case MMC:
    if (blkGetDriverState(&MMCD1) != BLK_READY)
        return RES_NOTRDY;
    if (mmcIsWriteProtected(&MMCD1))
        return RES_WRPRT;
    if (mmcStartSequentialWrite(&MMCD1, sector))
        return RES_ERROR;
    CACHE_FLUSH(buff, MMCSD_BLOCK_SIZE*count);
    while (count > 0) {
        if (mmcSequentialWrite(&MMCD1, buff))
            return RES_ERROR;
        buff += MMCSD_BLOCK_SIZE;
        count--;
    }
    if (mmcStopSequentialWrite(&MMCD1))
        return RES_ERROR;
    return RES_OK;
#else
  case SDC:
    if (blkGetDriverState(&SDCD1) != BLK_READY)
      return RES_NOTRDY;
    CACHE_FLUSH(buff, MMCSD_BLOCK_SIZE*count);
    if (sdcWrite(&SDCD1, sector, buff, count))
      return RES_ERROR;
    return RES_OK;
#endif
  }
  return RES_PARERR;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
    BYTE drv,        /* Physical drive nmuber (0..) */
    BYTE ctrl,        /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
  switch (drv) {
#if HAL_USE_MMC_SPI
  case MMC:
    switch (ctrl) {
    case CTRL_SYNC:
        return RES_OK;
    case GET_SECTOR_SIZE:
        *((WORD *)buff) = MMCSD_BLOCK_SIZE;
        return RES_OK;
#if _USE_ERASE
    case CTRL_ERASE_SECTOR:
        mmcErase(&MMCD1, *((DWORD *)buff), *((DWORD *)buff + 1));
        return RES_OK;
#endif
    default:
        return RES_PARERR;
    }
#else
  case SDC:
    switch (ctrl) {
    case CTRL_SYNC:
        return RES_OK;
    case GET_SECTOR_COUNT:
        *((DWORD *)buff) = mmcsdGetCardCapacity(&SDCD1);
        return RES_OK;
    case GET_SECTOR_SIZE:
        *((WORD *)buff) = MMCSD_BLOCK_SIZE;
        return RES_OK;
    case GET_BLOCK_SIZE:
        *((DWORD *)buff) = 256; /* 512b blocks in one erase block */
        return RES_OK;
#if _USE_ERASE
    case CTRL_ERASE_SECTOR:
        sdcErase(&SDCD1, *((DWORD *)buff), *((DWORD *)buff + 1));
        return RES_OK;
#endif
    default:
        return RES_PARERR;
    }
#endif
  }
  return RES_PARERR;
}

#if HAL_USE_RTC
	extern RTCDriver RTCD1;

	DWORD get_fattime(void) {
	    RTCDateTime timespec;

	    rtcGetTime(&RTCD1, &timespec);
	    return rtcConvertDateTimeToFAT(&timespec);
	}
#else
	DWORD get_fattime(void) {
	    return ((uint32_t)0 | (1 << 16)) | (1 << 21); /* wrong but valid time */
	}
#endif

#endif // GFX_USE_GFILE && GFILE_NEED_FATFS && GFX_USE_OS_CHIBIOS && !GFILE_FATFS_EXTERNAL_LIB
