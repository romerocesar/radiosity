
#include "mdl.H"
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

static mdlChunkSpec oldmdltypes[] = {
  {"msh     ", "ss,C"},
  {"mtrl    ", "s,C"},
  {"cmr     ", "s,C"},
  {"imgFlUV ", "sss,C"},
  {"bckgrnd ", ",C"},

  {"vrtxPstn", ",fff"},
  {"vrtxNrml", ",fff"},
  {"vrtxUV  ", ",ff"},
  {"vrtxIrrd", ",fff"},

  {"trngl   ", ",iii"},
  {"qdrltrl ", ",iiii"},
  {"trnglStr", "iii,i"},
  {"ptchIrrd", ",fff"},

  {"lmbrtn  ", "fff,s"},
  {"mtl     ", "ffff,s"},
  {"dlctrc  ", "fffff"},
  {"plshd   ", "fffff,s"},
  {"uth     ", "ffffffffff,s"},

  {"cmnt    ", ",s"},
  {"pnhl    ", "ffffffffff"},
  {"phngLmnr", "ffff,s"},
  {"avrg    ", "f,f"},
  {"cnstnt  ", "f,f"},
  {NULL,NULL}
};

static mdlChunkSpecList oldspeclist(oldmdltypes);

int conv_old_msh(mdlInput &, mdlOutput &, mdlKey, mdlUserParam);
int conv_old_mtrl(mdlInput &, mdlOutput &, mdlKey, mdlUserParam);
int conv_old_cmr(mdlInput &, mdlOutput &, mdlKey, mdlUserParam);
int conv_old_imgfluv(mdlInput &, mdlOutput &, mdlKey, mdlUserParam);
int conv_old_bckgrnd(mdlInput &, mdlOutput &, mdlKey, mdlUserParam);

static mdlCopyCallbackSpec mycallbackspecs[] = {
  {"msh",conv_old_msh},
  {"mtrl",conv_old_mtrl},
  {"cmr",conv_old_cmr},
  {"imgFlUV",conv_old_imgfluv},
  {"bckgrnd",conv_old_bckgrnd},
  {NULL,NULL}
};

mdlCopyCallbackList mycallbacks(mycallbackspecs);

int conv_old_msh(mdlInput &in, mdlOutput &out, mdlKey key, void */*data*/)
{
  out.BeginChunk(key);
  out.WriteString(in.ReadString());
  out.BeginChunk(mdlKey("mtrlNm"));
  out.WriteString(in.ReadString());
  out.EndChunk();
  in.CopyData(out,&mycallbacks,NULL);
  out.EndChunk();
  return TRUE;
}

void write_txtrmp(mdlOutput &out, char *name)
{
  out.BeginChunk(mdlKey("txtrMp"));
  out.WriteString("vrtxUV");
  out.BeginChunk(mdlKey("txtrNm"));
  out.WriteString(name);
  out.EndChunk();
  out.EndChunk();
}
    
int conv_old_lmbrtn(mdlInput &in, mdlOutput &out, mdlKey /*key*/, void */*data*/)
{
  float rgb[3];
  char *tname = NULL;
  
  out.BeginChunk(mdlKey("lmbrtn"));
  in.ReadFloats(rgb,3);
  if (in.NumRemain()) 
    tname = in.ReadString();
  if (tname && *tname)
    write_txtrmp(out,tname);
  else
    out.WriteFloatChunk(mdlKey("rgb"),rgb,3);
  out.EndChunk();
  return TRUE;
}

int conv_old_mtl(mdlInput &in, mdlOutput &out, mdlKey /*key*/, void */*data*/)
{
  float rgb[3], pexponent;
  char *tname = NULL;
  char *ename = NULL;

  out.BeginChunk(mdlKey("pMtl"));
  in.ReadFloats(rgb,3);
  pexponent = in.ReadFloat();
  if (in.NumRemain()) 
    tname = in.ReadString();
  if (in.NumRemain())
    ename = in.ReadString();
  if (tname && *tname)
    write_txtrmp(out,tname);
  else
    out.WriteFloatChunk(mdlKey("rgb"),rgb,3);
  if (ename && *ename)
    write_txtrmp(out,ename);
  else
    out.WriteFloatChunk(mdlKey("sclr"),&pexponent,1);
  out.EndChunk();
  return TRUE;
}

int conv_old_dlctrc(mdlInput &in, mdlOutput &out, mdlKey /*key*/, void */*data*/)
{
  float rgb[3], pexponent, ior;

  out.BeginChunk(mdlKey("pDlctrc"));
  in.ReadFloats(rgb,3);
  ior = in.ReadFloat();
  pexponent = in.ReadFloat();
  out.WriteFloatChunk(mdlKey("rgb"),rgb,3);
  out.WriteFloatChunk(mdlKey("sclr"),&ior,1);
  out.WriteFloatChunk(mdlKey("sclr"),&pexponent,1);
  out.EndChunk();
  return TRUE;
}

int conv_old_plshd(mdlInput &/*in*/, mdlOutput &/*out*/, mdlKey /*key*/, void */*data*/)
{
  cerr << "polished material converter not yet implemented\n";
  assert(FALSE);
  return TRUE;
}

int conv_old_uth(mdlInput &/*in*/, mdlOutput &/*out*/, mdlKey /*key*/, void */*data*/)
{
  cerr << "utah material converter not yet implemented\n";
  assert(FALSE);
  return TRUE;
}

