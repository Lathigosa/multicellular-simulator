#include "gui_window_main.h"
#include "resources/project_resources.h"
#include <gio/gfile.h>

#include <gtkmm/label.h>

#include "utilities/safe_pointer.h"

#include <iostream>
#include <string>
#include <gtkmm/image.h>

#include "gui_plane_cell_view.h"
#include "gui_plane_neural_network_view.h"

#include "cell_simulator/cell_system.h"

gui_window::gui_window() :  main_layout(),
                            main_sublayout(),
                            main_menu_bar(),
                            menu_files(),
							menu_files_menu(),
							menu_files_open(),
                            menu_edit(),
                            menu_view(),
                            menu_settings(),
                            menu_help(),
                            label("gtk.png"),
                            main_stack(),
                            stack_switcher(),
                            tab_cell_view(),
                            tab_cell_view_icon(),
                            tab_genetic_circuit_view(),
                            tab_genetic_circuit_view_icon(),
                            tab_neural_network_view(),
                            tab_neural_network_view_icon(),
							sfo()
{
    // Add all planes:
    planes.push_back(make_unique<gui_plane_cell_view>(sfo));
    planes.push_back(make_unique<gui_plane_neural_network_view>());

	

    // Cycle through all planes to get the name and image:
    for(unsigned int a=0; a<planes.size(); a++)
    {
        // Add the tab button & icon to the tab button list (toolbar on the left):
        tab_images.push_back(Gtk::Image());
        if (planes[a]->get_icon().get_storage_type() != Gtk::IMAGE_PIXBUF)
            tab_images[a].set("");
        else
            tab_images[a].set(planes[a]->get_icon().get_pixbuf());

        tab_buttons.push_back(Gtk::ToolButton());
        tab_buttons[a].set_label(planes[a]->get_name());
        tab_buttons[a].set_icon_widget(tab_images[a]);
        stack_switcher.append(tab_buttons[a]);

        // Register the signal when the tab button is clicked:
        tab_buttons[a].signal_clicked().connect(sigc::bind<unsigned int>(sigc::mem_fun(*this, &gui_window::on_stack_switcher_pressed), a));

        main_stack.add(*planes[a], std::to_string(a));
    }

    // Set window properties:
    set_default_size(200, 200);
    maximize();
    set_title(res::str::program_title);

    add(main_layout);

    stack_switcher.set_property("orientation", Gtk::ORIENTATION_VERTICAL);
    stack_switcher.set_property("toolbar-style", Gtk::TOOLBAR_ICONS);

    main_sublayout.pack_start(stack_switcher, Gtk::PACK_SHRINK);
    main_sublayout.pack_end(main_stack);

    main_layout.pack_start(main_menu_bar, Gtk::PACK_SHRINK);
    main_layout.pack_start(main_sublayout);

    menu_files.set_label(res::str::menu_files);
    main_menu_bar.append(menu_files);

	menu_files_open.set_label(res::str::menu_files_open);
	menu_files.set_submenu(menu_files_menu);

	menu_files_open.signal_activate().connect(sigc::mem_fun(*this, &gui_window::on_open_file));
	
	menu_files_menu.append(menu_files_open);
	

    show_all();
}

void gui_window::on_open_file()
{
	message_notify("Opened file");
	try
    {
		//sfo.world->running = false;
        sfo.open_file("test_lua_code.lua");
		planes[0]->signal_initialize();

		sfo.world->running = true;
		//for(int i=0; i<8; i++)
		//{
			sfo.world->simulate_all();
		//}
		
		sfo.world->flush();
    }
    catch(const sfo_parsing_exception& e)
    {
		message_notify("LUA parsing error:" << e.what());
    }
}

void gui_window::on_stack_switcher_pressed(unsigned int tab_index)
{
    main_stack.set_visible_child(std::to_string(tab_index), Gtk::StackTransitionType::STACK_TRANSITION_TYPE_CROSSFADE);
}

//void gui_window::add_plane(const std::unique_ptr<gui_plane_template> & plane)
//{
 //   planes.push_back(std::move(plane));

//}

gui_window::~gui_window()
{
    //dtor
}

void gui_window::show_window()
{
    // Use Gtk::Stack!
}

void gui_window::hide_window()
{

}
