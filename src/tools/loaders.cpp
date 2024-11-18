#include <engine/tools/loaders.h>

void VKFW::Tools::Loaders::load_OBJ(Core::Mesh* const mesh,
                                    const std::string fileName,
                                    bool              importMaterials,
                                    bool              calculateTangents,
                                    bool              overrideGeometry) {
    // std::this_thread::sleep_for(std::chrono::seconds(4)); //Debuging

    // Preparing output
    tinyobj::attrib_t                attrib;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;
    std::string                      warn;
    std::string                      err;

    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileName.c_str(), importMaterials ? nullptr : nullptr);

    // Check for errors
    if (!warn.empty())
    {
        DEBUG_LOG("WARN: " + warn);
    }
    if (!err.empty())
    {
        ERR_LOG(err);
        DEBUG_LOG("ERROR: Couldn't load mesh");
        return;
    }

    std::vector<Graphics::Vertex>                  vertices;
    std::vector<uint32_t>                          indices;
    std::unordered_map<Graphics::Vertex, uint32_t> uniqueVertices;

    size_t shape_id = 0;
    for (const tinyobj::shape_t& shape : shapes)
    {
        if (!shape.mesh.indices.empty())
        {
            // IS INDEXED
            for (const tinyobj::index_t& index : shape.mesh.indices)
            {
                Graphics::Vertex vertex = {};

                // Position and color
                if (index.vertex_index >= 0)
                {
                    vertex.pos.x = attrib.vertices[3 * index.vertex_index + 0];
                    vertex.pos.y = attrib.vertices[3 * index.vertex_index + 1];
                    vertex.pos.z = attrib.vertices[3 * index.vertex_index + 2];

                    vertex.color.r = attrib.colors[3 + index.vertex_index + 0];
                    vertex.color.g = attrib.colors[3 + index.vertex_index + 1];
                    vertex.color.b = attrib.colors[3 + index.vertex_index + 2];
                }
                // Normal
                if (index.normal_index >= 0)
                {
                    vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
                    vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
                    vertex.normal.z = attrib.normals[3 * index.normal_index + 2];
                }

                // Tangent
                vertex.tangent = {0.0, 0.0, 0.0};

                // UV
                if (index.texcoord_index >= 0)
                {
                    vertex.texCoord.x = attrib.texcoords[2 * index.texcoord_index + 0];
                    vertex.texCoord.y = attrib.texcoords[2 * index.texcoord_index + 1];
                }

                // Check if the vertex is already in the map
                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        } else
            // NOT INDEXED
            for (size_t i = 0; i < shape.mesh.num_face_vertices.size(); i++)
            {
                for (size_t j = 0; j < shape.mesh.num_face_vertices[i]; j++)
                {
                    Graphics::Vertex vertex{};
                    size_t vertex_index = shape.mesh.indices[i * shape.mesh.num_face_vertices[i] + j].vertex_index;
                    // Pos
                    if (!attrib.vertices.empty())
                    {
                        vertex.pos.x = attrib.vertices[3 * vertex_index + 0];
                        vertex.pos.y = attrib.vertices[3 * vertex_index + 1];
                        vertex.pos.z = attrib.vertices[3 * vertex_index + 2];
                    }
                    // Normals
                    if (!attrib.normals.empty())
                    {
                        vertex.normal.x = attrib.normals[3 * vertex_index + 0];
                        vertex.normal.y = attrib.normals[3 * vertex_index + 1];
                        vertex.normal.z = attrib.normals[3 * vertex_index + 2];
                    }
                    // Tangents

                    vertex.tangent = {0.0, 0.0, 0.0};

                    // UV
                    if (!attrib.texcoords.empty())
                    {
                        vertex.texCoord.x = attrib.texcoords[2 * vertex_index + 0];
                        vertex.texCoord.y = attrib.texcoords[2 * vertex_index + 1];
                    }
                    // COLORS
                    if (!attrib.colors.empty())
                    {
                        vertex.color.r = attrib.colors[3 * vertex_index + 0];
                        vertex.color.g = attrib.colors[3 * vertex_index + 1];
                        vertex.color.b = attrib.colors[3 * vertex_index + 2];
                    }

                    vertices.push_back(vertex);
                }
            }

        if (calculateTangents)
        {
            compute_tangents_gram_smidt(vertices, indices);
        }

        if (overrideGeometry)
        {
            Core::Geometry* oldGeom = mesh->get_geometry(shape_id);
            if (oldGeom)
            {
                oldGeom->fill(vertices, indices);
                continue;
            }
        }

        Core::Geometry* g = new Core::Geometry();
        g->fill(vertices, indices);
        mesh->push_geometry(g);

        shape_id++;
    }
    mesh->set_file_route(fileName);
    return;
}

