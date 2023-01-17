#include<GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLClearErrors();\
    x;\
    ASSERT (GLLogCall(#x, __FILE__, __LINE__))\

template< typename T >
std::string int_to_hex(T i)
{
    std::stringstream stream;
    stream << "0x"
        << std::setfill('0') << std::setw(sizeof(T) * 2)
        << std::hex << i;
    return stream.str();
}

static float ColorChanger(float value)
{
    if (value > 1.0f)
        return -0.05f;
    else if (value < 0.0f)
        return 0.05f;
}

static void GLClearErrors()
{
    GLenum glStatus = glGetError();
    while (glStatus != GL_NO_ERROR)
    {
        glStatus = glGetError();
    }
}

static bool GLLogCall(const char* method, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error](" << int_to_hex(error) << "):" << method << " "
            << file << " " << line << std::endl;
        return false;
    }
    return true;
}

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

enum class ShaderType
{
    NONE = -1,
    VERTEX = 0,
    FRAGMENT = 1
};

static ShaderProgramSource ParseShader(const std::string filePath)
{
    std::ifstream stream(filePath);
    std::string line;
    std::stringstream ss[2];

    ShaderType type = ShaderType::NONE;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
    return { ss[0].str(), ss[1].str() };
}

static int compileShader(const std::string& source, unsigned int type)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    // Error handling
    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;
    }
    return id;
}
static unsigned int createShader(const std::string& vertexShader, std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(vertexShader, GL_VERTEX_SHADER);
    unsigned int fs = compileShader(fragmentShader, GL_FRAGMENT_SHADER);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}
int main(void)
{
    GLFWwindow* window;
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(750, 675, "Pretty Page", NULL, NULL);
    if (!window)
    {
        GLCall(glfwTerminate());
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    glewInit();

    std::cout << glewGetString(GLEW_VERSION) << std::endl;
    float positions[] = {
        0.4f, -0.85f,    // 0
        0.85f, -0.4f,    // 1
        0.85f, 0.4f,     // 2
        0.4f, 0.85f,     // 3
        -0.4f, 0.85f,    // 4
        -0.85f, 0.4f,    // 5
        -0.85f, -0.4f,   // 6
        -0.4f, -0.85f,   // 7
    };

    unsigned int indicies[] = {
        0, 1, 2,
        2, 3, 4,
        4, 5, 6,
        6, 7, 0,
        6, 0, 2,
        6, 4, 2
    };

    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, 18 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));

    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 18 * sizeof(unsigned int), indicies, GL_STATIC_DRAW));

    ShaderProgramSource source = ParseShader("res\\shaders\\Basic.shader");

    unsigned int shader = createShader(source.VertexSource, source.FragmentSource);
    GLCall(glUseProgram(shader));

    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    ASSERT(location != 1);

    GLCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));

    /* Loop until the user closes the window */
    float r = 0.0f;
    float g = 0.2f;
    float b = 0.6f;
    float r_incriment = 0.05f;
    float g_incriment = 0.02f;
    float b_incriment = 0.09f;
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        GLCall(glUniform4f(location, r, g, b, 1.0f));
        GLCall(glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, nullptr));

        //incriment = ColorChanger(r);
       /* incriment = ColorChanger(g);
        g += incriment;
        incriment = ColorChanger(b);
        b += incriment;*/

        if (r > 1.0f)
            r_incriment = -0.05f;
        else if (r < 0.0f)
            r_incriment = 0.05f;

        if (g > 1.0f)
            g_incriment = -0.02f;
        else if (g < 0.0f)
            g_incriment = 0.02f;

        if (b > 1.0f)
            b_incriment = -0.01f;
        else if (b < 0.0f)
            b_incriment = 0.01f;

        r += r_incriment;
        g += g_incriment;
        b += b_incriment;

        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());
    }
    GLCall(glDeleteProgram(shader));

    glfwTerminate();
    return 0;
}