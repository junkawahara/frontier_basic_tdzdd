#include <vector>

using namespace tdzdd;

// This class manages vertex numbers on the frontier
// and where deg/comp of each vertex is stored.
class FrontierManager {
private:
    // input graph
    const tdzdd::Graph& graph_;

    // frontier_vss_[i] stores the vertices each of
    // which is incident to both at least one of e_0, e_1,...,e_{i-1}
    // and at least one of e_{i+1},e_{i+2},...,e_{m-1}, and also stores
    // both endpoints of e_i, where m is the number of edges.
    // Note that the definition of the frontier is different from
    // that in the paper [Kawahara+ 2017].
    // "vss" stands for "vertex set set".
    std::vector<std::vector<int> > frontier_vss_;

    // entering_vss_[i] stores the vertex numbers
    // that newly enter the frontier when processing the i-th edge.
    std::vector<std::vector<int> > entering_vss_;

    // leaving_vss_[i] stores the vertex numbers
    // that leave the frontier after the i-th edge is processed.
    std::vector<std::vector<int> > leaving_vss_;

    // translate the vertex number to the position in the PodArray
    std::vector<int> vertex_to_pos_;

    // the maximum frontier size
    int max_frontier_size_;

    void constructEnteringAndLeavingVss() {
        const int n = graph_.vertexSize();
        const int m = graph_.edgeSize();

        entering_vss_.resize(m);
        leaving_vss_.resize(m);

        // compute entering_vss_
        std::set<int> entered_vs;
        for (int i = 0; i < m; ++i) {
            const tdzdd::Graph::EdgeInfo& e = graph_.edgeInfo(i);
            if (entered_vs.count(e.v1) == 0) {
                entering_vss_[i].push_back(e.v1);
                entered_vs.insert(e.v1);
            }
            if (entered_vs.count(e.v2) == 0) {
                entering_vss_[i].push_back(e.v2);
                entered_vs.insert(e.v2);
            }
        }
        assert(entered_vs.size() == n);

        // compute leaving_vss_
        std::set<int> leaved_vs;
        for (int i = m - 1; i >= 0; --i) {
            const tdzdd::Graph::EdgeInfo& e = graph_.edgeInfo(i);
            if (leaved_vs.count(e.v1) == 0) {
                leaving_vss_[i].push_back(e.v1);
                leaved_vs.insert(e.v1);
            }
            if (leaved_vs.count(e.v2) == 0) {
                leaving_vss_[i].push_back(e.v2);
                leaved_vs.insert(e.v2);
            }
        }
        assert(leaved_vs.size() == n);
    }

    void construct() {
        const int n = graph_.vertexSize();
        const int m = graph_.edgeSize();
        max_frontier_size_ = 0;

        constructEnteringAndLeavingVss();

        std::vector<int> unused;
        for (int i = n - 1; i >= 0; --i) {
            unused.push_back(i);
        }

        vertex_to_pos_.resize(n + 1);
        std::set<int> current_vs;
        for (int i = 0; i < m; ++i) {
            const std::vector<int>& entering_vs = entering_vss_[i];
            for (size_t j = 0; j < entering_vs.size(); ++j) {
                int v = entering_vs[j];
                current_vs.insert(v);
                int u = unused.back();
                unused.pop_back();
                vertex_to_pos_[v] = u;
            }

            if (current_vs.size() > max_frontier_size_) {
                max_frontier_size_ = current_vs.size();
            }

            frontier_vss_.push_back(std::vector<int>());
            std::vector<int>& vs = frontier_vss_.back();
            for (std::set<int>::const_iterator itor = current_vs.begin();
                 itor != current_vs.end(); ++itor) {

                vs.push_back(*itor);
            }

            const std::vector<int>& leaving_vs = leaving_vss_[i];
            for (size_t j = 0; j < leaving_vs.size(); ++j) {
                int v = leaving_vs[j];
                current_vs.erase(v);
                unused.push_back(vertex_to_pos_[v]);
            }
        }
    }

