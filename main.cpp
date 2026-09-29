#include <iostream>                         // Permite mostrar mensajes y errores en consola
#include <cmath>                            // Permite usar funciones matemáticas como sin y cos
#include <cstddef>                          // Permite usar offsetof para ubicar datos dentro del vértice
#include <glad/glad.h>                      // Carga y permite usar las funciones de OpenGL
#include <GLFW/glfw3.h>                     // Crea la ventana y permite manejar teclado y mouse
#include <glm/glm/glm.hpp>                  // Permite trabajar con vectores y matrices
#include <glm/glm/gtc/matrix_transform.hpp> // Permite usar perspective, lookAt, rotate y transformaciones
#include <glm/glm/gtc/type_ptr.hpp>         // Permite enviar matrices de GLM hacia OpenGL

#define STB_IMAGE_IMPLEMENTATION            // Hace que stb_image implemente sus funciones en este archivo
#include "stb_image.h"                      // Permite cargar imágenes JPG y PNG para usarlas como texturas


// Variables de la cámara
glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);     // Posición inicial de la cámara
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);  // Dirección hacia donde mira la cámara
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);      // Dirección que representa arriba

float yaw = -90.0f;                         // Rotación horizontal de la cámara
float pitch = 0.0f;                         // Rotación vertical de la cámara
float lastX = 400.0f;                       // Última posición X conocida del mouse
float lastY = 400.0f;                       // Última posición Y conocida del mouse
bool firstMouse = true;                     // Evita un salto brusco al mover el mouse por primera vez

float deltaTime = 0.0f;                     // Tiempo transcurrido entre un frame y el siguiente
float lastFrame = 0.0f;                     // Tiempo del frame anterior


// El Vertex Shader recibe los vértices y calcula su posición final
const char* vertexShaderSource = R"(
#version 410 core

layout(location = 0) in vec3 aPos;          // Recibe X, Y y Z de cada vértice
layout(location = 1) in vec2 aTexCoord;     // Recibe U y V de la textura

uniform mat4 uMVP;                           // Uniform que recibe Model, View y Projection combinadas

out vec2 TexCoord;                           // Envía las coordenadas de textura al Fragment Shader

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);   // Transforma el vértice usando la matriz MVP
    TexCoord = aTexCoord;                    // Pasa las coordenadas de textura al Fragment Shader
}
)";


// El Fragment Shader decide el color final usando la textura
const char* fragmentShaderSource = R"(
#version 410 core

in vec2 TexCoord;                            // Recibe las coordenadas U y V
out vec4 FragColor;                          // Guarda el color final de cada fragmento

uniform sampler2D texture1;                  // Uniform que representa la textura

void main()
{
    FragColor = texture(texture1, TexCoord); // Obtiene el color correspondiente de la imagen
}
)";


// Esta estructura guarda toda la información que necesita cada vértice
struct VertexPyramid
{
    GLfloat pos[3];                          // Posición X, Y y Z
    GLfloat texCoord[2];                     // Coordenadas U y V de la textura
};


// Revisa si un shader compiló correctamente y muestra el error si existe
void printShaderLog(GLuint shader, const char* name)
{
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // Pregunta si el shader compiló correctamente

    if (!success)
    {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog); // Obtiene el mensaje de error
        std::cerr << "ERROR: Shader compile failed (" << name << ")\n" << infoLog << std::endl;
    }
}


// Revisa si el Vertex Shader y Fragment Shader se enlazaron correctamente
void printProgramLog(GLuint program)
{
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success); // Pregunta si el programa se enlazó correctamente

    if (!success)
    {
        GLchar infoLog[1024];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog); // Obtiene el mensaje de error
        std::cerr << "ERROR: Program link failed\n" << infoLog << std::endl;
    }
}


// Procesa las teclas WASD para mover la cámara
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); // ESC cierra la ventana

    float cameraSpeed = 2.0f * deltaTime;       // Mantiene la velocidad estable entre frames

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront; // W mueve la cámara hacia adelante

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront; // S mueve la cámara hacia atrás

    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp)); // Calcula la derecha de la cámara

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= cameraRight * cameraSpeed; // A mueve la cámara hacia la izquierda

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += cameraRight * cameraSpeed; // D mueve la cámara hacia la derecha
}


// Esta función se ejecuta cada vez que se mueve el mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = static_cast<float>(xpos); // Guarda la posición inicial X del mouse
        lastY = static_cast<float>(ypos); // Guarda la posición inicial Y del mouse
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX; // Movimiento horizontal del mouse
    float yoffset = lastY - static_cast<float>(ypos); // Movimiento vertical del mouse

    lastX = static_cast<float>(xpos); // Guarda la nueva posición X
    lastY = static_cast<float>(ypos); // Guarda la nueva posición Y

    float sensitivity = 0.1f;         // Controla la sensibilidad del mouse
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;                    // Mouse en X cambia la rotación horizontal
    pitch += yoffset;                  // Mouse en Y cambia la rotación vertical

    if (pitch > 89.0f)
        pitch = 89.0f;                 // Evita girar completamente hacia arriba

    if (pitch < -89.0f)
        pitch = -89.0f;                // Evita girar completamente hacia abajo

    glm::vec3 direction;               // Guarda la nueva dirección de la cámara

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); // Calcula X
    direction.y = sin(glm::radians(pitch));                           // Calcula Y
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)); // Calcula Z

    cameraFront = glm::normalize(direction); // Actualiza hacia dónde mira la cámara
}


