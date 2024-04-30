#ifndef MOUSEPICK_H_INCLUDED
#define MOUSEPICK_H_INCLUDED

class MousePicker {
public:
    static constexpr float RAY_RANGE = 600.0f;

    glm::vec3 currentRay = glm::vec3(0.0f);

    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    SDL_Window* window;

    MousePicker() = default;

    MousePicker(glm::mat4 projection, glm::mat4 view, SDL_Window* windowref) {
        projectionMatrix = projection;
        viewMatrix = view;
        window = windowref;
    }

    glm::vec3 getCurrentRay() {
        return currentRay;
    }

    void update(glm::mat4 view) {
        viewMatrix = view;
        currentRay = calculateMouseRay();
    }

private:
    glm::vec3 calculateMouseRay() {
        int imouseX, imouseY;

        SDL_GetMouseState(&imouseX, &imouseY);

        double mouseX = imouseX;
        double mouseY = imouseY;

        glm::vec2 normalizedCoords = getNormalisedDeviceCoordinates(mouseX, mouseY);
        glm::vec4 clipCoords(normalizedCoords.x, normalizedCoords.y, -1.0f, 1.0f);
        glm::vec4 eyeCoords = toEyeCoords(clipCoords);
        glm::vec3 worldRay = toWorldCoords(eyeCoords);
        return worldRay;
    }

    glm::vec3 toWorldCoords(glm::vec4 eyeCoords) {
        glm::mat4 invertedView = glm::inverse(viewMatrix);
        glm::vec4 rayWorld = invertedView * eyeCoords;
        glm::vec3 mouseRay(rayWorld.x, rayWorld.y, rayWorld.z);
        mouseRay = glm::normalize(mouseRay);
        return mouseRay;
    }

    glm::vec4 toEyeCoords(glm::vec4 clipCoords) {
        glm::mat4 invertedProjection = glm::inverse(projectionMatrix);
        glm::vec4 eyeCoords = invertedProjection * clipCoords;
        return glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
    }

    glm::vec2 getNormalisedDeviceCoordinates(float mouseX, float mouseY) {
        int w,h;
        SDL_GetWindowSize(window,&w,&h);
		float x = (2.0f * mouseX) / (float)w - 1.f;
		float y = (2.0f * mouseY) / (float)h - 1.f;
		return glm::vec2(x, y);
	}
};

#endif // MOUSEPICK_H_INCLUDED
