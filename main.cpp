#include <iostream>


#include "cxxopts.hpp"
#include "src/load_mesh.h"


namespace {

void printMeshDetails(const Mesh::TriMesh& mesh) {
    std::cout << "Mesh has " << mesh.numTriangles() << " triangles and " << mesh.numVertices() << " vertices"
              << std::endl;
    std::cout << "Mesh contains " << mesh.getTriangles().size() << " triangles" << std::endl;
    std::cout << "Mesh contains " << mesh.getVertices().size() << " vertices" << std::endl;
}

} // namespace


int main(int argc, char* argv[]) {
    cxxopts::Options options(argv[0], "Learn Mesh");
    options.add_options()("f, file", "Input file", cxxopts::value<std::string>())("h,help", "Print usage information");

    const auto parsedArgs = options.parse(argc, argv);
    if (parsedArgs.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    const auto filename = parsedArgs["file"].as<std::string>();
    std::cout << "Loading " << filename << std::endl;

    const auto loadMeshResult = Mesh::loadMesh(filename);
    if (!loadMeshResult) {
        std::cerr << "Failed to load mesh from " << filename << " with error : " << loadMeshResult.error().message
                  << std::endl;
        return 1;
    }
    printMeshDetails(loadMeshResult.value());
    std::cout << "Done" << std::endl;
    return 0;
}