
#include "mdl.H"
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "util.c"

mdlChunkSpecList myspeclist(NULL);

int def_callback(mdlInput &, mdlKey, mdlUserParam);

static mdlReadCallbackSpec mycallbackspecs[] = {
  {NULL,def_callback},
  {NULL,NULL}
};

mdlReadCallbackList mycallbacks(mycallbackspecs);

typedef struct tagCChunkRec {
  mdlKey key;
  int count;
  int total_length;
} CChunkRec;

class ChunkCount {
 public:
  CChunkRec *list;
  int num;
  int num_alloc;

  ChunkCount() 
    { num = 0; num_alloc = 500; list = NULL; ExpandList(); }
  ~ChunkCount() { free(list); }

  void ExpandList()
    { num_alloc *= 2; 
      list = (CChunkRec *)realloc(list,num_alloc*sizeof(list[0]));
      assert(list);
    }
  CChunkRec *InternalFind(const mdlKey &key)
    { for(int i=0;i<num;i++)
	if (list[i].key == key) return &list[i];
      return NULL;
    }
  void AddChunk(const mdlKey &key, int length)
    { CChunkRec *cur = InternalFind(key);
      if (cur == NULL) {
	if (num >= num_alloc)
	  ExpandList();
	cur = &list[num++];
	cur->key = key;
	cur->count = 0;
	cur->total_length = 0;
      }
      cur->count++;
      cur->total_length += length;
    }
  void Print(FILE *f)
    {
      fprintf(f,"%8s  %8s  %12s  %12s\n",
	      "key","count","total length","avg length");
      for(int i=0;i<num;i++) {
	fprintf(f,"%8s  %8d  %12d  %12g\n", (char*)list[i].key,
		list[i].count, list[i].total_length,
		list[i].total_length/(double)list[i].count);
      }
    }
};

ChunkCount counter;

int def_callback(mdlInput& in, mdlKey key, mdlUserParam /*user_data*/)
{
  counter.AddChunk(key,in.NumRemain());
  return FALSE;
}

void usage(void)
{
  cout << "chunkcount [-th] inputfile\n";
  cout << " -t <filename>   specify a file of chunk types\n";
  cout << " -h              view this help message\n";
  cout << "count the chunks in a mdl file.\n";
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
    counter.Print(stdout);
  }
  fclose(inf);
}



