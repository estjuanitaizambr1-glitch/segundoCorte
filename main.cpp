#include <iostream>          // Para mostrar mensajes en consola
#include <cmath>             // Para usar sin, cos, tan y sqrt
#include <glad/glad.h>       // Permite usar funciones de OpenGL
#include <GLFW/glfw3.h>      // Sirve para crear la ventana y manejar eventos

// Este shader se encarga de la posición de los triángulos
const char* vertexShaderSource = R"(
#version 410 core

layout(location = 0) in vec3 aPos; // Recibe la posición X, Y y Z de cada vértice

uniform mat4 model;       // Controla tamaño, giro y posición del triángulo
uniform mat4 view;        // Representa la cámara
uniform mat4 projection;  // Crea la perspectiva

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0); // Junta objeto, cámara y perspectiva
}
)";

// Este shader se encarga del color
const char* fragmentShaderSource = R"(
#version 410 core

out vec4 FragColor; // Aquí queda guardado el color final

uniform vec4 color; // Recibe el color desde C++

void main()
{
    FragColor = color; // Aplica el color al triángulo
}
)";

// Esta función crea la perspectiva
void perspective(float* m, float fov, float aspect, float n, float f)
{
    float t = tan(fov / 2.0f); // Calcula el campo de visión

    for (int i = 0; i < 16; i++)
        m[i] = 0.0f; // Primero pone toda la matriz en cero

    m[0] = 1.0f / (aspect * t);      // Perspectiva horizontal
    m[5] = 1.0f / t;                 // Perspectiva vertical
    m[10] = -(f + n) / (f - n);      // Profundidad
    m[11] = -1.0f;                   // Activa el efecto de perspectiva
    m[14] = -(2.0f * f * n) / (f - n); // Usa el plano cercano y lejano
}

// Esta función crea la cámara
void lookAt(float* m,
            float ex, float ey, float ez,
            float cx, float cy, float cz,
            float ux, float uy, float uz)
{
    // ex, ey, ez = posición de la cámara
    // cx, cy, cz = punto hacia donde mira
    // ux, uy, uz = dirección que se considera arriba

    float fx = cx - ex;
    float fy = cy - ey;
    float fz = cz - ez;

    float fl = sqrt(fx * fx + fy * fy + fz * fz); // Longitud del vector de mirada
    fx /= fl;
    fy /= fl;
    fz /= fl; // Normaliza el vector

    float sx = fy * uz - fz * uy;
    float sy = fz * ux - fx * uz;
    float sz = fx * uy - fy * ux;

    float sl = sqrt(sx * sx + sy * sy + sz * sz); // Longitud del vector lateral
    sx /= sl;
    sy /= sl;
    sz /= sl; // Normaliza el vector lateral

    float rx = sy * fz - sz * fy;
    float ry = sz * fx - sx * fz;
    float rz = sx * fy - sy * fx;

    // Arma la matriz de cámara
    m[0] = sx;   m[4] = sy;   m[8]  = sz;   m[12] = -(sx * ex + sy * ey + sz * ez);
    m[1] = rx;   m[5] = ry;   m[9]  = rz;   m[13] = -(rx * ex + ry * ey + rz * ez);
    m[2] = -fx;  m[6] = -fy;  m[10] = -fz;  m[14] =  (fx * ex + fy * ey + fz * ez);
    m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}

// Esta función crea la matriz de cada triángulo
void model(float* m, float scale, float z, float angle)
{
    // scale = tamaño del triángulo
    // z = qué tan adelante o atrás está
    // angle = cuánto ha girado en Z

    float c = cos(angle);
    float s = sin(angle);

    for (int i = 0; i < 16; i++)
        m[i] = 0.0f; // Limpia la matriz

    // Esto hace el giro sobre Z y también aplica la escala
    m[0] =  c * scale;
    m[1] =  s * scale;
    m[4] = -s * scale;
    m[5] =  c * scale;

    m[10] = 1.0f; // Mantiene el eje Z normal
    m[14] = z;    // Mueve el triángulo en profundidad
    m[15] = 1.0f; // Completa la matriz
}

