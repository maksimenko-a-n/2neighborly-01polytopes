#ifndef RWDB_H
#define RWDB_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>  // uint64_t

#define MAX_VERT 21
#define MAX_DIM 8
#define MAX_FACET 512

// structure for one line of the input file
typedef struct {
	unsigned char dimension; // the dimension of a space
	unsigned char proper_dim; // the proper dimension of the polytope
	unsigned char simplicial; // if simplicial&1 == 1, then the polytope is simplicial
					 // if simplicial&2 == 2, then the polytope is 3-neighborly
	uint8_t pyramid;  // if pyramid > 0, then one of the facets is adjacent with all the other facets
				   // and 'pyramid' is equal to the number of vertices which do not belong to the facet
	uint8_t simpliciality, simplicity; // The max degree of simpliciality and simplicity for some of types
    uint8_t minvinf, minfinv; // The minimum number of vertices (facets) in a facet (vertex) for some of types
	int fvector[MAX_DIM];
	int poly_num; // the number of polytopes with such f-vector
	int comb_num; // the number of combinatorial types with such f-vector
} sttline;

typedef uint32_t Trow;

typedef struct {
	Trow inc[MAX_FACET]; // Incidence matrix
    uint32_t beg, end; // The 0/1-polytopes in .all file
	uint8_t poly[MAX_VERT]; // The example of a 0/1-polytope
} cmbtype;


// Read line from input file sttfile
// The result of reading is stored in stat
// If the format of line is wrong, then return 2 
// If the end of file is reached, then return 0
int read_line(FILE *sttfile, sttline *stat);

// Write stat line in file
void write_line(FILE *file, sttline *stat);

// Read one type from file
int read_one_cmb(FILE *cmb_file, cmbtype *ctype, int vertices, int facets);

// Write cmb-type to file
int write_one_cmb(FILE *cmb_file, cmbtype *ctype, int vertices, int facets);

// Print the facet-vertex incidence matrix into file
void print_inc (FILE *f, Trow *inc, int vertices, int facets);

// Print the polytope vertices into file
void print_poly (FILE *f, uint8_t *poly, int dimension, int vertices);

// Print the polytope vertices and its facet-vertex incidence matrix into txt-file
void write_representative(FILE *f, cmbtype *ctype, int dimension, int vertices, int facets);

#endif /* RWDB_H */
