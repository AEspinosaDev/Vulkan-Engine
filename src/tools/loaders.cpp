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

    size_t shape_id = 0;
    for (const tinyobj::shape_t& shape : shapes)
    {
        std::vector<Graphics::Vertex>                  vertices;
        std::vector<uint32_t>                          indices;
        std::unordered_map<Graphics::Vertex, uint32_t> uniqueVertices;
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
            Core::Geometry::compute_tangents_gram_smidt(vertices, indices);
        }

        // if (overrideGeometry)
        // {
        //     Core::Geometry* oldGeom = mesh->get_geometry(shape_id);
        //     if (oldGeom)
        //     {
        //         oldGeom->fill(vertices, indices);
        //         continue;
        //     }
        // }

        Core::Geometry* g = new Core::Geometry();
        g->fill(vertices, indices);
        mesh->push_geometry(g);

        shape_id++;
    }
    mesh->set_file_route(fileName);
    return;
}

void VKFW::Tools::Loaders::load_OBJ2(Core::Mesh* const mesh,
                                     const std::string fileName,
                                     bool              importMaterials,
                                     bool              calculateTangents,
                                     bool              overrideGeometry) {

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

    std::vector<Graphics::Vertex>           vertices;
    std::vector<uint32_t>                   indices;
    std::unordered_map<glm::vec3, uint32_t> unique_vertices;

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Graphics::Vertex vertex;
            if (index.vertex_index >= 0)
            {

                vertex.pos   = {attrib.vertices[3 * index.vertex_index + 0],
                                attrib.vertices[3 * index.vertex_index + 1],
                                attrib.vertices[3 * index.vertex_index + 2]};
                vertex.color = {attrib.colors[3 * index.vertex_index + 0],
                                attrib.colors[3 * index.vertex_index + 1],
                                attrib.colors[3 * index.vertex_index + 2]};
            }
            if (index.normal_index >= 0)
            {
                vertex.normal = {attrib.normals[3 * index.normal_index + 0],
                                 attrib.normals[3 * index.normal_index + 1],
                                 attrib.normals[3 * index.normal_index + 2]};
            }

            vertex.tangent = {0.0, 0.0, 0.0};

            if (index.texcoord_index >= 0)
            {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0], attrib.texcoords[2 * index.texcoord_index + 1]};
            }
            

            if (unique_vertices.count(vertex.pos) == 0)
            {
                unique_vertices[vertex.pos] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(unique_vertices[vertex.pos]);
        }
    }
    if (calculateTangents)
    {
        Core::Geometry::compute_tangents_gram_smidt(vertices, indices);
    }

    Core::Geometry* g = new Core::Geometry();
    g->fill(vertices, indices);
    mesh->push_geometry(g);
    mesh->set_file_route(fileName);
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
            byte_buffer = Utils::read_file_binary(fileName);
            file_stream.reset(new Utils::MemoryStream((char*)byte_buffer.data(), byte_buffer.size()));
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
        Utils::ManualTimer readTimer;

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
            Core::Geometry::compute_tangents_gram_smidt(vertices, indices);
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

    unsigned short* segments = nullptr;
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

        if (fileExtension == PNG || fileExtension == JPG || fileExtension == "jpeg")
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
        texture->set_file_route(fileName);
        switch (textureFormat)
        {
        case TEXTURE_FORMAT_SRGB:
            texture->set_format(SRGBA_8);
            break;
        case TEXTURE_FORMAT_UNORM:
            texture->set_format(RGBA_8U);
            break;
        case TEXTURE_FORMAT_HDR:
            texture->set_format(SRGBA_16F);
            break;
        }
    } else
    {
#ifndef NDEBUG
        ERR_LOG("Failed to load texture PNG file" + fileName);
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
        texture->set_file_route(fileName);

    } else
    {
#ifndef NDEBUG
        ERR_LOG("Failed to load texture HDRi file" + fileName);
#endif
        return;
    };
#ifndef NDEBUG
    DEBUG_LOG("HDRi Texture loaded successfully");
