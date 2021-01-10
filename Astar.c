#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>
#include "my_functions.h"


/*  INSERTNODETOQUEUE
 *
 *  Puts node into queue dynamic list in a sorted way,
 *  from smallest f to biggest f, obtained from status.
 *  
 *
 *  Input:
 *      queue: queue pointer to pointer of the first element.
 *      nodeId: ID to put into list in a sorted way.
 *      status: vector of status of the nodes in the algorithm.
 */

void insertNodeToQueue(queue_t **queue, unsigned long nodeId,AStarStatus_t *status){
    queue_t *auxQueue,*queueIterator; 
    auxQueue = malloc(sizeof(queue_t)); assert(auxQueue);
    auxQueue->id = nodeId;
    //Insert at the begining
    if(status[(*queue)->id].f >= status[nodeId].f){
        auxQueue->next = *queue;
        *queue = auxQueue;
    //Insert somewhere else
    }else{
        queueIterator = *queue;
        while(queueIterator->next != NULL && 
              status[queueIterator->next->id].f < status[nodeId].f)
            queueIterator = queueIterator->next;
        auxQueue->next = queueIterator->next;
        queueIterator->next = auxQueue;
    }

}

/*  DELETENODEFROMQUEUE
 *
 *  Deletes dynamic list element of queue containing nodeID.
 *  User must be sure that the ID is in the list. 
 *
 *  Input:
 *      queue: queue pointer to pointer of the first element.
 *      nodeId: ID to put into list in a sorted way.
 */

void deleteNodefromQueue(queue_t **queue, unsigned long nodeId){
    queue_t *auxQueue,*queueIterator;
    //Delete first element
    if((*queue)->id == nodeId){
        auxQueue = *queue;
        *queue = auxQueue->next;
        free(auxQueue);
    //Delete other element
    }else{
        queueIterator = *queue;
        while(queueIterator->next->id != nodeId)
            queueIterator = queueIterator->next;
        auxQueue = queueIterator->next;
        queueIterator->next = auxQueue->next;
        free(auxQueue);
    }
}
 

/*  ASTARALGORITHM
 *
 *  Given a startin node and a target node in the graph, the a-star
 *  algorithm is applied to find a path between them, trying to make
 *  it as short as possible. The path can be reconstructed from the
 *  AStarStatus vector after its completation. 
 *
 *  Input:
 *      nodes: vector of nodes.
 *      status: vector of AStarStatus which will be modified.
 *      nNodes: number of nodes in vector.
 *      startNode: position of starting node in the vector of nodes.
 *      targetNode: position of target node in the vector of nodes.
 *
 *  Return: 0 if algorithm was successfull in finding a path, 1 otherwise.
 */

int aStarAlgorithm(node *nodes, AStarStatus_t *status, unsigned long startNode, unsigned long targetNode,unsigned long nNodes){
    int i;
    queue_t *open,*auxQueue;
    unsigned long currentNode, successorNode;
    double successorCurrentCost;
    
    /* Initialize */
    open = malloc(sizeof(queue_t)); assert(open);

    open->next = NULL;
    open->id = startNode;

    status[startNode].g = 0.;
    status[startNode].h = dis2nodes(nodes[startNode],nodes[targetNode]);
    status[startNode].f = status[startNode].g+status[startNode].h;
    status[startNode].whq = OPEN;    

    //Main Loop
    while(open != NULL){
        // Select current node with smallest f
        currentNode = open->id;
        // If current node is target node, we are done
        if(currentNode == targetNode)
            break;
        //Expand each successor of current node
        for(i=0; i<nodes[currentNode].narrow;i++){
            successorNode = nodes[currentNode].arrows_succesors[i];
            successorCurrentCost = status[currentNode].g + dis2nodes(nodes[successorNode],nodes[currentNode]);
            if(status[successorNode].whq == OPEN){
                if(status[successorNode].g <= successorCurrentCost)
                    continue;
                else
                    deleteNodefromQueue(&open,successorNode); 
            }else if(status[successorNode].whq == CLOSED){
                if(status[successorNode].g <= successorCurrentCost)
                    continue;
                //Add successor node to open list
                status[successorNode].whq = OPEN;
            }else{
                //Add successor node to open list
                status[successorNode].whq = OPEN;
                status[successorNode].h = dis2nodes(nodes[successorNode],nodes[targetNode]);
            }
            status[successorNode].g = successorCurrentCost;
            status[successorNode].f = status[successorNode].g + status[successorNode].h;
            status[successorNode].parent = currentNode;
            insertNodeToQueue(&open,successorNode,status);
        }
        //Add current node to CLOSED list (also remove from open)
        status[currentNode].whq = CLOSED;
        deleteNodefromQueue(&open,currentNode);
        
        }

        while(open != NULL){
            auxQueue = open;
            open = open->next;
            free(auxQueue);
        }
    
        //Return that it was found or not
        if(currentNode == targetNode)
            return 0;
        else
            return 1;
}


