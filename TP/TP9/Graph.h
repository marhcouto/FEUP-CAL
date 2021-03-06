/*
 * Graph.h.
 * For implementation of the minimum cost flow algorithm.
 * See TODOs for code to add/adapt.
 * FEUP, CAL, 2017/18.
 */
#ifndef GRAPH_H_
#define GRAPH_H_

#include <vector>
#include <queue>
#include <limits>
#include <iostream>
#include "MutablePriorityQueue.h"

using namespace std;

constexpr auto INF = std::numeric_limits<double>::max();

template <class T> class Vertex;
template <class T> class Edge;
template <class T> class Graph;

/*
 * ================================================================================================
 * Class Vertex
 * ================================================================================================
 */

template <class T>
class Vertex {
	T info;
	vector<Edge<T> *> outgoing;
	vector<Edge<T> *> incoming;

	bool visited;  // for path finding
	Edge<T> *path; // for path finding
	double dist;   // for path finding
	int queueIndex = 0; // required by MutablePriorityQueue

	Vertex(T in);
	void addEdge(Edge<T> *e);
	bool operator<(Vertex<T> & vertex) const; // required by MutablePriorityQueue

public:
	T getInfo() const;
	vector<Edge<T> *> getIncoming() const;
	vector<Edge<T> *> getOutgoing() const;
	friend class Graph<T>;
	friend class MutablePriorityQueue<Vertex<T>>;
};


template <class T>
Vertex<T>::Vertex(T in): info(in) {}

template <class T>
void Vertex<T>::addEdge(Edge<T> *e) {
	outgoing.push_back(e);
	e->dest->incoming.push_back(e);
}

template <class T>
bool Vertex<T>::operator<(Vertex<T> & vertex) const {
	return this->dist < vertex.dist;
}

template <class T>
T Vertex<T>::getInfo() const {
	return this->info;
}

template <class T>
vector<Edge<T> *>  Vertex<T>::getIncoming() const {
	return this->incoming;
}

template <class T>
vector<Edge<T> *>  Vertex<T>::getOutgoing() const {
	return this->outgoing;
}


/* ================================================================================================
 * Class Edge
 * ================================================================================================
 */

template <class T>
class Edge {
	Vertex<T> * orig;
	Vertex<T> * dest;
	double capacity;
	double cost;
	double flow;
	double startingCost; //Made later

	Edge(Vertex<T> *o, Vertex<T> *d, double capacity, double cost=0, double flow=0);

public:
	friend class Graph<T>;
	friend class Vertex<T>;
	double getFlow() const;
};

template <class T>
Edge<T>::Edge(Vertex<T> *o, Vertex<T> *d, double capacity, double cost, double flow):
	orig(o), dest(d), capacity(capacity), cost(cost), flow(flow){}

template <class T>
double  Edge<T>::getFlow() const {
	return this->flow;
}


/* ================================================================================================
 * Class Graph
 * ================================================================================================
 */

template <class T>
class Graph {
	vector<Vertex<T> *> vertexSet;

	void dijkstraShortestPath(Vertex<T> *s);
	void bellmanFordShortestPath(Vertex<T> *s);
	bool relax(Vertex<T> *v, Vertex<T> *w, Edge<T> *e, double residual, double cost);

    bool bfs(Vertex<T>* source, Vertex<T>* dest); //Made later
    void recalculateCosts(); //Made later
    void printThings(); //Made later
	void resetFlows();
	bool findAugmentationPath(Vertex<T> *s, Vertex<T> *t);
	void testAndVisit(queue< Vertex<T>*> &q, Edge<T> *e, Vertex<T> *w, double residual);
	double findMinResidualAlongPath(Vertex<T> *s, Vertex<T> *t);
	void augmentFlowAlongPath(Vertex<T> *s, Vertex<T> *t, double flow);

public:
	Vertex<T>* findVertex(const T &inf) const;
	vector<Vertex<T> *> getVertexSet() const;
	Vertex<T> *addVertex(const T &in);
	Edge<T> *addEdge(const T &sourc, const T &dest, double capacity, double cost, double flow=0);
	double getFlow(const T &sourc, const T &dest) const ;
	void fordFulkerson(T source, T target);
	double minCostFlow(T source, T target, double flow);
};

template <class T>
Vertex<T> * Graph<T>::addVertex(const T &in) {
	Vertex<T> *v = findVertex(in);
	if (v != nullptr)
		return v;
	v = new Vertex<T>(in);
	vertexSet.push_back(v);
	return v;
}

template <class T>
Edge<T> * Graph<T>::addEdge(const T &sourc, const T &dest, double capacity, double cost, double flow) {
	auto s = findVertex(sourc);
	auto d = findVertex(dest);
	if (s == nullptr || d == nullptr)
		return nullptr;
	Edge<T> *e = new Edge<T>(s, d, capacity, cost, flow);
	s->addEdge(e);
	return e;
}

