#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define EARTH_RADIUS 6371008.8 //mean earth radius (meters)
#define PI 3.14159

typedef struct {
    unsigned long id; // Node identification
    double lat, lon; // Node position
    unsigned short narrow; // Number of node successors; i. e. length of successors
    unsigned long *arrows_succesors;  // successors of node
} node;

typedef uint8_t Queue;
enum whichQueue {NONE, OPEN, CLOSED};

/*A Star status structure for a node */
typedef struct AStarStatus_s{
    double g,h,f;       //Cost
    unsigned long parent;    //Parent node 
    Queue whq;          //Queue status
} AStarStatus_t;

/*Dynamic list structure */
typedef struct queue_s{
    struct queue_s *next;
    unsigned long id;
} queue_t;


void ExitError(const char *miss, int errcode) {
    fprintf (stderr, "nnERROR: %s.nnStopping...nnnn", miss); exit(errcode);
}

/*  POW2
 *
 *  Input:
 *      a: a number.
 *
 *  Return: the potence of a number.
 */


double POW2(double a){
    return a*a;
}

/*  BINARYSEARCH

 *  Find the position of an ID in a vector.
 *
 *  Input:
 *      arr: vector where all the information is saved.
 *      start: where the search stats.
 *      last: where the search ends.
 *      item: node to find.
 *
 *  Return: the index of the vector where the item is located.
 */


int binarySearch(node arr[], unsigned long start, unsigned long last, unsigned long item) 
{ 
    if (last >= start) { 
        int mid = start + (last - start) / 2; 
		// If the element is present at the middle
        // itself
        if (arr[mid].id == item) 
            return mid; 
		
		// If element is smaller than mid, then
        // it can only be present in left subarray
        if (arr[mid].id > item) 
            return binarySearch(arr, start, mid - 1, item); 
		
		// Else the element can only be present
        // in right subarray
        return binarySearch(arr, mid + 1, last, item); 
    } 
	// We reach here when element is not
    // present in array
    return -1; 
} 

/*  DIS2NODES
 *
 *  Since we are on the
 *  Earth surface, we will use the great circle distance. In
 *  particular, we will use the haversine formula.
 *
 *  Input:
 *      n1,n2: nodes to compute the distance between.
 *
 *  Return: approximate distance in meters.
 */

double dis2nodes(node n1, node n2){
    double lat1 = n1.lat*PI/180;
    double lat2 = n2.lat*PI/180;
    double lon1 = n1.lon*PI/180;
    double lon2 = n2.lon*PI/180;
    double dlat = lat1-lat2;
    double dlon = lon1-lon2;
    double slat = sin(dlat*0.5);
    double slon = sin(dlon*0.5);

    return 2.*asin(sqrt(POW2(slat)+
           cos(lat1)*cos(lat2)*
           POW2(slon)))*EARTH_RADIUS;
}

