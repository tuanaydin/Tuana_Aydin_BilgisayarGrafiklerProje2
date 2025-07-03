/* Tuana Ayd�n
1316200027
Bilgisayar Grafikleri 2.Proje
Kedi ve M�ze Sahnesi
*/
//K�t�hane Tan�mlar�
#define STB_IMAGE_IMPLEMENTATION
#include "stb_easy_font.h"
#include "stb_image.h"
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>


///I��kland�rma ve Normalizasyon i�in vertex shader ve fragment shader�n tan�mlanamas�

//Vertex Shader

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;//texture1

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;//texture1

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;//Texture1
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

//Fragment Shader
//Phong Ayd�nlatma ile sahnede ���k-g�lge efekti uygulanarak ger�ek�ilik sa�land�.
const char* fragmentShaderSource = R"(
//Shader fragment
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 lightPos = vec3(0.0, 0.0, 0.0);
uniform vec3 viewPos = vec3(0.0, 0.0, 3.0);
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
uniform vec3 objectColor = vec3(0.8, 0.4, 0.2);
uniform sampler2D texture1;
uniform bool useTexture;

void main()
{
    // Ambient Ayd�nlatma
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse Ayd�nlatma 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular Ayd�nlatma
    float specularStrength = 0.6;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    
    //Phong Bu �� ayd�nlatman�nda toplam�
    vec3 result = (ambient + diffuse + specular) * objectColor;
    
if (useTexture)
        result *= texture(texture1, TexCoord).rgb;

    FragColor = vec4(result, 1.0);



//vec3 color = texture(texture1, TexCoord).rgb * result;
 //   FragColor = vec4(color, 0.5);
}
)";

//--------Shaderlar--------//

// Ekran Ayarlar�
const unsigned int WIDTH = 1500;
const unsigned int HEIGHT = 1200;

//Asans�r
//Asans�r Bilgileri
bool doorsOpen = false;
float doorAnimTime = 0.0f;
float doorAnimDuration = 1.0f;

//bool elevatorMovingUp = false;
//const float elevatorTargetY = 15.0f; // Hedef kat y�ksekli�i
//const float elevatorSpeed = 2.0f;    // Birim/saniye ��k�� h�z�


bool elevatorMoving = false;
bool elevatorGoingUp = false;

bool elevatorGoingDown = false;
float elevatorY = 0.0f;
float elevatorSpeed = 2.0f;
float elevatorTargetY_Up = 10.1f;
float elevatorTargetY_Down = 0.0f;

float elevatorWaitTime = 0.0f;
const float elevatorWaitDuration = 2.0f;

//Kat sistemi tan�m�(buraya ekle) �at� kat�na da ��kabilmeli
const int maxFloor = 2;
int currentFloor = 0;
float floorHeights[] = { 0.0f, 10.1f, 20.2f };


////Deneme 
// Kamera modlar�

enum CameraMode { FIRST_PERSON, THIRD_PERSON };
CameraMode currentCameraMode = THIRD_PERSON;

// (Kedinin pozisyonu kodda zaten var)
extern glm::vec3 catPosition; // ana tan�m main'in �st�nde


glm::vec3 catPosition = glm::vec3(0.0f, 10.6f, 0.0f);
glm::vec3 catFront = glm::vec3(0.0f, 0.0f, -1.0f);
float catYaw = 0.0f;


//Kap� 
// --- ODA KAPISI animasyonu i�in de�i�kenler ---
bool roomDoorOpen = false;
float roomDoorAnim = 0.0f;
float roomDoorAnimDuration = 1.0f;
float roomDoorMaxOffset = 1.5f;


// Kamera Ayarlar�
glm::vec3 cameraPos = glm::vec3(0.0f, 10.6f, 5.0f);//Kamera konumu 
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);//Kameran�n bakt��� y�n
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);//Kameran�n yukar�s�
float yaw = -90.0f, pitch = 0.0f;//Kameran�n a��s� 
float lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f;//farenin pozisyonu i�in de�erler
bool firstMouse = true;
float deltaTime = 0.0f;	// Bir �nceki frame ile bu frame aras�ndaki zaman
float lastFrame = 0.0f;



//---------Vertex Tan�mlamalar�----------//
float tabloVertices[] = {
    // positions        // normals         // tex coords
    -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // sol �st
    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // sol alt
     1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // sa� alt

    -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // sol �st
     1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // sa� alt
     1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f   // sa� �st
};


//ZemiN Doku vertex noktalar�
float floorVertices[] = {
    // pos                // normal         // tex coords
    -0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 5.0f,
     0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
     0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,  5.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,  5.0f, 0.0f,
    -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 5.0f
};

// K�p vertex (pozisyon + normal)
float cubeVertices[] = {
    // pozisyonlar       // normal vekt�rleri
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
     0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
     0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
     0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

     0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
     0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
     0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
};

//Texture Fonksiyonu Doku y�klenmesi
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    
    
    GLenum format = GL_RGB;
    if (nrComponents == 1)
        format = GL_RED;
    else if (nrComponents == 3)
        format = GL_RGB;
    else if (nrComponents == 4)
        format = GL_RGBA;
    
    
    
    
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Doku yuklenemedi: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}
//UI 
void drawText(float x, float y, const char* text)
{
    char buffer[99999]; // karakter verisi i�in buffer
    int num_quads;

    num_quads = stb_easy_font_print(x, y, (char*)text, NULL, buffer, sizeof(buffer));

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}


// Fonksiyon bildirimi
void framebuffer_size_callback(GLFWwindow* window, int width, int height);//Frame buffer
void processInput(GLFWwindow* window);//Klavye tu�lar�n�n fonksiyon �a�r�s�
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int createShaderProgram();
// 





