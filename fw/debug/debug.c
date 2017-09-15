

void   print_fmat3 (fmat3 *m, char *header);
void   print_fmat4 (fmat4 *m, char *header);
void  fprint_fmat4 (FILE *fp, fmat4 *m, char *header);

void save_vshader_results (volatile gpu_cfg_t* cfg);


void print_fmat4 (fmat4 *m, char *header) {
	printf ("%s\n", header);
	for (int i = 0; i < 4; i++) {
		printf("row %d: ", i);
		for (int j = 0; j < 4; j++)
			printf ("%f ", (*m)[i][j]);
		printf("\n");
	}
	printf("\n");
}

void print_fmat3 (fmat3 *m, char *header) {
	printf ("%s\n", header);
	for (int i = 0; i < 3; i++) {
		printf("row %d: ", i);
		for (int j = 0; j < 3; j++)
			printf ("%f ", (*m)[i][j]);
		printf("\n");
	}
	printf("\n");
}


void fprint_fmat4 (FILE *fp, fmat4 *m, char *header) {
	fprintf (fp, "%s\n", header);
	for (int i = 0; i < 4; i++) {
		fprintf(fp, "row %d: ", i);
		for (int j = 0; j < 4; j++)
			fprintf (fp, "%f ", (*m)[i][j]);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");
}


void save_vshader_results (volatile gpu_cfg_t* cfg) {
	FILE *fp = fopen ("vshader_tri_data_array.txt", "w");
	if (!fp) printf ("Error opening vshader_tri_data_array.txt\n");
	fprintf (fp, "Printing tri_data_array below:\n");
	
	volatile ObjectListNode* volatile obj_list_node = cfg->obj_list_ptr;		
	int num_of_faces = 0;
	while (obj_list_node != NULL) {
		num_of_faces += wfobj_get_num_of_faces(obj_list_node->obj->wfobj);
		obj_list_node = obj_list_node->next;
	}
	
	for (int i = 0; i < num_of_faces; i++) {
		
		volatile TrianglePShaderData *t = cfg->tri_data_array;
		
		fprintf (fp, "Triangle %d:\n", i);
		
		fprintf (fp, "\tScreen coords: (%d %d %d) (%d %d %d) (%d %d %d)\n", t[i].screen_coords[0].as_struct.x, t[i].screen_coords[0].as_struct.y, t[i].screen_coords[0].as_struct.z,
																			t[i].screen_coords[1].as_struct.x, t[i].screen_coords[1].as_struct.y, t[i].screen_coords[1].as_struct.z,
																			t[i].screen_coords[2].as_struct.x, t[i].screen_coords[2].as_struct.y, t[i].screen_coords[2].as_struct.z);
		
		fprintf (fp, "\tW reciprocal: %d %d %d\n", t[i].w_reciprocal[0], t[i].w_reciprocal[1], t[i].w_reciprocal[2]);
		
		for (int j = 0; j < 3; j++) {
			fprintf (fp, "\tVarying %d (%d written, %d read):", j, t[i].varying[j].num_of_words_written, t[i].varying[j].num_of_words_read);
			for (int k = 0; k < NUM_OF_VARYING_WORDS; k++) fprintf (fp, " %d", t[i].varying[j].data[k].as_fixpt_t);
			fprintf (fp, "\n");
		}
		
		fprintf (fp, "\tObject: scale (%f %f %f), rotate (%f %f %f), tran (%f %f %f)\n", t[i].obj->scale[0], t[i].obj->scale[1], t[i].obj->scale[2], t[i].obj->rotate[0], t[i].obj->rotate[1], t[i].obj->rotate[2], t[i].obj->tran[0], t[i].obj->tran[1], t[i].obj->tran[2]);
	}
	fclose (fp);
}

void save_vshader_results (volatile gpu_cfg_t* cfg) {
	FILE *fp = fopen ("vshader_obj_list.txt", "w");
	if (!fp) printf ("Error opening vshader_obj_list.txt\n");
	fprintf (fp, "Printing obj_list below:\n");

	volatile ObjectListNode* volatile obj_list_head = cfg->obj_list_ptr;
	volatile ObjectListNode* volatile obj_list_node;
	obj_list_node = obj_list_head;

	while (obj_list_node != NULL) {
		fprintf (fp, "New object:\n");
		for (size_t i = 0; i < wfobj_get_num_of_faces(obj_list_node->obj->wfobj); i++) {
			fprintf (fp, "Face %d\n", i);
			fprint_fmat4 (fp, &(obj_list_node->obj->model), "\tmodel:");
			fprint_fmat4 (fp, &(obj_list_node->obj->mvp),   "\tmvp:");
			fprint_fmat4 (fp, &(obj_list_node->obj->mvit),  "\tmvit:");
			for (int j = 0; j < MAX_NUM_OF_LIGHTS; j++) {
				fprintf (fp, "\t Shadow MVP %d\n", j);
				fprint_fmat4 (fp, &(obj_list_node->obj->shadow_mvp[j]), "\tshadow_mvp");
			}
			for (int j = 0; j < 3; j++) {
				Float3 v = wfobj_get_vtx_coords (obj_list_node->obj->wfobj, i, j);
				fprintf (fp, "\tVTX%d Coords: (%f %f %f)\n", j, v.as_struct.x, v.as_struct.y, v.as_struct.z);
			}
			fprintf (fp, "\n");
		}
		obj_list_node = obj_list_node->next;
	}
	fclose (fp);
}
