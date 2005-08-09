/**

  HwParse.cc

  Purpose:	ycp to libhd interface, parse hd_t and construct YCPValueRep

  Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
  Maintainer:	Arvin Schnell <arvin@suse.de>

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "HwProbe.h"
#include <ycp/y2log.h>

/* ------------------------------------------------------------------------- */
/* First, some helper functions  */


// hd_cpu_arch_t -> string
//
static const char*
cpu2string (hd_cpu_arch_t cpu_arch)
{
    const char *s;
    switch (cpu_arch)
    {
	case arch_intel:   s = "i386"; break;
	case arch_alpha:   s = "alpha"; break;
	case arch_sparc:   s = "sparc"; break;
	case arch_sparc64: s = "sparc64"; break;
	case arch_ppc:     s = "ppc"; break;
	case arch_ppc64:   s = "ppc64"; break;
	case arch_68k:     s = "m68k"; break;
	case arch_s390:    s = "s390_32"; break;
	case arch_s390x:   s = "s390_64"; break;
	case arch_ia64:    s = "ia64"; break;
	case arch_mips:    s = "mips"; break;
	case arch_arm:     s = "arm"; break;
	case arch_x86_64:  s = "x86_64"; break;
	default: s = "unknown";
    }
    return s;
}


// hd_boot_arch_t -> string
//
static const char*
boot2string (hd_boot_arch_t boot_arch)
{
    const char *s;
    switch (boot_arch)
    {
	case boot_lilo:  s = "lilo"; break;
	case boot_grub:  s = "grub"; break;
	case boot_milo:  s = "milo"; break;
	case boot_aboot: s = "aboot"; break;
	case boot_silo:  s = "silo"; break;
	case boot_ppc:   s = "ppc"; break;
	case boot_elilo: s = "elilo"; break;
	case boot_s390:  s = "s390"; break;
	case boot_mips:  s = "mips"; break;
	default: s = "unknown";
    }
    return s;
}


// hd_hotplug_t -> string
//
static const char*
hotplug2string (hd_hotplug_t hotplug)
{
    const char* s;
    switch (hotplug)
    {
	case hp_none:     s = 0; break;
	case hp_pcmcia:   s = "pcmcia"; break;
	case hp_cardbus:  s = "cardbus"; break;
	case hp_pci:      s = "pci"; break;
	case hp_usb:      s = "usb"; break;
	case hp_ieee1394: s = "ieee1394"; break;
	default: s = "unknown";
    }
    return s;
}


// enum access_type to YCPString
//
static YCPString
access2string (unsigned int acc)
{
    const char *s;
    switch ((enum access_flags)acc)
    {
	case acc_unknown: s = "?"; break;
	case acc_ro:      s = "r"; break;
	case acc_wo:      s = "w"; break;
	case acc_rw:      s = "rw"; break;
	default: s = "";
    }
    return YCPString (s);
}


/**
 *  hd_dev_num_t -> YCPMap
 */
static YCPMap
devnum2map (hd_dev_num_t devnum)
{
    YCPMap m;

    switch (devnum.type)
    {
	case 'b':
	    m.add (YCPString ("type"), YCPString ("b"));
	    break;
	case 'c':
	    m.add (YCPString ("type"), YCPString ("c"));
	    break;
	default:
	    m.add (YCPString ("type"), YCPString (""));
	    break;
    }

    m.add (YCPString ("major"), YCPInteger (devnum.major));
    m.add (YCPString ("minor"), YCPInteger (devnum.minor));
    m.add (YCPString ("range"), YCPInteger (devnum.range));

    return m;
}


/**
 *  Add the hd_dev_num_t devnum to the YCPMap m with key k if devnum and it's
 *  type is non-zero.
 */
static void
add_devnum (const hd_dev_num_t* devnum, YCPMap* m, const char* k)
{
    if (devnum && devnum->type)
	m->add (YCPString (k), devnum2map (*devnum));
}


/**
 *  Add the char* str to the YCPMap m with key k if str in non-zero.
 */
static void
add_str (const char* str, YCPMap* m, const char* k)
{
    if (str)
	m->add (YCPString (k), YCPString (str));
}


/**
 *  Add the str_list_t* strlist to the YCPMap m with key k if strlist is
 *  non-zero.
 */
static void
add_strlist (const str_list_t* strlist, YCPMap* m, const char* k)
{
    if (strlist)
    {
	YCPList l;
	while (strlist)
	{
	    if (strlist->str)
	    {
		l->add (YCPString (strlist->str));
	    }
	    strlist = strlist->next;
	}
	m->add (YCPString (k), l);
    }
}


/* ------------------------------------------------------------------------- */
/* Now the member functions  */

// convert a libhd resource information to a YCPMap
//

YCPMap
HwProbe::resource_type2map (const res_any_t *res, const char **name)
{
    YCPMap map;

#define RES2TYPE(t) t *r = (t *)res
    switch (res->type)
    {
	case res_any:
	{
	    *name = "any";
	}
	  break;
	case res_mem: {
	  RES2TYPE (res_mem_t);
	  *name = "mem";
	  map->add (YCPString ("start"), YCPInteger (r->base));
	  if (r->range > 0)
	      map->add (YCPString ("length"), YCPInteger (r->range));
	  map->add (YCPString ("active"), YCPBoolean ((r->enabled)?true:false));
	}
	break;
    case res_phys_mem: {
	RES2TYPE (res_phys_mem_t);
	*name = "phys_mem";
	if (r->range > 0)
	map->add (YCPString ("range"), YCPInteger (r->range));
    }
    break;
    case res_io: {
	RES2TYPE (res_io_t);
	*name = "io";
	map->add (YCPString ("start"), YCPInteger (r->base));
	map->add (YCPString ("length"), YCPInteger (r->range));
	map->add (YCPString ("mode"), access2string (r->access));
	map->add (YCPString ("active"),	YCPBoolean ((r->enabled)?true:false));
    }
    break;
    case res_irq: {
	RES2TYPE (res_irq_t);
	*name = "irq";
	map->add (YCPString ("irq"), YCPInteger (r->base));
	map->add (YCPString ("count"), YCPInteger (r->triggered));
	map->add (YCPString ("enabled"),	YCPBoolean ((r->enabled)?true:false));
    }
    break;
    case res_dma: {
	RES2TYPE (res_dma_t);
	*name = "dma";
	map->add (YCPString ("channel"), YCPInteger (r->base));
	map->add (YCPString ("enabled"),	YCPBoolean ((r->enabled)?true:false));
    }
    break;
    case res_monitor: {
	RES2TYPE (res_monitor_t);
	if (r->interlaced == 0)
	{
	*name = "monitor_resol";
	map->add (YCPString ("width"), YCPInteger (r->width));
	map->add (YCPString ("height"), YCPInteger (r->height));
	map->add (YCPString ("vfreq"), YCPInteger (r->vfreq));
	}
    }
    break;
    case res_size: {
	RES2TYPE (res_size_t);
	*name = "size";
	const char *s;
	switch (r->unit) {
	case size_unit_cm:	  s = "cm"; break;
	case size_unit_cinch:	  s = "cinch"; break;
	case size_unit_byte:	  s = "byte"; break;
	case size_unit_sectors:	  s = "sectors"; break;
	case size_unit_kbyte:	  s = "kb"; break;
	case size_unit_mbyte:	  s = "mb"; break;
	case size_unit_gbyte:	  s = "gb"; break;
	default: s = 0;	  break;
	}
	if (s != 0) {
	  map->add (YCPString ("unit"), YCPString (s));
	  map->add (YCPString ("x"), YCPInteger (r->val1));
	  map->add (YCPString ("y"), YCPInteger (r->val2));
	}
    }
    break;
    case res_disk_geo: {
	RES2TYPE (res_disk_geo_t);
	if (r->geotype == 0 || r->geotype == 1) {
	    *name = (r->geotype == 1) ? "disk_log_geo" : "disk_phys_geo";
	    map->add (YCPString ("cylinders"), YCPInteger (r->cyls));
	    map->add (YCPString ("heads"), YCPInteger (r->heads));
	    map->add (YCPString ("sectors"), YCPInteger (r->sectors));
	}
    }
    break;
    case res_cache: {
	RES2TYPE (res_cache_t);
	*name = "cache";
	map->add (YCPString ("size"), YCPInteger (r->size));
    }
    break;
    case res_baud: {
	RES2TYPE (res_baud_t);
	*name = "baud";
	map->add (YCPString ("speed"), YCPInteger (r->speed));
    }
    break;
    case res_init_strings: {
	RES2TYPE (res_init_strings_t);
	*name = "init_strings";
	if (r->init1 != 0)
	    map->add (YCPString ("init1"), YCPString (r->init1));
	if (r->init2 != 0)
	    map->add (YCPString ("init2"), YCPString (r->init2));
    }
    break;
    case res_pppd_option: {
	RES2TYPE (res_pppd_option_t);
	*name = "pppd_option";
	if (r->option != 0)
	    map->add (YCPString ("option"), YCPString (r->option));
    }
    break;
    case res_framebuffer:
    {
	RES2TYPE (res_framebuffer_t);
	*name = "framebuffer";
	map->add (YCPString ("width"), YCPInteger (r->width));
	map->add (YCPString ("height"), YCPInteger (r->height));
	map->add (YCPString ("color"), YCPInteger (r->colorbits));
	map->add (YCPString ("mode"), YCPInteger (r->mode));
    }
    break;
    case res_hwaddr:
    {
	RES2TYPE (res_hwaddr_t);
	*name = "hwaddr";
	map->add (YCPString ("addr"), YCPString (r->addr));
    }
    break;
    case res_link:
    {
	RES2TYPE (res_link_t);
	*name = "link";
	map->add (YCPString ("state"), YCPBoolean (r->state));
    }
    break;
    case res_wlan:
    {
	RES2TYPE (res_wlan_t);
	*name = "wlan";
	add_strlist (r->channels,	&map, "channels");
	add_strlist (r->frequencies,	&map, "frequencies");
	add_strlist (r->bitrates,	&map, "bitrates");
	add_strlist (r->auth_modes,	&map, "auth_modes");
	add_strlist (r->enc_modes,	&map, "enc_modes");
    }
    break;
    default:
	map = YCPNull();
	break;
    }
    if (!map.isNull() && map->size() == 0) {
	map = YCPNull();
    }
    return map;
#undef RES2TYPE
}


