
#include "mdl.H"
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "util.c"

mdlChunkSpecList myspeclist(NULL);
FileHistory myhistory;

int tri_callback(mdlInput &, mdlOutput &, mdlKey, mdlUserParam);
int history_callback(mdlInput &in, mdlOutput &/*out*/, mdlKey key, mdlUserParam /*udata*/)
{
  myhistory.Read(in,key);
  return TRUE;
}

static mdlCopyCallbackSpec mycallbackspecs[] = {
  {"trngl",tri_callback},
  {"flHstry",history_callback},
  {NULL,NULL}
};

mdlCopyCallbackList mycallbacks(mycallbackspecs);

typedef struct tagTriangle {
  long v[3];
  long samev[3];
  long neigh[3];
  long used;
} Triangle;

typedef struct tagStripRec {
  long t;
  long v[3];
  long parity;
} StripRec;

#define END_STRIP (-1)

class TriData {
 public:
  long numtri;
  Triangle *tri;
  long numstrip;
  long tris_in_strips;
  long *strip;
  long *endstrip;
  long maxvert;
  long *stack;
  long *stacktop;

  TriData();
  ~TriData();
  void Read(mdlInput& in);
  void Write(mdlOutput& out);
  void MakeStrips(void);
 private:  //internal routines
  void MakeSameV(void);
  long FindNeighbor(long tri, long v1, long v2);
  void MakeNeighbors(void);
  void ClearStackStrip(int tostrip);
  void MakeStackStrip(StripRec rec, StripRec *newstart);
};

 TriData::TriData()
{
  numtri = 0;
  tri = NULL;
  numstrip = 0;
  tris_in_strips = 0;
  strip = NULL;
  maxvert = 0;
  stack = NULL;
}

 TriData::~TriData()
{
  if (tri)
    free(tri);
  if (strip)
    free(strip);
  if (stack)
    free(stack);
}

void TriData::Read(mdlInput& in)
{
  long i,j;
  long vert;

  assert(tri==NULL);
  numtri = in.NumRemain()/3;
  tri = (Triangle *)calloc((int)(sizeof(Triangle)*numtri),1);
  for(i=0;i<numtri;i++) {
    for(j=0;j<3;j++) {
      vert = in.ReadInt();
      if (vert >= maxvert) maxvert = vert+1;
      tri[i].v[j] = vert;
    }
    if ((tri[i].v[0] == tri[i].v[1])||(tri[i].v[1] == tri[i].v[2])
	||(tri[i].v[2] == tri[i].v[0])) {
      fprintf(stderr,"warning skipping degenerate triangle.\n");
      numtri--;
      i--;
    }
  }
}

void TriData::Write(mdlOutput& out)
{
  long i;
  long cur, end;
  
  cur = 0;
  for(i=0;i<numstrip;i++) {
    for(end=cur;strip[end]!=END_STRIP;end++);
    out.WriteIntChunk(mdlKey("trnglStr"),&strip[cur],(int)(end-cur));
    cur = end + 1;
  }
  if (tris_in_strips < numtri) {
    out.BeginChunk(mdlKey("trngl"));
    for(i=0;i<numtri;i++) {
      if (!tri[i].used) {
	out.WriteInts(tri[i].v,3);
      }
    }
    out.EndChunk();
  }
}

void TriData::MakeSameV(void)
{
  long *prev;
  long i,j;
  long v;

  prev = (long *)malloc((int)(sizeof(long)*maxvert));
  assert(prev);
  for(i=0;i<maxvert;i++)
    prev[i] = -1;
  for(i=0;i<numtri;i++) {
    for(j=0;j<3;j++) {
      v = tri[i].v[j];
      tri[i].samev[j] = prev[v];
      prev[v] = i;
    }
  }
  for(i=0;i<numtri;i++) {
    for(j=0;j<3;j++) {
      v = tri[i].v[j];
      if (tri[i].samev[j] == -1)
	tri[i].samev[j] = prev[v];
    }
  }
  free(prev);
}

long TriData::FindNeighbor(long t, long v1, long v2)
{
  long i;
  long localv;
  long curt = t;

  for(;;) {
    for(i=0;i<3;i++)
      if (tri[curt].v[i] == v1)
	break;
    assert(i<3);
    localv = i;
    curt = tri[curt].samev[localv];
    assert(curt>=0&&curt<numtri);
    if (curt == t)
      return -1;  //no neighbor found
    for(i=0;i<3;i++)
      if (tri[curt].v[i] == v2)
	return curt;
  }
}
    
void TriData::MakeNeighbors(void)
{
  long i;

  for(i=0;i<numtri;i++) {
    tri[i].neigh[0] = FindNeighbor(i,tri[i].v[1],tri[i].v[2]);
    tri[i].neigh[1] = FindNeighbor(i,tri[i].v[2],tri[i].v[0]);
    tri[i].neigh[2] = FindNeighbor(i,tri[i].v[0],tri[i].v[1]);
  }
}

void TriData::ClearStackStrip(int tostrip)
{
  long *cur;
  long t,i;

  if (stacktop == stack)
    return;
  assert(stacktop-stack>3);
  if (tostrip) {
    numstrip++;
    tris_in_strips -= 2;
    for(cur=stack+1;cur<stacktop;) {
      tris_in_strips++;
      *endstrip++ = *cur++;
    }
    *endstrip++ = END_STRIP;
  } else {
    cur = stack;
    t = *cur++;
    cur += 2;  //point to last vertex in first triangle
      for(;cur<stacktop;cur++) {
	assert(t>=0&&t<numtri);
	assert(*cur>=0&&*cur<maxvert);
	assert(tri[t].used);
	tri[t].used = FALSE;
	for(i=0;i<3;i++)
	  if (tri[t].v[i] == cur[-2])
	    break;
	assert(i<3);
	t = tri[t].neigh[i];
      }
  }
  stacktop = stack;
}

