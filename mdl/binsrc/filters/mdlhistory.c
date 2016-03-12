
#include "mdl.H"
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "util.c"

mdlChunkSpecList myspeclist(NULL);
FileHistory myhistory;
int flhstry_callback(mdlInput& in, mdlKey mkey, 
		     mdlUserParam /*userdata*/)
{
  myhistory.Read(in,mkey);
  return TRUE;
}

static mdlReadCallbackSpec mycallbackspecs[] = {
  {"flHstry",flhstry_callback},
  {NULL,NULL}
};

mdlReadCallbackList mycallbacks(mycallbackspecs);

void usage(void)
{
  cout << "mdlhistory [-th] inputfile\n";
  cout << " -t <filename>   specify a file of chunk types\n";
  cout << " -h              view this help message\n";
  cout << "prints the file history chunk of a  mdl file to standard out.\n";
  exit(0);
}

int main(int argc, char **argv)
{
  char infile[128];
  FILE *inf;
  char c;

  while((c = getopt(argc, argv, "ht:")) != (char)-1)
    switch (c) {
    case 'h':
      usage();
      break;
    case 't':
      ParseTypeSpecFile(optarg,&myspeclist);
      break;
    default:
      cout << "invalid option " << c << endl;
      usage();
    }
  if (argc - optind < 1) {
    cout << "not enough arguments\n";
    usage();
  }
  if (argc - optind > 1) {
    cout << "too many arguments\n";
    usage();
  }
  strcpy(infile,argv[optind]);
  inf = fopen(infile,"rb");
  if (inf == NULL) {
    cout << "could not open input file: "<<infile<<"\n";
    exit(1);
  }
  {
    mdlInput inmodel(inf,TRUE,&myspeclist);
    inmodel.ReadData(&mycallbacks,NULL);
    myhistory.Print();
  }
  fclose(inf);
}