    void print() {
        for (int i = 0; i < graph_.edgeSize(); ++i) {
            std::cout << "[";
            for (size_t j = 0; j < entering_vss_[i].size(); ++j) {
                std::cout << entering_vss_[i][j] << ", ";
            }
            std::cout << "]";
            std::cout << "[";
            for (size_t j = 0; j < leaving_vss_[i].size(); ++j) {
                std::cout << leaving_vss_[i][j] << ", ";
            }
            std::cout << "]";
            std::cout << "[";
            for (size_t j = 0; j < frontier_vss_[i].size(); ++j) {
                std::cout << frontier_vss_[i][j] << ", ";
            }
            std::cout << "]" << std::endl;
        }

        for (int v = 1; v <= graph_.vertexSize(); ++v) {
            std::cout << vertex_to_pos_[v] << ", ";
        }
        
        std::cout << "max f size = " << max_frontier_size_ << std::endl;
    }

public:
    FrontierManager(const tdzdd::Graph& graph) : graph_(graph) {
        construct();
    }

    // This function returns the maximum frontier size.
    int getMaxFrontierSize() const {
        return max_frontier_size_;
    }

    // This function returns the vector that stores the vertex numbers
    // that newly enter the frontier when processing the (index)-th edge.
    const std::vector<int>& getEnteringVs(int index) const {
        return entering_vss_[index];
    }

    // This function returns the vector that stores the vertex numbers
    // that leave the frontier after the (index)-th edge is processed.
    const std::vector<int>& getLeavingVs(int index) const {
        return leaving_vss_[index];
    }

    // This function returns the vector that stores the vertex numbers
    // that leave the frontier after the (index)-th edge is processed.
    const std::vector<int>& getFrontierVs(int index) const {
        return frontier_vss_[index];
    }

    // This function translates the vertex number to the position
    // in the PodArray used by FrontierExampleSpec.
    int vertexToPos(int v) const {
        return vertex_to_pos_[v];
    }
};

// data associated with each vertex on the frontier
class FrontierData {
public:
    short deg;
    short comp;
};

class FrontierExampleSpec
    : public tdzdd::PodArrayDdSpec<FrontierExampleSpec, FrontierData, 2> {
private:
    // input graph
    const tdzdd::Graph& graph_;
    // number of vertices
    const int n_;
    // number of edges
    const int m_;

    const FrontierManager fm_;

    // This function gets deg of v.
    short getDeg(FrontierData* data, int v) const {
        return data[fm_.vertexToPos(v)].deg;
    }

    // This function sets deg of v to be d.
    void setDeg(FrontierData* data, int v, short d) const {
        data[fm_.vertexToPos(v)].deg = d;
    }

    // This function gets comp of v.
    short getComp(FrontierData* data, int v) const {
        return data[fm_.vertexToPos(v)].comp;
    }

    // This function sets comp of v to be c.
    void setComp(FrontierData* data, int v, short c) const {
        data[fm_.vertexToPos(v)].comp = c;
    }

    void initializeDegComp(FrontierData* data) const {
        for (int i = 0; i < fm_.getMaxFrontierSize(); ++i) {
            data[i].deg = 0;
            data[i].comp = 0;
        }
    }

public:
    FrontierExampleSpec(const tdzdd::Graph& graph) : graph_(graph),
                                                     n_(graph_.vertexSize()),
                                                     m_(graph_.edgeSize()),
                                                     fm_(graph_)
    {
        setArraySize(fm_.getMaxFrontierSize());
    }

    int getRoot(FrontierData* data) const {
        initializeDegComp(data);
        return m_;
    }

    int getChild(FrontierData* data, int level, int value) const {
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

            int c1 = getComp(data, edge.v1);
            int c2 = getComp(data, edge.v2);
            if (c1 != c2) { // connected components c1 and c2 become connected
                int cmin = std::min(c1, c2);
                int cmax = std::max(c1, c2);

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

            // The degree of v must be 0 or 2.
            if (getDeg(data, v) != 0 && getDeg(data, v) != 2) {
                return 0;
            }
            bool comp_found = false;
            bool deg_found = false;
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
                for (size_t k = 0; k < i; ++k) {
                    if (w == leaving_vs[k]) {
                        continue;
                    }
                }
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
                // Here, deg of v is 0 or 2. If deg of v is 0,
                // this means that v is isolated.
                // If deg of v is 2, and there is a vertex whose
                // deg is at least 1, this means that there is a
                // connected component other than that of v.
                // That is, the generated subgraph is not connected.
                // Then, we return the 0-terminal.
                assert(getDeg(data, v) == 0 || getDeg(data, v) == 2);
                if (getDeg(data, v) > 0 && deg_found) {
                    return 0; // return the 0-terminal.
                } else if (getDeg(data, v) > 0) { // If deg of v is 2,
                    // and there is no vertex whose deg is at least 1
                    // a single cycle is completed.
                    // Then, we return the 1-terminal
                    return -1; // return the 1-terminal
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
