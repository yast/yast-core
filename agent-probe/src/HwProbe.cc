/**

  HwProbe.cc

  Purpose:	hardware autoprobe repository access
		handling of .probe paths

  Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
  Maintainer:	Arvin Schnell <arvin@suse.de>

  see doc/hwprobe.html for a description

*/


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "HwProbe.h"
#include <ycp/y2log.h>

/*
 * convert YCPSymbol to hd_status_value_t
 *
 */

static hd_status_value_t
sym_to_status (const YCPSymbol& symbol)
{
    hd_status_value_t status = status_unknown;

    if (symbol->symbol() == "no")
    {
	status = status_no;
    }
    else if (symbol->symbol() == "yes")
    {
	status = status_yes;
    }
    else if (symbol->symbol() == "new")
    {
	status = status_new;
    }
    return status;
}


/*
 * convert hd_status_value_t to YCPSymbol
 *
 */

static const YCPSymbol
status_to_sym (hd_status_value_t status)
{
    switch (status)
    {
	case status_no:
	    return YCPSymbol("no");
	case status_yes:
	    return YCPSymbol("yes");
	case status_new:
	    return YCPSymbol("new");
	default:
	    return YCPSymbol("unknown");
    }
}


/*
 * helper function for Read(.status ...)
 *
 */

static const YCPValue
readStatus (hd_data_t *hd_data, int which, const YCPValue& arg)
{
    static YCPValue unknown = YCPSymbol ("unknown");
    y2debug ("readStatus (%p, %d, %s)", hd_data, which, arg.isNull()?"NULL":arg->toString().c_str());
    if (arg.isNull()
	|| !arg->isString())
    {
	return YCPError ("Bad key for Read(.probe.status, key)", unknown);
    }

    hd_t *hd = hd_read_config (hd_data, arg->asString()->value_cstr());
    if (hd == 0)
    {
	y2error ("hd_read_config('%s') == 0", arg->asString()->value_cstr());
	return YCPError ("Unknown key for Read(.probe.status, key)", unknown);
    }

    switch (which)
    {
	case 0:		// configured
	    return status_to_sym ((hd_status_value_t)hd->status.configured);
	case 1:		// available
	    return status_to_sym ((hd_status_value_t)hd->status.available);
	case 2:		// needed
	    return status_to_sym ((hd_status_value_t)hd->status.needed);
	case 3:		// config_string
	    return YCPString ((hd->config_string == NULL) ? "" : hd->config_string);
	default:
	    return unknown;
    }
}


int
HwProbe::doScan (int force)
{
    if (hd_base == 0)
	return -1;

    return (hd_data_first (hd_base) != 0) ? 0 : -1;
}


HwProbe::HwProbe()
{
    y2debug ("HwProbe::HwProbe()");

    // create lock file
    int f = open ("/var/lib/hardware/LOCK", O_CREAT);
    if (f > 0) close (f);

    hd_base = (hd_data_t *)calloc (1, sizeof (hd_data_t));
    hd_scan (hd_base);
}


HwProbe::~HwProbe()
{
    y2debug ("HwProbe::~HwProbe()");
    if (hd_base)
    {
	hd_free_hd_data (hd_base);
	free (hd_base);
    }
    // remove lock file
    unlink ("/var/lib/hardware/LOCK");
}


// ------------------------------------------------------------------

/**
 * Read
 *
 * read value from relative path
 */

YCPValue
HwProbe::Read(const YCPPath& path, const YCPValue& arg, const YCPValue& optarg)
{
    if (hd_base == 0)
    {
	y2error ("hw probe failed");
	return YCPVoid();
    }

    y2debug ("Read (%s)", path->toString().c_str());

    return checkPath (path, arg, YCPNull(), 0);
}


/**
 * Write
 *
 * write value to relative path
 * only .status allowed
 * @example SCR::Write (.probe.status.configured, "unique_key", `yes);
 */

