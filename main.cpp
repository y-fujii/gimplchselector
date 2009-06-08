/*
	L*C*h* color selector for GIMP
	by y.fujii <y-fujii at mimosa-pudica.net>, public domain
*/

#include <glib-object.h>
#include <libgimpmodule/gimpmodule.h>
#include "selector-lch.hpp"


extern "C" const GimpModuleInfo* gimp_module_query( GTypeModule* ) {
	static const GimpModuleInfo info = {
		GIMP_MODULE_ABI_VERSION,
		const_cast<char*>( "CIE LCh color selector" ),
		const_cast<char*>( "y.fujii <y-fujii at mimosa-pudica.net>" ),
		const_cast<char*>( "v0.2" ),
		const_cast<char*>( "public domain" ),
		const_cast<char*>( "2009" )
	};
	return &info;
}

extern "C" gboolean gimp_module_register( GTypeModule* module ) {
	LchSelectorClass::register_( module );
	return TRUE;
}