/**
 *  Adds an entry to the map containing the key as key and the strlist
 *  and value. Does nothing if strlist is 0.
 */
static void
strlist2ycplist (const str_list_t* strlist, YCPMap *map, const char* key)
{
    if (!strlist)
	return;

    YCPList ycplist;

    while (strlist)
    {
	if (strlist->str)
	{
	    ycplist->add (YCPString (strlist->str));
	}
	strlist = strlist->next;
    }

    (*map)->add (YCPString (key), ycplist);
}


/**
 * convert driver_info_t entry to map value
 */

YCPMap
HwProbe::driver_info2map (const driver_info_t *drvinfo, const char **name)
{
    YCPMap map;

#define DRV2TYPE(t) t *d = (t *)drvinfo

    switch (drvinfo->any.type)
    {
	case di_module:
	{
	    DRV2TYPE (driver_info_module_t);
	    *name = "drivers";

	    // prepare a list of [ [<modname>, <modargs>], [...], ... ]
	    // for the list of modules and arguments coming from libhd

	    YCPList modules;

	    str_list_t *nptr, *aptr;
	    nptr = d->names;
	    aptr = d->mod_args;
	    while (nptr != 0 && aptr != 0)
	    {
		if (nptr->str)
		{
		    YCPList a_module;
		    a_module->add (YCPString (nptr->str));
		    a_module->add (YCPString (aptr->str?aptr->str:""));
		    modules->add (a_module);
		}
		nptr = nptr->next;
		aptr = aptr->next;
	    }
	    if (modules->size() > 0)
	    {
		map->add (YCPString ("modules"), modules);
	    }
	    map->add (YCPString ("active"), YCPBoolean (d->active?true:false));
	    map->add (YCPString ("modprobe"), YCPBoolean (d->modprobe?true:false));
	    if (d->conf)
	    {
		map->add (YCPString ("conf"), YCPString (d->conf));
	    }
	}
	break;

	case di_mouse:
	{
	    DRV2TYPE (driver_info_mouse_t);
	    *name = "mouse";
	    if (d->buttons > 0)
	    {
		map->add (YCPString ("buttons"), YCPInteger (d->buttons));
	    }
	    if (d->wheels > 0)
	    {
		map->add (YCPString ("wheels"), YCPInteger (d->wheels));
	    }
	    if (d->xf86)
	    {
		map->add (YCPString ("xf86"), YCPString (d->xf86));
	    }
	    if (d->gpm)
	    {
		map->add (YCPString ("gpm"), YCPString (d->gpm));
	    }
	}
	break;

	case di_x11:
	{
	    DRV2TYPE (driver_info_x11_t);
	    *name = "x11";
	    if (d->server)
	    {
		map->add (YCPString ("server"), YCPString (d->server));
	    }
	    if (d->xf86_ver)
	    {
		map->add (YCPString ("version"), YCPString (d->xf86_ver));
	    }
	    if (d->x3d)
	    {
		map->add (YCPString ("has_3d"), YCPBoolean(true));
	    }
	    if (d->colors.all != 0)
	    {
		map->add (YCPString ("c8"), YCPBoolean (d->colors.c8 ? true : false));
		map->add (YCPString ("c15"), YCPBoolean (d->colors.c15 ? true : false));
		map->add (YCPString ("c16"), YCPBoolean (d->colors.c16 ? true : false));
		map->add (YCPString ("c24"), YCPBoolean (d->colors.c24 ? true : false));
		map->add (YCPString ("c32"), YCPBoolean (d->colors.c32 ? true : false));
	    }
	    if (d->dacspeed > 0)
	    {
		map->add (YCPString ("dacspeed"), YCPInteger (d->dacspeed));
	    }

	    strlist2ycplist (d->extensions, &map, "extensions");
	    strlist2ycplist (d->options, &map, "options");
	    strlist2ycplist (d->raw, &map, "raw");
	    if (d->script && *(d->script))
	    {
		map->add (YCPString ("script3d"), YCPString (d->script));
	    }
	}
	break;

	case di_display:
	{
	    DRV2TYPE (driver_info_display_t);
	    *name = "display";
	    map->add (YCPString ("width"), YCPInteger (d->width));
	    map->add (YCPString ("height"), YCPInteger (d->height));
	    map->add (YCPString ("min_vsync"), YCPInteger (d->min_vsync));
	    map->add (YCPString ("max_vsync"), YCPInteger (d->max_vsync));
	    map->add (YCPString ("min_hsync"), YCPInteger (d->min_hsync));
	    map->add (YCPString ("max_hsync"), YCPInteger (d->max_hsync));
	    map->add (YCPString ("bandwidth"), YCPInteger (d->bandwidth));
	}
	break;

	case di_isdn:
	{
	    DRV2TYPE (driver_info_isdn_t);
	    *name = "isdn";
	    map->add (YCPString ("type"), YCPInteger (d->i4l_type));
	    map->add (YCPString ("subtype"), YCPInteger (d->i4l_subtype));
	    if (d->i4l_name)
	    {
		map->add (YCPString ("name"), YCPString (d->i4l_name));
	    }
	    if (d->params != 0)
	    {
		YCPMap pMap;
		isdn_parm_t *ip = d->params;
		while (ip)
		{
		    if (ip->valid)
		    {
			YCPList pList;
			if (1)
			{
			    int i;
			    unsigned int *vp = ip->alt_value;

			    pList->add (YCPInteger (ip->value));
			    i = ip->alt_values;
			    while (i-- > 0)
			    {
				pList->add (YCPInteger (*vp));
				vp++;
			    }
			    pMap->add (YCPString(ip->name), pList);
			    if (ip->conflict)
			    {
				pMap->add (YCPString ("conflict"), YCPBoolean (true));
			    }
			}
		    }
		    ip = ip->next;
		}
		map->add (YCPString ("parameter"), pMap);
	    }
	}
	break;

	case di_dsl:
	{
	    DRV2TYPE (driver_info_dsl_t);
	    *name = "dsl";
	    map->add (YCPString ("mode"), YCPString (d->mode));
	    map->add (YCPString ("name"), YCPString (d->name));
	}
	break;

	case di_kbd:
	{
	    DRV2TYPE (driver_info_kbd_t);
	    *name = "keyboard";
	    if (d->XkbRules)
	    {
		map->add (YCPString ("xkbrules"), YCPString (d->XkbRules));
	    }
	    if (d->XkbModel)
	    {
		map->add (YCPString ("xkbmodel"), YCPString (d->XkbModel));
	    }
	    if (d->XkbLayout)
	    {
		map->add (YCPString ("xkblayout"), YCPString (d->XkbLayout));
	    }
	    if (d->keymap)
	    {
		map->add (YCPString ("keymap"), YCPString (d->keymap));
	    }
	}
	break;

	default:
	break;

    } /* switch ()  */

#undef DRV2TYPE
    return map;
}


