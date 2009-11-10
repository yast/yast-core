/*

    DBusMsg

*/

#include "DBusMsg.h"
#include "YCP.h"
#include <ycp/Parser.h>
#include <ycp/y2log.h>
#include <y2util/stringutil.h>
using namespace stringutil;

DBusMsg::DBusMsg() : msg(NULL)
{
}

// create a copy of the DBusMsg object
DBusMsg::DBusMsg(const DBusMsg &orig)
{
    // release the old pointer
    release();

    // store a copy of the DBusMessage pointer
    msg = orig.msg;

    // increase the reference counter
    y2debug("Increasing the ref counter of the DBusMessage");
    dbus_message_ref(msg);
}

// assignment operator
DBusMsg &DBusMsg::operator=(const DBusMsg &orig)
{
    // self assignment check
    if (this == &orig) return *this;

    // release the old message
    release();

    // store a copy of the DBusMessage pointer
    msg = orig.msg;

    // increase the reference counter
    dbus_message_ref(msg);

    return *this;
}

void DBusMsg::createCall(const std::string &service, const std::string &object,
    const std::string &interface, const std::string &method)
{
    release();
    msg = dbus_message_new_method_call(service.c_str(), object.c_str(),
	interface.c_str(), method.c_str());
}

void DBusMsg::createReply(const DBusMsg &m)
{
    release();
    msg = dbus_message_new_method_return(m.getMessage());
}

void DBusMsg::createError(const DBusMsg &m, const std::string &error_msg, const std::string &error_code)
{
    release();
    msg = dbus_message_new_error(m.getMessage(), error_code.c_str(), error_msg.c_str());
}

void DBusMsg::release()
{
    if (msg != NULL)
    {
	y2debug("Decreasing DBusMessage reference counter");
	// decrease the reference counter (release the message if it's 0)
	dbus_message_unref(msg);
    }
}

DBusMsg::~DBusMsg()
{
    release();
}

bool DBusMsg::addValue(int type, void* data)
{
    if (msg != NULL)
    {
	DBusMessageIter it;

	dbus_message_iter_init_append(msg, &it);
	if (dbus_message_iter_append_basic(&it, type, data))
	{
	    return true;
	}
    }

    return false;
}

bool DBusMsg::addValue(int type, void* data, DBusMessageIter *i)
{
    if (msg != NULL && i != NULL && (dbus_message_iter_append_basic(i, type, data)))
    {
	return true;
    }

    return false;
}

bool DBusMsg::addString(const std::string &val)
{
    const char *ptr = val.c_str();
    return addValue(DBUS_TYPE_STRING, &ptr);
}

bool DBusMsg::addInt64(dbus_int64_t val)
{
    return addValue(DBUS_TYPE_INT64, &val);
}

bool DBusMsg::addInt32(dbus_int32_t val)
{
    return addValue(DBUS_TYPE_INT32, &val);
}

bool DBusMsg::addBoolean(dbus_bool_t val)
{
    return addValue(DBUS_TYPE_BOOLEAN, &val);
}

bool DBusMsg::addDouble(double val)
{
    return addValue(DBUS_TYPE_DOUBLE, &val);
}

DBusMessage* DBusMsg::getMessage() const
{
    return msg;
}

void DBusMsg::setMessage(DBusMessage *message)
{
    msg = message;
}

bool DBusMsg::isMethodCall(const std::string &interface, const std::string &method) const
{
    if (msg != NULL)
    {
	return dbus_message_is_method_call(msg, interface.c_str(), method.c_str());
    }

    return false;
}

std::string DBusMsg::path() const
{
    if (msg != NULL)
    {
	const char *p = dbus_message_get_path(msg);
	if (p != NULL)
	    return std::string(p);
    }

    return std::string();
}

bool DBusMsg::empty() const
{
    return msg == NULL;
}

std::string DBusMsg::interface() const
{
    if (msg == NULL)
    {
	return std::string();
    }

    const char *interface = dbus_message_get_interface(msg);

    if (interface != NULL)
    {
	return std::string(interface);
    }

    return std::string();
}

std::string DBusMsg::method() const
{
    if (msg == NULL)
    {
	return std::string();
    }

    const char *member = dbus_message_get_member(msg);

    if (member != NULL)
    {
	return std::string(member);
    }

    return std::string();
}

bool DBusMsg::addYCPValue(const YCPValue &val)
{
    if (val.isNull())
    {
	y2milestone("Ignoring YCPNull value");
	return false;
    }

    y2debug("Adding YCP value: %s", val->toString().c_str());

    // create insert iterator
    DBusMessageIter it;
    dbus_message_iter_init_append(msg, &it);

    // add the value
    bool ret = addYCPValue(val, &it);

    return ret;
}

