#include "rwdb.h"


int main(int argc, char *argv[]) {

	if(argc < 2) {
		printf("Usage: read [.stt file] [par1] [par2] ...\n");
		return 1;
	}

	// Read arguments from the command line
	int only_lines = 0; // Write only f-vectors
	int only1representative = 1; // Write only one repesentative per combinatorial type (don't read .01 files)
	int write_incmatrix = 1; // Write facet-vertex incidence matrix
	int fix_facets = 0; // If fix_facets > 0, then select only polytopes with fix_facets facets
	int fix_vertices = 0; // If fix_vertices > 0, then select only polytopes with fix_vertices vertices
	int there_are_errors = 0; // Errors when reading parameters
	int i;
	for(i = 2; i < argc; i++)
	{
		if (strcmp(argv[i], "-fv") == 0)
			only_lines = 1;
		else if (strcmp(argv[i], "-one") == 0)
			only1representative = 1;
		else if (strcmp(argv[i], "-all") == 0)
			only1representative = 0;
		else if (strcmp(argv[i], "-inc") == 0)
			write_incmatrix = 1;
		else if (strcmp(argv[i], "-notinc") == 0)
			write_incmatrix = 0;
		else if (strcmp(argv[i], "-f") == 0){
			i++;
			fix_facets = atoi(argv[i]);
		}	
		else if (strcmp(argv[i], "-v") == 0){
			i++;
			fix_vertices = atoi(argv[i]);
		}	
		else
		{
			printf ("Unrecognized option: %s\n", argv[i]);
			printf ("The list of possible options:\n");
			printf ("-fv    write only f-vectors\n");
			printf ("-one   write only one representative per combinatorial type\n");
			printf ("-all   write all representatives for a combinatorial type\n");
			printf ("-inc   write facet-vertex incidence matrix\n");
			printf ("-notinc   don't write facet-vertex incidence matrix\n");
			printf ("-f N   select only polytopes with N facets\n");
			printf ("-v M   select only polytopes with M vertices\n");
			printf ("\n");
			there_are_errors = 10;
		}
	}

	FILE *fvec_file; // Input file
	fvec_file = fopen(argv[1], "r");
	if (!fvec_file){
		printf ("Cann't open %s\n", argv[1]);
		return 2;
	}
	printf ("Read from %s\n", argv[1]);

	char out_fname[128]; // The name of the output file
	sprintf(out_fname, "%s", argv[1]);
	if (fix_vertices > 0)
		sprintf(out_fname + strlen(out_fname), "-%dv", fix_vertices);
	if (fix_facets > 0)
		sprintf(out_fname + strlen(out_fname), "-%df", fix_facets);
	sprintf(out_fname + strlen(out_fname), ".txt");
	FILE *outfile = fopen(out_fname, "w"); // Output file
	if (!outfile){
		printf ("Cann't open %s\n", out_fname);
		return 3;
	}	
	printf ("Write the result to %s\n", out_fname);

	uint64_t totalp = 0, totalc = 0, totalf = 0; // The total numbers of polytopes, combtypes, and f-vectors
	// Read input file line by line
	int line_num; 
	for (line_num = 1; ; line_num++){
		// Read the current f-vector (line from the input file)
		sttline stat; 
		int ret = read_line(fvec_file, &stat);
		if (ret == 0)	break;
		if (ret != 1){
			printf ("WARNING: wrong format in line %d of input file %s (error code %d)\n", line_num, argv[1], ret); 
			continue;
		}
		int dimension = stat.dimension;
		int vertices = stat.fvector[0];
		int facets = stat.fvector[stat.proper_dim - 1];
		
		// Restrictions on f-vectors
		if (fix_facets > 0 && fix_facets != facets) continue;
		if (fix_vertices > 0 && fix_vertices != vertices) continue;
		
		// Put here your restrictions on f-vectors...

		// Write the f-vector to the outfile
		write_line(outfile, &stat);
		totalp += stat.poly_num;
		totalc += stat.comb_num;
		totalf++;
		if (only_lines)	continue;

		char fname[256];
		sprintf (fname, "%dd/%02dv/%03df/%02dv%03df", dimension, vertices, facets, vertices, facets);
		int k;
		for (k = 2; k < dimension-1; k++)
			sprintf (fname + strlen(fname), "-%04d", stat.fvector[k]);
		int fname_len = strlen(fname);

		FILE *repfile = NULL; // File with combinatorial types
		repfile = fopen(fname, "rb");
		if (repfile == NULL){
            printf ("ERROR: cann't open file %s\n", fname);
            fprintf (outfile, "ERROR: cann't open file %s\n", fname);
			continue;
        }
		//else
		//	printf ("Read %s\n", fname);

		FILE *dbfile = NULL; // *.all file
		if (!only1representative){
			sprintf (fname + fname_len, ".01");
			dbfile = fopen(fname, "rb");
			if (dbfile == NULL){
				printf ("ERROR: Cann't open %s\n", fname);
				fprintf (outfile, "ERROR: Cann't open %s\n", fname);
				continue;
			}
		}	
		
		// Read all combinatorial types
		int comb_type_num;	
		for (comb_type_num = 0; comb_type_num < stat.comb_num; comb_type_num++) {
			cmbtype ctype; // Keeps data of the current combinatorial type (the incidence matrix and one representative)
			int ret = read_one_cmb(repfile, &ctype, vertices, facets);
			if (ret != 0){
				printf ("ERROR %d: Cann't read ctype %d\n", ret, comb_type_num);
				fprintf (outfile, "ERROR %d: Cann't read ctype %d\n", ret, comb_type_num);
				break;
			}
			fprintf (outfile, "Comb type %d:\n", comb_type_num);
			if (only1representative){
				if (write_incmatrix)
					print_inc (outfile, ctype.inc, vertices, facets);
				fprintf (outfile, "0/1-classes from %d to %d\nRepresentative:\n", ctype.beg, ctype.end);
				print_poly (outfile, ctype.poly, dimension, vertices);
			}
			else{	
				if (write_incmatrix)
					print_inc (outfile, ctype.inc, vertices, facets);
				fprintf (outfile, "0/1-classes from %d to %d:\n", ctype.beg, ctype.end);
				//fprintf (outfile, "Represented by %d polytopes:\n", ctype.end - ctype.beg + 1);
				// Read all representatives of the current combinatorial type
				fseek (dbfile, vertices * ctype.beg, SEEK_SET);
				int j;
				for (j = ctype.beg; j <= ctype.end; j++){
					uint8_t cur_poly[MAX_VERT];
					ret = fread(cur_poly, 1, vertices, dbfile);
					if(ret != vertices){
						printf ("ERROR: Cann't read polytope %d\n", j);
						fprintf (outfile, "ERROR: Cann't read polytope %d\n", j);
						break;
					}	
					fprintf (outfile, "Polytope %d:\n", j); // - ctype.beg + 1);
					print_poly (outfile, cur_poly, dimension, vertices);
				}
			}	
			//fprintf (outfile, "\n");
			
			//if(comb_type_num%100 == 0)
			//	printf("%d\n", comb_type_num);
		}
		if (repfile != NULL)
			fclose(repfile);
		if (dbfile != NULL)
			fclose(dbfile);
	}	

	fclose(fvec_file);
	fprintf (outfile, "Total: %lld f-vectors, %lld combinatorial types, and %lld polytopes\n", totalf, totalc, totalp);
	fclose(outfile);
//	printf (" %3d\n", line_num);

	return 0;
}