int main()
{
    glfwInit(); // Inicializa GLFW

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);                 // Usa OpenGL versión 4.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);                 // Específicamente OpenGL 4.1
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Usa el perfil moderno Core
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Necesario para OpenGL en macOS
    glfwWindowHint(GLFW_DEPTH_BITS, 24);                           // Solicita un buffer de profundidad de 24 bits

    GLFWwindow* window = glfwCreateWindow(800, 800, "Piramide 3D", nullptr, nullptr); // Crea la ventana

    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); // Hace que esta ventana sea el contexto actual de OpenGL

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // GLAD obtiene las funciones de OpenGL
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, 800, 800); // Define el área donde OpenGL dibuja
    glEnable(GL_DEPTH_TEST);     // Activa la prueba de profundidad para superficies visibles
    glDepthFunc(GL_LESS);        // Conserva el fragmento que está más cerca de la cámara

    glfwSetCursorPosCallback(window, mouse_callback);            // Conecta el movimiento del mouse con la cámara
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Captura el mouse como en un videojuego


    // La pirámide tiene 4 caras laterales y una base cuadrada dividida en 2 triángulos
    VertexPyramid pyramid[] =
    {
        // Cara frontal
        {{ 0.0f,  0.8f,  0.0f}, {0.5f, 1.0f}}, // Punta
        {{-0.8f, -0.8f,  0.8f}, {0.0f, 0.0f}}, // Inferior izquierda
        {{ 0.8f, -0.8f,  0.8f}, {1.0f, 0.0f}}, // Inferior derecha

        // Cara derecha
        {{ 0.0f,  0.8f,  0.0f}, {0.5f, 1.0f}}, // Punta
        {{ 0.8f, -0.8f,  0.8f}, {0.0f, 0.0f}}, // Frente derecha
        {{ 0.8f, -0.8f, -0.8f}, {1.0f, 0.0f}}, // Atrás derecha

        // Cara trasera
        {{ 0.0f,  0.8f,  0.0f}, {0.5f, 1.0f}}, // Punta
        {{ 0.8f, -0.8f, -0.8f}, {0.0f, 0.0f}}, // Atrás derecha
        {{-0.8f, -0.8f, -0.8f}, {1.0f, 0.0f}}, // Atrás izquierda

        // Cara izquierda
        {{ 0.0f,  0.8f,  0.0f}, {0.5f, 1.0f}}, // Punta
        {{-0.8f, -0.8f, -0.8f}, {0.0f, 0.0f}}, // Atrás izquierda
        {{-0.8f, -0.8f,  0.8f}, {1.0f, 0.0f}}, // Frente izquierda

        // Primer triángulo de la base
        {{-0.8f, -0.8f,  0.8f}, {0.0f, 1.0f}},
        {{ 0.8f, -0.8f,  0.8f}, {1.0f, 1.0f}},
        {{ 0.8f, -0.8f, -0.8f}, {1.0f, 0.0f}},

        // Segundo triángulo de la base
        {{-0.8f, -0.8f,  0.8f}, {0.0f, 1.0f}},
        {{ 0.8f, -0.8f, -0.8f}, {1.0f, 0.0f}},
        {{-0.8f, -0.8f, -0.8f}, {0.0f, 0.0f}}
    };


    // Crea y compila el Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);               // Crea el Vertex Shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);        // Le pasa su código fuente
    glCompileShader(vertexShader);                                        // Compila el shader
    printShaderLog(vertexShader, "VERTEX");                               // Revisa si hubo errores

    // Crea y compila el Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);           // Crea el Fragment Shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);    // Le pasa su código fuente
    glCompileShader(fragmentShader);                                      // Compila el shader
    printShaderLog(fragmentShader, "FRAGMENT");                           // Revisa si hubo errores

    // Une los shaders en un programa
    GLuint shaderProgram = glCreateProgram();                             // Crea el programa de shaders
    glAttachShader(shaderProgram, vertexShader);                          // Agrega el Vertex Shader
    glAttachShader(shaderProgram, fragmentShader);                        // Agrega el Fragment Shader
    glLinkProgram(shaderProgram);                                         // Enlaza los shaders
    printProgramLog(shaderProgram);                                       // Revisa errores de enlace

    glDeleteShader(vertexShader);                                         // Ya no se necesita separado
    glDeleteShader(fragmentShader);                                       // Ya no se necesita separado

    GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");           // Busca la variable uniform uMVP


    // Configura VAO y VBO
    GLuint VAO, VBO;                                                      // VAO organiza y VBO guarda los vértices
    glGenVertexArrays(1, &VAO);                                           // Genera el VAO
    glGenBuffers(1, &VBO);                                                // Genera el VBO

    glBindVertexArray(VAO);                                               // Activa el VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);                                   // Activa el VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid), pyramid, GL_STATIC_DRAW); // Envía los vértices de CPU a GPU

    GLsizei stride = static_cast<GLsizei>(sizeof(VertexPyramid));          // Tamaño completo de cada vértice

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (GLvoid*)offsetof(VertexPyramid, pos));          // Indica dónde están X, Y y Z
    glEnableVertexAttribArray(0);                                         // Activa el atributo de posición

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          (GLvoid*)offsetof(VertexPyramid, texCoord));     // Indica dónde están U y V
    glEnableVertexAttribArray(1);                                         // Activa el atributo de textura

    glBindBuffer(GL_ARRAY_BUFFER, 0);                                     // Deja de usar el VBO por ahora
    glBindVertexArray(0);                                                 // Deja de usar el VAO por ahora


    // Crea y configura la textura
    GLuint texture;                                                       // Guarda la referencia de la textura
    glGenTextures(1, &texture);                                           // Genera una textura
    glBindTexture(GL_TEXTURE_2D, texture);                                // Activa la textura

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);         // Repite horizontalmente
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);         // Repite verticalmente
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Suaviza al alejarse
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);     // Suaviza al acercarse

    stbi_set_flip_vertically_on_load(true);                               // Voltea la imagen para coincidir con OpenGL

    int textureWidth, textureHeight, channels;                            // Guarda información de la imagen
    unsigned char* data = stbi_load("textura.JPG", &textureWidth,
                                    &textureHeight, &channels, 0);        // Carga textura.JPG

    if (data)
    {
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;               // Decide si la imagen usa RGB o RGBA

        glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth,
                     textureHeight, 0, format, GL_UNSIGNED_BYTE, data);   // Envía los píxeles a OpenGL

        glGenerateMipmap(GL_TEXTURE_2D);                                  // Genera versiones pequeñas de la textura

        std::cout << "Textura cargada: "
                  << textureWidth << " x " << textureHeight << std::endl; // Muestra sus dimensiones
    }
    else
    {
        std::cerr << "ERROR: No se pudo cargar textura.JPG" << std::endl;
    }

    stbi_image_free(data);                                                // Libera la imagen de la memoria de CPU


    glUseProgram(shaderProgram);                                          // Activa el programa de shaders
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);      // Uniform texture1 usa la unidad 0

    glClearColor(0.07f, 0.08f, 0.14f, 1.0f);                             // Define el color del fondo


    // Render loop: se repite mientras la ventana esté abierta
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());           // Obtiene el tiempo actual
        deltaTime = currentFrame - lastFrame;                             // Calcula el tiempo entre frames
        lastFrame = currentFrame;                                         // Guarda el tiempo para el siguiente frame

        processInput(window);                                             // Revisa W, A, S, D y ESC

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);               // Limpia color y profundidad del frame anterior
        glUseProgram(shaderProgram);                                      // Activa los shaders

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);                  // Obtiene el tamaño actual de la ventana
        glViewport(0, 0, width, height);                                  // Ajusta el área de renderizado

        float aspect = height > 0 ? (float)width / (float)height : 1.0f; // Relación ancho/alto

        // Projection define la perspectiva de la escena
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), aspect, 0.1f, 100.0f);                  // FOV, aspecto, near y far

        // View representa la cámara
        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp);                                                    // WASD cambia posición y mouse cambia dirección

        // Model transforma directamente la pirámide
        glm::mat4 model = glm::mat4(1.0f);                               // Empieza con la matriz identidad
        model = glm::rotate(model, currentFrame * 0.5f,
                            glm::vec3(0.0f, 1.0f, 0.0f));                 // Rota la pirámide sobre su propio eje Y

        // Combina las tres matrices y actualiza la variable uniform
        glm::mat4 mvp = projection * view * model;                        // MVP = Projection * View * Model
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));    // Envía uMVP desde CPU al Vertex Shader

        glActiveTexture(GL_TEXTURE0);                                     // Activa la unidad de textura 0
        glBindTexture(GL_TEXTURE_2D, texture);                            // Activa textura.JPG
        glBindVertexArray(VAO);                                           // Activa los vértices de la pirámide

        glDrawArrays(GL_TRIANGLES, 0, 18);                                // Dibuja los 6 triángulos de la pirámide

        glfwSwapBuffers(window);                                          // Muestra el frame terminado
        glfwPollEvents();                                                 // Procesa teclado, mouse y eventos
    }


    // Libera los recursos utilizados
    glDeleteVertexArrays(1, &VAO);                                        // Elimina el VAO
    glDeleteBuffers(1, &VBO);                                             // Elimina el VBO
    glDeleteTextures(1, &texture);                                        // Elimina la textura
    glDeleteProgram(shaderProgram);                                       // Elimina el programa de shaders

    glfwDestroyWindow(window);                                            // Destruye la ventana
    glfwTerminate();                                                      // Finaliza GLFW

    return 0;                                                             // Termina correctamente el programa
}