#endif // DEBUG
}
void VKFW::Tools::Loaders::load_3D_texture(Core::ITexture* const texture,
                                           const std::string     fileName,
                                           uint16_t              depth,
                                           TextureFormatType     textureFormat) {
    int            w, h, ch;
    unsigned char* imgCache = nullptr;
    imgCache                = stbi_load(fileName.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    if (imgCache)
    {
        texture->set_type(TextureTypeFlagBits::TEXTURE_3D);
        int      largerSide  = w > h ? w : h;
        int      shorterSide = w > h ? h : w;
        uint16_t finalDepth  = depth == 0 ? largerSide / shorterSide : depth;
        texture->set_image_cache(
            imgCache,
            {static_cast<unsigned int>(shorterSide), static_cast<unsigned int>(largerSide / finalDepth), finalDepth},
            4);
        // Set automatically the optimal format for each type.
        // User can override it after, I he need some other more specific format ...
        texture->set_file_route(fileName);

        switch (textureFormat)
        {
        case TEXTURE_FORMAT_SRGB:
            texture->set_format(SRGBA_8);
            break;
        case TEXTURE_FORMAT_UNORM:
            texture->set_format(RGBA_8U);
            break;
        case TEXTURE_FORMAT_HDR:
            texture->set_format(SRGBA_16F);
            break;
        }
    } else
    {
#ifndef NDEBUG
        ERR_LOG("Failed to load texture PNG file" + fileName);
#endif
        return;
    };
#ifndef NDEBUG
    DEBUG_LOG("PNG Texture loaded successfully");
#endif // DEBUG
}
VKFW::Core::Transform VKFW::Tools::Loaders::SceneLoader::load_transform(tinyxml2::XMLElement* obj) {
    tinyxml2::XMLElement* transformElement = obj->FirstChildElement("Transform");

    Core::Transform transform = {};
    if (transformElement)
    {
        tinyxml2::XMLElement* scaleElement    = transformElement->FirstChildElement("scale");
        tinyxml2::XMLElement* positionElement = transformElement->FirstChildElement("translate");
        tinyxml2::XMLElement* rotationElement = transformElement->FirstChildElement("rotate");

        if (positionElement)
        {
            transform.position.x = positionElement->FloatAttribute("x");
            transform.position.y = positionElement->FloatAttribute("y");
            transform.position.z = positionElement->FloatAttribute("z");
        }

        if (rotationElement)
        {
            transform.rotation.x = math::radians(rotationElement->FloatAttribute("x"));
            transform.rotation.y = math::radians(rotationElement->FloatAttribute("y"));
            transform.rotation.z = math::radians(rotationElement->FloatAttribute("z"));
        }

        if (scaleElement)
        {
            transform.scale.x = scaleElement->FloatAttribute("x");
            transform.scale.y = scaleElement->FloatAttribute("y");
            transform.scale.z = scaleElement->FloatAttribute("z");
        }
    };
    return transform;
}

void VKFW::Tools::Loaders::SceneLoader::save_transform(const Core::Transform& transform,
                                                       tinyxml2::XMLElement*  parentElement) {
    tinyxml2::XMLDocument* doc = parentElement->GetDocument();

    if (!doc)
        return;

    tinyxml2::XMLElement* transformElement = doc->NewElement("Transform");

    // Save position
    tinyxml2::XMLElement* positionElement = doc->NewElement("translate");
    positionElement->SetAttribute("x", transform.position.x);
    positionElement->SetAttribute("y", transform.position.y);
    positionElement->SetAttribute("z", transform.position.z);
    transformElement->InsertEndChild(positionElement);

    // Save rotation
    tinyxml2::XMLElement* rotationElement = doc->NewElement("rotate");
    rotationElement->SetAttribute("x", math::degrees(transform.rotation.x));
    rotationElement->SetAttribute("y", math::degrees(transform.rotation.y));
    rotationElement->SetAttribute("z", math::degrees(transform.rotation.z));
    transformElement->InsertEndChild(rotationElement);

    // Save scale
    tinyxml2::XMLElement* scaleElement = doc->NewElement("scale");
    scaleElement->SetAttribute("x", transform.scale.x);
    scaleElement->SetAttribute("y", transform.scale.y);
    scaleElement->SetAttribute("z", transform.scale.z);
    transformElement->InsertEndChild(scaleElement);

    // Attach the transform element to the parent
    parentElement->InsertEndChild(transformElement);
}
void VKFW::Tools::Loaders::SceneLoader::load_children(tinyxml2::XMLElement* element,
                                                      Core::Object3D* const parent,
                                                      std::string           resourcesPath) {
    for (tinyxml2::XMLElement* meshElement = element->FirstChildElement("Mesh"); meshElement;
         meshElement                       = meshElement->NextSiblingElement("Mesh"))
    {
        Core::Mesh* mesh = new Core::Mesh();
        /*
        LOAD GEOMETRY
        */
        std::string meshType = std::string(meshElement->Attribute("type")); // "file" is expected
        if (meshType == "file")
        {
            tinyxml2::XMLElement* filenameElement = meshElement->FirstChildElement("Filename");
            if (filenameElement)
            {
                Loaders::load_3D_file(
                    mesh, resourcesPath + std::string(filenameElement->Attribute("value")), m_asyncLoad);
            }
        }
        if (meshType == "plane")
        {
            mesh->push_geometry(Core::Geometry::create_quad());
        }
        if (meshType == "sphere")
        {
            Loaders::load_3D_file(mesh, ENGINE_RESOURCES_PATH "meshes/sphere.obj", m_asyncLoad);
        }
        if (meshType == "cube")
        {
            mesh->push_geometry(Core::Geometry::create_cube());
        }

        /*
        SET TRANSFORM
        */
        mesh->set_transform(load_transform(meshElement));
        /*
        SET PARAMS
        */
        if (meshElement->Attribute("name"))
            mesh->set_name(meshElement->Attribute("name"));
        // if (meshElement->BoolAttribute("alphaTest"))
        //     mesh->ray_hittable(meshElement->BoolAttribute("rayHittable"))

        tinyxml2::XMLElement* materialElement = meshElement->FirstChildElement("Material");
        Core::IMaterial*      mat             = nullptr;
        if (materialElement)
        {
            std::string materialType = std::string(materialElement->Attribute("type"));
            if (materialType == "physical")
            {
                mat                              = new Core::PhysicalMaterial();
                Core::PhysicalMaterial* material = static_cast<Core::PhysicalMaterial*>(mat);

                if (materialElement->FirstChildElement("albedo"))
                {
                    Vec4 albedo = Vec4(0.0);
                    albedo.r    = materialElement->FirstChildElement("albedo")->FloatAttribute("r");
                    albedo.g    = materialElement->FirstChildElement("albedo")->FloatAttribute("g");
                    albedo.b    = materialElement->FirstChildElement("albedo")->FloatAttribute("b");
                    albedo.a    = materialElement->FirstChildElement("albedo")->FloatAttribute("a");
                    material->set_albedo(albedo);
                }

                if (materialElement->FirstChildElement("roughness"))
                    material->set_roughness(materialElement->FirstChildElement("roughness")->FloatAttribute("value"));
                if (materialElement->FirstChildElement("metalness"))
                    material->set_metalness(materialElement->FirstChildElement("metalness")->FloatAttribute("value"));

                if (materialElement->FirstChildElement("tile"))
                {
                    Vec2 tiling = Vec2(1, 1);
                    tiling.x    = materialElement->FirstChildElement("tile")->FloatAttribute("u");
                    tiling.y    = materialElement->FirstChildElement("tile")->FloatAttribute("v");
                    material->set_tile(tiling);
                }
                if (materialElement->FirstChildElement("emission"))
                {
                    Vec3 emission = Vec3(0.0);
                    emission.r    = materialElement->FirstChildElement("emission")->FloatAttribute("r");
                    emission.g    = materialElement->FirstChildElement("emission")->FloatAttribute("g");
                    emission.b    = materialElement->FirstChildElement("emission")->FloatAttribute("b");
                    material->set_emissive_color(emission);
                    if (materialElement->FirstChildElement("emissionPower"))
                        material->set_emission_intensity(
                            materialElement->FirstChildElement("emissionPower")->FloatAttribute("value"));
                }

                // Textures
                tinyxml2::XMLElement* texturesElement = materialElement->FirstChildElement("Textures");
                if (texturesElement)
                {
                    tinyxml2::XMLElement* albedoTexture = texturesElement->FirstChildElement("albedo");
                    if (albedoTexture)
                    {
                        Core::ITexture* texture = new Core::Texture();
                        Loaders::load_texture(texture,
                                              resourcesPath + std::string(albedoTexture->Attribute("path")),
                                              TEXTURE_FORMAT_SRGB,
                                              m_asyncLoad);
                        material->set_albedo_texture(texture);
                    }
                    tinyxml2::XMLElement* normalTexture = texturesElement->FirstChildElement("normals");
                    if (normalTexture)
                    {
                        Core::ITexture* texture = new Core::Texture();
                        Loaders::load_texture(texture,
                                              resourcesPath + std::string(normalTexture->Attribute("path")),
                                              TEXTURE_FORMAT_UNORM,
                                              m_asyncLoad);
                        material->set_normal_texture(texture);
                    }
                    tinyxml2::XMLElement* roughTexture = texturesElement->FirstChildElement("roughness");
                    if (roughTexture)
                    {
                        Core::ITexture* texture = new Core::Texture();
                        Loaders::load_texture(texture,
                                              resourcesPath + std::string(roughTexture->Attribute("path")),
                                              TEXTURE_FORMAT_UNORM,
                                              m_asyncLoad);
                        material->set_roughness_texture(texture);
                    }
                    tinyxml2::XMLElement* metalTexture = texturesElement->FirstChildElement("metalness");
                    if (metalTexture)
                    {
                        Core::ITexture* texture = new Core::Texture();
                        Loaders::load_texture(texture,
                                              resourcesPath + std::string(metalTexture->Attribute("path")),
                                              TEXTURE_FORMAT_UNORM,
                                              m_asyncLoad);
                        material->set_metallic_texture(texture);
                    }
                    tinyxml2::XMLElement* aoTexture = texturesElement->FirstChildElement("ao");
                    if (aoTexture)
                    {
                        Core::ITexture* texture = new Core::Texture();
                        Loaders::load_texture(texture,
                                              resourcesPath + std::string(aoTexture->Attribute("path")),
                                              TEXTURE_FORMAT_UNORM,
                                              m_asyncLoad);
                        material->set_occlusion_texture(texture);
                    }
                    tinyxml2::XMLElement* emissiveTexture = texturesElement->FirstChildElement("emission");
                    if (emissiveTexture)
                    {
                        Core::ITexture* texture = new Core::Texture();
                        Loaders::load_texture(texture,
                                              resourcesPath + std::string(emissiveTexture->Attribute("path")),
                                              TEXTURE_FORMAT_SRGB,
                                              m_asyncLoad);
                        material->set_emissive_texture(texture);
                    }
                    tinyxml2::XMLElement* maskTexture = texturesElement->FirstChildElement("mask");
                    if (maskTexture)
                    {
                        Core::ITexture* texture = new Core::Texture();
                        Loaders::load_texture(texture,
                                              resourcesPath + std::string(maskTexture->Attribute("path")),
                                              TEXTURE_FORMAT_UNORM,
                                              m_asyncLoad);
                        if (std::string(maskTexture->Attribute("type")) == "UnityHDRP")
                            material->set_mask_texture(texture, MaskType::UNITY_HDRP);
                        if (std::string(maskTexture->Attribute("type")) == "Unreal")
                            material->set_mask_texture(texture, MaskType::UNREAL_ENGINE);
                        if (std::string(maskTexture->Attribute("type")) == "UnityURP")
                            material->set_mask_texture(texture, MaskType::UNITY_URP);
                    }
                }
            }
            if (materialType == "phong")
            {
            }
            if (materialType == "unlit")
            {
                mat                           = new Core::UnlitMaterial();
                Core::UnlitMaterial* material = static_cast<Core::UnlitMaterial*>(mat);

                if (materialElement->FirstChildElement("color"))
                {
                    Vec4 color = Vec4(0.0);
                    color.r    = materialElement->FirstChildElement("color")->FloatAttribute("r");
                    color.g    = materialElement->FirstChildElement("color")->FloatAttribute("g");
                    color.b    = materialElement->FirstChildElement("color")->FloatAttribute("b");
                    color.a    = materialElement->FirstChildElement("color")->FloatAttribute("a");

                    material->set_color(color);
                }
                if (materialElement->FirstChildElement("tile"))
                {
                    Vec2 tiling = Vec2(1, 1);
                    tiling.x    = materialElement->FirstChildElement("tile")->FloatAttribute("u");
                    tiling.y    = materialElement->FirstChildElement("tile")->FloatAttribute("v");
                    material->set_tile(tiling);
                }
                tinyxml2::XMLElement* texturesElement = materialElement->FirstChildElement("Textures");
                if (texturesElement)
                {
                    tinyxml2::XMLElement* albedoTexture = texturesElement->FirstChildElement("color");
                    if (albedoTexture)
                    {
                        Core::ITexture* texture = new Core::Texture();
                        Loaders::load_texture(texture,
                                              resourcesPath + std::string(albedoTexture->Attribute("path")),
                                              TEXTURE_FORMAT_SRGB,
                                              m_asyncLoad);
                        material->set_color_texture(texture);
                    }
                }
            }
            if (materialType == "hair")
            {
            }
        }
        if (mat)
            mat->enable_alpha_test(meshElement->BoolAttribute("alphaTest"));
        mesh->push_material(mat ? mat : new Core::PhysicalMaterial(Vec4(0.5, 0.5, 0.5, 1.0)));

        if (meshElement->FirstChildElement("Children"))
            load_children(meshElement->FirstChildElement("Children"), mesh, resourcesPath);

        parent->add_child(mesh);
    }
    // Load lights
    for (tinyxml2::XMLElement* lightElement = element->FirstChildElement("Light"); lightElement;
         lightElement                       = lightElement->NextSiblingElement("Light"))
    {
        Core::Light* light     = nullptr;
        std::string  lightType = std::string(lightElement->Attribute("type"));
        if (lightType == "point")
        {
            light                        = new Core::PointLight();
            Core::PointLight* pointlight = static_cast<Core::PointLight*>(light);

            if (lightElement->FirstChildElement("influence"))
                pointlight->set_area_of_effect(lightElement->FirstChildElement("influence")->FloatAttribute("value"));
        }

        if (lightType == "directional")
        {
            light                            = new Core::DirectionalLight(Vec3(0.0));
            Core::DirectionalLight* dirLight = static_cast<Core::DirectionalLight*>(light);

            if (lightElement->FirstChildElement("direction"))
            {

                Vec3 dir = Vec3(1.0);
                dir.x    = lightElement->FirstChildElement("direction")->FloatAttribute("x");
                dir.y    = lightElement->FirstChildElement("direction")->FloatAttribute("y");
                dir.z    = lightElement->FirstChildElement("direction")->FloatAttribute("z");

                dirLight->set_direction(dir);
            }
        }
        if (lightType == "spot")
        {
        }
        if (light)
        {
            if (lightElement->Attribute("name"))
                light->set_name(lightElement->Attribute("name"));

            light->set_transform(load_transform(lightElement));

            if (lightElement->FirstChildElement("intensity"))
                light->set_intensity(lightElement->FirstChildElement("intensity")->FloatAttribute("value"));

            if (lightElement->FirstChildElement("color"))
            {
                Vec3 color = Vec3(0.0);
                color.r    = lightElement->FirstChildElement("color")->FloatAttribute("r");
                color.g    = lightElement->FirstChildElement("color")->FloatAttribute("g");
                color.b    = lightElement->FirstChildElement("color")->FloatAttribute("b");

                light->set_color(color);
            }
            // Shadows
            tinyxml2::XMLElement* shadowElement = lightElement->FirstChildElement("Shadow");
            if (shadowElement)
            {
                std::string shadowType = std::string(shadowElement->Attribute("type"));
                if (shadowType == "rt")
                {
                    light->set_shadow_type(ShadowType::RAYTRACED_SHADOW);
                    if (shadowElement->FirstChildElement("samples"))
                        light->set_shadow_ray_samples(
                            shadowElement->FirstChildElement("samples")->IntAttribute("value"));
                    if (shadowElement->FirstChildElement("area"))
                        light->set_area(shadowElement->FirstChildElement("area")->FloatAttribute("value"));
                }
                if (shadowType == "classic")
                {
                    light->set_shadow_type(ShadowType::BASIC_SHADOW);
                    /*
                    TBD
                    */
                }
                if (shadowType == "vsm")
                {
                    light->set_shadow_type(ShadowType::VSM_SHADOW);
                    /*
                    TBD
                    */
                }
            }

            if (lightElement->FirstChildElement("Children"))
                load_children(lightElement->FirstChildElement("Children"), light, resourcesPath);

            parent->add_child(light);
        }
    }
}

void VKFW::Tools::Loaders::SceneLoader::save_children(tinyxml2::XMLElement* parentElement,
                                                      Core::Object3D* const parent) {

    tinyxml2::XMLDocument* doc = parentElement->GetDocument();

    for (Core::Object3D* child : parent->get_children())
    {
        if (child->get_type() == ObjectType::MESH)
        {
            Core::Mesh* mesh = static_cast<Core::Mesh*>(child);

            tinyxml2::XMLElement* meshElement = doc->NewElement("Mesh");
            if (mesh->get_file_route() != "")
                meshElement->SetAttribute("type", "file");
            else
                meshElement->SetAttribute("type", "primitive");

            tinyxml2::XMLElement* filenameElement = doc->NewElement("Filename");
            filenameElement->SetAttribute("value", mesh->get_file_route().c_str());
            meshElement->InsertEndChild(filenameElement);
            // Save transform
            Core::Transform transform = mesh->get_transform();
            save_transform(transform, meshElement);

            meshElement->SetAttribute("name", mesh->get_name().c_str());

            // // Save material
            Core::IMaterial* mat = mesh->get_material();
            if (mat)
            {
                tinyxml2::XMLElement* materialElement = doc->NewElement("Material");
                materialElement->SetAttribute("type", mat->get_shaderpass_ID().c_str());

                if (mat->get_shaderpass_ID() == "physical")
                {
                    Core::PhysicalMaterial* material = static_cast<Core::PhysicalMaterial*>(mat);
                    // Save Albedo
                    tinyxml2::XMLElement* albedoElement = doc->NewElement("albedo");
                    Vec3                  albedo        = material->get_albedo();
                    albedoElement->SetAttribute("r", albedo.r);
                    albedoElement->SetAttribute("g", albedo.g);
                    albedoElement->SetAttribute("b", albedo.b);
                    materialElement->InsertEndChild(albedoElement);
                    tinyxml2::XMLElement* opElement = doc->NewElement("opacity");
                    opElement->SetAttribute("value", material->get_opacity());
                    materialElement->InsertEndChild(opElement);

                    // Save Roughness
                    tinyxml2::XMLElement* roughnessElement = doc->NewElement("roughness");
                    roughnessElement->SetAttribute("value", material->get_roughness());
                    materialElement->InsertEndChild(roughnessElement);

                    // Save Metallic
                    tinyxml2::XMLElement* metallicElement = doc->NewElement("metallic");
                    metallicElement->SetAttribute("value", material->get_metalness());
                    materialElement->InsertEndChild(metallicElement);

                    // Save Emission
                    tinyxml2::XMLElement* emissionElement = doc->NewElement("emission");
                    Vec3                  emission        = material->get_emissive_color();
                    emissionElement->SetAttribute("r", emission.r);
                    emissionElement->SetAttribute("g", emission.g);
                    emissionElement->SetAttribute("b", emission.b);
                    materialElement->InsertEndChild(emissionElement);

                    // Save Textures
                    if (!material->get_textures().empty())
                    {
                        tinyxml2::XMLElement* texturesElement = doc->NewElement("Textures");

                        if (material->get_albedo_texture())
                        {
                            tinyxml2::XMLElement* albedoTextureElement = doc->NewElement("albedo");
                            albedoTextureElement->SetAttribute(
                                "path", material->get_albedo_texture()->get_file_route().c_str());
                            texturesElement->InsertEndChild(albedoTextureElement);
                        }

                        if (material->get_normal_texture())
                        {
                            tinyxml2::XMLElement* normalsTextureElement = doc->NewElement("normals");
                            normalsTextureElement->SetAttribute(
                                "path", material->get_normal_texture()->get_file_route().c_str());
                            texturesElement->InsertEndChild(normalsTextureElement);
                        }
                        if (material->get_roughness_texture())
                        {
                            tinyxml2::XMLElement* normalsTextureElement = doc->NewElement("roughness");
                            normalsTextureElement->SetAttribute(
                                "path", material->get_roughness_texture()->get_file_route().c_str());
                            texturesElement->InsertEndChild(normalsTextureElement);
                        }
                        if (material->get_metallic_texture())
                        {
                            tinyxml2::XMLElement* normalsTextureElement = doc->NewElement("metalness");
                            normalsTextureElement->SetAttribute(
                                "path", material->get_metallic_texture()->get_file_route().c_str());
                            texturesElement->InsertEndChild(normalsTextureElement);
                        }

                        if (material->get_emissive_texture())
                        {
                            tinyxml2::XMLElement* emissionTextureElement = doc->NewElement("emission");
                            emissionTextureElement->SetAttribute(
                                "path", material->get_emissive_texture()->get_file_route().c_str());
                            texturesElement->InsertEndChild(emissionTextureElement);
                        }

                        materialElement->InsertEndChild(texturesElement);
                    }
                }
                // Append to parent element
                meshElement->InsertEndChild(materialElement);
            }

            // Recursively save children
            tinyxml2::XMLElement* childrenElement = doc->NewElement("Children");
            save_children(childrenElement, mesh);
            meshElement->InsertEndChild(childrenElement);

            parentElement->InsertEndChild(meshElement);
        }
        if (child->get_type() == ObjectType::LIGHT)
        {
            Core::Light* light = static_cast<Core::Light*>(child);

            tinyxml2::XMLElement* lightElement     = doc->NewElement("Light");
            tinyxml2::XMLElement* directionElement = nullptr;
            tinyxml2::XMLElement* influenceElement = nullptr;
            switch (light->get_light_type())
            {
            case LightType::POINT:
                lightElement->SetAttribute("type", "point");
                influenceElement = doc->NewElement("influence");
                influenceElement->SetAttribute("value", static_cast<Core::PointLight*>(light)->get_area_of_effect());
                lightElement->InsertEndChild(influenceElement);
                break;
            case LightType::DIRECTIONAL:
                lightElement->SetAttribute("type", "directional");
                directionElement = doc->NewElement("direction");
                Vec3 direction   = static_cast<Core::DirectionalLight*>(light)->get_direction();
                directionElement->SetAttribute("x", direction.x);
                directionElement->SetAttribute("y", direction.y);
                directionElement->SetAttribute("z", direction.z);
                lightElement->InsertEndChild(directionElement);
                break;
            case LightType::SPOT:
                lightElement->SetAttribute("type", "spot");
                break;
            }
            lightElement->SetAttribute("name", light->get_name().c_str());

            // Save transform
            Core::Transform transform = light->get_transform();
            save_transform(transform, lightElement);

            // Save Intensity
            tinyxml2::XMLElement* intensityElement = doc->NewElement("intensity");
            intensityElement->SetAttribute("value", light->get_intensity());
            lightElement->InsertEndChild(intensityElement);

            // Save Color
            tinyxml2::XMLElement* colorElement = doc->NewElement("color");
            Vec3                  color        = light->get_color();
            colorElement->SetAttribute("r", color.r);
            colorElement->SetAttribute("g", color.g);
            colorElement->SetAttribute("b", color.b);
            lightElement->InsertEndChild(colorElement);

            if (light->get_cast_shadows())
            {
                tinyxml2::XMLElement* shadowElement = doc->NewElement("Shadow");
                switch (light->get_shadow_type())
                {
                case ShadowType::BASIC_SHADOW:
                    shadowElement->SetAttribute("type", "basic");
                    break;
                case ShadowType::VSM_SHADOW:
                    shadowElement->SetAttribute("type", "vsm");
                    break;
                case ShadowType::RAYTRACED_SHADOW:
                    shadowElement->SetAttribute("type", "rt");
                    break;
                }

                tinyxml2::XMLElement* samplesElement = doc->NewElement("samples");
                samplesElement->SetAttribute("value", light->get_shadow_ray_samples());
                shadowElement->InsertEndChild(samplesElement);

                tinyxml2::XMLElement* areaElement = doc->NewElement("area");
                areaElement->SetAttribute("value", light->get_area());
                shadowElement->InsertEndChild(areaElement);

                lightElement->InsertEndChild(shadowElement);
            }

            // Recursively save children
            tinyxml2::XMLElement* childrenElement = doc->NewElement("Children");
            save_children(childrenElement, light);
            lightElement->InsertEndChild(childrenElement);

            parentElement->InsertEndChild(lightElement);
        }
    }
}
void VKFW::Tools::Loaders::SceneLoader::load_scene(Core::Scene* const scene, const std::string fileName) {

    if (!scene)
        throw VKFW_Exception("Scene is null pointer");

    tinyxml2::XMLDocument doc;
    doc.LoadFile(fileName.c_str());

    std::string resources = "";
    if (doc.FirstChildElement("Scene")->FirstChildElement("Resources"))
    {
        resources = std::string(doc.FirstChildElement("Scene")->FirstChildElement("Resources")->Attribute("path"));
    }

    // Load the camera
    tinyxml2::XMLElement* cameraElement = doc.FirstChildElement("Scene")->FirstChildElement("Camera");
    if (cameraElement)
    {
        Core::Camera* camera = new Core::Camera();
        /*
        SET TRANSFORM
        */
        camera->set_transform(load_transform(cameraElement));
        camera->set_rotation({-90, 0, 0}, true);
        /*
        SET PARAMS
        */
        camera->set_far(cameraElement->FloatAttribute("far", 100.0f));
        camera->set_near(cameraElement->FloatAttribute("near", 0.1f));
        camera->set_field_of_view(cameraElement->FloatAttribute("fov", 75.0f));
        scene->add(camera);
    }

    // Load Hierarqy of children
    load_children(doc.FirstChildElement("Scene"), scene, resources);

    // Ambient
    tinyxml2::XMLElement* ambientElement = doc.FirstChildElement("Scene")->FirstChildElement("Enviroment");
    if (ambientElement)
    {
        std::string ambientType = std::string(ambientElement->Attribute("type"));
        if (ambientType == "constant")
        {
            if (ambientElement->FirstChildElement("intensity"))
                scene->set_ambient_intensity(ambientElement->FirstChildElement("intensity")->FloatAttribute("value"));
            if (ambientElement->FirstChildElement("color"))
            {
                Vec3 color = Vec3(0.0);
                color.r    = ambientElement->FirstChildElement("color")->FloatAttribute("r");
                color.g    = ambientElement->FirstChildElement("color")->FloatAttribute("g");
                color.b    = ambientElement->FirstChildElement("color")->FloatAttribute("b");

                scene->set_ambient_color(color);
            }
        }
        if (ambientType == "HDRi")
        {
            tinyxml2::XMLElement* filenameElement = ambientElement->FirstChildElement("Filename");
            if (filenameElement)
            {
                if (ambientElement->FirstChildElement("color"))
                {
                    Vec3 color = Vec3(0.0);
                    color.r    = ambientElement->FirstChildElement("color")->FloatAttribute("r");
                    color.g    = ambientElement->FirstChildElement("color")->FloatAttribute("g");
                    color.b    = ambientElement->FirstChildElement("color")->FloatAttribute("b");

                    scene->set_ambient_color(color);
                }
                Core::TextureHDR* envMap = new Core::TextureHDR();
                Tools::Loaders::load_texture(envMap,
                                             resources + std::string(filenameElement->Attribute("value")),
                                             TEXTURE_FORMAT_HDR,
                                             m_asyncLoad);
                Core::Skybox* sky = new Core::Skybox(envMap);
                sky->set_sky_type(EnviromentType::IMAGE_BASED_ENV);
                if (ambientElement->FirstChildElement("intensity"))
                {
                    sky->set_color_intensity(ambientElement->FirstChildElement("intensity")->FloatAttribute("value"));
                    scene->set_ambient_intensity(
                        ambientElement->FirstChildElement("intensity")->FloatAttribute("value"));
                }
                scene->set_skybox(sky);
            }
        }
        if (ambientType == "procedural")
        {

            Core::Skybox* sky = new Core::Skybox();
            sky->set_sky_type(EnviromentType::PROCEDURAL_ENV);
            if (ambientElement->FirstChildElement("intensity"))
            {
                sky->set_color_intensity(ambientElement->FirstChildElement("intensity")->FloatAttribute("value"));
                scene->set_ambient_intensity(ambientElement->FirstChildElement("intensity")->FloatAttribute("value"));
            }
            scene->set_skybox(sky);
        }
    }

    else
    {
        scene->set_ambient_color(Vec3(0.0));
        scene->set_ambient_intensity(0.0f);
    }
}

void VKFW::Tools::Loaders::SceneLoader::save_scene(Core::Scene* const scene, const std::string fileName) {
    if (!scene)
    {
        throw VKFW_Exception("Scene is null pointer");
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* sceneElement = doc.NewElement("Scene");

    doc.InsertFirstChild(sceneElement);

    tinyxml2::XMLElement* resourcesElement = doc.NewElement("Resources");
    resourcesElement->SetAttribute("path", "");
    sceneElement->InsertEndChild(resourcesElement);

    // Save camera
    Core::Camera* camera = scene->get_active_camera();
    if (camera)
    {
        tinyxml2::XMLElement* cameraElement = doc.NewElement("Camera");

        Core::Transform transform = camera->get_transform();
        save_transform(transform, cameraElement);
        cameraElement->SetAttribute("far", camera->get_far());
        cameraElement->SetAttribute("near", camera->get_near());
        cameraElement->SetAttribute("fov", camera->get_field_of_view());

        sceneElement->InsertEndChild(cameraElement);
    }

    // Recursively save children
    save_children(sceneElement, scene); // Assume get_root() gets the root object

    // Save environment settings
    tinyxml2::XMLElement* environmentElement = doc.NewElement("Enviroment");
    Vec3                  ambientColor       = scene->get_ambient_color();
    float                 ambientIntensity   = scene->get_ambient_intensity();

    if (scene->get_skybox())
    {
        environmentElement->SetAttribute("type", "skybox");

        tinyxml2::XMLElement* filenameElement = doc.NewElement("Filename");
        filenameElement->SetAttribute("value", scene->get_skybox()->get_enviroment_map()->get_file_route().c_str());
        environmentElement->InsertEndChild(filenameElement);
    } else
    {
        environmentElement->SetAttribute("type", "constant");

        tinyxml2::XMLElement* colorElement = doc.NewElement("color");
        colorElement->SetAttribute("r", ambientColor.r);
        colorElement->SetAttribute("g", ambientColor.g);
        colorElement->SetAttribute("b", ambientColor.b);
        environmentElement->InsertEndChild(colorElement);
    }

    tinyxml2::XMLElement* intensityElement = doc.NewElement("intensity");
    intensityElement->SetAttribute("value", ambientIntensity);
    environmentElement->InsertEndChild(intensityElement);

    sceneElement->InsertEndChild(environmentElement);

    // Save the XML to file
    doc.SaveFile(fileName.c_str());
}