bool DBusMsg::addValueAs(const YCPValue &val, constTypePtr rettype)
{
    if (val.isNull())
    {
	y2error("Ignoring YCPNull value");
	return false;
    }

    y2debug("Adding YCP value: %s", val->toString().c_str());

    if (!rettype)
    {
	// return as the current type if not specified
	rettype = Type::vt2type(val->valuetype());
    }
    else
    {
	// the required type does not match the value
	if (rettype->matchvalue(val) < 0)
	{
	    y2error("Value %s does not match requested type %s",
		val->toString().c_str(), rettype->toString().c_str());
	    return false;
	}
	else
	{
	    y2debug("Requested type matches the value");
	}
    }


    // create insert iterator
    DBusMessageIter it;
    dbus_message_iter_init_append(msg, &it);

    // add the value
    bool ret = addValueAt(val, &it, rettype);

    return ret;
}

bool DBusMsg::addValueAt(const YCPValue &val, DBusMessageIter *i, constTypePtr rtype)
{
    y2milestone("Returning YCP value as type: %s", rtype->toString().c_str());

    int type = typeInt(val);

    DBusMessageIter variant_it;
    DBusMessageIter *it_backup = i;
    if (rtype && rtype->isAny())
    {
	// open variant container for "any" type
	y2debug("Opening VARIANT container with type %s", typeStr(val, false).c_str());
	dbus_message_iter_open_container(i, DBUS_TYPE_VARIANT, typeStr(val, false).c_str(), &variant_it);

	i = &variant_it;
    }

    if (val->isInteger())
    {
	dbus_int64_t i64 = val->asInteger()->value();
	addValue(type, &i64, i);
    }
    else if (val->isFloat())
    {
	double d = val->asFloat()->value();
	addValue(type, &d, i);
    }
    else if (val->isString())
    {
	const char *str = val->asString()->value().c_str();
	addValue(type, &str, i);
    }
    else if (val->isBoolean())
    {
	dbus_bool_t b = val->asBoolean()->value();
	addValue(type, &b, i);
    }
    // add path and symbol as string
    else if (val->isPath() || val->isSymbol() || val->isTerm() || val->isCode())
    {
	std::string val_str(val->toString());
	const char *str = val_str.c_str();
	// add path as a string
	addValue(type, &str, i);
    }
    else if (val->isList())
    {
	YCPList lst = val->asList();
	std::string list_type("v");
	constTypePtr list_type_ptr = Type::Any;
	
	if (rtype->isList())
	{
	    list_type_ptr = ((constListTypePtr)rtype)->type();
	    y2debug("Found type list<%s>", list_type_ptr->toString().c_str());

	    if (!list_type_ptr->isAny())
	    {
		list_type = YCPTypeSignature(list_type_ptr);
	    }
	}

	DBusMessageIter array_it;

	// open array container
	y2debug("Opening array container with type: %s", list_type.c_str());

	dbus_message_iter_open_container(i, DBUS_TYPE_ARRAY, list_type.c_str(), &array_it);

	int sz = lst->size();
	int index = 0;
	while(index < sz)
	{
	    y2debug("Adding YCP value at index %d", index);

	    // add the raw YCP value
	    addValueAt(lst->value(index), &array_it, list_type_ptr);

	    index++;
	}

	// close array container
	y2debug("Closing array container");
	dbus_message_iter_close_container(i, &array_it);
    }
    else if (val->isMap())
    {
	YCPMap map = val->asMap();

	std::string map_key_type("s");
	std::string map_val_type("v");

	// YCPMap can contain only YCPString, YCPInteger or YCPSymbol as the key
	constTypePtr map_key_type_ptr = Type::String;
	constTypePtr map_val_type_ptr = Type::Any;
	
	if (rtype->isMap())
	{
	    map_key_type_ptr = ((constMapTypePtr)rtype)->keytype();

	    if (map_key_type_ptr->isAny())
	    {
		map_key_type_ptr = Type::String;
	    }

	    map_val_type_ptr = ((constMapTypePtr)rtype)->valuetype();
	    y2debug("Found type map<%s,%s>", map_key_type_ptr->toString().c_str(), map_val_type_ptr->toString().c_str());

	    if (!map_key_type_ptr->isAny())
	    {
		map_key_type = YCPTypeSignature(map_key_type_ptr);
	    }
	    if (!map_val_type_ptr->isAny())
	    {
		map_val_type = YCPTypeSignature(map_val_type_ptr);
	    }
	}

	DBusMessageIter array_it;

	std::string map_signature("{" + map_key_type + map_val_type + "}");

	// open array container
	y2debug("Opening DICT container with signature: %s", map_signature.c_str());
	dbus_message_iter_open_container(i, DBUS_TYPE_ARRAY, map_signature.c_str(), &array_it);

	for(YCPMapIterator mit = map.begin(); mit != map.end() ; ++mit)
	{
	    YCPValue key = mit.key();
	    YCPValue val = mit.value();

	    DBusMessageIter map_item_it;
	    y2debug("Opening map item container");

	    dbus_message_iter_open_container(&array_it, DBUS_TYPE_DICT_ENTRY, 0, &map_item_it);

	    // convert YCPInteger key to string for "any" type
	    if (key->isInteger() && map_key_type_ptr == Type::String)
	    {
		key = YCPString(key->toString());
	    }

	    // add the key
	    addValueAt(key, &map_item_it, map_key_type_ptr);

	    // add the value
	    addValueAt(val, &map_item_it, map_val_type_ptr);

	    // close map item
	    dbus_message_iter_close_container(&array_it, &map_item_it);
	    y2debug("Closing map item container");
	}

	// close array container
	dbus_message_iter_close_container(i, &array_it);
	y2debug("Closing DICT container");
    }
    else
    {
	y2error("Unsupported type %s, value: %s", Type::vt2type(val->valuetype())->toString().c_str(),
	    val->toString().c_str());

	// TODO add as string?
    }

    if (rtype && rtype->isAny())
    {
	// close variant container for "any" type
	y2debug("Closing VARIANT container");
	dbus_message_iter_close_container(it_backup, &variant_it);
    }

    return true;
}

