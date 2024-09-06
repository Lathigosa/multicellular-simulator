#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include "utilities/queue.h"
#include "utilities/safe_pointer.h"

#include <memory>
#include <vector>

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/menubar.h>
#include <gtkmm/stack.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/image.h>
#include <gtkmm/menu.h>

#include "gui/gui_plane_template.h"

#include "core/simulation_file_object.h"


//class gui_plane : public Gtk::Widget
//{
//public:
//    gui_plane()
//    {
//
//    }
//    gui_plane(const gui_plane& parent)
//    {
//
//    }
//    virtual ~gui_plane() {}
//
//    //virtual const Glib::RefPtr<Gdk::Pixbuf>& get_icon() {return Gdk::Pixbuf())};
//    virtual Gtk::Image get_icon() const {return Gtk::Image("");}
//    virtual const char * get_name() const {return "No name specified.";};
//protected:
//    int var;
//};

class gui_window : public Gtk::Window
{
public:
    gui_window();
    virtual ~gui_window();

    void show_window();

    void hide_window();

    //void add_plane(const std::unique_ptr<gui_plane_template> & plane);

protected:
    Gtk::VBox main_layout;
    Gtk::HBox main_sublayout;


    Gtk::MenuBar main_menu_bar;
    Gtk::MenuItem menu_files;
		Gtk::Menu menu_files_menu;
		Gtk::MenuItem menu_files_open;
    Gtk::MenuItem menu_edit;
    Gtk::MenuItem menu_view;
    Gtk::MenuItem menu_settings;
    Gtk::MenuItem menu_help;
    Gtk::Label label;

    Gtk::Stack main_stack;
    Gtk::Toolbar stack_switcher;
    Gtk::ToolButton tab_cell_view;
    Gtk::Image tab_cell_view_icon;
    Gtk::ToolButton tab_genetic_circuit_view;
    Gtk::Image tab_genetic_circuit_view_icon;
    Gtk::ToolButton tab_neural_network_view;
    Gtk::Image tab_neural_network_view_icon;

    //test_plane test;
    std::vector<std::unique_ptr<gui_plane_template>> planes;
    std::vector<Gtk::Image> tab_images;
    std::vector<Gtk::ToolButton> tab_buttons;

    void on_stack_switcher_pressed(unsigned int tab_index);
	void on_open_file();

	// The currently opened project:
	simulation_file_object sfo;

private:
};

#endif // GUI_WINDOW_H
