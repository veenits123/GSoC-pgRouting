#ifndef GRAPH_MINIMIZER_H_
#define GRAPH_MINIMIZER_H_

/*

Edge types:

0: Normal edge
1: Updated Edge
2: New Edge

*/
#include <deque>
#include <vector>
#include <set>
#include <stack>
#include <map>
#include <iostream>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include "../../common/src/myGraph.hpp"
#include "../../mydijkstra/src/myDijkstra.hpp"
#include "../../common/src/structs.h"
using namespace std;
using namespace boost;
template < class G >
class Graph_Minimizer
:public My_dijkstra<G> {
public:
	My_dijkstra<G> *reduced_graph;
  //constrictor for this class which inherits the base graph
	explicit Graph_Minimizer(graphType gtype, const int initial_size)
	:My_dijkstra<G>(gtype, initial_size) {
		reduced_graph=new My_dijkstra<G>(gtype,initial_size);
	}
	typedef typename boost::graph_traits < G >::vertex_descriptor V;
	typedef typename boost::graph_traits < G >::edge_descriptor E;
	typedef typename boost::graph_traits < G >::vertex_iterator V_i;

  //typedef typename std::map<V,E> removed_edges;
	typedef typename std::map<V,deque<Edge> > removed_V;
	typedef  std::deque<Edge>::iterator removed_E_i;
	typedef typename std::map< int64_t, std::vector<V> > degree_to_V;
	typedef typename std::map< int64_t, std::vector<V> >::iterator degree_to_V_i;
	typedef typename std::vector<V>::iterator Q_i;
	typedef typename boost::graph_traits < G >::out_edge_iterator EO_i;
	typedef typename std::map<V,deque<Edge> >::iterator removed_V_i;
	typedef typename std::map<int64_t , pair<int64_t,int64_t> > psuedo_E;
	typedef typename std::map<int64_t , pair<int64_t,int64_t> >::iterator psuedo_E_i;
	int64_t last_edge_id;
	typedef deque<Edge> unpackedPath;
	unpackedPath source_un_p,target_un_p;
	removed_V removedVertices;
	psuedo_E psuedoEdges;

  //initializes the graph with the given edges
	void
	initialize_graph(Edge *data_edges, int64_t count) {
		// /cout << "Initializing......." << endl;
		this->insert_data(data_edges, count);
		last_edge_id=count;
		reduced_graph->insert_data(data_edges,count);
	}


	void remove_1_degree_vertices()
	{
		EO_i out,out_end;
		V front=reduced_graph->degree_to_V_map[1].front();

		std::vector<V> one_degree_vertices=reduced_graph->degree_to_V_map[1];
		degree_to_V_i it;
		//cout << "1 degree vertices " << one_degree_vertices.size()  << endl;
		while (reduced_graph->degree_to_V_map[1].size()>0)
		{
			//cout << "Front " << reduced_graph->graph[front].id << endl;
			for (boost::tie(out, out_end) = out_edges(front, reduced_graph->graph);
				out != out_end; ++out) 
			{
				V s=source(*out, reduced_graph->graph);
				V t=target(*out, reduced_graph->graph);
				int source_id=reduced_graph->graph[s].id;
				int target_id=reduced_graph->graph[t].id;
				reduced_graph->remove_vertex(source_id);
				//cout << "Removing " << source_id  << endl;
				//cout << "Neighbor" << target_id << endl;
				reduced_graph->graph[t].contractions++;
				int prev_target_degree=reduced_graph->graph[t].degree;
				reduced_graph->graph[t].degree--;
				int final_target_degree=prev_target_degree-1;
				reduced_graph->degree_to_V_map[final_target_degree].push_back(t);
				Edge removed_edge=reduced_graph->graph[*out];
				//cout << "removing" << " (" << removed_edge.source<< ", " << removed_edge.target << ")" << endl;
				removedVertices[front].push_front(removed_edge);
			}
			reduced_graph->degree_to_V_map[1].erase(reduced_graph->degree_to_V_map[1].begin(),
				reduced_graph->degree_to_V_map[1].begin()+1);
			front=reduced_graph->degree_to_V_map[1].front();
		}

	}


	void remove_2_degree_vertices()
	{
		EO_i out,out_end;
		std::vector<V> neighbors_desc;
		std::vector<V> two_degree_vertices=reduced_graph->degree_to_V_map[2];
		std::vector<V> two_degree_vertices_0;
		for (Q_i it = two_degree_vertices.begin() ; it != two_degree_vertices.end(); ++it)
		{
			if (reduced_graph->graph[*it].contractions==0)
			{
				//cout << reduced_graph->graph[*it].id << endl;
				
				two_degree_vertices_0.push_back(*it);
			}
		}
		degree_to_V_i it;
		//cout << "2 degree vertices " << two_degree_vertices_0.size()  << endl;
		if (two_degree_vertices_0.size()==0)
		{
			return;
		}
		
		V front=two_degree_vertices_0.front();
		int64_t front_id;
		while (two_degree_vertices_0.size()>0)
		{
			//cout << "2 degree vertices " << two_degree_vertices_0.size()  << endl;
			//cout << "Edge count" << num_edges(reduced_graph->graph) << endl;
			//cout << "Front " << reduced_graph->graph[front].id << endl;
			neighbors_desc.clear();
				front=two_degree_vertices_0.front();
				reduced_graph->get_vertex_id(front,front_id);
			if (reduced_graph->graph[front].contractions==0)
			{
				/* code */
				
				int count=0;
				int64_t tid1,tid2;
				V  tdesc1,tdesc2;
				float min_distance=0.00000;
				for (boost::tie(out, out_end) = out_edges(front, reduced_graph->graph);
					out != out_end; ++out) 
				{
					V t=target(*out, reduced_graph->graph);
					min_distance+=reduced_graph->graph[*out].cost;
					reduced_graph->graph[t].contractions++;
					neighbors_desc.push_back(t);
				//removedVertices[t].push(front);
					count++;
				}
				if (count==2)
				{
				/* code */
					Edge shortcut;
					tdesc1=neighbors_desc[0];
					tdesc2=neighbors_desc[1];
					reduced_graph->get_vertex_id(tdesc1,tid1);
					reduced_graph->get_vertex_id(tdesc2,tid2);
					int64_t eid1=reduced_graph->graph[edge(tdesc1, front,reduced_graph->graph).first].id;
					int64_t eid2=reduced_graph->graph[edge(front, tdesc2,reduced_graph->graph).first].id;
					int64_t eid;
					bool b1=edge(tdesc1, tdesc2,reduced_graph->graph).second;
					bool b2=edge(tdesc2, tdesc1,reduced_graph->graph).second;
				//cout << b1 << "," << b2 << endl;
					if ( b1 || b2)
					{
						eid=reduced_graph->graph[edge(tdesc1, tdesc2,reduced_graph->graph).first].id;
						float dist=reduced_graph->graph[edge(tdesc1, tdesc2,reduced_graph->graph).first].cost;
						if (fabs(min_distance-dist)>=0.000001)
						{
							min_distance=dist;
						}
						else
						{
							reduced_graph->graph[edge(tdesc1, tdesc2,reduced_graph->graph).first].cost=min_distance;
							reduced_graph->graph[edge(tdesc1, tdesc2,reduced_graph->graph).first].type=1;
						}
					}
					else
					{
						shortcut.id=++last_edge_id;
						eid=shortcut.id;
						shortcut.source=tid1;
						shortcut.target=tid2;
						shortcut.cost=min_distance;
						shortcut.type=2;
						reduced_graph->graph_add_edge(shortcut);
					}
					psuedoEdges[eid]=make_pair(eid1,eid2);
					Edge removed_edge;
					for (int i = 0; i < 2; ++i)
					{
						V t=neighbors_desc[i];
						removed_edge=reduced_graph->graph[edge(front, t,reduced_graph->graph).first];
						removedVertices[front].push_front(removed_edge);
					}
					reduced_graph->remove_vertex(front_id);
				}
			}
			two_degree_vertices_0.erase(two_degree_vertices_0.begin(),
				two_degree_vertices_0.begin()+1);

		}
	}



	void remove_3_degree_vertices()
	{
		EO_i out,out_end;
		std::vector<V> three_degree_vertices=reduced_graph->degree_to_V_map[3];
		std::vector<V> three_degree_vertices_0;
		std::vector<V> neighbors_desc;
		for (Q_i it = three_degree_vertices.begin() ; it != three_degree_vertices.end(); ++it)
		{
			if (reduced_graph->graph[*it].contractions==0)
			{
				//cout << reduced_graph->graph[*it].id << endl;
				three_degree_vertices_0.push_back(*it);

			}
		}
		degree_to_V_i it;
		if (three_degree_vertices_0.size()==0)
		{
			return;
		}
		cout << "3 degree vertices " << three_degree_vertices_0.size()  << endl;
		V front=three_degree_vertices_0.front();
		int64_t front_id;
		while (three_degree_vertices_0.size()>0)
		{
			//cout << "2 degree vertices " << two_degree_vertices_0.size()  << endl;
			//cout << "Edge count" << num_edges(reduced_graph->graph) << endl;
			front=three_degree_vertices_0.front();
			//cout << "Front " << reduced_graph->graph[front].id << endl;
			neighbors_desc.clear();
			reduced_graph->get_vertex_id(front,front_id);
			int count=0;
			int64_t tid1,tid2;
			V  tdesc1,tdesc2;
			float min_distance=0.00000;
			for (boost::tie(out, out_end) = out_edges(front, reduced_graph->graph);
				out != out_end; ++out) 
			{
				V t=target(*out, reduced_graph->graph);
				reduced_graph->graph[t].contractions++;
				neighbors_desc.push_back(t);
				count++;
			}
			if (count==3)
			{
				/* code */
				for (int i=0;i<3; i++)
				{
					tdesc1=neighbors_desc[i];
				//cout << "tdesc1 " << reduced_graph->graph[tdesc1].id << endl;
					for (int j = i+1; j < 3; ++j)
					{
						tdesc2=neighbors_desc[j];
					//cout << "tdesc2 " << reduced_graph->graph[tdesc2].id << endl;
					//cout << tdesc1 << "," << tdesc2 << endl;
						Edge shortcut;
						float dist1=reduced_graph->graph[edge(front, tdesc2,reduced_graph->graph).first].cost;
						float dist2=reduced_graph->graph[edge(front, tdesc1,reduced_graph->graph).first].cost;
						int64_t eid1=reduced_graph->graph[edge(tdesc1, front,reduced_graph->graph).first].id;
						int64_t eid2=reduced_graph->graph[edge(front, tdesc2,reduced_graph->graph).first].id;
						int64_t eid;
						min_distance=dist1+dist2;
						bool b1=edge(tdesc1, tdesc2,reduced_graph->graph).second;
						bool b2=edge(tdesc2, tdesc1,reduced_graph->graph).second;
						if ( b1 || b2)
						{
							float dist=reduced_graph->graph[edge(tdesc1, tdesc2,reduced_graph->graph).first].cost;
							if (fabs(min_distance-dist)>=0.000001)
							{
								min_distance=dist;
							}
							else
							{
								reduced_graph->graph[edge(tdesc1, tdesc2,reduced_graph->graph).first].cost=min_distance;
								reduced_graph->graph[edge(tdesc1, tdesc2,reduced_graph->graph).first].type=1;
								eid=reduced_graph->graph[edge(tdesc1, tdesc2,reduced_graph->graph).first].id;
							}
						}
						else
						{
							reduced_graph->get_vertex_id(tdesc1,tid1);
							reduced_graph->get_vertex_id(tdesc2,tid2);
							shortcut.id=++last_edge_id;
							eid=shortcut.id;
							shortcut.source=tid1;
							shortcut.target=tid2;
							shortcut.cost=min_distance;
							shortcut.type=2;
							reduced_graph->graph_add_edge(shortcut);
						}
						psuedoEdges[eid]=make_pair(eid1,eid2);
					}
				}
				Edge removed_edge;
				for (int i = 0; i < 3; ++i)
				{
					V t=neighbors_desc[i];
					removed_edge=reduced_graph->graph[edge(front, t,reduced_graph->graph).first];
					removedVertices[front].push_front(removed_edge);
				}
				reduced_graph->remove_vertex(front_id);
			}
			three_degree_vertices_0.erase(three_degree_vertices_0.begin(),
				three_degree_vertices_0.begin()+1);
		}
	}

	int getreducedGraph(Edge **reduced_list)
	{
		int reduced_size=(int)num_edges(reduced_graph->graph);
		if ((*reduced_list)==NULL)
			(*reduced_list)=(Edge *)malloc(sizeof(Edge)*reduced_size);
		else
			(*reduced_list)=(Edge *)realloc((*reduced_list),sizeof(Edge)*reduced_size);
		V_i vi;
		EO_i out,out_end;
		int count=0;
		for (vi = vertices(reduced_graph->graph).first; vi != vertices(reduced_graph->graph).second; ++vi) {
			for (boost::tie(out, out_end) = out_edges(*vi, reduced_graph->graph);
				out != out_end; ++out)
			{
				V source_desc=source(*out, reduced_graph->graph);
				V target_desc=target(*out, reduced_graph->graph);
				int source_id=reduced_graph->graph[source_desc].id;
				int target_id=reduced_graph->graph[target_desc].id;
				
				if (target_desc>source_desc)
				{
					(*reduced_list)[count].id=reduced_graph->graph[*out].id;
					(*reduced_list)[count].source=source_id;
					(*reduced_list)[count].target=target_id;
					(*reduced_list)[count].cost=reduced_graph->graph[*out].cost;
					count++;
				}
				
			}
		}
		return count;
	}



	void contract_to_level(int level)
	{
		switch(level)
		{
			case 0 :
			this->remove_1_degree_vertices(); 
			break;
			case 1 :
			this->remove_1_degree_vertices();
			this->remove_2_degree_vertices(); 
			break;
			case 2:
			this->remove_1_degree_vertices();
			this->remove_2_degree_vertices();
			this->remove_3_degree_vertices();
			default :
		//	do nothing;
			break;
		}
	}


	void print_removed_vertices()
	{

		for (removed_V_i iter = removedVertices.begin(); iter != removedVertices.end(); iter++)
		{
			cout << "id: " << reduced_graph->graph[iter->first].id << endl;

			for (removed_E_i edge_iter = iter->second.begin(); edge_iter != iter->second.end(); edge_iter++)
			{
				Edge temp=*edge_iter ;
				cout << "(" << temp.source<< ", " << temp.target << "), ";
			}
			cout << endl;
		}
	}

	void print_psuedo_edges()
	{

		for (psuedo_E_i iter = psuedoEdges.begin(); iter != psuedoEdges.end(); iter++)
		{
			cout << "e1: " << iter->second.first << ",e: "<< iter->first << ",e2: " << iter->second.second << endl;
		}
	}


	void find_source_vertex(int64_t origId,int64_t &compId,unpackedPath &unpack)
	{
		V orig,comp;
		reduced_graph->get_vertex_descriptor(origId,orig);
		reduced_graph->get_vertex_descriptor(compId,comp);
		V temp=orig;
		while(!(removedVertices.find(temp) == removedVertices.end()))
		{
			Edge first=removedVertices[temp].front();
			if (first.source==origId)
			{
				//cout << "source" << endl;
				reduced_graph->get_vertex_descriptor(first.target,temp);
				unpack.push_back(first);
			}
			else if (first.target==origId)
			{
				//cout << "target" << endl;
				reduced_graph->get_vertex_descriptor(first.source,temp);
				unpack.push_back(first);
			}

		}
		reduced_graph->get_vertex_id(temp,compId);
		return;
	}
	void find_target_vertex(int64_t origId,int64_t &compId,unpackedPath &unpack)
	{
		V orig,comp;
		reduced_graph->get_vertex_descriptor(origId,orig);
		reduced_graph->get_vertex_descriptor(compId,comp);
		V temp=orig;
		while(!(removedVertices.find(temp) == removedVertices.end()))
		{
			Edge first=removedVertices[temp].front();
			if (first.source==origId)
			{
				//cout << "source" << endl;
				reduced_graph->get_vertex_descriptor(first.target,temp);
				unpack.push_front(first);
			}
			else if (first.target==origId)
			{
				//cout << "target" << endl;
				reduced_graph->get_vertex_descriptor(first.source,temp);
				unpack.push_front(first);
			}

		}
		reduced_graph->get_vertex_id(temp,compId);
		return;
	}


	void dijkstra_on_contracted(int64_t src,int64_t dest,Edge **path,int &size)
	{
		Edge **mainPath=NULL;
		unpackedPath srcPath,targetPath;
		int64_t closest_src,closest_target;
		int src_size,target_size;
		find_source_vertex(src,closest_src,srcPath);
		find_target_vertex(dest,closest_target,targetPath);
		src_size=srcPath.size();
		target_size=targetPath.size();

		reduced_graph->perform_dijkstra(closest_src,closest_target,mainPath,size);

		int total_size=size+src_size+target_size;
		*path=(Edge*)malloc(total_size*sizeof(Edge));
		for (int i = 0; i < src_size; ++i)
		{
			(*path)[i].id=srcPath[i].id;
			(*path)[i].source=srcPath[i].source;
			(*path)[i].target=srcPath[i].target;
			(*path)[i].cost=srcPath[i].cost;
		}
		for (int i = src_size; i < src_size+size; ++i)
		{
			(*path)[i].id=(*mainPath)[i-src_size].id;
			(*path)[i].source=(*mainPath)[i-src_size].source;
			(*path)[i].target=(*mainPath)[i-src_size].target;
			(*path)[i].cost=(*mainPath)[i-src_size].cost;
		}
		for (int i = src_size+size; i < total_size; ++i)
		{
			(*path)[i].id=targetPath[i-src_size-size].id;
			(*path)[i].source=targetPath[i-src_size-size].source;
			(*path)[i].target=targetPath[i-src_size-size].target;
			(*path)[i].cost=targetPath[i-src_size-size].cost;
		}
		size=total_size;
	}

	void print_path(Edge **path,int size)
	{
		cout << "Path for dijkstra......" << endl; 
		for (int i = 0; i < size; ++i)
		{
			cout << "id:- " << (*path)[i].id << " src:- " << (*path)[i].source << " dest:- " << (*path)[i].target << " cost " << (*path)[i].cost << endl;
		}
	}
};
#endif