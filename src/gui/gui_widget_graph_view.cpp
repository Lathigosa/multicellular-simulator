#include "gui_widget_graph_view.h"

#include <iostream>

const char * const source_vertex = "#version 330 core\n"
"attribute vec2 coord2d;"
"attribute float value;"
"varying vec4 f_color;"
"uniform float offset_x;"
"uniform float scale_x;"

"void main(void) {"
  "gl_Position = vec4((float(gl_VertexID) + offset_x) * scale_x, value, 0, 1);"
  "f_color = vec4(value / 2.0 + 0.5, float(gl_VertexID) * 0.1, 1, 1);"
"}";

const char * const source_vertex_axes = "#version 330 core\n"
"attribute vec2 coordinate;"
"varying vec4 f_color;"
"uniform vec2 scale;"
"uniform vec4 color;"

"void main(void) {"
  "gl_Position = vec4(coordinate.x * scale.x, coordinate.y * scale.y, 0, 1);"
  "f_color = color;"
"}";

const char * const source_fragment = "#version 330 core\n"
"varying vec4 f_color;"

"void main(void) {"
    "gl_FragColor = f_color;"
"}";

gui_widget_graph_view::gui_widget_graph_view()
{
    // Connect gl area signals
    gl_area.signal_realize().connect(sigc::mem_fun(*this, &gui_widget_graph_view::realize));
    // Important that the unrealize signal calls our handler to clean up
    // GL resources _before_ the default unrealize handler is called (the "false")
    gl_area.signal_unrealize().connect(sigc::mem_fun(*this, &gui_widget_graph_view::unrealize), false);
    gl_area.signal_render().connect(sigc::mem_fun(*this, &gui_widget_graph_view::render), false);
    gl_area.signal_resize().connect(sigc::mem_fun(*this, &gui_widget_graph_view::resize));
    gl_area.set_hexpand(true);
    gl_area.set_vexpand(true);
    //gl_area.set_auto_render(true);

    add(gl_area);
    gl_area.show();

    // This is necessary to prevent GPU-related crashes when the widget size becomes 0:
    set_size_request(16, 16);
}

gui_widget_graph_view::~gui_widget_graph_view()
{
    //dtor
}

void gui_widget_graph_view::realize()
{
    gl_area.make_current();
    try
    {
        gl_area.throw_if_error();
        //init_buffers();
        //init_shaders();

        /// Initialize shaders:
        GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
        GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

        // Compile the vertex shader:
        glShaderSource(shader_vertex, 1, &source_vertex, nullptr);
        glCompileShader(shader_vertex);

        GLuint shader_vertex_axes = glCreateShader(GL_VERTEX_SHADER);

        glShaderSource(shader_vertex_axes, 1, &source_vertex_axes, nullptr);
        glCompileShader(shader_vertex_axes);

        // Compile the fragment shader:
        glShaderSource(shader_fragment, 1, &source_fragment, nullptr);
        glCompileShader(shader_fragment);

        // Link the program:
        shader_program = glCreateProgram();
        glAttachShader(shader_program, shader_vertex);
        glAttachShader(shader_program, shader_fragment);
        glLinkProgram(shader_program);

        shader_program_axes = glCreateProgram();
        glAttachShader(shader_program_axes, shader_vertex_axes);
        glAttachShader(shader_program_axes, shader_fragment);
        glLinkProgram(shader_program_axes);

        // Fetch the GLuint pointers:
        attribute_value = glGetAttribLocation(shader_program, "value");
        uniform_offset_x = glGetUniformLocation(shader_program, "offset_x");
        uniform_scale_x = glGetUniformLocation(shader_program, "scale_x");

        attribute_axes_coordinate = glGetAttribLocation(shader_program_axes, "coordinate");
        uniform_axes_color = glGetUniformLocation(shader_program_axes, "color");
        uniform_axes_scale = glGetUniformLocation(shader_program_axes, "scale");

        // Delete remains:
        glDeleteShader(shader_vertex);
        glDeleteShader(shader_fragment);
        glDeleteShader(shader_vertex_axes);

        // Set up obligatory VAO:
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        GLint framebuffer_null;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer_null);
        std::cout << "framebuffer " << framebuffer_null << std::endl;

        // TEST ======================================
        // Set up MSAA framebuffer:
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // Create the multisampled color texture:
        glGenTextures(1, &texture_framebuffer);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_framebuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, gl_area.get_width(), gl_area.get_height(), GL_TRUE);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture_framebuffer, 0);

        // Create the also multisampled renderbuffer object for depth and stencil attachments:
        glGenRenderbuffers(1, &render_buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, gl_area.get_width(), gl_area.get_height());
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Create the VBO for the axes:
        glGenBuffers(1, &VBO_axes);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, nullptr, GL_DYNAMIC_DRAW);     // The VBO consists of 8 floats.


        // END TEST ======================================

        // Set up OpenGL:
        glEnable(GL_MULTISAMPLE);

        // TEST:
        set_data_point_amount(2048);

        std::vector<float> some_data = {0};
        for(int a=1; a<256; a++)
        {
            float b = float(a) * 0.1;
            //some_data.push_back(float(b*b)/(1+float(b*b)));
        }
        std::cout << add_graph(some_data) << std::endl;

        for(int a=1; a<1024; a++)
        {
            float b = float(a) * 0.03;
            add_data_point(0, (float(b*b)/(1+float(b*b))));
        }

        // This is a test:
        //add_data_point(0, 0.2);
        add_data_point(0, 0.2);
        add_data_point(0, 0.2);
        add_data_point(0, 0.2);
        add_data_point(0, 0.2);

        // This is a test:
        add_data_point(1, 0.2);
    }
    catch(const Gdk::GLError& gle)
    {
        //cerr << "An error occurred making the context current during realize:" << endl;
        //cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << endl;
    }
    //set_realized();
}

