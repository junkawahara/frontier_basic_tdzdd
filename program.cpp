#include <iostream>
#include <string>
#include <sstream>

#include "tdzdd/DdSpec.hpp"
#include "tdzdd/DdEval.hpp"
#include "tdzdd/eval/Cardinality.hpp"
#include "tdzdd/DdStructure.hpp"
#include "tdzdd/util/Graph.hpp"

using namespace tdzdd;

#include "FrontierManager.hpp"
#include "FrontierSingleCycle.hpp"
#include "FrontierSTPath.hpp"
#include "FrontierTree.hpp"
#include "FrontierMatching.hpp"
#include "FrontierMate.hpp"

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
            FrontierSingleCycleSpec spec(graph, false);
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
        tdzdd::Graph graph;
        bool is_path = false;
        bool is_ham_path = false;
        bool is_cycle = false;
        bool is_ham_cycle = false;
        bool is_forest = false;
        bool is_tree = false;
        bool is_stree = false;
        bool is_matching = false;
        bool is_cmatching = false;
        bool is_dot = false;

        bool readfirst = false;
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == std::string("--path")) {
                is_path = true;
            } else if (std::string(argv[i]) == std::string("--hampath")) {
                is_ham_path = true;
            } else if (std::string(argv[i]) == std::string("--cycle")) {
                is_cycle = true;
            } else if (std::string(argv[i]) == std::string("--hamcycle")) {
                is_ham_cycle = true;
            } else if (std::string(argv[i]) == std::string("--forest")) {
                is_forest = true;
            } else if (std::string(argv[i]) == std::string("--tree")) {
                is_tree = true;
            } else if (std::string(argv[i]) == std::string("--stree")) {
                is_stree = true;
            } else if (std::string(argv[i]) == std::string("--matching")) {
                is_matching = true;
            } else if (std::string(argv[i]) == std::string("--cmatching")) {
                is_cmatching = true;
            } else if (std::string(argv[i]) == std::string("--show")) {
                tdzdd::MessageHandler::showMessages(true);
            } else if (std::string(argv[i]) == std::string("--dot")) {
                is_dot = true;
            } else if (argv[i][0] == '-') {
                std::cerr << "unknown option " << argv[i] << std::endl;
                return 1;
            } else {
                if (!readfirst) {
                    readfirst = true;
                    graph.readEdges(argv[i]);
                } else {
                    std::cerr << "illegal argument " << argv[i] << std::endl;
                    return 1;
                }
            }
        }

        FrontierManager fm(graph);

        std::cerr << "# of vertices = " << graph.vertexSize() << std::endl;
        std::cerr << "# of edges = " << graph.edgeSize() << std::endl;

        DdStructure<2> dd;

        if (is_path) {
            std::ostringstream oss;
            oss << graph.vertexSize();
            //FrontierSTPathSpec spec(graph, false, graph.getVertex("1"), graph.getVertex(oss.str()));
            FrontierMateSpec spec(graph, false, false, graph.getVertex("1"), graph.getVertex(oss.str()));
            dd = DdStructure<2>(spec);
        } else if (is_ham_path) {
            FrontierSTPathSpec spec(graph, true, 1, graph.vertexSize());
            dd = DdStructure<2>(spec);
        } else if (is_cycle) {
            //FrontierSingleCycleSpec spec(graph, false);
            //std::cerr << "here";
            FrontierMateSpec spec(graph, true, false, -1, -1);
            dd = DdStructure<2>(spec);
        } else if (is_ham_cycle) {
            FrontierSingleCycleSpec spec(graph, true);
            dd = DdStructure<2>(spec);
        } else if (is_forest) {
            FrontierTreeSpec spec(graph, false, false);
            dd = DdStructure<2>(spec);
        } else if (is_tree) {
            FrontierTreeSpec spec(graph, true, false);
            dd = DdStructure<2>(spec);
        } else if (is_stree) {
            FrontierTreeSpec spec(graph, true, true);
            dd = DdStructure<2>(spec);
        } else if (is_matching) {
            FrontierMatchingSpec spec(graph, false);
            dd = DdStructure<2>(spec);
        } else if (is_cmatching) {
            FrontierMatchingSpec spec(graph, true);
            dd = DdStructure<2>(spec);
        } else {
            std::cerr << "Please specify a kind of subgraphs." << std::endl;
            exit(1);
        }

        std::cerr << "# of ZDD nodes = " << dd.size() << std::endl;
        std::cerr << "# of solutions = " << dd.zddCardinality() << std::endl;

        if (is_dot) {
            dd.dumpDot(std::cout);
        }
    }

    return 0;
}
