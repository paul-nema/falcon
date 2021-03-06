#ifndef GTK_ENTRYBUFFER_HPP
#define GTK_ENTRYBUFFER_HPP

#include "modgtk.hpp"


#if GTK_CHECK_VERSION( 2, 18, 0 )

namespace Falcon {
namespace Gtk {

/**
 *  \class Falcon::Gtk::EntryBuffer
 */
class EntryBuffer
    :
    public Gtk::CoreGObject
{
public:

    EntryBuffer( const Falcon::CoreClass*, const GtkEntryBuffer* = 0 );

    static Falcon::CoreObject* factory( const Falcon::CoreClass*, void*, bool );

    static void modInit( Falcon::Module* );

    static FALCON_FUNC init( VMARG );

    static FALCON_FUNC get_text( VMARG );

    static FALCON_FUNC set_text( VMARG );

    static FALCON_FUNC get_bytes( VMARG );

    static FALCON_FUNC get_length( VMARG );

    static FALCON_FUNC get_max_length( VMARG );

    static FALCON_FUNC set_max_length( VMARG );

    //static FALCON_FUNC insert_text( VMARG );

    //static FALCON_FUNC delete_text( VMARG );

    //static FALCON_FUNC emit_deleted_text( VMARG );

    //static FALCON_FUNC emit_inserted_text( VMARG );

};


} // Gtk
} // Falcon

#endif // GTK_CHECK_VERSION( 2, 18, 0 )

#endif // !GTK_ENTRYBUFFER_HPP

// vi: set ai et sw=4:
// kate: replace-tabs on; shift-width 4;
