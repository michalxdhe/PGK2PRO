#ifndef CAMERAWORKS_H_INCLUDED
#define CAMERAWORKS_H_INCLUDED

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const float YAW         = 0.0f;
const float PITCH       =  60.0f;
const float SPEED       =  4.5f;
const float SENSITIVITY =  0.1f;

/** \brief Klasa definiujaca kamere he
 */
class RevoltingCamera
{

public:
    bool w,a,s,d,m;
    glm::vec3 Target;
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // katy eulera
    float Yaw;
    float Pitch;
    // opcje kamery
    float MovementSpeed;
    float MouseSensitivity;
    float ZoomSensitivity;

    float zoom = 0;

    RevoltingCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
    {
        w=a=s=d=m=0;
        Target = target;
        Position = position;
        WorldUp = up;
        Up = glm::vec3(0.0f, 1.0f, 0.0f);
        Yaw = yaw;
        Pitch = pitch;
        ZoomSensitivity = 0.6f;
        zoom = glm::distance(Position,Target);
        updateCameraPos();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Target, Up);
    }

    /** \brief Przetwarza input kamery
     *
     * \param direction Camera_Movement kierunek z enum'a
     * \param deltaTime float
     * \return void
     *
     */
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
        {
            Target += Front * velocity;
            Position += Front * velocity;
        }
        if (direction == BACKWARD)
        {
            Target -= Front * velocity;
            Position -= Front * velocity;
        }
        if (direction == LEFT)
        {
            Target -= Right * velocity;
            Position -= Right * velocity;
        }
        if (direction == RIGHT)
        {
            Target += Right * velocity;
            Position += Right * velocity;
        }
    }

    void doZoom(float val)
    {
        zoom -= val * ZoomSensitivity;
        if(zoom < 1.f)
            zoom = 1.f;
        if(zoom > 25.f)
            zoom = 25.f;
        updateCameraPos();
    }


    /** \brief Logika ruchu kamery
     * przetwarza input na podstawie offsetu x i y od srodka, albo dowolnego innego punktu
     * \param xoffset float
     * \param yoffset float
     * \param true GLboolean constrainPitch= kregoslup lub bez kregoslupa
     * \return void
     *
     */
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraPos();
    }


private:

    /** \brief Siema, co tam
     *
     * \return void
     *
     */
    void updateCameraPos()
    {
        float x = cos(glm::radians(Yaw));
        float y = sin(glm::radians(Pitch));
        float z = sin(glm::radians(Yaw));

        //float m = sqrt((1 - (y*y))/((x*x)+(z*z)));
        float m = abs(cos(glm::radians(Pitch)));
        Position = zoom * glm::vec3(m*x,y,m*z) + Target;

        glm::vec3 front = glm::vec3(Target.x, Position.y, Target.z) - Position;
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
    }

};

#endif // CAMERAWORKS_H_INCLUDED