int conv_old_reflective_material(mdlInput &in, mdlOutput &out, 
				 mdlKey key, void */*data*/)
{
  if (key == mdlKey("lmbrtn"))
    conv_old_lmbrtn(in,out,key,NULL);
  else if (key == mdlKey("mtl"))
    conv_old_mtl(in,out,key,NULL);
  else if (key == mdlKey("dlctrc"))
    conv_old_dlctrc(in,out,key,NULL);
  else if (key == mdlKey("plshd"))
    conv_old_plshd(in,out,key,NULL);
  else if (key == mdlKey("uth"))
    conv_old_uth(in,out,key,NULL);
  else {
    cerr << "unrecognized material: "<<(char *)key<<"\n";
    assert(FALSE);
  }
  return TRUE;
}

int conv_old_phnglmnr(mdlInput &in, mdlOutput &out, mdlKey key, void */*data*/)
{
  float rgb[3], pexponent;
  char *tname = NULL;
  char *ename = NULL;

  out.BeginChunk(mdlKey("pLmnr"));
  in.ReadFloats(rgb,3);
  pexponent = in.ReadFloat();
  if (in.NumRemain()) 
    tname = in.ReadString();
  if (in.NumRemain())
    ename = in.ReadString();
  in.EndChunk();
  key = in.BeginChunk();
  conv_old_reflective_material(in,out,key,NULL);
  if (tname && *tname)
    write_txtrmp(out,tname);
  else
    out.WriteFloatChunk(mdlKey("rgb"),rgb,3);
  if (ename && *ename)
    write_txtrmp(out,ename);
  else
    out.WriteFloatChunk(mdlKey("sclr"),&pexponent,1);
  out.EndChunk();
  return TRUE;
}

int conv_old_mtrl(mdlInput &in, mdlOutput &out, mdlKey key, void */*data*/)
{
  out.BeginChunk(mdlKey("nmdMtrl"));
  out.WriteString(in.ReadString());
  key = in.BeginChunk();
  if (key == mdlKey("phngLmnr"))
    conv_old_phnglmnr(in,out,key,NULL);
  else
    conv_old_reflective_material(in,out,key,NULL);
  in.EndChunk();
  out.EndChunk();
  return TRUE;
}

int conv_old_cmr(mdlInput &in, mdlOutput &out, mdlKey key, void */*data*/)
{
  float from[3], at[3], up[3], angle;

  out.BeginChunk(mdlKey("cmr"));
  out.WriteString(in.ReadString());
  key = in.BeginChunk();
  assert(key==mdlKey("pnhl"));
  in.ReadFloats(from,3);
  in.ReadFloats(at,3);
  in.ReadFloats(up,3);
  angle = in.ReadFloat();
  in.EndChunk();
  out.WriteFloats(from,3); /* from point */
  at[0] -= from[0];
  at[1] -= from[1];
  at[2] -= from[2];
  out.WriteFloats(at,3); /* at vector */
  out.WriteFloats(up,3); /* up vector */
  out.WriteFloat(1.0);   /* image plane distance */
  angle = tan(angle);
  out.WriteFloat(angle); /* image width */
  out.WriteFloat(angle); /* image height */
  out.WriteFloat(0);
  out.WriteFloat(0);     /* image center (0,0) */
  out.WriteFloat(0);     /* time at which to take image */
  out.EndChunk();
  return TRUE;
}

int conv_old_imgfluv(mdlInput &in, mdlOutput &out, mdlKey /*key*/, void */*data*/)
{
  out.BeginChunk(mdlKey("nmdTxtr"));
  out.WriteString(in.ReadString());
  out.BeginChunk(mdlKey("imgFl"));
  out.WriteString(in.ReadString());
  out.WriteString(in.ReadString());
  out.EndChunk();
  out.EndChunk();
  return TRUE;
}

int conv_old_bckgrnd(mdlInput &/*in*/, mdlOutput &/*out*/, mdlKey /*key*/, void */*data*/)
{
  cerr << "background radiance converter not yet implemented\n";
  assert(FALSE);
  return TRUE;
}

void usage(void)
{
  cout << "oldmdl2mdl [-aph] inputfile outputfile\n";
  cout << " -a              create ASCII outputfile\n";
  cout << " -p              use stdin and stdout for i/o\n";
  cout << " -h              view this help message\n";
  cout << "converts a mdl version 1.0 file to a mdl version 2.0 file\n";
  cout << "You can safely ignore the Old-style file warnings.\n";
  exit(0);
}

void writecommandline(mdlOutput &out, int argc, char **argv)
{
  char buf[512];
  char *cur, *from;
  int i;

  cur = buf;
  for(i=0;i<argc;i++) {
    if (i > 0)
      *cur++ = ' ';
    from = argv[i];
    while(*from) 
      *cur++ = *from++;
  }
  *cur = 0;
  assert(cur-buf < 512);
  out.WriteString(buf);
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

  while((c = getopt(argc, argv, "hap")) != (char)-1)
    switch (c) {
    case 'h':
      usage();
      break;
    case 'a':
      use_binary = FALSE;
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
    mdlInput inmodel(inf,TRUE,&oldspeclist);
    mdlOutput outmodel(outf,use_binary,TRUE);
    inmodel.CopyData(outmodel,&mycallbacks,NULL);
    outmodel.BeginChunk(mdlKey("flHstry"));
    writecommandline(outmodel,argc,argv);
    outmodel.EndChunk();
  }
  fclose(inf);
  fclose(outf);
}









