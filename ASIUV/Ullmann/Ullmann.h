#ifndef ULLMANN
#define ULLMANN

#include"TraversalAlgorithm.h"
#include<vector>
#include<string>
#include<map>


class Ullmann {

private:

	std::string dataGraphFileName;
	std::string queryGraphFileName;

	std::map<int, vector<int>* > candidates;

	std::map<int,int> embedding; /// mapping M: V(q) -> V(g)
	std::map<int,int> inverseEmbedding; ///inverse mapping W: V(g) -> V(q)

	AdjacenceListsGRAPH * dataGraph;
	AdjacenceListsGRAPH * queryGraph;
	vector<AdjacenceListsGRAPH> dataGraphVector;
	vector<AdjacenceListsGRAPH> queryGraphVector;


	//@common
	long sufficientNumberOfEmbeddings;
	long newEmbeddingsForEachQuery;
	long long totalNumberOfEmbeddings;
	long long numberOfRecursiveCalls;

public:

	Ullmann(std::string dataGraphFileName, std::string queryGraphFileName);
	//~Ullmann();	

	void execute();

	/* Subgraph Isomorphism Framework Function */
	void clean(); // clean function for each indicidual subgraph isomorphism (1 query , 1 data), will be executed in the beginning of genericQueryProc

	void genericQueryProc();

	void filterCandidates();

	void subgraphSearch();

	AdjacenceListsGRAPH::Vertex nextQueryVertex();

	bool refineCandidates(AdjacenceListsGRAPH::Vertex & u, const int & v);

	bool isJoinable(int u, int v);

	void updateState(int u, int v);

	void restoreState(int u, int v);


	//@common
	/* Common Utility Function  */

	void showEmbedding();

	//@common
	// Getter and setter
	long totalComputation();

	long totalEmbeddings();

	long totalRecursiveCalls();

	int  totalNumberOfDataGraphs();

	int  totalNumberOfQueryGraphs();

};



#endif /* ULLMANN */