YCPValue
HwProbe::hd2value (hd_t *hd)
{
    YCPMap out;
    const char* s;

    // we're just probing this hardware, so the user is running yast2
    // and configuring this hardware.

    if (hd->status.configured == status_new)
	hd->status.configured = status_no;

    hd_write_config (hd_base, hd);

    if (hd->broken)
    {
	out->add (YCPString ("broken"), YCPBoolean (true));
    }

    // cardtype

    if (hd->is.agp)
    {
	out->add (YCPString ("cardtype"), YCPString ("AGP"));
    }
    else if (hd->is.isapnp)
    {
	out->add (YCPString ("cardtype"), YCPString ("PnP"));
    }

    // misc entries in is structure

    if (hd->is.notready)
    {
	out->add (YCPString ("notready"), YCPBoolean (true));
    }

    if (hd->is.manual)
    {
	out->add (YCPString ("manual"), YCPBoolean (true));
    }

    if (hd->is.softraiddisk)
    {
	out->add (YCPString ("softraiddisk"), YCPBoolean (true));
    }

    if (hd->is.zip)
    {
	out->add (YCPString ("zip"), YCPBoolean (true));
    }

    if (hd->is.cdr)
    {
	out->add (YCPString ("cdr"), YCPBoolean (true));
    }

    if (hd->is.cdrw)
    {
	out->add (YCPString ("cdrw"), YCPBoolean (true));
    }

    if (hd->is.dvd)
    {
	out->add (YCPString ("dvd"), YCPBoolean (true));
    }

    if (hd->is.dvdr)
    {
	out->add (YCPString ("dvdr"), YCPBoolean (true));
    }

    if (hd->is.dvdram)
    {
	out->add (YCPString ("dvdram"), YCPBoolean (true));
    }

    if (hd->is.pppoe)
    {
	out->add (YCPString ("pppoe"), YCPBoolean (true));
    }

    if (hd->is.wlan)
    {
	out->add (YCPString ("wlan"), YCPBoolean (true));
    }

    // hd detail

    if (hd->detail)
    {
	switch (hd->detail->type)
	{
	    case hd_detail_bios: {
		bios_info_t *info = hd->detail->bios.data;
		out->add (YCPString ("apm_supported"), YCPBoolean ((info->apm_supported)?true:false));
		out->add (YCPString ("apm_enabled"), YCPBoolean ((info->apm_enabled)?true:false));
		out->add (YCPString ("apm_version"), YCPInteger ((info->apm_ver)));
		out->add (YCPString ("apm_subversion"), YCPInteger ((info->apm_subver)));
		out->add (YCPString ("apm_biosflags"), YCPInteger ((info->apm_bios_flags)));
		out->add (YCPString ("is_pnp_bios"), YCPBoolean ((info->is_pnp_bios)));
		out->add (YCPString ("lba_support"), YCPBoolean ((info->lba_support)));

		// add SMBIOS info if it is present
		if (hd_base->smbios)
		{
		    // result is a list (there can be multiple components (e.g.
		    // processor, card slot) present in the system)
		    YCPList smbioslist;
		    hd_smbios_t *sm;

		    // cycle through all smbios entries
		    for(sm = hd_base->smbios; sm; sm = sm->next) {

			if (sm->any.type == sm_biosinfo)
			{
			    YCPMap biosinfo;

			    if (sm->biosinfo.vendor) biosinfo->add(YCPString("vendor"), YCPString(sm->biosinfo.vendor));
			    if (sm->biosinfo.version) biosinfo->add(YCPString("version"), YCPString(sm->biosinfo.version));
			    if (sm->biosinfo.date) biosinfo->add(YCPString("date"), YCPString(sm->biosinfo.date));
			    biosinfo->add(YCPString("start_address"), YCPInteger(sm->biosinfo.start));
			    biosinfo->add(YCPString("rom_size"), YCPInteger(sm->biosinfo.rom_size));
			    biosinfo->add(YCPString("type"), YCPString("biosinfo"));

			    smbioslist->add(biosinfo);
			}

			if (sm->any.type == sm_sysinfo) {
			    YCPMap sysinfo;

			    if (sm->sysinfo.manuf) sysinfo->add(YCPString("manufacturer"), YCPString(sm->sysinfo.manuf));
			    if (sm->sysinfo.product) sysinfo->add(YCPString("product"), YCPString(sm->sysinfo.product));
			    if (sm->sysinfo.version) sysinfo->add(YCPString("version"), YCPString(sm->sysinfo.version));
			    if (sm->sysinfo.serial) sysinfo->add(YCPString("serial"), YCPString(sm->sysinfo.serial));
			    if (sm->sysinfo.wake_up.name) sysinfo->add(YCPString("wake_up"), YCPString(sm->sysinfo.wake_up.name));
			    if (sm->sysinfo.wake_up.id) sysinfo->add(YCPString("wake_up_id"), YCPInteger(sm->sysinfo.wake_up.id));
			    sysinfo->add(YCPString("type"), YCPString("sysinfo"));

			    smbioslist->add(sysinfo);
			}

			if (sm->any.type == sm_boardinfo)
			{
			    YCPMap boardinfo;

			    if (sm->boardinfo.manuf) boardinfo->add(YCPString("manufacturer"), YCPString(sm->boardinfo.manuf));
			    if (sm->boardinfo.product) boardinfo->add(YCPString("product"), YCPString(sm->boardinfo.product));
			    if (sm->boardinfo.version) boardinfo->add(YCPString("version"), YCPString(sm->boardinfo.version));
			    if (sm->boardinfo.serial) boardinfo->add(YCPString("serial"), YCPString(sm->boardinfo.serial));
			    if (sm->boardinfo.asset) boardinfo->add(YCPString("asset_tag"), YCPString(sm->boardinfo.asset));
			    if (sm->boardinfo.location) boardinfo->add(YCPString("location"), YCPString(sm->boardinfo.location));
			    boardinfo->add(YCPString("type"), YCPString("boardinfo"));

			    smbioslist->add(boardinfo);
			}

			if (sm->any.type == sm_chassis)
			{
			    YCPMap chassis;

			    if (sm->chassis.manuf) chassis->add(YCPString("manufacturer"), YCPString(sm->chassis.manuf));
			    if (sm->chassis.version) chassis->add(YCPString("version"), YCPString(sm->chassis.version));
			    if (sm->chassis.serial) chassis->add(YCPString("serial"), YCPString(sm->chassis.serial));
			    if (sm->chassis.asset) chassis->add(YCPString("asset_tag"), YCPString(sm->chassis.asset));
			    if (sm->chassis.lock) chassis->add(YCPString("lock_present"), YCPBoolean(true));
			    if (sm->chassis.oem) chassis->add(YCPString("oem_info"), YCPInteger(sm->chassis.oem));
			    if (sm->chassis.ch_type.name) chassis->add(YCPString("chassis_type"), YCPString(sm->chassis.ch_type.name));
			    if (sm->chassis.ch_type.id) chassis->add(YCPString("chassis_type_id"), YCPInteger(sm->chassis.ch_type.id));
			    chassis->add(YCPString("type"), YCPString("chassis"));

			    smbioslist->add(chassis);
			}

			if (sm->any.type == sm_processor)
			{
			    YCPMap processor;

			    if (sm->processor.socket) processor->add(YCPString("socket"), YCPString(sm->processor.socket));
			    if (sm->processor.upgrade.name) processor->add(YCPString("socket_type"), YCPString(sm->processor.upgrade.name));
			    processor->add(YCPString("socket_status"), YCPString(sm->processor.sock_status ? "Populated" : "Empty"));
			    if (sm->processor.pr_type.name) processor->add(YCPString("processor_type"), YCPString(sm->processor.pr_type.name));
			    if (sm->processor.family.name) processor->add(YCPString("family"), YCPString(sm->processor.family.name));
			    if (sm->processor.manuf) processor->add(YCPString("manufacturer"), YCPString(sm->processor.manuf));
			    if (sm->processor.version) processor->add(YCPString("version"), YCPString(sm->processor.version));
			    if (sm->processor.serial) processor->add(YCPString("serial"), YCPString(sm->processor.serial));
			    if (sm->processor.asset) processor->add(YCPString("asset_tag"), YCPString(sm->processor.asset));
			    if (sm->processor.part) processor->add(YCPString("part_number"), YCPString(sm->processor.part));
			    if (sm->processor.cpu_id) processor->add(YCPString("pocessor_id"), YCPInteger(sm->processor.cpu_id));
			    if (sm->processor.cpu_status.name) processor->add(YCPString("cpu_status"), YCPString(sm->processor.cpu_status.name));
			    if (sm->processor.voltage) processor->add(YCPString("voltage"), YCPFloat(float(sm->processor.voltage) / 10.0));
			    if (sm->processor.ext_clock) processor->add(YCPString("ext_clock"), YCPInteger(sm->processor.ext_clock));
			    if (sm->processor.max_speed) processor->add(YCPString("max_speed"), YCPInteger(sm->processor.max_speed));
			    if (sm->processor.current_speed) processor->add(YCPString("current_speed"), YCPInteger(sm->processor.current_speed));
			    if (sm->processor.l1_cache) processor->add(YCPString("L1_cache_idx"), YCPInteger(sm->processor.l1_cache));
			    if (sm->processor.l2_cache) processor->add(YCPString("L2_cache_idx"), YCPInteger(sm->processor.l2_cache));
			    if (sm->processor.l3_cache) processor->add(YCPString("L3_cache_idx"), YCPInteger(sm->processor.l3_cache));
			    processor->add(YCPString("type"), YCPString("processor"));

			    smbioslist->add(processor);
			}

			if (sm->any.type == sm_cache)
			{
			    YCPMap cache;

			    if (sm->cache.socket) cache->add(YCPString("designation"), YCPString(sm->cache.socket));
			    cache->add(YCPString("level"), YCPInteger(sm->cache.level + 1));
			    cache->add(YCPString("state"), YCPString(sm->cache.state ? "Enabled" : "Disabled"));
			    if (sm->cache.mode.name) cache->add(YCPString("mode"), YCPString(sm->cache.mode.name));

			    if (sm->cache.location.name) {
				cache->add(YCPString("location_id"), YCPInteger(sm->cache.location.id));
				cache->add(YCPString("location"), YCPString(sm->cache.location.name));
				cache->add(YCPString("socketed"), YCPBoolean(sm->cache.socketed));
			    }

			    if (sm->cache.ecc.name) cache->add(YCPString("ecc"), YCPString(sm->cache.ecc.name));
			    if (sm->cache.cache_type.name) cache->add(YCPString("cache_type"), YCPString(sm->cache.cache_type.name));
			    if (sm->cache.assoc.name) cache->add(YCPString("associativity"), YCPString(sm->cache.assoc.name));
			    if (sm->cache.max_size) cache->add(YCPString("max_size"), YCPInteger(sm->cache.max_size));
			    if (sm->cache.current_size) cache->add(YCPString("current_size"), YCPInteger(sm->cache.current_size));
			    if (sm->cache.speed) cache->add(YCPString("speed"), YCPInteger(sm->cache.speed));

			    cache->add(YCPString("type"), YCPString("cache"));

			    smbioslist->add(cache);
			}

			if (sm->any.type == sm_connect)
			{
			    YCPMap connect;

			    if (sm->connect.port_type.name) connect->add(YCPString("port_type"), YCPString(sm->connect.port_type.name));
			    if (sm->connect.i_des) connect->add(YCPString("internal_designator"), YCPString(sm->connect.i_des));
			    if (sm->connect.x_des) connect->add(YCPString("external_designator"), YCPString(sm->connect.x_des));
			    if (sm->connect.i_type.name) connect->add(YCPString("internal_connector"), YCPString(sm->connect.i_type.name));
			    if (sm->connect.x_type.name) connect->add(YCPString("external_connector"), YCPString(sm->connect.x_type.name));
			    connect->add(YCPString("type"), YCPString("port_connector"));

			    smbioslist->add(connect);
			}

			if (sm->any.type == sm_slot)
			{
			    YCPMap sslot;

			    if (sm->slot.desig) sslot->add(YCPString("designation"), YCPString(sm->slot.desig));
			    if (sm->slot.slot_type.name) sslot->add(YCPString("slot_type"), YCPString(sm->slot.slot_type.name));
			    if (sm->slot.bus_width.name) sslot->add(YCPString("bus_width"), YCPString(sm->slot.bus_width.name));
			    if (sm->slot.usage.name) sslot->add(YCPString("status"), YCPString(sm->slot.usage.name));
			    if (sm->slot.length.name) sslot->add(YCPString("length"), YCPString(sm->slot.length.name));
			    sslot->add(YCPString("slot_id"), YCPInteger(sm->slot.id));
			    sslot->add(YCPString("type"), YCPString("system_slot"));

			    smbioslist->add(sslot);
			}

			if (sm->any.type == sm_onboard)
			{
			    YCPList onboard_devices;

			    for (unsigned int u = 0; u < sm->onboard.dev_len; u++) {
				YCPMap onboard_device;

				if (sm->onboard.dev[u].type.name) onboard_device->add(YCPString("device_type"), YCPString(sm->onboard.dev[u].type.name));
				if (sm->onboard.dev[u].name) onboard_device->add(YCPString("name"), YCPString(sm->onboard.dev[u].name));
				onboard_device->add(YCPString("enabled"), YCPBoolean(sm->onboard.dev[u].status));

				onboard_devices->add(onboard_device);
			    }

			    YCPMap onboard;
			    onboard->add(YCPString("devices"), onboard_devices);
			    onboard->add(YCPString("type"), YCPString("onboard_devices"));

			    smbioslist->add(onboard);
			}

			if (sm->any.type == sm_oem)
			{
			    YCPList oem_strings;
			    for (str_list_t *sl = sm->oem.oem_strings; sl; sl = sl->next) {
				if (sl->str) oem_strings->add(YCPString(sl->str));
			    }

			    YCPMap oemstr;
			    oemstr->add(YCPString("oem_strings"), oem_strings);
			    oemstr->add(YCPString("type"), YCPString("oem_strings"));

			    smbioslist->add(oemstr);
			}

			if (sm->any.type == sm_config)
			{
			    YCPList config;

			    for (str_list_t *sl = sm->config.options; sl; sl = sl->next) {
				if (sl->str) config->add(YCPString(sl->str));
			    }

			    YCPMap conf;
			    conf->add(YCPString("config_options"), config);
			    conf->add(YCPString("type"), YCPString("config_options"));

			    smbioslist->add(conf);
			}

			if (sm->any.type == sm_lang)
			{
			    YCPList langs;
			    YCPMap language;
			    str_list_t *sl;

			    if ((sl = sm->lang.strings)) {
				for(; sl; sl = sl->next) {
				  if (sl->str) langs->add(YCPString(sl->str));
				}

				language->add(YCPString("languages"), langs);
			    }

			    if (sm->lang.current) language->add(YCPString("current"), YCPString(sm->lang.current));
			    language->add(YCPString("type"), YCPString("language_info"));

			    smbioslist->add(language);
			}

			if (sm->any.type == sm_group)
			{
			    YCPMap assoc;

			    if (sm->group.name) assoc->add(YCPString("group_name"), YCPString(sm->group.name));

			    if (sm->group.items_len) {
				YCPList items;

				for (int i = 0; i < sm->group.items_len; i++) {
				    items->add(YCPInteger(sm->group.item_handles[i]));
				}

				assoc->add(YCPString("items"), items);
			    }
			    assoc->add(YCPString("type"), YCPString("group_associations"));

			    smbioslist->add(assoc);
			}

			if (sm->any.type == sm_memarray)
			{
			    YCPMap memarray;

			    if (sm->memarray.use.name) memarray->add(YCPString("use"), YCPString(sm->memarray.use.name));
			    if (sm->memarray.location.name) memarray->add(YCPString("location"), YCPString(sm->memarray.location.name));
			    memarray->add(YCPString("slots"), YCPInteger(sm->memarray.slots));
			    if (sm->memarray.max_size) memarray->add(YCPString("max_size"), YCPInteger(sm->memarray.max_size));
			    if (sm->memarray.ecc.name) memarray->add(YCPString("ecc"), YCPString(sm->memarray.ecc.name));

			    if (sm->memarray.error_handle != 0xfffe ) {
				if (sm->memarray.error_handle != 0xffff) {
				    memarray->add(YCPString("error_handle"), YCPInteger(sm->memarray.error_handle));
				}
			    }
			    memarray->add(YCPString("type"), YCPString("memarray"));

			    smbioslist->add(memarray);
			}

			if (sm->any.type == sm_memdevice)
			{
			    YCPMap memdevice;

			    if (sm->memdevice.location) memdevice->add(YCPString("location"), YCPString(sm->memdevice.location));
			    if (sm->memdevice.bank) memdevice->add(YCPString("bank"), YCPString(sm->memdevice.bank));
			    if (sm->memdevice.manuf) memdevice->add(YCPString("manufacturer"), YCPString(sm->memdevice.manuf));
			    if (sm->memdevice.serial) memdevice->add(YCPString("serial"), YCPString(sm->memdevice.serial));
			    if (sm->memdevice.asset) memdevice->add(YCPString("asset_tag"), YCPString(sm->memdevice.asset));
			    if (sm->memdevice.part) memdevice->add(YCPString("part_number"), YCPString(sm->memdevice.part));
			    memdevice->add(YCPString("memory_array"), YCPInteger(sm->memdevice.array_handle));

			    if (sm->memdevice.error_handle != 0xfffe) {
				if (sm->memdevice.error_handle != 0xffff) {
				    memdevice->add(YCPString("error_handle"), YCPInteger(sm->memdevice.error_handle));
				}
			    }

			    if (sm->memdevice.form.name) memdevice->add(YCPString("form_factor"), YCPString(sm->memdevice.form.name));
			    if (sm->memdevice.mem_type.name) memdevice->add(YCPString("mem_type"), YCPString(sm->memdevice.mem_type.name));
			    memdevice->add(YCPString("width"), YCPInteger(sm->memdevice.width));
			    if (sm->memdevice.eccbits) memdevice->add(YCPString("eccbits"), YCPInteger(sm->memdevice.eccbits));
			    memdevice->add(YCPString("size"), YCPInteger(sm->memdevice.size));
			    if (sm->memdevice.speed) memdevice->add(YCPString("speed"), YCPInteger(sm->memdevice.speed));
			    memdevice->add(YCPString("type"), YCPString("memdevice"));

			    smbioslist->add(memdevice);
			}

			if (sm->any.type == sm_memerror)
			{
			    YCPMap memerror;

			    if (sm->memerror.err_type.name) memerror->add(YCPString("err_type"), YCPString(sm->memerror.err_type.name));
			    if (sm->memerror.granularity.name) memerror->add(YCPString("granularity"), YCPString(sm->memerror.granularity.name));
			    if (sm->memerror.operation.name) memerror->add(YCPString("operation"), YCPString(sm->memerror.operation.name));
			    if (sm->memerror.syndrome) memerror->add(YCPString("syndrome"), YCPInteger(sm->memerror.syndrome));

			    if (sm->memerror.array_addr != (1U << 31)) memerror->add(YCPString("array_addr"), YCPInteger(sm->memerror.array_addr));
			    if (sm->memerror.device_addr != (1U << 31)) memerror->add(YCPString("device_addr"), YCPInteger(sm->memerror.device_addr));
			    if (sm->memerror.range != (1U << 31)) memerror->add(YCPString("range"), YCPInteger(sm->memerror.range));
			    memerror->add(YCPString("type"), YCPString("memerror"));

			    smbioslist->add(memerror);
			}

			if (sm->any.type == sm_memarraymap)
			{
			    YCPMap memarraymap;

			    memarraymap->add(YCPString("array_handle"), YCPInteger(sm->memarraymap.array_handle));
			    memarraymap->add(YCPString("part_width"), YCPInteger(sm->memarraymap.part_width));

			    memarraymap->add(YCPString("start_addr"), YCPInteger(sm->memarraymap.start_addr));
			    memarraymap->add(YCPString("end_addr"), YCPInteger(sm->memarraymap.end_addr));
			    memarraymap->add(YCPString("type"), YCPString("memarraymap"));

			    smbioslist->add(memarraymap);
			}

			if (sm->any.type == sm_memdevicemap)
			{
			    YCPMap memdevicemap;

			    memdevicemap->add(YCPString("memdevice_handle"), YCPInteger(sm->memdevicemap.memdevice_handle));
			    memdevicemap->add(YCPString("arraymap_handle"), YCPInteger(sm->memdevicemap.arraymap_handle));

			    if (sm->memdevicemap.row_pos != 0xff) memdevicemap->add(YCPString("row"), YCPInteger(sm->memdevicemap.row_pos));

			    if (!sm->memdevicemap.interleave_pos || sm->memdevicemap.interleave_pos != 0xff) {
				memdevicemap->add(YCPString("interleave_pos"), YCPInteger(sm->memdevicemap.interleave_pos));
			    }

			    if(!sm->memdevicemap.interleave_depth || sm->memdevicemap.interleave_depth != 0xff) {
				memdevicemap->add(YCPString("interleave_depth"), YCPInteger(sm->memdevicemap.interleave_depth));
			    }

			    memdevicemap->add(YCPString("start_addr"), YCPInteger(sm->memdevicemap.start_addr));
			    memdevicemap->add(YCPString("end_addr"), YCPInteger(sm->memdevicemap.end_addr));
			    memdevicemap->add(YCPString("type"), YCPString("memdevicemap"));

			    smbioslist->add(memdevicemap);
			}

			if (sm->any.type == sm_mouse)
			{
			    YCPMap mouse;

			    if (sm->mouse.mtype.name) mouse->add(YCPString("mtype"), YCPString(sm->mouse.mtype.name));
			    if (sm->mouse.interface.name) mouse->add(YCPString("interface"), YCPString(sm->mouse.interface.name));
			    if (sm->mouse.buttons) mouse->add(YCPString("buttons"), YCPInteger(sm->mouse.buttons));
			    mouse->add(YCPString("type"), YCPString("mouse"));

			    smbioslist->add(mouse);
			}

			if (sm->any.type == sm_secure)
			{
			    YCPMap secure;

			    if (sm->secure.power.name) secure->add(YCPString("power"), YCPString(sm->secure.power.name));
			    if (sm->secure.keyboard.name) secure->add(YCPString("keyboard"), YCPString(sm->secure.keyboard.name));
			    if (sm->secure.admin.name) secure->add(YCPString("admin"), YCPString(sm->secure.admin.name));
			    if (sm->secure.reset.name) secure->add(YCPString("reset"), YCPString(sm->secure.reset.name));
			    secure->add(YCPString("type"), YCPString("secure"));

			    smbioslist->add(secure);
			}

			if (sm->any.type == sm_power)
			{
			    YCPMap power;

			    power->add(YCPString("hour"), YCPInteger(sm->power.hour));
			    power->add(YCPString("minute"), YCPInteger(sm->power.minute));
			    power->add(YCPString("second"), YCPInteger(sm->power.second));
			    power->add(YCPString("day"), YCPInteger(sm->power.day));
			    power->add(YCPString("month"), YCPInteger(sm->power.month));
			    power->add(YCPString("type"), YCPString("power"));

			    smbioslist->add(power);
			}

			if (sm->any.type == sm_mem64error)
			{
			    YCPMap mem64error;

			    if (sm->mem64error.err_type.name) mem64error->add(YCPString("err_type"), YCPString(sm->mem64error.err_type.name));
			    if (sm->mem64error.granularity.name) mem64error->add(YCPString("granularity"), YCPString(sm->mem64error.granularity.name));
			    if (sm->mem64error.operation.name) mem64error->add(YCPString("operation"), YCPString(sm->mem64error.operation.name));
			    if (sm->mem64error.syndrome) mem64error->add(YCPString("syndrome"), YCPInteger(sm->mem64error.syndrome));

			    if (sm->mem64error.array_addr != (1llu << 63) && sm->mem64error.array_addr != (1ll << 31)) {
				mem64error->add(YCPString("array_addr"), YCPInteger(sm->mem64error.array_addr));
			    }

			    if (sm->mem64error.device_addr != (1llu << 63) && sm->mem64error.device_addr != (1ll << 31)) {
				mem64error->add(YCPString("device_addr"), YCPInteger(sm->mem64error.device_addr));
			    }

			    if (sm->mem64error.range != (1u << 31)) mem64error->add(YCPString("range"), YCPInteger(sm->mem64error.range));
			    mem64error->add(YCPString("type"), YCPString("mem64error"));

			    smbioslist->add(mem64error);
			}

			// unknown type
			if (sm->any.type != sm_end)
			{
			    YCPMap unknown;

			    unknown->add(YCPString("type"), YCPString(sm->any.type == sm_inactive ? "inactive" : "unknown"));
			    unknown->add(YCPString("type_id"), YCPInteger(sm->any.type));

			    smbioslist->add(unknown);
			}
		    }

		    out->add (YCPString ("smbios"), smbioslist);
		}
	    }
	    break;
	    case hd_detail_cpu: {
		cpu_info_t *info = hd->detail->cpu.data;
		s = cpu2string (info->architecture);
		out->add (YCPString ("architecture"), YCPString (s));
		out->add (YCPString ("vendor"), YCPString (info->vend_name?info->vend_name:""));
		out->add (YCPString ("name"), YCPString (info->model_name?info->model_name:""));
		out->add (YCPString ("clock"), YCPInteger (info->clock));
		out->add (YCPString ("cache"), YCPInteger (info->cache));
		out->add (YCPString ("siblings"), YCPInteger (info->units));

		if ((info->architecture == arch_intel)
		   || (info->architecture == arch_x86_64))
		{
		    out->add (YCPString ("family"), YCPInteger (info->family));
		    out->add (YCPString ("model"), YCPInteger (info->model));
		}
		else if (info->architecture == arch_alpha)
		{
		    out->add (YCPString ("variation"), YCPInteger (info->family));
		    out->add (YCPString ("revision"), YCPInteger (info->model));
		    if (info->vend_name)
		    {
			out->add (YCPString ("system"), YCPString (info->vend_name));
		    }
		    if (info->model_name)
		    {
			out->add (YCPString ("model"), YCPString (info->model_name));
		    }
		    if (info->platform)
		    {
			out->add (YCPString ("platform"), YCPString (info->platform));
		    }
		}
		else if ((info->architecture == arch_ppc)
			 || (info->architecture == arch_ppc64))
		{
		    if (info->model_name)
		    {
			out->add (YCPString ("model"), YCPString (info->model_name));
		    }
		    if (info->vend_name)
		    {
			out->add (YCPString ("system"), YCPString (info->vend_name));
		    }
		}
		else if (info->architecture == arch_ia64) {
		    if (info->model_name)
		    {
			out->add (YCPString ("model"), YCPString (info->model_name));
		    }
		    out->add (YCPString ("revision"), YCPInteger (info->stepping));
		}
	    }
	    break;
	    case hd_detail_sys: {
		sys_info_t *info = hd->detail->sys.data;
		if (info)
		{
		    if (info->system_type)
			out->add (YCPString ("system"), YCPString (info->system_type));
		    if (info->generation)
			out->add (YCPString ("generation"), YCPString (info->generation));
		    if (info->vendor)
			out->add (YCPString ("vendor"), YCPString (info->vendor));
		    if (info->model)
			out->add (YCPString ("model"), YCPString (info->model));
		}
	    }
	    break;
	    case hd_detail_scsi: {
		scsi_t *info = hd->detail->scsi.data;
		if (info)
		{
		    YCPMap detail;
#if defined(__s390__) || defined(__s390x__)
		    char buf[128];
		    snprintf (buf, 128, "0x%llx", info->wwpn);
		    detail->add (YCPString ("wwpn"), YCPString (buf));
		    snprintf (buf, 128, "0x%llx", info->fcp_lun);
		    detail->add (YCPString ("fcp_lun"), YCPString (buf));
		    if (info->controller_id)
			detail->add (YCPString ("controller_id"), YCPString (info->controller_id));
#endif
		    detail->add (YCPString ("host"), YCPInteger (info->host));
		    detail->add (YCPString ("channel"), YCPInteger (info->channel));
		    detail->add (YCPString ("id"), YCPInteger (info->id));
		    detail->add (YCPString ("lun"), YCPInteger (info->lun));
		    out->add (YCPString ("detail"), detail);
		}
	    }
	    break;
	    case hd_detail_ccw: {
		ccw_t *info = hd->detail->ccw.data;
		if (info)
		{
		    YCPMap detail;
		    detail->add (YCPString ("lcss"), YCPInteger ((int)info->lcss));
		    detail->add (YCPString ("cu_model"), YCPInteger ((int)info->cu_model));
		    detail->add (YCPString ("dev_model"), YCPInteger ((int)info->dev_model));
		    out->add (YCPString ("detail"), detail);
		}
	    }
	    break;
	    default:
	    break;
	}
    }

    // bus

    s = hd->bus.name;
    if (s)
    {
	out->add (YCPString ("bus"), YCPString (s));
    }

    s = hd_busid_to_hwcfg (hd->bus.id);
    if (s)
    {
	out->add (YCPString ("bus_hwcfg"), YCPString (s));
    }

    if (hd->bus.id == bus_pci)
    {
	int i = hd->slot >> 8;

	if (i != 0)
	{
	    out->add (YCPString ("bus_id"), YCPInteger (i));
	}

	i = hd->slot & 0xff;
	if (i != 0)
	{
	    out->add (YCPString ("slot_id"), YCPInteger (i));
	}
	if (hd->func != 0)
	{
	    out->add (YCPString ("func_id"), YCPInteger (hd->func));
	}
    }

    // hotplug
    s = hotplug2string (hd->hotplug);
    if (s)
    {
	out->add (YCPString ("hotplug"), YCPString (s));
    }

    // device name

    s = hd->device.name;
    if (s)
    {
	out->add (YCPString ("device"), YCPString (s));
    }

    // vendor name

    s = hd->vendor.name;
    if (s)
    {
	out->add (YCPString ("vendor"), YCPString (s));
    }

    // sub device name

    s = hd->sub_device.name;
    if (s)
    {
	out->add (YCPString ("sub_device"), YCPString (s));
    }

    // sub vendor name

    s = hd->sub_vendor.name;
    if (s)
    {
	out->add (YCPString ("sub_vendor"), YCPString (s));
    }

    // HAL udi

    s = hd->udi;
    if (s)
    {
	out->add (YCPString ("udi"), YCPString (s));
    }

    // unique key

    s = hd->unique_id;
    if (s)
    {
	out->add (YCPString ("unique_key"), YCPString (s));
    }

    if (hd->old_unique_id != 0
	&& strcmp (s, hd->old_unique_id) != 0)
    {
	out->add (YCPString ("old_unique_key"), YCPString (hd->old_unique_id));
    }

    // vendor, device, subvendor, subdevice id, if known

    if (hd->vendor.id > 0)
    {
	out->add (YCPString ("vendor_id"), YCPInteger (hd->vendor.id));
    }

    if (hd->device.id > 0)
    {
	out->add (YCPString ("device_id"), YCPInteger (hd->device.id));
    }

    if (hd->sub_vendor.id > 0)
    {
	out->add (YCPString ("sub_vendor_id"), YCPInteger (hd->sub_vendor.id));
    }

    if (hd->sub_device.id > 0)
    {
	out->add (YCPString ("sub_device_id"), YCPInteger (hd->sub_device.id));
    }

    // class and subclass id

    out->add (YCPString ("class_id"), YCPInteger (hd->base_class.id));
    out->add (YCPString ("sub_class_id"), YCPInteger (hd->sub_class.id));

    // revision

    s = hd->revision.name;
    if ((s == 0) && (hd->revision.id != 0))
    {
	static char revbuf[64];
	sprintf (revbuf, "%d", hd->revision.id);
	s = revbuf;
    }
    if (s)
	out->add (YCPString ("rev"), YCPString (s));

    // compat vendor name

    s = hd->compat_vendor.name;
    if (s)
	out->add (YCPString ("compat_vendor"), YCPString (s));

    // compat device name

    s = hd->compat_device.name;
    if (s)
	out->add (YCPString ("compat_device"), YCPString (s));

    // linux device name, name2, names and num

    add_str (hd->unix_dev_name, &out, "dev_name");
    add_str (hd->unix_dev_name2, &out, "dev_name2");
    add_strlist (hd->unix_dev_names, &out, "dev_names");
    add_devnum (&hd->unix_dev_num, &out, "dev_num");

// the sysfs path could be quite long but noone needs it, see sysfs_bus_id below
#if 0
    // sysfs path

    add_str (hd->sysfs_id, &out, "sysfs_id");
#endif

    // sysfs bus id

    add_str (hd->sysfs_bus_id, &out, "sysfs_bus_id");

    // BIOS id
    s = hd->rom_id;
    if (s)
	out->add (YCPString ("bios_id"), YCPString (s));

    // Programming interface
    if (hd->prog_if.id != 0)
	out->add (YCPString ("prog_if"), YCPInteger (hd->prog_if.id));

    if (hd->driver != 0)
	out->add (YCPString ("driver"), YCPString (hd->driver));

    // model (combined vendor and device names)
    // since model may already be inserted by the cpu stuff above we have
    // to check for it's present first (kind of weird)
    s = hd->model;
    if (s && out->value (YCPString ("model")).isNull())
	out->add (YCPString ("model"), YCPString (s));

    // driver info
    const driver_info_t* drvinfo = hd->driver_info;
    YCPList drvlist;
    while (drvinfo)
    {
	const char *name = 0;

	YCPMap drvmap = driver_info2map (drvinfo, &name);

	if (drvmap->size() > 0)
	{
	    drvlist->add (drvmap);
	}
	drvinfo = drvinfo->next;

	if (name && (drvinfo == 0) && (drvlist->size() > 0))
	{
	    out->add (YCPString (name), drvlist);
	}
    }

    // requires
    strlist2ycplist (hd->requires, &out, "requires");

    // map of resources

    if (hd->res > 0)
    {
	YCPMap map;
	hd_res_t *resource;

	// loop over resources

	for (resource = hd->res; resource != 0; resource = resource->next)
	{
	  const char *name = 0;
	  YCPMap res = resource_type2map ((res_any_t *)resource, &name);
	  // add resource to map under name
	  // if named value already exists, make it a list of maps

	  if (!res.isNull() && name)
	  {
	      YCPValue mapkey = YCPString (name);
	      YCPValue mapval = map->value (mapkey);

	      YCPList resList;				// new list of resource maps

	      if ((!mapval.isNull())
		  && (mapval->isList ()))	// mapkey existant
	      {
		  resList = mapval->asList();
	      }
	      resList->add (res);
	      map->add (mapkey, resList);
	  } // if (res && name)

	} // loop over resources

	if (map->size() > 0)
	{
	    out->add (YCPString ("resource"), map);
	}
    }

    if (!out.isNull() && out->size() == 0)
    {
	out = YCPNull();
    }

    return out;
}


