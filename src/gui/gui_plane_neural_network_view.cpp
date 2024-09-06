#include "gui/gui_plane_neural_network_view.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "resources/project_resources.h"

gui_plane_neural_network_view::gui_plane_neural_network_view()
{
    //ctor
    test_label.set_label("HBHBHBHB");
    add(test_label);
}

gui_plane_neural_network_view::gui_plane_neural_network_view(const gui_plane_neural_network_view& parent)
{

}

gui_plane_neural_network_view::~gui_plane_neural_network_view()
{
    //dtor
}

Gtk::Image gui_plane_neural_network_view::get_icon() const
{
    if (g_file_test(res::img::switcher_cell_view, G_FILE_TEST_EXISTS) == true)
        return Gtk::Image(Gdk::Pixbuf::create_from_file(res::img::switcher_neural_network_view, 24, 24));

    return Gtk::Image("");
}

const char * gui_plane_neural_network_view::get_name() const
{
    return res::str::plane_neural_network_view_name;
}

//void gui_plane_neural_network_view::on_realize()
//{
//    set_window(get_parent_window());
//    set_realized();
//}
