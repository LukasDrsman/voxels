#pragma once

#include <functional>
#include <fstream>
#include <array>
#include "voxel.hpp"
#include <cstdlib>
#include <ctime>

namespace vox
{
    typedef std::vector<std::pair<glm::vec3, glm::vec3>> PointCloud;

    inline Octree *genericVolume(std::function<bool(glm::vec3)> enclosed, glm::vec3 center, float norm, int max_depth, glm::vec3 color)
    {
        if (enclosed(center) && max_depth == 0) return new Octree(color);
        if (max_depth == 0) return new Octree();

        std::array<Octree *, 8> children{};

        for (int i = 0; i < 8; i++)
        {
            children[i] = genericVolume(enclosed,
                DCENTERS[i] * norm * 0.5f + center,
                norm * 0.5, max_depth - 1, color
            );
        }

        return new Octree(children);
    }

    inline Octree *genericPointCloud(PointCloud point_cloud, glm::vec3 center, float norm, int max_depth)
    {
        if (max_depth == 0)
        {
            int enclosed_count = 0;
            glm::vec3 color{0, 0, 0};

            for (const auto [coord, col] : point_cloud)
                if (lp::lInfNorm(coord - center) < norm)
                {
                    enclosed_count++;
                    color += col;
                }

            if (enclosed_count == 0) return new Octree();
            return new Octree(1.0f / static_cast<float>(enclosed_count) * color);
        }

        std::array<Octree *, 8> children{};

        for (int i = 0; i < 8; i++)
        {
            children[i] = genericPointCloud(
                point_cloud, DCENTERS[i] * norm * 0.5f + center,
                norm * 0.5f, max_depth - 1
            );
        }

        return new Octree(children);
    }

    PointCloud randomPointCloud(int count)
    {
        std::srand(std::time(nullptr));

        PointCloud ret{{}};
        for (int i = 0; i < count; i++)
        {
            ret.emplace_back(
                glm::vec3{(std::rand() % 1000 - 500) / 1000.f,
                            (std::rand() % 1000 - 500) / 1000.f,
                            (std::rand() % 1000 - 500) / 1000.f},
                glm::vec3{(std::rand() % 220 + 35) / 255.f,
                            (std::rand() % 220 + 35) / 255.f,
                            (std::rand() % 220 + 35) / 255.f}
            );
        }

        return ret;
    }

    PointCloud pcdToPointCloud(const std::string& pcd_path, glm::vec3 model_color, float scale = 1.0f)
    {
        std::ifstream pcd_file;
        pcd_file.open(pcd_path);

        std::string line;
        while (std::getline(pcd_file, line))
        {
            std::istringstream iss(line);
            std::string key, value;
            if (!(iss >> key >> value)) throw std::runtime_error("Fuck");
            std::cout << key << value << std::endl;
            if (key == "DATA")
            {
                if (value == "ascii") break;
                throw std::runtime_error("Unsupported pcd format error.");
            }
        }

        PointCloud ret{{}};
        float x, y, z;
        while (pcd_file >> x >> y >> z)
        {
            std::cout << x << y << z << std::endl;
            ret.emplace_back(
                glm::vec3{x, y, z} * scale,
                model_color
            );
        }

        pcd_file.close();

        return ret;
    }

}