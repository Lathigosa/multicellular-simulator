#include "gui/gui_plane_cell_view.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "resources/project_resources.h"

#include <iostream>

gui_plane_cell_view::gui_plane_cell_view(simulation_file_object& ref) : cell_viewer(ref),
											graph_viewer(),
                                             main_paned(),
											sfo(ref)
{
    //ctor
    test_label.set_label("HAHAHA");
    //add(test_label);

    main_paned.add1(graph_viewer);
	main_paned.add2(cell_viewer);

    add(main_paned);
    main_paned.show();

    // ==== TEST AREA ===== //
    //cell_viewer.gl_area.signal_realize().connect(sigc::mem_fun(*this, &gui_plane_cell_view::on_gl_area_realized), true);
}



gui_plane_cell_view::~gui_plane_cell_view()
{
    //dtor
}

void gui_plane_cell_view::signal_initialize()
{
	cell_viewer.initialize_renderer();
}

void gui_plane_cell_view::on_gl_area_realized()
{
    //cell_viewer.set_data_point_amount(256);

    //std::vector<float> some_data = {1.0, 0.2, 0.2, 0.4, 0.3, 0.1, 0.8, 1.0, 0.2, 0.2, 0.4, 0.3, 0.1, 0.8, 1.0, 0.2, 0.2, 0.4, 0.3, 0.1};
    //std::cout << cell_viewer.add_graph(some_data);
}

Gtk::Image gui_plane_cell_view::get_icon() const
{
    if (g_file_test(res::img::switcher_cell_view, G_FILE_TEST_EXISTS) == true)
        return Gtk::Image(Gdk::Pixbuf::create_from_file(res::img::switcher_cell_view, 24, 24));

    return Gtk::Image("");
}

const char * gui_plane_cell_view::get_name() const
{
    return res::str::plane_cell_view_name;
}

//void gui_plane_cell_view::on_realize()
//{
//    set_window(get_parent_window());
//    set_realized();
//}
