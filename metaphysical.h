#ifndef METAPHYSICAL_H_INCLUDED
#define METAPHYSICAL_H_INCLUDED

glm::mat4 calculateLightSpaceMatrix(glm::vec3 lightPos, glm::vec3 up)
{
    glm::mat4 viewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), up);

    float near_plane = 0.5f;
    float far_plane = 5.0f;

    glm::mat4 projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

    glm::mat4 lightSpaceMatrix = projectionMatrix * viewMatrix;

    return lightSpaceMatrix;
}

class Object
{

public:

    virtual ~Object() = default;

    virtual void update(double deltaTime) = 0;
    virtual void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms) = 0;
};

class Hexagon : public Object{

public:
    Model model;
    glm::mat4 view;
    HexCell cell;
    glm::vec3 scale;

    Hexagon() = default;

    Hexagon(Model model, HexCell komorka){
    this->model = model;
    cell = komorka;
    scale = glm::vec3(1.f);
    //if(cell.LogicPos.x == 1 && cell.LogicPos.z == -1)
   //     view = glm::translate(glm::mat4(1.f),glm::vec3(cell.LogicPos.x*0.5f,1.f,cell.LogicPos.z*0.5f));
    //else
        view = glm::translate(glm::mat4(1.f),glm::vec3(cell.LogicPos.x*0.45f * scale.x ,0.f,cell.LogicPos.z * 0.52f * scale.z + (-cell.LogicPos.x * -0.26f * scale.z)));
        view = glm::scale(view,scale);
    }

    void update(double deltaTime){

    }

    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms){
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(view));
    this->model.Draw(shaderProgram);
    }
};


class Cube : public Object{

public:
    glm::mat4 model;
    GLuint VBO, VAO;
    std::vector<float> vertices;

    Cube() = default;

    Cube(float w, float h, float l, glm::vec3 pos = glm::vec3(0.f,0.f,0.f))
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model,pos);
        remodel(w,h,l);
    }

    void remodel(float w, float h, float l)
    {
        vertices =
    {
        -w, -h, -l,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        w, -h, -l,   0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
        w,  h, -l,   0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        w,  h, -l,   0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -w,  h, -l,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -w, -h, -l,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -w, -h,  l,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        w, -h,  l,   0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
        w,  h,  l,   0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        w,  h,  l,   0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -w,  h,  l,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -w, -h,  l,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -w,  h,  l, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -w,  h, -l, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -w, -h, -l, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -w, -h, -l, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -w, -h,  l, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -w,  h,  l, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        w,  h,  l,   1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        w,  h, -l,   1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        w, -h, -l,   1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        w, -h, -l,   1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        w, -h,  l,   1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        w,  h,  l,   1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -w, -h, -l,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
        w, -h, -l,   0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
        w, -h,  l,   0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        w, -h,  l,   0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -w, -h,  l,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -w, -h, -l,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -w,  h, -l,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        w,  h, -l,   0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
        w,  h,  l,   0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        w,  h,  l,   0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -w,  h,  l,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -w,  h, -l,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));


        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void update(double deltaTime){
            //model = glm::rotate(model, 0.001f, glm::vec3(0.0f, 1.0f, 1.0f));
    }

    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms){
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

};

class LightSource : public Object{

public:
    glm::mat4 lightSpaceMatrix;
    glm::vec3 lightTarget;
    glm::vec3 lightPos = glm::vec3(0.f,3.f,0.00000001f);

    void bindDepthShader(std::vector<unsigned int>shaderPrograms)
    {
        glUseProgram(shaderPrograms[2]);
        glUniformMatrix4fv(glGetUniformLocation(shaderPrograms[2],"lightSpaceMatrix"),1,GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    }
};

class LightCube : public LightSource{

public:
    glm::mat4 model = glm::mat4(1.0f);

    GLuint VBO, VAO;
    std::vector<float> vertices;

    LightCube() = default;

    LightCube(float w, float h, float l)
    {
      remodel(w,h,l);
      model = glm::translate(model,lightPos);
    }