bool DBusMsg::addYCPValueRaw(const YCPValue &val, DBusMessageIter *i)
{
    int type = typeInt(val);

    if (val->isInteger())
    {
	dbus_int64_t i64 = val->asInteger()->value();
	addValue(type, &i64, i);
    }
    else if (val->isFloat())
    {
	double d = val->asFloat()->value();
	addValue(type, &d, i);
    }
    else if (val->isString())
    {
	const char *str = val->asString()->value().c_str();
	addValue(type, &str, i);
    }
    else if (val->isBoolean())
    {
	dbus_bool_t b = val->asBoolean()->value();
	addValue(type, &b, i);
    }
    // add path and symbol as string
    else if (val->isPath() || val->isSymbol())
    {
	std::string val_str = val->toString();
	const char *str = val_str.c_str();
	// add path as a string
	addValue(type, &str, i);
    }
    else if (val->isTerm())
    {
	// send YCPTerm as a list: [ name [args] ]
	YCPTerm t = val->asTerm();

	YCPString term_name(t->name());
	YCPList term_list(t->args());

	int sz = term_list->size();
	int index = 0;

	DBusMessageIter array_it;

	// open array container
	y2debug("Opening array container");
	dbus_message_iter_open_container(i, DBUS_TYPE_ARRAY, "(bsv)", &array_it);

	// add the name
	addYCPValue(term_name, &array_it);

	while(index < sz)
	{
	    y2debug("Adding YCP value at index %d", index);
	    // add an item
	    addYCPValue(term_list->value(index), &array_it);

	    index++;
	}

	// close array container
	y2debug("Closing array container");
	dbus_message_iter_close_container(i, &array_it);
    }
    else if (val->isList())
    {
	YCPList lst = val->asList();
	int sz = lst->size();
	int index = 0;

	DBusMessageIter array_it;

	// open array container
	y2debug("Opening array container");
	dbus_message_iter_open_container(i, DBUS_TYPE_ARRAY, "(bsv)", &array_it);
	while(index < sz)
	{
	    y2debug("Adding YCP value at index %d", index);
	    // add an item
	    addYCPValue(lst->value(index), &array_it);

	    index++;
	}

	// close array container
	y2debug("Closing array container");
	dbus_message_iter_close_container(i, &array_it);
    }
    else if (val->isMap())
    {
	YCPMap map = val->asMap();

	DBusMessageIter array_it;

	// open array container
	y2debug("Opening DICT container");
	dbus_message_iter_open_container(i, DBUS_TYPE_ARRAY, "{(bsv)(bsv)}", &array_it);

	for(YCPMapIterator mit = map.begin(); mit != map.end() ; ++mit)
	{
	    YCPValue key = mit.key();
	    YCPValue val = mit.value();

	    DBusMessageIter map_item_it;
	    y2debug("Opening map item container");

	    dbus_message_iter_open_container(&array_it, DBUS_TYPE_DICT_ENTRY, 0, &map_item_it);

	    // key of the DBus DICT struct must be a basic type, omit the struct header
	    // see http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-signatures
	    addYCPValueRaw(key, &map_item_it);
	    // add the value
	    addYCPValue(val, &map_item_it);

	    // close map item
	    dbus_message_iter_close_container(&array_it, &map_item_it);
	}

	// close array container
	dbus_message_iter_close_container(i, &array_it);
    }
    else if (val->isCode())
    {
	std::string code(val->toString());
	// add the block as string
	const char *str = code.c_str();
	addValue(type, &str, i);
    }
    else
    {
	y2error("Unsupported type %s, value: %s", Type::vt2type(val->valuetype())->toString().c_str(),
	    val->toString().c_str());

	// TODO add as string?
    }

    return true;
}