int main()
{
    glfwInit(); // Inicia GLFW

    // Esta es la versión que te funciona en Mac
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Necesario en macOS

    // Crea la ventana
    GLFWwindow* window = glfwCreateWindow(800, 800, "Tres Triangulos", NULL, NULL);

    // Revisa si la ventana sí se creó
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); // Activa la ventana actual

    // Carga GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 800); // Área donde se va a dibujar

    glEnable(GL_DEPTH_TEST); // Hace que OpenGL sepa cuál triángulo está adelante y cuál atrás

    glEnable(GL_BLEND); // Activa la transparencia
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Mezcla el color con lo de atrás

    // Estos 3 puntos forman un triángulo
    GLfloat vertices[] =
    {
        -0.5f, -0.3f, 0.0f, // Punto inferior izquierdo
         0.5f, -0.3f, 0.0f, // Punto inferior derecho
         0.0f,  0.6f, 0.0f  // Punto superior
    };

    // Crea el Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Le pasa el código
    glCompileShader(vertexShader); // Lo compila

    // Crea el Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // Le pasa el código
    glCompileShader(fragmentShader); // Lo compila

    // Crea el programa y une los shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Ya no hacen falta los shaders por separado
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // VAO organiza y VBO guarda los vértices
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO); // Activa el VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Activa el VBO

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Manda los vértices a la GPU

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Le dice a OpenGL cómo leer los vértices
    glEnableVertexAttribArray(0); // Activa ese atributo

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Deja de usar el VBO por ahora
    glBindVertexArray(0); // Deja de usar el VAO por ahora

    // Busca las variables dentro del shader
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

    // Crea la matriz de perspectiva
    float proj[16];
    perspective(proj, 45.0f * 3.1416f / 180.0f, 1.0f, 0.1f, 100.0f);

    // Bucle principal
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.07f, 0.08f, 0.14f, 1.0f); // Color del fondo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpia color y profundidad

        glUseProgram(shaderProgram); // Activa el shader

        float t = (float)glfwGetTime(); // Tiempo que lleva abierto el programa

        // Esto hace que la cámara gire en círculo despacito
        float radius = 5.0f;
        float camX = sin(t * 0.25f) * radius;
        float camZ = cos(t * 0.25f) * radius - 1.5f;

        float view[16];

        // Crea la cámara
        lookAt(
            view,
            camX, 0.5f, camZ,   // Dónde está la cámara
            0.0f, 0.0f, -1.5f,  // Hacia dónde mira
            0.0f, 1.0f, 0.0f    // Qué dirección es arriba
        );

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view); // Manda la cámara
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, proj); // Manda la perspectiva

        glBindVertexArray(VAO); // Activa los vértices

        float M[16]; // Aquí se guarda la matriz de cada triángulo

        // Triángulo grande
        model(M, 1.0f, 0.0f, t * 0.30f); // Grande, adelante y girando en Z
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, M);
        glUniform4f(colorLoc, 1.0f, 0.62f, 0.76f, 0.60f); // Rosado con transparencia
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Triángulo mediano
        model(M, 0.7f, -1.5f, t * 0.30f); // Mediano y más atrás
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, M);
        glUniform4f(colorLoc, 0.75f, 0.60f, 0.95f, 0.60f); // Morado con transparencia
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Triángulo pequeño
        model(M, 0.4f, -3.0f, t * 0.30f); // Pequeño y más al fondo
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, M);
        glUniform4f(colorLoc, 0.55f, 0.80f, 1.0f, 0.60f); // Azul con transparencia
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window); // Muestra lo dibujado
        glfwPollEvents(); // Revisa eventos
    }

    glDeleteVertexArrays(1, &VAO); // Borra el VAO
    glDeleteBuffers(1, &VBO); // Borra el VBO
    glDeleteProgram(shaderProgram); // Borra el programa

    glfwDestroyWindow(window); // Cierra la ventana
    glfwTerminate(); // Cierra GLFW

    return 0;
}
