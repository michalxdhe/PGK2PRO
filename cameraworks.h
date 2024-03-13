#ifndef CAMERAWORKS_H_INCLUDED
#define CAMERAWORKS_H_INCLUDED

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;

/** \brief Klasa definiujaca kamere he
 */
class PlayerCamera
{

public:
    bool w,a,s,d,m;
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

    PlayerCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
    {
        w=a=s=d=m=0;
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
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
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
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
        Pitch -= yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }


private:

    /** \brief Nie kaz mi tego komentowac, self explanatory funkcja
     *
     * \return void
     *
     */
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

#endif // CAMERAWORKS_H_INCLUDED
