#ifndef FRONTIER_ST_PATH_HPP
#define FRONTIER_ST_PATH_HPP

#include <vector>
#include <climits>

using namespace tdzdd;

// data associated with each vertex on the frontier
class FrontierData2 {
public:
    short deg;
    short comp;
};

class FrontierSTPathSpec
    : public tdzdd::PodArrayDdSpec<FrontierSTPathSpec, FrontierData2, 2> {
private:
    // input graph
    const tdzdd::Graph& graph_;
    // number of vertices
    const short n_;
    // number of edges
    const int m_;

    const bool isHamiltonian_;

    // endpoints of a path
    const short s_;
    const short t_;

    const FrontierManager fm_;

    const int s_entered_level_;
    const int t_entered_level_;
    const int all_v_entered_level_;

    // This function gets deg of v.
    short getDeg(FrontierData2* data, short v) const {
        return data[fm_.vertexToPos(v)].deg;
    }

    // This function sets deg of v to be d.
    void setDeg(FrontierData2* data, short v, short d) const {
        data[fm_.vertexToPos(v)].deg = d;
    }

    // This function gets comp of v.
    short getComp(FrontierData2* data, short v) const {
        return data[fm_.vertexToPos(v)].comp;
    }

    // This function sets comp of v to be c.
    void setComp(FrontierData2* data, short v, short c) const {
        data[fm_.vertexToPos(v)].comp = c;
    }

    void initializeDegComp(FrontierData2* data) const {
        for (int i = 0; i < fm_.getMaxFrontierSize(); ++i) {
            data[i].deg = 0;
            data[i].comp = 0;
        }
    }

    int computeEnteredLevel(short v) const {
        return m_ - fm_.getVerticesEnteringLevel(v);
    }

    int computeAllVEnteredLevel() const {
        int min_level = m_;
        for (short v = 1; v <= n_; ++v) {
            int level = computeEnteredLevel(v);
            if (level < min_level) {
                min_level = level;
            }
        }
        return min_level;
    }

public:
    FrontierSTPathSpec(const tdzdd::Graph& graph,
                       bool isHamiltonian, short s, short t)
        : graph_(graph),
          n_(static_cast<short>(graph_.vertexSize())),
          m_(graph_.edgeSize()),
          isHamiltonian_(isHamiltonian),
          s_(s),
          t_(t),
          fm_(graph_),
          s_entered_level_(computeEnteredLevel(s)),
          t_entered_level_(computeEnteredLevel(t)),
          all_v_entered_level_(computeAllVEnteredLevel())
    {
        if (graph_.vertexSize() > SHRT_MAX) { // SHRT_MAX == 32767
            std::cerr << "The number of vertices should be at most "
                      << SHRT_MAX << std::endl;
            exit(1);
        }
        setArraySize(fm_.getMaxFrontierSize());
    }

    int getRoot(FrontierData2* data) const {
        initializeDegComp(data);
        return m_;
    }

    int getChild(FrontierData2* data, int level, int value) const {
        assert(1 <= level && level <= m_);

        // edge index (starting from 0)
        const int edge_index = m_ - level;
        // edge that we are processing.
        // The endpoints of "edge" are edge.v1 and edge.v2.
        const Graph::EdgeInfo& edge = graph_.edgeInfo(edge_index);

        // initialize deg and comp of the vertices newly entering the frontier
        const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
        for (size_t i = 0; i < entering_vs.size(); ++i) {
            int v = entering_vs[i];
            // initially the value of deg is 0
            setDeg(data, v, 0);
            // initially the value of comp is the vertex number itself
            setComp(data, v, v);
        }

        // vertices on the frontier
        const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

        if (value == 1) { // if we take the edge (go to 1-arc)
            // increment deg of v1 and v2 (recall that edge = {v1, v2})
            setDeg(data, edge.v1, getDeg(data, edge.v1) + 1);
            setDeg(data, edge.v2, getDeg(data, edge.v2) + 1);

            short c1 = getComp(data, edge.v1);
            short c2 = getComp(data, edge.v2);
            if (c1 != c2) { // connected components c1 and c2 become connected
                short cmin = std::min(c1, c2);
                short cmax = std::max(c1, c2);

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

            if (v == s_ || v == t_) {
                // The degree of s and t must be 1.
                if (getDeg(data, v) != 1) {
                    return 0;
                }
            } else {
                if (isHamiltonian_) {
                    // The degree of v (!= s, t) must be 2.
                    if (getDeg(data, v) != 2) {
                        return 0;
                    }
                } else {
                    // The degree of v (!= s, t) must be 0 or 2.
                    if (getDeg(data, v) != 0 && getDeg(data, v) != 2) {
                        return 0;
                    }
                }
            }
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
                // has already left the frontier
                bool found_left = false;
                for (size_t k = 0; k < i; ++k) {
                    if (w == leaving_vs[k]) {
                        found_left = true;
                        break;
                    }
                }
                if (found_left) {
                    continue;
                }
                frontier_exists = true;
                // w has the component number same as that of v
                if (getComp(data, w) == getComp(data, v)) {
                    comp_found = true;
                }
                // The degree of w is at least 1.
                if (getDeg(data, w) > 0) {
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
                // If deg of v is 0, this means that v is isolated.
                // If deg of v is at least 1, and there is a vertex whose
                // deg is at least 1, this means that there is a
                // connected component other than that of v.
                // That is, the generated subgraph is not connected.
                // Then, we return the 0-terminal.
                assert(getDeg(data, v) <= 2);
                if (getDeg(data, v) > 0 && deg_found) {
                    return 0; // return the 0-terminal.
                } else if (getDeg(data, v) > 0) { // If deg of v is 2,
                    // and there is no vertex whose deg is at least 1,
                    // a single cycle is completed.
                    // Then, we return the 1-terminal.
                    if (isHamiltonian_) {
                        if (frontier_exists
                            || level > all_v_entered_level_) {
                            return 0;
                        } else {
                            return -1; // return the 1-terminal
                        }
                    } else {
                        if (level > s_entered_level_
                            || level > t_entered_level_) {
                            return 0;
                        } else {
                            return -1; // return the 1-terminal
                        }
                    }
                }
            }
            // Since deg and comp of v are never used until the end,
            // we erase the values.
            setDeg(data, v, -1);
            setComp(data, v, -1);
        }
        if (level == 1) {
            // If we come here, the edge set is empty (taking no edge).
            return 0;
        }
        assert(level - 1 > 0);
        return level - 1;
    }
};

#endif // FRONTIER_ST_PATH_HPP
