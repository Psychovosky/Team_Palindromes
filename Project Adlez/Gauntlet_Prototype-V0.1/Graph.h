#pragma once
#include "HelpfulData.h"
#include "GraphNode.h"
#include "Texture.h"

class Graph
{
private:
	// the number of nodes in the graph
	int m_size;
	// relation ship of nodes in the graph
	bool **m_AdjacencyMatrix;
	// all nodes in the graph
	TemplateVector<GraphNode*> m_nodes;
	// texture used for debugging
	Texture m_nodeTex;
	Texture m_neighborTex;
public:
	Graph();
	~Graph();
	void release();
	void initialize(int a_size);
	int createNode(V2DF a_pos);
	void setOneWayConnection(int a_start, int a_end, int cost);
	void setTwoWayConnection(int a_start, int a_end, int costA, int costB);
	void setTwoWayConnection(int a_start, int a_end, int costA);
	void render();
	GraphNode* getNode(int a_node);
};