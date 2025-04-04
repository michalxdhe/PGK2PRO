#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED

#include "common.h"
#include "stb_image.h"

/** \brief Tworzenie i Generowanie openGl'em tekstury z pliku
 *
 * \param destination unsigned int& docelowy int gdzie zapisze ID
 * \param path const char* Path skad pobiera obrazek
 * \return bool czy sie udalo
 *
 */
bool createAndLoadTexture(unsigned int &destination, const char* path, bool flipped = true)
{
    glGenTextures(1, &destination);
    glBindTexture(GL_TEXTURE_2D, destination);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(flipped);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else
        {
            std::cerr << "Unsupported number of channels: " << nrChannels << std::endl;
            stbi_image_free(data);
            return false;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return false;
    }
    stbi_image_free(data);
    return true;
}

bool readShaderFile(const char* filePath, std::string& shaderCode)
{
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        shaderFile.open(filePath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();
        return true;
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        return false;
    }
}
/** \brief Tworzy i kompiluje shader zwracajac jego id
 *
 * \param source const char* kod zrodlowy glsl'a
 * \param type int GL_VERTEX_SHADER lub GL_FRAGMENT_SHADER
 * \param destination GLuint& zmienna do ktorej zapisane jest ID
 * \return bool czy sie udalo
 *
 */
bool createAndCompileShader(const char* filePath, int type, GLuint &destination)
{
    std::string shaderCode;
    if (!readShaderFile(filePath, shaderCode))
    {
        return false;
    }

    const char* source = shaderCode.c_str();

    int  success;
    char infoLog[512];

    if(type == GL_VERTEX_SHADER)
    {
        destination = glCreateShader(GL_VERTEX_SHADER);

        glShaderSource(destination, 1, &source, NULL);
        glCompileShader(destination);


        glGetShaderiv(destination, GL_COMPILE_STATUS, &success);

        if(!success)
        {
            glGetShaderInfoLog(destination, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            return 0;
        }
    }

    if(type == GL_FRAGMENT_SHADER)
    {
        destination = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(destination, 1, &source, NULL);
        glCompileShader(destination);


        glGetShaderiv(destination, GL_COMPILE_STATUS, &success);

        if(!success)
        {
            glGetShaderInfoLog(destination, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
            return 0;
        }
    }

    if(type == GL_COMPUTE_SHADER)
    {
        destination = glCreateShader(GL_COMPUTE_SHADER);

        glShaderSource(destination, 1, &source, NULL);
        glCompileShader(destination);


        glGetShaderiv(destination, GL_COMPILE_STATUS, &success);

        if(!success)
        {
            glGetShaderInfoLog(destination, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
            return 0;
        }
    }

    return 1;
}

GLuint createProgram(unsigned int vertexShader, unsigned int fragmentShader)
{
    unsigned int shaderProgram;

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    return shaderProgram;
}

GLuint createProgram(unsigned int computeShader)
{
    unsigned int shaderProgram;

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, computeShader);
    glLinkProgram(shaderProgram);

    return shaderProgram;
}

#endif // SHADERS_H_INCLUDED
