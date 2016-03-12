#include "ply.h"

/*

This program is adapted from the file "plytest.c", version 1.1, that is
distributed with zippack.  This was its header comment:

> Sample code showing how to read and write PLY polygon files.
> Greg Turk, March 1994

This program reads a PLY file and writes it out as a mdl file.  It was written
by Steve Marschner in October 1996.

*/

#include <stdio.h>
#include <math.h>
#include <ply.h>
#include <stdlib.h>
#include <assert.h>

#include <mdl.H>


/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;             /* the usual 3-space position of a vertex */
} Vertex;

typedef struct Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
} Face;



/* information needed to describe the user's data to the PLY routines */

char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};


Vertex *vertices;


void usage(char *argv0) {
   cerr << "Usage: " << argv0
	<< " [-a] [-m materialname] file.ply > file.mdl" << endl;
   cerr << " -a              create ascii output file" << endl;
   cerr << " -m <material>   specify the material name (default 'plymat')"
	<< endl;
   exit(1);
   }


/******************************************************************************
Read in a PLY file.
******************************************************************************/

int main(int argc, char *argv[]) {
   int i,j;
   PlyFile *ply;
   int nelems;
   char **elist;
   int file_type;
   float version;
   int nprops;
   int num_elems;
   PlyProperty **plist;
   char *elem_name;
   int num_comments;
   char **comments;
   int num_obj_info;
   char **obj_info;

   int binary = 1;
   char *materialName = "plymat";

   {
      int c;

      while ((c = getopt(argc, argv, "am:")) != -1)
	 switch (c) {
	  case 'a':
	    binary = 0;
	    break;
	  case 'm':
	    materialName = optarg;
	    break;
	  default:
	    usage(argv[0]);
	    }

      if (argc - optind < 1) {
	 cerr << "too few arguments" << endl;
	 usage(argv[0]);
	 }
      if (argc - optind > 1) {
	 cerr << "too many arguments" << endl;
	 usage(argv[0]);
	 }
      }
   

   /* open a PLY file for reading */
   ply = ply_open_for_reading(argv[optind], &nelems, &elist, 
			      &file_type, &version);

   // Open a mdl file for writing
   mdlOutput outp(stdout, binary);

   // Start the mesh chunk
   outp.BeginChunk("msh");
   outp.WriteString("from ply file");

   // Write a material name reference
   outp.BeginChunk("mtrlNm"); {
      outp.WriteString(materialName);
      outp.EndChunk();
      }

   /* print what we found out about the file */
   fprintf (stderr, "ply2mdl: ply version %f\n", version);
   fprintf (stderr, "ply2mdl: ply type %d\n", file_type);

   /* go through each kind of element that we learned is in the file */
   /* and read them */

   for (i = 0; i < nelems; i++) {

      /* get the description of the first element */
      elem_name = elist[i];
      plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

      /* print the name of the element, for debugging */
      fprintf (stderr, "ply2mdl: element %s %d\n", elem_name, num_elems);

      /* if we're on vertex elements, read them in */
      if (equal_strings ("vertex", elem_name)) {

	 /* create a vertex list to hold all the vertices */
	 vertices = (Vertex *) malloc (sizeof (Vertex) * num_elems);

	 /* set up for getting vertex elements */
	 ply_get_property (ply, elem_name, &vert_props[0]);
	 ply_get_property (ply, elem_name, &vert_props[1]);
	 ply_get_property (ply, elem_name, &vert_props[2]);

	 cerr << "ply2mdl: Reading " << num_elems << " vertices." << endl;

	 /* grab all the vertex elements */
	 for (j = 0; j < num_elems; j++) {

	    /* grab an element from the file */
	    ply_get_element (ply, (void *) &vertices[j]);
	    }
	 
	 cerr << "ply2mdl: Writing " << num_elems << " vertices." << endl;

	 // Write out the vertices in a batch
	 outp.WriteFloatChunk("vrtxPstn", (float *) vertices, 3 * num_elems);
	 }


      /* if we're on face elements, read them in */
      if (equal_strings ("face", elem_name)) {

	 Face face;

	 /* set up for getting face elements */
	 ply_get_property (ply, elem_name, &face_props[0]);

	 // Remember what kind of chunk we have open -- 0 is none.
	 int last_nverts = 0;

	 cerr << "ply2mdl: Converting " << num_elems << " faces." << endl;

	 /* grab all the face elements */
	 for (j = 0; j < num_elems; j++) {

	    /* grab an element from the file */
	    ply_get_element (ply, (void *) &face);

	    // Write it to the mdl file
	    if (face.nverts == 3) {
	       if (last_nverts != 3) {
		  if (last_nverts != 0)
		     outp.EndChunk();
		  outp.BeginChunk("trngl");
		  last_nverts = 3;
		  }
	       outp.WriteInts((long *) face.verts, 3);
	       }

	    else if (face.nverts == 4) {
	       if (last_nverts != 4) {
		  if (last_nverts != 0)
		     outp.EndChunk();
		  outp.BeginChunk("qdrltrl");
		  last_nverts = 4;
		  }
	       outp.WriteInts((long *) face.verts, 4);
	       }

	    else {
	       if (last_nverts != 0)
		  outp.EndChunk();
	       outp.WriteFloatChunk("plygn", (float *) face.verts, face.nverts);
	       last_nverts = 0;
	       }
	    }


	 // End any open patch chunks.

	 if (last_nverts != 0)
	    outp.EndChunk();
	 
	 }

    
      /* print out the properties we got, for debugging */
      for (j = 0; j < nprops; j++)
	 fprintf (stderr, "property %s\n", plist[j]->name);
      }


   // End the mesh chunk

   outp.EndChunk();


   // Write a file history chunk to say where this came from

   outp.BeginChunk("flHstry");

   {
      // Write the command line to the file history

      char buf[512];
      char *bufend = buf + 511;
      char *cur, *from;
      int i;
      
      cur = buf;
      for(i = 0; i < argc && cur < bufend; i++) {
	 if (i > 0)
	    *cur++ = ' ';
	 from = argv[i];
	 while(*from && cur < bufend) 
	    *cur++ = *from++;
	 }
      *cur = 0;
      assert(cur-buf < 512);
      
      outp.WriteString(buf);
      }
   
   {
      // grab the comments in the ply file and stuff them in the file history
      
      char buf[512];
      comments = ply_get_comments (ply, &num_comments);
      for (i = 0; i < num_comments; i++) {
	 if (strlen(comments[i]) > 475)
	    comments[i][475] = 0;
	 sprintf(buf, "ply comment = %s", comments[i]);
	 outp.WriteString(buf);
	 }
      }

   {
      // grab and print out the object information
     
      char buf[512];
      obj_info = ply_get_obj_info (ply, &num_obj_info);
      for (i = 0; i < num_obj_info; i++) {
	 if (strlen(comments[i]) > 475)
	    comments[i][475] = 0;
	 sprintf(buf, "ply obj_info = %s", obj_info[i]);
	 outp.WriteString(buf);
	 }
      }

   outp.EndChunk();

   /* close the PLY file */
   ply_close (ply);
   }


