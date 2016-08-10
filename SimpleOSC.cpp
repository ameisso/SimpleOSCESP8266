#include "SimpleOSC.h"

OSCMessage::OSCMessage()
{
  setupMessage();
}

OSCMessage::OSCMessage(const char * address)
{
  setupMessage();
  outputAddress = address;
}

void OSCMessage::setupMessage()
{
  incomingBuffer = NULL;
  inputDatas = NULL;
  incomingBufferSize = 0;

  outputDatas = NULL;
  outputDatasSize = 0;
}

//DESTRUCTOR
OSCMessage::~OSCMessage()
{
  empty();
}

void OSCMessage::empty()
{
  free(incomingBuffer);
  free(inputDatas);
  free(outputDatas);
}

///RECEIVING
void OSCMessage::fill(char incomingByte)
{
  incomingBuffer = (char *) realloc ( incomingBuffer, incomingBufferSize + 1 );
  if (incomingBuffer != NULL)
  {
    incomingBuffer[incomingBufferSize++] = incomingByte;
  }
}

bool OSCMessage::parse()
{
//  Serial.println("\n----");
  //Serial.print("got : ");
  int addressLocation = -1;
  int typesLocation = -1;
  int dataLocation = -1;
/*for(int i = 0 ; i < incomingBufferSize ; i++)
  {
    Serial.print (incomingBuffer[i],DEC);  Serial.print(" ");
  }
  Serial.println();*/
  for(int i = 0 ; i < incomingBufferSize ; i++)
  {
    if (incomingBuffer[i] == 44)
    {
      char charAddress[i-1];
      memcpy(charAddress,incomingBuffer,i-1);
      inputAddress = charAddress;
      addressLocation = i;
      //Serial.print("address : ");  Serial.println(inputAddress);
    }
    else if ( addressLocation > 0 && incomingBuffer[i] == 0 && typesLocation < 0)
    {
      char charTypes[i-addressLocation-1];
      memcpy(charTypes,&incomingBuffer[addressLocation+1],i-1);
      inputTypes = charTypes;
      typesLocation = i;
      //Serial.print("types : ");  Serial.println(inputTypes);
    }
    else if ( addressLocation > 0 && typesLocation > 0 && dataLocation <0)
    {
      dataLocation = incomingBufferSize;
      int typesCount = typesLocation - addressLocation - 1;
      int bitCount = 4*round((typesCount+1)/4)+3;
      int zeroCount = bitCount-typesCount;
      //Serial.print("datas : ");
      inputDatasSize = incomingBufferSize-(typesLocation+zeroCount);
      inputDatas = (char *) malloc ( inputDatasSize );
      memcpy(inputDatas,&incomingBuffer[typesLocation+zeroCount], inputDatasSize);
      for(int j = 0 ; j < inputDatasSize ; j++)
      {
      if( j%4 == 0 )
      {
      //Serial.println();
    }
    //Serial.print (inputDatas[j],DEC);  Serial.print(" ");
  }
}
else if ( addressLocation > 0 && typesLocation > 0 && dataLocation > 0 )
{
  return true;
}
}
return false;
}

void OSCMessage::route(const char * pattern, void (*callback)(OSCMessage &))
{
  if( inputAddress == pattern)
  {
    callback(*this);
  }
}

String OSCMessage::getInputTypes()
{
  return inputTypes;
}

int OSCMessage::finalOffsetForOffset(int offset)
{
  //Serial.print("input :");
  int finalOffset = 0;
  for (size_t i = 0; i < offset; i++)
  {
    if (inputTypes.charAt(i)== 'i' || inputTypes.charAt(i) == 'f')
    {
      finalOffset++;
    }
    else if (inputTypes.charAt(i)== 's')
    {
      for(int j = 4*finalOffset ; j < inputDatasSize ; j++)
      {
        if( j%4 == 0 )
        {
          finalOffset++;
        }
        if(inputDatas[j]==0)
        {
          break;
        }
      }
    }
  }
  //Serial.print("\n----\nfinal offset");
  //Serial.println(finalOffset);
  return finalOffset;
}

int OSCMessage::getInt(int offset)
{
  int finalOffset = finalOffsetForOffset(offset);
  return (inputDatas[4*finalOffset] << 24) | (inputDatas[4*finalOffset+1] << 16) | (inputDatas[4*finalOffset+2] << 8) | inputDatas[4*finalOffset+3];
}

float OSCMessage::getFloat(int offset)
{
  int finalOffset = finalOffsetForOffset(offset);
  union {
    float f;
    char b[4];
  } u;
  u.b[3] = inputDatas[4*finalOffset];
  u.b[2] = inputDatas[4*finalOffset+1];
  u.b[1] = inputDatas[4*finalOffset+2];
  u.b[0] = inputDatas[4*finalOffset+3];
  return u.f;
}

