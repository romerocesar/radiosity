#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "mdl.H"


float rgb[] = { 0.1, 0.2, 0.3 };
float pstn[] = { 1.0, 2.0, 3.0,
		 4.0, 5.0, 6.0,
		 7.0, 8.0, 9.0 };
long tri[] = { 0, 1, 2, 1, 2, 3, 3, 2, 0 };

char *histStr = "Created by new-test as a test file.";

mdlChunkSpec specs1[] = {
   { "moof", "f" },
   { "ack", "iff" },
   { NULL, NULL }
   };

mdlChunkSpec specs2[] = {
   { NULL, NULL }
   };

mdlChunkSpec specs3[] = {
   { "blarg", "issf" },
   { NULL, NULL }
   };



int main(int argc, char *argv[]) {
   
   char testFName[100];
   sprintf(testFName, "%d-test1.mdlb", getpid());

   FILE *fp = fopen(testFName, "w+");
   if (!fp) {
      perror(testFName);
      exit(1);
      }
   mdlOutput test1o(fp, 1, 1, mdlOpposite);

   test1o.BeginChunk("msh"); {
      test1o.WriteString("Name");
      test1o.BeginChunk("lmbrtn"); {
	 test1o.BeginChunk("rgb"); {
	    test1o.WriteFloat(rgb[0]);
	    test1o.WriteFloats(&rgb[1], 2);
	    test1o.EndChunk();
	    }
	 test1o.EndChunk();
	 }
      test1o.BeginChunk("vrtxPstn"); {
	 test1o.WriteFloats(pstn, 12);
	 test1o.EndChunk();
	 }
      test1o.BeginChunk("trngl"); {
	 test1o.WriteInt(tri[0]);
	 test1o.WriteInt(tri[1]);
	 test1o.WriteInt(tri[2]);
	 test1o.WriteInts(&tri[3], 6);
	 test1o.EndChunk();
	 }
      test1o.EndChunk();
      }

   test1o.BeginChunk("flHstry"); {
      test1o.WriteString(histStr);
      test1o.EndChunk();
      }


   rewind(fp);

   
   mdlInput test1i(fp, 1, NULL);

   mdlKey k;

   k = test1i.BeginChunk(); {
      assert(k == mdlKey("msh"));
      test1i.ReadString();
      k = test1i.BeginChunk(); {
	 assert(k == mdlKey("lmbrtn"));
	 k = test1i.BeginChunk(); {
	    assert(k == mdlKey("rgb"));
	    float rgb2[3];
	    rgb2[0] = test1i.ReadFloat();
	    test1i.ReadFloats(&rgb2[1], 2);
	    for (int i = 0; i < 3; i++)
	       assert(rgb2[i] == rgb[i]);
	    test1i.EndChunk();
	    }
	 test1i.EndChunk();
	 }
      k = test1i.BeginChunk(); {
	 assert(k == mdlKey("vrtxPstn"));
	 float pstn2[12];
	 test1i.ReadFloats(pstn2, 12);
	 for (int i = 0; i < 12; i++)
	    assert(pstn2[i] == pstn[i]);
	 test1i.EndChunk();
	 }
      k = test1i.BeginChunk(); {
	 assert(k == mdlKey("trngl"));
	 long tri2[9];
	 tri2[0] = test1i.ReadInt();
	 tri2[1] = test1i.ReadInt();
	 tri2[2] = test1i.ReadInt();
	 test1i.ReadInts(&tri2[3], 6);
	 for (int i = 0; i < 9; i++)
	    assert(tri2[i] == tri[i]);
	 test1i.EndChunk();
	 }
      test1i.EndChunk();
      }

   k = test1i.BeginChunk(); {
      assert(k == mdlKey("flHstry"));
      char * blarg;
      blarg = test1i.ReadString();
      assert(0 == strcmp(blarg, histStr));
      test1i.EndChunk();
      }

   
   fclose(fp);

   if (argc == 1)
      unlink(testFName);
   else
      cerr << "Leaving " << testFName << endl;




   
   mdlChunkSpecList csl(specs1);
   csl.Append(specs2);
   const char *spec;
   spec = csl.Find("blarg");
   assert(spec == NULL);
   csl.Append(specs3);
   spec = csl.Find("blarg");
   assert(spec && 0 == strcmp(spec, "issf"));
   spec = csl.Find("ack");
   assert(spec && 0 == strcmp(spec, "iff"));

   
   sprintf(testFName, "%d-test2.mdla", getpid());

   fp = fopen(testFName, "w+");
   if (!fp) {
      perror(testFName);
      exit(1);
      }
   mdlOutput test2o(fp, 0, 1, mdlSame, &csl);

   test2o.BeginChunk("blarg"); {
      test2o.WriteInt(1);
      test2o.WriteString("a");
      test2o.WriteString("b");
      test2o.WriteFloat(1.0);
      test2o.EndChunk();
      }

   rewind(fp);

   mdlInput test2i(fp, 1, &csl);

   test2i.BeginChunk(); {
      test2i.ReadInt();
      test2i.ReadString();
      test2i.ReadString();
      test2i.ReadFloat();
      test2i.EndChunk();
      }

   fclose(fp);

   
   if (argc == 1)
      unlink(testFName);
   else
      cerr << "Leaving " << testFName << endl;


   }