YCPBoolean
HwProbe::Write(const YCPPath& path, const YCPValue& value, const YCPValue& arg)
{
    y2debug ("Write (%s:%s)", path->toString().c_str(), value->toString().c_str());

    // dont check type of arg yet, might be symbol or string
    if (value.isNull()
	|| arg.isNull()
	|| !value->isString())
    {
	ycp2error ("Bad parameters for Write(.probe.status...)");
	return YCPBoolean (false);
    }

    YCPValue v = checkPath (path, value, arg, 1);
    return v.isNull () ? YCPNull () : v->asBoolean ();
}


/**
 * Dir
 *
 * show subtree possibilities
 */

YCPList
HwProbe::Dir(const YCPPath& path)
{
    YCPValue v = checkPath (path, YCPNull(), YCPNull(), 2);
    return v.isNull() || v->isVoid() ? YCPNull () : v->asList ();
}


/**
 * checkPath
 * check given path
 * @param path path to check
 * @param func == 0 for read, == 1 for write, == 2 for dir
 * or return full value for sub-tree
 */

YCPValue
HwProbe::checkPath (const YCPPath& path, const YCPValue& arg,
		    const YCPValue& writeval, int func)
{
    YCPValue value = YCPNull();

    y2debug ("checkPath (%s)", path->toString().c_str());

    typedef struct
    {
	char *pathname;
	int type;
	hd_probe_feature_t feature;
	void *data;
    } subPath;

#define pr_null (hd_probe_feature_t)0

    // possible branches for 'probe.status'

    static subPath sub_status[] = {
	{ "configured",		0,	pr_null,	0 },
	{ "available",		1,	pr_null,	0 },
	{ "needed",		2,	pr_null,	0 },
	{ "info",		3,	pr_null,	0 },
	{ 0, 0, pr_null, 0 }
    };

    // these are the top-level branches

    static subPath top_path[] = {
	{ "has_pcmcia",		 2, pr_pci,	0},
	{ "architecture",	 4, pr_cpu,	0},
	{ "boot_arch",		 5, pr_cpu,	0},
	{ "version",		 6, pr_null,	0},
	{ "boot_disk",		 7, pr_bios,	0},
	{ "cdb_isdn",		 8, pr_null,	0},
	{ "has_smp",		 9, pr_null,	0},
	{ "bios_video",		10, pr_bios,	0},
	{ "is_uml",		11, pr_null,	0},
	{ "framebuffer",	13, pr_fb,	0},
	{ "status",		14, pr_null,	sub_status},
	/* now the hw_items  */
#define ITEM(x) ((int)x + 42)
	{ "cdrom",		ITEM(hw_cdrom),		pr_null,	0},
	{ "floppy",		ITEM(hw_floppy),	pr_null,	0},
	{ "disk",		ITEM(hw_disk),		pr_null,	0},
	{ "netif",		ITEM(hw_network),	pr_null,	0},
	{ "display",		ITEM(hw_display),	pr_null,	0},
	{ "mouse",		ITEM(hw_mouse),		pr_null,	0},
	{ "keyboard",		ITEM(hw_keyboard),	pr_null,	0},
	{ "sound",		ITEM(hw_sound),		pr_null,	0},
	{ "isdn",		ITEM(hw_isdn),		pr_null,	0},
	{ "modem",		ITEM(hw_modem),		pr_null,	0},
	{ "storage",		ITEM(hw_storage_ctrl),	pr_null,	0},
	{ "netcard",		ITEM(hw_network_ctrl),	pr_null,	0},
	{ "monitor",		ITEM(hw_monitor),	pr_null,	0},
	{ "printer",		ITEM(hw_printer),	pr_null,	0},
	{ "tv",			ITEM(hw_tv),		pr_null,	0},
	{ "dvb",		ITEM(hw_dvb),		pr_null,	0},
	{ "scanner",		ITEM(hw_scanner),	pr_null,	0},
	{ "system",		ITEM(hw_sys),		pr_null,	0},
	{ "camera",		ITEM(hw_camera),	pr_null,	0},
	{ "chipcard",		ITEM(hw_chipcard),	pr_null,	0},
	{ "usbctrl",		ITEM(hw_usb_ctrl),	pr_null,	0},
	{ "ieee1394ctrl",	ITEM(hw_ieee1394_ctrl),	pr_null,	0}, // aka firewire
	{ "hub",		ITEM(hw_hub),		pr_null,	0},
	{ "scsi",		ITEM(hw_scsi),		pr_null,	0},
	{ "ide",		ITEM(hw_ide),		pr_null,	0},
	{ "memory",		ITEM(hw_memory),	pr_null,	0},
	{ "fbdev",		ITEM(hw_framebuffer),	pr_null,	0},
	{ "usb",		ITEM(hw_usb),		pr_null,	0},
	{ "pci",		ITEM(hw_pci),		pr_null,	0},
	{ "isapnp",		ITEM(hw_isapnp),	pr_null,	0},
	{ "cpu",		ITEM(hw_cpu),		pr_null,	0},
	{ "braille",		ITEM(hw_braille),	pr_null,	0},
	{ "joystick",		ITEM(hw_joystick),	pr_null,	0},
	{ "bios",		ITEM(hw_bios),		pr_null,	0},
	{ "pppoe",		ITEM(hw_pppoe),		pr_null,	0},
	{ "wlan",		ITEM(hw_wlan),		pr_null,	0},
	{ "redasd",		ITEM(hw_redasd),	pr_null,	0},
	{ "block",		ITEM(hw_block),		pr_null,	0},
	{ "tape",		ITEM(hw_tape),		pr_null,	0},
	{ "bluetooth",		ITEM(hw_bluetooth),	pr_null,	0},
	{ "dsl",		ITEM(hw_dsl),		pr_null,	0},
//not-yet-in-hwinfo	{ "vbe",		ITEM(hw_vbe),		pr_null,	0},
	{ 0, 0, pr_null, 0 }
    };

#undef pr_null

    int len = path->length ();
    int cidx = 0;	// component_str index

#define PATHCOUNT 3
    int typelist[PATHCOUNT] = { -1, -1, -1 };

    subPath *path_desc = top_path;

    // scan given path

    while (cidx < len)
    {
	string path_name = path->component_str (cidx);

	int i = 0;
	while (path_desc[i].pathname != 0)
	{
	    if (path_name == path_desc[i].pathname)
		break;
	    i++;
	}

	if (path_desc[i].pathname == 0)
	{
	    y2warning ("Unrecognized path '%s'", path->toString().c_str());
	    return YCPVoid ();
	}

	typelist[cidx] = path_desc[i].type;

	// check for added ".manual"
	if ((cidx == len-2)
	    && (path->component_str (cidx+1) == "manual"))
	{
	   typelist[cidx+1] = typelist[cidx];	// put the real hw_X one ahead
	   typelist[cidx] = ITEM(hw_manual);	// 'prefix' with hw_manual
	   cidx++;
	}

	if (path_desc[i].feature != 0)
	{
	    // free and use afterwards is a bad idea (see bug#44855)
	    // hd_free_hd_data (hd_base); // free data of last scan
	    hd_set_probe_feature (hd_base, path_desc[i].feature);
	    hd_scan (hd_base);
	}

	// sub-path allowed ?
	path_desc = (subPath *)(path_desc[i].data);
	if (path_desc == 0)
	    break;

	cidx++;

	if (cidx >= PATHCOUNT)
	    break;

    } // cidx < len


    y2debug ("checkPath (cidx %d, len %d)", cidx, len);

    // more static path description, loop over it
    //   and collect data
    //

    if (path_desc != 0)
    {
	YCPMap map;
	YCPList list;

	int i = 0;
	while (path_desc[i].pathname != 0)
	{
	    if (func == 0)		// Read ()
	    {
		YCPPath p;
		for (int j = 0; j < cidx; j++)
		{
		    p->append (path->component_str (j));
		}
		p->append (string (path_desc[i].pathname));
		YCPValue result = Read (p, arg);

		y2debug ("Read(%s) -> %s", p->toString().c_str(), (result.isNull())?"nix":result->toString().c_str());

		if (!(result.isNull() || result->isVoid()
		    || (result->isList() && result->asList()->isEmpty())))
		{
		    map->add (YCPString (path_desc[i].pathname), result);
		}
	    }
	    else if (func == 1) 	// Write ()
	    {
		return YCPError ("Use fully qualified path for Write (.probe.status...)");
	    }
	    else	// assume Dir()
	    {	// return names of sub-paths
		list->add (YCPString (path_desc[i].pathname));
	    }
	    i++;
	}

	if (func == 0)
	    value = map;
	else
	    value = list;

    }
    else   // no more static path info
    {
	if (func == 0)		// Read()
	{
	    switch (typelist[0])
	    {
		case 2:		// has_pcmcia
		    value = YCPBoolean (hd_has_pcmcia (hd_base) ? true : false);
		break;
		case 4:		// cpu_arch
		    hd_free_hd_list (hd_list (hd_base, hw_cpu, 1, 0)); // to trigger scanning
		    value = cpuArch ();
		break;
		case 5:		// boot_arch
		    hd_free_hd_list (hd_list (hd_base, hw_cpu, 1, 0)); // to trigger scanning
		    value = bootArch ();
		break;
		case 6:		// version
		    value = YCPString (__DATE__", "__TIME__);
		break;
		case 7:		// boot_disk
		    value = bootDisk ();
		break;
		case 8:		// cdb_isdn
		    value = cdb_isdnData ();
		break;
		case 9:		// has_smp
		    value = YCPBoolean (hd_smp_support (hd_base) ? true : false);
		break;
		case 10:		// bios_video
		    byItem (hw_display, false);
		    value = biosVideo ();
		break;
		case 11:		// is_uml
		    value = YCPBoolean (hd_is_uml (hd_base) ? true : false);
		break;
		case 13:		// framebuffer
		    value = vesaFramebuffer ();
		break;
		case 14:		// status
		    value = readStatus (hd_base, typelist[1], arg);
		break;
		case ITEM(hw_manual):
		    value = filterManual ((hd_hw_item_t)(typelist[1]-42));
		break;
		default:
		    if (typelist[0] > 42)
		    {
			value = byItem ((hd_hw_item_t)(typelist[0]-42), true);
		    }
		    else
			value = YCPVoid ();
		break;
	    }
	}
	else if (func == 1)	// Write()
	{
	    // arg == unique_key
	    // writeval == symbol

	    if (typelist[0] == 14)	// .status
	    {
		hd_status_t status = { 0 };	// default: no changes

		if (typelist[1] == 3)
		{
		    if (!writeval->isString())
		    {
			return YCPError ("Argument must be string", YCPBoolean (false));
		    }
		    return YCPBoolean (hd_change_config_status (hd_base, arg->asString()->value_cstr(),
							 status, writeval->asString()->value_cstr()) == 0);
		}

		if (!writeval->isSymbol())
		{
		    return YCPError ("Argument must be symbol", YCPBoolean (false));
		}

		hd_status_value_t status_value = sym_to_status (writeval->asSymbol());

		switch (typelist[1])
		{
		    case 0:		// configured
			status.configured = status_value;
		    break;
		    case 1:		// available
			status.available = status_value;
		    break;
		    case 2:		// needed
			status.needed = status_value;
		    break;
		    default:
		    break;
		}
		value = YCPBoolean (hd_change_config_status (hd_base, arg->asString()->value_cstr(), status, 0) == 0);
	    }
	}
	else			// assume Dir()
	{
	    value = YCPVoid ();
	}
    }

    if (value.isNull())
	value = YCPVoid ();

    y2debug ("--> checkPath (%s)", value->toString().c_str());

    return value;
}
