#include "Map.h"
#include "ReadInTrajs.h"

string rootFilePath = "D:\\Document\\Subjects\\Computer\\Develop\\Data\\SingaporeData\\";
Map routeNetwork = Map(rootFilePath, 500);

void main(){
	ofstream fout("RouteNetworkData.js");
	fout.precision(11);
	fout << "data = " << endl;
	fout << "{\"city\":\"Singapore\",\"edges\":[";
	int edgeIndex = 0;
	for each(Edge* edge in routeNetwork.edges){
		if (edge != NULL){
			if (edgeIndex > 0){
				fout << "," << endl;
			}			
			fout << "{\"edgeId\":" << edge->id << ",\"numOfFigures\":" << edge->figure->size() << ",\"figures\":[";
			int figureIndex = 0;
			for each (GeoPoint* figPoint in *(edge->figure)){
				fout << "{\"x\":" << figPoint->lon << ",\"y\":" << figPoint->lat << "}";
				if (figureIndex < edge->figure->size()-1){
					fout << ",";
				}
				figureIndex++;
			}
			fout << "]}";
		}		
		edgeIndex++;
	}
	fout << "]}" << endl;
	fout.close();
}