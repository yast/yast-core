/**

  HwProbe.h

  Purpose:	class definition for HwProbe

  Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
  Maintainer:	Arvin Schnell <arvin@suse.de>

*/

// -*- c++ -*-

#ifndef HwProbe_h
#define HwProbe_h

#include <stdio.h>
#include <sys/types.h>

#include <hd.h>		// libhd

#include <scr/SCRAgent.h>

#include "Y2.h"

/**
 * @short SCR Agent for access to hardware autoprobe
 */
class HwProbe : public SCRAgent
{
public:
    /**
     * Creates a new HwProbe.
     */
    HwProbe();

    /**
     * Cleans up
     */
    ~HwProbe ();

    /**
     * Reads data. Destroy the result after use.
     * @param path Specifies what part of the subtree should
     * be read. The path is specified _relatively_ to Root()!
     */
    YCPValue Read(const YCPPath& path, const YCPValue& arg = YCPNull(), const YCPValue& optarg = YCPNull());

    /**
     * Writes data. Destroy the result after use.
     */
    YCPBoolean Write(const YCPPath& path, const YCPValue& value, const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees.
     */
    YCPList Dir(const YCPPath& path);

    // ----------------------------------------

private:
    /**
     * pointer to complete hd structure
     */

    hd_data_t *hd_base;

    /**
     * convert hd_data_t entry to value
     */
    YCPValue hd2value (hd_t *hd);

    /**
     * convert a hd_data_t linked list to a ycplist
     */
    YCPList hdlist2ycplist (hd_t *hd, hd_hw_item_t filteritem = (hd_hw_item_t)0);

    /**
     * convert res_t entry to map value
     */
    YCPMap resource_type2map (const res_any_t *res, const char **name);

    /**
     * convert driver_info_t entry to map value
     */
    YCPMap driver_info2map (const driver_info_t *drv, const char **name);

    /**
     * lookup hardware components matching item
     * return YCPValue
     */
    YCPValue byItem (hd_hw_item_t item, bool re_probe);

    /**
     * lookup 'manual' (configured) hardware components matching item
     * return YCPValue
     */
    YCPValue filterManual (hd_hw_item_t item);

    /**
     * lookup boot architecture
     * return YCPValue
     */
    YCPValue bootArch ();

    /**
     * lookup cpu architecture
     * return YCPValue
     */
    YCPValue cpuArch ();

    /**
     * lookup boot disk
     * return YCPValue
     */
    YCPValue bootDisk ();

    /**
     * lookup isdn hardware data
     * return YCPValue
     */
    YCPValue cdb_isdnData ();

    /**
     * lookup which video card is used by bios
     * return YCPValue
     */
    YCPValue biosVideo ();

    /**
     * finds out framebuffer data
     * return YCPValue
     */
    YCPValue vesaFramebuffer ();

    /**
     * check path for read, write, and dir
     * func == 0 for read, == 1 for write, == 2 for dir.
     */
    YCPValue checkPath (const YCPPath& path, const YCPValue& arg, const YCPValue& writeval, int func);

    /**
     * scan the hardware
     */
    int doScan (int force);
};


// helper macros to hide hd_data_t internals

#define hd_data_first(hddata) (hddata->hd)
#define hd_next(hd) (hd->next)

#endif // HwProbe_h
