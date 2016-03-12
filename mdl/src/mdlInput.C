#include "mdl.H"

#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>


static void ErrMsg(const char *format, ...) {
   va_list ap;

   fprintf(stderr, "mdlInput: ");

   va_start(ap, format);
   vfprintf(stderr, format, ap);
   va_end(ap);

   fprintf(stderr, "\n");
   }



static inline int min(int a, int b) { return (a < b) ? a : b; }

// --- Methods of mdlInput ----------------------------------------------


mdlInput::mdlInput(FILE *inFi_, int checkTypes_,
		   const mdlChunkSpecList *specs_) {

   int e;
   char magicBuf[8];

   inFi = inFi_;
   checkTypes = checkTypes_;
   specs = specs_;

   stringData = NULL;
   stringDataSize = 0;

   if (2 > (e = fread(magicBuf, 4, 2, inFi))) {
      ErrMsg("File is not a valid mdl file (error reading ID)");
      if (e < 0) {
	 ErrMsg("  System error in fread (ID).");
	 perror("    error was");
	 }
      ErrMsg("  Incomplete read (ID). (%d < %d)", e, 8);
      inFi = NULL;
      return;
      }

   mdlKey fileMagic(magicBuf);
   if (fileMagic == asciiMagic || fileMagic == oldAsciiMagic) {
      if (fileMagic == oldAsciiMagic)
	 cerr << "mdlInput warning: This is an old-format file." << endl;
      binary = FALSE;
      FILE *binaryFile = tmpfile();
      stat = mdlCompile("mdlCompile", inFi, binaryFile,
			checkTypes, specs);
      if (stat != mdlStatus::STAT_OK) {
	 inFi = NULL;
	 return;
	 }
      inFi = binaryFile;
      fseek(inFi, 8, SEEK_SET);
      fileByteOrder = mdlSame;
      }
   else if (fileMagic == binaryMagic || fileMagic == oldBinaryMagic ||
	    fileMagic == swappedBinaryMagic) {
      if (fileMagic == oldBinaryMagic)
	 cerr << "mdlInput warning: This is an old-format file." << endl;
      if ((*(const long *) (const char *) fileMagic & 0xff) == 'm') {
	 cerr << "mdlInput warning: This file has a non-native byte order "
	      << "(performance will suffer)." << endl;
	 fileByteOrder = mdlOpposite;
	 }
      else
	 fileByteOrder = mdlSame;
      binary = TRUE;
      }
   else {
      ErrMsg("File is not a valid mdl file (bad ID)");
      ErrMsg("  ID was '%s'.", (char *) fileMagic);
      inFi = NULL;
      return;
      }

   remain = new int[mdlMaxNestLev + 1];
   assert(remain);

   if (checkTypes) {
      typeSpec = new const char *[mdlMaxNestLev + 1];
      assert(typeSpec);
      nextType = new const char *[mdlMaxNestLev + 1];
      assert(nextType);
      repeatPt = new const char *[mdlMaxNestLev + 1];
      assert(repeatPt);
      typeKey = new mdlKey[mdlMaxNestLev + 1];
      assert(typeKey);
      typeSpec[0] = 0;
      }

   ResetCounts();

   stat = mdlStatus::STAT_OK;
   verifyObject();
   }


   
mdlInput::~mdlInput() {
   verifyObject();

   if (!binary)
      fclose(inFi);

   delete [] remain;

   if (checkTypes) {
       delete [] typeSpec;
       delete [] nextType;
       delete [] repeatPt;
       delete [] typeKey;
       }

   if (stringData) delete [] stringData;
   }



void mdlInput::ResetCounts() {

   struct stat st;

   nestLev = 0;
   
   if (0 != fstat(fileno(inFi), &st)) {
      perror("Error in fstat");
      inFi = NULL;
      return;
      }

   long pos = ftell(inFi);

   remain[0] = ((int) st.st_size / 4) - (int) (pos / 4);
}



long mdlInput::GetBookmark() {

   if (NestLevel() != 0) {
      stat = mdlStatus::STAT_BOOKMARK_ERROR;
      return -1; // -1 is an invalid bookmark
      }

   return ftell(inFi);
   }



void mdlInput::GotoBookmark(long pos) {

   if (pos == -1) return; // -1 is an invalid bookmark
   fseek(inFi, pos, SEEK_SET);
   ResetCounts();
   }



void mdlInput::TypeStart(mdlKey k) {

   assert(checkTypes);

   if (specs)
      typeSpec[nestLev] = mdlFindChunkSpec(k, *specs);
   else
      typeSpec[nestLev] = mdlFindChunkSpec(k);
   nextType[nestLev] = typeSpec[nestLev];
   repeatPt[nestLev] = 0;
   typeKey[nestLev] = k;

   if (!typeSpec[nestLev])
      ErrMsg("No type info for key '%s'", (char *) typeKey[nestLev]);
   }