    void remodel(float w, float h, float l)
    {
        vertices =
    {
        -w, -h, -l,  0.0f,  0.0f, -1.0f,
        w, -h, -l,   0.0f,  0.0f, -1.0f,
        w,  h, -l,   0.0f,  0.0f, -1.0f,
        w,  h, -l,   0.0f,  0.0f, -1.0f,
        -w,  h, -l,  0.0f,  0.0f, -1.0f,
        -w, -h, -l,  0.0f,  0.0f, -1.0f,

        -w, -h,  l,  0.0f,  0.0f,  1.0f,
        w, -h,  l,   0.0f,  0.0f,  1.0f,
        w,  h,  l,   0.0f,  0.0f,  1.0f,
        w,  h,  l,   0.0f,  0.0f,  1.0f,
        -w,  h,  l,  0.0f,  0.0f,  1.0f,
        -w, -h,  l,  0.0f,  0.0f,  1.0f,

        -w,  h,  l, -1.0f,  0.0f,  0.0f,
        -w,  h, -l, -1.0f,  0.0f,  0.0f,
        -w, -h, -l, -1.0f,  0.0f,  0.0f,
        -w, -h, -l, -1.0f,  0.0f,  0.0f,
        -w, -h,  l, -1.0f,  0.0f,  0.0f,
        -w,  h,  l, -1.0f,  0.0f,  0.0f,

        w,  h,  l,   1.0f,  0.0f,  0.0f,
        w,  h, -l,   1.0f,  0.0f,  0.0f,
        w, -h, -l,   1.0f,  0.0f,  0.0f,
        w, -h, -l,   1.0f,  0.0f,  0.0f,
        w, -h,  l,   1.0f,  0.0f,  0.0f,
        w,  h,  l,   1.0f,  0.0f,  0.0f,

        -w, -h, -l,  0.0f, -1.0f,  0.0f,
        w, -h, -l,   0.0f, -1.0f,  0.0f,
        w, -h,  l,   0.0f, -1.0f,  0.0f,
        w, -h,  l,   0.0f, -1.0f,  0.0f,
        -w, -h,  l,  0.0f, -1.0f,  0.0f,
        -w, -h, -l,  0.0f, -1.0f,  0.0f,

        -w,  h, -l,  0.0f,  1.0f,  0.0f,
        w,  h, -l,   0.0f,  1.0f,  0.0f,
        w,  h,  l,   0.0f,  1.0f,  0.0f,
        w,  h,  l,   0.0f,  1.0f,  0.0f,
        -w,  h,  l,  0.0f,  1.0f,  0.0f,
        -w,  h, -l,  0.0f,  1.0f,  0.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void update(double deltaTime){
        glm::mat3 rotationMatrix = glm::mat3(model);
        lightTarget = rotationMatrix * glm::vec3(0.f,-1.f,0.f); //ten drugi vector w mnozeniu to tam gdzie celuje swiatlo
        lightPos = glm::vec3(model * glm::vec4(glm::vec3(0.f,0.f,0.f), 1.0f));
        lightSpaceMatrix = calculateLightSpaceMatrix(lightPos, glm::vec3(0.f,1.f,0.f));
        //model = glm::translate(model,glm::vec3(0.f, 0.f, 0.1f));
        //model = glm::rotate(model, 0.01f, glm::vec3(1.0f, 0.f, 0.0f));
    }

    void render(unsigned int shaderProgram, std::vector<unsigned int> shaderPrograms){
        glUseProgram(shaderProgram);

        glUniform3f(glGetUniformLocation(shaderProgram,"lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3fv(glGetUniformLocation(shaderProgram,"lightPos"),1, glm::value_ptr(lightPos));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"lightSpaceMatrix"),1,GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        glUseProgram(shaderPrograms[1]);

        unsigned int modelLoc = glGetUniformLocation(shaderPrograms[1], "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
};

#endif // METAPHYSICAL_H_INCLUDED
