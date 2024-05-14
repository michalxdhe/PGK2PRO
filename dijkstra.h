#ifndef DIJKSTRA_H_INCLUDED
#define DIJKSTRA_H_INCLUDED

#include "boardlogic.h"

using namespace std;

unordered_map<glm::vec3, int> getPathableCells(unordered_map<glm::vec3, HexCell> HexGrid, glm::vec3 source)
{
    unordered_map<glm::vec3, int> distances;
    for (auto& pair : HexGrid) {
        distances[pair.first] = numeric_limits<int>::max();
    }

    distances[source] = 0;

    queue<glm::vec3> q;
    q.push(source);

    while (!q.empty()) {
        glm::vec3 current = q.front();
        q.pop();

        for (auto& neighbor : getNeighbors(current)) {
            if (HexGrid.find(neighbor) != HexGrid.end() && HexGrid[neighbor].passable) {
                if (distances[current] + 1 < distances[neighbor]) {
                    distances[neighbor] = distances[current] + 1;
                    q.push(neighbor);
                }
            }
        }
    }

    return distances;
}

#endif // DIJKSTRA_H_INCLUDED
