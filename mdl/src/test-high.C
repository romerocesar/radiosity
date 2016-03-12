#include "mdl.H"
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>




void ReadColor(mdlInput &inp, mdlKey k) {

   if (k == mdlKey("rgb")) {
      cerr << "rgb color."
	   << " r = " << inp.ReadFloat()
	   << " g = " << inp.ReadFloat()
	   << " b = " << inp.ReadFloat()
	   << endl;
      }
   else
      cerr << "unrecognized color keyword " << k << endl;
   }



void ReadMaterial(mdlInput &inp, mdlKey k) {

   if (k == mdlKey("lmbrtn")) {
      ReadColor(inp, inp.BeginChunk());
      inp.EndChunk();
      }
   else
      cerr << "unrecognized material keyword " << k << endl;
   }


void ReadMesh(mdlInput &inp, mdlKey k) {

   cerr << "Found a mesh." << endl;
   char *name = strdup(inp.ReadString());
   cerr << "  Name is '" << name << "'." << endl;

   ReadMaterial(inp, inp.BeginChunk());
   inp.EndChunk();
   
   while (inp.NumRemain() != 0) {
      
      mdlKey k = inp.BeginChunk();
      
      if (k == mdlKey("vrtxPstn")) {
	 cerr << "  Vertex positions: " << endl;
	 while (inp.NumRemain() != 0) {
	    float x, y, z;
	    x = inp.ReadFloat();
	    y = inp.ReadFloat();
	    z = inp.ReadFloat();
	    cerr << "   (" << x << ", " << y << ", " << z << ")"
		 << endl;
	    }
	 }
      
      else if (k == mdlKey("vrtxNrml")) {
	 int count = inp.NumRemain() / 3;
	 cerr << "  Vertex normals (" << count << "): " << endl;
	 float (*nrms)[3] = (float (*)[3]) new float[3 * count];
	 inp.ReadFloats((float *) nrms, count * 3);
	 for (int i = 0; i < count; i++) {
	    float x, y, z;
	    x = nrms[i][0];
	    y = nrms[i][1];
	    z = nrms[i][2];
	    cerr << "   (" << x << ", " << y << ", " << z << ")"
		 << endl;
	    }
	 }
      
      else if (k == mdlKey("trngl")) {
	 cerr << "  Triangles: " << endl;
	 while (inp.NumRemain() != 0) {
	    long n, m, p;
	    n = inp.ReadInt();
	    m = inp.ReadInt();
	    p = inp.ReadInt();
	    cerr << "   (" << n << ", " << m << ", " << p << ")"
		 << endl;
	    }
	 }
      
      else {
	 cerr << "  Weird keyword " << k << "; ignoring..." << endl;
	 }
      
      inp.EndChunk();
      }
   }



void DoOldTest(mdlInput &inp) {

   while (inp.NumRemain() != 0) {

      mdlKey k = inp.BeginChunk();

      if (k == mdlKey("msh")) {
	 ReadMesh(inp, k);
	 }

      else if (k == mdlKey("nmdMtrl")) {
	 cerr << "Found a material." << endl;
	 char *name = inp.ReadString();
	 cerr << "  Name is '" << name << "'." << endl;
	 ReadMaterial(inp, inp.BeginChunk());
	 inp.EndChunk();
	 }

      else {
	 cerr << "Unrecognized object keyword " << k << endl;
	 }

      inp.EndChunk();
      }
   }



int RgbToSclr(mdlInput &inp, mdlOutput &outp, mdlKey, mdlUserParam) {
   
   float r = inp.ReadFloat();
   float g = inp.ReadFloat();
   float b = inp.ReadFloat();

   outp.BeginChunk("sclr");
   outp.WriteFloat((r + g + b) / 3.0);
   outp.EndChunk();

   return 1;
   }



mdlCopyCallbackSpec cbs[] = {
   { "rgb", RgbToSclr },
   { NULL, NULL }
   };


void DoCallbackTest1(mdlInput &inp, mdlOutput &outp) {

   mdlCopyCallbackList cbl(cbs);

   inp.CopyData(outp, &cbl);
   }


int main(int argc, char *argv[]) {

   mdlInput inp(stdin);

   if (argc == 1)
      DoOldTest(inp);

   else if (argc == 3) {
      mdlOutput outp(stdout);
      int n = atoi(argv[2]);
      switch (n) {
       case 1:
	 DoCallbackTest1(inp, outp);
	 break;
       default:
	 cerr << "Bad test number " << n << endl;
	 }
      }

   else {

      ifstream cmdStream(argv[1]);

      for (;;) {

	 char cmd;
	 int count;

	 cmdStream >> cmd >> count;

	 if (!cmdStream)
	    break;

	 switch (cmd) {
	  case 'i':
	    if (count == 0)
	       cout << "Int: " << inp.ReadInt();
	    else {
	       long *data = new long[count];
	       cout << inp.ReadInts(data, count) << " Ints:\n  first "
		    << data[0] << "; last " << data[count-1];
	       delete [] data;
	       }
	    break;
	  case 'f':
	    if (count == 0)
	       cout << "Float: " << inp.ReadFloat();
	    else {
	       float *data = new float[count];
	       cout << inp.ReadFloats(data, count) << " Floats:\n  first "
		    << data[0] << "; last " << data[count-1];
	       delete [] data;
	       }
	    break;
	  case 's':
	    cout << "String: " << inp.ReadString();
	    break;
	  case 'C':
	    cout << "Start: " << inp.BeginChunk();
	    break;
	  case 'c':
	    inp.EndChunk();
	    cout << "End.";
	    break;
	  case 'n':
	    cout << "Remain: " << inp.NumRemain();
	    break;
	  case 'l':
	    cout << "Level: " << inp.NestLevel();
	    break;
	  case 'd':
	    mdlStatus::DieOnError(0);
	    cerr << "DieOnError is off" << endl;
	    continue;
	  case 'D':
	    mdlStatus::DieOnError(1);
	    cerr << "DieOnError is on" << endl;
	    continue;
	    }
	 cout << "  --> [" << inp.LastStatus().Name() << "]" << endl;
	 }

      }

   exit(0);
   return 0;
   }