String OSCMessage::getString(int offset)
{
  int finalOffset = finalOffsetForOffset(offset);
  String s = String();
  for (size_t i = finalOffset*4; i < inputDatasSize; i++)
  {
    if( inputDatas[i]==0 )
    {
      break;
    }
    s.concat(inputDatas[i]);
  }
  return s;
}

///SENDING
void OSCMessage::addInt(int value)
{
  outputDatas = (char *) realloc ( outputDatas, outputDatasSize + 4 );
  if (outputDatas != NULL)
  {
    outputDatas[outputDatasSize] = (value >> 24) & 0xFF;
    outputDatas[outputDatasSize+1] = (value >> 16) & 0xFF;
    outputDatas[outputDatasSize+2] = (value >> 8) & 0xFF;
    outputDatas[outputDatasSize+3] = value & 0xFF;
    outputDatasSize += 4;
    outputTypes.concat('i');
  }
}

void OSCMessage::addFloat(float value)
{
  outputDatas = (char *) realloc ( outputDatas, outputDatasSize + 4 );
  if (outputDatas != NULL)
  {
    union {
      float f;
      char b[4];
    } u;
    u.f = value;
    outputDatas[outputDatasSize] = u.b[3];
    outputDatas[outputDatasSize+1] = u.b[2];
    outputDatas[outputDatasSize+2] = u.b[1];
    outputDatas[outputDatasSize+3] = u.b[0];
    outputDatasSize += 4;
    outputTypes.concat('f');
  }
}

void OSCMessage::addString(const char * value)
{
  int stringLength = strlen(value);
  int bitCount = 4*round((strlen(value)+1)/4)+4;
  int zeroCount = bitCount - stringLength+1;
  outputDatas = (char *) realloc ( outputDatas, outputDatasSize + bitCount );
  if (outputDatas != NULL)
  {
    memset(&outputDatas[outputDatasSize],0,bitCount);
    memcpy(&outputDatas[outputDatasSize],value,stringLength);
    outputDatasSize += bitCount;
    outputTypes.concat('s');
  }
}

void OSCMessage::prinOutputDatas()
{
  Serial.print(outputAddress);    Serial.print(" ");      Serial.print (outputDatasSize);  Serial.println(" : ");
  for (int i = 0 ; i < outputDatasSize ; i++)
  {
    if( i%4 == 0  && i > 0)
    {
      Serial.println();
    }
    Serial.print (outputDatas[i],DEC);  Serial.print(" ");
  }
  Serial.println("\n----");
}


void OSCMessage::send(WiFiUDP &udp, IPAddress remoteIP, unsigned int outPort)
{
  //Serial.print ("remote : "); Serial.print(remoteIP); Serial.print(":"); Serial.println(outPort);
  //prinOutputDatas();

  int addressBitCount = 4*round((strlen(outputAddress.c_str())+3)/4);
  int addresszeroCount = addressBitCount-strlen(outputAddress.c_str());
  int outputBufferSize = addressBitCount+1;//address+padding+comma
  //Serial.print ("types : "); Serial.println(outputTypes);
  outputBufferSize += strlen(outputTypes.c_str());
  int outputTypesBitsCount = 4*round((strlen(outputTypes.c_str())+1)/4)+3;
  int outputTypesZerosCount = outputTypesBitsCount-strlen(outputTypes.c_str());
  outputBufferSize += outputTypesZerosCount; //outputTypePadding
  outputBufferSize += outputDatasSize;
  //Serial.print ("bufferSize : ");  Serial.println(outputBufferSize);
  uint8_t nullChar = '\0';

  char outputBuffer[outputBufferSize];
  memset(outputBuffer,nullChar,outputBufferSize);
  memcpy(&outputBuffer[0],outputAddress.c_str(),strlen(outputAddress.c_str()));
  outputBuffer[strlen(outputAddress.c_str())+addresszeroCount] = ',';
  memcpy(&outputBuffer[strlen(outputAddress.c_str())+addresszeroCount+1],outputTypes.c_str(),strlen(outputTypes.c_str()));
  memcpy(&outputBuffer[strlen(outputAddress.c_str())+addresszeroCount+1+strlen(outputTypes.c_str())+outputTypesZerosCount],outputDatas,outputDatasSize);

  udp.beginPacket(remoteIP,outPort);
  udp.write((uint8_t *)&outputBuffer,outputBufferSize);
  udp.endPacket();
}