// convert a linked list of hd_t entries
// to a YCPList data structure
//
YCPList
HwProbe::hdlist2ycplist (hd_t *hd, hd_hw_item_t filteritem)
{
    YCPList list;

    while (hd)
    {
	if ((filteritem == 0)			// zero means 'all'
	    || (filteritem == hd->hw_class))	// only those matching filter
	{
	    list->add (hd2value (hd));
	}
	hd = hd->next;
    }

    return list;
}


// probe for item
// return ycplist
//

YCPValue
HwProbe::byItem (hd_hw_item_t item, bool re_probe)
{
    return hdlist2ycplist (hd_list (hd_base, item, 1, 0));
}


// probe for hw_manual and filter by item
// return ycplist
//

YCPValue
HwProbe::filterManual (hd_hw_item_t item)
{
    return hdlist2ycplist (hd_list (hd_base, hw_manual, 1, 0), item);
}


// check boot architecture
//
YCPValue
HwProbe::bootArch ()
{
    return YCPString (boot2string (hd_boot_arch (hd_base)));
}


// check system architecture
//
YCPValue
HwProbe::cpuArch ()
{
    return YCPString (cpu2string (hd_cpu_arch (hd_base)));
}


// check boot disk
//
YCPValue
HwProbe::bootDisk ()
{
    int i;
    hd_t *hd = hd_get_device_by_idx (hd_base, hd_boot_disk (hd_base, &i));
    if ((hd != 0) && (hd->unix_dev_name != 0))
	return YCPString (hd->unix_dev_name);
    else
	return YCPString ("");
}

