#include <mdl.H>



int SkipChunk(mdlInput &, mdlOutput &, mdlKey, mdlUserParam) {
   return 1;
   }




int main(int argc, char *argv[]) {

   mdlCopyCallbackSpec *cbss = new mdlCopyCallbackSpec[argc + 1];
   int ncbss = 0;

   int binary = TRUE;

   for (int i = 1; i < argc; i++) {
      if (argv[i][0] == '-') {
	 for (int j = 1; argv[i][j]; j++)
	    switch (argv[i][j]) {
	     case 'a':
	       binary = FALSE;
	       break;
	     default:
	       cerr << "Usage: " << argv[0]
		    << " [-a] [key key ...] < in.mdl > out.mdl" << endl;
	       cerr << "   -a -- output ascii file" << endl;
	       }
	 }
      else {
	 cbss[ncbss].fn = SkipChunk;
	 cbss[ncbss].key = argv[i];
	 ncbss++;
	 }
      }

   cbss[ncbss].fn = NULL;
   cbss[ncbss].key = NULL;

   mdlCopyCallbackList cbl(cbss);

   mdlInput inp(stdin, FALSE);
   mdlOutput outp(stdout, binary, FALSE);

   inp.CopyData(outp, &cbl);

   return 0;   
   }
