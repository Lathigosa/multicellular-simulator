#pragma once
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

using Vertex  = glm::vec3;
using Edge    = glm::uvec2;
using Face    = glm::uvec3;

class Icosphere {
public:
    Icosphere(unsigned int subdivisions = 0) : m_subdivisions(subdivisions)
    {
        generateBaseIcosahedron();
        for(unsigned int i = 0; i < subdivisions; ++i)
            subdivide();
    }

    const std::vector<Vertex>& vertices() const { return m_vertices; }
    const std::vector<Edge>& edges() const { return m_edges; }
    const std::vector<Face>& faces() const { return m_faces; }

private:
    unsigned int m_subdivisions;

    std::vector<Vertex> m_vertices;
    std::vector<Edge> m_edges;
    std::vector<Face> m_faces;

    void generateBaseIcosahedron() {
        const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

        m_vertices = {
            {-1,  t, 0}, { 1,  t, 0}, {-1, -t, 0}, { 1, -t, 0},
            {0, -1,  t}, {0,  1,  t}, {0, -1, -t}, {0,  1, -t},
            { t, 0, -1}, { t, 0,  1}, {-t, 0, -1}, {-t, 0,  1}
        };
        for(auto& v : m_vertices) v = glm::normalize(v);

        m_faces = {
            {0,11,5},{0,5,1},{0,1,7},{0,7,10},{0,10,11},
            {1,5,9},{5,11,4},{11,10,2},{10,7,6},{7,1,8},
            {3,9,4},{3,4,2},{3,2,6},{3,6,8},{3,8,9},
            {4,9,5},{2,4,11},{6,2,10},{8,6,7},{9,8,1}
        };

        buildEdges();
    }

    void subdivide() {
        std::vector<Face> new_faces;
        std::map<std::pair<unsigned int,unsigned int>, unsigned int> midpoint_cache;

        auto getMidpoint = [&](unsigned int a, unsigned int b) -> unsigned int {
            auto key = std::minmax(a,b);
            if(auto it = midpoint_cache.find(key); it != midpoint_cache.end())
                return it->second;

            Vertex mid = glm::normalize((m_vertices[a] + m_vertices[b]) * 0.5f);
            m_vertices.push_back(mid);
            size_t idx = m_vertices.size() - 1;
            midpoint_cache[key] = idx;
            return idx;
        };

        for(auto& f : m_faces) {
            unsigned int a = getMidpoint(f.x, f.y);
            unsigned int b = getMidpoint(f.y, f.z);
            unsigned int c = getMidpoint(f.z, f.x);

            new_faces.push_back({f.x, a, c});
            new_faces.push_back({f.y, b, a});
            new_faces.push_back({f.z, c, b});
            new_faces.push_back({a, b, c});
        }

        m_faces = std::move(new_faces);
        buildEdges();
    }

    void buildEdges() {
        m_edges.clear();
        std::set<std::pair<unsigned int,unsigned int>> seen;
        for(auto& f : m_faces) {
            auto addEdge = [&](unsigned int u, unsigned int v) {
                auto key = std::minmax(u,v);
                if(seen.insert(key).second) m_edges.push_back({key.first,key.second});
            };
            addEdge(f.x, f.y);
            addEdge(f.y, f.z);
            addEdge(f.z, f.x);
        }
    }
};