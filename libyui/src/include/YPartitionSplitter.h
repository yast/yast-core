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

  File:	      YPartitionSplitter.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YPartitionSplitter_h
#define YPartitionSplitter_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPInteger.h>


class YMacroRecorder;

/**
 * @short Implementation of the Slider widget
 */
class YPartitionSplitter : public YWidget
{
public:

    /**
     * Constructor
     */
    YPartitionSplitter( const YWidgetOpt &	opt,
			int 			usedSize,
			int 			totalFreeSize,
			int 			newPartSize,
			int 			minNewPartSize,
			int 			minFreeSize,
			const YCPString &	usedLabel,
			const YCPString &	freeLabel,
			const YCPString &	newPartLabel,
			const YCPString &	freeFieldLabel,
			const YCPString &	newPartFieldLabel );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YPartitionSplitter"; }

    /**
     * Implements the ChangeWidget() UI command.
     */
    YCPValue changeWidget( const YCPSymbol & property,
			   const YCPValue &  newValue );

    /**
     * Implements the QueryWidget() UI command.
     */
    YCPValue queryWidget( const YCPSymbol & property );


    /**
     * Change the slider value.
     *
     * Overload this, but call YPartitionSplitter::setValue()
     * at the end of your own method.
     */
    virtual void setValue( int newValue );


    // Access methods

    int 		usedSize()           	{ return _usedSize;		}
    int 		totalFreeSize()      	{ return _totalFreeSize;	}
    int 		newPartSize()           { return _newPartSize;		}
    int 		minNewPartSize()	{ return _minNewPartSize;	}
    int			maxNewPartSize()	{ return _totalFreeSize - _minFreeSize;    }
    int 		minFreeSize()        	{ return _minFreeSize;			   }
    int 		maxFreeSize()        	{ return _totalFreeSize - _minNewPartSize; }
    int			remainingFreeSize()	{ return _totalFreeSize - _newPartSize;    }
    const YCPString 	usedLabel()          	{ return _usedLabel;		}
    const YCPString 	freeLabel()          	{ return _freeLabel;		}
    const YCPString 	newPartLabel()         	{ return _newPartLabel;		}
    const YCPString 	freeFieldLabel()     	{ return _freeFieldLabel;	}
    const YCPString 	newPartFieldLabel()    	{ return _newPartFieldLabel;	}


    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     *
     * This widget is a special case: It has several subwidgets that might get
     * a keyboard shortcut ( yet none of them can be set later with
     * UI::ChangeWidget() ). It ( currently ) cannot be treated properly in the
     * shortcut checker / shortcut conflict resolver. This function is
     * overwritten here merely for the sake of completeness.
     */
    const char *shortcutProperty() { return ( const char * ) 0; }

    /**
     * The name of the widget property that will return user input.
     * Inherited from YWidget.
     **/
    const char *userInputProperty() { return YUIProperty_Value; }


private:

    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );


    int 			_usedSize;
    int 			_totalFreeSize;
    int 			_newPartSize;
    int 			_minNewPartSize;
    int 			_minFreeSize;
    const YCPString &	_usedLabel;
    const YCPString &	_freeLabel;
    const YCPString &	_newPartLabel;
    const YCPString &	_freeFieldLabel;
    const YCPString &	_newPartFieldLabel;
};


#endif // YPartitionSplitter_h
