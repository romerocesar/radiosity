#include "mdl.H"

#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>


const int mdlMaxNestLev = 500;

// --- Methods of mdlKey; some should probably become inline ----------------
const mdlKey asciiMagic("mdlflA20");
const mdlKey binaryMagic("mdlflB20");
const mdlKey swappedBinaryMagic("fldm02Bl");
const mdlKey endChunkKey("end");
const mdlKey oldAsciiMagic("mdlflAsc");
const mdlKey oldBinaryMagic("mdlflBin");
const mdlKey endLstKey("endLst");
const mdlKey endObjctKey("endObjct");

mdlKey::mdlKey(long l1, long l2) {
   data[0] = l1; data[1] = l2;
   terminator = '\0';
   }

mdlKey::mdlKey(const char *str, int len) {

   if (len > 8) len = 8;
   strncpy((char *) data, str, len);

   int i;

   for (i = 0; i < len && ((char *) data)[i]; i++);
   for ( ; i < 8; i++)
      ((char *) data)[i] = ' ';

   terminator = '\0';
   }

mdlKey::~mdlKey() {
   }


mdlKey::mdlKey(const mdlKey &k) {
   data[0] = k.data[0]; data[1] = k.data[1];
   terminator = '\0';
   }

mdlKey &mdlKey::operator =(const mdlKey &k) {
   data[0] = k.data[0]; data[1] = k.data[1];
   terminator = '\0';
   return *this;
   }

mdlKey::operator char *() const {
   return (char *) (&data[0]);
   }

mdlKey::operator long *() const {
   return (long *) (&data[0]);
   }


int operator ==(const mdlKey &k1, const mdlKey &k2) {
   return k1.data[0] == k2.data[0] && k1.data[1] == k2.data[1];
   }

int operator !=(const mdlKey &k1, const mdlKey &k2) {
   return !(k1.data[0] == k2.data[0] && k1.data[1] == k2.data[1]);
   }


ostream &operator <<(ostream &ost, const mdlKey &k) {
   ost << (const char *) (&k.data[0]);
   return ost;
   }




// --- Methods of mdlCopyCallbackList ------


int mdlCopyCallbackList::Call(mdlInput &inp, mdlOutput &outp,
			      mdlKey k, mdlUserParam data) const {

   const mdlCopyCallbackSpec *def = NULL;

   for (const mdlCopyCallbackSpec *p = list; p->fn; p++)
      if (p->key == 0)
	 def = p;
      else if (mdlKey(p->key) == k)
	 return p->fn(inp, outp, k, data);

   if (def)
      return def->fn(inp, outp, k, data);

   return 0;
   }


// --- Methods of mdlReadCallbackList ------

int mdlReadCallbackList::Call(mdlInput &inp, 
			      mdlKey k, mdlUserParam data) const {

   const mdlReadCallbackSpec *def = NULL;

   for (const mdlReadCallbackSpec *p = list; p->fn; p++)
      if (p->key == 0)
	 def = p;
      else if (mdlKey(p->key) == k)
	 return p->fn(inp, k, data);

   if (def)
      return def->fn(inp, k, data);

   return 0;
   }


// --- Definitions of mdlStatus constants and methods---
static void ErrMsg(const char *format, ...) {
   va_list ap;

   fprintf(stderr, "mdlStatus: ");

   va_start(ap, format);
   vfprintf(stderr, format, ap);
   va_end(ap);

   fprintf(stderr, "\n");
   }

const int mdlStatus::STAT_OK = 0;

const int mdlStatus::STAT_BAD_STREAM  = -1;
const int mdlStatus::STAT_EXCESSIVE_NESTING = -2;
const int mdlStatus::STAT_BAD_END = -3;
const int mdlStatus::STAT_TYPE_ERROR = -4;
const int mdlStatus::STAT_NO_TYPE_INFO = -5;
const int mdlStatus::STAT_BOOKMARK_ERROR = -6;
const int mdlStatus::STAT__MIN = -6;

const int mdlStatus::STAT_NO_MORE = 1;
const int mdlStatus::STAT__MAX = 2;

int mdlStatus::dieOnError = 1;

const char *mdlStatus::nameCode[] = {
   "STAT_BOOKMARK_ERROR",
   "STAT_NO_TYPE_INFO",
   "STAT_TYPE_ERROR",
   "STAT_BAD_END",
   "STAT_EXCESSIVE_NESTING",
   "STAT_BAD_STREAM",
   "STAT_OK",
   "STAT_NO_MORE",
   "(bad status)"
   };

mdlStatus::mdlStatus(int n) {
   code = n;
   if (dieOnError && Error())
      Die();
   }

mdlStatus::~mdlStatus() {
   }

mdlStatus::mdlStatus(const mdlStatus &s) {
   code = s.code;
   if (dieOnError && Error())
      Die();
   }

mdlStatus &mdlStatus::operator =(const mdlStatus &s) {
   code = s.code;
   if (dieOnError && Error())
      Die();
   return *this;
   }


const char *mdlStatus::Name() {
   if (code < STAT__MIN || code >= STAT__MAX)
      return nameCode[STAT__MAX - STAT__MIN];
   else
      return nameCode[code - STAT__MIN];
   };

void mdlStatus::Die() {
   ErrMsg("Command failed; status = %s", Name());
   exit(-1);
   }

