c#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include "mdl.H"
#include "mdlScan.H"


#define MIN(a,b) (((a) < (b)) ? (a) : (b))


const int NUM_OLD_WARNINGS = 10;



static const char *gExecName;


static int my_fwrite(FILE *outFi, const void *data, size_t size) {
   if (1 > fwrite(data, size * 4, 1, outFi)) {
      fprintf(stderr, "%s error: Error writing to file", gExecName);
      perror(gExecName);
      return 1;
      }
   return 0;
   }


static int my_fseek(FILE *outFi, long offset) {
   if (0 > fseek(outFi, offset, SEEK_SET)) {
      fprintf(stderr, "%s error: Error in fseek(%d)\n", gExecName, offset);
      if (errno == ESPIPE)
	 fprintf(stderr, "%s error:  -- input must not be a pipe\n",
		 gExecName);
      else
	 perror(gExecName);
      return 1;
      }
   return 0;
   }




int CompileChunk(FILE *outFi, mdlScan &scanner, mdlKey key,
		 int checkTypes, const mdlChunkSpecList *specs,
		 int &typeError, int &oldWarnings, int &badError) {
   int length;
   long sizePos;
   long savePos;
   int t, i;
   void *data;
   long iData;
   float fData;
   char *sData = NULL;
   mdlKey keyData;
   int size;
   int n, len;
   const char *typeSpec, *nextType, *repeatPt;
   char realType;

   if (checkTypes) {
       if (specs)
	  typeSpec = mdlFindChunkSpec(key, *specs);
       else
	  typeSpec = mdlFindChunkSpec(key);

       nextType = typeSpec;
       repeatPt = 0;

       if (!typeSpec) {
	  fprintf(stderr, "%s warning: No type info for key '%.*s'\n",
		  gExecName, 8, (char *) key);
	  }
       }
   else 
       typeSpec = 0;

   length = 0;
   sizePos = ftell(outFi);
   if (my_fwrite(outFi, &length, 1)) {
      badError = 1;
      return 0;
      }

   for (;;) {

      t = scanner.NextToken();

      switch (t) {
       case MDL_TOKEN_INT:
	 iData = scanner.GetInt();
	 data = (void *) &iData;
	 size = 1;
	 realType = 'i';
	 break;
	 
       case MDL_TOKEN_FLOAT:
	 fData = scanner.GetFloat();
	 data = (void *) &fData;
	 size = 1;
	 realType = 'f';
	 break;
	 
       case MDL_TOKEN_STRING:
	 len = strlen(scanner.GetString());
	 size = len / 4 + 1;
	 n = size * 4;
	 if (sData) free(sData);
	 sData = (char *) malloc(n);
	 strncpy(sData, scanner.GetString(), len);
	 for (i = len; i < n; i++)
	    sData[i] = '\0';
	 data = (void *) sData;
	 realType = 's';
	 break;

       case MDL_TOKEN_KEYWORD:
	 keyData = scanner.GetKey();
	 data = (void *) (char *) keyData;
	 size = 2;
	 break;

       case 0:
	 fprintf(stderr, "%s error: Premature end-of-file (missing end)\n",
		 gExecName);
	 badError = 1;
	 return 0;

       default:
	 fprintf(stderr, "%s error: line %d: Syntax error\n", gExecName,
		 scanner.CurrentLineNumber());
	 badError = 1;
	 return 0;
	 }


      if (t == MDL_TOKEN_KEYWORD) {

	 int isEnd = (keyData == endChunkKey);
	 if (!isEnd && (keyData == endLstKey || keyData == endObjctKey)) {
	    isEnd = 1;
	    if (oldWarnings < NUM_OLD_WARNINGS) {
	       fprintf(stderr, "%s warning: line %d: Old-style end keyword\n",
		       gExecName, scanner.CurrentLineNumber());
	       if (++oldWarnings >= NUM_OLD_WARNINGS)
		  fprintf(stderr, "%s info: Ignoring subsequent old-style "
			  "end keywords.\n", gExecName);
	       }
	    }

	 if (isEnd) {
	    savePos = ftell(outFi);
	    if (my_fseek(outFi, sizePos)) {
	       badError = 1;
	       return 0;
	       }
	    if (my_fwrite(outFi, &length, 1)) {
	       badError = 1;
	       return 0;
	       }
	    if (my_fseek(outFi, savePos)) {
	       badError = 1;
	       return 0;
	       }
	    if (typeSpec) {
	       if (*nextType != 0 && *nextType != ',') {
		  fprintf(stderr, "%s error: line %d: Type conflict in "
			  "'%.*s' -- missing data %s\n", gExecName,
			  scanner.CurrentLineNumber(), 8,
			  (const char *) key, nextType);
		  typeError = 1;
		  }
	       }
	    return length + 1;
	    }

	 else {
	    realType = 'C';
	    }
	 }


      if (typeSpec) {

	 while(*nextType == ',') {
	    repeatPt = nextType;
	    nextType++;
	    }

	 if (*nextType != realType) {
	    if (*nextType != '\0') {
	       fprintf(stderr,
		       "%s error: line %d: Type conflict in '%.*s' "
		       "-- expected %c; got %c\n", gExecName,
		       scanner.CurrentLineNumber(), 8, (const char *) key,
		       *nextType, realType);
	       typeError = 1;
	       }
	    else {
	       fprintf(stderr,
		       "%s error: line %d: Type conflict in '%.*s' "
		       "-- extra data of type %c\n", gExecName,
		       scanner.CurrentLineNumber(), 8, (const char *) key,
		       realType);
	       typeError = 1;
	       }
	    }

	 nextType++;
	 if (*nextType == '\0' && repeatPt) {
	    nextType = repeatPt;
	    }
	 }

      if (my_fwrite(outFi, data, size)) {
	 badError = 1;
	 return 0;
	 }
      length += size;

      if (sData) {
	 delete [] sData;
	 sData = 0;
	 }

      if (t == MDL_TOKEN_KEYWORD)
	 length += CompileChunk(outFi, scanner, keyData, checkTypes, specs,
				typeError, oldWarnings, badError);

      if (badError)
	 return 0;
      }

   }





