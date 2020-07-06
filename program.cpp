#include <iostream>


#include "tdzdd/DdSpec.hpp"
#include "tdzdd/DdEval.hpp"
#include "tdzdd/eval/Cardinality.hpp"
//#include "tdzdd/DdSpecOp.hpp"
#include "tdzdd/DdStructure.hpp"
#include "tdzdd/util/Graph.hpp"

using namespace tdzdd;

#include "FrontierExample.hpp"

int main(int argc, char** argv) {

    if (argc <= 1) {
        std::cerr << "usage: " << argv[0] << " <graph_file>" << std::endl;
        return 1;
    }

    tdzdd::MessageHandler::showMessages(true);

    tdzdd::Graph graph;

    graph.readEdges(argv[1]);

    FrontierManager fm(graph);

    std::cerr << "# of vertices = " << graph.vertexSize() << std::endl;
    std::cerr << "# of edges = " << graph.edgeSize() << std::endl;

    FrontierExampleSpec spec(graph);
    /*
    FrontierValue fv;
    FrontierData data[100];

    std::cerr << spec.getRoot(fv, data) << std::endl;
    std::cerr << spec.getChild(fv, data, 12, 1) << std::endl;
    std::cerr << spec.getChild(fv, data, 11, 1) << std::endl;
    std::cerr << spec.getChild(fv, data, 10, 0) << std::endl;
    std::cerr << spec.getChild(fv, data, 9, 1) << std::endl;
    std::cerr << spec.getChild(fv, data, 8, 0) << std::endl;
    std::cerr << spec.getChild(fv, data, 7, 1) << std::endl;
    */

    DdStructure<2> dd(spec);

    std::cerr << "# of ZDD nodes = " << dd.size() << std::endl;
    std::cerr << "# of solutions = " << dd.zddCardinality() << std::endl;

    //dd.dumpDot(std::cout);

    return 0;
}
