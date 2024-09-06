#ifndef GUI_PLANE_TEMPLATE_H
#define GUI_PLANE_TEMPLATE_H

#include <gtkmm/box.h>
#include <gtkmm/widget.h>
#include <gtkmm/image.h>

class gui_plane_template : public Gtk::Box
{
public:
    gui_plane_template()
    {

    }
    gui_plane_template(const gui_plane_template& from)
    {

    }
    virtual ~gui_plane_template() {}

	virtual void signal_initialize() {}

    virtual Gtk::Image get_icon() const {return Gtk::Image("");}
    virtual const char * get_name() const {return "No name specified.";}
};

#endif // GUI_PLANE_TEMPLATE_H
