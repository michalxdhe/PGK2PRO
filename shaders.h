#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED

/** \brief Tworzenie i Generowanie openGl'em tekstury z pliku
 *
 * \param destination unsigned int& docelowy int gdzie zapisze ID
 * \param path const char* Path skad pobiera obrazek
 * \return bool czy sie udalo
 *
 */
bool createAndLoadTexture(unsigned int &destination, const char* path)
{
    glGenTextures(1, &destination);
    glBindTexture(GL_TEXTURE_2D, destination);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
        stbi_image_free(data);
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

#endif // SHADERS_H_INCLUDED
