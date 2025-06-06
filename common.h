#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <queue>
#include <vector>
#include <list>
#include <limits>
#include <map>
#include <chrono>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#endif // COMMON_H_INCLUDED
