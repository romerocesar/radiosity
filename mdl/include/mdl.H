#ifndef _MODELFILE_H_
#define _MODELFILE_H_

#include <stdio.h>
#include <iostream.h>


#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif


// Magic numbers
extern const int mdlMaxNestLev;

typedef enum mdlByteOrder {
  mdlBigEndian,
  mdlLittleEndian,
  mdlSame,
  mdlOpposite
} mdlByteOrder;




class mdlStatus {
 public:
   static const int STAT_OK;
   
   static const int STAT_BAD_STREAM;    // The i/o stream is somehow invalid
   static const int STAT_EXCESSIVE_NESTING;   // Chunks nested too deeply
   static const int STAT_BAD_END;       // More EndChunks than BeginChunks
   static const int STAT_TYPE_ERROR;    // You asked for the wrong type
   static const int STAT_NO_TYPE_INFO;  // You need to have type info
   static const int STAT_BOOKMARK_ERROR; // Asked for a bookmark inside a chunk
   static const int STAT__MIN;
   
   static const int STAT_NO_MORE;     // No more of what you requested remain
   static const int STAT__MAX;
   
   
   mdlStatus(int n = STAT_OK);
   ~mdlStatus();
   mdlStatus(const mdlStatus &s);
   mdlStatus &operator =(const mdlStatus &s);
   
   operator int() { return code; }
   int Error()    { return (code < 0); }
   int OK()       { return (code == STAT_OK); }
   
   const char *Name();
   
   static void DieOnError(int die) { dieOnError = die; }
   
 private:
   static int dieOnError;  // defaults to 1 (true)
   int code;
   
   static const char *nameCode[];
   
   void Die();
   };  




struct mdlKey {
 public:

   mdlKey(long = 0, long = 0);
   mdlKey(const char *, int len = 8);
   ~mdlKey();

   mdlKey(const mdlKey &);
   mdlKey &operator =(const mdlKey &);

   // Make it a string; be careful not write something that accidentally uses
   // the string after the mdlKey gets deleted!
   operator char *() const;

   // Give a pointer to an array of 2 longs.
   operator long *() const;

   friend int operator ==(const mdlKey &, const mdlKey &);
   friend int operator !=(const mdlKey &, const mdlKey &);

   friend ostream &operator <<(ostream &, const mdlKey &);


 private:
   long data[2];
   char terminator;
   };




// Magic keys
extern const mdlKey asciiMagic;
extern const mdlKey binaryMagic;
extern const mdlKey swappedBinaryMagic;
extern const mdlKey endChunkKey;
// Old keys
extern const mdlKey oldAsciiMagic;
extern const mdlKey oldBinaryMagic;
extern const mdlKey endLstKey;
extern const mdlKey endObjctKey;




//
// Stuff for keeping track of type specifications.  mdlFindChunkSpec takes a
// key and gives you a type spec like "ssi,C", which would mean that that
// chunk should consist of two strings, an int, and zero or more chunks.  If
// you want to add chunk specs for your own purposes, you can pass a
// mdlChunkSpecList and it'll look there before looking at the library
// defaults.
//

//
// A specification of a chunk type, which associates a key with a type
// specification string for objects with that key.  When you make a
// mdlChunkSpecList from an array of these, you should terminate the array
// with a NULL key.
//

struct mdlChunkSpec {

   const char *key;
   const char *spec;
   };


//
// Just use this to encapsulate an array of mdlChunkSpecs to be passed to
// mdlFindChunkSpec.  It's just here in case we want to change the internal
// representation.
//

class mdlSLE;

class mdlChunkSpecList {
 public:
   mdlChunkSpecList();
   mdlChunkSpecList(const mdlChunkSpec *ps);
   ~mdlChunkSpecList();
   mdlChunkSpecList(const mdlChunkSpecList &l);
   mdlChunkSpecList &operator =(const mdlChunkSpecList &l);

   // Add a NULL-terminated array of specs to this list
   void Append(const mdlChunkSpec *ps);

   // Look up a mdlKey and return the associated string, or NULL
   const char *Find(mdlKey) const;

 private:
   mdlSLE *list;
   int n, nAlloc;
   };


const char *mdlFindChunkSpec(mdlKey key);
const char *mdlFindChunkSpec(mdlKey key, const mdlChunkSpecList &list);




