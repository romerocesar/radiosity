
#include "mdl.H"
#include <string.h>



static int max(int a, int b) { return (a > b) ? a : b; }


static mdlChunkSpec defaultSpecs[] = {

   // Meshes
   { "msh     ", "sC,C" },
   { "plnrMsh ", "sC,C" },
   
   // Vertex data chunks
   { "vrtxPstn", ",fff" },
   { "vrtxNrml", ",fff" },
   { "vrtxUV  ", ",ff" },
   { "vrtxIrrd", ",fff" },
   { "vrtxRdEx", ",fff" },

   // Transformation chunks
   { "trnsltn ", "fff" },
   { "unfrmscl", "f" },
   { "axsnglrt", "ffff" },

   // Patch definition chunks
   { "trngl   ", ",iii" },
   { "qdrltrl ", ",iiii" },
   { "trnglStr", "iii,i" },
   { "plygn   ", "iii,i" },
   { "cmplxPly", "C,C" },

   // Patch data chunks
   { "ptchIrrd", ",fff" },
   { "ptchRdEx", ",fff" },
   
   // Material definition chunks
   { "lmbrtn  ", "C,C" },
   { "pMtl    ", "CC,C" },
   { "pDlctrc ", "CCC,C" },
   { "lyrd    ", "CCC,C" },
   { "uth     ", "CCCC,C" },
   { "ornnyr  ", "CC,C" },
   { "pLmnr   ", "CCC,C" },

   // A camera
   { "cmr     ", "sfffffffffffffff,C" },

   // Camera description chunks
   { "fcsDstnc", "f" },
   { "aprtr   ", "f" },
   { "shrt    ", "f" },

   // Naming chunks
   { "nmdMtrl ", "sC" },
   { "nmdTxtr ", "sC" },
   { "nmdClr  ", "sC" },
   { "mtrlNm  ", "s" },
   { "txtrNm  ", "s" },
   { "clrNm   ", "s" },

   // Texture chunks
   { "imgFl   ", "ss" },
   { "imgDt   ", "iiC" },
   { "txtrMp  ", "sC" },

   // Color chunks
   { "rgb     ", "fff" },
   { "sclr    ", "f" },
   { "spctrl  ", "ff,ff" },
   { "rgbLst  ", ",fff" },
   { "sclrLst ", ",f" },

   // Miscellaneous
   { "bckgrnd ", ",C" },
   { "cmnt    ", ",s" },
   { "avrg    ", "f,f" },
   { "cnstnt  ", "f,f" },
   { "nGrd    ", "" },
   { "flHstry ", ",s" },
   { "chnkTyp ", "ss" },

   { NULL, NULL }
   };


static
mdlChunkSpecList defaultSpecList(defaultSpecs);


//
// A Spec List Entry
//

class mdlSLE {
 public:

   mdlSLE(const mdlChunkSpec &);

   mdlSLE();
   ~mdlSLE();
   mdlSLE(const mdlSLE &);
   mdlSLE &operator =(const mdlSLE &);

   const char *key() const { return k; }
   const char *spec() const { return s; }

 private:
   char *k;
   char *s;
   };



mdlSLE::mdlSLE() {
   k = s = 0;
   }

mdlSLE::mdlSLE(const mdlChunkSpec &cs) {
   k = s = 0;
   if (cs.key) k = strdup(cs.key);
   if (cs.spec) s = strdup(cs.spec);
   }

mdlSLE::~mdlSLE() {
   if (k) delete k;
   if (s) delete s;
   }

mdlSLE::mdlSLE(const mdlSLE &sle) {
   k = s = 0; 
   if (sle.k) k = strdup(sle.k);
   if (sle.s) s = strdup(sle.s);
   }

mdlSLE &mdlSLE::operator =(const mdlSLE &sle) {
   if (k) delete k;
   if (s) delete s;
   k = s = 0; 
   if (sle.k) k = strdup(sle.k);
   if (sle.s) s = strdup(sle.s);
   return *this;
   }




mdlChunkSpecList::mdlChunkSpecList() {
   nAlloc = 1;
   n = 0;
   list = new mdlSLE[1];
   }


mdlChunkSpecList::mdlChunkSpecList(const mdlChunkSpec *ps) {
   nAlloc = 1;
   n = 0;
   list = new mdlSLE[1];
   Append(ps);
   }


mdlChunkSpecList::~mdlChunkSpecList() {
   delete [] list;
   }


mdlChunkSpecList::mdlChunkSpecList(const mdlChunkSpecList &l) {
   nAlloc = n = l.n;
   list = new mdlSLE[n];
   for (int i = 0; i < n; i++)
      list[n] = l.list[n];
   }


mdlChunkSpecList &mdlChunkSpecList::operator =(const mdlChunkSpecList &l) {
   delete [] list;
   nAlloc = n = l.n;
   list = new mdlSLE[n];
   for (int i = 0; i < n; i++)
      list[i] = l.list[i];
   return *this;
   }


void mdlChunkSpecList::Append(const mdlChunkSpec *ps) {

   int nNew = 0;
   for (const mdlChunkSpec *q = ps; q->key; q++)
      nNew++;

   if (n + nNew > nAlloc) {
      int newNAlloc;
      newNAlloc = max(nAlloc * 2, n + nNew);
      mdlSLE *tmpL = list;
      list = new mdlSLE[newNAlloc];
      nAlloc = newNAlloc;
      for (int i = 0; i < n; i++)
	 list[i] = tmpL[i];
      delete [] tmpL;
      }   

   for (int i = 0; i < nNew; i++)
      list[n + i] = ps[i];

   n += nNew;
   }



const char *mdlChunkSpecList::Find(mdlKey key) const {

   for (int i = 0; i < n; i++)
      if (mdlKey(list[i].key()) == key)
	 return list[i].spec();
   
   return NULL;
   }




const char *mdlFindChunkSpec(mdlKey key) {

   return defaultSpecList.Find(key);
   }



const char *mdlFindChunkSpec(mdlKey key, const mdlChunkSpecList &list) {

   const char *res;
   
   res = list.Find(key);

   if (!res)
      res = defaultSpecList.Find(key);

   return res;
   }