mdlStatus mdlCompile(const char *execName, FILE *inFi, FILE *outFi,
		     int checkTypes, const mdlChunkSpecList *specs) {
   int t;
   void *data;
   int size;
   int typeError = 0;
   int badError = 0;

   gExecName = execName;

   int oldWarnings = 0;


   /* verify output is seekable */
   
   if (my_fseek(outFi, 0))
      return mdlStatus::STAT_BAD_STREAM;
   

   /* Write file ID */

   if (my_fwrite(outFi, (const char *)binaryMagic, 2))
      return mdlStatus::STAT_BAD_STREAM;
   

   mdlScan scanner(inFi);

   while ((t = scanner.NextToken()) != MDL_TOKEN_EOF && !badError) {

      if (t != MDL_TOKEN_KEYWORD) {
	 fprintf(stderr,
		 "%s error: line %d: Data (%s) at top level\n",
		 gExecName, scanner.CurrentLineNumber(), 
		 scanner.TokenName(t));
	 return 1;
	 }

      mdlKey key = scanner.GetKey();
      data = (void *) (char *) key;
      size = 2;
      
      if (key == endChunkKey) {
	 fprintf(stderr,
		 "%s error: line %d: Extra 'end' at top level\n",
		 gExecName, scanner.CurrentLineNumber());
	 return 1;
	 }

      if (my_fwrite(outFi, data, size))
	 return mdlStatus::STAT_BAD_STREAM;
      
      CompileChunk(outFi, scanner, key, checkTypes, specs, typeError,
		   oldWarnings, badError);
      }

   if (badError)
      return mdlStatus::STAT_BAD_STREAM;
   if (typeError)
      return mdlStatus::STAT_TYPE_ERROR;

   return mdlStatus::STAT_OK;      
   }

