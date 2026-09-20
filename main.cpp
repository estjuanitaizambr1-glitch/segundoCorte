#include <iostream>                         // Permite mostrar mensajes y errores en consola
#include <glad/glad.h>                      // Carga y permite usar las funciones de OpenGL
#include <GLFW/glfw3.h>                     // Crea la ventana y permite manejar sus eventos

#include <glm/glm/glm.hpp>                  // Permite trabajar con vectores y matrices
#include <glm/glm/gtc/matrix_transform.hpp> // Permite usar perspective, lookAt y rotate
#include <glm/glm/gtc/type_ptr.hpp>          // Permite enviar matrices GLM a OpenGL

#include <cstddef>                           // Permite usar offsetof


// Vertex Shader: se encarga principalmente de calcular la posición de cada vértice
const char* vertexShaderSource = R"(
#version 410 core

layout(location = 0) in vec3 aPos;           // Recibe la posición X, Y y Z del vértice
layout(location = 1) in vec4 aColor;         // Recibe el color R, G, B y Alpha del vértice

uniform mat4 uMVP;                            // Matriz que combina Model, View y Projection

out vec4 vertexColor;                         // Variable que envía el color al Fragment Shader

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);    // Transforma la posición usando la matriz MVP
    vertexColor = aColor;                     // Pasa el color del vértice al Fragment Shader
}
)";


// Fragment Shader: se encarga de decidir el color final de cada fragmento
const char* fragmentShaderSource = R"(
#version 410 core

in vec4 vertexColor;                          // Recibe el color enviado por el Vertex Shader
out vec4 FragColor;                           // Guarda el color final que aparecerá en pantalla

void main()
{
    FragColor = vertexColor;                  // Aplica el color recibido al fragmento
}
)";


// Esta función revisa si un shader tuvo errores al compilar
void printShaderLog(GLuint shader, const char* name)
{
    GLint success = 0;                        // Guarda si la compilación fue correcta o no

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // Consulta el estado de compilación

    if (!success)                             // Entra solamente si ocurrió un error
    {
        GLchar infoLog[1024];                 // Aquí se guarda el mensaje del error

        glGetShaderInfoLog(
            shader,
            sizeof(infoLog),
            nullptr,
            infoLog
        );

        std::cerr << "ERROR: Shader compile failed ("
                  << name << ")\n"
                  << infoLog << std::endl;    // Muestra el error en consola
    }
}


// Esta función revisa si hubo errores al unir los shaders
void printProgramLog(GLuint program)
{
    GLint success = 0;                        // Guarda si el programa se enlazó correctamente

    glGetProgramiv(program, GL_LINK_STATUS, &success); // Consulta el estado del programa

    if (!success)                             // Entra solamente si hubo un error
    {
        GLchar infoLog[1024];                 // Guarda el mensaje del error

        glGetProgramInfoLog(
            program,
            sizeof(infoLog),
            nullptr,
            infoLog
        );

        std::cerr << "ERROR: Program link failed\n"
                  << infoLog << std::endl;    // Muestra el error en consola
    }
}


// Esta estructura define qué información tiene cada vértice
struct VertexTriangle
{
    GLfloat pos[3];                           // Posición: X, Y, Z
    GLfloat color[4];                         // Color: R, G, B, Alpha
};

// Crea otro nombre para la estructura siguiendo la organización del profesor
using Vertex3angle = VertexTriangle;


