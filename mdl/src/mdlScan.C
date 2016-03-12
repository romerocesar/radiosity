#include "mdlScan.H"
#include <ctype.h>
#include <string.h>

static inline int CharInString(char c, char *str)
{
  return (c && strchr(str,c));
}

mdlScan::mdlScan(FILE *file, int report_comments)
{ 
  f = file; 
  line_num = 1;
  token_buf = NULL;
  preserve_comments = report_comments;
  SetBufSize(512); 
  ResetToken(); 
  curchar = ' ';
  prevchar = ' ';
  token_on_newline = FALSE;
  GetNextChar(); 
  token_type = MDL_TOKEN_JUNK;
}

void mdlScan::SetBufSize(int newsize)
{
  token_buf = (char *)realloc(token_buf,(newsize+1)*sizeof(token_buf[0]));
  if (token_buf == NULL) {
    cerr << "Error: out of memory in mdlScan.\n";
    exit(1);
  }
  token_max_size = newsize;
}

/* used if we want to enforce strict checking on the validity of numbers */
int StrictCheckNumber(char *str)
{
  int exp_found = FALSE;
  int dot_found = FALSE;
  int digit_found = FALSE;
  int sign_ok = TRUE;
  
  while(*str) {
    if ((*str == '+')||(*str == '-')) {
      if (!sign_ok) return -1;
      sign_ok = FALSE;
    }
    else if (*str == '.') {
      if (dot_found) return -1;
      dot_found = TRUE;
      sign_ok = FALSE;
    }
    else if ((*str == 'e')||(*str == 'E')) {
      if (exp_found) return -1;
      exp_found = TRUE;
      dot_found = TRUE;
      digit_found = FALSE;
      sign_ok = TRUE;
    }
    else if (isdigit(*str)) {
      digit_found = TRUE;
      sign_ok = FALSE;
    }
    else
      return -1;
    str++;
  }
  if (!digit_found)
    return -1;
  return 0;
}
      
void mdlScan::PullNumber(void)
{
  do {
    if (isdigit(curchar) || CharInString(curchar,"+-Ee."))
      AddChar();
    else
      break;
  } while(!GetNextChar());
  TerminateToken();
  if (StrictCheckNumber(token_buf))
    token_type = MDL_TOKEN_JUNK;
  else if (strpbrk(token_buf,".Ee"))
    token_type = MDL_TOKEN_FLOAT;
  else
    token_type = MDL_TOKEN_INT;
}

void mdlScan::PullString(void)
{
  while(!GetNextChar()) {
    if (curchar == '\"') {
      token_type = MDL_TOKEN_STRING;
      GetNextChar(); /* don't want next call to see ending " */
      break;
    }
    else if (curchar == '\n') {
      token_type = MDL_TOKEN_STRERRNL;
      break;
    }
    AddChar();
  }
  TerminateToken();
}

void mdlScan::PullKeyword(void)
{
  do {
    if (isspace(curchar))
      break;
    AddChar();
  } while(!GetNextChar());
  TerminateToken();
  if ((strlen(token_buf) <= 8)
      && (isalpha(*token_buf) || (*token_buf == '_')))
    token_type = MDL_TOKEN_KEYWORD;
  else
    token_type = MDL_TOKEN_JUNK;
}

void mdlScan::PullComment(void)
{
  if (curchar == '%') {
    token_type = MDL_TOKEN_PERCENT_COMMENT;
    while(!GetNextChar()) {
      if (curchar == '\n') 
	break;
      AddChar();
    }
  }
  else if (curchar == '[') {
    int nestlevel = 1;
    while(!GetNextChar()) {
      if (curchar == ']') {
	if (--nestlevel <= 0) {
	  GetNextChar();    /* don't want next call to see ] */
	  break;
	}
      }
      else if (curchar == '[') {
	nestlevel++;
      }
      AddChar();
    }
    if (nestlevel <= 0)
      token_type = MDL_TOKEN_BRACKET_COMMENT;
    else
      token_type = MDL_TOKEN_COMMENTERREOF;
  }
  else
    token_type = MDL_TOKEN_JUNK;
  TerminateToken();
}

void mdlScan::SkipPercentComment(void)
{
  while(!GetNextChar()) {
    if (curchar == '\n')
      break;
  }
}

int mdlScan::SkipBracketComment(void)
{
  int nestlevel = 1;
  while(!GetNextChar()) {
    if (curchar == ']') {
      if (--nestlevel <= 0)
	break;
    }
    else if (curchar == '[')
      nestlevel++;
  }
  GetNextChar();
  return (nestlevel > 0);
}

int mdlScan::NextToken(void)
{
  if (curchar == (char)EOF)
    return (token_type = MDL_TOKEN_EOF);
  ResetToken();
  for(;;) {
    if (curchar == '%') {
      if (preserve_comments)
	break;
      else
	SkipPercentComment();
    }
    else if (curchar == '[') {
      if (preserve_comments) 
	break;
      else if (SkipBracketComment())
	return (token_type = MDL_TOKEN_COMMENTERREOF);
    }
    else if (!isspace(curchar))
      break;
    if (GetNextChar())
      return (token_type = MDL_TOKEN_EOF);
  }
  token_on_newline = CharInString(prevchar,"\n\r");
  if (isdigit(curchar) || CharInString(curchar,"+-."))
    PullNumber();
  else if (curchar == '\"')
    PullString();
  else if ((curchar == '%')||(curchar == '['))
    PullComment();
  else
    PullKeyword();
  return token_type;
}

const char *mdlScan::TokenName(int type)
{
  switch (type) {
  case MDL_TOKEN_EOF:
    return "<end of input>";
  case MDL_TOKEN_INT:
    return "T_INT";
  case MDL_TOKEN_FLOAT:
    return "T_FLOAT";
  case MDL_TOKEN_KEYWORD:
    return "T_KEYWORD";
  case MDL_TOKEN_STRING:
    return "T_STRING";
  case MDL_TOKEN_PERCENT_COMMENT:
    return "T_PERCENT_COMMENT";
  case MDL_TOKEN_BRACKET_COMMENT:
    return "T_BRACKET_COMMENT";
  case MDL_TOKEN_COMMENTERREOF:
    return "T_UNEXPECTED_END_OF_FILE_IN_BRACKET_COMMENT";
  case MDL_TOKEN_STRERRNL:
    return "T_STRERR_NEWLINE";
  case MDL_TOKEN_JUNK:
    return "T_JUNK";
  }
  return "<bad token val>";
}

mdlScanBookmark mdlScan::GetBookmark(void)
{
  mdlScanBookmark mark;
  mark.filepos = ftell(f);
  mark.linenum = line_num;
  mark.curchar = curchar;
  mark.prevchar = prevchar;
  return mark;
}

void mdlScan::GotoBookmark(const mdlScanBookmark &mark)
{
  fseek(f,mark.filepos,SEEK_SET);
  line_num = mark.linenum;
  curchar = mark.curchar;
  prevchar = mark.prevchar;
  token_type = MDL_TOKEN_JUNK;
  token_on_newline = FALSE;
  ResetToken();
  TerminateToken();
}


