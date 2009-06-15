
/*
  DBusMsg
*/

#ifndef DBUSMSG_H
#define DBUSMSG_H

#include <string>
#include <dbus/dbus.h>

class YCPValue;

// DBusMessage wrapper
class DBusMsg
{

    public:

	DBusMsg();
	~DBusMsg();

	// copying functions
	DBusMsg(const DBusMsg &);
	DBusMsg &operator = (const DBusMsg &);

	// create a method call
	void createCall(const std::string &service, const std::string &object,
	    const std::string &interface, const std::string &method);

	// create a reply
	void createReply(const DBusMsg &m);

	// create an error (exception)
	void createError(const DBusMsg &m, const std::string &error_msg,
	    // use the generic code by default
	    const std::string &error_code = std::string(DBUS_ERROR_FAILED));

	bool addString(const std::string &val);
	bool addInt64(dbus_int64_t val);
	bool addInt32(dbus_int32_t val);
	bool addBoolean(bool val);
	bool addDouble(double val);

	bool addYCPValue(const YCPValue &val);
	bool addValue(const YCPValue &val);

	YCPValue getYCPValue(int index) const;

	bool isMethodCall(const std::string &interface, const std::string &method) const;
	int arguments() const;
	int type() const;
	bool empty() const;

	DBusMessage *getMessage() const;
	void setMessage(DBusMessage *message);

	std::string interface() const;
	std::string method() const;
	std::string path() const;
	std::string sender() const;

	static const char *YCPValueSignature();

    private:

	bool addValue(int type, void* data);
	bool addValue(int type, void* data, DBusMessageIter *i);
	bool addValueAt(const YCPValue &val, DBusMessageIter *i, bool bsv_encoding = true);
	bool addYCPValue(const YCPValue &v, DBusMessageIter *i);
	bool addYCPValueRaw(const YCPValue &val, DBusMessageIter *i);
	void release();
	DBusMessage *msg;

	int typeInt(const YCPValue &val) const;
	std::string typeStr(const YCPValue &val, bool bsv_enc = true) const;

	YCPValue getYCPValue(DBusMessageIter *it) const;
	YCPValue getYCPValueRaw(DBusMessageIter *it, const std::string &ycp_type = std::string()) const;
};

#endif

