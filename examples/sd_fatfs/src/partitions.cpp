#include "ff.h"

static_assert(FF_MULTI_PARTITION == 1, "This example requires explicit partition mapping");
static_assert(FF_VOLUMES == 2, "Update VolToPart when changing the number of volumes");

/* Both volumes use physical drive 0 (MMC0). Partition numbers are one-based
 * MBR table entries; these are NOT physical drive numbers.
 * Keep this C linkage: the table is consumed by the vendored ff.c.
 */
extern "C"
{
    PARTITION VolToPart[FF_VOLUMES] =
    {
        {0u, 1u},     // 0: -> MMC0, first primary MBR partition
        {0u, 2u}      // 1: -> MMC0, second primary MBR partition
    };
}