bool DBusMsg::addYCPValue(const YCPValue &v, DBusMessageIter *i)
{
    YCPValue val = v;
    // create sub iterator for STRUCT container
    DBusMessageIter sub;

    y2debug("Opening STRUCT container");
    // open container
    dbus_message_iter_open_container(i, DBUS_TYPE_STRUCT, NULL, &sub);

    // add nil flag
    dbus_bool_t nil_flag = val.isNull() || val->isVoid();
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_BOOLEAN, &nil_flag);

    y2debug("Added nil flag: %s", nil_flag ? "true" : "false");

    if (nil_flag)
    {
	// insert dummy YCPBoolean(false) just to keep the content
	// consistent with the signature, the value must be ignored
	val = YCPBoolean(false);
    }

    // add variable type
    std::string vt(Type::vt2type(val->valuetype())->toString());
    const char *ycp_type = vt.c_str();

    dbus_message_iter_append_basic(&sub, DBUS_TYPE_STRING, &ycp_type);
    y2debug("Adding variable type %s: %s", ycp_type, val->toString().c_str());

    y2debug("Opening VARIANT container with type %s", typeStr(val).c_str());

    // add VARIANT container
    DBusMessageIter var_it;
    dbus_message_iter_open_container(&sub, DBUS_TYPE_VARIANT, typeStr(val).c_str(), &var_it);

    // add the raw YCP value
    addYCPValueRaw(val, &var_it);

    y2debug("Closing VARIANT container");
    dbus_message_iter_close_container(&sub, &var_it);

    y2debug("Closing STRUCT container");
    // close container
    dbus_message_iter_close_container(i, &sub);

    return true;
}

int DBusMsg::arguments() const
{
    int ret = 0;
    DBusMessageIter it;

    if (msg == NULL)
    {
	return ret;
    }

    // read the parameters
    if (!dbus_message_iter_init(msg, &it))
    {
	return ret;
    }

    // there is at least one argument
    ret = 1;

    // iterate over the other arguments
    while (dbus_message_iter_next(&it))
    {
	ret++;
    }

    y2milestone("Message has %d arguments", ret);

    return ret;
}


// returns NULL if "it" does not point to a string
const char * DBusMsg::getString(DBusMessageIter *it) const
{
    const char *s = NULL;
    if (dbus_message_iter_get_arg_type (it) == DBUS_TYPE_STRING)
    {
	dbus_message_iter_get_basic(it, &s);
    }
    return s;
}

// "it" is the inside iterator
YCPList DBusMsg::getYCPValueList(DBusMessageIter *it, constTypePtr valuetype) const
{
    YCPList lst;

    while (dbus_message_iter_get_arg_type (it) != DBUS_TYPE_INVALID)
    {
	// FIXME this can fail
	YCPValue list_val = getYCPValue(it, valuetype);
	lst->add(list_val);

	dbus_message_iter_next(it);
    }
    return lst;
}

// "it" is the inside iterator
YCPMap DBusMsg::getYCPValueMap(DBusMessageIter *it, constTypePtr keytype, constTypePtr valuetype) const
{
    YCPMap map;

    while (dbus_message_iter_get_arg_type (it) != DBUS_TYPE_INVALID)
    {
	// is it a map or a list?
	if (dbus_message_iter_get_arg_type(it) == DBUS_TYPE_DICT_ENTRY)
	{
	    DBusMessageIter mapit;
	    dbus_message_iter_recurse(it, &mapit);

	    // DICT keys cannot be structs, skip Bsv
	    YCPValue key = getYCPValueRawType(&mapit, keytype);

	    dbus_message_iter_next(&mapit);

	    // read the value
	    YCPValue val = getYCPValue(&mapit, valuetype);

	    map->add(key, val);
	}
	// else FIXME how to signal error

	dbus_message_iter_next(it);
    }
    return map;
}