template <class T>
Vertex<T>* Graph<T>::findVertex(const T & inf) const {
	for (auto v : vertexSet)
		if (v->info == inf)
			return v;
	return nullptr;
}

template <class T>
double Graph<T>::getFlow(const T &sourc, const T &dest) const {
	auto s = findVertex(sourc);
	auto d = findVertex(dest);
	if (s == nullptr || d == nullptr)
		return 0.0;
	for (auto e : s->outgoing)
		if (e->dest == d)
			return e->flow;
	return 0.0;
}

template <class T>
vector<Vertex<T> *> Graph<T>::getVertexSet() const {
	return vertexSet;
}

/**************** Maximum Flow Problem  ************/

/**
 * Finds the maximum flow in a graph using the Ford Fulkerson algorithm
 * (with the improvement of Edmonds-Karp).
 * Assumes that the graph forms a flow network from source vertex 's'
 * to sink vertex 't' (distinct vertices).
 * Receives as arguments the source and target vertices (identified by their contents).
 * The result is defined by the "flow" field of each edge.
 */
template <class T>
void Graph<T>::fordFulkerson(T source, T target) {
	// Obtain the source (s) and target (t) vertices
	Vertex<T>* s = findVertex(source);
	Vertex<T>* t = findVertex(target);
	if (s == nullptr || t == nullptr || s == t)
		throw "Invalid source and/or target vertex";

	// Apply algorithm as in slides
	resetFlows();
	while( findAugmentationPath(s, t) ) {
		double f = findMinResidualAlongPath(s, t);
		augmentFlowAlongPath(s, t, f);
	}
}

template <class T>
void Graph<T>::resetFlows() {
	for (auto v : vertexSet)
		for (auto e: v->outgoing)
			e->flow = 0;
}

template<class T>
bool Graph<T>::findAugmentationPath(Vertex<T> *s, Vertex<T> *t) {
	for(auto v : vertexSet)
		v->visited = false;
	s->visited = true;
	queue< Vertex<T>* > q;
	q.push(s);
	while( ! q.empty() && ! t->visited) {
		auto v = q.front();
		q.pop();
		for(auto e: v->outgoing)
			testAndVisit(q, e, e->dest, e->capacity - e->flow);
		for(auto e: v->incoming)
			testAndVisit(q, e, e->orig, e->flow);
	}
	return t->visited;
}

/**
 * Auxiliary function used by findAugmentationPath.
 */
template<class T>
void Graph<T>::testAndVisit(queue< Vertex<T>*> &q, Edge<T> *e, Vertex<T> *w, double residual) {
	// TODO: adapt in order to use only edges with null cost
	if (! w-> visited && residual > 0 && e->cost == 0) {
		w->visited = true;
		w->path = e;
		q.push(w);
	}
}

template<class T>
double Graph<T>::findMinResidualAlongPath(Vertex<T> *s, Vertex<T> *t) {
	double f = INF;
	for (auto v = t; v != s; ) {
		auto e = v->path;
		if (e->dest == v) {
			f = min(f, e->capacity - e->flow);
			v = e->orig;
		}
		else {
			f = min(f, e->flow);
			v = e->dest;
		}
	}
	return f;
}

template<class T>
void Graph<T>::augmentFlowAlongPath(Vertex<T> *s, Vertex<T> *t, double f) {
	for (auto v = t; v != s; ) {
		auto e = v->path;
		if (e->dest == v) {
			e->flow += f;
			v = e->orig;
		}
		else {
			e->flow -= f;
			v = e->dest;
		}
	}
}


/**************** Minimum Cost Flow Problem  ************/


/**
 * Computes the shortest distance (with minimum cost) from "s" to all other vertices
 * in the residuals graph, using only edges with non-null residuals,
 * based on the Dijkstra algorithm.
 * The result is indicated by the field "dist" of each vertex.
 */
template<class T>
void Graph<T>::dijkstraShortestPath(Vertex<T> *s ) {
    for(auto v : vertexSet)
        v->dist = INF;
    s->dist = 0;
    MutablePriorityQueue<Vertex<T>> q;
    q.insert(s);
    while( ! q.empty() ) {
        auto v = q.extractMin();
        for (auto e : v->outgoing) {
            auto oldDist = e->dest->dist;
            if (relax(v, e->dest, e, e->capacity - e->flow, e->cost)){
                if (oldDist==INF)
                    q.insert(e->dest);
                else
                    q.decreaseKey(e->dest);
            }
        }
        for (auto e : v->incoming) {
            auto oldDist = e->orig->dist;
            if (relax(v, e->orig, e, e->flow, -e->cost)) {
                if (oldDist == INF)
                    q.insert(e->orig);
                else
                    q.decreaseKey(e->orig);
            }
        }
    }
}

