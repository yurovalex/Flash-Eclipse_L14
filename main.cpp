#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cmath>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;



class guiDialog
{
public:
    guiDialog(GLFWwindow *w);
    void CreateGuiDialog();
    void ShowGui();
    auto GetParam() const
    {
        return objPar;
    }


    virtual ~guiDialog();
private:
    GLFWwindow* window;

    struct PropertiesObject
    {
        PropertiesObject(float a, float b, float c):
            valueRed(a), valueGreen(b), valueBlue(c)
        {}
        float valueRed;
        float valueGreen;
        float valueBlue;
    } objPar;

};

guiDialog::guiDialog(GLFWwindow *w):window(w), objPar{0.1,0.5,0.0}
{
    //Инициализация интерфейса ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void guiDialog::CreateGuiDialog()
{
    //Запуск нового окна ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //Элементы интерфейса в окне
    if (ImGui::Begin("Control propeties")) {
        ImGui::Text("Flash Model of Color");
        ImGui::SliderFloat("Red", &objPar.valueRed, 0.0f, 1.0f);
        ImGui::SliderFloat("Green", &objPar.valueGreen, 0.0f, 1.0f);
        ImGui::SliderFloat("Blue", &objPar.valueBlue, 0.0f, 1.0f);
    }
    ImGui::End();
}

void guiDialog::ShowGui()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

guiDialog::~guiDialog()
{
    //Освобождение ресурсов ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

class UsingOpenGL
{
private:
    //vertex Shader
    const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    void main()
    {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0)  ;
    }
)";
    //fragment Shader
    const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 ourColor;
    uniform vec2 u_resolution;
    uniform float u_time;

// Redefine below to see the tiling...
//#define SHOW_TILING


void main()
{
    float TAU = 6.28318530718;
    int MAX_ITER = 5;
    float time = u_time * .5+23.0;
    // uv should be the 0-1 uv of texture...
    vec2 uv = gl_FragCoord.xy / u_resolution.xy;

#ifdef SHOW_TILING
        vec2 p = mod(uv*TAU*2.0, TAU)-250.0;
#else
    vec2 p = mod(uv*TAU, TAU)-250.0;
#endif
        vec2 i = vec2(p);
        float c = 1.0;
        float inten = .005;

        for (int n = 0; n < MAX_ITER; n++)
        {
                float t = time * (1.0 - (3.5 / float(n+1)));
                i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
                c += 1.0/length(vec2(p.x / (sin(i.x+t)/inten),p.y / (cos(i.y+t)/inten)));
        }
        c /= float(MAX_ITER);
        c = 1.17-pow(c, 1.4);
        vec3 colour = vec3(pow(abs(c), 8.0));
    colour = clamp(colour + vec3(ourColor.xyz), 0.0, 1.0);

        #ifdef SHOW_TILING
        // Flash tile borders...
        vec2 pixel = 2.0 / u_resolution.xy;
        uv *= 2.0;
        float f = floor(mod(u_time*.5, 2.0)); 	    // Flash value.
        vec2 first = step(pixel, uv) * f;		   	// Rule out first screen pixels and flash.
        uv  = step(fract(uv), pixel);				// Add one line of pixels per tile.
        colour = mix(colour, vec3(1.0, 1.0, 0.0), (uv.x + uv.y) * first.x * first.y); // Yellow line
        #endif
        FragColor = vec4(colour, 1.0);
    }
)";

    public:

    UsingOpenGL() = default;

    auto InitShaders()->decltype(auto)
    {
        // build and compile our shader program
        // ------------------------------------
        // vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        // link shaders
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    auto InitBuffers()->decltype(auto)
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(VAO);
        return;
    }

    int GetShaderProgram() const noexcept
    {
        return shaderProgram;
    }

    virtual ~UsingOpenGL()
    {
        // optional: de-allocate all resources once they've outlived their purpose:
        // ------------------------------------------------------------------------
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);
    }

private:
    unsigned int shaderProgram;
    unsigned int VBO, VAO;
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[18] = {
        1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f,  -1.0f, 0.0f
    };
};

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "L14_FragmentShaders", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    UsingOpenGL obj;
    obj.InitShaders();
    obj.InitBuffers();


    guiDialog myGUI(window);
    glfwSetWindowSize(window, SCR_WIDTH, SCR_HEIGHT);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // be sure to activate the shader before any calls to glUniform
        glUseProgram(obj.GetShaderProgram());

        // update shader uniform
        double  timeValue = glfwGetTime();
        float greenValue = static_cast<float>(sin(timeValue) / 2.0 + 0.5);
        int vertexColorLocation = glGetUniformLocation(obj.GetShaderProgram(), "ourColor");
        int resScreen = glGetUniformLocation(obj.GetShaderProgram(), "u_resolution");
        int countTime = glGetUniformLocation(obj.GetShaderProgram(), "u_time");
        glUniform4f(vertexColorLocation, myGUI.GetParam().valueRed, greenValue, myGUI.GetParam().valueBlue, 1.0f);
        glUniform2f(resScreen,SCR_WIDTH, SCR_HEIGHT);
        glUniform1f (countTime,timeValue);
        // render the triangle
        glDrawArrays(GL_TRIANGLES, 0, 6);


        myGUI.CreateGuiDialog();
        myGUI.ShowGui();
        myGUI.GetParam();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}



