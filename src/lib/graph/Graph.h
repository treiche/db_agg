/*
 * Graph.h
 *
 *  Created on: Dec 29, 2014
 *      Author: arnd
 */

#ifndef GRAPH_H_
#define GRAPH_H_

#include <list>
#include <map>
#include <iostream>
#include <fstream>

namespace db_agg {

template <typename C, typename N,typename E, typename P>
class Graph {
protected:
    std::list<N> nodes;
    std::map<N,std::list<P>> node2ports;
    std::list<E> edges;
    std::list<C> clusters;
    std::map<C,std::list<N>> cluster2nodes;

    C getCluster(N node) {
        for (auto c2n:cluster2nodes) {
            for (auto n:c2n.second) {
                if (n == node) {
                    return c2n.first;
                }
            }
        }
        return nullptr;
    }

public:
    bool hasCluster(C c) {
        for (auto c2n:cluster2nodes) {
            if (c2n.first == c) {
                return true;
            }
        }
        return false;
    }

    void addCluster(C cluster) {
        clusters.push_back(cluster);
    }

    bool hasNode(N n) {
        for (auto node:nodes) {
            if (node == n) {
                return true;
            }
        }
        return false;
    }

    void addNode(N node) {
        nodes.push_back(node);
    }

    void addNode(C cluster, N node) {
        cluster2nodes[cluster].push_back(node);
        if (!hasCluster(cluster)) {
            clusters.push_back(cluster);
        }
        nodes.push_back(node);
    }

    void addPort(N node, P port) {
        node2ports[node].push_back(port);
    }

    void addEdge(E edge) {
        edges.push_back(edge);
    }

    void dumpGraphML(std::string outputDir) {
        std::ofstream out(outputDir+"/executionPlan.xml");
        toGraphML(out);
        out.close();
    }

    void toGraphML(std::ostream& out) {
        using namespace std;
        string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<graphml xmlns="http://graphml.graphdrawing.org/xmlns"  
      xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
      xsi:schemaLocation="http://graphml.graphdrawing.org/xmlns 
        http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd">

        )";
        out << xml;
        out << "<graph id='G'>";

        out << endl << "<!-- clusters -->" << endl;
        for (auto cluster:clusters) {
            out << "<node id='" << cluster->getId() << "'>";
            out << "<graph id='"  << cluster->getId() << ":'>";
            for (auto node:cluster2nodes[cluster]) {
                out << "<node id='" << node->getId() << "'>";
                for (auto n2p:node2ports[node]) {
                    out << "<port name='" << n2p->getId() << "'/>"<< endl;
                }
                out << "</node>";
            }
            out << "</graph>";
            out << "</node>";
        }

        out << endl <<  "<!-- nodes -->" << endl;
        for (auto node:nodes) {
            if (!getCluster(node)) {
                out << "<node id='" << node->getId() << "'>";
                for (auto n2p:node2ports[node]) {
                    out << "<port name='" << n2p->getId() << "'/>"<< endl;
                }
                out << "</node>";
            }
        }
        out << endl << "<!-- edges -->" << endl;
        for (auto edge:edges) {
            out << "<edge source='" << edge->getSource()->getId() << "' target='" << edge->getTarget()->getId() << "'/>";
        }
        out << "</graph></graphml>";
    }
};


}



#endif /* GRAPH_H_ */
