#ifndef _H_MDLSCAN
#define _H_MDLSCAN

#include <mdl.H>
#include <stdlib.h>

#define MDL_TOKEN_EOF      0
#define MDL_TOKEN_INT      1
#define MDL_TOKEN_FLOAT    2
#define MDL_TOKEN_KEYWORD  3
#define MDL_TOKEN_STRING   4
#define MDL_TOKEN_PERCENT_COMMENT 5
#define MDL_TOKEN_BRACKET_COMMENT 6
#define MDL_TOKEN_COMMENTERREOF 31
#define MDL_TOKEN_STRERRNL 32
#define MDL_TOKEN_JUNK     33

class mdlScanBookmark {
  friend class mdlScan;
private:
  long filepos;
  int linenum;
  char curchar;
  char prevchar;
};

class mdlScan {
  int token_max_size;
  char *token_buf;
  int token_length;
  int line_num;
  int token_type;
  int token_on_newline;
  FILE *f;
  int preserve_comments;
  char curchar;
  char prevchar;

 public:
  mdlScan(FILE *file, int report_comments = FALSE);
  ~mdlScan() { free(token_buf); }

  int NextToken(void);

  int GetInt(void) { return atoi(token_buf); }
  float GetFloat(void) { return atof(token_buf); }
  const char *GetString(void) { return token_buf; }
  mdlKey GetKey(void) { return mdlKey(token_buf); }
  
  int CurrentLineNumber() { return line_num; }

  /* return name as string for token type */
  static const char *TokenName(int type);  
  const char *CurrentTokenName() { return mdlScan::TokenName(token_type); }

  int TokenWasOnNewline() { return (prevchar == '\n'); }
  mdlScanBookmark GetBookmark(void);
  void GotoBookmark(const mdlScanBookmark &mark);

 private:  /*routines for internal use only!*/

  void SetBufSize(int size);
  void ResetToken(void) { token_length = 0; }
  void AddChar(void) { 
    if (token_length >= token_max_size)
      SetBufSize(2*token_max_size);
    token_buf[token_length++] = curchar;
  }
  void AddChar(char c) { 
    if (token_length >= token_max_size)
      SetBufSize(2*token_max_size);
    token_buf[token_length++] = c;
  }
  void TerminateToken() { token_buf[token_length] = 0; }
  char GetNextChar() {
    if (curchar == '\n')
      line_num++;
    prevchar = curchar;
    curchar = fgetc(f);
    return (curchar == (char)EOF);
  }
  void PullNumber(void);
  void PullString(void);
  void PullKeyword(void);
  void PullComment(void);
  void SkipPercentComment(void);
  int SkipBracketComment(void);
};

#endif //_H_MDLSCAN



