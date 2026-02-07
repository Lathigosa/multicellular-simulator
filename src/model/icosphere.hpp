#pragma once
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <array>

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
    const std::vector<glm::uvec2> vertex_neighbors_of_edge() const { return m_edges_vertex_neighbors; }
    const std::vector<std::array<uint, 8>>& vertex_edges() const { return m_vertex_edges; }
    const std::vector<std::array<uint, 8>>& vertex_opposite_edges() const { return m_vertex_opposite_edges; }

private:
    unsigned int m_subdivisions;

    std::vector<Vertex> m_vertices;
    std::vector<Edge> m_edges;
    std::vector<Face> m_faces;
    std::vector<glm::uvec2> m_edges_vertex_neighbors;
    std::vector<std::array<uint, 8>> m_vertex_edges;
    std::vector<std::array<uint, 8>> m_vertex_opposite_edges;

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
        m_edges_vertex_neighbors.clear();

        // For each edge, store the opposite vertices from adjacent faces
        std::map<std::pair<unsigned int, unsigned int>, std::vector<unsigned int>> edgeOpposites;

        for (const auto& f : m_faces) {
            auto addEdge = [&](unsigned int u, unsigned int v, unsigned int opp) {
                auto key = std::minmax(u, v);
                edgeOpposites[key].push_back(opp);
            };

            addEdge(f.x, f.y, f.z);
            addEdge(f.y, f.z, f.x);
            addEdge(f.z, f.x, f.y);
        }

        // Prepare vertex -> edge adjacency
        m_vertex_edges.clear();
        m_vertex_edges.resize(m_vertices.size());
        for (auto& arr : m_vertex_edges)
            arr.fill(std::numeric_limits<unsigned int>::max());

        // Build final edge list and neighbor list
        unsigned int edgeIndex = 0;
        for (const auto& [key, opps] : edgeOpposites) {
            unsigned int u = key.first;
            unsigned int v = key.second;

            m_edges.push_back({ u, v });

            if (opps.size() == 2) {
                m_edges_vertex_neighbors.push_back({ opps[0], opps[1] });
            } else if (opps.size() == 1) {
                m_edges_vertex_neighbors.push_back({ opps[0], opps[0] });
            } else {
                m_edges_vertex_neighbors.push_back({ 0u, 0u });
            }

            // Register this edge with both endpoint vertices
            auto register_edge = [&](unsigned int vertex) {
                auto& list = m_vertex_edges[vertex];
                for (size_t i = 0; i < 8; ++i) {
                    if (list[i] == std::numeric_limits<unsigned int>::max()) {
                        list[i] = edgeIndex;
                        return;
                    }
                }
                // If this ever triggers, your valence assumption is violated
                // You may want to assert or handle dynamically
            };

            register_edge(u);
            register_edge(v);

            ++edgeIndex;
        }

        // Create vertex_opposite_edges:
        std::map<std::pair<uint, uint>, uint> edgeIndexOf;
        for (uint e = 0; e < m_edges.size(); ++e) {
            uint u = m_edges[e].x;
            uint v = m_edges[e].y;
            edgeIndexOf[std::minmax(u, v)] = e;
        }

        m_vertex_opposite_edges.clear();
        m_vertex_opposite_edges.resize(m_vertices.size());
        for (auto& arr : m_vertex_opposite_edges)
            arr.fill(std::numeric_limits<uint>::max());

        for (const auto& f : m_faces) {
            uint i = f.x;
            uint j = f.y;
            uint k = f.z;

            uint e_jk = edgeIndexOf[std::minmax(j, k)];
            uint e_ki = edgeIndexOf[std::minmax(k, i)];
            uint e_ij = edgeIndexOf[std::minmax(i, j)];

            auto register_opposite = [&](uint vertex, uint edge) {
                auto& list = m_vertex_opposite_edges[vertex];
                for (uint& slot : list) {
                    if (slot == std::numeric_limits<uint>::max()) {
                        slot = edge;
                        return;
                    }
                }
                // valence overflow -> should not happen on an icosphere
            };

            register_opposite(i, e_jk);
            register_opposite(j, e_ki);
            register_opposite(k, e_ij);
        }
    }
};