#include "mdl.H"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream.h>

mdlOutput::mdlOutput(FILE *fp, int binary, int typeCheck, mdlByteOrder outputOrder, mdlChunkSpecList *specs)
{
  mdlOutput::specs = specs;
  mdlOutput::binary = binary;
  mdlOutput::typeCheck = typeCheck;
  mdlOutput::fp = fp;
  nestingLevel = 0;
  lengthPositions = new unsigned long[mdlMaxNestLev];
  nextTypes = new const char*[mdlMaxNestLev];
  repeatPts = new const char*[mdlMaxNestLev];
  currentKeys = new mdlKey[mdlMaxNestLev];
  long endianCheck = 0x12345678;
  if (((char *)(&endianCheck))[0] == 0x12)
    {
      machineOrder = mdlBigEndian;
    }
  else
    {
      machineOrder = mdlLittleEndian;
    }
  switch(outputOrder)
    {
    case mdlLittleEndian:
    case mdlBigEndian:
      mdlOutput::outputOrder = outputOrder;
      break;
    case mdlSame:
      mdlOutput::outputOrder = machineOrder;
      break;
    case mdlOpposite:
      mdlOutput::outputOrder = (machineOrder == mdlBigEndian ? mdlLittleEndian : mdlBigEndian);
      break;
    }
  if (binary)
    {
      long *d = (long *)binaryMagic;
      if (mdlOutput::outputOrder != machineOrder)
	{
	  FlipWord((char *)&d[0]);
	  FlipWord((char *)&d[1]);
	}
      fwrite(d,sizeof(long),2,fp);
    }
  else
    {
      char *s = (char *)asciiMagic;
      fprintf(fp,"%s\n",s);
    }
  if (typeCheck)
    {
      nextTypes[nestingLevel] = ",C";
      nextTypes[nestingLevel]--;
    }
  currentKeys[nestingLevel] = mdlKey("");
}

mdlOutput::~mdlOutput()
{
  while (nestingLevel > 0)
    {
      EndChunk();
    }
  delete [] lengthPositions;
  delete [] nextTypes;
  delete [] repeatPts;
  delete [] currentKeys;
}

inline void 
mdlOutput::FlipWord(char *w)
{
  char tmp;
  tmp = w[0];
  w[0] = w[3];
  w[3] = tmp;
  tmp = w[1];
  w[1] = w[2];
  w[2] = tmp;
}
  
  
int
mdlOutput::WriteInt(long data)
{
  if (typeCheck)
    {
      NextType();	  
      if (nextTypes[nestingLevel][0] != 'i')
	{
	  cerr << "Expecting type (" << nextTypes[nestingLevel][0] << ") got (i)" << endl;
	  cerr << "While processing chunk " << (char *)currentKeys[nestingLevel] << endl;
	  int pos = (char *)nextTypes[nestingLevel] - 
	    (char *)(specs ? 
		     mdlFindChunkSpec(currentKeys[nestingLevel],*specs) : 
		     mdlFindChunkSpec(currentKeys[nestingLevel]));
	  cerr << "Position in type string: " << pos << endl;
	  stat = mdlStatus::STAT_TYPE_ERROR;
	  return 0;
	}
    }
  if (binary)
    {
      if (outputOrder != machineOrder)
	{
	  FlipWord((char *)&data);
	}
      fwrite(&data,sizeof(long),1,fp);
    }
  else
    {
      fprintf(fp,"%ld ",data);
    }
  return 1;
}

int
mdlOutput::WriteFloat(float data)
{
  if (typeCheck)
    {
      NextType();	  
      if (nextTypes[nestingLevel][0] != 'f')
	{
	  cerr << "Expecting type (" << nextTypes[nestingLevel][0] << ") got (f)" << endl;
	  cerr << "While processing chunk " << (char *)currentKeys[nestingLevel] << endl;
	  int pos = (char *)nextTypes[nestingLevel] - 
	    (char *)(specs ? mdlFindChunkSpec(currentKeys[nestingLevel],*specs) : 
		     mdlFindChunkSpec(currentKeys[nestingLevel]));
	  cerr << "Position in type string: " << pos << endl;
	  stat = mdlStatus::STAT_TYPE_ERROR;
	  return 0;
	}
    }
  if (binary)
    {
      if (outputOrder != machineOrder)
	{
	  FlipWord((char *)&data);
	}
      fwrite(&data,sizeof(float),1,fp);
    }
  else
    {
      fprintf(fp,"%.6e ",data);
    }
  return 1;
}

