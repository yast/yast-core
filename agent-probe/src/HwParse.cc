/**

    HwParse.cc

    Purpose:	ycp to libhd interface, parse hd_t and construct YCPValueRep
    Creator:	kkaempf@suse.de
    Maintainer:	kkaempf@suse.de

    $Id$
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "HwProbe.h"
#include <ycp/y2log.h>

/* --------------------------------------------------------------------------------------------------*/
/* First, some helper functions  */


// hd_cpu_arch_t -> string
//
static char *
cpu2string (hd_cpu_arch_t cpu_arch)
{
    char *s;
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
static char *
boot2string (hd_boot_arch_t boot_arch)
{
    char *s;
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
static char*
hotplug2string (hd_hotplug_t hotplug)
{
    char* s;
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
    char *s;
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


/* --------------------------------------------------------------------------------------------------*/
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
	  *name ="mem";
	  map->add (YCPString ("start"), YCPInteger (r->base));
	  if (r->range > 0)
	      map->add (YCPString ("length"), YCPInteger (r->range));
	  map->add (YCPString ("active"), 	YCPBoolean ((r->enabled)?true:false));
	}
	break;
    case res_phys_mem: {
	RES2TYPE (res_phys_mem_t);
	*name ="phys_mem";
	if (r->range > 0)
	map->add (YCPString ("range"), YCPInteger (r->range));
    }
    break;
    case res_io: {
	RES2TYPE (res_io_t);
	*name ="io";
	map->add (YCPString ("start"), YCPInteger (r->base));
	map->add (YCPString ("length"), YCPInteger (r->range));
	map->add (YCPString ("mode"), access2string (r->access));
	map->add (YCPString ("active"),	YCPBoolean ((r->enabled)?true:false));
    }
    break;
    case res_irq: {
	RES2TYPE (res_irq_t);
	*name ="irq";
	map->add (YCPString ("irq"), YCPInteger (r->base));
	map->add (YCPString ("count"), YCPInteger (r->triggered));
	map->add (YCPString ("enabled"),	YCPBoolean ((r->enabled)?true:false));
    }
    break;
    case res_dma: {
	RES2TYPE (res_dma_t);
	*name ="dma";
	map->add (YCPString ("channel"), YCPInteger (r->base));
	map->add (YCPString ("enabled"),	YCPBoolean ((r->enabled)?true:false));
    }
    break;
    case res_monitor: {
	RES2TYPE (res_monitor_t);
	if (r->interlaced == 0)
	{
	*name ="monitor_resol";
	map->add (YCPString ("width"), YCPInteger (r->width));
	map->add (YCPString ("height"), YCPInteger (r->height));
	map->add (YCPString ("vfreq"), YCPInteger (r->vfreq));
	}
    }
    break;
    case res_size: {
	RES2TYPE (res_size_t);
	*name ="size";
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
	*name = (r->logical) ? "disk_log_geo" : "disk_phys_geo";
	map->add (YCPString ("cylinders"), YCPInteger (r->cyls));
	map->add (YCPString ("heads"), YCPInteger (r->heads));
	map->add (YCPString ("sectors"), YCPInteger (r->sectors));
    }
    break;
    case res_cache: {
	RES2TYPE (res_cache_t);
	*name ="cache";
	map->add (YCPString ("size"), YCPInteger (r->size));
    }
    break;
    case res_baud: {
	RES2TYPE (res_baud_t);
	*name ="baud";
	map->add (YCPString ("speed"), YCPInteger (r->speed));
    }
    break;
    case res_init_strings: {
	RES2TYPE (res_init_strings_t);
	*name ="init_strings";
	if (r->init1 != 0)
	map->add (YCPString ("init1"), YCPString (r->init1));
	if (r->init2 != 0)
	map->add (YCPString ("init2"), YCPString (r->init2));
    }
    break;
    case res_pppd_option: {
	RES2TYPE (res_pppd_option_t);
	*name ="pppd_option";
	if (r->option != 0)
	map->add (YCPString ("option"), YCPString (r->option));
    }
    break;
	case res_framebuffer:
	{
	    RES2TYPE (res_framebuffer_t);
	    *name ="framebuffer";
	    map->add (YCPString ("width"), YCPInteger (r->width));
	    map->add (YCPString ("height"), YCPInteger (r->height));
	    map->add (YCPString ("color"), YCPInteger (r->colorbits));
	    map->add (YCPString ("mode"), YCPInteger (r->mode));
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
strlist2ycplist (const str_list_t* strlist, YCPMap map, const char* key)
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

    map->add (YCPString (key), ycplist);
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

	    strlist2ycplist (d->extensions, map, "extensions");
	    strlist2ycplist (d->options, map, "options");
	    strlist2ycplist (d->raw, map, "raw");
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
			if (1 || (ip->flags & P_DEFINE))
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
    char *s;

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
    else if (hd->is.cardbus)
    {
	out->add (YCPString ("cardtype"), YCPString ("cardbus"));
    }
    else if (hd->is.pcmcia)
    {
	out->add (YCPString ("cardtype"), YCPString ("pcmcia"));
    }
    else if (hd->is.notready)
    {
	out->add (YCPString ("notready"), YCPBoolean (true));
    }

    if (hd->detail)
    {
	switch (hd->detail->type)
	{
	    case hd_detail_cdrom: {
		string cdtype = "cdrom";
		if (hd->detail->cdrom.data->dvdram)
		    cdtype = "dvdram";
		else if (hd->detail->cdrom.data->cdrw)
		    cdtype = "cdrw";
		else if (hd->detail->cdrom.data->dvdr)
		    cdtype = "dvdr";
		else if (hd->detail->cdrom.data->cdr)
		    cdtype = "cdr";
		else if (hd->detail->cdrom.data->dvd)
		    cdtype = "dvd";
		out->add (YCPString ("cdtype"), YCPString (cdtype));
	    }
	    break;
	    case hd_detail_floppy: {
	    }
	    break;
	    case hd_detail_bios: {
		bios_info_t *info = hd->detail->bios.data;
		out->add (YCPString ("apm_supported"), YCPBoolean ((info->apm_supported)?true:false));
		out->add (YCPString ("apm_enabled"), YCPBoolean ((info->apm_enabled)?true:false));
		out->add (YCPString ("apm_version"), YCPInteger ((info->apm_ver)));
		out->add (YCPString ("apm_subversion"), YCPInteger ((info->apm_subver)));
		out->add (YCPString ("apm_biosflags"), YCPInteger ((info->apm_bios_flags)));
		out->add (YCPString ("is_pnp_bios"), YCPBoolean ((info->is_pnp_bios)));
		out->add (YCPString ("lba_support"), YCPBoolean ((info->lba_support)));
	    }
	    break;
	    case hd_detail_cpu: {
		cpu_info_t *info = hd->detail->cpu.data;
		s = cpu2string (info->architecture);
		out->add (YCPString ("architecture"), YCPString (s));
		out->add (YCPString ("vendor"), YCPString (info->vend_name?info->vend_name:""));
		out->add (YCPString ("name"), YCPString (info->model_name?info->model_name:""));

		if (info->architecture == arch_intel)
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
	    default:
	    break;
	}
    }

    // bus

    s = hd_bus_name (hd_base, hd->bus);
    if (s)
    {
	out->add (YCPString ("bus"), YCPString (s));
    }

    if (hd->bus == bus_pci)
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

    s = hd->dev_name;
    if (s == 0)
	s = hd_device_name (hd_base, hd->vend, hd->dev);
    if (s)
	out->add (YCPString ("device"), YCPString (s));

    // vendor name

    s = hd->vend_name;
    if (s == 0)
	s = hd_vendor_name (hd_base, hd->vend);
    if (s)
	out->add (YCPString ("vendor"), YCPString (s));

    // sub device name

    s = hd->sub_dev_name;
    if (s == 0)
	s = hd_sub_device_name (hd_base, hd->vend, hd->dev, hd->sub_vend, hd->sub_dev);
    if (s)
	out->add (YCPString ("sub_device"), YCPString (s));

    // sub vendor name

    s = hd->sub_vend_name;
    if (s == 0)
	s = hd_vendor_name (hd_base, hd->sub_vend);
    if (s)
	out->add (YCPString ("sub_vendor"), YCPString (s));

    // unique key

    s = hd->unique_id;
    if (s)
	out->add (YCPString ("unique_key"), YCPString (s));
    if (hd->old_unique_id != 0
	&& strcmp (s, hd->old_unique_id) != 0)
    {
	out->add (YCPString ("old_unique_key"), YCPString (hd->old_unique_id));
    }

    // vendor, device, subvendor, subdevice id, if known

    if (hd->vend > 0)
	out->add (YCPString ("vendor_id"), YCPInteger (hd->vend));
    if (hd->dev > 0)
	out->add (YCPString ("device_id"), YCPInteger (hd->dev));
    if (hd->sub_vend > 0)
	out->add (YCPString ("sub_vendor_id"), YCPInteger (hd->sub_vend));
    if (hd->sub_dev > 0)
	out->add (YCPString ("sub_device_id"), YCPInteger (hd->sub_dev));

    // class and subclass id

    out->add (YCPString ("class_id"), YCPInteger (hd->base_class));
    out->add (YCPString ("sub_class_id"), YCPInteger (hd->sub_class));

    // revision

    s = hd->rev_name;
    if ((s == 0) && (hd->rev != 0))
    {
	static char revbuf[64];
	sprintf (revbuf, "%d", hd->rev);
	s = revbuf;
    }
    if (s)
	out->add (YCPString ("rev"), YCPString (s));

    // compat vendor name

    if (hd->compat_vend)
    {
	s = hd_vendor_name (hd_base, hd->compat_vend);
	if (s)
	    out->add (YCPString ("compat_vendor"), YCPString (s));
    }

    // compat device name

    if (hd->compat_vend && hd->compat_dev)
    {
	s = hd_device_name (hd_base, hd->compat_vend, hd->compat_dev);
	if (s)
	    out->add (YCPString ("compat_device"), YCPString (s));
    }

    // linux device name

    s = hd->unix_dev_name;
    if (s)
	out->add (YCPString ("dev_name"), YCPString (s));

    // BIOS id

    s = hd->rom_id;
    if (s)
	out->add (YCPString ("bios_id"), YCPString (s));

    // Programming interface
    if (hd->prog_if != 0)
	out->add (YCPString ("prog_if"), YCPInteger (hd->prog_if));

    if (hd->driver != 0)
	out->add (YCPString ("driver"), YCPString (hd->driver));

    // model (combined vendor and device names)
    // since model may already be inserted by the cpu stuff above we have
    // to check for it's present first (kind of weird)
    s = hd->model;
    if (s && !out->haskey (YCPString ("model")))
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
    strlist2ycplist (hd->requires, out, "requires");

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

	  if (!res.isNull() && name) {

	      YCPValue mapkey = YCPString (name);
	      YCPValue mapval = map->value (mapkey);

	      if ((!mapval.isNull())
		  && (mapval->isList ())) {	// mapkey existant
		  mapval->asList()->add (res);
		  map->add (mapkey, mapval);
	      }
	      else {
		  YCPList resList;		// new list of resource maps
		  resList->add (res);
		  map->add (mapkey, resList);
	      }
	  } // if (res && name)

	} // loop over resources

	if (map->size() > 0)
	out->add (YCPString ("resource"), map);
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


// return isdn hardware data
//
YCPValue
HwProbe::ihwData ()
{
    YCPList isdnList;

    y2debug ("ihwData()");

    ihw_card_info *icp;
    ihw_driver_info *idp;
    ihw_para_info *para_p;
    int card, drv, pnr;

    y2debug ("IHW_VERSION %4x", hd_ihw_get_version());
    y2debug ("IHW_DATA VERSION %x",hd_ihw_get_db_version());
    y2debug ("IHW_DATA DATE %s", hd_ihw_get_db_date());

    card = 0;
    while ((icp = hd_ihw_get_card(card++)))
    {
	YCPMap  isdnEntry;
	YCPMap  isdnParameter;
	YCPList isdnDriver;

	if (icp->name)
	{
	    isdnEntry->add (YCPString ("name"), YCPString (icp->name));
	}

	/* temporary until libhd update */
#if (IHW_VERSION == 0x0203)
	isdnEntry->add (YCPString ("features"), YCPInteger (1));
	isdnEntry->add (YCPString ("lines"), YCPInteger (1));
#else
	isdnEntry->add (YCPString ("features"), YCPInteger (icp->features));
	isdnEntry->add (YCPString ("lines"), YCPInteger (icp->line_cnt));
#endif
	drv = icp->driver;
	while ((idp = hd_ihw_get_driver(drv))) {
	    YCPMap  isdnDrvEntry;

	    isdnDrvEntry->add (YCPString ("name"), YCPString(idp->name));
	    isdnDrvEntry->add (YCPString ("mod_name"), YCPString(idp->mod_name));
	    isdnDrvEntry->add (YCPString ("type"), YCPInteger (idp->typ));
	    isdnDrvEntry->add (YCPString ("subtype"), YCPInteger (idp->subtyp));
	    isdnDrvEntry->add (YCPString ("drvid"), YCPInteger (idp->drvid));
	    isdnDrvEntry->add (YCPString ("features"), YCPInteger (idp->features));
	    isdnDrvEntry->add (YCPString ("arch"), YCPInteger (idp->arch));
	    if (idp->description)
		isdnDrvEntry->add (YCPString ("description"), YCPString(idp->description));
	    /* temporary until libhd update */
	    if (idp->need_pkg) {
		YCPList	pkgs;
		char	*s, *t;
		char	*str = strdup(idp->need_pkg);

		for (s = str; (t = strchr(s, ',')); s = t + 1) {
		    *t = 0;
		    pkgs->add (YCPString (s));
		}
		pkgs->add (YCPString (s));
		isdnDrvEntry->add (YCPString ("need_pkg"), pkgs);
		free(str);
	    }
	    isdnDriver->add (isdnDrvEntry);
	    drv = idp->next_drv;
	}
	isdnEntry->add (YCPString ("driver"), isdnDriver);
	isdnEntry->add (YCPString ("class"), YCPInteger (icp->Class));
	if ((icp->Class == CLASS_PCI) || (icp->Class == CLASS_ISAPNP) ||
	    (icp->Class == CLASS_USB))
	{
	    isdnEntry->add (YCPString ("vendor"), YCPInteger (icp->vendor));
	    isdnEntry->add (YCPString ("device"), YCPInteger (icp->device));
	    isdnEntry->add (YCPString ("subvendor"), YCPInteger (icp->subvendor));
	    isdnEntry->add (YCPString ("subdevice"), YCPInteger (icp->subdevice));
	}
	pnr = 1;
	while ((para_p = hd_ihw_get_parameter(icp->handle, pnr++)))
	{
	    y2debug ("para number %d", pnr-1);
	    if (para_p->flags & P_DEFINE)
	    {
		YCPList defineList;
		defineList->add (YCPInteger (para_p->def_value));
		if (para_p->list)
		{
		    unsigned long *p = (unsigned long *)(para_p->list + 1);
		    int i,para_count = *para_p->list;

		    for (i=0; i<para_count; i++)
		    {
			defineList->add (YCPInteger (*p++));
		    }
		}
		isdnParameter->add (YCPString (para_p->name), defineList);
		isdnEntry->add (YCPString ("parameter"), isdnParameter);
	    }
	}

	isdnList->add (isdnEntry);
    }

    return isdnList;
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
