#ifndef GUI_PLANE_CELL_VIEW_H
#define GUI_PLANE_CELL_VIEW_H

#include "gui/gui_plane_template.h"

#include "gui/gui_widget_cell_view.h"
#include "gui/gui_widget_graph_view.h"

#include <gtkmm/label.h>
#include <gtkmm/hvbox.h>
#include <gtkmm/paned.h>

#include "core/simulation_file_object.h"

class gui_plane_cell_view : public gui_plane_template
{
public:
    gui_plane_cell_view(simulation_file_object& ref);
    gui_plane_cell_view(const gui_plane_cell_view& parent) = delete;
    virtual ~gui_plane_cell_view();

    Gtk::Image get_icon() const override;
    const char * get_name() const override;

	void signal_initialize() override;
protected:
    //void on_realize();
    gui_widget_cell_view cell_viewer;
	gui_widget_graph_view graph_viewer;

    Gtk::Label test_label;

    Gtk::Paned main_paned;

    //THIS IS A TEST: (TODO): remove:
    void on_gl_area_realized();

	simulation_file_object& sfo;
};

#endif // GUI_PLANE_CELL_VIEW_H
