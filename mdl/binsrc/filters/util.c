// utility file of routines

int ParseTypeSpecFile(const char *filename, mdlChunkSpecList *list)
{
  FILE *f = fopen(filename,"rb");
  if (f == NULL) {
    cout << "could not open type spec file: "<<filename<<"\n";
    return -1;
  }
  {
    mdlInput in(f);
    mdlKey key;
    char name[32];  /*names limited to 9 characters*/
    char type[256];
    char *str;
    mdlChunkSpec spec[2] = {
      {name, type},
      {NULL, NULL}
    };
    while(in.NumRemain()) {
      key = in.BeginChunk();
      if (key == mdlKey("chnkTyp")) {
	str = in.ReadString();
	strcpy(name,str);
	str = in.ReadString();
	strcpy(type,str);
	list->Append(spec);
      }
      in.EndChunk();
    }
  }
  fclose(f);
  return 0;
}

class FileHistory {
  char **strings;
  int numstrings;

 public:
  FileHistory() { numstrings = 0; strings = NULL; }
  ~FileHistory() { for(int i=0;i<numstrings;i++) 
		     free(strings[i]);
		   free(strings); }
  void Read(mdlInput &in, const mdlKey &key) {
    if (key != mdlKey("flHstry")) return;
    while(in.NumRemain()) {
      char *str = in.ReadString();
      numstrings++;
      strings = (char **)realloc(strings,numstrings*sizeof(strings[0]));
      assert(strings);
      char *cpy = (char *)malloc((strlen(str)+1)*sizeof(char));
      assert(cpy);
      strcpy(cpy,str);
      strings[numstrings-1] = cpy;
    }
  }
  void Write(mdlOutput &out) {
    out.BeginChunk(mdlKey("flHstry"));
    for(int i=0;i<numstrings;i++)
      out.WriteString(strings[i]);
    out.EndChunk();
  }
  void Print() {
    for(int i=0;i<numstrings;i++)
      cout << strings[i]<<'\n';
  }
  void Append(char *newstr) {
    numstrings++;
    strings = (char **)realloc(strings,numstrings*sizeof(strings[0]));
    assert(strings);
    char *cpy = (char *)malloc((strlen(newstr)+1)*sizeof(char));
    assert(cpy);
    strcpy(cpy,newstr);
    strings[numstrings-1] = cpy;
  }
  void AppendCommandLine(int argc, char **argv) {
    char buf[5120];
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
    assert(cur-buf < 5120);
    Append(buf);
  }
};

    
