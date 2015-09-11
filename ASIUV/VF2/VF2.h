#ifndef VF_2
#define VF_2

#include"TraversalAlgorithm.h"
#include<vector>
#include<stack>
#include<string>
#include<map>

using namespace std;

class VF2 {

private:

	//@Common
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

	//@Unique
	int* Cg;
	std::map<int,std::pair<int,int>> Cq_Mq; // store the |Cq| and |adj(u)/Cq/Mq| 
	int numberOfCgAdj;
	int numberOfCg_Mg_Adj;
	std::vector<TreeNode> queryMatchingSuquence; //vertexId, parentId; // Don'e clean after reset the data of tester

public:

	VF2(std::string dataGraphFileName, std::string queryGraphFileName);
	//~VF2();	

	void execute();

	/* Subgraph Isomorphism Framework Function */
	void clean(); // clean function for each indicidual subgraph isomorphism (1 query , 1 data), will be executed in the beginning of genericQueryProc

	void genericQueryProc();

	void filterCandidates();

	void subgraphSearch();

	AdjacenceListsGRAPH::Vertex nextQueryVertex();

	bool refineCandidates(AdjacenceListsGRAPH::Vertex & u,const int & v);

	bool isJoinable(int u, int v);

	void updateState(int u, int v);

	void restoreState(int u, int v);
	/* End of the framework */


	
	//@Unique VF2
	bool isFlexible(AdjacenceListsGRAPH::Vertex & u, int & v);   // check Cq and Cg requirements

	//@Unique Vf2, calculate the matching order according to the BFS tree 
	void preCalculateMatchingOrder();

	void preCalculateCq();

	void updateCg(int & v);

	void restoreCg(int & v);

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



#endif /* VF2 */
