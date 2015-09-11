#include"VF2.h"
#include"TimeUtility.h"
#include"AdjacenceListsGRAPH_IO.h"
#include<iostream>
#include<vector>
#include<map>
#include<set>

using namespace std;
/*Helper Functions, Use With Care*/

//@unique for VF2 to be used in the update Cg
int vd_parent_level;

//@common
map<int,int>::iterator Global_Map_Int_Iterator; // one use it with in the isJoinable()
std::map<int,std::stack<int>>::iterator Global_stack_Iterator; // used in refineCandidate

VF2::VF2(string dataGraphFileName, string queryGraphFileName) {

	VF2::dataGraphFileName = dataGraphFileName;
	VF2::queryGraphFileName = queryGraphFileName;

	std::ifstream dataGraphFile = std::ifstream(dataGraphFileName);
	std::ifstream queryGraphFile = std::ifstream(queryGraphFileName);

	if(!dataGraphFile.is_open()){
		cout<<"data graph file doesn't exist"<<endl;
		exit(1);
	}
	if(!queryGraphFile.is_open()) {
		cout<<"query graph file doesn't exist"<<endl;
		exit(1);
	}
	AdjacenceListsGRAPH_IO::loadGraphFromFile(dataGraphFile, dataGraphVector);

	for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) {
		dataGraphVector[dataGraphIndex].buildLabelVertexList();
	}

	AdjacenceListsGRAPH_IO::loadGraphFromFile(queryGraphFile, queryGraphVector);

	numberOfRecursiveCalls = 0;
	sufficientNumberOfEmbeddings = 1000; // only calculate 1000 embeddings for each query graph
	totalNumberOfEmbeddings = 0;

	// @Unique VF2
	Cg = new int[0];
}

void VF2::execute(){

	for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++){
		
		dataGraph = &dataGraphVector[dataGraphIndex];

		for(int queryGraphIndex = 0; queryGraphIndex < queryGraphVector.size(); queryGraphIndex ++){
			queryGraph = &queryGraphVector[queryGraphIndex];
			genericQueryProc();
			cout<<queryGraphIndex<<" : finish one query with embeddings :"<<newEmbeddingsForEachQuery<<endl;
		}

		cout<<dataGraphIndex<<" finish one data graph ******* "<<endl;
	}
}

//@unique
void VF2::preCalculateCq() {

	std::pair<int,int> Cq_Mq_pair;
	set<int> Cq;
	bool* flags = new bool[queryMatchingSuquence.size()]();

	for(vector<TreeNode>::iterator queryVertexIterator = queryMatchingSuquence.begin(); queryVertexIterator != queryMatchingSuquence.end(); queryVertexIterator++){
		Cq_Mq_pair.first = 0;
		Cq_Mq_pair.second = 0;
		// update Cq
		flags[queryVertexIterator->id] = true;

		AdjacenceListsGRAPH::adjIterator queryVertexAdj(queryGraph, queryVertexIterator->id);
		for(AdjacenceListsGRAPH::link t = queryVertexAdj.begin(); !queryVertexAdj.end() ; t = queryVertexAdj.next()){
			if(flags[t->v] == true){
				continue;
			}
			if(Cq.insert(t -> v).second == false) {
				Cq_Mq_pair.first ++;
			}
			else {
				Cq_Mq_pair.second ++;
			}
		}
		Cq_Mq.insert(std::pair<int,std::pair<int,int>>(queryVertexIterator->id, Cq_Mq_pair));
	}

	delete[] flags;
}


void VF2::preCalculateMatchingOrder(){
	queryMatchingSuquence = TraversalAlgorithm::DFS_VertexSequence(queryGraph,0);
}

void VF2::clean(){
	candidates.clear(); // clear the candidates

	//@Unique VF2
	preCalculateMatchingOrder();

	// reallocate the memory for Cg
	try{
		Cg = new int[dataGraph->getNumberOfVertexes()]();
	}
	catch(std::bad_alloc& exc){
		cout<<"Allocate memory for Cg failed. Application teminated. "<<endl;
		exit(1);
	}

	Cq_Mq.clear(); // store the |Cq| and |adj(u)/Cq/Mq| 

	numberOfCgAdj = -1;
	numberOfCg_Mg_Adj = -1;

	preCalculateCq();

	newEmbeddingsForEachQuery = 0;
}

void VF2::genericQueryProc(){

	clean();

	/* filter candidates with a different label */
	filterCandidates();

	if(candidates.size() != queryGraph -> getNumberOfVertexes()){
		return;
	}
	subgraphSearch();
}

//@Common
void VF2::filterCandidates(){

	map<int,vector<int>>::iterator candidateSetsIterator;

	vector<AdjacenceListsGRAPH::Vertex> * queryVertexLists = queryGraph -> getVertexList();
	map<int,vector<int>> * labelDataVertexList = dataGraph -> getLabelVertexList();


	for(vector<AdjacenceListsGRAPH::Vertex>::iterator queryVertexIterator = queryVertexLists->begin(); queryVertexIterator != queryVertexLists->end(); queryVertexIterator++){

		candidateSetsIterator = labelDataVertexList -> find(queryVertexIterator->label);
		
		if(candidateSetsIterator != labelDataVertexList -> end()) {
			candidates.insert(std::pair<int,std::vector<int> *>(queryVertexIterator->id, & candidateSetsIterator->second));
		}
		else{
			// to do ? the query has no embeddings
			return;
		}
	}
}

