#include "MapMatchingUsingBiasStatistic.h"

list<Edge*> MapMatching(list<GeoPoint*> &trajectory){
	for each (GeoPoint* trajPoint in trajectory)
	{
		pair<int, int>gridCellIndex = routeNetwork.findGridCellIndex(trajPoint->lat, trajPoint->lon);
		if (biasSet.find(gridCellIndex) != biasSet.end()){

		}
	}
}