int mdlInput::TypeCheck(char realType, int count) {

   assert(checkTypes);

   if (typeSpec[nestLev]) {

      int seenARepeat = 0;

      while (count > 0) {

	 while (*(nextType[nestLev]) == ',') {
	    repeatPt[nestLev] = nextType[nestLev];
	    nextType[nestLev]++;
	    seenARepeat = 1;
	    }
      
	 if (*(nextType[nestLev]) != realType)
	    break;
	 
	 nextType[nestLev]++;
	 count--;

	 if (*(nextType[nestLev]) == '\0' && repeatPt[nestLev]) {
	    if (seenARepeat) {
	       int repeatSize = nextType[nestLev] - repeatPt[nestLev];
	       count = count % repeatSize;
	       }
	    seenARepeat = 1;
	    nextType[nestLev] = repeatPt[nestLev];
	    }
	 }

      if (count > 0) {
	 if (*(nextType[nestLev]) != '\0') {
	    ErrMsg("Type conflict in '%s' -- should be %c; asked for %c",
		   (char *) typeKey[nestLev], *(nextType[nestLev]), realType);
	    }
	 else {
	    ErrMsg("Type conflict in '%s' -- asked for extra data %c",
		   (char *) typeKey[nestLev], realType);
	    }
	 return 1;
	 }

      }

   return 0;
   }



void mdlInput::FlipWord(long *lp) {
   *lp = ((*lp & 0xff000000) >> 24 |
	  (*lp & 0x00ff0000) >> 8  |
	  (*lp & 0x0000ff00) << 8  |
	  (*lp & 0x000000ff) << 24);
   }



char mdlInput::NextType() {

   if (!checkTypes) {
      stat = mdlStatus::STAT_NO_TYPE_INFO;
      return '\0';
      }
   
   if (nestLev == 0)
      return 'C';

   if(!typeSpec[nestLev]) {
      stat = mdlStatus::STAT_NO_TYPE_INFO;
      return '\0';
      }
   
   while (*(nextType[nestLev]) == ',') {
      repeatPt[nestLev] = nextType[nestLev];
      nextType[nestLev]++;
      }

   return *(nextType[nestLev]);
   }



mdlKey mdlInput::BeginChunk() {
   verifyObject();

   if (!inFi) { 
      stat = mdlStatus::STAT_BAD_STREAM; 
      return mdlKey(0, 0);
      }

   if (checkTypes && TypeCheck('C')) {
      stat = mdlStatus::STAT_TYPE_ERROR; 
      return mdlKey(0, 0);
      }

   mdlKey data;
   int e;

   if (nestLev >= mdlMaxNestLev) {
       ErrMsg("Excessive nesting in BeginChunk at level %d.", nestLev);
       stat = mdlStatus::STAT_EXCESSIVE_NESTING;
       return mdlKey(0, 0);
       }
   
   if (remain[nestLev] == 0) {
       stat = mdlStatus::STAT_NO_MORE;
       return mdlKey(0, 0);
       }

   nestLev += 1;

   if (2 > (e = fread((long *) data, 4, 2, inFi))) {
      if (e < 0) {
	 ErrMsg("System error in fread (keyword).");
	 perror("  error was");
	 }
      else
	 ErrMsg("Incomplete read (keyword).");
      inFi = NULL;
      stat = mdlStatus::STAT_BAD_STREAM;
      return mdlKey(0, 0);
      }

   if (1 > (e = fread(&remain[nestLev], 4, 1, inFi))) {
      if (e < 0) {
	 ErrMsg("System error in fread (obj length).");
	 perror("  error was");
	 }
      ErrMsg("Incomplete read (obj length).");
      inFi = NULL;
      stat = mdlStatus::STAT_BAD_STREAM;
      return mdlKey(0, 0);
      }

   if (fileByteOrder == mdlOpposite) FlipWord((long *) &remain[nestLev]);

   remain[nestLev - 1] -= remain[nestLev] + 3;

   if (remain[nestLev] < 0 || remain[nestLev - 1] < 0) {
      ErrMsg("Read bad length; input file is invalid."); 
      if (!checkTypes)
	 ErrMsg("Probably a type error -- try using type checking");
      stat = mdlStatus::STAT_BAD_STREAM;
      return mdlKey(0, 0);
      }
   
   if (checkTypes) TypeStart(data);

   stat = mdlStatus::STAT_OK;
   verifyObject();
   return data;
   }