//Main fonksiyonu
int main()
{
    //GLFW'�n ba�lat�lmas�
    if (!glfwInit())
    {
        printf("GLFW ba�at�lamad�");
        glfwTerminate();
        return -1;
    }
    //OpenGL Versiyonu 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Pencerenin olu�turulmas�
    GLFWwindow* mainwindow = glfwCreateWindow(WIDTH, HEIGHT, "M�zede Tek Bas�na Bir Kedicik Proje Odevi", NULL, NULL);
    if (!mainwindow)
    {
        printf("Pencere olusturulamadi!");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(mainwindow);
    glfwSetFramebufferSizeCallback(mainwindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(mainwindow, mouse_callback);
    glfwSetInputMode(mainwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD y�kle
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("GLAD yuklenemedi!");
        return -1;
    }
    glEnable(GL_DEPTH_TEST);//Derinlik testi (z-buffering) a��l�r. 3D sahnelerde g�rsel do�ruluk
    unsigned int shaderProgram = createShaderProgram();

    // VAO & VBO tan�mlamalar�
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);//poziyon bilgisi
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));//
    glEnableVertexAttribArray(1);


    //Doku tan�mlamalar�
     ////Texturen VAO/VBO Tan�mlamalar�-------------
    unsigned int floorVAO, floorVBO;
    unsigned int floorTexture = loadTexture("angele-kamp--OQbUQce54k-unsplash.jpg");

    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    // aPos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // aNormal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // aTexCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //Dokular�n Y�klenmesi
    //unsigned int textureVanGogh = loadTexture("VanGogh-starry_night.jpg");
    unsigned int textureAbstract = loadTexture("sheldon-liu-FrQKfzoTgsw-unsplash.jpg");
    unsigned int textureAtmosfer = loadTexture("Texturelabs_Atmosphere_147S.jpg");
    unsigned int textureSky= loadTexture("Texturelabs_Sky_146S.jpg");
    unsigned int Binadisi = loadTexture("Texturelabs_Brick_142S.jpg");
    stbi_set_flip_vertically_on_load(true);
    unsigned int textureSky2 = loadTexture("William-Hogarth-An-Election-The-Election-Entertainment-Soane-Museum.jpg");
    unsigned int tablo1 = loadTexture("paul-blenkhorn-832BT5eutOY-unsplash.jpg");
    unsigned int tablo2 = loadTexture("Texturelabs_InkPaint_374S.jpg");
    unsigned int tablo3 = loadTexture("Texturelabs_InkPaint_393S.jpg");
    unsigned int tablo4 = loadTexture("Mona_Lisa.jpg");
    unsigned int leonardo = loadTexture("The-Last-Supper-Restored-Da-Vinci_32x16.jpg");
    unsigned int inci = loadTexture("Johannes_Vermeer_(1632-1675)_-_The_Girl_With_The_Pearl_Earring_(1665).jpg");
    unsigned int korku1 = loadTexture("scream.jpg");
    
    // VAO, VBO olu�tur
    unsigned int artVAO, artVBO;
    glGenVertexArrays(1, &artVAO);
    glGenBuffers(1, &artVBO);

    glBindVertexArray(artVAO);
    glBindBuffer(GL_ARRAY_BUFFER, artVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tabloVertices), tabloVertices, GL_STATIC_DRAW);

    // Pozisyon
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    
    //--------------------ANA D�NG�--------------------------------------
    
    // Ana d�ng�
    while (!glfwWindowShouldClose(mainwindow))
    {
        // Zaman hesapla
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        
        if (elevatorWaitTime > 0.0f) {
            elevatorWaitTime -= deltaTime;
            if (elevatorWaitTime <= 0.0f) {
                elevatorMoving = true;
            }
        }
        //std::cout << "Kat: " << currentFloor << " | Y�kseklik: " << elevatorY << " | Hareket: " << elevatorMoving << std::endl;
        //std::cout << "yukar� Tu�: " << glfwGetKey(mainwindow, GLFW_KEY_UP) << std::endl;

        // Girdi i�le
        processInput(mainwindow);

        // Ekran� temizle
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        if (elevatorMoving) {
            float targetY = floorHeights[currentFloor];

            if (elevatorY < targetY) {
                elevatorY += elevatorSpeed * deltaTime;
                if (elevatorY >= targetY) {
                    elevatorY = targetY;
                    elevatorMoving = false;
                    doorsOpen = true;
                    doorAnimTime = 0.0f;
                }
            }
            else if (elevatorY > targetY) {
                elevatorY -= elevatorSpeed * deltaTime;
                if (elevatorY <= targetY) {
                    elevatorY = targetY;
                    elevatorMoving = false;
                    doorsOpen = true;
                    doorAnimTime = 0.0f;
                }
            }
        }
        //// ---- BURAYA EKLE ----
        //if (currentCameraMode == THIRD_PERSON) {
        //    glm::vec3 offset(0.0f, 3.0f, 5.0f);
        //    cameraPos = catPosition + offset;
        //    cameraFront = glm::normalize(catPosition - cameraPos);
        //}
        //else if (currentCameraMode == FIRST_PERSON) {
        //    cameraPos = catPosition + glm::vec3(0.0f, 0.7f, 0.0f); // Kedi g�z y�ksekli�i
        //    // cameraFront mouse_callback ile g�ncelleniyor, aynen b�rak.
        //}
        //// ---- EKLEME SONU ----

        //if (currentCameraMode == THIRD_PERSON) {
        //    glm::vec3 offset(0.0f, 3.0f, 5.0f);
        //    cameraPos = catPosition + offset;
        //    cameraFront = glm::normalize(catPosition - cameraPos);
        //}


        //else if (currentCameraMode == FIRST_PERSON) {
        //    // Kedi ba�� hizas�: biraz �stte ve �nde
        //    cameraPos = catPosition + glm::vec3(0.0f, 0.85f, 1.5f);
        //    // cameraFront mouse ile y�netiliyor
        //}

        //if (currentCameraMode == FIRST_PERSON) {
        //    cameraPos = catPosition + glm::vec3(0.0f, 1.0f, 0.0f); // Kedinin ba�� seviyesinde
        //    // cameraFront = kedinin bak�� y�n�, yani catYaw
        //    cameraFront = glm::vec3(sin(catYaw), 0, cos(catYaw));
        //}


        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(VAO);

       //
       if (currentCameraMode == THIRD_PERSON) {
        glm::vec3 offset(0.0f, 3.0f, 5.0f);
        cameraPos = catPosition + offset;
        cameraFront = glm::normalize(catPosition - cameraPos);
    }
        else if (currentCameraMode == FIRST_PERSON) {
           float ileriMesafe = 2.25f; // ne kadar �nde olsun (0.2 - 0.5 aras� dene)
           glm::vec3 ileriVektor = glm::normalize(glm::vec3(sin(glm::radians(yaw)), 0, cos(glm::radians(yaw))));
           cameraPos = catPosition + glm::vec3(0.0f, 0.7f, 0.0f) + ileriVektor * ileriMesafe;
        }
        
        
        
        
        
        
        
        
        
        
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

        //Dokular�n eklenmesi
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);  // Zemin gibi texture'l� objeler
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));

        //Zemin dokusu
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(40.0f, 1.0f, 40.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Tablo
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureAbstract);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: Tabloyu konumland�r (�rnek olarak sa� duvara yak�n ve biraz yukar�da)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-16.5f, 5.0f, -19.9f));
        model = glm::scale(model, glm::vec3(2.5f, 4.0f, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Tablo
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureAtmosfer);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: Tabloyu konumland�r (�rnek olarak sa� duvara yak�n ve biraz yukar�da)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(16.5f, 5.0f, -19.9f));
        model = glm::scale(model, glm::vec3(5.0f, 4.0f, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Ana d�ng�de Tablo1
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureSky);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: Tabloyu konumland�r (�rnek olarak sa� duvara yak�n ve biraz yukar�da)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-19.5f, 5.5f, -12.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(6.0f, 4.0f, 4.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Ana d�ng�de Tablo1
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tablo2);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: Tabloyu konumland�r (�rnek olarak sa� duvara yak�n ve biraz yukar�da)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-19.5f, 15.5f, -12.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(6.0f, 4.0f, 4.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Ana d�ng�de Tablo3
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tablo3);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: Tabloyu konumland�r (�rnek olarak sa� duvara yak�n ve biraz yukar�da)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-16.5f, 15.0f, -19.9f));
        //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(6.0f, 4.0f, 4.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Ana d�ng�de Tablo3
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tablo1);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: 
        model = glm::translate(glm::mat4(1.0f), glm::vec3(16.5f, 15.0f, -18.9f));
        //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(6.0f, 4.0f, 4.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Ana d�ng�de Tablo1
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureSky2);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: Tabloyu konumland�r (�rnek olarak sa� duvara yak�n ve biraz yukar�da)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(19.75f, 15.5f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Ana d�ng�de Tablo1
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tablo4);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: Tabloyu konumland�r (�rnek olarak sa� duvara yak�n ve biraz yukar�da)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(19.5f, 15.5f, -12.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 4.0f, 3.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Ana d�ng�de �nci
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inci);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: Tabloyu konumland�r (�rnek olarak sa� duvara yak�n ve biraz yukar�da)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-19.75f, 15.5f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 4.0f, 3.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Ana d�ng�de �nci
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, leonardo);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: Tabloyu konumland�r (�rnek olarak sa� duvara yak�n ve biraz yukar�da)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 15.5f, 19.5f));
        //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(6.0f, 4.0f, 6.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Kedinin korktu�u tablo
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, korku1);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);

        // Model matrisi: Tabloyu konumland�r (�rnek olarak sa� duvara yak�n ve biraz yukar�da)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(19.5f, 15.5f, 15.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 5.0f, 3.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(artVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


      ///  -----   Doku Olmayan Nesneler

        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), false);  // Zemin gibi texture'l� objeler
        
        glBindVertexArray(VAO);
       
        // Ye�illik rengi
        glm::vec3 yesillik(0.13f, 0.55f, 0.13f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(yesillik));

        // Ye�illik alan 
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.3f, 0.0f));
        model = glm::scale(model, glm::vec3(100.0f, 0.1f, 100.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Duvar rengi
        glm::vec3 duvar_rengi(0.141f, 0.369f, 0.306f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(duvar_rengi));

        // �n duvar (kamera kar��s�)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, -20.0f));
        model = glm::scale(model, glm::vec3(40.0f, 10.0f, 0.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Arka duvar
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 20.0f));
        model = glm::scale(model, glm::vec3(40.0f, 10.0f, 0.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Sol duvar
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 5.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 10.0f, 40.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Sa� duvar
        model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, 5.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 10.0f, 40.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //---------------------------------------------------
        // �at� duvar (kamera kar��s�)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 20.0f, -20.0f));
        model = glm::scale(model, glm::vec3(40.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // �at� Arka duvar
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 20.0f, 20.0f));
        model = glm::scale(model, glm::vec3(40.0f, 10.0f, 0.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // �at� kat� duvar
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 20.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 10.0f, 40.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // �at� Kat� 
        model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 10.0f, 40.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);


        //Sergi Alanlar� Duvarlar
        for (int s = 1; s > -2; s -= 2)
        {
            for (int k = 1; k > -2; k -= 2)
            {
                model = glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f * k, 5.0f, 5.0f * s));
                model = glm::scale(model, glm::vec3(10.0f, 10.0f, 0.1f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);

            }
        }

        ////Ana holden ay�ran duvarlar
        for (int s = 1; s > -2; s -= 2)
        {
            for (int k = 1; k > -2; k -= 2)
            {
                model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f * k, 5.0f, 16.0f * s));
                model = glm::scale(model, glm::vec3(0.1f, 10.0f, 8.0f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);

            }
        }

        // Tavan
        glm::vec3 tavan_rengi(0.75f, 0.75f, 0.75f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(tavan_rengi));

        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));
        model = glm::scale(model, glm::vec3(40.0f, 0.1f, 40.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //Sergi Alanlar� Duvarlar
        for (int s = 1; s > -2; s -= 2)
        {
            for (int k = 1; k > -2; k -= 2)
            {
                model = glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f * k, 5.0f, 5.0f * s));
                model = glm::scale(model, glm::vec3(10.0f, 10.0f, 0.1f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);

            }
        }

        ////Ana holden ay�ran duvarlar
        for (int s = 1; s > -2; s -= 2)
        {
            for (int k = 1; k > -2; k -= 2)
            {
                model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f * k, 5.0f, 16.0f * s));
                model = glm::scale(model, glm::vec3(0.1f, 10.0f, 8.0f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);

            }
        }
        
        
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), false);  // Zemin gibi texture'l� objeler
        // Kutular
        glm::vec3 kutu_rengi(0.65f, 0.50f, 0.32f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(kutu_rengi));

        model = glm::translate(glm::mat4(1.0f), glm::vec3(-17.0f, 11.5f, 12.5f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::translate(glm::mat4(1.0f), glm::vec3(-12.0f, 11.5f, 12.5f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);



        glm::vec3 kutu_rengi1(1.0f,0.0f, 0.0f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(kutu_rengi1));

        model = glm::translate(glm::mat4(1.0f), glm::vec3(-17.0f, 3.5f, 12.5f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //Kedinin Olaca�� kutu
        glm::vec3 kutu_rengi2(0.65f, 0.50f, 0.32f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(kutu_rengi2));
        // Alt taban (Y = 0)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.1f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 0.05f, 2.0f)); // kal�nl��� az
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36); // k�p ile �al���yorsan

        // Arka duvar (Z = -1)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 11.0f, -1.0f));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 0.05f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // �n
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 11.0f, 1.0f));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 0.05f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Sol
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 11.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.05f, 2.0f, 2.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Sa� duvar
        model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 11.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.05f, 2.0f, 2.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);


        //Asans�r 
        // Asans�r platform y�ksekli�i
       // float elevatorHeight = 0.5f + sin(glfwGetTime()) * 2.0f; // Zamanla yukar�-a�a�� hareket eder, �rnek animasyon

        // Kap� animasyonu hesaplama
        float doorMaxOffset = 1.5f;
        if (doorAnimTime < doorAnimDuration) {
            doorAnimTime += deltaTime;
        }
        float progress = glm::clamp(doorAnimTime / doorAnimDuration, 0.0f, 1.0f);
        float currentOffset = doorsOpen ? progress * doorMaxOffset : (1.0f - progress) * doorMaxOffset;

        //// Asans�r hareketi g�ncelle
        //if (elevatorMovingUp) {
        //    elevatorY += elevatorSpeed * deltaTime;
        //    if (elevatorY >= elevatorTargetY) {
        //        elevatorY = elevatorTargetY;
        //        elevatorMovingUp = false;
        //        doorsOpen = true;
        //        doorAnimTime = 0.0f;
        //    }
        //}

        
        //Asans�r Rengi
        glm::vec3 asans�r_rengi(0.2f, 0.2f, 0.2f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(asans�r_rengi));

        // Asans�r kutusu merkez pozisyonu (�r: (10,0,0) veya (0,0,0))
        glm::vec3 shaftCenter = glm::vec3(0.0f, 0.0f, -17.0f);

        // ALT TABAN
        model = glm::translate(glm::mat4(1.0f), shaftCenter + glm::vec3(0.0f, 0.0f, -1.75f));
        model = glm::scale(model, glm::vec3(7.5f, 0.05f, 7.5f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ARKA DUVAR (Z = -1)
        model = glm::translate(glm::mat4(1.0f), shaftCenter + glm::vec3(0.0f, 2.0f, -2.75f));
        model = glm::scale(model, glm::vec3(7.5f, 40.0f, 0.05f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

       
        // Sol Kap�
        glm::mat4 leftDoor = glm::translate(glm::mat4(1.0f), shaftCenter + glm::vec3(-1.9f - currentOffset, 2.0f, 0.75f));
        leftDoor = glm::scale(leftDoor, glm::vec3(3.5f, 40.0f, 0.05f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(leftDoor));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Sa� Kap�
        glm::mat4 rightDoor = glm::translate(glm::mat4(1.0f), shaftCenter + glm::vec3(1.9f + currentOffset, 2.0f, 0.75f));
        rightDoor = glm::scale(rightDoor, glm::vec3(3.5f, 40.0f, 0.05f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(rightDoor));
        glDrawArrays(GL_TRIANGLES, 0, 36);



        // SOL DUVAR (X = -1)
        model = glm::translate(glm::mat4(1.0f), shaftCenter + glm::vec3(-3.5f, 2.0f, -1.0f));
        model = glm::scale(model, glm::vec3(0.05f, 40.0f, 3.5f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // SA� DUVAR (X = +1)
        model = glm::translate(glm::mat4(1.0f), shaftCenter + glm::vec3(3.5f, 2.0f, -1.0f));
        model = glm::scale(model, glm::vec3(0.05f, 40.0f, 3.5f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Platform (Kabinin taban�, Y = hareketli y�kseklik)
        model = glm::translate(glm::mat4(1.0f), shaftCenter + glm::vec3(0.0f, elevatorY, -1.0f));
        model = glm::scale(model, glm::vec3(7.5f, 0.08f, 3.5f)); // kutudan biraz k���k
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);


//------------------------------------------------------Duvarlar---------------------------------
        // ODA KAPISI animasyonunu g�ncelle
        if (roomDoorAnim < roomDoorAnimDuration) {
            roomDoorAnim += deltaTime;
        }
        float roomDoorProgress = glm::clamp(roomDoorAnim / roomDoorAnimDuration, 0.0f, 1.0f);
        float roomDoorOffset = roomDoorOpen ? roomDoorProgress * roomDoorMaxOffset : (1.0f - roomDoorProgress) * roomDoorMaxOffset;
///Kap�----------------------------------------------------------------------------------------
        glm::vec3 doorColor = glm::vec3(0.7f, 0.5f, 0.3f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(doorColor));
     
        glm::vec3 duvarMerkez(10.0f, 15.0f, 8.0f); // 2. kat, sa� b�lme duvar�
        float kap�Yukseklik = 10.0f;
        float kap�Geni�lik = 4.0f;

        // Sol Kap� (Z y�n�nde sola kayar)
        glm::mat4 odaLeftDoor = glm::translate(glm::mat4(1.0f),
            duvarMerkez + glm::vec3(0.0f, 0.0f, -kap�Geni�lik / 2.0f - roomDoorOffset));
        odaLeftDoor = glm::scale(odaLeftDoor, glm::vec3(0.1f, kap�Yukseklik, kap�Geni�lik));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(odaLeftDoor));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Sa� Kap� (Z y�n�nde sa�a kayar)
        glm::mat4 odaRightDoor = glm::translate(glm::mat4(1.0f),
            duvarMerkez + glm::vec3(0.0f, 0.0f, kap�Geni�lik / 2.0f + roomDoorOffset));
        odaRightDoor = glm::scale(odaRightDoor, glm::vec3(0.1f, kap�Yukseklik, kap�Geni�lik));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(odaRightDoor));
        glDrawArrays(GL_TRIANGLES, 0, 36);
      
//--------- KED�---------------------------------------------------------------

        // Kediyi �izdi�in yere (g�vde ba�lang�c�)
// --- KED� G�VDE ---
       // Y�ne d�nen model matris
        glm::vec3 kedi_rengi(1.0f, 1.0f, 1.0f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(kedi_rengi));
       
        glm::mat4 catModel = glm::translate(glm::mat4(1.0f), catPosition);
        catModel = glm::rotate(catModel, catYaw, glm::vec3(0, 1, 0));
        catModel = glm::scale(catModel, glm::vec3(1.0f, 1.0f, 1.5f)); // G�vde
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(catModel));
        glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f); // G�vde beyaz
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // --- Ba� ---
        glm::mat4 head = glm::translate(catModel, glm::vec3(0.0f, 0.8f, 0.6f));
        head = glm::scale(head, glm::vec3(0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(head));
        glUniform3f(objectColorLoc, 1.0f, 0.9f, 0.9f);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // --- Kuyruk ---
        glm::mat4 tail = glm::translate(catModel, glm::vec3(0.0f, 0.1f, -1.0f));
        tail = glm::scale(tail, glm::vec3(0.2f, 0.2f, 0.8f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tail));
        glUniform3f(objectColorLoc, 0.6f, 0.6f, 0.6f);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // --- Kulaklar ---
        glm::mat4 earL = glm::translate(catModel, glm::vec3(-0.2f, 1.1f, 0.7f));
        earL = glm::scale(earL, glm::vec3(0.2f, 0.4f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(earL));
        glUniform3f(objectColorLoc, 0.95f, 0.85f, 0.85f);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glm::mat4 earR = glm::translate(catModel, glm::vec3(0.2f, 1.1f, 0.7f));
        earR = glm::scale(earR, glm::vec3(0.2f, 0.4f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(earR));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // --- Bacaklar (Animasyonlu) ---
        float animationTime = glfwGetTime();
        float swing = sin(animationTime * 5.0f) * 0.2f;
        std::vector<glm::vec3> legOffsets = {
            glm::vec3(-0.3f, -0.75f,  0.4f + swing),
            glm::vec3(0.3f, -0.75f,  0.4f - swing),
            glm::vec3(-0.3f, -0.75f, -0.4f - swing),
            glm::vec3(0.3f, -0.75f, -0.4f + swing)
        };
        for (auto& offset : legOffsets) {
            glm::mat4 leg = glm::translate(catModel, offset);
            leg = glm::scale(leg, glm::vec3(0.2f, 0.5f, 0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(leg));
            glUniform3f(objectColorLoc, 0.9f, 0.9f, 0.95f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


     

        //Kedici�in kontrol�!!!!!
//---------------------------------------------------------------------------------------------------------------------------------
        //�st Kat
        // === 2. Kat Zemini ===

        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);  // Zemin gibi texture'l� objeler

        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

        // Model matrisi (10 birim yukar�)
        glm::mat4 model2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.1f, 0.0f));
        model2 = glm::scale(model2, glm::vec3(40.0f, 1.0f, 40.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model2));

        glBindVertexArray(floorVBO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Deneme texture'�n olmad��� VBO'nun y�klenmesi
        glBindVertexArray(VBO);

        //Nesneler---------------------------------------

        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), false);  // Zemin gibi texture'l� objeler

        // Duvar rengi
        glm::vec3 duvar_rengi2(0.141f, 0.369f, 0.306f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(duvar_rengi2));
        // �n duvar (kamera kar��s�)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 15.0f, -20.0f));
        model = glm::scale(model, glm::vec3(40.0f, 10.0f, 0.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Arka duvar
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 15.0f, 20.0f));
        model = glm::scale(model, glm::vec3(40.0f, 10.0f, 0.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Sol duvar
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 15.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 10.0f, 40.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Sa� duvar
        model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, 15.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 10.0f, 40.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Tavan

        glUniform3fv(objectColorLoc, 1, glm::value_ptr(tavan_rengi));

        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 20.0f, 0.0f));
        model = glm::scale(model, glm::vec3(40.0f, 0.1f, 40.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glUniform3fv(objectColorLoc, 1, glm::value_ptr(duvar_rengi2));
        //Sergi Alanlar� Duvarlar
        for (int s = 1; s > -2; s -= 2)
        {
            for (int k = 1; k > -2; k -= 2)
            {
                model = glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f * k, 15.0f, 5.0f * s));
                model = glm::scale(model, glm::vec3(10.0f, 10.0f, 0.1f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);

            }
        }

        ////Ana holden ay�ran duvarlar
        for (int s = 1; s > -2; s -= 2)
        {
            for (int k = 1; k > -2; k -= 2)
            {
                model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f * k, 15.0f, 16.0f * s));
                model = glm::scale(model, glm::vec3(0.1f, 10.0f, 8.0f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);

            }
        }
    
        glUseProgram(0);
        glDisable(GL_DEPTH_TEST);
        // ---------- UI Yaz� Ba�lang�c� ----------
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, WIDTH, HEIGHT, 0, -1, 1); // 2D ekran

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        //glScalef(3.0f, 3.0f, 1.0f);
        //glColor3f(0.0f, 0.0f, 0.0f); // Beyaz yaz�

        //drawText(30, HEIGHT-30, "YON TUSLARI: Kamera Hareketi | F = FPS | K = TPS | E = Kapi");

        //if (currentFloor == 0)
        //    drawText(30, HEIGHT - 60, "Zemin Kattasiniz | Tablo: Starry Night");
        //else if (currentFloor == 1)
        //    drawText(30, HEIGHT - 60, "1. Katta: Modern Sanat Bolumu");
        //else if (currentFloor == 2)
        //    drawText(30, HEIGHT - 60, "Cati Kati: Temiz Hava Burada!");

        // �l�ek fakt�r�
        float scale = 3.0f;

        glScalef(scale, scale, 1.0f);

        // drawText �a�r�lar�: koordinatlar �l�eklenmeden �nceki de�erler olmal�!
        drawText(30 / scale, (HEIGHT - 30) / scale, "YON TUSLARI: Kamera Hareketi | F = FPS | K = TPS | E = Kapi");

        if (currentFloor == 0)
            drawText(30 / scale, (HEIGHT - 60) / scale, "Zemin Kattasiniz | Tablo: Starry Night");
        else if (currentFloor == 1)
            drawText(30 / scale, (HEIGHT - 60) / scale, "1. Katta: Modern Sanat Bolumu");
        else if (currentFloor == 2)
            drawText(30 / scale, (HEIGHT - 60) / scale, "Cati Kati: Temiz Hava Burada!");
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        // ---------- UI Yaz� Sonu ----------
        glEnable(GL_DEPTH_TEST);

        // Buffers
        glfwSwapBuffers(mainwindow);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// G�r�nt�n�n ekranda nereye �izilece�i ekran�n tamam�na �izdirmi� oluyoruz.
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// Klavyeden Tu�larla Kontrol
void processInput(GLFWwindow* window){
    //const float cameraSpeed = 2.5f * deltaTime;
    //if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)//Ekrandan ��k��
    //    glfwSetWindowShouldClose(window, true);

    //// FPS - TPS kamera ge�i�i
    //if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    //    currentCameraMode = FIRST_PERSON;
    //if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    //    currentCameraMode = THIRD_PERSON;


    //if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)//Ekrandan ��k��
    //    glfwSetWindowShouldClose(window, true);
    //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //    cameraPos += cameraSpeed * cameraFront;
    //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //    cameraPos -= cameraSpeed * cameraFront;
    //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    ///----------------------Yukar�s� ilk �al��an versiyon

    const float cameraSpeed = 2.5f * deltaTime;
    const float catSpeed = 2.0f * deltaTime;

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)//Ekrandan ��k��
       glfwSetWindowShouldClose(window, true);
    // Kamera modlar� aras� ge�i�
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        currentCameraMode = FIRST_PERSON;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        currentCameraMode = THIRD_PERSON;

    // Kamera hareketi (sadece TPS modda aktif olmal�)
    if (currentCameraMode == THIRD_PERSON) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    // FPS modunda: Kedi hareketi (kamera de�il!)
    if (currentCameraMode == FIRST_PERSON) {
        glm::vec3 forward = glm::normalize(glm::vec3(sin(glm::radians(yaw)), 0, cos(glm::radians(yaw))));
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            catPosition += forward * catSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            catPosition -= forward * catSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            catPosition -= right * catSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            catPosition += right * catSpeed;
    
    }

    /// Asans�r kontrol�
    static bool ePressedLastFrame = false;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (!ePressedLastFrame) {
            doorsOpen = !doorsOpen;
            doorAnimTime = 0.0f; // yeniden animasyon ba�las�n
        }
        ePressedLastFrame = true;
    }
    else {
        ePressedLastFrame = false;
    }

    //static bool eKeyPressed = false;
    //if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    //    if (!eKeyPressed && doorsOpen && elevatorY == 0.0f) {
    //        elevatorMovingUp = true;
    //        doorsOpen = false; // Hareketten �nce kap� kapans�n
    //        doorAnimTime = 0.0f;
    //    }
    //    eKeyPressed = true;
    //}
    //else {
    //    eKeyPressed = false;
    //}

    //Asans�r a�a�� yukar� kontrol
    static bool upPressedLastFrame = false;
    static bool downPressedLastFrame = false;

    if (!elevatorMoving && elevatorWaitTime <= 0.0f) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !upPressedLastFrame && currentFloor < maxFloor) {
            currentFloor++;
            elevatorWaitTime = elevatorWaitDuration;
            std::cout << "yukar� Tu�una bas�ld�. Kat: " << currentFloor << std::endl;
        }
        upPressedLastFrame = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !downPressedLastFrame && currentFloor > 0) {
            currentFloor--;
            elevatorWaitTime = elevatorWaitDuration;
            std::cout << "a�a�� Tu�una bas�ld�. Kat: " << currentFloor << std::endl;
        }
        downPressedLastFrame = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    } 

    // ODA KAPISI Space ile a�/kapat
    static bool spacePressedLastFrame = false;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!spacePressedLastFrame) {
            roomDoorOpen = !roomDoorOpen;
            roomDoorAnim = 0.0f; // Animasyon ba�las�n
        }
        spacePressedLastFrame = true;
    }
    else {
        spacePressedLastFrame = false;
    }



    //Kap�n�n A��l�p Kapanmas�
    /*static bool mPressedLastFrame = false;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        if (!mPressedLastFrame && playerIsNearDoor) {
            roomDoorOpen = !roomDoorOpen;
            roomDoorAnim = 0.0f;
        }
        mPressedLastFrame = true;
    }
    else {
        mPressedLastFrame = false;
    }*/

  /*  float catSpeed = 2.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        currentCameraMode = FIRST_PERSON;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        currentCameraMode = THIRD_PERSON;

    if (currentCameraMode == FIRST_PERSON) {
        glm::vec3 forward = glm::vec3(sin(catYaw), 0, cos(catYaw));
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            catPosition += forward * catSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            catPosition -= forward * catSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            catPosition -= right * catSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            catPosition += right * catSpeed;
    }
    else {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            catPosition.z += catSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            catPosition.z -= catSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            catPosition.x -= catSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            catPosition.x += catSpeed;*/
   //}
   // if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    //    glfwSetWindowShouldClose(window, true);
    
    
    
    //const float catSpeed = 2.0f * deltaTime;

    //// Mod ge�i�i en ba�ta olmal�!
    //if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    //    currentCameraMode = FIRST_PERSON;
    //if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    //    currentCameraMode = THIRD_PERSON;

    //if (currentCameraMode == FIRST_PERSON) {
    //    glm::vec3 forward = glm::vec3(sin(catYaw), 0, cos(catYaw));
    //    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

    //    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //        catPosition += forward * catSpeed;
    //    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //        catPosition -= forward * catSpeed;
    //    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //        catPosition -= right * catSpeed;
    //    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //        catPosition += right * catSpeed;
    //}
    //else {
    //    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    //        catPosition.z += catSpeed;
    //    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    //        catPosition.z -= catSpeed;
    //    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    //        catPosition.x -= catSpeed;
    //    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    //        catPosition.x += catSpeed;
    //}

    //if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    //    glfwSetWindowShouldClose(window, true);
}





   // //////if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
   // ////if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
   // ////    catPosition.z -= catSpeed;
   // ////if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
   // ////    catPosition.z += catSpeed;
   // ////if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
   // ////    catPosition.x -= catSpeed;
   // ////if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
   // ////    catPosition.x += catSpeed;


   // //   // FPS MODUNDA: Kediyi kamera y�n�ne g�re hareket ettir
   // //if (currentCameraMode == FIRST_PERSON) {
   // //    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
   // //        catPosition += cameraFront * catSpeed;
   // //    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
   // //        catPosition -= cameraFront * catSpeed;
   // //    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
   // //        catPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * catSpeed;
   // //    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
   // //        catPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * catSpeed;
   // //}
   // //// THIRD_PERSON MODUNDA: Kedi eski x/z eksenlerinde hareket etsin
   // //else {
   // //    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
   // //        catPosition.z -= catSpeed;
   // //    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
   // //        catPosition.z += catSpeed;
   // //    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
   // //        catPosition.x -= catSpeed;
   // //    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
   // //        catPosition.x += catSpeed;
   // //}

   // 
   // if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)//Ekrandan ��k��
   //     glfwSetWindowShouldClose(window, true);
   // /*
   // if () {
   //     if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
   //         cameraPos += cameraSpeed * cameraFront;
   //     if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
   //         cameraPos -= cameraSpeed * cameraFront;
   //     if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
   //         cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
   //     if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
   //         cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;*/

   // }




