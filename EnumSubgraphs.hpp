#ifndef ENUM_SUBGRAPHS_HPP
#define ENUM_SUBGRAPHS_HPP

#include <iostream>

class EnumSubgraphs {
public:
    template <int ARITY>
    static void enumColorfulSubgraphs(std::ostream& os,
        const Graph& graph, const tdzdd::DdStructure< ARITY >& dd)
    {
        std::vector<std::pair<int, int> > vec;
        enumColorfulSubgraphs(dd.root(), vec, os, graph, dd);
    }

    static void enumSubgraphs(std::ostream& os,
        const Graph& graph, const tdzdd::DdStructure<2>& dd)
    {
        enumColorfulSubgraphs<2>(os, graph, dd);
    }

private:
    template <int ARITY>
    static void enumColorfulSubgraphs(NodeId node,
        std::vector<std::pair<int, int> >& vec,
        std::ostream& os, const Graph& graph,
        const tdzdd::DdStructure< ARITY >& dd)
    {
        if (node == 0) { // reach 0-terminal
            return;
        } else if (node == 1) { // reach 1-terminal. Output the corresponding set.
            for (int i = graph.edgeSize(); i >= 1; --i) {
                bool found = false;
                for (size_t j = 0; j < vec.size(); ++j) {
                    if (vec[j].first == i) {
                        found = true;
                        os << vec[j].second;
                        break;
                    }
                }
                if (!found) {
                    os << "0";
                }
                if (i > 1) {
                    os << " ";
                }
            }
            os << "\n";
            return;
        } else {
            for (int c = 0; c < ARITY; ++c) {
                NodeId cnode = dd.child(node, c); // get c-child node
                if (c >= 1) {
                    // store the pair of the edge number and color number
                    vec.push_back(std::make_pair(node.row(), c));
                }
                // recursive call
                enumColorfulSubgraphs(cnode, vec, os, graph, dd);
                if (c >= 1) {
                    vec.pop_back();
                }
            }
        }
    }
};

#endif // ENUM_SUBGRAPHS_HPP