void gui_widget_graph_view::unrealize()
{
    gl_area.make_current();
    try
    {
        gl_area.throw_if_error();

        // Delete buffers and program
        //glDeleteBuffers(1, &m_Vao);
        //glDeleteProgram(m_Program);
    }
    catch(const Gdk::GLError& gle)
    {
        //cerr << "An error occurred making the context current during unrealize" << endl;
        //cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << endl;
    }
}

bool gui_widget_graph_view::render(const Glib::RefPtr<Gdk::GLContext>& context /* context */)
{
    try
    {
        gl_area.throw_if_error();
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        //draw_triangle();

        // Render points:
        render_line_graph();


        glFlush();

        return true;
    }
    catch(const Gdk::GLError& gle)
    {
        //cerr << "An error occurred in the render callback of the GLArea" << endl;
        //cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << endl;
        return false;
    }
}

void gui_widget_graph_view::resize(int width, int height)
{
    if(width <= 0)
        return;

    if(height <= 0)
        return;

    //gl_area.make_current();
    try
    {
        gl_area.throw_if_error();

        glViewport(0, 0, width, height);

        // Resize the framebuffer:
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_framebuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture_framebuffer, 0);

        // Resize the renderbuffer:
        glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

    }
    catch(const Gdk::GLError& gle)
    {
        std::cerr << "An error occurred making the context current during resize." << std::endl;
        //cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << endl;
    }
}

int gui_widget_graph_view::add_graph(std::vector<float> &array_float)
{
    //if(gl_area.get_realized() == false)
    //    return false;

    gl_area.make_current();
    try
    {
        gl_area.throw_if_error();

        GLuint vbo;
        glGenBuffers(1, &vbo);

        VBO_list.push_back(vbo);
        graph_length.push_back(0);          // Set the used size of this graph.

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        int size;
        if(array_float.size() < data_point_amount)
            size = array_float.size();
        else
            size = data_point_amount;

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data_point_amount, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * size, &array_float.at(0));

    } catch(const Gdk::GLError& gle)
    {
        //cerr << "An error occurred making the context current during unrealize" << endl;
        //cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << endl;
        return -1;
    }

    // Return the graph index:
    return VBO_list.size() - 1;
}
int gui_widget_graph_view::add_graph(GLuint VBO)
{
    VBO_list.push_back(VBO);

    // TODO: See if the size of the VBO matters!
    graph_length.push_back(0);

    // Return the graph index:
    return VBO_list.size() - 1;
}