int main(){
    int i;
    node *nodes;
    unsigned long num_nodos;

    unsigned long ntotnsucc;
    unsigned long *allsuccessors;

    struct timeval tval_before, tval_after, tval_result; //Timing

    /* Read binary file */

    FILE *fin;

    if ((fin = fopen ("archivo.bin", "r")) == NULL)
        ExitError("the data file does not exist or cannot be opened", 11);

    /* Global data header */
    if( fread(&num_nodos, sizeof(unsigned long), 1, fin) +
        fread(&ntotnsucc, sizeof(unsigned long), 1, fin) != 2 )
            ExitError("when reading the header of the binary data file", 12);

    if((nodes = (node *) malloc(num_nodos*sizeof(node))) == NULL)
        ExitError("when allocating memory for the nodes vector", 13);

    if( (
        allsuccessors = (unsigned long *) malloc(ntotnsucc*sizeof(unsigned long))
        ) == NULL)
            ExitError("when allocating memory for the edges vector", 15);

    /* Reading all data from file */
    if( fread(nodes, sizeof(node), num_nodos, fin) != num_nodos )
        ExitError("when reading nodes from the binary data file", 17);

    if(fread(allsuccessors, sizeof(unsigned long), ntotnsucc, fin) != ntotnsucc)
        ExitError("when reading sucessors from the binary data file", 18);
    fclose(fin);

    /* Setting pointers to successors */
    for(i=0; i < num_nodos; i++) if(nodes[i].narrow) {
        nodes[i].arrows_succesors = allsuccessors; allsuccessors += nodes[i].narrow;
    }

    
    /* Write information about the process in the logs_final_route.txt. */
    FILE *stderr;
    stderr = fopen("logs_final_route.txt", "w");

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(stderr,"- %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    /* Find initial and target nodes */
    /* bcn = 240949599 , sevilla  = 195977239 */
    
    unsigned long startNode = binarySearch(nodes, 0, num_nodos,240949599);
    unsigned long targetNode = binarySearch(nodes, 0, num_nodos,195977239);
    AStarStatus_t *status; //A star status vector, for all nodes
    unsigned long aux1;

    if(startNode == -1){
        fprintf(stderr,"- ERROR: Start node not found in graph.\n");
        return -1;
    }else
        fprintf(stderr,"- Starting node found in position %lu.\n",startNode);
    if(targetNode == -1){
        fprintf(stderr,"- ERROR: Target node not found in graph.\n");
        return -2;
    }else
        fprintf(stderr,"- Target node found in position %lu.\n",targetNode);
    
    /* Initiate status */    
   status = malloc(sizeof(AStarStatus_t)*num_nodos); 
   assert(status);
   for(int i=0; i<num_nodos;i++)
        status[i].whq = NONE;

    /* Apply the aStarAlgorithm  */
    gettimeofday(&tval_before,NULL);
    if(aStarAlgorithm(nodes,status,startNode,targetNode,num_nodos) == 0){
        fprintf(stderr,"- Solution found, with distance %lf meters.\n",status[targetNode].f);
    }else{
        fprintf(stderr,"- ERROR: No path was found\n");
    }
    gettimeofday(&tval_after,NULL);
    timersub(&tval_after,&tval_before,&tval_result);
    fprintf(stderr,"- Time of algorithm: %2ld.%06ld seconds\n", (long int)tval_result.tv_sec,(long int)tval_result.tv_usec);

    /* Write the route between the two nodes in the solution.txt file.  */
    FILE *solutionF;
    solutionF = fopen("solution.txt","w");

    fprintf(solutionF,"\n\n AStar finished. Time of algorithm: %2ld.%06ld seconds.\n\n",(long int)tval_result.tv_sec,(long int)tval_result.tv_usec);

    fprintf(solutionF,"Distance %lf meters.\n\n",status[targetNode].f);

    /* Counter of nodes visited */
    int nodes_visited = 1;
    aux1 = targetNode;
    while(aux1 != startNode){
        nodes_visited++;
        aux1 = status[aux1].parent;
    }

    fprintf(solutionF,"The number of nodes visited is %d .\n\n",nodes_visited);


    /*Print solution*/
    if(solutionF != NULL){
        aux1 = targetNode;
        while(aux1 != startNode){
        /*for(i=0;i<nodes[status[aux1].parent].narrow;i++){
            if(arrows[nodes[status[aux1].parent].arrows_succesors[i]].ndestination==nodes[aux1].id && arrows[nodes[status[aux1].parent].arrows_succesors[i]].norigen==nodes[status[aux1].parent].id){
                strcpy(name_node,arrows[nodes[status[aux1].parent].arrows_succesors[i]].street);
            }
        }
        */
        
            fprintf(solutionF,"Node id: %10lu | Distance: %10.2lf  \n",
                    nodes[aux1].id,status[aux1].g);

            aux1 = status[aux1].parent;
        }

        fprintf(solutionF,"Node id: %10lu | Distance: %10.2lf \n",
                nodes[aux1].id,status[aux1].g);

        fclose(solutionF);
    }else{
        fprintf(stderr,"Could not create solution file\n");
    }

    fclose(stderr);
    
    return 0;

}