void mdlInput::EndChunk() {
   verifyObject();

   if (!inFi) { 
      stat = mdlStatus::STAT_BAD_STREAM; 
      return;
      }

   if (0 > fseek(inFi, remain[nestLev] * 4, SEEK_CUR)) {
      perror("Error in fseek");
      inFi = NULL;
      return;
      }

   nestLev = nestLev - 1;

   if (nestLev < 0) {
      ErrMsg("Unbalanced EndChunk.");
      stat = mdlStatus::STAT_BAD_END;
      return;
      }

   stat = mdlStatus::STAT_OK;
   verifyObject();
   }






long mdlInput::ReadInt() {
   verifyObject();
 
   if (!inFi) {
       stat = mdlStatus::STAT_BAD_STREAM; 
       return 0;
       }

   if (checkTypes && TypeCheck('i')) {
      stat = mdlStatus::STAT_TYPE_ERROR; 
      return 0;
      }
   
   long data;
   int e;

   if (nestLev == 0) {
      stat = mdlStatus::STAT_TYPE_ERROR;
      return 0;
      }

   if (remain[nestLev] == 0) {
      stat = mdlStatus::STAT_NO_MORE;
      return 0;
      }   
   
   if (1 > (e = fread(&data, 4, 1, inFi))) {
      if (e < 0) {
	 ErrMsg("System error in fread (int).");
	    perror("  error was");
	 }
      ErrMsg("Incomplete read (int).");
      inFi = NULL;
      stat = mdlStatus::STAT_BAD_STREAM;
      return 0;
      }

   if (fileByteOrder == mdlOpposite) FlipWord(&data);
   
   remain[nestLev] -= 1;

   stat = mdlStatus::STAT_OK;

   verifyObject();
   return data;
   }





float mdlInput::ReadFloat() {
   verifyObject();
 
   if (!inFi) {
       stat = mdlStatus::STAT_BAD_STREAM;
       return 0;
       }
   
   if (checkTypes && TypeCheck('f')) {
      stat = mdlStatus::STAT_TYPE_ERROR; 
      return 0;
      }
   
   float data;
   int e;

   if (nestLev == 0) {
      stat = mdlStatus::STAT_TYPE_ERROR;
      return 0;
      }
       
   if (remain[nestLev] == 0) {
      stat = mdlStatus::STAT_NO_MORE;
      return 0;
      }
   
   if (1 > (e = fread(&data, 4, 1, inFi))) {
      if (e < 0) {
	 ErrMsg("System error in fread (float).");
	 perror("  error was");
	 }
      ErrMsg("Incomplete read (float).");
      inFi = NULL;
      stat = mdlStatus::STAT_BAD_STREAM;
      return 0;
      }

   if (fileByteOrder == mdlOpposite) FlipWord((long *) &data);
   
   remain[nestLev] -= 1;

   stat = mdlStatus::STAT_OK;

   verifyObject();
   return data;
   }





char *mdlInput::ReadString() {
   verifyObject();
 
   if (!inFi) {
       stat = mdlStatus::STAT_BAD_STREAM;
       return NULL;
       }

   if (checkTypes && TypeCheck('s')) {
      stat = mdlStatus::STAT_TYPE_ERROR; 
      return 0;
      }
   
   int e;
   
   if (nestLev == 0) {
      stat = mdlStatus::STAT_TYPE_ERROR;
      return 0;
      }
      
   if (remain[nestLev] == 0) {
      stat = mdlStatus::STAT_NO_MORE;
      return 0;
      }

   int length = 0;
   char *buf;
   
   do {

      while (length + 5 > stringDataSize) {
	 int newSize = (stringDataSize == 0) ? 5 : stringDataSize * 2;
	 char *tmpData = new char [newSize];
	 if (!tmpData) {
	    ErrMsg("Failure allocating memory (fatal).");
	    exit(-1);
	    }
	 memcpy(tmpData, stringData, stringDataSize);
	 if (stringData) delete [] stringData;
	 stringData = tmpData;
	 stringDataSize = newSize;
	 }
      
      if (remain[nestLev] == 0) {
	 ErrMsg("End of list while reading string; truncating.");
	 break;
	 }

      buf = stringData + length;
      if (1 > (e = fread(buf, 4, 1, inFi))) {
	 if (e < 0) {
	    ErrMsg("System error in fread (string).");
	    perror("  error was");
	    }
	 ErrMsg("Incomplete read (string).");
	 inFi = NULL;
	 stat = mdlStatus::STAT_BAD_STREAM;
	 return 0;
	 }

      remain[nestLev] -= 1;
      length += 4;
      
      } while (buf[0] != 0 && buf[1] != 0 && buf[2] != 0 && buf[3] != 0);

   buf[4] = '\0';
   length = strlen(stringData);
   
   stat = mdlStatus::STAT_OK;

   verifyObject();
   return stringData;
   }



