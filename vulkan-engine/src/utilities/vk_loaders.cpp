#define TINYOBJLOADER_IMPLEMENTATION
#define TINYPLY_IMPLEMENTATION
#include <engine/utilities/vk_loaders.h>

bool vke::loaders::load_OBJ(Mesh *const mesh, bool overrideGeometry, const std::string fileName, bool importMaterials, bool calculateTangents)
{

    // Preparing output
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

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
        return false;
    }

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    size_t shape_id = 0;
    for (const tinyobj::shape_t &shape : shapes)
    {
        if (!shape.mesh.indices.empty())
        {
            // IS INDEXED
            for (const tinyobj::index_t &index : shape.mesh.indices)
            {
                Vertex vertex = {};

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
        }
        else
            // NOT INDEXED
            for (size_t i = 0; i < shape.mesh.num_face_vertices.size(); i++)
            {
                for (size_t j = 0; j < shape.mesh.num_face_vertices[i]; j++)
                {
                    Vertex vertex{};
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
            Geometry *oldGeom = mesh->get_geometry(shape_id);
            if (oldGeom)
            {
                oldGeom->fill(vertices, indices);
                continue;
            }
        }

        Geometry *g = new Geometry();
        g->fill(vertices, indices);
        mesh->set_geometry(g);

        shape_id++;
    }
    return true;
}
void vke::loaders::compute_tangents_gram_smidt(std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices)
{
    if (!indices.empty())
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            size_t i0 = indices[i];
            size_t i1 = indices[i + 1];
            size_t i2 = indices[i + 2];

            Vec3 tangent = utils::get_tangent_gram_smidt(vertices[i0].pos, vertices[i1].pos, vertices[i2].pos,
                                                                vertices[i0].texCoord, vertices[i1].texCoord, vertices[i2].texCoord,
                                                                vertices[i0].normal);

            vertices[i0].tangent += tangent;
            vertices[i1].tangent += tangent;
            vertices[i2].tangent += tangent;
        }
    else
        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            Vec3 tangent = utils::get_tangent_gram_smidt(vertices[i].pos, vertices[i + 1].pos, vertices[i + 2].pos,
                                                                vertices[i].texCoord, vertices[i + 1].texCoord, vertices[i + 2].texCoord,
                                                                vertices[i].normal);

            vertices[i].tangent += tangent;
            vertices[i + 1].tangent += tangent;
            vertices[i + 2].tangent += tangent;
        }
}
bool vke::loaders::load_PLY(Mesh *const mesh, bool overrideGeometry, const std::string fileName, bool preload, bool verbose, bool calculateTangents)
{

    std::unique_ptr<std::istream> file_stream;
    std::vector<uint8_t> byte_buffer;
    try
    {
        // For most files < 1gb, pre-loading the entire file upfront and wrapping it into a
        // stream is a net win for parsing speed, about 40% faster.
        if (preload)
        {
            byte_buffer = utils::read_file_binary(fileName);
            file_stream.reset(new utils::memory_stream((char *)byte_buffer.data(), byte_buffer.size()));
        }
        else
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
            for (const auto &c : file.get_comments())
                std::cout << "\t[ply_header] Comment: " << c << std::endl;
            for (const auto &c : file.get_info())
                std::cout << "\t[ply_header] Info: " << c << std::endl;

            for (const auto &e : file.get_elements())
            {
                std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
                for (const auto &p : e.properties)
                {
                    std::cout << "\t[ply_header] \tproperty: " << p.name << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
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
        {
            positions = file.request_properties_from_element("vertex", {"x", "y", "z"});
        }
        catch (const std::exception &e)
        {
            std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            normals = file.request_properties_from_element("vertex", {"nx", "ny", "nz"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            colors = file.request_properties_from_element("vertex", {"red", "green", "blue", "alpha"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            colors = file.request_properties_from_element("vertex", {"r", "g", "b", "a"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            texcoords = file.request_properties_from_element("vertex", {"u", "v"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            texcoords = file.request_properties_from_element("vertex", {"s", "t"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        // Providing a list size hint (the last argument) is a 2x performance improvement. If you have
        // arbitrary ply files, it is best to leave this 0.
        try
        {
            faces = file.request_properties_from_element("face", {"vertex_indices"}, 3);
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        // Tristrips must always be read with a 0 list size hint (unless you know exactly how many elements
        // are specifically in the file, which is unlikely);
        try
        {
            tripstrip = file.request_properties_from_element("tristrips", {"vertex_indices"}, 0);
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }
        utils::ManualTimer readTimer;

        readTimer.start();
        file.read(*file_stream);
        readTimer.stop();

        if (verbose)
        {

            const float parsingTime = static_cast<float>(readTimer.get()) / 1000.f;
            std::cout << "\tparsing " << size_mb << "mb in " << parsingTime << " seconds [" << (size_mb / parsingTime) << " MBps]" << std::endl;

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
                std::cout << "\tRead " << (tripstrip->buffer.size_bytes() / tinyply::PropertyTable[tripstrip->t].stride) << " total indices (tristrip) " << std::endl;
        }

        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        if (positions)
        {
            const float *posData = reinterpret_cast<const float *>(positions->buffer.get());
            const float *normalData;
            const float *colorData;
            const float *uvData;
            if (normals)
                normalData = reinterpret_cast<const float *>(normals->buffer.get());
            if (colors)
                colorData = reinterpret_cast<const float *>(colors->buffer.get());
            if (texcoords)
                uvData = reinterpret_cast<const float *>(texcoords->buffer.get());

            for (size_t i = 0; i < positions->count; i++)
            {

                Vec3 position = Vec3(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]);
                Vec3 normal = normals ? Vec3(normalData[i * 3], normalData[i * 3 + 1], normalData[i * 3 + 2]) : Vec3(0.0f);
                Vec3 color = colors ? Vec3(colorData[i * 3], colorData[i * 3 + 1], colorData[i * 3 + 2]) : Vec3(1.0f);
                Vec2 uv = texcoords ? Vec2(uvData[i * 2], uvData[i * 2 + 1]) : Vec2(0.0f);

                vertices.push_back({position, normal, {0.0f, 0.0f, 0.0f}, uv, color});
            }
        }
        unsigned *facesData = reinterpret_cast<unsigned *>(faces->buffer.get());
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
            Geometry *oldGeom = mesh->get_geometry();
            if (oldGeom)
            {
                oldGeom->fill(vertices, indices);
            }
            return true;
        }

        Geometry *g = new Geometry();
        g->fill(vertices, indices);
        mesh->set_geometry(g);

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }

    return false;
}
bool vke::loaders::load_3D_file(Mesh *const mesh, bool overrideGeometry, const std::string fileName)
{
    size_t dotPosition = fileName.find_last_of(".");

    if (dotPosition != std::string::npos)
    {

        std::string fileExtension = fileName.substr(dotPosition + 1);

        if (fileExtension == OBJ)
        {
            return loaders::load_OBJ(mesh, overrideGeometry, fileName, false, true);
        }
        else if (fileExtension == PLY)
        {
            return loaders::load_PLY(mesh, overrideGeometry, fileName, true, false, true);
        }
        else
        {
            std::cerr << "Unsupported file format: " << fileExtension << std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "Invalid file name: " << fileName << std::endl;
        return false;
    }
}