// convert String list ("a,b,c") into a YCPList

YCPList String2List(const char *sl)
{
    YCPList l;
    char *s, *t;
    char *str = strdup(sl);

    for (s = str; (t = strchr(s, ',')); s = t + 1) {
	*t = 0;
	l->add (YCPString (s));
    }
    l->add (YCPString (s));
    free(str);
    return l;
}

// return isdn hardware data
//
YCPValue
HwProbe::cdb_isdnData ()
{
    YCPMap isdnMap;
    YCPMap VendorMap;
    YCPMap CardMap;

    y2debug ("CDBISDNData()");

    cdb_isdn_card *cic;
    cdb_isdn_vario *civ;
    cdb_isdn_vendor *vend;
    int card, vario, v;

    y2debug ("CDBISDN_VERSION %4x", hd_cdbisdn_get_version());
    y2debug ("CDBISDN_DATA VERSION %x",hd_cdbisdn_get_db_version());
    y2debug ("CDBISDN_DATA DATE %s", hd_cdbisdn_get_db_date());

    v=0;
    while ((vend = hd_cdbisdn_get_vendor(v++)))
    {
	YCPMap  isdnVendor;

	isdnVendor->add (YCPString ("name"), YCPString (vend->name));
	isdnVendor->add (YCPString ("shortname"), YCPString (vend->shortname));
	isdnVendor->add (YCPString ("refcnt"), YCPInteger (vend->refcnt));
	isdnVendor->add (YCPString ("VendorID"), YCPInteger (vend->vnr));
	VendorMap->add (YCPInteger (vend->vnr), isdnVendor);
    }
    isdnMap->add (YCPString ("Vendors"), VendorMap);

    card = 1;
    while ((cic = hd_cdbisdn_get_card(card++)))
    {
	YCPMap  isdnEntry;
	YCPList isdnDriver;

	if (cic->name)
	{
	    isdnEntry->add (YCPString ("name"), YCPString (cic->name));
	}

	if (cic->lname)
	{
	    isdnEntry->add (YCPString ("longname"), YCPString (cic->lname));
	}

	isdnEntry->add (YCPString ("VendorRef"), YCPInteger (cic->vhandle));
	isdnEntry->add (YCPString ("features"), YCPInteger (cic->features));
	isdnEntry->add (YCPString ("lines"), YCPInteger (cic->line_cnt));
	vario = cic->vario;
	while ((civ = hd_cdbisdn_get_vario(vario))) {
	    YCPMap  isdnDrvEntry;

	    isdnDrvEntry->add (YCPString ("name"), YCPString(civ->name));
	    isdnDrvEntry->add (YCPString ("mod_name"), YCPString(civ->mod_name));
	    isdnDrvEntry->add (YCPString ("type"), YCPInteger (civ->typ));
	    isdnDrvEntry->add (YCPString ("subtype"), YCPInteger (civ->subtyp));
	    isdnDrvEntry->add (YCPString ("drvid"), YCPInteger (civ->drvid));
	    if (civ->description)
		isdnDrvEntry->add (YCPString ("description"), YCPString(civ->description));
	    if (civ->info)
		isdnDrvEntry->add (YCPString ("info"), YCPString(civ->info));
	    if (civ->need_pkg)
		isdnDrvEntry->add (YCPString ("need_pkg"), String2List(civ->need_pkg));
	    if (civ->features && civ->features[0])
		isdnDrvEntry->add (YCPString ("features"), String2List(civ->features));
	    if (civ->protocol)
		isdnDrvEntry->add (YCPString ("protocol"), String2List(civ->protocol));
	    if (civ->irq && civ->irq[0])
		isdnDrvEntry->add (YCPString ("IRQ"), String2List(civ->irq));
	    if (civ->io && civ->io[0])
		isdnDrvEntry->add (YCPString ("IO"), String2List(civ->io));
	    if (civ->membase && civ->membase[0])
		isdnDrvEntry->add (YCPString ("MEMBASE"), String2List(civ->membase));
	    isdnDriver->add (isdnDrvEntry);
	    vario = civ->next_vario;
	}
	isdnEntry->add (YCPString ("driver"), isdnDriver);
	isdnEntry->add (YCPString ("class"), YCPString (cic->Class));
	isdnEntry->add (YCPString ("bus"), YCPString (cic->bus));
	if (cic->vendor != 0)
	{
	    isdnEntry->add (YCPString ("vendor"), YCPInteger (cic->vendor));
	    isdnEntry->add (YCPString ("device"), YCPInteger (cic->device));
	    isdnEntry->add (YCPString ("subvendor"), YCPInteger (cic->subvendor));
	    isdnEntry->add (YCPString ("subdevice"), YCPInteger (cic->subdevice));
	}
	isdnEntry->add (YCPString ("CardID"), YCPInteger (cic->handle));
	isdnEntry->add (YCPString ("revision"), YCPInteger (cic->revision));
	CardMap->add (YCPInteger (cic->handle), isdnEntry);
    }
    isdnMap->add (YCPString ("Cards"), CardMap);

    return isdnMap;
}


