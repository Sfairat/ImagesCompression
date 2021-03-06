#pragma once

#include "GenericCodec.h"
#include <map>
#include "SFImage.h"
#include "PlatformSpecific.h"
#include "SFTask.h"
#include "SFTaskScheduler.h"

#undef super
#undef self
#define super GenericCodec
#define self JPEGCodec

class JPEGCodec :
	public GenericCodec
{
	DEFAULT_METACLASS_DECLARATION(self, super);

	static int _zig[64], _dezig[64];
	static void _fillZig();

	SFImage *image;

	int width, height;

	int precison;
	int componentsCount;

	struct ComponentInfo
	{
		int id;
		int horizontalSubsampling;
		int verticalSubsampling;
		int quantizationTableID;
		int huffmanTableDCID, huffmanTableACID;
		inline int subBlocksPerBlock() { return horizontalSubsampling * verticalSubsampling; };
		ComponentInfo()
		{
			horizontalSubsampling = verticalSubsampling = 1;
		};
	};

	ComponentInfo componentsInfo[16];

	float **pixels;
	int *quantizationTables[16];
	byte curByte;
	byte curFlag;
	int curBitPos;

	int singleDecodeBlockSize;
	int mcusPerRow, mcusPerCol;
	int mcuBlockWidth, mcuBlockHeight;
	int mcuWidth, mcuHeight;
	int mcuSize, mcuBlockSize;

	struct CurrentDecodeBlockInfo
	{
		int *block;
		int pos;
		int posInSmallBlock;
		int startMCUIndex;
		inline void nextBlock() { posInSmallBlock = 0; pos += 64; };
		inline void writeNext(int val) 
		{
			block[pos + _zig[posInSmallBlock++]] = val; 
		};

		CurrentDecodeBlockInfo(int blockSize, int _startMCUIndex)
		{
			startMCUIndex = _startMCUIndex;
			block = new int[blockSize];
			posInSmallBlock = 0;
			pos = 0;
		};
		~CurrentDecodeBlockInfo()
		{
			delete [] block;
		}
	} *decodeBlockInfo;

	int restartInterval;

	struct HuffmanDecodeInfo
	{
		int minCode[16];
		int maxCode[16];
		int valPtr[16];
		int *values;

		HuffmanDecodeInfo() { values = NULL; };
		~HuffmanDecodeInfo() { delete [] values; };
	};
	HuffmanDecodeInfo dcHuffmanDecodeTables[16], acHuffmanDecodeTables[16];

	int *encodedBlocks;
	int encodeQuality;

	SF_FORCE_INLINE void _readNum(int &to, int len);
	SF_FORCE_INLINE byte readBit(int &to); //If it finds a marker, it returns it's value, 0 otherwise


	struct HuffmanEncodeInfo
	{
		int lengths[256], codes[256];
		int lengthsCount[16];
		int freqSortedVals[256];
	};
	HuffmanEncodeInfo dcHuffmanEncodeTables[2], acHuffmanEncodeTables[2];

	wstring comment;

	virtual void free();

	enum JPEGSection
	{
		JPEGSectionSOI = 0xD8, 
		JPEGSectionSOF0 = 0xC0, //Baseline DCT
		JPEGSectionSOF2 = 0xC2, //Progressive DCT
		JPEGSectionDHT = 0xC4, 
		JPEGSectionDQT = 0xDB, 
		JPEGSectionSOS = 0xDA, 
		JPEGSectionCOM = 0xFE, 
		JPEGSectionEOI = 0xD9, 
	};

	static int defaultQuantizationTable[2][64];

	//public:
	class JPEGEncodeTask : public SFTask
	{
		DEFAULT_METACLASS_DECLARATION(JPEGEncodeTask, SFTask);

	public:
		JPEGCodec *codec;
		int startBlockIndex, blocksCount;
		virtual void runTask()
		{
			codec->_encodeSingleBlockThreaded(startBlockIndex, blocksCount);
		}

		virtual void free()
		{
			codec->release();
			SFTask::free();
		}
	};

	class JPEGDecodeTask : public SFTask
	{
		DEFAULT_METACLASS_DECLARATION(JPEGDecodeTask, SFTask);

	public:
		JPEGCodec *codec;

		CurrentDecodeBlockInfo *decodeBlockInfo;

		virtual void runTask()
		{
			codec->_decodeSingleBlockThreaded(decodeBlockInfo);
		}

		virtual void free()
		{
			codec->release();
			SFTask::free();
		}
	};

	friend class JPEGEncodeTask;
	friend class JPEGDecodeTask;

public:
	
	virtual JPEGCodec* init();

	SFImage* getImage() { return image; };
	void setImage(SFImage *img) { image->release(); image = img; img->retain(); };

//Encoding
public:
	virtual void runEncode();
private:
	void _decodeComment();
	void _decodeSOF0();
	void _decodeDQT();
	void _decodeDHT();
	void _decodeSOS();
	void _dumpQuantizationTables();
	void _decodeSingleBlock(CurrentDecodeBlockInfo *blockAddr);
	void _decodeSingleBlockThreaded(CurrentDecodeBlockInfo *blockAddr);

	SF_FORCE_INLINE int _scanDCCoef(int compNum, int prevVal);
	SF_FORCE_INLINE int _scanACCoef(int compNum);
	SF_FORCE_INLINE void _decodeMCU(int *dcPredictors);

	void _cleanupDecode();
	int taskIndex;
	SFTaskScheduler *taskScheduler;

//Decoding
public:
	void setHorizontalSubsamplingForComponent(int horizontalSubsampling, int component);
	void setVerticalSubsamplingForComponent(int verticalSubsampling, int component);

	void setEncodeQuality(int _encodeQuality) { encodeQuality = _encodeQuality;};
	virtual void runDecode();

private:
	inline static int lengthOfNum(int num);

	void _writeBit(int bit);
	void _flushBits();
	void _writeNum(int num, int len, bool isSigned);

	void _encodeComment();
	void _encodeSOF0();
	void _encodeDQT();
	void _encodeDHT();
	void _encodeSOS();
	void _prepareQuantizationMatrix();
	void _encodeSingleBlock(int startBlockIndex, int blocksCount);
	void _encodeSingleBlockThreaded(int startBlockIndex, int blocksCount);
	void _prepareHuffmanTables();
	void _buildHuffmanEncodeTable(HuffmanEncodeInfo *huffmanPtr, int *frequencies);

	void _cleanupEncode();
//Other
public:
	int getHorizontalSubsamplingForComponent(int component);
	int getVerticalSubsamplingForComponent(int component);

protected:
	virtual ~JPEGCodec(void);
};