// returns YCPNull without an error if "it" does not point to an integer/byte
YCPValue DBusMsg::getYCPValueInteger(DBusMessageIter *it) const
{
    int type = dbus_message_iter_get_arg_type(it);
    YCPValue ret;

    switch (type) {
	case DBUS_TYPE_INT64:
	{
	    dbus_int64_t i;
	    dbus_message_iter_get_basic(it, &i);
	    ret = YCPInteger(i);
	    break;
	}
	case DBUS_TYPE_UINT64:
	{
	    // warning: YCPInteger is signed!
	    dbus_uint64_t i;
	    dbus_message_iter_get_basic(it, &i);
	    ret = YCPInteger(i);
	    break;
	}
	case DBUS_TYPE_INT32:
	{
	    dbus_int32_t i;
	    dbus_message_iter_get_basic(it, &i);
	    ret = YCPInteger(i);
	    break;
	}
	case DBUS_TYPE_UINT32:
	{
	    dbus_uint32_t i;
	    dbus_message_iter_get_basic(it, &i);
	    ret = YCPInteger(i);
	    break;
	}
	case DBUS_TYPE_INT16:
	{
	    dbus_int16_t i;
	    dbus_message_iter_get_basic(it, &i);
	    ret = YCPInteger(i);
	    break;
	}
	case DBUS_TYPE_UINT16:
	{
	    dbus_uint16_t i;
	    dbus_message_iter_get_basic(it, &i);
	    ret = YCPInteger(i);
	    break;
	}
	case DBUS_TYPE_BYTE:
	{
	    unsigned char i;
	    dbus_message_iter_get_basic(it, &i);
	    ret = YCPInteger(i);
	    break;
	}
    }
    return ret;
}

// here ycptype is NOT Type::Unspec
YCPValue DBusMsg::getYCPValueRawType(DBusMessageIter *it, constTypePtr ycptype) const
{
    if (ycptype->isAny())
	return getYCPValueRawAny(it);

    YCPValue ret;

    int type = dbus_message_iter_get_arg_type(it);
    bool mismatch = true;

    if (ycptype->isBoolean())
    {
	if (type == DBUS_TYPE_BOOLEAN)
	{
	    bool b;
	    dbus_message_iter_get_basic(it, &b);
	    ret = YCPBoolean(b);
	    mismatch = false;
	}
    }
    else if (ycptype->isInteger())
    {
	ret = getYCPValueInteger(it);
	mismatch = ret.isNull();
    }
    else if (ycptype->isFloat())
    {
	if (type == DBUS_TYPE_DOUBLE)
	{
	    double d;
	    dbus_message_iter_get_basic(it, &d);
	    ret = YCPFloat(d);
	    mismatch = false;
	}
    }
    else if (ycptype->isList())
    {
	// DBUS_TYPE_ARRAY is used for YCPList and YCPMap
	// TODO sending a dict where a list is expected will confusingly complain about value type mismatch
	if (type == DBUS_TYPE_ARRAY)
	{
	    DBusMessageIter sub;
	    dbus_message_iter_recurse(it, &sub);

	    constListTypePtr list_type = (constListTypePtr) ycptype;
	    constTypePtr valuetype = list_type->type();

	    // FIXME this can fail
	    ret = getYCPValueList(&sub, valuetype);
	    mismatch = false;
	}
    }
    else if (ycptype->isMap())
    {
	if (type == DBUS_TYPE_ARRAY)
	{
	    DBusMessageIter sub;
	    dbus_message_iter_recurse(it, &sub);

	    constMapTypePtr map_type = (constMapTypePtr) ycptype;
	    constTypePtr keytype = map_type->keytype();
	    constTypePtr valuetype = map_type->valuetype();

	    ret = getYCPValueMap(&sub, keytype, valuetype);
	    mismatch = false;
	}
    }
    else if (ycptype->isTerm())
    {
	// array with first item being the name
	if (type == DBUS_TYPE_ARRAY) 
	{
	    DBusMessageIter sub;
	    dbus_message_iter_recurse(it, &sub);

	    std::string term_name;
	    YCPList term_list;

	    if (dbus_message_iter_get_arg_type (&sub) != DBUS_TYPE_INVALID)
	    {
		YCPValue ytn = getYCPValue(&sub, Type::String);
		if (!ytn.isNull() && ytn->isString())
		{
		    term_name = ytn->asString()->value();
		    dbus_message_iter_next(&sub);

		    term_list = getYCPValueList(&sub, Type::Unspec);

		    y2debug("Received TERM: name: %s, list: %s", term_name.c_str(), term_list->toString().c_str());

		    ret = YCPTerm(term_name, term_list);
		    mismatch = false;
		}
		else
		{
		    y2error("Expecting string (term name) in the list");
		    return YCPVoid(); // FIXME
		}
	    }
	}
    }
    else if (ycptype->isString())
    {
	const char *s = getString(it);
	if (s != NULL)
	{
	    ret = YCPString(s);
	    mismatch = false;
	}
    }
    else if (ycptype->isSymbol())
    {
	const char *s = getString(it);
	if (s != NULL)
	{
	    ret = YCPSymbol(s);
	    mismatch = false;
	}
    }
    else if (ycptype->isPath())
    {
	const char *s = getString(it);
	if (s != NULL)
	{
	    ret = YCPPath(s);
	    mismatch = false;
	}
    }
    // parsed YCP code, wow. 
    //FIXME does the execution precede auth checks?!
    else if (ycptype->isBlock())
    {
	const char *s = getString(it);
	if (s != NULL)
	{
	    if (*s == '\0')
	    {
		y2warning("The code block is empty");
		ret = YCPVoid();
	    }
	    else
	    {
		// parse the string, recreate the YCPBlock again
		Parser parser(s);
		parser.setBuffered();
		YCodePtr p = parser.parse();

		if (!p)
		{
		    y2error("Parse error in YCP code: %s", s);
		    ret = YCPVoid();
		}
		else
		{
		    ret = p->isBlock ()? YCPCode (p): p->evaluate (true);
		}
	    }
	    mismatch = false;
	}
    }
    else 
    {
	y2error ("Missing code to convert DBus data to YCP type %s",
		 ycptype->toString().c_str());
    }

    if (mismatch)
    {
	string e = form("Data mismatch, "
			"expecting YCP type %s, got DBus type %c",
		 ycptype->toString().c_str(), (char)type);
	throw DBusException(DBUS_ERROR_INVALID_ARGS, e);
    }

    return ret;
}