// return description of graphics controller used by bios
//

YCPValue
HwProbe::biosVideo ()
{
    hd_t *bios_video;
    YCPList result;

    bios_video = hd_get_device_by_idx (hd_base, hd_display_adapter (hd_base));

    if (bios_video)
    {
	result->add (hd2value (bios_video));
    }

    return result;
}

// return description of framebuffer
//

YCPValue
HwProbe::vesaFramebuffer ()
{
    YCPList result;


//  hd_set_probe_feature (hd_base, pr_misc);
//  hd_set_probe_feature (hd_base, pr_prom);
//  hd_set_probe_feature (hd_base, pr_bios_vbe);
//  hd_set_probe_feature (hd_base, pr_fb);
    YCPValue framebuffer = byItem (hw_framebuffer, true);

    if (framebuffer.isNull())
    {
	return framebuffer;
    }
    if (!framebuffer->isList()
	|| (framebuffer->asList()->size() < 1))
    {
	return YCPVoid();
    }
    YCPMap first_framebuffer = framebuffer->asList()->value(0)->asMap();

    YCPValue fb_resource = first_framebuffer->value (YCPString("resource"));
    if (fb_resource.isNull() || !fb_resource->isMap())
    {
	return YCPVoid();
    }
    YCPValue fb_resolutions = fb_resource->asMap()->value (YCPString("framebuffer"));
    if (fb_resolutions.isNull())
    {
	return YCPVoid();
    }
    return fb_resolutions;
}

// EOF