int main()
{
    glfwInit();                               // Inicializa GLFW antes de crear la ventana


    // Configura la versión de OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);       // Versión mayor: 4
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);       // Versión menor: 1
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Usa el perfil Core
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Necesario para OpenGL en macOS
    glfwWindowHint(GLFW_DEPTH_BITS, 24);                 // Solicita 24 bits para profundidad


    // Aquí se guardan los tres triángulos en un mismo arreglo
    // Cada triángulo necesita 3 vértices, por eso tenemos 9 vértices en total
    // Cada vértice contiene primero XYZ y después RGBA
    Vertex3angle triangles[] =
    {
        // Triángulo grande rosado: está adelante porque Z = 0
        {{-0.50f, -0.30f,  0.0f}, {1.00f, 0.62f, 0.76f, 0.60f}}, // Vértice inferior izquierdo
        {{ 0.50f, -0.30f,  0.0f}, {1.00f, 0.62f, 0.76f, 0.60f}}, // Vértice inferior derecho
        {{ 0.00f,  0.60f,  0.0f}, {1.00f, 0.62f, 0.76f, 0.60f}}, // Vértice superior

        // Triángulo mediano morado: está más atrás porque Z = -1.5
        {{-0.35f, -0.21f, -1.5f}, {0.75f, 0.60f, 0.95f, 0.60f}}, // Vértice inferior izquierdo
        {{ 0.35f, -0.21f, -1.5f}, {0.75f, 0.60f, 0.95f, 0.60f}}, // Vértice inferior derecho
        {{ 0.00f,  0.42f, -1.5f}, {0.75f, 0.60f, 0.95f, 0.60f}}, // Vértice superior

        // Triángulo pequeño azul: está todavía más atrás porque Z = -3
        {{-0.20f, -0.12f, -3.0f}, {0.55f, 0.80f, 1.00f, 0.60f}}, // Vértice inferior izquierdo
        {{ 0.20f, -0.12f, -3.0f}, {0.55f, 0.80f, 1.00f, 0.60f}}, // Vértice inferior derecho
        {{ 0.00f,  0.24f, -3.0f}, {0.55f, 0.80f, 1.00f, 0.60f}}  // Vértice superior
    };


    // Crea una ventana de 800 x 800 píxeles
    GLFWwindow* window = glfwCreateWindow(
        800,                                   // Ancho de la ventana
        800,                                   // Alto de la ventana
        "Tres Triangulos",                     // Nombre de la ventana
        nullptr,
        nullptr
    );


    // Revisa si la ventana pudo crearse correctamente
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl; // Muestra el error
        glfwTerminate();                       // Finaliza GLFW
        return -1;                             // Termina el programa indicando error
    }

    glfwMakeContextCurrent(window);            // Hace que esta ventana use el contexto OpenGL


    // GLAD busca y carga las funciones de OpenGL que vamos a utilizar
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl; // Muestra el error
        glfwTerminate();                       // Finaliza GLFW
        return -1;                             // Termina el programa
    }


    glViewport(0, 0, 800, 800);                // Define inicialmente el área donde OpenGL dibuja

    glEnable(GL_DEPTH_TEST);                    // Activa la prueba de profundidad
    glDepthFunc(GL_LESS);                       // El fragmento más cercano gana sobre el más lejano

    glEnable(GL_BLEND);                         // Activa la mezcla de colores para usar transparencia

    // SRC_ALPHA usa el Alpha del objeto y ONE_MINUS_SRC_ALPHA usa lo que queda del fondo
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Crea el objeto que almacenará el Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,                          // Shader al que se le enviará el código
        1,                                     // Cantidad de cadenas de código
        &vertexShaderSource,                   // Código fuente del Vertex Shader
        nullptr
    );

    glCompileShader(vertexShader);             // Compila el Vertex Shader
    printShaderLog(vertexShader, "VERTEX");    // Revisa si hubo errores


    // Crea el objeto que almacenará el Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,                        // Shader al que se le enviará el código
        1,                                     // Cantidad de cadenas
        &fragmentShaderSource,                 // Código fuente del Fragment Shader
        nullptr
    );

    glCompileShader(fragmentShader);           // Compila el Fragment Shader
    printShaderLog(fragmentShader, "FRAGMENT");// Revisa si hubo errores


    // Crea el Shader Program que junta el Vertex Shader y el Fragment Shader
    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);   // Agrega el Vertex Shader al programa
    glAttachShader(shaderProgram, fragmentShader); // Agrega el Fragment Shader al programa
    glLinkProgram(shaderProgram);                  // Une ambos shaders en un solo programa

    printProgramLog(shaderProgram);                // Revisa si hubo errores al enlazarlos

    glDeleteShader(vertexShader);                  // Ya no necesitamos el shader individual
    glDeleteShader(fragmentShader);                // Ya no necesitamos el shader individual


    // Busca la ubicación de la variable uniform uMVP dentro del Vertex Shader
    GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");


    GLuint VAO_tri, VBO_tri;                       // Variables donde se guardarán VAO y VBO

    glGenVertexArrays(1, &VAO_tri);                // Crea un VAO
    glGenBuffers(1, &VBO_tri);                     // Crea un VBO

    glBindVertexArray(VAO_tri);                     // Activa el VAO que vamos a configurar
    glBindBuffer(GL_ARRAY_BUFFER, VBO_tri);         // Activa el VBO como buffer de vértices


    // Copia todos los datos del arreglo triangles desde CPU hacia la GPU
    // Se mandan juntos los 9 vértices con sus posiciones y colores
    glBufferData(
        GL_ARRAY_BUFFER,                           // Es un buffer de vértices
        sizeof(triangles),                         // Tamaño total de todos los datos
        triangles,                                 // Datos que queremos enviar
        GL_STATIC_DRAW                             // Los datos no cambiarán constantemente
    );


    // Stride = distancia en memoria entre el inicio de un vértice y el siguiente
    // sizeof calcula automáticamente el tamaño completo de Vertex3angle
    // Un vértice contiene 3 floats de posición + 4 floats de color = 7 floats
    GLsizei stride3angles = static_cast<GLsizei>(sizeof(Vertex3angle));


    // Explica cómo encontrar la posición XYZ dentro de cada vértice
    glVertexAttribPointer(
        0,                                        // Location 0 = aPos en el Vertex Shader
        3,                                        // Tiene 3 valores: X, Y, Z
        GL_FLOAT,                                 // Cada valor es de tipo float
        GL_FALSE,                                 // No normaliza los valores
        stride3angles,                            // Distancia entre un vértice y el siguiente
        (GLvoid*)offsetof(Vertex3angle, pos)       // Posición donde empieza pos dentro del struct
    );

    glEnableVertexAttribArray(0);                 // Activa el atributo location 0 = posición


    // Explica cómo encontrar el color RGBA dentro de cada vértice
    glVertexAttribPointer(
        1,                                        // Location 1 = aColor en el Vertex Shader
        4,                                        // Tiene 4 valores: R, G, B, Alpha
        GL_FLOAT,                                 // Cada valor es de tipo float
        GL_FALSE,                                 // No normaliza los valores
        stride3angles,                            // Distancia entre un vértice y el siguiente
        (GLvoid*)offsetof(Vertex3angle, color)     // Posición donde empieza color en el struct
    );

    glEnableVertexAttribArray(1);                 // Activa el atributo location 1 = color

    glBindBuffer(GL_ARRAY_BUFFER, 0);             // Desactiva el VBO por ahora
    glBindVertexArray(0);                         // Desactiva el VAO por ahora


    // Define el color que aparece detrás de los triángulos
    // Los valores representan R, G, B y Alpha
    glClearColor(0.07f, 0.08f, 0.14f, 1.0f);


    // El bucle se repite continuamente mientras la ventana siga abierta
    while (!glfwWindowShouldClose(window))
    {
        // Borra el frame anterior antes de dibujar el siguiente
        // COLOR limpia los colores y DEPTH limpia la información de profundidad
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);              // Activa el programa de shaders


        int width = 0;                            // Guardará el ancho actual
        int height = 0;                           // Guardará el alto actual

        // Obtiene el tamaño actual del framebuffer
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);           // Ajusta el dibujo al tamaño de la ventana


        // Aspect = ancho dividido entre alto
        // Evita que los objetos se deformen cuando cambia el tamaño de la ventana
        float aspect = height > 0
            ? static_cast<float>(width) / static_cast<float>(height)
            : 1.0f;


        // PROJECTION determina cómo se representa una escena 3D en la pantalla 2D
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),                  // Campo de visión de 45 grados
            aspect,                               // Relación entre ancho y alto
            0.1f,                                 // Plano cercano: lo mínimo que puede verse
            100.0f                                // Plano lejano: lo máximo que puede verse
        );


        // Obtiene los segundos que han pasado desde que GLFW comenzó
        // Como t cambia continuamente, podemos utilizarlo para crear animación
        float t = static_cast<float>(glfwGetTime());

        float radius = 5.0f;                      // Distancia de la cámara al centro


        // Calcula la posición de la cámara en X y Z
        // sin y cos producen un movimiento circular
        float camX = glm::sin(t * 0.25f) * radius;
        float camZ = glm::cos(t * 0.25f) * radius - 1.5f;


        // VIEW representa la cámara y desde dónde estamos observando la escena
        glm::mat4 view = glm::lookAt(
            glm::vec3(camX, 0.5f, camZ),          // Eye: posición de la cámara
            glm::vec3(0.0f, 0.0f, -1.5f),        // Center: punto hacia donde mira
            glm::vec3(0.0f, 1.0f, 0.0f)          // Up: dirección considerada como arriba
        );


        // MODEL representa las transformaciones que se aplican al objeto
        // Comienza como matriz identidad, es decir, sin transformación
        glm::mat4 model = glm::mat4(1.0f);


        // Rotate modifica Model para hacer girar los tres triángulos
        model = glm::rotate(
            model,                                // Matriz que queremos transformar
            t * 0.30f,                            // Ángulo: aumenta con el tiempo
            glm::vec3(0.0f, 0.0f, 1.0f)          // Eje Z: indica alrededor de qué eje gira
        );


        // Combina las tres matrices necesarias para transformar los vértices
        // El orden es importante: Projection * View * Model
        glm::mat4 mvp = projection * view * model;


        // Envía la matriz MVP desde el programa de C++ hasta el Vertex Shader
        glUniformMatrix4fv(
            mvpLoc,                               // Ubicación del uniform uMVP
            1,                                    // Se envía una sola matriz
            GL_FALSE,                             // No transpone la matriz
            glm::value_ptr(mvp)                   // Convierte la matriz al formato de OpenGL
        );


        glBindVertexArray(VAO_tri);               // Activa el VAO que contiene los triángulos


        // GL_TRIANGLES agrupa los vértices de 3 en 3
        // Vértices 0,1,2 = rosado
        // Vértices 3,4,5 = morado
        // Vértices 6,7,8 = azul
        glDrawArrays(
            GL_TRIANGLES,                         // Dibuja triángulos
            0,                                    // Empieza desde el primer vértice
            9                                     // Dibuja los 9 vértices
        );


        glfwSwapBuffers(window);                  // Muestra en pantalla el frame terminado
        glfwPollEvents();                         // Procesa teclado, mouse y eventos de ventana
    }


    // Libera de la memoria los recursos que creamos
    glDeleteVertexArrays(1, &VAO_tri);            // Elimina el VAO
    glDeleteBuffers(1, &VBO_tri);                 // Elimina el VBO
    glDeleteProgram(shaderProgram);               // Elimina el programa de shaders

    glfwDestroyWindow(window);                    // Destruye la ventana
    glfwTerminate();                              // Finaliza GLFW

    return 0;                                     // Indica que el programa terminó correctamente
}
