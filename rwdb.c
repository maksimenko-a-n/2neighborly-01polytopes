#include "rwdb.h"

// Read line from input file sttfile
// The result of reading is stored in stat
// If the format of line is wrong, then return 2 
// If the end of file is reached, then return 0
int read_line(FILE *sttfile, sttline *stat)
{
	const size_t buffer_size = 1024;
	char buffer[buffer_size];
	char *pch;
	pch = fgets (buffer, buffer_size, sttfile);
	//printf ("line: %s", buffer);
	if ( pch == NULL){
		//printf ("The end of file\n");
		return 0;
	}	

	stat->simplicity = 1;
	stat->simpliciality = 3;
	stat->pyramid = 0;
	stat->minvinf = 0;
	stat->minfinv = MAX_VERT;
	
	char *bracket = strchr(buffer,'(');
	if (bracket == NULL || (*bracket) != '(')
		return 3;
	if (pch != bracket){
		if (memcmp(buffer, "simplex", 7) == 0){
			stat->simpliciality = MAX_DIM;
		}
		else{
			*bracket = '\0';
			// Read simplicity
			pch = strchr(buffer,'s');
			if (pch != NULL && (*pch) == 's')
				stat->simplicity = atoi(pch+1);
			// Read simpliciality
			pch = strchr(buffer,'c');
			if (pch != NULL && (*pch) == 'c')
				stat->simpliciality = atoi(pch+1);
			// Read pyramidality
			pch = strchr(buffer,'p');
			if (pch != NULL && (*pch) == 'p')
				stat->pyramid = atoi(pch+1);
			// Read min vertices in a facet
			pch = strchr(buffer,'v');
			if (pch != NULL && (*pch) == 'v')
				stat->minvinf = atoi(pch+1);
			// Read min facets in a vertex
			pch = strchr(buffer,'f');
			if (pch != NULL && (*pch) == 'f')
				stat->minfinv = atoi(pch+1);
			*bracket = '(';
		}
	}	
	
	// Read fvector
	pch = bracket + 1;
	int k;
	for (k = 0; k < MAX_DIM; k++){
		stat->fvector[k] = strtol (pch, &pch, 10);
		if ((*pch) == ')')
			break;
	}
	/*
	//printf ("k: %d\n", k);
	if (dimension > 0){
		if (k != dimension-1)
			return 4;
	}		
	else
		*/
	stat->dimension = k+1;

	//vertices = stat->fvector[0];
	int j;
	for (j = k; j > 0; j--)
		if (stat->fvector[j] != 0)
			break;
	//facets = stat->fvector[j];
	stat->proper_dim = j+1;
	// Read combnum
	pch = strchr(pch,'C');
	if (pch == NULL || (*pch) != 'C')
		return 5;
	stat->comb_num = atoi(pch+1);
	
	// Read polynum
	pch = strchr(buffer,'P');
	if (pch == NULL || (*pch) != 'P')
		return 6;
	stat->poly_num = atoi(pch+1);
	return 1;
}


// Write stat line in file
void write_line(FILE *file, sttline *stat)
{
    if (stat->simplicity + stat->simpliciality > stat->proper_dim)
		fprintf (file, "simplex");
    else{    
        if (stat->simplicity > 1)
            fprintf (file, "s%d", stat->simplicity);
        if (stat->simpliciality > 3)
            fprintf (file, "c%d", stat->simpliciality);
        if (stat->pyramid > 0)
            fprintf (file, "p%d", stat->pyramid);
        if (stat->minvinf > stat->proper_dim)
            fprintf (file, "v%d", stat->minvinf);
        if (stat->minfinv < stat->fvector[0] - 1)
            fprintf (file, "f%d", stat->minfinv);
    }        
	fprintf (file, "(");
	int k;
	for (k = 0; k < stat->dimension; k++)
		fprintf (file, " %d", stat->fvector[k]);
		//fprintf (file, " %4d", stat->fvector[k]);
	fprintf (file, ")C%dP%d\n", stat->comb_num, stat->poly_num);
}

// Read one type from file
// stt -- input data with f-vector
int read_one_cmb(FILE *cmb_file, cmbtype *ctype, int vertices, int facets){
    if (vertices > 16){
        if (fread(ctype->inc, sizeof(Trow), facets, cmb_file) != facets) return 1;
    }    
    else{
        uint16_t buffer[MAX_FACET];
        if (fread(buffer, sizeof(uint16_t), facets, cmb_file) != facets) return 2;
        Trow *inc = ctype->inc;
		int j;
        for (j = 0; j < facets; j++)
            inc[j] = buffer[j];
    }        
   	if (fread(&(ctype->beg), sizeof(uint32_t), 1, cmb_file) != 1) return 3;
   	if (fread(&(ctype->end), sizeof(uint32_t), 1, cmb_file) != 1) return 4;
	if (fread(ctype->poly, 1, vertices, cmb_file) != vertices) return 5;
	return 0;
}


// Write cmb-type
int write_one_cmb(FILE *cmb_file, cmbtype *ctype, int vertices, int facets){
    if (vertices > 16){
        if (fwrite(ctype->inc, sizeof(Trow), facets, cmb_file) != facets) return 1;
    }    
    else{
        Trow *inc = ctype->inc;
        uint16_t buffer[MAX_FACET];
		int j;
        for (j = 0; j < facets; j++)
            buffer[j] = inc[j];
        if (fwrite(buffer, sizeof(uint16_t), facets, cmb_file) != 1) return 2;
    }        
   	if (fwrite(&(ctype->beg), sizeof(uint32_t), 1, cmb_file) != 1) return 3;
   	if (fwrite(&(ctype->end), sizeof(uint32_t), 1, cmb_file) != 1) return 4;
	if (fwrite(ctype->poly, 1, vertices, cmb_file) != vertices) return 5;
    return 0;
}


// Print the facet-vertex incidence matrix into file
void print_inc (FILE *f, Trow *inc, int vertices, int facets){
	fprintf(f, "inc matrix:\n%d %d\n", facets, vertices);
	const int shift = vertices-1;
	const Trow y = 1 << shift;
	int i, j;
	for(i = 0; i < facets; i++){
		Trow x = inc[i];
		for (j = 0; j < vertices; j++, x <<= 1)
			fprintf(f, " %d", (x&y) >> shift);
		fprintf(f, "\n");
	}
	fprintf(f, "end\n");
}

// Print the polytope vertices into file
void print_poly (FILE *f, uint8_t *poly, int dimension, int vertices){
	fprintf(f, "%d %d\n", vertices, dimension);
	const int shift = dimension-1;
	const uint8_t y = 1 << shift;
	int i, j;
	for(i = 0; i < vertices; i++){
		uint8_t x = poly[i];
		for (j = 0; j < dimension; j++, x <<= 1)
			fprintf(f, " %d", (x&y) >> shift);
			//fprintf(f, " & %d", (x&y) >> shift); // LaTeX
		fprintf(f, "\n");
		//fprintf(f, "\\\\\n"); // LaTeX
	}
	fprintf(f, "end\n");
}

// Print the polytope vertices and its facet-vertex incidence matrix into txt-file
void write_representative(FILE *f, cmbtype *ctype, int dimension, int vertices, int facets){
	print_inc (f, ctype->inc, vertices, facets);
	fprintf (f, "0/1-classes from %d to %d\npolytope:\n", ctype->beg, ctype->end);
	print_poly (f, ctype->poly, dimension, vertices);
	//fprintf (f, "\n");
}
