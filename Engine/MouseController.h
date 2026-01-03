class MouseController{
public:
    float lastX = 0, lastY = 0;
    float sensitivity = 0.1;
    float pitch = 0;
    float yaw = -90;

    bool first = false;

    MouseController(float s) : sensitivity(s) {}
    MouseController() = default;

    ~MouseController(){

    }

    void CalculateMouseOffset(float xpos,  float ypos) {

        if(first){
            lastX = xpos;
            lastY = ypos;
            first = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if(pitch > 89.0f)
            pitch =  89.0f;

        if(pitch < -89.0f)
            pitch = -89.0f;   
    }


};