void gui_widget_graph_view::set_data_point_amount(int amount)
{
    // TODO: resize buffers!
    data_point_amount = amount;
}

bool gui_widget_graph_view::add_data_point(int graph, float data)
{
    if(graph >= VBO_list.size())
        return false;

    if(graph_length.at(graph) >= data_point_amount)
    {
        if(wrap_around == false)
            return false;
        else
            graph_length.at(graph) = 0;         // If wrap_around is enabled, then the graph will be finished from the start.
    }


    gl_area.make_current();
    try
    {
        gl_area.throw_if_error();

        glBindBuffer(GL_ARRAY_BUFFER, VBO_list.at(graph));


        glBufferSubData(GL_ARRAY_BUFFER, graph_length.at(graph) * sizeof(float), sizeof(float), &data);

        graph_length.at(graph) += 1;            // Increase the length of the graph by one.

    } catch(const Gdk::GLError& gle)
    {
        //cerr << "An error occurred making the context current during unrealize" << endl;
        //cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << endl;
        return false;
    }

    // Return the graph index:
    return true;
}

bool gui_widget_graph_view::set_data_point(int graph, int position, float data)
{
    if(graph >= VBO_list.size())
        return false;


    gl_area.make_current();
    try
    {
        gl_area.throw_if_error();

        glBindBuffer(GL_ARRAY_BUFFER, VBO_list.at(graph));

        glBufferSubData(GL_ARRAY_BUFFER, position * sizeof(float), sizeof(float), &data);

    } catch(const Gdk::GLError& gle)
    {
        //cerr << "An error occurred making the context current during unrealize" << endl;
        //cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << endl;
        return false;
    }

    // Return the graph index:
    return true;
}

bool gui_widget_graph_view::button_event(GdkEventButton * release_event)
{
    std::cout << "ADDED DATA POINT" << std::endl;
    // This is a test:
    add_data_point(0, 0.2);
}

void gui_widget_graph_view::render_axes()
{
    //gl_area.attach_buffers();
    glUseProgram(shader_program_axes);

    glUniform2f(uniform_axes_scale, 1.0, 1.0);
    glUniform4f(uniform_axes_color, 0.5, 1.0, 1.0, 1.0);

    // Update the VBO for the horizontal and vertical axes:
    GLfloat vertices[8] = {
            0.5 / float(gl_area.get_width()) + x_offset * x_scale,  1.0,  0.5 / float(gl_area.get_width()) + x_offset * x_scale, -1.0,
            1.0,  0.5 / float(gl_area.get_height()), -1.0,  0.5 / float(gl_area.get_height())
        };

    glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);

    glEnableVertexAttribArray(attribute_axes_coordinate);

    glVertexAttribPointer(
            attribute_axes_coordinate,     // attribute
            2,                   // number of elements per vertex, here (x, y)
            GL_FLOAT,            // the type of each element
            GL_FALSE,            // take our values as-is
            0,                   // no space between values
            0                    // use the vertex buffer object
        );

    glDrawArrays(GL_LINES, 0, 4);
}

void gui_widget_graph_view::render_line_graph()
{
    gl_area.attach_buffers();
    GLint framebuffer_standard;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer_standard);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 1. draw scene as normal in multisampled buffers
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);

    // Render all graphs sequentially (TODO: parallelize):
    for(int a=0; a<VBO_list.size(); a++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO_list.at(a));

        glEnableVertexAttribArray(attribute_value);

        glVertexAttribPointer(
            attribute_value,     // attribute
            1,                   // number of elements per vertex, here (value)
            GL_FLOAT,            // the type of each element
            GL_FALSE,            // take our values as-is
            0,                   // no space between values
            0                    // use the vertex buffer object
        );

        glUniform1f(uniform_offset_x, x_offset);
        glUniform1f(uniform_scale_x, x_scale);

        glDrawArrays(GL_LINE_STRIP, 0, data_point_amount);
    }

    render_axes();

    //glReadBuffer(GL_COLOR_ATTACHMENT0);
    //glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // 2. now blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored in screenTexture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //gl_area.attach_buffers();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_standard);
    glBlitFramebuffer(0, 0, gl_area.get_width(), gl_area.get_height(), 0, 0, gl_area.get_width(), gl_area.get_height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}