YCPValue DBusMsg::getYCPValueRawAny(DBusMessageIter *it) const
{
    YCPValue ret;

    int type = dbus_message_iter_get_arg_type(it);
    // TODO support more types
    if (type == DBUS_TYPE_BOOLEAN)
    {
	bool b;
	dbus_message_iter_get_basic(it, &b);
	ret = YCPBoolean(b);
    }
    else if (type == DBUS_TYPE_STRING)
    {
	const char *s;
	dbus_message_iter_get_basic(it, &s);
	ret = YCPString(s);

    }
    else if (type == DBUS_TYPE_ARRAY)
    {
	DBusMessageIter sub;
	dbus_message_iter_recurse(it, &sub);

	// DBUS_TYPE_ARRAY is used for YCPList and YCPMap
	y2debug("Reading RAW DBus array");

	// is the container a map or a list?
	// An empty map is indistinguishable from a list!
	if (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_DICT_ENTRY)
	{
	    y2debug("Found a map");
	    ret = getYCPValueMap(&sub, Type::Any, Type::Unspec);
	}
	else
	{
	    y2debug("Found a list");

	    YCPList lst;

	    while (dbus_message_iter_get_arg_type (&sub) != DBUS_TYPE_INVALID)
	    {
		YCPValue list_val = getYCPValue(&sub, Type::Unspec);
		lst->add(list_val);

		dbus_message_iter_next(&sub);
	    }

	    ret = lst;
	}
    }
    else if (type == DBUS_TYPE_DOUBLE)
    {
	double d;
	dbus_message_iter_get_basic(it, &d);
	ret = YCPFloat(d);
    }
    else if (type == DBUS_TYPE_VARIANT)
    {
	DBusMessageIter sub;
	dbus_message_iter_recurse(it, &sub);

	y2debug("Found a DBus variant");
	YCPValue val;

	// there should be just one value inside the container
	if (dbus_message_iter_get_arg_type (&sub) != DBUS_TYPE_INVALID)
	{
	    val = getYCPValueRawAny(&sub);
	}
	// FIXME YCPNull possible
	ret = val;
    }
    else
    {
	ret = getYCPValueInteger(it);
	if (ret.isNull())
	{
	    string e = form("Unsupported DBus type: %d (%c)", type, (char)type);
	    throw DBusException(DBUS_ERROR_INVALID_ARGS, e);
	}
    }

    return ret;
}

// main getter dispatcher 
YCPValue DBusMsg::getYCPValue(DBusMessageIter *it, constTypePtr ycptype) const
{
    int type = dbus_message_iter_get_arg_type(it);
    y2debug("Found DBus type: %d (%c)", type, (char)type);

    YCPValue ret;
    if (type != DBUS_TYPE_STRUCT)
    {
	if (ycptype->isUnspec())
	    ret = getYCPValueRawAny(it);
	else
	    ret = getYCPValueRawType(it, ycptype);

//	y2milestone("Using RAW dbus value '%s' instead of (bsv) YCPValue structure", ret->toString().c_str());
    }
    else
	ret = getYCPValueBsv(it, ycptype);

    if (ret.isNull())	// TODO unify
    {
	ret = YCPVoid();
    }
    return ret;
}

