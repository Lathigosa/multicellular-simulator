#ifndef GUI_PLANE_NEURAL_NETWORK_VIEW_H
#define GUI_PLANE_NEURAL_NETWORK_VIEW_H

#include "gui/gui_plane_template.h"

#include <gtkmm/label.h>

class gui_plane_neural_network_view : public gui_plane_template
{
public:
    gui_plane_neural_network_view();
    gui_plane_neural_network_view(const gui_plane_neural_network_view& parent);
    virtual ~gui_plane_neural_network_view();

    Gtk::Image get_icon() const override;
    const char * get_name() const override;
protected:
    //void on_realize();

    Gtk::Label test_label;
};

#endif // GUI_PLANE_NEURAL_NETWORK_VIEW_H
