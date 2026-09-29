#ifndef SD_FATFS_H
#define SD_FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Test both configured MMC0 partitions, independently. Existing files are
 * preserved: the example creates a numbered test file with FA_CREATE_NEW.
 * Returns true only if every configured partition passes, including unmount.
 */
bool sd_fatfs_test(void);

#ifdef __cplusplus
}
#endif

#endif /* SD_FATFS_H */