//
// A function to make an ascii mdlfl into a binary one.  Used by mdlInput.
// Input should be pointing just after file ID; i.e. ftell(inFi) == 8.
// Output must be seekable (not a pipe).
// If you want to disable type checking, pass 0 for checkTypes.
// If you want to specify an additional type list, pass a ptr to it in specs.
// Returns STAT_OK, STAT_TYPE_ERROR, or STAT_BAD_STREAM
//

mdlStatus mdlCompile(const char *execName, FILE *inFi, FILE *outFi,
		     int checkTypes = 1,
		     const mdlChunkSpecList *specs = NULL);




//
// Types for callbacks that get called from mdlInput::CopyData and
// mdlInput::ReadData
//

class mdlInput;
class mdlOutput;
class mdlKey;

typedef void *mdlUserParam;
typedef int (*mdlCopyCallback)(mdlInput &, mdlOutput &,
			       mdlKey, mdlUserParam);
typedef int (*mdlReadCallback)(mdlInput &, mdlKey, mdlUserParam);


//
// A specification of a copy callback, which associates a key with a handler
// for objects with that key.  When you make a mdlCopyCallbackList from an
// array of these, you should terminate the array with a NULL fn.
//

struct mdlCopyCallbackSpec {
   const char *key;      // NULL for default handler
   mdlCopyCallback fn;
   };


class mdlCopyCallbackList {
 public:
   mdlCopyCallbackList(const mdlCopyCallbackSpec *cs) : list(cs) {}
   ~mdlCopyCallbackList() {}
   mdlCopyCallbackList(const mdlCopyCallbackList &l) : list(l.list) {}
   mdlCopyCallbackList &operator =(const mdlCopyCallbackList &l)
      { list = l.list;  return *this; }

   int Call(mdlInput &, mdlOutput &, mdlKey, mdlUserParam) const;

 private:
   const mdlCopyCallbackSpec *list;
   };


//
// A specification of a read callback, which associates a key with a handler
// for objects with that key.  When you make a mdlReadCallbackList from an
// array of these, you should terminate the array with a NULL fn.
//

struct mdlReadCallbackSpec {
   const char *key;      // NULL for default handler
   mdlReadCallback fn;
   };


class mdlReadCallbackList {
 public:
   mdlReadCallbackList(const mdlReadCallbackSpec *cs) : list(cs) {}
   ~mdlReadCallbackList() {}
   mdlReadCallbackList(const mdlReadCallbackList &l) : list(l.list) {}
   mdlReadCallbackList &operator =(const mdlReadCallbackList &l)
      { list = l.list;  return *this; }

   int Call(mdlInput &, mdlKey, mdlUserParam) const;

 private:
   const mdlReadCallbackSpec *list;
   };




class mdlOutput;

class mdlInput {
 public:


   // Create an object to read from an already opened input stream.  If you
   // want to prevent type checking for some reason, you can pass zero for the
   // second arg.  If you want to specify some types beyond the standard ones,
   // pass some in the third arg.
   mdlInput(FILE *, int checkTypes = 1,
	    const mdlChunkSpecList *specs = NULL);
   ~mdlInput();

   // The status of the last operation that you did (will always be
   // non-negative (i.e. non-error) if mdlInputStatus::dieOnError is true.)

   mdlStatus LastStatus() { return stat; }

   // This function moves you into the scope of the next sub-chunk of the
   // current chunk, leaving you looking at the first thing in that sub-chunk.
   // If there is no more stuff in the current chunk, returns with a status of
   // STAT_NO_MORE.

   mdlKey BeginChunk();

   // This function moves you back into the scope of the chunk enclosing the
   // current chunk, leaving you looking at the first thing following the
   // current chunk.

   void EndChunk();

   // These things give you the next token.  If there are no more things in
   // the current chunk, returns with a status of STAT_NO_MORE.  ReadString
   // returns a string that is valid until the next call or until 
   // the mdlInput object is destroyed; the second version stores into a
   // user-supplied array, dying if the string is too long.

   long ReadInt();
   float ReadFloat();
   char *ReadString();

   // These functions work in the same way, but read many data items at once.
   // The return value is a count of words actually read.

   int ReadInts(long *dest, int n);
   int ReadFloats(float *dest, int n);

