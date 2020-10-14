#ifndef FRONTIER_MATE_HPP
#define FRONTIER_MATE_HPP

#include <vector>
#include <climits>

using namespace tdzdd;

// data associated with each vertex on the frontier
typedef short FrontierMate;

class FrontierMateSpec
    : public tdzdd::PodArrayDdSpec<FrontierMateSpec, FrontierMate, 2> {
private:
    // input graph
    const tdzdd::Graph& graph_;
    // number of vertices
    const short n_;
    // number of edges
    const int m_;

    const bool isCycle_;
    const bool isHamiltonian_;

    // endpoints of a path
    const short s_;
    const short t_;

    const FrontierManager fm_;

    const int s_entered_level_;
    const int t_entered_level_;
    const int all_entered_level_;

    short getMate(FrontierMate* data, short v) const {
        return data[fm_.vertexToPos(v)];
    }

    void setMate(FrontierMate* data, short v, short d) const {
        data[fm_.vertexToPos(v)] = d;
    }

    void initializeMate(FrontierMate* data) const {
        for (int i = 0; i < fm_.getMaxFrontierSize(); ++i) {
            data[i] = 0;
        }
    }

    int computeEnteredLevel(short v) const {
        if (isCycle_) {
            return -1; // This value is never used.
        } else {
            return m_ - fm_.getVerticesEnteringLevel(v);
        }
    }

public:
    // for cycles
    FrontierMateSpec(const tdzdd::Graph& graph,
                     bool isHamiltonian)
        : graph_(graph),
          n_(static_cast<short>(graph_.vertexSize())),
          m_(graph_.edgeSize()),
          isCycle_(true),
          isHamiltonian_(isHamiltonian),
          s_(-1),
          t_(-1),
          fm_(graph_),
          s_entered_level_(-1),
          t_entered_level_(-1),
          all_entered_level_(m_ - fm_.getAllVerticesEnteringLevel())
    {
        if (graph_.vertexSize() > SHRT_MAX) { // SHRT_MAX == 32767
            std::cerr << "The number of vertices should be at most "
                      << SHRT_MAX << std::endl;
            exit(1);
        }
        setArraySize(fm_.getMaxFrontierSize());
    }

    // for paths
    FrontierMateSpec(const tdzdd::Graph& graph,
                     bool isHamiltonian, int s, int t)
        : graph_(graph),
          n_(static_cast<short>(graph_.vertexSize())),
          m_(graph_.edgeSize()),
          isCycle_(false),
          isHamiltonian_(isHamiltonian),
          s_(s),
          t_(t),
          fm_(graph_),
          s_entered_level_(computeEnteredLevel(s)),
          t_entered_level_(computeEnteredLevel(t)),
          all_entered_level_(m_ - fm_.getAllVerticesEnteringLevel())
    {
        if (graph_.vertexSize() > SHRT_MAX) { // SHRT_MAX == 32767
            std::cerr << "The number of vertices should be at most "
                      << SHRT_MAX << std::endl;
            exit(1);
        }
        setArraySize(fm_.getMaxFrontierSize());
    }

    int getRoot(FrontierMate* data) const {
        initializeMate(data);
        return m_;
    }

    int getChild(FrontierMate* data, int level, int value) const {
        assert(1 <= level && level <= m_);

        // edge index (starting from 0)
        int edge_index = m_ - level;
        // edge that we are processing.
        // The endpoints of "edge" are edge.v1 and edge.v2.
        const Graph::EdgeInfo& edge = graph_.edgeInfo(edge_index);
        // vertices on the frontier
        const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

        // initialize deg and comp of the vertices newly entering the frontier
        const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
        for (size_t i = 0; i < entering_vs.size(); ++i) {
            int v = entering_vs[i];
            setMate(data, v, v);
            if (!isCycle_) {
                if (v == s_) {
                    setMate(data, v, t_);
                    for (size_t j = 0; j < frontier_vs.size(); ++j) {
                        int w = frontier_vs[j];
                        if (v != w && getMate(data, w) == s_) {
                            setMate(data, v, w);
                        }
                    }
                } else if (v == t_) {
                    setMate(data, v, s_);
                    for (size_t j = 0; j < frontier_vs.size(); ++j) {
                        int w = frontier_vs[j];
                        if (v != w && getMate(data, w) == t_) {
                            setMate(data, v, w);
                        }
                    }
                }
            }
        }

        if (value == 1) { // if we take the edge (go to 1-arc)
            if (getMate(data, edge.v1) == 0 || getMate(data, edge.v2) == 0) {
                return 0;
            } else if (getMate(data, edge.v1) == edge.v2) {
                if (!isCycle_ && level > s_entered_level_
                    && level > t_entered_level_) {
                    return 0;
                }
                for (size_t i = 0; i < frontier_vs.size(); ++i) {
                    int v = frontier_vs[i];
                    if (v != edge.v1 && v != edge.v2) {
                        if (isHamiltonian_) {
                            if (getMate(data, v) != 0) {
                                return 0;
                            }
                        } else {
                            if (getMate(data, v) != 0 && getMate(data, v) != v) {
                                return 0;
                            }
                        }
                    }
                }
                if (isHamiltonian_) {
                    if (level > all_entered_level_) {
                        return 0;
                    }
                }
                return -1; // return the 1-terminal
            }

            short sm = getMate(data, edge.v1);
            short dm = getMate(data, edge.v2);

            setMate(data, edge.v1, 0);
            setMate(data, edge.v2, 0);
            if ((sm != s_ || level <= s_entered_level_) &&
                (sm != t_ || level <= t_entered_level_)) {
                setMate(data, sm, dm);
            }
            if ((dm != s_ || level <= s_entered_level_) &&
                (dm != t_ || level <= t_entered_level_)) {
                setMate(data, dm, sm);
            }
        }

        // vertices that are leaving the frontier
        const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
        for (size_t i = 0; i < leaving_vs.size(); ++i) {
            int v = leaving_vs[i];

            if (isHamiltonian_) {
                // The degree of v (!= s, t) must be 2.
                if (getMate(data, v) != 0) {
                    return 0;
                }
            } else {
                // The degree of v (!= s, t) must be 0 or 2.
                if (getMate(data, v) != 0 && getMate(data, v) != v) {
                    return 0;
                }
            }
            // Since deg and comp of v are never used until the end,
            // we erase the values.
            setMate(data, v, -1);
        }
        if (level == 1) {
            // If we come here, the edge set is empty (taking no edge).
            return 0;
        }
        assert(level - 1 > 0);
        return level - 1;
    }
};

#endif // FRONTIER_MATE_HPP