// "it" must point to a struct
// May throw a DBusException
YCPValue DBusMsg::getYCPValueBsv(DBusMessageIter *it, constTypePtr ycptype) const
{
    DBusMessageIter struct_iter;
    dbus_message_iter_recurse(it, &struct_iter);

    int type = dbus_message_iter_get_arg_type(&struct_iter);
    bool received_nil = false;
    
    // read the nil flag at the beginning
    if (type == DBUS_TYPE_BOOLEAN)
    {
	dbus_bool_t b;
	dbus_message_iter_get_basic(&struct_iter, &b);

	y2debug("Nil flag: %s", b ? "true" : "false");

	if (b)
	{
	    y2debug("HEADER: Received nil value");
	    received_nil = true;
	}
    }
    else
    {
	string e = form("1st field in BSV must be a boolean nil flag (seen '%c')", (char) type);
	throw DBusException(DBUS_ERROR_INVALID_ARGS, e);
    }

    // read the data type in the header
    dbus_message_iter_next(&struct_iter);

    constTypePtr vycptype = Type::Unspec;
    type = dbus_message_iter_get_arg_type(&struct_iter);
    if (type == DBUS_TYPE_STRING)
    {
	const char *str;
	dbus_message_iter_get_basic(&struct_iter, &str);
	y2debug("HEADER: type: %s", str);

	std::string ycp_type = str;
	if (ycp_type == "string" || ycp_type.empty())
	    vycptype = Type::String;
	else if (ycp_type == "symbol")
	    vycptype = Type::Symbol;
	else if (ycp_type == "path")
	    vycptype = Type::Path;
	else if (ycp_type == "list")
	    vycptype = Type::List;
	else if (ycp_type == "map")
	    vycptype = Type::Map;
	else if (ycp_type == "block")
	    vycptype = Type::Block;
	else {
	    string e = form("Dunno how to translate BSV type '%s' to YCP type", str);
	    throw DBusException(DBUS_ERROR_INVALID_SIGNATURE, e);
	}
    }
    else
    {
	string e = form("2nd field in BSV must be a string YCP type name (seen %d, '%c')", type, (char) type);
	throw DBusException(DBUS_ERROR_INVALID_ARGS, e);
    }
    
    // read the YCP value in the variant container
    dbus_message_iter_next(&struct_iter);

    type = dbus_message_iter_get_arg_type(&struct_iter);

    if (type != DBUS_TYPE_VARIANT)
    {
	string e = form("3rd field in BSV must be a variant payload (seen %d, '%c')", type, (char) type);
	throw DBusException(DBUS_ERROR_INVALID_ARGS, e);
    }

    DBusMessageIter variant_iter;
    dbus_message_iter_recurse(&struct_iter, &variant_iter);

    YCPValue ret = getYCPValueRawType(&variant_iter, vycptype);

    return (received_nil) ? YCPVoid() : ret;
}

// the public getter: idx
YCPValue DBusMsg::getYCPValue(int index, constTypePtr ycptype) const
{
    YCPValue ret = YCPNull();

    if (msg != NULL)
    {
	DBusMessageIter it;

	// read the parameters
	if (!dbus_message_iter_init(msg, &it))
	{
	    return ret;
	}

	// there is at least one argument
	int i = 0;

	// iterate over the other arguments
//wasteful API?
	while (i < index && dbus_message_iter_next(&it))
	{
	    i++;
	}

	if (i == index)
	{
	    ret = getYCPValue(&it, ycptype);
	}
	else
	{
	    y2debug("No argument at index %d", i);
	}
    }

    return ret;
}

int DBusMsg::typeInt(const YCPValue &val) const
{
    if (val.isNull())
    {
	y2warning("Ignoring NULL YCPValue");
	return DBUS_TYPE_INVALID;
    }
    else if (val->isInteger())
    {
	// YCPInteger is 64 bits wide
	return DBUS_TYPE_INT64;
    }
    else if (val->isFloat())
    {
	return DBUS_TYPE_DOUBLE;
    }
    else if (val->isString() || val->isTerm() || val->isPath() || val->isSymbol() || val->isCode())
    {
	return DBUS_TYPE_STRING;
    }
    else if (val->isTerm())
    {
	return DBUS_TYPE_ARRAY;
    }
    else if (val->isBoolean())
    {
	return DBUS_TYPE_BOOLEAN;
    }
    else if (val->isList())
    {
	return DBUS_TYPE_ARRAY;
    }
    else if (val->isMap())
    {
	return DBUS_TYPE_ARRAY;
    }
    
    y2warning("Unsuppoerted type");
    return DBUS_TYPE_INVALID;
}