   // This function reads to the end of the current chunk, calling callbacks
   // in the callback list when it encounters matching keywords.  All chunks
   // that aren't handled by a callback are copied to the output.  A chunk
   // qualifies as not handled if there is no callback matching its keyword or
   // if the callback returns false (which it should do without reading any
   // data).  A callback that returns true is taking responsibility for the
   // whole chunk; no sub-chunks will be copied automatically unless the
   // callback calls CopyData recursively.

   void CopyData(mdlOutput &, const mdlCopyCallbackList * = NULL, 
		 mdlUserParam = NULL);

   // This function is just like CopyData, but doesn't write the data
   // anywhere.  This lets you structure your program as if you were using
   // CopyData even if you're not copying data.

   void ReadData(const mdlReadCallbackList * = NULL, mdlUserParam = NULL);

   // This function tells you how many words remain in the chunk you're
   // currently reading. Note that this tells you how many ints or floats
   // remain, but doesn't count strings or chunks.  If the remaining data
   // contains strings or chunks, the only significance of the return value is
   // whether or not it is equal to zero.

   int NumRemain();

   // What is the current nesting level?  0 is file scope.

   int NestLevel() { return nestLev; }

   // What is the appropriate next type?  Valid results are 'i', 'f', 's', or
   // 'C'.  Will return '\0' and set status if there's no type info.

   char NextType();

   // This function takes this object back to the state it was in just after
   // the constructor finished.  Will return STAT_BAD_STREAM if an error
   // occurs, which will happen if you try to rewind a pipe.

   void Rewind();

   // The two ways to read the variable part of a chunk are to repeatedly read
   // elements until you get STAT_NO_MORE, or, if you are reading only ints or
   // only floats, to ask how many words there are, allocate space, and read
   // them all in a single operation.

   // This function returns a bookmark that can be returned to later using
   // GotoBookmark().  It can only be called at top level (when not reading
   // any chunks).

   long GetBookmark();

   // Return to a bookmark obtained using GetBookmark().

   void GotoBookmark(long);

   // It's OK to compare these bookmarks for equality or to tell which is
   // earlier in the file, and it's even OK to consider their difference to be
   // a number of bytes.  The beginning of the file is not necessarily zero.



 private:

   void verifyObject(); // consistency check

   void ResetCounts();

   void TypeStart(mdlKey);
   int TypeCheck(char, int = 1);

   void FlipWord(long *);

   int binary;
   FILE *inFi;

   mdlStatus stat;

   int nestLev;
   int *remain;
   char *stringData;
   int stringDataSize;

   int checkTypes;
   const mdlChunkSpecList *specs;

   const char **typeSpec, **nextType, **repeatPt;
   mdlKey *typeKey;

   mdlByteOrder fileByteOrder;
   };





class mdlOutput {
public:

  // Make an object that writes to a particular stream, either in ASCII (the
  // default) or in binary.
  mdlOutput(FILE *, int binary = FALSE, int typeCheck = TRUE,
	    mdlByteOrder = mdlSame, mdlChunkSpecList *spec = 0);
  ~mdlOutput();

  // These functions are used to place delimiters around chunks.
  void BeginChunk(mdlKey startKey);
  void EndChunk();

  // These functions each write a single data item.
  int WriteInt(long);
  int WriteFloat(float);
  int WriteString(const char *);
   
  // These functions write a bunch of data items.
  int WriteInts(const long *, unsigned int n);
  int WriteFloats(const float *, unsigned int n);

  // These functions should be able to be faster for writing to binary files
  int WriteIntChunk(mdlKey startKey, const long *, unsigned int n);
  int WriteFloatChunk(mdlKey startKey, const float *, unsigned int n);

  // These functions do pretty formatting stuff in ASCII files; in binary
  // files they are no-ops.  ASCII files might get indented nicely, if it
  // turns out to be easy.
  void NewLine();
  void Comment(const char *);
    
  // Status of last operation.
  mdlStatus
  LastStatus()
  {
    return stat;
  }

private:
  void
  NextType();

  inline void
  FlipWord(char *w);

  int binary;
  int typeCheck;
  unsigned long *lengthPositions;
  const char **nextTypes;
  const char **repeatPts;
  mdlKey *currentKeys;
  int nestingLevel;
  mdlStatus stat;
  FILE *fp;
  mdlByteOrder machineOrder;
  mdlByteOrder outputOrder;
  mdlChunkSpecList *specs;
};

#endif // _MODELFILE_H_