void VF2::subgraphSearch() {

	numberOfRecursiveCalls ++;

	if( embedding.size() == queryGraph -> getNumberOfVertexes()){
		// FIND AN EMBDDING // // To Do ?
		//showEmbedding();
		totalNumberOfEmbeddings ++;
		newEmbeddingsForEachQuery ++;
		return ;
	}

	AdjacenceListsGRAPH::Vertex u = nextQueryVertex();
	vector<int> * candidates_u = candidates.find(u.id) -> second;

	// For each v in C(u)
	for(vector<int>::iterator v = candidates_u -> begin(); v != candidates_u -> end(); v++){

		if(newEmbeddingsForEachQuery >= sufficientNumberOfEmbeddings){
			break; // only calculate 1000 embeddings for each query graph
		}
		//refine candidate
		if(!refineCandidates(u,*v)){
			continue;
		}
		// check whether the corresponding edges exists
		if(!isJoinable(u.id, *v)){
			continue;
		}
		if(!isFlexible(u, *v)){
			continue;
		}
		
		updateCg(*v); 
		// Next
		updateState(u.id, *v);
		subgraphSearch();
		restoreState(u.id, *v);

		restoreCg(*v);     
	}

}


bool VF2::isFlexible(AdjacenceListsGRAPH::Vertex & u, int & v){

	numberOfCgAdj = 0;
	numberOfCg_Mg_Adj = 0;

	AdjacenceListsGRAPH::adjIterator dataVertexAdj(dataGraph, v);
	for(AdjacenceListsGRAPH::link t = dataVertexAdj.begin(); !dataVertexAdj.end() ; t = dataVertexAdj.next()){
		if(inverseEmbedding.find(t->v) == inverseEmbedding.end()) {
			if(Cg[t->v] == 0){
				numberOfCg_Mg_Adj ++;  // if t -> v is not in Embedding and not in Cg
			}
			else {
				numberOfCgAdj ++; // if v in Cg
			}
		}
	}

	std::pair<int,int> Cq_Mq_pair = Cq_Mq.find(u.id) -> second;

	/* According to the description of An inembedding.size() comparison, the folowing pruning rule may be wrong */
	if( Cq_Mq_pair.first > numberOfCgAdj) {
		return false;
	}
	if( Cq_Mq_pair.second > numberOfCg_Mg_Adj){
		return false;
	}

	return true;
}



AdjacenceListsGRAPH::Vertex VF2::nextQueryVertex() {
	return (queryGraph -> getVertexList())->at(queryMatchingSuquence.at(embedding.size()).id);
}


bool VF2::refineCandidates(AdjacenceListsGRAPH::Vertex & u, const int & v){

	// Cannot map a node twice, TODO ? For hyber-node
	if(inverseEmbedding.find(v) != inverseEmbedding.end()) {
		return false;
	}
	// prune out any vertex v such that v is not connected from already matched query vertices
	int parentNodeMatchedVertexId = 0;
	if(embedding.size() == 0) {
		parentNodeMatchedVertexId = -1;  // the first node
	}
	else {
		parentNodeMatchedVertexId = embedding.find(queryMatchingSuquence.at(embedding.size()).parentId)->second;
	}
	if( parentNodeMatchedVertexId!= -1 && !(dataGraph ->edge(v, parentNodeMatchedVertexId))){
		return false;
	}
	return true;
}

bool VF2::isJoinable(int u, int v) {

	AdjacenceListsGRAPH::adjIterator adjIterator(queryGraph, u);

	for(AdjacenceListsGRAPH::link t = adjIterator.begin();  !adjIterator.end(); t=adjIterator.next()){
		if(embedding.find(t->v) != embedding.end()){
			// u has an edge with query vertex t->v which has already been matched
			if( dataGraph -> edge( v, embedding.find(t->v)->second)){
				continue;
			}else{
				return false;
			}
		}
	}
	return true;
}





void VF2::updateCg(int & v) {

	vd_parent_level = Cg[v];
	Cg[v] = 0;

	AdjacenceListsGRAPH::adjIterator dataVertexAdj(dataGraph, v);
	for(AdjacenceListsGRAPH::link t = dataVertexAdj.begin(); !dataVertexAdj.end() ; t = dataVertexAdj.next()){
		if (inverseEmbedding.find(t->v) == inverseEmbedding.end() && Cg[t->v] == 0) // if va is not in Mg and not in Cg
			Cg[t->v] = inverseEmbedding.size() + 1; // add it to Cg and mark it with the current embedding.size()
	}
}

void VF2::restoreCg(int & v){
	AdjacenceListsGRAPH::adjIterator dataVertexAdj(dataGraph, v);
	for(AdjacenceListsGRAPH::link t = dataVertexAdj.begin(); !dataVertexAdj.end() ; t = dataVertexAdj.next()){
		int va = t->v;
		if (Cg[va] == embedding.size() + 1){
			Cg[va] = 0;
		}
	}

	Cg[v] = vd_parent_level;
}




// @Common


void VF2::updateState(int u, int v){
	embedding.insert(std::pair<int,int>(u, v));
	inverseEmbedding.insert(std::pair<int,int>(v, u));
}

void VF2::restoreState(int u, int v) {
	embedding.erase(u);
	inverseEmbedding.erase(v);
}



void VF2::showEmbedding() {
	std::cout<<"{";
	for(int i = 0; i < queryGraph -> getNumberOfVertexes(); i++){
		cout<<"Embedding: "<<i<<"->"<<embedding[i]<<" , ";
	}
	cout<<"}"<<endl;
}
// Getter and setter
long VF2::totalComputation() {
	return dataGraphVector.size() * queryGraphVector.size();
}

long VF2::totalEmbeddings() {
	return totalNumberOfEmbeddings;
}


long VF2::totalRecursiveCalls(){
	return numberOfRecursiveCalls;
}

int  VF2::totalNumberOfDataGraphs(){
	return dataGraphVector.size();
}
int  VF2::totalNumberOfQueryGraphs(){
	return queryGraphVector.size();
}