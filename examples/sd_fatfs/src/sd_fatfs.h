#ifndef SD_FATFS_H
#define SD_FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*  Mount the microSD card of the BeagleBone Black, report what the file system looks like, list the
 *  root directory, then write a small text file and read it back to prove the write path works.
 *
 *  Returns true when every step succeeded.
 */
bool sd_fatfs_test(void);

#ifdef __cplusplus
}
#endif

#endif /* SD_FATFS_H */
