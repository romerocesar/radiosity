#include "printtoken.h"
#include <ctype.h>
#include <string.h>
#include <assert.h>

void LineBuffer::Indent(int level)
{
  char *first;
  int ilen = level*indentsize;
  int i;

  CheckRoom(ilen);
  for(first=buf;*first;first++) {
    if (*first != ' ')
      break;
    ilen--;
  }
  if (*first != 0) {
    length += ilen;
    if (ilen < 0) {
      memmove(buf,&buf[-ilen],length+ilen);
      return;
    }
    else if (ilen == 0)
      return;
    else 
      memmove(&buf[ilen],buf,length);
  }
  else {
    ilen = level*indentsize;
    length = ilen;
    buf[ilen] = 0;
  }
  for(i=0;i<ilen;i++)
    buf[i] = ' ';
}

void LineBuffer::Break()
{
  assert(IsBreakPoint());
  assert(temp==0);
  temp = buf[breakpt];
  assert(temp);
  buf[breakpt] = 0;
  length = breakpt;
}

void LineBuffer::Restore()
{
  assert(temp);
  buf[breakpt] = temp;
  temp = 0;
  length = strlen(buf);
}

void LineBuffer::RestoreAfterBreak()
{
  int len;

  assert(temp);
  buf[breakpt] = temp;
  temp = 0;
  len = strlen(&buf[breakpt]);
  memmove(buf,&buf[breakpt],len);
  length = len;
}

void LineBuffer::Append(char c)
{
  buf[length++] = c;
  if (length >= max_size)
    SetSize(max_size*2);
  buf[length] = 0;
}

void LineBuffer::Append(const char *str)
{
  int slen = strlen(str);
  CheckRoom(slen);
  strcat(buf,str);
  length += slen;
}

void LineBuffer::AppendInt(long i)
{
  CheckRoom(32);
  length += sprintf(&buf[length],"%li",i);
}

void LineBuffer::AppendFloat(float f)
{
  char flbuf[512];
  int flbuf_len;

  sprintf(flbuf,"%g",f);
  if (!strpbrk(flbuf,".eE"))
    strcat(flbuf,".0");
  flbuf_len = strlen(flbuf);
  CheckRoom(flbuf_len);
  strcpy(&buf[length],flbuf);
  length += flbuf_len;
}

void LineBuffer::AppendString(const char *string)
{
  Append('\"');
  Append(string);
  Append('\"');
}

void LineBuffer::AppendKey(const mdlKey &key)
{
  char keystr[16];
  
  CheckRoom(16);
  strcpy(keystr,(char *)key);
  for(int i=0;keystr[i];i++) {
    if (keystr[i] == ' ') {
      keystr[i] = 0;
      break;
    }
  }
  strcpy(&buf[length],keystr);
  length += strlen(keystr);
}

void LineBuffer::SetSize(int size)
{
  buf = (char *)realloc(buf,size*sizeof(buf[0]));
  if (!buf) {
    fprintf(stderr,"Error: out of memory in LineBuffer\n");
    exit(1);
  }
  max_size = size;
}

void mdlTokenIO::SetChunkType(const mdlKey &key)
{
  curtype = repeatpt = NULL;
  type = mdlFindChunkSpec(key);
  NextType();
}

void mdlTokenIO::AddToken(LineBuffer *buf, mdlScan *scan, int type)
{
  mdlKey nkey;

  switch(type) {
  case MDL_TOKEN_INT:
    if (curtype && (*curtype == 'f'))
      buf->AppendFloat(scan->GetFloat());
    else if (curtype && (*curtype != 'i')) 
      ReportTokenError(scan,type);
    else
      buf->AppendInt(scan->GetInt());
    NextType();
    break;
  case MDL_TOKEN_FLOAT:
    if (curtype && (*curtype != 'f')) 
      ReportTokenError(scan,type);
    else
      buf->AppendFloat(scan->GetFloat());
    NextType();
    break;
  case MDL_TOKEN_STRING:
    if (curtype && (*curtype != 's'))
      ReportTokenError(scan,type);
    else
      buf->AppendString(scan->GetString());
    NextType();
    break;
  case MDL_TOKEN_PERCENT_COMMENT:
    buf->Append('%');
    buf->Append(scan->GetString());
    break;
  case MDL_TOKEN_BRACKET_COMMENT:
    buf->Append('[');
    buf->Append(scan->GetString());
    buf->Append(']');
    break;
  case MDL_TOKEN_KEYWORD:
    nkey = scan->GetKey();
    if (nkey == endChunkKey) {
      if (curtype && (*curtype != 0) && (curtype != repeatpt)) {
	fprintf(stderr,"Error: unexpected end of chunk on line #%i\n",
		scan->CurrentLineNumber());
	exit(1);
      }
      repeatpt = NULL;
      curtype = "E";
    }
    else {
      if (curtype && (*curtype != 'C')) 
	ReportTokenError(scan,type);
      NextType();
    }
    buf->AppendKey(nkey);
    break;
  default:
    ReportTokenError(scan,type);
    break;
  }
}

void mdlTokenIO::SkipChunk(mdlScan *scan)
{
  if (curtype && (*curtype != 'C'))
    ReportTokenError(scan,MDL_TOKEN_KEYWORD);
  NextType();
}

void mdlTokenIO::NextType()
{
  if (curtype) {
    curtype++;
    if (repeatpt && (*curtype == 0))
      curtype = repeatpt;
  }
  else if (type)
    curtype = type;
  if (curtype && (*curtype == ',')) {
    curtype++;
    repeatpt = curtype;
  }
}

void mdlTokenIO::ReportTokenError(mdlScan *scan, int tokentype)
{
  if (curtype) 
    fprintf(stderr,"Error wrong type: Expected %c, got %s on line #%i\n",
	    *curtype,mdlScan::TokenName(tokentype),
	    scan->CurrentLineNumber());
  else 
    fprintf(stderr,"Error bad token: Got %s on line #%i\n",
	  mdlScan::TokenName(tokentype),scan->CurrentLineNumber());
  exit(1);
}



