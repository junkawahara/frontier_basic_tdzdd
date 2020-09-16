#ifndef FRONTIER_TREE_HPP
#define FRONTIER_TREE_HPP

#include <vector>

using namespace tdzdd;

typedef unsigned short ushort;

typedef unsigned short FrontierTreeData;

class FrontierTreeSpec
    : public tdzdd::PodArrayDdSpec<FrontierTreeSpec, FrontierTreeData, 2> {
private:
    // input graph
    const tdzdd::Graph& graph_;
    // number of vertices
    const int n_;
    // number of edges
    const int m_;

    const bool isSpanning_;

    const FrontierManager fm_;

    // This function gets whether the degree of v is at least 1 or not.
    bool getDeg(FrontierTreeData* data, int v) const {
        return ((data[fm_.vertexToPos(v)] >> 15) & 1u) != 0;
    }

    // This function sets deg of v to be d.
    void setDeg(FrontierTreeData* data, int v) const {
        data[fm_.vertexToPos(v)] |= (1u << 15);
    }

    // This function sets deg of v to be d.
    void resetDeg(FrontierTreeData* data, int v) const {
        data[fm_.vertexToPos(v)] &= ~(1u << 15);
    }

    // This function gets comp of v.
    ushort getComp(FrontierTreeData* data, int v) const {
        return data[fm_.vertexToPos(v)] & 0x7fffu;
    }

    // This function sets comp of v to be c.
    void setComp(FrontierTreeData* data, int v, ushort c) const {
        assert(c < 0x8000u);
        data[fm_.vertexToPos(v)] = (0x8000u & data[fm_.vertexToPos(v)]) | c;
    }

    void initializeData(FrontierTreeData* data) const {
        for (int i = 0; i < fm_.getMaxFrontierSize(); ++i) {
            data[i] = 0;
        }
    }

public:
    FrontierTreeSpec(const tdzdd::Graph& graph,
                     bool isSpanning) : graph_(graph),
                                        n_(graph_.vertexSize()),
                                        m_(graph_.edgeSize()),
                                        isSpanning_(isSpanning),
                                        fm_(graph_)
    {
        if (n_ >= (1 << 15)) {
            std::cerr << "The number of vertices must be smaller than 2^15."
                      << std::endl;
            exit(1);
        }
        setArraySize(fm_.getMaxFrontierSize());
    }

    int getRoot(FrontierTreeData* data) const {
        initializeData(data);
        return m_;
    }

    int getChild(FrontierTreeData* data, int level, int value) const {
        assert(1 <= level && level <= m_);

        // edge index (starting from 0)
        int edge_index = m_ - level;
        // edge that we are processing.
        // The endpoints of "edge" are edge.v1 and edge.v2.
        const Graph::EdgeInfo& edge = graph_.edgeInfo(edge_index);

        // initialize deg and comp of the vertices newly entering the frontier
        const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
        for (size_t i = 0; i < entering_vs.size(); ++i) {
            int v = entering_vs[i];
            // initially the value of comp is the vertex number itself
            setComp(data, v, static_cast<ushort>(v));
        }

        // vertices on the frontier
        const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

        if (value == 1) { // if we take the edge (go to 1-arc)
            ushort c1 = getComp(data, edge.v1);
            ushort c2 = getComp(data, edge.v2);

            if (c1 == c2) { // Any cycle must not occur.
                return 0;
            }

            // increment deg of v1 and v2 (recall that edge = {v1, v2})
            setDeg(data, edge.v1);
            setDeg(data, edge.v2);

            if (c1 != c2) { // connected components c1 and c2 become connected
                ushort cmin = std::min(c1, c2);
                ushort cmax = std::max(c1, c2);

                // replace component number cmin with cmax
                for (size_t i = 0; i < frontier_vs.size(); ++i) {
                    int v = frontier_vs[i];
                    if (getComp(data, v) == cmin) {
                        setComp(data, v, cmax);
                    }
                }
            }
        }

        // vertices that are leaving the frontier
        const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
        for (size_t i = 0; i < leaving_vs.size(); ++i) {
            int v = leaving_vs[i];

            if (isSpanning_) {
                if (!getDeg(data, v)) { // the degree of v must be at least 1
                    return 0;
                }
            }

            // The degree of v must be 0 or 2.
            //if (getDeg(data, v) != 0 && getDeg(data, v) != 2) {
            //    return 0;
            //}
            bool comp_found = false;
            bool deg_found = false;
            bool frontier_exists = false;
            // Search a vertex that has the component number same as that of v.
            // Also check whether a vertex whose degree is at least 1 exists
            // on the frontier.
            for (size_t j = 0; j < frontier_vs.size(); ++j) {
                int w = frontier_vs[j];
                if (w == v) { // skip if w is the leaving vertex
                    continue;
                }
                // skip if w is one of the vertices that
                // has already leaved the frontier
                bool found_leaved = false;
                for (size_t k = 0; k < i; ++k) {
                    if (w == leaving_vs[k]) {
                        found_leaved = true;
                        break;
                    }
                }
                if (found_leaved) {
                    continue;
                }
                frontier_exists = true;
                // w has the component number same as that of v
                if (getComp(data, w) == getComp(data, v)) {
                    comp_found = true;
                }
                // The degree of w is at least 1.
                if (getDeg(data, w)) {
                    deg_found = true;
                }
                if (deg_found && comp_found) {
                    break;
                }
            }
            // There is no vertex that has the component number
            // same as that of v. That is, the connected component
            // of v becomes determined.
            if (!comp_found) {
                // Here, deg of v is 0 or 2. If deg of v is 0,
                // this means that v is isolated.
                // If deg of v is 2, and there is a vertex whose
                // deg is at least 1, this means that there is a
                // connected component other than that of v.
                // That is, the generated subgraph is not connected.
                // Then, we return the 0-terminal.
                //assert(getDeg(data, v) == 0 || getDeg(data, v) == 2);
                if (getDeg(data, v) && deg_found) {
                    return 0; // return the 0-terminal.
                } else if (getDeg(data, v)) { // If deg of v is 2,
                    // and there is no vertex whose deg is at least 1
                    // a single cycle is completed.
                    // Then, we return the 1-terminal

                    if (isSpanning_) {
                        if (frontier_exists) {
                            return 0;
                        } else {
                            return -1; // return the 1-terminal
                        }
                    } else {
                        return -1; // return the 1-terminal
                    }
                }
            }

            // Since deg and comp of v are never used until the end,
            // we erase the values.
            resetDeg(data, v);
            setComp(data, v, 0);
        }
        if (level == 1) {
            // If we come here, the edge set is empty (taking no edge).
            return 0;
        }
        /*std::cerr << "level = " << level << ", value = " << value << ", ";
        for (size_t i = 0; i < frontier_vs.size(); ++i) {
            int v = frontier_vs[i];
            std::cerr << v << ": " << (getDeg(data, v) ? "1": "0") << "," << getComp(data, v) << "  ";
        }

        std::cerr << std::endl;*/

        assert(level - 1 > 0);
        return level - 1;
    }
};

#endif // FRONTIER_TREE_HPP
