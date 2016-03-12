
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
  cout << "mdlcopy [-atph] inputfile outputfile\n";
  cout << " -a              create ASCII outputfile\n";
  cout << " -p              use stdin and stdout for i/o\n";
  cout << " -L              write output as little endian\n";
  cout << " -B              write output as big endian\n";
  cout << " -t <filename>   specify a file of chunk types\n";
  cout << " -h              view this help message\n";
  cout << "copies its input to the output.\n";
  exit(0);
}

int main(int argc, char **argv)
{
  char infile[128];
  char outfile[128];
  int use_binary = TRUE;
  int use_pipeio = FALSE;
  mdlByteOrder endianess = mdlSame;
  FILE *inf;
  FILE *outf;
  char c;

  while((c = getopt(argc, argv, "hapt:LB")) != (char)-1)
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
    case 'p':
      use_pipeio = TRUE;
      break;
    default:
      cout << "invalid option " << c << endl;
      usage();
    }
  if (use_pipeio) {
    if (argc != optind) {
      cout << "too many arguments";
      usage();
    }
    inf = stdin;
    outf = stdout;
  }
  else {
    if (argc - optind < 2) {
      cout << "not enough arguments\n";
      usage();
    }
    if (argc - optind > 2) {
      cout << "too many arguments\n";
      usage();
    }
    strcpy(infile,argv[optind]);
    strcpy(outfile,argv[optind+1]);
    inf = fopen(infile,"rb");
    if (inf == NULL) {
      cout << "could not open input file: "<<infile<<"\n";
      exit(1);
    }
    outf = fopen(outfile,(use_binary)?"wb":"w");
    if (outf == NULL) {
      cout << "could not open output file: "<<outfile<<"\n";
      exit(1);
    }
  }
  {
    mdlInput inmodel(inf,TRUE,&myspeclist);
    mdlOutput outmodel(outf,use_binary,TRUE,endianess,&myspeclist);
    inmodel.CopyData(outmodel,&mycallbacks,NULL);
    myhistory.AppendCommandLine(argc, argv);
    myhistory.Write(outmodel);
  }
  fclose(inf);
  fclose(outf);
}



