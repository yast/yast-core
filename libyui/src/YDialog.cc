/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:	      YDialog.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui"
#include <ycp/y2log.h>
#include "YDialog.h"
#include "YShortcutManager.h"


YDialog::YDialog(YWidgetOpt &opt)
    : YContainerWidget(opt)
{
    _hasDefaultSize.setValue(opt.hasDefaultSize.value());
    _hasWarnColor.setValue(opt.hasWarnColor.value());
    _hasInfoColor.setValue(opt.hasInfoColor.value());
    _isDecorated.setValue(opt.isDecorated.value());
}


YDialog::~YDialog()
{
}


bool YDialog::isDialog() const
{
    return true;
}


void YDialog::setInitialSize()
{
    y2debug("Setting initial dialog size");
    setSize(nicesize(YD_HORIZ), nicesize(YD_VERT));
    y2debug("Initial dialog size done");
}


void YDialog::checkKeyboardShortcuts()
{
    YShortcutManager shortcutManager( this );
    shortcutManager.checkShortcuts();
}

