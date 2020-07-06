#include <iostream>
#include <string>
#include <sstream>

#include "tdzdd/DdSpec.hpp"
#include "tdzdd/DdEval.hpp"
#include "tdzdd/eval/Cardinality.hpp"
#include "tdzdd/DdStructure.hpp"
#include "tdzdd/util/Graph.hpp"

using namespace tdzdd;

#include "FrontierExample.hpp"

std::string getVertex(int i, int j) {
    std::ostringstream oss;
    oss << i << ":" << j;
    return oss.str();
}

void makeGridGraph(tdzdd::Graph& graph, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (j < n - 1) {
                graph.addEdge(getVertex(i, j), getVertex(i, j + 1));
            }
            if (i < n - 1) {
                graph.addEdge(getVertex(i, j), getVertex(i + 1, j));
            }
        }
    }
    graph.update();
}

int main(int argc, char** argv) {

    if (argc == 1) {
        // from https://oeis.org/A140517
        std::string solutions[] = {"0", "0", "1", "13", "213", "9349",
                                   "1222363", "487150371", "603841648931",
                                   "2318527339461265",
                                   "27359264067916806101"};

        for (int n = 2; n <= 10; ++n) {
            tdzdd::Graph graph;
            makeGridGraph(graph, n);
            FrontierManager fm(graph);
            FrontierExampleSpec spec(graph);
            DdStructure<2> dd(spec);
            std::cerr << "n = " << n << ", # of solutions = "
                      << dd.zddCardinality();
            if (dd.zddCardinality() != solutions[n]) {
                std::cerr << ", which is different from expected number "
                          << solutions[n] << ".";
            }
            std::cerr << std::endl;
        }
    } else {
        tdzdd::MessageHandler::showMessages(true);

        tdzdd::Graph graph;

        graph.readEdges(argv[1]);

        FrontierManager fm(graph);

        std::cerr << "# of vertices = " << graph.vertexSize() << std::endl;
        std::cerr << "# of edges = " << graph.edgeSize() << std::endl;

        FrontierExampleSpec spec(graph);

        DdStructure<2> dd(spec);

        std::cerr << "# of ZDD nodes = " << dd.size() << std::endl;
        std::cerr << "# of solutions = " << dd.zddCardinality() << std::endl;

        //dd.dumpDot(std::cout);
    }

    return 0;
}
