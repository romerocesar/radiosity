#ifndef _H_PRINTTOKEN
#define _H_PRINTTOKEN

#include <stdio.h>
#include <stdlib.h>
#include "mdl.H"
#include "mdlScan.H"

class LineBuffer {
  char *buf;
  int length;
  int max_size;
  int breakpt;
  int indentsize;
  char temp;

 public:
  LineBuffer() 
    { buf = NULL; max_size = 0; length = 0; SetSize(512); 
      breakpt = NULL; indentsize = 2; temp = 0; }
  ~LineBuffer() { if (buf) free(buf); }
  void Clear() { buf[0] = 0; length = 0; breakpt = NULL; temp = 0; }
  void Indent(int level);
  void SetBreakPoint() { if (temp == 0) breakpt = length; }
  int IsBreakPoint() { return ((breakpt > 0)&&(breakpt<length)); }
  void Break();
  void Restore();
  void RestoreAfterBreak();
  void Append(char c);
  void Append(const char *str);
  void AppendFloat(float f);
  void AppendInt(long i);
  void AppendString(const char *str);
  void AppendKey(const mdlKey &key);
  int Length() { return length; }
  const char *Contents() { return buf; }
 private:
  void CheckRoom(int check) 
    { if ((max_size - length) <= check) 
	SetSize((check<max_size)?(2*max_size):(max_size+check+1)); }
  void SetSize(int size);
};

class mdlTokenIO {
  const char *type;
  const char *curtype,*repeatpt;

 public:
  mdlTokenIO() { type = curtype = repeatpt = NULL; }
  ~mdlTokenIO() {}
  
  void SetChunkType(const mdlKey &key);
  void AddToken(LineBuffer *buf, mdlScan *scan, int type);
  void SkipChunk(mdlScan *scan);
  int IsRepeatPoint() 
    { return (type && repeatpt && (curtype == repeatpt)); }
 private:
  void NextType();
  void ReportTokenError(mdlScan *scan, int tokentype);
};

#endif