int
mdlOutput::WriteString(const char *data)
{
  if (typeCheck)
    {
      NextType();	  
      if (nextTypes[nestingLevel][0] != 's')
	{
	  cerr << "Expecting type (" << nextTypes[nestingLevel][0] << ") got (s)" << endl;
	  cerr << "While processing chunk " << (char *)currentKeys[nestingLevel] << endl;
	  int pos = (char *)nextTypes[nestingLevel] - 
	    (char *)(specs ? mdlFindChunkSpec(currentKeys[nestingLevel],*specs) : 
		     mdlFindChunkSpec(currentKeys[nestingLevel]));
	  cerr << "Position in type string: " << pos << endl;
	  stat = mdlStatus::STAT_TYPE_ERROR;
	  return 0;
	}
    }
  if (binary)
    {
      int length = strlen(data) + 1;
      fwrite(data,sizeof(char),length,fp);
      if (length%4 != 0)
	{
	  char bogus[3];
	  bogus[0] = 0;
	  bogus[1] = 0;
	  bogus[2] = 0;
	  int zeroBytes = (((length>>2) + 1)<<2)-length;
	  fwrite(bogus,sizeof(char),zeroBytes,fp);
	}
    }
  else
    {
      fprintf(fp,"\"%s\" ",data);
    }
  return 1;
}

void
mdlOutput::BeginChunk(mdlKey k)
{
  if (nestingLevel == mdlMaxNestLev)
    {
      stat = mdlStatus::STAT_EXCESSIVE_NESTING;
      return;
    }
  if (typeCheck)
    {
      NextType();
      if (nextTypes[nestingLevel][0] == 'C')
	{
	  const char *nextType = (specs ? mdlFindChunkSpec(k,*specs):mdlFindChunkSpec(k));
	  nextTypes[nestingLevel+1] = nextType-1;
	  if (nextType == 0)
	    {
	      cerr << "Unknown chunk type: " << (char *)k << endl;
	      stat = mdlStatus::STAT_TYPE_ERROR;
	      return;
	    }
	  repeatPts[nestingLevel+1] = 0;
	}
      else
	{
	  cerr << "Expecting type (" << nextTypes[nestingLevel][0] << ") got (C)" << endl;
	  cerr << "While processing chunk " << (char *)currentKeys[nestingLevel] << endl;
	  int pos = (char *)nextTypes[nestingLevel] - 
	    (char *)(specs ? mdlFindChunkSpec(currentKeys[nestingLevel],*specs): 
		     mdlFindChunkSpec(currentKeys[nestingLevel]));
	  cerr << "Position in type string: " << pos << endl;
	  stat = mdlStatus::STAT_TYPE_ERROR;
	  return;
	}
    }
  nestingLevel++;
  if (binary)
    {
      unsigned int nW;
      char *d = (char *)k;
      nW = fwrite(d,sizeof(char),8,fp);
      if (nW != 8)
	{
	  stat = mdlStatus::STAT_BAD_STREAM;
	  return;
	}
      long placeHolder = 0;
      nW = fwrite(&placeHolder,sizeof(long),1,fp);
      if (nW != 1)
	{
	  stat = mdlStatus::STAT_BAD_STREAM;
	  return;
	}	    
      lengthPositions[nestingLevel] = ftell(fp) - sizeof(long);
    }
  else
    {
      char *s = (char *)k;
      fprintf(fp,"%s\n",s);
    }
  currentKeys[nestingLevel] = k;
}

void
mdlOutput::EndChunk()
{
  if (nestingLevel == 0)
    {
      cerr << "EOC at level 0" << endl;
      stat = mdlStatus::STAT_TYPE_ERROR;
      return;
    }
  if (typeCheck)
    {
      nextTypes[nestingLevel]++;
      if (nextTypes[nestingLevel][0] != 0 && nextTypes[nestingLevel][0] != ',')
	{
	  cerr << "Expecting type (" << nextTypes[nestingLevel][0] << ") got (EOC)" << endl;
	  cerr << "While processing chunk " << (char *)currentKeys[nestingLevel] << endl;
	  int pos = (char *)nextTypes[nestingLevel] - 
	    (char *)(specs ? mdlFindChunkSpec(currentKeys[nestingLevel],*specs):
		     mdlFindChunkSpec(currentKeys[nestingLevel]));
	  cerr << "Position in type string: " << pos << endl;
	  stat = mdlStatus::STAT_TYPE_ERROR;
	  return;
	}
    }	  
  if (binary)
    {
      unsigned long currentPos = ftell(fp);
      fseek(fp,lengthPositions[nestingLevel],SEEK_SET);
      unsigned long dataLength = currentPos-lengthPositions[nestingLevel]-sizeof(long);
      if ((dataLength%4) != 0)
	{
	  fprintf(stderr,"Internal error: mdlOutput::EndChunk\n");
	  exit(-2);
	}
      dataLength = dataLength/sizeof(long);
      if (outputOrder != machineOrder)
	{
	  FlipWord((char *)&dataLength);
	}
      unsigned int nW = fwrite(&dataLength,sizeof(long),1,fp);
      if (nW != 1)
	{
	  stat = mdlStatus::STAT_BAD_STREAM;
	  return;
	}	    
      fseek(fp,currentPos,SEEK_SET);
    }
  else
    {
      char *s = (char *) endChunkKey;
      fprintf(fp,"%s\n",s);
    }
  
  stat = mdlStatus::STAT_OK;
  nestingLevel--;
}