void TriData::MakeStackStrip(StripRec rec, StripRec *newstart)
{
  long i,t;
  long parity;

  assert(stack&&(stacktop==stack));
  if (newstart)
    newstart->t = -1;
  t = rec.t;
  if (t < 0)
    return;
  parity = rec.parity;
  assert(!tri[t].used);
  *stacktop++ = rec.t;
  *stacktop++ = rec.v[0];
  *stacktop++ = rec.v[1];
  *stacktop++ = rec.v[2];
  for(;;) {
    if (newstart&&parity) {    //setup rec to be beginning of a strip in
      newstart->t = t;         //in the reverse direction.
      newstart->v[0] = stacktop[-1];
      newstart->v[1] = stacktop[-2];
      newstart->v[2] = stacktop[-3];
    }
    parity = !parity;
    tri[t].used = TRUE;
    for(i=0;i<3;i++)
      if (tri[t].v[i] == stacktop[-3])
	break;
    assert(i<3);
    t = tri[t].neigh[i];
    if ((t<0)||tri[t].used)
      break;
    for(i=0;i<3;i++)
      if ((tri[t].v[i] != stacktop[-2])&&(tri[t].v[i] != stacktop[-1]))
	break;
    assert(i<3);
    *stacktop++ = tri[t].v[i];
  }
}
    
void TriData::MakeStrips(void)
{
  long i,j;
  long len,maxlen,beststrip;
  StripRec rec[6];

  assert(strip==NULL);
  assert(stack==NULL);
  MakeSameV();
  MakeNeighbors();
  strip = (long *)malloc((int)(sizeof(long)*3*numtri));
  endstrip = strip;
  stack = (long *)malloc((int)(sizeof(long)*(numtri+3)));
  stacktop = stack;
  for(i=0;i<numtri;i++) {
    if (tri[i].used)
      continue;
    for(j=0;j<3;j++) {
      rec[j].t = i;
      rec[j].v[0] = tri[i].v[j];
      rec[j].v[1] = tri[i].v[(j+2)%3];
      rec[j].v[2] = tri[i].v[(j+1)%3];
      rec[j].parity = TRUE;
    }
    for(j=0;j<3;j++) {
      rec[j+3].t = i;
      rec[j+3].v[0] = tri[i].v[j];
      rec[j+3].v[1] = tri[i].v[(j+1)%3];
      rec[j+3].v[2] = tri[i].v[(j+2)%3];
      rec[j+3].parity = FALSE;
    }
    maxlen = -1;
    beststrip = 0;
    for(j=0;j<6;j++) {
      MakeStackStrip(rec[j],&rec[j]);
      ClearStackStrip(FALSE);
      MakeStackStrip(rec[j],NULL);
      len = stacktop - stack - 3;
      ClearStackStrip(FALSE);
      if (len > maxlen) {
	beststrip = j;
	maxlen = len;
      }
    }
    if (maxlen > 1) {
      MakeStackStrip(rec[beststrip],NULL);
      ClearStackStrip(TRUE);
    }
  }
}

long tot_tri = 0;
long tot_strip = 0;
long tot_tri_in_strip = 0;

int tri_callback(mdlInput& in, mdlOutput& out, 
		 mdlKey /*key*/, mdlUserParam /*user_data*/)
{
  TriData data;
  data.Read(in);
  if (data.numtri > 0) {
    data.MakeStrips();
    data.Write(out);
    tot_tri += data.numtri;
    tot_strip += data.numstrip;
    tot_tri_in_strip += data.tris_in_strips;
  }
  return TRUE;
}

void usage(void)
{
  cout << "trianglestrip [-atph] inputfile outputfile\n";
  cout << " -a              create ASCII outputfile\n";
  cout << " -p              use stdin and stdout for i/o\n";
  cout << " -t <filename>   specify a file of chunk types\n";
  cout << " -h              view this help message\n";
  cout << "tries to turn triangles into triangle strips.\n";
  exit(0);
}

int main(int argc, char **argv)
{
  char infile[128];
  char outfile[128];
  int use_binary = TRUE;
  int use_pipeio = FALSE;
  FILE *inf;
  FILE *outf;
  char c;

  while((c = getopt(argc, argv, "hapt:")) != (char)-1)
    switch (c) {
    case 'h':
      usage();
      break;
    case 'a':
      use_binary = FALSE;
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
//    cerr << myspeclist.Find(mdlKey("customjk")) << "\n";
    mdlInput inmodel(inf,TRUE,&myspeclist);
    mdlOutput outmodel(outf,use_binary,TRUE,mdlSame,&myspeclist);
    inmodel.CopyData(outmodel,&mycallbacks,NULL);
    myhistory.AppendCommandLine(argc, argv);
    long lone = tot_tri - tot_tri_in_strip;
    {
      char strbuf[512];
      
      if (tot_tri > 0)
	sprintf(strbuf,"average strip length %g",
		(tot_tri_in_strip+lone)/(float)(tot_strip + lone));
      else
	sprintf(strbuf,"no triangles found");
      myhistory.Append(strbuf);
    }
    myhistory.Write(outmodel);
    if (tot_tri > 0) {
      cerr << "found "<<tot_tri<<" triangles\n";
      cerr << "put "<<tot_tri_in_strip<<" triangles into "
	<<tot_strip<<" strips\n";
      cerr << "average strip length "
	<<(tot_tri_in_strip+lone)/(float)(tot_strip + lone)<<"\n";
    }
    else
      cerr << "no triangle found\n";
  }
  fclose(inf);
  fclose(outf);
}