void VKFW::Tools::Loaders::load_PLY(Core::Mesh* const mesh,
                                    const std::string fileName,
                                    bool              preload,
                                    bool              verbose,
                                    bool              calculateTangents,
                                    bool              overrideGeometry) {

    std::unique_ptr<std::istream> file_stream;
    std::vector<uint8_t>          byte_buffer;
    try
    {
        // For most files < 1gb, pre-loading the entire file upfront and wrapping it into a
        // stream is a net win for parsing speed, about 40% faster.
        if (preload)
        {
            byte_buffer = Graphics::Utils::read_file_binary(fileName);
            file_stream.reset(new Graphics::Utils::memory_stream((char*)byte_buffer.data(), byte_buffer.size()));
        } else
        {
            file_stream.reset(new std::ifstream(fileName, std::ios::binary));
        }

        if (!file_stream || file_stream->fail())
            throw std::runtime_error("file_stream failed to open " + fileName);

        file_stream->seekg(0, std::ios::end);
        const float size_mb = file_stream->tellg() * float(1e-6);
        file_stream->seekg(0, std::ios::beg);

        tinyply::PlyFile file;
        file.parse_header(*file_stream);

        if (verbose)
        {
            std::cout << "\t[ply_header] Type: " << (file.is_binary_file() ? "binary" : "ascii") << std::endl;
            for (const auto& c : file.get_comments())
                std::cout << "\t[ply_header] Comment: " << c << std::endl;
            for (const auto& c : file.get_info())
                std::cout << "\t[ply_header] Info: " << c << std::endl;

            for (const auto& e : file.get_elements())
            {
                std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
                for (const auto& p : e.properties)
                {
                    std::cout << "\t[ply_header] \tproperty: " << p.name
                              << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
                    if (p.isList)
                        std::cout << " (list_type=" << tinyply::PropertyTable[p.listType].str << ")";
                    std::cout << std::endl;
                }
            }
        }
        // Because most people have their own mesh types, tinyply treats parsed data as structured/typed byte buffers.
        std::shared_ptr<tinyply::PlyData> positions, normals, colors, texcoords, faces, tripstrip;

        // // The header information can be used to programmatically extract properties on elements
        // // known to exist in the header prior to reading the data. For brevity of this sample, properties
        // // like vertex position are hard-coded:
        try
        { positions = file.request_properties_from_element("vertex", {"x", "y", "z"}); } catch (const std::exception& e)
        { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try
        {
            normals = file.request_properties_from_element("vertex", {"nx", "ny", "nz"});
        } catch (const std::exception& e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            colors = file.request_properties_from_element("vertex", {"red", "green", "blue", "alpha"});
        } catch (const std::exception& e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            colors = file.request_properties_from_element("vertex", {"r", "g", "b", "a"});
        } catch (const std::exception& e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        { texcoords = file.request_properties_from_element("vertex", {"u", "v"}); } catch (const std::exception& e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        { texcoords = file.request_properties_from_element("vertex", {"s", "t"}); } catch (const std::exception& e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        // Providing a list size hint (the last argument) is a 2x performance improvement. If you have
        // arbitrary ply files, it is best to leave this 0.
        try
        { faces = file.request_properties_from_element("face", {"vertex_indices"}, 3); } catch (const std::exception& e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        // Tristrips must always be read with a 0 list size hint (unless you know exactly how many elements
        // are specifically in the file, which is unlikely);
        try
        {
            tripstrip = file.request_properties_from_element("tristrips", {"vertex_indices"}, 0);
        } catch (const std::exception& e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }
        Graphics::Utils::ManualTimer readTimer;

        readTimer.start();
        file.read(*file_stream);
        readTimer.stop();

        if (verbose)
        {

            const float parsingTime = static_cast<float>(readTimer.get()) / 1000.f;
            std::cout << "\tparsing " << size_mb << "mb in " << parsingTime << " seconds [" << (size_mb / parsingTime)
                      << " MBps]" << std::endl;

            if (positions)
                std::cout << "\tRead " << positions->count << " total vertices " << std::endl;
            if (normals)
                std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
            if (colors)
                std::cout << "\tRead " << colors->count << " total vertex colors " << std::endl;
            if (texcoords)
                std::cout << "\tRead " << texcoords->count << " total vertex texcoords " << std::endl;
            if (faces)
                std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;
            if (tripstrip)
                std::cout << "\tRead " << (tripstrip->buffer.size_bytes() / tinyply::PropertyTable[tripstrip->t].stride)
                          << " total indices (tristrip) " << std::endl;
        }

        std::vector<Graphics::Vertex> vertices;
        std::vector<uint32_t>         indices;

        if (positions)
        {
            const float* posData = reinterpret_cast<const float*>(positions->buffer.get());
            const float* normalData;
            const float* colorData;
            const float* uvData;
            if (normals)
                normalData = reinterpret_cast<const float*>(normals->buffer.get());
            if (colors)
                colorData = reinterpret_cast<const float*>(colors->buffer.get());
            if (texcoords)
                uvData = reinterpret_cast<const float*>(texcoords->buffer.get());

            for (size_t i = 0; i < positions->count; i++)
            {

                Vec3 position = Vec3(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]);
                Vec3 normal =
                    normals ? Vec3(normalData[i * 3], normalData[i * 3 + 1], normalData[i * 3 + 2]) : Vec3(0.0f);
                Vec3 color = colors ? Vec3(colorData[i * 3], colorData[i * 3 + 1], colorData[i * 3 + 2]) : Vec3(1.0f);
                Vec2 uv    = texcoords ? Vec2(uvData[i * 2], uvData[i * 2 + 1]) : Vec2(0.0f);

                vertices.push_back({position, normal, {0.0f, 0.0f, 0.0f}, uv, color});
            }
        }
        unsigned* facesData = reinterpret_cast<unsigned*>(faces->buffer.get());
        for (size_t i = 0; i < faces->count; ++i)
        {
            unsigned int vertexIndex1 = static_cast<unsigned int>(facesData[i]);
            unsigned int vertexIndex2 = static_cast<unsigned int>(facesData[i + 1]);
            unsigned int vertexIndex3 = static_cast<unsigned int>(facesData[i + 2]);

            indices.push_back(facesData[3 * i]);
            indices.push_back(facesData[3 * i + 1]);
            indices.push_back(facesData[3 * i + 2]);
        }

        if (calculateTangents && normals)
        {
            compute_tangents_gram_smidt(vertices, indices);
        }

        if (overrideGeometry)
        {
            Core::Geometry* oldGeom = mesh->get_geometry();
            if (oldGeom)
            {
                oldGeom->fill(vertices, indices);
            }
            return;
        }

        Core::Geometry* g = new Core::Geometry();
        g->fill(vertices, indices);
        mesh->push_geometry(g);
        mesh->set_file_route(fileName);
    } catch (const std::exception& e)
    { std::cerr << "Caught tinyply exception: " << e.what() << std::endl; }
}
void VKFW::Tools::Loaders::load_3D_file(Core::Mesh* const mesh,
                                        const std::string fileName,
                                        bool              asynCall,
                                        bool              overrideGeometry) {
    size_t dotPosition = fileName.find_last_of(".");

    if (dotPosition != std::string::npos)
    {

        std::string fileExtension = fileName.substr(dotPosition + 1);

        if (fileExtension == OBJ)
        {
            if (asynCall)
            {
                std::thread loadThread(Loaders::load_OBJ, mesh, fileName, false, true, overrideGeometry);
                loadThread.detach();
            } else
                Loaders::load_OBJ(mesh, fileName, false, true, overrideGeometry);

            return;
        }
        if (fileExtension == PLY)
        {
            if (asynCall)
            {
                std::thread loadThread(Loaders::load_PLY, mesh, fileName, true, false, true, overrideGeometry);
                loadThread.detach();
            } else
                Loaders::load_PLY(mesh, fileName, true, false, true, overrideGeometry);

            return;
        }
        if (fileExtension == HAIR)
        {
            if (asynCall)
            {
                std::thread loadThread(Loaders::load_hair, mesh, fileName.c_str());
                loadThread.detach();
            } else
                Loaders::load_hair(mesh, fileName.c_str());

            return;
        }

        std::cerr << "Unsupported file format: " << fileExtension << std::endl;
    } else
    {
        std::cerr << "Invalid file name: " << fileName << std::endl;
    }
}
void VKFW::Tools::Loaders::load_hair(Core::Mesh* const mesh, const char* fileName) {

#define HAIR_FILE_SEGMENTS_BIT 1
#define HAIR_FILE_POINTS_BIT 2
#define HAIR_FILE_THICKNESS_BIT 4
#define HAIR_FILE_TRANSPARENCY_BIT 8
#define HAIR_FILE_COLORS_BIT 16
#define HAIR_FILE_INFO_SIZE 88

    unsigned short* segments;
    float*          points;
    float*          dirs;
    float*          thickness;
    float*          transparency;
    float*          colors;

    struct Header {
        char         signature[4]; //!< This should be "HAIR"
        unsigned int hair_count;   //!< number of hair strands
        unsigned int point_count;  //!< total number of points of all strands
        unsigned int arrays;       //!< bit array of data in the file

        unsigned int d_segments;     //!< default number of segments of each strand
        float        d_thickness;    //!< default thickness of hair strands
        float        d_transparency; //!< default transparency of hair strands
        float        d_color[3];     //!< default color of hair strands

        char info[HAIR_FILE_INFO_SIZE]; //!< information about the file
    };

    Header header;

    header.signature[0]   = 'H';
    header.signature[1]   = 'A';
    header.signature[2]   = 'I';
    header.signature[3]   = 'R';
    header.hair_count     = 0;
    header.point_count    = 0;
    header.arrays         = 0; // no arrays
    header.d_segments     = 0;
    header.d_thickness    = 1.0f;
    header.d_transparency = 0.0f;
    header.d_color[0]     = 1.0f;
    header.d_color[1]     = 1.0f;
    header.d_color[2]     = 1.0f;
    memset(header.info, '\0', HAIR_FILE_INFO_SIZE);

    FILE* fp;
    fp = fopen(fileName, "rb");
    if (fp == nullptr)
        return;

    // read the header
    size_t headread = fread(&header, sizeof(Header), 1, fp);

    // Check if header is correctly read
    if (headread < 1)
        return;

    // Check if this is a hair file
    if (strncmp(header.signature, "HAIR", 4) != 0)
        return;

    // Read segments array
    if (header.arrays & HAIR_FILE_SEGMENTS_BIT)
    {
        segments         = new unsigned short[header.hair_count];
        size_t readcount = fread(segments, sizeof(unsigned short), header.hair_count, fp);
        if (readcount < header.hair_count)
        {
            std::cerr << "Error reading segments" << std::endl;
            return;
        }
    }

    // Read points array
    if (header.arrays & HAIR_FILE_POINTS_BIT)
    {
        points           = new float[header.point_count * 3];
        size_t readcount = fread(points, sizeof(float), header.point_count * 3, fp);
        if (readcount < header.point_count * 3)
        {
            std::cerr << "Error reading points" << std::endl;
            return;
        }
    }

    // Read thickness array
    if (header.arrays & HAIR_FILE_THICKNESS_BIT)
    {
        thickness        = new float[header.point_count];
        size_t readcount = fread(thickness, sizeof(float), header.point_count, fp);
        if (readcount < header.point_count)
        {
            std::cerr << "Error reading thickness" << std::endl;
            return;
        }
    }

    // Read thickness array
    if (header.arrays & HAIR_FILE_TRANSPARENCY_BIT)
    {
        transparency     = new float[header.point_count];
        size_t readcount = fread(transparency, sizeof(float), header.point_count, fp);
        if (readcount < header.point_count)
        {
            std::cerr << "Error reading alpha" << std::endl;
            return;
        }
    }

    // Read colors array
    if (header.arrays & HAIR_FILE_COLORS_BIT)
    {
        colors           = new float[header.point_count * 3];
        size_t readcount = fread(colors, sizeof(float), header.point_count * 3, fp);
        if (readcount < header.point_count * 3)
        {
            std::cerr << "Error reading colors" << std::endl;
            return;
        }
    }

    fclose(fp);

    auto computeDirection =
        [](float* d, float& d0len, float& d1len, float const* p0, float const* p1, float const* p2) {
            // line from p0 to p1
            float d0[3];
            d0[0]         = p1[0] - p0[0];
            d0[1]         = p1[1] - p0[1];
            d0[2]         = p1[2] - p0[2];
            float d0lensq = d0[0] * d0[0] + d0[1] * d0[1] + d0[2] * d0[2];
            d0len         = (d0lensq > 0) ? (float)sqrt(d0lensq) : 1.0f;

            // line from p1 to p2
            float d1[3];
            d1[0]         = p2[0] - p1[0];
            d1[1]         = p2[1] - p1[1];
            d1[2]         = p2[2] - p1[2];
            float d1lensq = d1[0] * d1[0] + d1[1] * d1[1] + d1[2] * d1[2];
            d1len         = (d1lensq > 0) ? (float)sqrt(d1lensq) : 1.0f;

            // make sure that d0 and d1 has the same length
            d0[0] *= d1len / d0len;
            d0[1] *= d1len / d0len;
            d0[2] *= d1len / d0len;

            // direction at p1
            d[0]         = d0[0] + d1[0];
            d[1]         = d0[1] + d1[1];
            d[2]         = d0[2] + d1[2];
            float dlensq = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
            float dlen   = (dlensq > 0) ? (float)sqrt(dlensq) : 1.0f;
            d[0] /= dlen;
            d[1] /= dlen;
            d[2] /= dlen;

            // return d0len;
        };

    auto fillDirectionArray = [=](float* dir) {
        if (dir == nullptr || header.point_count <= 0 || points == nullptr)
            return;

        int p = 0; // point index
        for (unsigned int i = 0; i < header.hair_count; i++)
        {
            int s = (segments) ? segments[i] : header.d_segments;
            if (s > 1)
            {
                // direction at point1
                float len0, len1;
                computeDirection(
                    &dir[(p + 1) * 3], len0, len1, &points[p * 3], &points[(p + 1) * 3], &points[(p + 2) * 3]);

                // direction at point0
                float d0[3];
                d0[0]          = points[(p + 1) * 3] - dir[(p + 1) * 3] * len0 * 0.3333f - points[p * 3];
                d0[1]          = points[(p + 1) * 3 + 1] - dir[(p + 1) * 3 + 1] * len0 * 0.3333f - points[p * 3 + 1];
                d0[2]          = points[(p + 1) * 3 + 2] - dir[(p + 1) * 3 + 2] * len0 * 0.3333f - points[p * 3 + 2];
                float d0lensq  = d0[0] * d0[0] + d0[1] * d0[1] + d0[2] * d0[2];
                float d0len    = (d0lensq > 0) ? (float)sqrt(d0lensq) : 1.0f;
                dir[p * 3]     = d0[0] / d0len;
                dir[p * 3 + 1] = d0[1] / d0len;
                dir[p * 3 + 2] = d0[2] / d0len;

                // We computed the first 2 points
                p += 2;

                // Compute the direction for the rest
                for (int t = 2; t < s; t++, p++)
                {
                    computeDirection(
                        &dir[p * 3], len0, len1, &points[(p - 1) * 3], &points[p * 3], &points[(p + 1) * 3]);
                }

                // direction at the last point
                d0[0]          = -points[(p - 1) * 3] + dir[(p - 1) * 3] * len1 * 0.3333f + points[p * 3];
                d0[1]          = -points[(p - 1) * 3 + 1] + dir[(p - 1) * 3 + 1] * len1 * 0.3333f + points[p * 3 + 1];
                d0[2]          = -points[(p - 1) * 3 + 2] + dir[(p - 1) * 3 + 2] * len1 * 0.3333f + points[p * 3 + 2];
                d0lensq        = d0[0] * d0[0] + d0[1] * d0[1] + d0[2] * d0[2];
                d0len          = (d0lensq > 0) ? (float)sqrt(d0lensq) : 1.0f;
                dir[p * 3]     = d0[0] / d0len;
                dir[p * 3 + 1] = d0[1] / d0len;
                dir[p * 3 + 2] = d0[2] / d0len;
                p++;
            } else if (s > 0)
            {
                // if it has a single segment
                float d0[3];
                d0[0]                = points[(p + 1) * 3] - points[p * 3];
                d0[1]                = points[(p + 1) * 3 + 1] - points[p * 3 + 1];
                d0[2]                = points[(p + 1) * 3 + 2] - points[p * 3 + 2];
                float d0lensq        = d0[0] * d0[0] + d0[1] * d0[1] + d0[2] * d0[2];
                float d0len          = (d0lensq > 0) ? (float)sqrt(d0lensq) : 1.0f;
                dir[p * 3]           = d0[0] / d0len;
                dir[p * 3 + 1]       = d0[1] / d0len;
                dir[p * 3 + 2]       = d0[2] / d0len;
                dir[(p + 1) * 3]     = dir[p * 3];
                dir[(p + 1) * 3 + 1] = dir[p * 3 + 1];
                dir[(p + 1) * 3 + 2] = dir[p * 3 + 2];
                p += 2;
            }
            //*/
        }
    };

    dirs = new float[header.point_count * 3];
    fillDirectionArray(dirs);

    std::vector<Graphics::Vertex> vertices;
    vertices.reserve(header.point_count * 3);
    std::vector<uint32_t> indices;

    size_t index   = 0;
    size_t pointId = 0;
    for (size_t hair = 0; hair < header.hair_count; hair++)
    {
        glm::vec3 color        = {((float)rand()) / RAND_MAX, ((float)rand()) / RAND_MAX, ((float)rand()) / RAND_MAX};
        size_t    max_segments = segments ? segments[hair] : header.d_segments;
        for (size_t i = 0; i < max_segments; i++)
        {
            vertices.push_back({{points[pointId], points[pointId + 1], points[pointId + 2]},
                                {0.0f, 0.0f, 0.0f},
                                {dirs[pointId], dirs[pointId + 1], dirs[pointId + 2]},
                                {0.0f, 0.0f},
                                color});
            indices.push_back(index);
            indices.push_back(index + 1);
            index++;
            pointId += 3;
        }
        vertices.push_back({{points[pointId], points[pointId + 1], points[pointId + 2]},
                            {0.0f, 0.0f, 0.0f},
                            {dirs[pointId], dirs[pointId + 1], dirs[pointId + 2]},
                            {0.0f, 0.0f},
                            color});
        pointId += 3;
        index++;
    }

    Core::Geometry* g = new Core::Geometry();
    g->fill(vertices, indices);
    mesh->push_geometry(g);
    mesh->set_file_route(std::string(fileName));
}

void VKFW::Tools::Loaders::load_texture(Core::ITexture* const texture,
                                        const std::string     fileName,
                                        TextureFormatType     textureFormat,
                                        bool                  asyncCall) {
    size_t dotPosition = fileName.find_last_of(".");

    if (dotPosition != std::string::npos)
    {

        std::string fileExtension = fileName.substr(dotPosition + 1);

        if (fileExtension == PNG || fileExtension == JPG)
        {
            if (asyncCall)
            {
                std::thread loadThread(
                    Loaders::load_PNG, static_cast<Core::Texture*>(texture), fileName, textureFormat);
                loadThread.detach();
            } else
                Loaders::load_PNG(static_cast<Core::Texture*>(texture), fileName, textureFormat);

            return;
        }
        if (fileExtension == HDR)
        {
            if (asyncCall)
            {
                std::thread loadThread(Loaders::load_HDRi, static_cast<Core::TextureHDR*>(texture), fileName);
                loadThread.detach();
            } else
                Loaders::load_HDRi(static_cast<Core::TextureHDR*>(texture), fileName);

            return;
        }

        std::cerr << "Unsupported file format: " << fileExtension << std::endl;
    } else
    {
        std::cerr << "Invalid file name: " << fileName << std::endl;
    }
}

void VKFW::Tools::Loaders::load_PNG(Core::Texture* const texture,
                                    const std::string    fileName,
                                    TextureFormatType    textureFormat) {
    int            w, h, ch;
    unsigned char* imgCache = nullptr;
    imgCache                = stbi_load(fileName.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    if (imgCache)
    {
        texture->set_image_cache(imgCache, {static_cast<unsigned int>(w), static_cast<unsigned int>(h), 1}, 4);
        // Set automatically teh optimal format for each type.
        // User can override it after, I he need some other more specific format ...
        switch (textureFormat)
        {
        case TextureFormatType::COLOR_FORMAT:
            texture->set_format(SRGBA_8);
            break;
        case TextureFormatType::NORMAL_FORMAT:
            texture->set_format(RGBA_8U);
            break;
        case TextureFormatType::HDR_FORMAT:
            texture->set_format(SRGBA_16F);
            break;
        }
    } else
    {
#ifndef NDEBUG
        DEBUG_LOG("Failed to load texture PNG file" + fileName);
#endif
        return;
    };
#ifndef NDEBUG
    DEBUG_LOG("PNG Texture loaded successfully");
#endif // DEBUG
}

void VKFW::Tools::Loaders::load_HDRi(Core::TextureHDR* const texture, const std::string fileName) {
    int    w, h, ch;
    float* HDRcache = nullptr;
    HDRcache        = stbi_loadf(fileName.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    if (HDRcache)
    {
        texture->set_image_cache(HDRcache, {static_cast<unsigned int>(w), static_cast<unsigned int>(h), 1}, 4);
        texture->set_format(SRGBA_32F);
    } else
    {
#ifndef NDEBUG
        DEBUG_LOG("Failed to load texture HDRi file" + fileName);
#endif
        return;
    };
#ifndef NDEBUG
    DEBUG_LOG("HDRi Texture loaded successfully");
#endif // DEBUG
}
void VKFW::Tools::Loaders::load_3D_texture(Core::ITexture* const texture,
                                           const std::string    fileName,
                                           uint16_t             depth,
                                           TextureFormatType    textureFormat) {
    int            w, h, ch;
    unsigned char* imgCache = nullptr;
    imgCache                = stbi_load(fileName.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    if (imgCache)
    {
        texture->set_type(TextureType::TEXTURE_3D);
        int      largerSide  = w > h ? w : h;
        int      shorterSide = w > h ? h : w;
        uint16_t finalDepth  = depth == 0 ? largerSide / shorterSide : depth;
        texture->set_image_cache(
            imgCache,
            {static_cast<unsigned int>(shorterSide), static_cast<unsigned int>(largerSide / finalDepth), finalDepth},
            4);
        // Set automatically the optimal format for each type.
        // User can override it after, I he need some other more specific format ...
        switch (textureFormat)
        {
        case TextureFormatType::COLOR_FORMAT:
            texture->set_format(SRGBA_8);
            break;
        case TextureFormatType::NORMAL_FORMAT:
            texture->set_format(RGBA_8U);
            break;
        case TextureFormatType::HDR_FORMAT:
            texture->set_format(SRGBA_16F);
            break;
        }
    } else
    {
#ifndef NDEBUG
        DEBUG_LOG("Failed to load texture PNG file" + fileName);
#endif
        return;
    };
#ifndef NDEBUG
    DEBUG_LOG("PNG Texture loaded successfully");
#endif // DEBUG
}
void VKFW::Tools::Loaders::compute_tangents_gram_smidt(std::vector<Graphics::Vertex>& vertices,
                                                       const std::vector<uint32_t>&   indices) {
    if (!indices.empty())
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            size_t i0 = indices[i];
            size_t i1 = indices[i + 1];
            size_t i2 = indices[i + 2];

            Vec3 tangent = Graphics::Utils::get_tangent_gram_smidt(vertices[i0].pos,
                                                                   vertices[i1].pos,
                                                                   vertices[i2].pos,
                                                                   vertices[i0].texCoord,
                                                                   vertices[i1].texCoord,
                                                                   vertices[i2].texCoord,
                                                                   vertices[i0].normal);

            vertices[i0].tangent += tangent;
            vertices[i1].tangent += tangent;
            vertices[i2].tangent += tangent;
        }
    else
        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            Vec3 tangent = Graphics::Utils::get_tangent_gram_smidt(vertices[i].pos,
                                                                   vertices[i + 1].pos,
                                                                   vertices[i + 2].pos,
                                                                   vertices[i].texCoord,
                                                                   vertices[i + 1].texCoord,
                                                                   vertices[i + 2].texCoord,
                                                                   vertices[i].normal);

            vertices[i].tangent += tangent;
            vertices[i + 1].tangent += tangent;
            vertices[i + 2].tangent += tangent;
        }
}
