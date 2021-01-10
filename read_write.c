#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "my_functions.h"


int main(){

	  FILE *fichero;

	  fichero = fopen("logs_read_data.txt", "w"); // log file to save extra information

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(fichero,"- %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    FILE* stream2 = fopen("spain.csv", "r");

    char line[79857]; //line of the file
    char *field;   //string between |
    char nombre[300];  //auxiliar to save the node name
    unsigned long num_nodos = 0, cont_node = 0;
    int direction, posicio, n,j = 0; // j counter of number of ways // n of arrows of each node
    int *campos_por_linea = NULL;


    // First loop to count the number of nodes in the file and the fields in each ways' row.

    while(fgets(line, sizeof(line), stream2)){
        char* tmp = strdup(line); 
        
        if( (field = strsep(&tmp, "|") ) != NULL ){

          if( strcmp(field, "node") == 0 ){ 
            num_nodos++;

          }else if( strcmp(field, "way") == 0 ){
            posicio = 1;
            j++;   
            campos_por_linea = realloc(campos_por_linea,j*sizeof(int));   
            while( (field = strsep(&tmp, "|") ) != NULL ){
              
              if (posicio == 7 ){  // to save if it is oneway or twoways
                if(strcmp(field, "oneway")==0){ // oneway
                  direction = 1;
                }else{// twoways
                  direction = 2;
                }
              }
              posicio++;
            }
            campos_por_linea[j-1] = posicio;    
          }
        }
    }
    fclose(stream2);

    
    // Second loop to fill the information.

    FILE* stream = fopen("spain.csv", "r");

    node *nodes; //list of nodes pointers.

	  nodes = (node *)malloc(num_nodos*sizeof(node));
    
    for(int i = 0; i < num_nodos; i++){
        nodes[i].arrows_succesors = NULL;
        nodes[i].arrows_succesors = (unsigned long *)malloc(20*sizeof(unsigned long));
    }

    int i = 0;
    unsigned long no_nodos = 0;


    while (fgets(line, sizeof(line), stream)){
        char* tmp = strdup(line); 
        int flag=0;  
        if( (field = strsep(&tmp, "|") ) != NULL ){

        /*
        We take from nodes:
          - id
          - latitude
          - longitude
        */

          if( strcmp(field, "node") == 0 && cont_node<num_nodos){ 
            posicio = 1;
            while( (field = strsep(&tmp, "|") ) != NULL ){
              if(posicio == 1 ){ nodes[cont_node].id = atol(field);  } 
              if(posicio == 9 ){ nodes[cont_node].lat = atof(field); }
              if(posicio == 10 ){ nodes[cont_node].lon = atof(field); }
              nodes[cont_node].narrow = 0;
              posicio++;
            }
            cont_node++; 
          }

        /*
        We take from ways:
          - name of street
          - the direction of the way, oneway of twoways.
          - the successors of the node
          - the number of successors for each node
        */
  
          else if (strcmp(field, "way") == 0 ){  
            posicio = 1;
            int new_position;
            unsigned long id_position_seg, id_position_prev;
            while( (field = strsep(&tmp, "|") ) != NULL){
                if (posicio == 2 ){  //to save the street node.
                    strcpy(nombre,field);
                }

                if (posicio == 7 ){ 
                  if(strcmp(field, "oneway")==0){ // oneway
                    direction = 1;
                  }else{// twoways
                    direction = 2;
                  }
                }
              
              /*
              Flag is an auxiliar to don't take account in the nodes' successors that don't appear in the node vector.
              If the flag is 0, the node don't appear in the node vector. And when an existing node is reached, the flag turns 1.
              */

              if(flag==0){
                if(posicio >= 9){
                  if( binarySearch(nodes,0,num_nodos, atol(field))!= -1){ 
                      id_position_prev = binarySearch(nodes,0,num_nodos, atol(field));
                      flag = 1;
                      new_position = posicio;
                  }
                } 
              }else if(flag==1){
                if(new_position > 9 && new_position <= (campos_por_linea[i]-1) ){ 

                  if( binarySearch(nodes,0,num_nodos, atol(field))!= -1){ 
                      id_position_seg = binarySearch(nodes,0,num_nodos, atol(field)); 

                      if(direction == 1){
                        n = nodes[id_position_prev].narrow;
                        nodes[id_position_prev].arrows_succesors[n] = id_position_seg;
                        nodes[id_position_prev].narrow = nodes[id_position_prev].narrow + 1;
                      }
                      
                      else if (direction == 2){
                        n = nodes[id_position_prev].narrow;
                        nodes[id_position_prev].arrows_succesors[n] = id_position_seg;
                        nodes[id_position_prev].narrow = nodes[id_position_prev].narrow + 1;

                        n = nodes[id_position_seg].narrow;
                        nodes[id_position_seg].arrows_succesors[n] = id_position_prev;
                        nodes[id_position_seg].narrow = nodes[id_position_seg].narrow + 1;
                      }
                  }else if (binarySearch(nodes,0,num_nodos, atol(field)) == -1){
                    no_nodos++;
                    flag=0;
                  }
                  id_position_prev = id_position_seg;
                }
              }
            posicio++; 
            new_position++;
          }
            i++;  //nunber of ways
          }

        }
    }

    fclose(stream);

    printf("Data read\n");
    
    fprintf(fichero,"- %lu of the %lu nodes don't appear in the node list.\n",no_nodos,num_nodos);
  
    // Creation of the binary file.

    FILE *fin;
    char name[200];
    
    if(nodes == NULL)
        ExitError("when allocating memory for the nodes vector", 5);

    unsigned long ntotnsucc=0UL;
        
    for(i=0; i < num_nodos; i++) {
      ntotnsucc = ntotnsucc + nodes[i].narrow;
    }
    strcpy(name, "archivo.bin");

    if ((fin = fopen (name, "wb")) == NULL)
        ExitError("the output binary data file cannot be opened", 31);

    if( fwrite(&num_nodos, sizeof(unsigned long), 1, fin) +
        fwrite(&ntotnsucc, sizeof(unsigned long), 1, fin) != 2 )
            ExitError("when initializing the output binary data file", 32);

    if( fwrite(nodes, sizeof(node), num_nodos, fin) != num_nodos )
            ExitError("when writing nodes to the output binary data file", 32);


    for(i=0; i < num_nodos; i++) if(nodes[i].narrow){
        if( fwrite(nodes[i].arrows_succesors, sizeof(unsigned long), nodes[i].narrow, fin) != nodes[i].narrow )
            ExitError("when writing edges to the output binary data file", 32);
    }
    free(nodes);

    fclose(fin);
    fprintf(fichero,"- Binary file generated.\n");
    fclose(fichero);  
}
