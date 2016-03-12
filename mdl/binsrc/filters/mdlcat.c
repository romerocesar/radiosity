
#include "mdl.H"
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "util.c"

mdlChunkSpecList myspeclist(NULL);
FileHistory myhistory;
int flhstry_callback(mdlInput& in, mdlOutput& /*out*/, mdlKey mkey, 
		     mdlUserParam /*userdata*/)
{
  myhistory.Read(in,mkey);
  return TRUE;
}
static mdlCopyCallbackSpec mycallbackspecs[] = {
  {"flHstry",flhstry_callback},
  {NULL,NULL}
};
mdlCopyCallbackList mycallbacks(mycallbackspecs);

void usage(void)
{
  cout << "mdlcat [-athLB] inputfile(s)\n";
  cout << " -a              create ASCII outputfile\n";
  cout << " -L              write output as little endian\n";
  cout << " -B              write output as big endian\n";
  cout << " -t <filename>   specify a file of chunk types\n";
  cout << " -h              view this help message\n";
  cout << "concatenates its input files to standard out.\n";
  exit(0);
}

int main(int argc, char **argv)
{
  char infile[128];
  int use_binary = TRUE;
  mdlByteOrder endianess = mdlSame;
  FILE *inf;
  FILE *outf;
  char c;

  while((c = getopt(argc, argv, "hat:LB")) != (char)-1)
    switch (c) {
    case 'h':
      usage();
      break;
    case 'a':
      use_binary = FALSE;
      break;
    case 'L':
      endianess = mdlLittleEndian;
      break;
    case 'B':
      use_binary = mdlBigEndian;
      break;
    case 't':
      ParseTypeSpecFile(optarg,&myspeclist);
      break;
    default:
      cout << "invalid option " << c << endl;
      usage();
    }
  outf = stdout;
  {
    mdlOutput outmodel(outf,use_binary,TRUE,endianess,&myspeclist);
    if (argc - optind < 1) {
      cout << "not enough arguments\n";
      usage();
    }
    for(;argc>optind;optind++) {
      strcpy(infile,argv[optind]);
      inf = fopen(infile,"rb");
      if (inf == NULL) {
	cout << "could not open input file: "<<infile<<"\n";
	exit(1);
      }
      {
	char buf[256];
	sprintf(buf,"------file--%s----",infile);
	myhistory.Append(buf);
	mdlInput inmodel(inf,TRUE,&myspeclist);
	inmodel.CopyData(outmodel,&mycallbacks,NULL);
      }
      fclose(inf);
    }
    myhistory.AppendCommandLine(argc, argv);
    myhistory.Write(outmodel);
  }
}



