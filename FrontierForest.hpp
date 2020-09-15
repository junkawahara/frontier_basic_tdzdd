#ifndef FRONTIER_FOREST_HPP
#define FRONTIER_FOREST_HPP

#include <vector>

using namespace tdzdd;

typedef unsigned short ushort;

typedef unsigned short FrontierForestData;

class FrontierForestSpec
    : public tdzdd::PodArrayDdSpec<FrontierForestSpec, FrontierForestData, 2> {
private:
    // input graph
    const tdzdd::Graph& graph_;
    // number of vertices
    const int n_;
    // number of edges
    const int m_;

    const FrontierManager fm_;

    // This function gets comp of v.
    ushort getComp(FrontierForestData* data, int v) const {
        return data[fm_.vertexToPos(v)];
    }

    // This function sets comp of v to be c.
    void setComp(FrontierForestData* data, int v, ushort c) const {
        data[fm_.vertexToPos(v)] = c;
    }

    void initializeData(FrontierForestData* data) const {
        for (int i = 0; i < fm_.getMaxFrontierSize(); ++i) {
            data[i] = 0;
        }
    }

public:
    FrontierForestSpec(const tdzdd::Graph& graph)
        : graph_(graph),
          n_(graph_.vertexSize()),
          m_(graph_.edgeSize()),
          fm_(graph_)
    {
        if (n_ >= (1 << 16)) {
            std::cerr << "The number of vertices must be smaller than 2^15."
                      << std::endl;
            exit(1);
        }
        setArraySize(fm_.getMaxFrontierSize());
    }

    int getRoot(FrontierForestData* data) const {
        initializeData(data);
        return m_;
    }

    int getChild(FrontierForestData* data, int level, int value) const {
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

            // Since deg and comp of v are never used until the end,
            // we erase the values.
            setComp(data, v, 0);
        }
        if (level == 1) {
            return -1;
        }
        assert(level - 1 > 0);
        return level - 1;
    }
};

#endif // FRONTIER_FOREST_HPP
