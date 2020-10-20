#ifndef FRONTIER_TWO_SINGLE_CYCLES_HPP
#define FRONTIER_TWO_SINGLE_CYCLES_HPP

#include <vector>
#include <climits>

using namespace tdzdd;

static const int NUMBER_OF_COLORS = 2; // This value can be changed.

class FrontierTwoCyclesCompletedData {
public:
    bool completed[NUMBER_OF_COLORS];
};

// data associated with each vertex on the frontier
class FrontierTwoCyclesDegCompData {
public:
    short deg[NUMBER_OF_COLORS];
    short comp[NUMBER_OF_COLORS];
};

class FrontierTwoSingleCyclesSpec
    : public tdzdd::HybridDdSpec<FrontierTwoSingleCyclesSpec,
                                 FrontierTwoCyclesCompletedData,
                                 FrontierTwoCyclesDegCompData, NUMBER_OF_COLORS + 1> {
private:
    // input graph
    const tdzdd::Graph& graph_;
    // number of vertices
    const short n_;
    // number of edges
    const int m_;

    const FrontierManager fm_;

    // This function gets deg of v.
    short getDeg(FrontierTwoCyclesDegCompData* data, short v, int color) const {
        return data[fm_.vertexToPos(v)].deg[color - 1];
    }

    // This function sets deg of v to be d.
    void setDeg(FrontierTwoCyclesDegCompData* data, short v, short d, int color) const {
        data[fm_.vertexToPos(v)].deg[color - 1] = d;
    }

    // This function gets comp of v.
    short getComp(FrontierTwoCyclesDegCompData* data, short v, int color) const {
        return data[fm_.vertexToPos(v)].comp[color - 1];
    }

    // This function sets comp of v to be c.
    void setComp(FrontierTwoCyclesDegCompData* data, short v, short c, int color) const {
        data[fm_.vertexToPos(v)].comp[color - 1] = c;
    }

    void initializeDegComp(FrontierTwoCyclesDegCompData* data) const {
        for (int i = 0; i < fm_.getMaxFrontierSize(); ++i) {
            for (int j = 0; j < NUMBER_OF_COLORS; ++j) {
                data[i].deg[j] = 0;
                data[i].comp[j] = 0;
            }
        }
    }

public:
    FrontierTwoSingleCyclesSpec(const tdzdd::Graph& graph)
        : graph_(graph),
          n_(static_cast<short>(graph_.vertexSize())),
          m_(graph_.edgeSize()),
          fm_(graph_)
    {
        if (graph_.vertexSize() > SHRT_MAX) { // SHRT_MAX == 32767
            std::cerr << "The number of vertices should be at most "
                      << SHRT_MAX << std::endl;
            exit(1);
        }
        setArraySize(fm_.getMaxFrontierSize());
    }

    int getRoot(FrontierTwoCyclesCompletedData& cdata,
                FrontierTwoCyclesDegCompData* ddata) const {
        for (int i = 0; i < NUMBER_OF_COLORS; ++i) {
            cdata.completed[i] = false;
        }
        initializeDegComp(ddata);
        return m_;
    }

    int getChild(FrontierTwoCyclesCompletedData& cdata,
                 FrontierTwoCyclesDegCompData* ddata, int level, int value) const {
        assert(1 <= level && level <= m_);

        // edge index (starting from 0)
        const int edge_index = m_ - level;
        // edge that we are processing.
        // The endpoints of "edge" are edge.v1 and edge.v2.
        const Graph::EdgeInfo& edge = graph_.edgeInfo(edge_index);

        const int color = value;

        // initialize deg and comp of the vertices newly entering the frontier
        const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
        for (size_t i = 0; i < entering_vs.size(); ++i) {
            int v = entering_vs[i];
            for (int col = 1; col <= NUMBER_OF_COLORS; ++col) {
                // initially the value of deg is 0
                setDeg(ddata, v, 0, col);
                // initially the value of comp is the vertex number itself
                setComp(ddata, v, v, col);
            }
        }

        // vertices on the frontier
        const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

        if (value >= 1) { // if we take the edge (go to c-arc for c >= 1)
            // increment deg of v1 and v2 (recall that edge = {v1, v2})
            setDeg(ddata, edge.v1, getDeg(ddata, edge.v1, color) + 1, color);
            setDeg(ddata, edge.v2, getDeg(ddata, edge.v2, color) + 1, color);

            short c1 = getComp(ddata, edge.v1, color);
            short c2 = getComp(ddata, edge.v2, color);
            if (c1 != c2) { // connected components c1 and c2 become connected
                short cmin = std::min(c1, c2);
                short cmax = std::max(c1, c2);

                // replace component number cmin with cmax
                for (size_t i = 0; i < frontier_vs.size(); ++i) {
                    int v = frontier_vs[i];
                    if (getComp(ddata, v, color) == cmin) {
                        setComp(ddata, v, cmax, color);
                    }
                }
            }
        }

        // vertices that are leaving the frontier
        const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
        for (size_t i = 0; i < leaving_vs.size(); ++i) {
            int v = leaving_vs[i];

            for (int col = 1; col <= NUMBER_OF_COLORS; ++col) {
                // The degree of v must be 0 or 2.
                if (getDeg(ddata, v, col) != 0 && getDeg(ddata, v, col) != 2) {
                    return 0;
                }
            }

            bool samecomp_found[NUMBER_OF_COLORS];
            bool nonisolated_found[NUMBER_OF_COLORS];

            for (int col = 1; col <= NUMBER_OF_COLORS; ++col) {
                samecomp_found[col - 1] = false;
                nonisolated_found[col - 1] = false;
            }

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
                for (int col = 1; col <= NUMBER_OF_COLORS; ++col) {
                    // w has the component number same as that of v
                    if (getComp(ddata, w, col) == getComp(ddata, v, col)) {
                        samecomp_found[col - 1] = true;
                    }
                    // The degree of w is at least 1.
                    if (getDeg(ddata, w, col) > 0) {
                        nonisolated_found[col - 1] = true;
                    }
                    //if (nonisolated_found && samecomp_found) {
                    //    break;
                    //}
                }
            }
            // There is no vertex that has the component number
            // same as that of v. That is, the connected component
            // of v becomes determined.
            for (int col = 1; col <= NUMBER_OF_COLORS; ++col) {
                if (!samecomp_found[col - 1]) {
                    // Here, deg of v is 0 or 2.
                    assert(getDeg(ddata, v, col) == 0 || getDeg(ddata, v, col) == 2);

                    // Check whether v is isolated.
                    // If v is isolated (deg of v is 0), nothing occurs.
                    if (getDeg(ddata, v, col) > 0) {
                        // Check whether there is a
                        // connected component other than that of v,
                        // that is, the generated subgraph is not connected.
                        // If so, we return the 0-terminal.
                        if (nonisolated_found[col - 1]) {
                            return 0; // return the 0-terminal.
                        } else {
                            // Here, a single cycle is completed.
                            cdata.completed[col - 1] = true;
                            bool allCompleted = true;
                            for (int otherCol = 1; otherCol <= NUMBER_OF_COLORS; ++otherCol) {
                                if (cdata.completed[otherCol - 1]) {
                                    allCompleted = false;
                                    break;
                                }
                            }
                            if (allCompleted) {
                                return -1; // return the 1-terminal
                            }
                        }
                    }
                }
            }
            // Since deg and comp of v are never used until the end,
            // we erase the values.
            for (int col = 1; col <= NUMBER_OF_COLORS; ++col) {
                setDeg(ddata, v, -1, col);
                setComp(ddata, v, -1, col);
            }
        }
        if (level == 1) {
            // If we come here, the edge set is empty (taking no edge).
            return 0;
        }
        assert(level - 1 > 0);
        return level - 1;
    }
};

#endif // FRONTIER_TWO_SINGLE_CYCLES_HPP