int
mdlOutput::WriteInts(const long *data, unsigned int n)
{
  if (typeCheck)
    {
      for (int i = 0; i < n; i++)
	{
	  NextType();
	  if (nextTypes[nestingLevel][0] != 'i')
	    {
	      cerr << "Expecting type (" << nextTypes[nestingLevel][0] << ") got (i)" << endl;
	      cerr << "While processing chunk " << (char *)currentKeys[nestingLevel] << endl;
	      int pos = (char *)nextTypes[nestingLevel] - 
		(char *)(specs ? mdlFindChunkSpec(currentKeys[nestingLevel],*specs) : 
			 mdlFindChunkSpec(currentKeys[nestingLevel]));
	      cerr << "Position in type string: " << pos << endl;
	      stat = mdlStatus::STAT_TYPE_ERROR;
	      return i;
	    }
	}
    }
  if (binary)
    {
      long *copiedData;
      if (outputOrder != machineOrder)
	{
	  unsigned int i;
	  copiedData = new long[n];
	  for (i = 0; i < n; i++)
	    {
	      copiedData[i] = data[i];
	    }
	  for (i = 0; i < n; i++)
	    {
	      FlipWord((char *)&copiedData[i]);
	    }
	  data = copiedData;
	}
      unsigned int nW = fwrite(data,sizeof(long),n,fp);
      if (nW != n)
	{
	  stat = mdlStatus::STAT_BAD_STREAM;
	  return nW;
	}
      if (outputOrder != machineOrder)
	{
	  delete [] copiedData;
	}
    }
  else
    {
      for (unsigned int i = 0; i < n; i++)
	{
	  fprintf(fp,"%ld ",data[i]);
	}
    }
  stat = mdlStatus::STAT_OK;
  return n;
}

int
mdlOutput::WriteFloats(const float *data, unsigned int n)
{
  if (typeCheck)
    {
      for (int i = 0; i < n; i++)
	{
	  NextType();
	  if (nextTypes[nestingLevel][0] != 'f')
	    {
	      cerr << "Expecting type (" << nextTypes[nestingLevel][0] << ") got (f)" << endl;
	      cerr << "While processing chunk " << (char *)currentKeys[nestingLevel] << endl;
	      int pos = (char *)nextTypes[nestingLevel] - 
		(char *)(specs ? mdlFindChunkSpec(currentKeys[nestingLevel],*specs):
			 mdlFindChunkSpec(currentKeys[nestingLevel]));
	      cerr << "Position in type string: " << pos << endl;
	      stat = mdlStatus::STAT_TYPE_ERROR;
	      return i;
	    }
	}
    }
  if (binary)
    {
      float *copiedData;
      if (outputOrder != machineOrder)
	{
	  unsigned int i;
	  copiedData = new float[n];
	  for (i = 0; i < n; i++)
	    {
	      copiedData[i] = data[i];
	    }
	  for (i = 0; i < n; i++)
	    {
	      FlipWord((char *)&copiedData[i]);
	    }
	  data = copiedData;
	}
      unsigned int nW = fwrite(data,sizeof(float),n,fp);
      if (nW != n)
	{
	  stat = mdlStatus::STAT_BAD_STREAM;
	  return nW;
	}
      if (outputOrder != machineOrder)
	{
	  delete [] copiedData;
	}	    
    }
  else
    {
      for (unsigned int i = 0; i < n; i++)
	{
	  fprintf(fp,"%.6e ",data[i]);
	}
    }
  stat = mdlStatus::STAT_OK;
  return n;
}

int
mdlOutput::WriteIntChunk(mdlKey startKey, const long *data, unsigned int n)
{
  BeginChunk(startKey);
  if (stat.Error())
    {
      return 0;
    }
  int nW = WriteInts(data, n);
  if (stat.Error())
    {
      return nW;
    }
  EndChunk();
  return nW;
}

int
mdlOutput::WriteFloatChunk(mdlKey startKey, const float *data, unsigned int n)
{
  BeginChunk(startKey);
  if (stat.Error())
    {
      return 0;
    }
  int nW = WriteFloats(data, n);
  if (stat.Error())
    {
      return nW;
    }
  EndChunk();
  return nW;
}

void
mdlOutput::NewLine()
{
  if (!binary)
    {
      fprintf(fp,"\n");
    }
}

void
mdlOutput::Comment(const char *c)
{
  if (!binary)
    {
      fprintf(fp,"[%s]",c);
    }
}

void
mdlOutput::NextType()
{
  nextTypes[nestingLevel]++;
  if ((nextTypes[nestingLevel][0] == 0) && (repeatPts[nestingLevel] != 0))
    {
      nextTypes[nestingLevel] = repeatPts[nestingLevel] + 1;
    }
  while (nextTypes[nestingLevel][0] == ',')
    {
      repeatPts[nestingLevel] = nextTypes[nestingLevel];
      nextTypes[nestingLevel]++;
    }
}


