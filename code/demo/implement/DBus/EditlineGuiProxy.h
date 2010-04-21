
#ifndef DBUS_EDITLINEGUIPROXY_H_
#define DBUS_EDITLINEGUIPROXY_H_

#include <string>
#include "EditlineGuiProxy.dbusproxy.h"

namespace casa {

    class EditlineGuiProxy :
	private edu::nrao::casa::editlinegui_proxy,
	public DBus::IntrospectableProxy,
	public DBus::ObjectProxy {

    public:

	static const char **execArgs( ) {
	    static const char *args[] = { "dDBus", (char*) 0 };
	    return args;
	}

	static std::string dbusName( ) { return "editlinegui"; }

	EditlineGuiProxy( const std::string &name=dbusName( ) ) : 
		DBus::ObjectProxy( DBusSession::instance().connection( ),
				   dbus::object(name).c_str(), 
				   dbus::path(name).c_str() ) { }

	std::string get( ) { return edu::nrao::casa::editlinegui_proxy::get( ); }
	void set( const std::string &txt ) { edu::nrao::casa::editlinegui_proxy::set( txt ); }
    };
}

#endif
