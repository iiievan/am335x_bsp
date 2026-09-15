#ifndef SD_BLOCKS_H
#define SD_BLOCKS_H

#ifdef __cplusplus
extern "C" {
#endif

/*  Bring the microSD slot of the BeagleBone Black up, identify the card and exercise the raw
 *  block interface: a read of the first sectors, then a non-destructive single block and multi
 *  block write/verify/restore round on a scratch area of the card.
 *
 *  Returns true when every step succeeded.
 */
bool sd_blocks_test(void);

#ifdef __cplusplus
}
#endif

#endif /* SD_BLOCKS_H */
