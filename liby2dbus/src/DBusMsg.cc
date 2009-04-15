
/*

    DBusMsg

*/

#include "DBusMsg.h"
#include "YCP.h"
#include <ycp/Parser.h>
#include <ycp/y2log.h>

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

bool DBusMsg::addBoolean(bool val)
{
    return addValue(DBUS_TYPE_INT64, &val);
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

bool DBusMsg::addValue(const YCPValue &val)
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
    bool ret = addValueAt(val, &it);

    return ret;
}

bool DBusMsg::addValueAt(const YCPValue &val, DBusMessageIter *i)
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
	int sz = lst->size();
	int index = 0;

	// use string as fallback for empty map
	std::string list_type(sz ? typeStr(lst->value(0)) : DBUS_TYPE_STRING_AS_STRING);

	// dbus allows only basic complete types in a list
	const std::string valid_list_types(DBUS_TYPE_INT64_AS_STRING DBUS_TYPE_DOUBLE_AS_STRING
	    DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_BOOLEAN_AS_STRING);

	if (valid_list_types.find(list_type) == std::string::npos)
	{
	    y2error("Invalid type of list item (non-basic type): %s, ignoring the list", list_type.c_str());
	    return false;
	}

	DBusMessageIter array_it;

	// open array container
	y2debug("Opening array container");

	dbus_message_iter_open_container(i, DBUS_TYPE_ARRAY, list_type.c_str(), &array_it);
	while(index < sz)
	{
	    y2debug("Adding YCP value at index %d", index);

	    YCPValue list_item = lst->value(index);

	    if (typeStr(list_item) != list_type)
	    {
		y2error("Found different type in list: %s (expected %s) - ignoring item %s",
		    typeStr(list_item), list_type.c_str(),
		    list_item->toString().c_str());
	    }
	    else
	    {
		// add the raw YCP value
		addValueAt(lst->value(index), &array_it);
	    }

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

	YCPMap::const_iterator mit = map.begin();
	// use string as fallback for empty map
	std::string key_type(mit == map.end() ? DBUS_TYPE_STRING_AS_STRING : typeStr(mit->first));

	// dbus allows only basic types as key type in a map
	const std::string valid_key_types(DBUS_TYPE_INT64_AS_STRING DBUS_TYPE_DOUBLE_AS_STRING
	    DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_BOOLEAN_AS_STRING);

	if (valid_key_types.find(key_type) == std::string::npos)
	{
	    y2error("Invalid type of key in map (non-basic type): %s, ignoring the map", key_type.c_str());
	    return false;
	}

	std::string map_signature("{" + key_type + "v}");

	// open array container
	y2debug("Opening DICT container");
	dbus_message_iter_open_container(i, DBUS_TYPE_ARRAY, map_signature.c_str(), &array_it);

	for(YCPMap::const_iterator mit = map.begin(); mit != map.end() ; ++mit)
	{
	    YCPValue key = mit->first;
	    YCPValue val = mit->second;

	    // error: this is a different key type than the announced type
	    if (key_type != typeStr(key))
	    {
		y2error("Found different key type %s (expected %s) - ignoring pair $[ %s : %s ]",
		    typeStr(key), key_type.c_str(),
		    key->toString().c_str(), val->toString().c_str());

		continue;
	    }

	    DBusMessageIter map_item_it;
	    y2debug("Opening map item container");

	    dbus_message_iter_open_container(&array_it, DBUS_TYPE_DICT_ENTRY, 0, &map_item_it);

	    // add the key
	    addValueAt(key, &map_item_it);

	    // add VARIANT container
	    DBusMessageIter var_it2;
	    y2debug("Opening VARIANT container with type %s", typeStr(val));
	    dbus_message_iter_open_container(&map_item_it, DBUS_TYPE_VARIANT, typeStr(val), &var_it2);

	    // add the key
	    addValueAt(val, &var_it2);

	    y2debug("Closing VARIANT container");
	    dbus_message_iter_close_container(&map_item_it, &var_it2);

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

	for(YCPMap::const_iterator mit = map.begin(); mit != map.end() ; ++mit)
	{
	    YCPValue key = mit->first;
	    YCPValue val = mit->second;

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

    y2debug("Opening VARIANT container with type %s", typeStr(val));

    // add VARIANT container
    DBusMessageIter var_it;
    dbus_message_iter_open_container(&sub, DBUS_TYPE_VARIANT, typeStr(val), &var_it);

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


YCPValue DBusMsg::getYCPValueRaw(DBusMessageIter *it, const std::string &ycp_type) const
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

	static const char* block_prefix = "block ";
	static const int block_prefix_len = ::strlen(block_prefix);

	// use ycp_type to return the correct type
	if (ycp_type.empty() || ycp_type == "string")
	    ret = YCPString(s);
	else if (ycp_type == "symbol")
	    ret = YCPSymbol(s);
	else if (ycp_type == "path")
	    ret = YCPPath(s);
	else if (std::string(ycp_type, 0, block_prefix_len) == block_prefix)
	{
	    y2debug("Found YCP block");

	    if (s == NULL || *s == '\0')
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
		YCPValue contents = YCPNull ();
		
		if (!p)
		{
		    y2error("Parse error in YCP code: %s", s);
		}
		else
		{
		    if (p->isBlock ())
		    {
			contents = YCPCode (p);
		    }
		    else
		    {
			contents = p->evaluate (true);
		    }
		}

		ret = !contents.isNull() ? contents : YCPVoid();
	    }
	}
	else
	{
	    y2warning("Unknown STRING data, returning as YCPString");
	    // default is YCPString
	    ret = YCPString(s);
	}
    }
    else if (type == DBUS_TYPE_ARRAY)
    {
	DBusMessageIter sub;
	dbus_message_iter_recurse(it, &sub);

	// DBUS_TYPE_ARRAY is used for YCPList and YCPMap
	if (ycp_type == "list")
	{
	    y2debug("Found an YCPList container");
	    YCPList lst;

	    while (dbus_message_iter_get_arg_type (&sub) != DBUS_TYPE_INVALID)
	    {
		YCPValue list_val = getYCPValue(&sub);
		lst->add(list_val);

		dbus_message_iter_next(&sub);
	    }

	    ret = lst;
	}
	else if (ycp_type == "map")
	{
	    y2debug("Found an YCPMap container");
	    YCPMap map;
	    
	    while (dbus_message_iter_get_arg_type (&sub) != DBUS_TYPE_INVALID)
	    {
		// is it a map or a list?
		if (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_DICT_ENTRY)
		{
		    DBusMessageIter mapit;
		    dbus_message_iter_recurse(&sub, &mapit);

		    // read the key without the header
		    YCPValue key = getYCPValueRaw(&mapit);

		    dbus_message_iter_next(&mapit);

		    // read the value
		    YCPValue val = getYCPValue(&mapit);

		    map->add(key, val);
		}

		dbus_message_iter_next(&sub);
	    }

	    ret = map;
	}
	else if (ycp_type == "term")
	{
	    y2debug("Found an YCPTerm container");

	    YCPList term_list;
	    std::string term_name;

	    int index = 0;
	    while (dbus_message_iter_get_arg_type (&sub) != DBUS_TYPE_INVALID)
	    {
		YCPValue list_val = getYCPValue(&sub);

		if (index == 0)
		{
		    if (!list_val.isNull() && list_val->isString())
		    {
			term_name = list_val->asString()->value();
		    }
		    else
		    {
			y2error("Expecting string (term name) in the list");
			return YCPVoid();
		    }
		}
		else
		{
		    term_list->add(list_val);
		}

		dbus_message_iter_next(&sub);
		index++;
	    }

	    y2debug("Received TERM: name: %s, list: %s", term_name.c_str(), term_list->toString().c_str());

	    YCPTerm term(term_name, term_list);
	    ret = term;
	}
	else if (ycp_type.empty())
	{
	    y2debug("Reading RAW DBus array");

	    // is the container a map or a list?
	    if (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_DICT_ENTRY)
	    {
		y2debug("Found a map");

		YCPMap map;

		while (dbus_message_iter_get_arg_type (&sub) != DBUS_TYPE_INVALID)
		{
		    // is it a map or a list?
		    if (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_DICT_ENTRY)
		    {
			DBusMessageIter mapit;
			dbus_message_iter_recurse(&sub, &mapit);

			// read the key without the header
			YCPValue key = getYCPValueRaw(&mapit);

			dbus_message_iter_next(&mapit);

			// read the value
			YCPValue val = getYCPValueRaw(&mapit);

			map->add(key, val);
		    }

		    dbus_message_iter_next(&sub);
		}

		ret = map;
	    }
	    else
	    {
		y2debug("Found a list");

		YCPList lst;

		while (dbus_message_iter_get_arg_type (&sub) != DBUS_TYPE_INVALID)
		{
		    YCPValue list_val = getYCPValue(&sub);
		    lst->add(list_val);

		    dbus_message_iter_next(&sub);
		}

		ret = lst;
	    }
	}
	else
	{
	    y2error("Unknown container type for DBUS_TYPE_ARRAY: %s", ycp_type.c_str());
	    ret = YCPVoid();
	}
    }
    else if (type == DBUS_TYPE_DOUBLE)
    {
	double d;
	dbus_message_iter_get_basic(it, &d);
	ret = YCPFloat(d);
    }
    else if (type == DBUS_TYPE_INT64)
    {
	dbus_int64_t i;
	dbus_message_iter_get_basic(it, &i);
	ret = YCPInteger(i);
    }
    else if (type == DBUS_TYPE_UINT64)
    {
	// warning: YCPInteger is signed!
	dbus_uint64_t i;
	dbus_message_iter_get_basic(it, &i);
	ret = YCPInteger(i);
    }
    else if (type == DBUS_TYPE_INT32)
    {
	dbus_int32_t i;
	dbus_message_iter_get_basic(it, &i);
	ret = YCPInteger(i);
    }
    else if (type == DBUS_TYPE_UINT32)
    {
	dbus_uint32_t i;
	dbus_message_iter_get_basic(it, &i);
	ret = YCPInteger(i);
    }
    else if (type == DBUS_TYPE_INT16)
    {
	dbus_int16_t i;
	dbus_message_iter_get_basic(it, &i);
	ret = YCPInteger(i);
    }
    else if (type == DBUS_TYPE_UINT16)
    {
	dbus_uint16_t i;
	dbus_message_iter_get_basic(it, &i);
	ret = YCPInteger(i);
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
	    val = getYCPValueRaw(&sub);
	}

	ret = val;
    }
    else
    {
	y2error("Unsupported DBus type: %d (%c)", type, (char)type);
    }

    return ret;
}