//Fare hareketlerinin kontrol�
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
 
    //
    //if (currentCameraMode != FIRST_PERSON)
    //    return;


    //if (firstMouse) //�lk fare hareketi
    //{
    //    lastX = xpos;
    //    lastY = ypos;
    //    firstMouse = false;
    //    return;
    //}
    ////Poziyon farklar�
    //float xoffset = xpos - lastX;
    //float yoffset = lastY - ypos;
    //lastX = xpos; lastY = ypos;
    ////Hassasiyet
    //float sensitivity = 0.1f;
    //xoffset *= sensitivity;
    //yoffset *= sensitivity;
    ////Y�n a��lar� 
    //yaw += xoffset;//sa�a sola d�n�� 
    //pitch += yoffset;//Yukar� a�a�� d�n��
    ////S�n�rland�rma ba� a�a�� olamams� i�in 90 derecede
    //if (pitch > 89.0f)
    //{
    //    pitch = 89.0f;
    //}
    //if (pitch < -89.0f) {
    //    pitch = -89.0f;
    //}
    //
    ////Yeni y�n vekt�r�n�n hesab�
    //glm::vec3 front;
    //front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    //front.y = sin(glm::radians(pitch));
    //front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    //cameraFront = glm::normalize(front);
    //
    //Yukar�s� ilk �al��an versiyon
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    if (currentCameraMode == FIRST_PERSON) {
        yaw += xoffset;       // FPS modunda YAW hem kamera hem kedi
        pitch += yoffset;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        catYaw = glm::radians(yaw); // <<< Buras� �ok �nemli!
    }
    else {
        yaw += xoffset;
        pitch += yoffset;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    /*  static bool firstMouse = true;
    static float lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f;*/

    //if (firstMouse) {
    //    lastX = xpos;
    //    lastY = ypos;
    //    firstMouse = false;
    //}
    //float xoffset = xpos - lastX;
    //float yoffset = lastY - ypos;
    //lastX = xpos;
    //lastY = ypos;

    //float sensitivity = 0.1f;
    //xoffset *= sensitivity;
    //yoffset *= sensitivity;

    //if (currentCameraMode == FIRST_PERSON) {
    //    catYaw += glm::radians(xoffset); // Radyan olarak biriktir!
    //    pitch += yoffset;
    //    if (pitch > 89.0f) pitch = 89.0f;
    //    if (pitch < -89.0f) pitch = -89.0f;
    //}
    //else {
    //    yaw += xoffset;
    //    pitch += yoffset;
    //    if (pitch > 89.0f) pitch = 89.0f;
    //    if (pitch < -89.0f) pitch = -89.0f;
    //}
    
    //static bool firstMouse = true;
    //static float lastX = 0.0f, lastY = 0.0f;
    //extern CameraMode currentCameraMode;
    //extern float yaw, pitch;
    //extern float catYaw;

    //// �lk mouse hareketinde ilk de�erleri ata
    //if (firstMouse) {
    //    lastX = xpos;
    //    lastY = ypos;
    //    firstMouse = false;
    //    return;
    //}

    //// Mouse hareketi fark�n� hesapla
    //float xoffset = xpos - lastX;
    //float yoffset = lastY - ypos;
    //lastX = xpos;
    //lastY = ypos;

    //// Hassasiyet uygula
    //float sensitivity = 0.1f;
    //xoffset *= sensitivity;
    //yoffset *= sensitivity;

    //if (currentCameraMode == FIRST_PERSON) {
    //    catYaw += xoffset * 0.01f; // Kedinin y�n�n� de�i�tir (FPS bak�� y�n�)
    //    pitch += yoffset * 0.01f;  // Yukar�/a�a�� bak��
    //    if (pitch > 89.0f) pitch = 89.0f;
    //    if (pitch < -89.0f) pitch = -89.0f;
    //}
    //else {
    //    yaw += xoffset;
    //    pitch += yoffset;
    //    if (pitch > 89.0f) pitch = 89.0f;
    //    if (pitch < -89.0f) pitch = -89.0f;
    //}

    //// FPS modunda kamera, kedinin y�n�n� takip edecek
    //glm::vec3 front;
    //if (currentCameraMode == FIRST_PERSON) {
    //    front.x = sin(catYaw) * cos(glm::radians(pitch));
    //    front.y = sin(glm::radians(pitch));
    //    front.z = cos(catYaw) * cos(glm::radians(pitch));
    //}
    //else {
    //    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    //    front.y = sin(glm::radians(pitch));
    //    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    //}
    //
    //cameraFront = glm::normalize(front);
    
    
     
    
    ////if (currentCameraMode == FIRST_PERSON) {
    ////    catYaw += xoffset * 0.01f; // Hassasiyeti ayarla
    ////    pitch += yoffset * 0.01f;  // istersen yukar�/a�a�� bak�� i�in
    ////}


    //if (firstMouse) //�lk fare hareketi
    //{
    //    lastX = xpos;
    //    lastY = ypos;
    //    firstMouse = false;
    //    return;
    //}

    ////Poziyon farklar�
    //float xoffset = xpos - lastX;
    //float yoffset = lastY - ypos;
    //lastX = xpos; lastY = ypos;
    ////Hassasiyet
    //float sensitivity = 0.1f;
    //xoffset *= sensitivity;
    //yoffset *= sensitivity;
    ////Y�n a��lar� 
    //yaw += xoffset;//sa�a sola d�n�� 
    //pitch += yoffset;//Yukar� a�a�� d�n��
    ////S�n�rland�rma ba� a�a�� olamams� i�in 90 derecede
    //if (pitch > 89.0f)
    //{
    //    pitch = 89.0f;
    //}
    //if (pitch < -89.0f) {
    //    pitch = -89.0f;
    //}
    ////Yeni y�n vekt�r�n�n hesab�
    //glm::vec3 front;
    //front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    //front.y = sin(glm::radians(pitch));
    //front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    //cameraFront = glm::normalize(front);
}




//Shader kodlar�n�n derlenmesi
unsigned int createShaderProgram() {
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);//Vertex shader
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);//Fragment Shader
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);
    //Shader programm� olu�turulur
    unsigned int shaderProgram = glCreateProgram();
    //Vertex shader la fragment shader programa ba�lan�r
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    //Programa linklenir
    glLinkProgram(shaderProgram);
    //L�nklendi�i i�in silinebirlirler.
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return shaderProgram;
}