std::string DBusMsg::typeStr(const YCPValue &val, bool bsv_enc) const
{
    if (val.isNull())
    {
	y2warning("Ignoring NULL YCPValue");
	return DBUS_TYPE_INVALID_AS_STRING;
    }
    else if (val->isInteger())
    {
	// YCPInteger is 64 bits wide
	return DBUS_TYPE_INT64_AS_STRING;
    }
    else if (val->isFloat())
    {
	return DBUS_TYPE_DOUBLE_AS_STRING;
    }
    else if (val->isString() || val->isPath() || val->isSymbol())
    {
	return DBUS_TYPE_STRING_AS_STRING;
    }
    else if (val->isTerm())
    {
	return bsv_enc ? "a(bsv)" : "as";
    }
    else if (val->isBoolean())
    {
	return DBUS_TYPE_BOOLEAN_AS_STRING;
    }
    else if (val->isList())
    {
	if (bsv_enc)
	{
	    return "a(bsv)";
	}
	else
	{
	    return "av";
	}
    }
    else if (val->isCode())
    {
	return DBUS_TYPE_STRING_AS_STRING;
    }
    else if (val->isMap())
    {
	YCPMap map = val->asMap();

	// get key type, use string as a fallback for an empty map
	std::string key_type((map.size() > 0) ? typeStr(map.begin().key(), bsv_enc) : "s");

	if (bsv_enc)
	{
	    // key of the DBus DICT struct must be a basic type
	    // see http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-signatures
	    return std::string(DBUS_TYPE_ARRAY_AS_STRING) + DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING +
		+ key_type.c_str() + "(bsv)" + DBUS_DICT_ENTRY_END_CHAR_AS_STRING;
	}
	else
	{
	    return "a{sv}";
	}
    }
    
    y2error("Unsupported type %s, value: %s", Type::vt2type(val->valuetype())->toString().c_str(),
	val->toString().c_str());

    return DBUS_TYPE_INVALID_AS_STRING;
}

std::string DBusMsg::sender() const
{
    if (msg != NULL)
    {
	const char *sender = dbus_message_get_sender(msg);

	if (sender != NULL)
	{
	    return std::string(dbus_message_get_sender(msg));
	}
    }

    return std::string();
}

int DBusMsg::type() const
{
    if (msg != NULL)
    {
	return dbus_message_get_type(msg);
    }

    return DBUS_MESSAGE_TYPE_INVALID;
}

const char *DBusMsg::YCPValueSignature()
{
    return "(bsv)";
}

std::string DBusMsg::YCPTypeSignature(constTypePtr type)
{
    // handle any type specially
    if (type->isAny())
    {
	return "v";
    }

    if (type->isList())
    {
	constTypePtr list_type = ((constListTypePtr)type)->type();
	y2debug("type list<%s>", list_type->toString().c_str());

	std::string list_type_str(YCPTypeSignature(list_type));

	if (list_type_str.empty())
	{
	    throw SignatureException();
	}

	return std::string("a") + list_type_str;
    }

    if (type->isMap())
    {
	constMapTypePtr mt = (constMapTypePtr)type;
	constTypePtr key_type = mt->keytype();
	constTypePtr val_type = mt->valuetype();

	if (key_type->isAny())
	{
	    key_type = Type::String;
	}

	y2debug("type map<%s,%s>", key_type->toString().c_str(), val_type->toString().c_str());

	std::string key_type_str(YCPTypeSignature(key_type));
	std::string val_type_str(YCPTypeSignature(val_type));

	if (key_type_str.empty() || val_type_str.empty())
	{
	    throw SignatureException();
	}

	return std::string("a{") + key_type_str + val_type_str + "}";
    }

    YCPValueType vt = type->valueType();
    std::string ret;

    switch (vt)
    {
	case(YT_VOID) : ret = ""; break;
	case(YT_BOOLEAN) : ret = DBUS_TYPE_BOOLEAN_AS_STRING; break;
	case(YT_INTEGER) : ret = DBUS_TYPE_INT64_AS_STRING; break;
	case(YT_FLOAT) : ret = DBUS_TYPE_DOUBLE_AS_STRING; break;
	case(YT_STRING) : ret = DBUS_TYPE_STRING_AS_STRING; break;
	case(YT_PATH) : ret = DBUS_TYPE_STRING_AS_STRING; break;
	case(YT_SYMBOL) : ret = DBUS_TYPE_STRING_AS_STRING; break;
	case(YT_TERM) : ret = DBUS_TYPE_STRING_AS_STRING; break;
//	case(YT_CODE) : ret = DBUS_TYPE_STRING_AS_STRING; break;

	default : y2error("Type '%s' is not supported", type->toString().c_str()); throw SignatureException();
    }

    return ret;
}

SignatureException::SignatureException()
{
    y2error("Signature exception");
}
