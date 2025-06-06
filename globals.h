#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED
/** \brief Tutaj mozna trzymac pseudo-globalne zmienne dla organizacji
 */
namespace Globals
{
static int64_t numberOfEntities = 0;
static int64_t currentlyHoveredID = -1;
static int64_t windowH;
static int64_t windowW;
static float cameraX;
static float cameraY;
static float cameraZ;
static float closestUnitToCam = -1.f;
};





#endif // GLOBALS_H_INCLUDED