int mdlInput::ReadInts(long *dest, int n) {
   verifyObject();
 
   if (!inFi) {
      stat = mdlStatus::STAT_BAD_STREAM;
      return 0;
      }
   
   if (nestLev == 0) {
      stat = mdlStatus::STAT_TYPE_ERROR;
      return 0;
      }

   int count;
   int e;   

   stat = mdlStatus::STAT_OK;	 
   
   count = min(n, (int) remain[nestLev]);
   
   if (checkTypes && TypeCheck('i', count)) {
      stat = mdlStatus::STAT_TYPE_ERROR; 
      return 0;
      }

   if (count > (e = fread(dest, 4, count, inFi))) {
      if (e < 0) {
	 ErrMsg("System error in fread (int).");
	 perror("  error was");
	 }
      ErrMsg("Incomplete read (int).");
      inFi = NULL;
      stat = mdlStatus::STAT_BAD_STREAM;
      return 0;
      }
   
   if (fileByteOrder == mdlOpposite) {
      for (int i = 0; i < count; i++)
	 FlipWord(&dest[i]);
      }

   remain[nestLev] -= count;
   
   if (count < n)
      stat = mdlStatus::STAT_NO_MORE;

   verifyObject();
   return count;
   }



int mdlInput::ReadFloats(float *dest, int n) {
   verifyObject();
 
   if (!inFi) {
      stat = mdlStatus::STAT_BAD_STREAM;
      return 0;
      }
   
   if (nestLev == 0) {
      stat = mdlStatus::STAT_TYPE_ERROR;
      return 0;
      }

   int count;
   int e;

   stat = mdlStatus::STAT_OK;	 
   
   count = min(n, (int) remain[nestLev]);
   
   if (checkTypes && TypeCheck('f', count)) {
      stat = mdlStatus::STAT_TYPE_ERROR; 
      return 0;
      }

   if (count > (e = fread(dest, 4, count, inFi))) {
      if (e < 0) {
	 ErrMsg("System error in fread (float).");
	 perror("  error was");
	 }
      ErrMsg("Incomplete read (float).");
      inFi = NULL;
      stat = mdlStatus::STAT_BAD_STREAM;
      return 0;
      }
   
   if (fileByteOrder == mdlOpposite) {
      for (int i = 0; i < count; i++)
	 FlipWord((long *) &dest[i]);
      }

   remain[nestLev] -= count;
   
   if (count < n)
      stat = mdlStatus::STAT_NO_MORE;

   verifyObject();
   return count;
   }




void mdlInput::CopyData(mdlOutput &outp, const mdlCopyCallbackList *cbl,
			mdlUserParam data) {
    
   while (NumRemain() > 0) {
      switch (NextType()) {

       case 'i':
	 outp.WriteInt(ReadInt());
	 break;

       case 'f':
	 outp.WriteFloat(ReadFloat());
	 break;

       case 's':
	 outp.WriteString(ReadString());
	 break;

       case 'C': {
	  mdlKey k = BeginChunk();
	  if (!cbl || !cbl->Call(*this, outp, k, data)) {
	     outp.BeginChunk(k);
	     CopyData(outp, cbl, data);
	     outp.EndChunk();
	     }
	  EndChunk();
	  break;
	  }

       case '\0':
	 stat = mdlStatus::STAT_NO_TYPE_INFO;
	 return;

       default:
	 assert(0);
	 }
      }
   }




void mdlInput::ReadData(const mdlReadCallbackList *cbl, mdlUserParam data) {
    
   while (NumRemain() > 0) {
      switch (NextType()) {

       case 'i':
	 ReadInt();
	 break;

       case 'f':
	 ReadFloat();
	 break;

       case 's':
	 ReadString();
	 break;

       case 'C': {
	  mdlKey k = BeginChunk();
	  if (!cbl || !cbl->Call(*this, k, data))
	     ReadData(cbl, data);
	  EndChunk();
	  break;
	  }

       case '\0':
	 stat = mdlStatus::STAT_NO_TYPE_INFO;
	 return;

       default:
	 assert(0);
	 }
      }
   }



int mdlInput::NumRemain() {
   verifyObject();

   return remain[nestLev];
   }





void mdlInput::Rewind() {

   if (!inFi) {
      stat = mdlStatus::STAT_BAD_STREAM;
      return;
      }
   
   if (0 > fseek(inFi, 8, SEEK_SET)) {
      inFi = NULL;
      stat = mdlStatus::STAT_BAD_STREAM;
      return;
      }

   ResetCounts();

   stat = mdlStatus::STAT_OK;
   }







// Consistency check
void mdlInput::verifyObject() {

   // Don't want invalid counts or levels
   assert (0 <= nestLev && nestLev <= mdlMaxNestLev);
   for (int i = 0; i <= nestLev; i++) {
      assert(remain[i] >= 0);
      }

   // Probably will catch a mashed object
   assert(binary == TRUE || binary == FALSE);
   }