YCPValue DBusMsg::getYCPValue(DBusMessageIter *it) const
{
    int type = dbus_message_iter_get_arg_type(it);
    y2debug("Found DBus type: %d (%c)", type, (char)type);

    if (type != DBUS_TYPE_STRUCT)
    {
	YCPValue ret = getYCPValueRaw(it, "");
	if (ret.isNull())
	{
	    ret = YCPVoid();
	}

	y2milestone("Using RAW dbus value '%s' instead of (bsv) YCPValue structure", ret->toString().c_str());

	return ret;
    }

    DBusMessageIter struct_iter;
    dbus_message_iter_recurse(it, &struct_iter);

    type = dbus_message_iter_get_arg_type(&struct_iter);
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
	y2error("Missing nil flag in the response");
	return YCPVoid();
    }

    // read the data type in the header
    dbus_message_iter_next(&struct_iter);

    std::string ycp_type;
    type = dbus_message_iter_get_arg_type(&struct_iter);
    if (type == DBUS_TYPE_STRING)
    {
	const char *str;
	dbus_message_iter_get_basic(&struct_iter, &str);
	y2debug("HEADER: type: %s", str);
	ycp_type = str;
    }
    else
    {
	y2error("Missing datatype flag in the response");
	return YCPVoid();
    }
    
    // read the YCP value in the variant container
    dbus_message_iter_next(&struct_iter);

    type = dbus_message_iter_get_arg_type(&struct_iter);

    if (type != DBUS_TYPE_VARIANT)
    {
	y2error("Expecting VARIANT type in the response");
	return YCPVoid();
    }

    DBusMessageIter variant_iter;
    dbus_message_iter_recurse(&struct_iter, &variant_iter);

    YCPValue ret = getYCPValueRaw(&variant_iter, ycp_type);

    return (received_nil) ? YCPVoid() : ret;
}

YCPValue DBusMsg::getYCPValue(int index) const
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
	while (i < index && dbus_message_iter_next(&it))
	{
	    i++;
	}

	if (i == index)
	{
	    ret = getYCPValue(&it);
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

const char * DBusMsg::typeStr(const YCPValue &val) const
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
	return "a(bsv)";
    }
    else if (val->isBoolean())
    {
	return DBUS_TYPE_BOOLEAN_AS_STRING;
    }
    else if (val->isList())
    {
	return "a(bsv)";
    }
    else if (val->isCode())
    {
	return DBUS_TYPE_STRING_AS_STRING;
    }
    else if (val->isMap())
    {
	YCPMap map = val->asMap();

	// get key type, use string as a fallback for an empty map
	std::string key_type((map.size() > 0) ? typeStr(map.begin()->first) : "s");

	// key of the DBus DICT struct must be a basic type
	// see http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-signatures
	return (std::string(DBUS_TYPE_ARRAY_AS_STRING) + DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING +
	    + key_type.c_str() + "(bsv)" + DBUS_DICT_ENTRY_END_CHAR_AS_STRING).c_str();
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