/**
 * Computes the shortest distance (with minimum cost) from "s" to all other vertices
 * in the residuals graph, using only edges with non-null residuals,
 * based on the Bellman-Ford algorithm.
 * The result is indicated by the field "dist" of each vertex.
 */
template<class T>
void Graph<T>::bellmanFordShortestPath(Vertex<T> *s ) {
	for(auto v : vertexSet)
		v->dist = INF;
	s->dist = 0;
	for (unsigned i = 1; i < vertexSet.size(); i++)
		for (auto v: vertexSet) {
			for (auto e : v->outgoing)
				relax(v, e->dest, e, e->capacity - e->flow, e->cost);
			for (auto e : v->incoming)
				relax(v, e->orig, e, e->flow, -e->cost);
		}
}

/**
 * Auxiliary function used by Dijkstra and Bellman-Ford algorithms.
 * Analyzes edge (v, w) with a given residual and cost.
 */

template<class T>
bool Graph<T>::relax(Vertex<T> *v, Vertex<T> *w, Edge<T> *e, double residual, double cost) {
	if (residual > 0 && v->dist + cost < w->dist) {
		w->dist = v->dist + cost;
		w->path = e;
		return true;
	}
	else
		return false;
}

template <class T>
void Graph<T>::printThings() {
    for (auto v : vertexSet) {
        for (auto e: v->outgoing) {
            std::cout << e->orig->info << " -> " << e->dest->info << " cost:" << e->cost << " flow:" << e->flow <<
            " distO:" << e->orig->dist << " distD:" << e->dest->dist << std::endl;
        }
        for (auto e: v->incoming) {
            std::cout << e->dest->info << " -> " << e->orig->info << " cost:" << e->cost << " flow:" << e->flow <<
                      " distO:" << e->dest->dist << " distD:" << e->orig->dist << std::endl;
        }
    }
}

template <class T>
void Graph<T>::recalculateCosts() {
    for (auto v : vertexSet) {
        for (auto e: v->outgoing) {
            if (e->capacity - e->flow > 0)
                e->cost -= (e->dest->dist - e->orig->dist);
        }
        for (auto e: v->incoming) {
            if (e->flow > 0)
                e->cost -= (e->orig->dist - e->dest->dist);
        }
    }
}

template <class T>
bool Graph<T>::bfs(Vertex<T>* source, Vertex<T>* dest) {

    for (auto& v : vertexSet)
        v->visited = false;

    bool found = false;
    std::queue<Vertex<T>*> q;

    q.push(source);
    while (!q.empty() && !found) {
        for (auto& e : q.front()->outgoing) {
            if (e->capacity - e->flow <= 0 || e->dest->visited) continue;
            if (e->dest->info == dest->info) {
                return true;
            }
            e->dest->visited = true;
            q.push(e->dest);
        }
        q.pop();
    }
    return false;
}

/**
 * Determines the minimum cost flow in a flow network.
 * Receives as arguments the source and sink vertices (identified by their info),
 * and the intended flow.
 * Returns the calculated minimum cost for delivering the intended flow (or the highest
 * possible flow, if the intended flow is higher than supported by the network).
 * The calculated flow in each edge can be consulted with the "getFlow" function.
 * Notice: Currently, the costs of the edges are modified by the algorithm.
 */
template <class T>
double Graph<T>::minCostFlow(T source, T sink, double flow) {

    Vertex<T>* sourceV = findVertex(source);
    Vertex<T>* sinkV = findVertex(sink);

    if (sourceV == NULL || sinkV == NULL)
        throw "Invalid source and/or target vertex";

    for (auto v : vertexSet) {
        for (auto e: v->outgoing)
            e->startingCost = e->cost;
    }

    double currentFlow = 0.0; // To check when the desired flow has been attained
    double minCost = 0.0;

    resetFlows();

    bool firstIteration = true;
    while(currentFlow < flow) {

        if (!bfs(sourceV, sinkV)) break; // Check if there are shortest paths from source to sink

        if (firstIteration) {
            bellmanFordShortestPath(sourceV); // Only needed for first iteration, after that, no edges with negative weight will show up
            firstIteration = false;
        }
        else
            dijkstraShortestPath(sourceV);

        recalculateCosts(); // Recalculate costs given the new distances of the nodes

        while(findAugmentationPath(sourceV, sinkV)) { // Check for an Augmentation Path
            //Apply it
            double f = findMinResidualAlongPath(sourceV, sinkV);
            augmentFlowAlongPath(sourceV, sinkV, f);
            currentFlow += f;
        }
    }

    for (auto v : vertexSet) {
        for (auto e: v->outgoing)
            minCost += e->flow * e->startingCost; // Calculate minCostFlow
    }


    return minCost;
}



#endif /* GRAPH_H_ */
