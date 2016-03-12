
#include "mdl.H"
#include "mdlScan.H"
#include "printtoken.h"

static int LINE_LENGTH = 78;
static int MAX_SINGLELINE_NESTING = 2;
static int LONG_FORMAT = FALSE;

int TryAsSingleLine(mdlScan *scan, LineBuffer *buf, mdlKey key, 
		    int maxlen, int maxnest)
{
  mdlTokenIO tio;
  mdlKey nkey;
  int level = 1;
  int type;

  if ((maxnest < 1) || (buf->Length() > maxlen))
    return TRUE;
  buf->AppendKey(key);
  tio.SetChunkType(key);
  while (level > 0) {
    buf->Append(' ');
    switch (type = scan->NextToken()) {
    case MDL_TOKEN_PERCENT_COMMENT:
    case MDL_TOKEN_BRACKET_COMMENT:
      return TRUE;
    case MDL_TOKEN_KEYWORD:
      nkey = scan->GetKey();
      if (nkey == endChunkKey) {
	level--;
	tio.AddToken(buf,scan,type);
      }
      else {
	tio.SkipChunk(scan);
	if (TryAsSingleLine(scan,buf,nkey,maxlen,maxnest-1))
	  return TRUE;
      }
      break;
    default:
      tio.AddToken(buf,scan,type);
      break;
    }
    if (buf->Length() > maxlen)
      return TRUE;
  }
  return FALSE;
}

void PrintMultiLine(mdlScan *scan, FILE *outf, LineBuffer *buf, 
		    mdlKey key, int nestlevel)
{
  mdlTokenIO tio;
  mdlKey nkey;
  int isnewline;
  int breakbefore;
  int breakafter;
  LineBuffer nextbuf;
  int level;
  int type;
  mdlScanBookmark place;

  buf->Clear();
  buf->Indent(nestlevel);
  buf->AppendKey(key);
  isnewline = FALSE;
  tio.SetChunkType(key);
  if (tio.IsRepeatPoint()) {
    if (LONG_FORMAT) {
      fprintf(outf,"%s\n",buf->Contents());
      buf->Clear();
      isnewline = TRUE;
    }
    else
      buf->SetBreakPoint();
  }
  level = nestlevel + 1;
  while(level > nestlevel) {
    breakbefore = breakafter = FALSE;
    nextbuf.Clear();
    switch (type = scan->NextToken()) {
    case MDL_TOKEN_KEYWORD:
      nkey = scan->GetKey();
      if (nkey == endChunkKey) {
	tio.AddToken(&nextbuf,scan,type);
	breakbefore = TRUE;
	breakafter = TRUE;
	level--;
      }
      else {
	tio.SkipChunk(scan);
	place = scan->GetBookmark();
	if ((!TryAsSingleLine(scan,&nextbuf,nkey,
			      LINE_LENGTH,MAX_SINGLELINE_NESTING))
	    && (nextbuf.Length() + buf->Length() < LINE_LENGTH)) {
	  breakbefore = TRUE;
	  breakafter = TRUE;
	}
	else {
	  nextbuf.Clear();
	  scan->GotoBookmark(place);
	  if (!isnewline) {
	    fprintf(outf,"%s\n",buf->Contents());
	    buf->Clear();
	    buf->Indent(level);
	    isnewline = TRUE;
	  }
	  PrintMultiLine(scan,outf,buf,nkey,level);
	}
      }
      break;
    case MDL_TOKEN_PERCENT_COMMENT:
      breakafter=TRUE;   /*intentional fallthrough*/
    default:
      tio.AddToken(&nextbuf,scan,type);
      break;
    }
    if ((buf->Length() + nextbuf.Length() >= LINE_LENGTH)
	&& buf->IsBreakPoint() ) {
      buf->Break();
      fprintf(outf,"%s\n",buf->Contents());
      buf->RestoreAfterBreak();
      if (*buf->Contents != 0)
	buf->Indent(level);
      else 
	isnewline = TRUE;
    }
    if (buf->Length() + nextbuf.Length() >= LINE_LENGTH)
      breakbefore = TRUE;
    if (breakbefore && !isnewline) {
      fprintf(outf,"%s\n",buf->Contents());
      buf->Clear();
      isnewline = TRUE;
    }
    if (nextbuf.Length() > 0) {
      if (isnewline)
	buf->Indent(level);
      else
	buf->Append(' ');
      buf->Append(nextbuf.Contents());
      isnewline = FALSE;
      if (tio.IsRepeatPoint()) {
	if (LONG_FORMAT)
	  breakafter = TRUE;
	else
	  buf->SetBreakPoint();
      }
    }
    if (breakafter && !isnewline) {
      fprintf(outf,"%s\n",buf->Contents());
      buf->Clear();
      isnewline = TRUE;
    }
  }
}    

void prettyprint(mdlScan *scan, FILE *outf)
{
  int type;
  LineBuffer buf;
  mdlScanBookmark place;
  mdlKey nkey;

  if ((scan->NextToken() != MDL_TOKEN_KEYWORD)
       ||(scan->GetKey() != asciiMagic)) {
    fprintf(stderr,"This file is not an mdl ASCII file\n");
    exit(1);
  }
  fprintf(outf,"%s\n",(char *)asciiMagic);
  while((type = scan->NextToken()) != MDL_TOKEN_EOF) {
    switch (type) {
    case MDL_TOKEN_PERCENT_COMMENT:
      fprintf(outf,"%%%s\n",scan->GetString());
      break;
    case MDL_TOKEN_BRACKET_COMMENT:
      fprintf(outf,"[%s]\n",scan->GetString());
      break;
    case MDL_TOKEN_KEYWORD:
      nkey = scan->GetKey();
      place = scan->GetBookmark();
      buf.Clear();
      if ((!TryAsSingleLine(scan,&buf,nkey,
			    LINE_LENGTH,MAX_SINGLELINE_NESTING))
	&& (buf.Length() < LINE_LENGTH))
	  fprintf(outf,"%s\n",buf.Contents());
      else {
	scan->GotoBookmark(place);
	PrintMultiLine(scan,outf,&buf,nkey,0);
      }
      break;
    default:
      fprintf(stderr,"Error: unexpected token %s\n",scan->GetString());
      break;
    }
  }
}

void usage(void)
{
  cout << "mdlprettyprint [-hl] inputfile outputfile\n";
  cout << " -l              use longer format\n";
  cout << " -h              view this help message\n";
  cout << "performs pretty printing on an ASCII mdl file.\n";
  exit(0);
}

int main(int argc, char **argv)
{
  char infile[128];
  char outfile[128];
  FILE *inf;
  FILE *outf;
  char c;

  while((c = getopt(argc, argv, "hl")) != (char)-1)
    switch (c) {
    case 'h':
      usage();
      break;
    case 'l':
      MAX_SINGLELINE_NESTING = 1;
      LONG_FORMAT = TRUE;
      break;
    default:
      cout << "invalid option " << c << endl;
      usage();
    }
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
  inf = fopen(infile,"r");
  if (inf == NULL) {
    cout << "could not open input file: "<<infile<<"\n";
    exit(1);
  }
  outf = fopen(outfile,"w");
  if (outf == NULL) {
    cout << "could not open output file: "<<outfile<<"\n";
    exit(1);
  }
  {
    mdlScan scanner(inf,TRUE);

    prettyprint(&scanner,outf);
  }
  fclose(inf);
  fclose(outf);
}
