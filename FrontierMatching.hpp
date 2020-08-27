#ifndef FRONTIER_MATCHING_HPP
#define FRONTIER_MATCHING_HPP

#include <vector>

using namespace tdzdd;

typedef unsigned char FrontierMatchingData;

class FrontierMatchingSpec
    : public tdzdd::PodArrayDdSpec<FrontierMatchingSpec, FrontierMatchingData, 2> {
private:
    // input graph
    const tdzdd::Graph& graph_;
    // number of vertices
    const int n_;
    // number of edges
    const int m_;

    const int dataSize_;

    const bool isComplete_;

    const FrontierManager fm_;

    bool getUsed(FrontierMatchingData* data, int v) const {
        return ((data[(v - 1) / 8] >> ((v - 1) % 8)) & 1u) != 0;
    }

    void setUsed(FrontierMatchingData* data, int v) const {
        data[(v - 1) / 8] |= (1u << ((v - 1) % 8));
    }

    void resetUsed(FrontierMatchingData* data, int v) const {
        data[(v - 1) / 8] &= ~(1u << ((v - 1) % 8));
    }

    void initializeData(FrontierMatchingData* data) const {
        for (int i = 0; i < dataSize_; ++i) {
            data[i] = 0;
        }
    }

public:
    FrontierMatchingSpec(const tdzdd::Graph& graph,
                         bool isComplete) : graph_(graph),
                                            n_(graph_.vertexSize()),
                                            m_(graph_.edgeSize()),
                                            dataSize_((n_ - 1) / 8 + 1),
                                            isComplete_(isComplete),
                                            fm_(graph_)
    {
        setArraySize(dataSize_);
    }

    int getRoot(FrontierMatchingData* data) const {
        initializeData(data);
        return m_;
    }

    int getChild(FrontierMatchingData* data, int level, int value) const {
        assert(1 <= level && level <= m_);

        // edge index (starting from 0)
        int edge_index = m_ - level;
        // edge that we are processing.
        // The endpoints of "edge" are edge.v1 and edge.v2.
        const Graph::EdgeInfo& edge = graph_.edgeInfo(edge_index);

        // initialize data of the vertices newly entering the frontier
        const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
        for (size_t i = 0; i < entering_vs.size(); ++i) {
            int v = entering_vs[i];
            resetUsed(data, v);
        }

        // vertices on the frontier
        const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

        if (value == 1) { // if we take the edge (go to 1-arc)
            if (getUsed(data, edge.v1)) {
                return 0;
            }
            if (getUsed(data, edge.v2)) {
                return 0;
            }
            // increment deg of v1 and v2 (recall that edge = {v1, v2})
            setUsed(data, edge.v1);
            setUsed(data, edge.v2);
        }

        // vertices that are leaving the frontier
        const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
        for (size_t i = 0; i < leaving_vs.size(); ++i) {
            int v = leaving_vs[i];

            if (isComplete_) {
                if (!getUsed(data, v)) {
                    return 0;
                }
            }
            // Since deg and comp of v are never used until the end,
            // we erase the values.
            resetUsed(data, v);
        }
        if (level == 1) {
            // If we come here, the edge set is empty (taking no edge).
            return -1;
        }
        assert(level - 1 > 0);
        return level - 1;
    }
};

#endif // FRONTIER_MATCHING_HPP
