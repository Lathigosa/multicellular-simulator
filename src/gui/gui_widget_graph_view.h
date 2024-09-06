#ifndef GUI_WIDGET_GRAPH_VIEW_H
#define GUI_WIDGET_GRAPH_VIEW_H

#include <vector>

//#include "utilities/queue.h"

//#define GLEW_STATIC
//#include <GL/glew.h> // install libgl by running "sudo apt-get install libgl-dev" or similar.
#include <epoxy/gl.h>

#include <gtkmm/glarea.h>
#include <gtkmm/hvbox.h>


class gui_widget_graph_view : public Gtk::VBox
{
    public:
        gui_widget_graph_view();
        gui_widget_graph_view(const gui_widget_graph_view& from);

        virtual ~gui_widget_graph_view();

        //void add_graph(queue<double> array_double);         // Adds a new graph to display to the widget.
        int add_graph(std::vector<float> &array_float);          // Adds a new graph to display to the widget. The array representing the data is not managed by this object. Its creation and destruction must be done by the user.
        int add_graph(GLuint VBO);                         // Adds a graph to display to the widget from a foreign VBO. This VBO will not be maintained by gui_widget_graph_view. It is expected to be an array of floats.

        void set_data_point_amount(int amount);             // Sets the amount of data points, which determines the size of the graph.

        bool add_data_point(int graph, float data);         // Add data point to a certain graph.
        bool set_data_point(int graph, int position, float data);   // Set data point of a certain graph.

        Gtk::GLArea gl_area;
    protected:


        void realize();
        void unrealize();
        bool render(const Glib::RefPtr<Gdk::GLContext>& /* context */);
        void resize(int width, int height);


    bool button_event(GdkEventButton * release_event);

    private:
        bool wrap_around = true;

        void render_axes();                                 // Renders the axes, if present.
        void render_line_graph();                           // Renders data as a line graph.

        int data_point_amount;                              // Amount of data points, determines the size of the VBO. It should be slightly larger than the amount of data points if the user wants to expand the data set.

        //vector<vector<float>> data_list;                  // A list of pointers to arrays of floats. These pointers are not managed by this object, their creation and destruction must be done by the user.
        std::vector<GLuint> VBO_list;                       // The VBOs containing the data.
        std::vector<int> graph_length;                      // The list of the amount of data points in each graph.
        //std::vector<vec3> graph_color;                            // The color of each VBO graph.

        double x_scale = 0.005;
        double y_scale = 1.0;

        double x_offset = -128.0;
        double y_offset = 0.0;

        GLuint VAO=0;
        GLuint shader_program;
        GLuint shader_program_axes;
        GLuint framebuffer;
        GLuint texture_framebuffer;
        GLuint render_buffer;

        GLuint VBO_axes;

        GLuint attribute_value;
        GLuint uniform_offset_x;
        GLuint uniform_scale_x;

        GLuint attribute_axes_coordinate;
        GLuint uniform_axes_color;
        GLuint uniform_axes_scale;
};

#endif // GUI_WIDGET_GRAPH_VIEW_H
