/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MmpDecoderAudio_AAC_PV.hpp"
#include "../MmpComm/MmpUtil.hpp"


/////////////////////////////////////////////////////////////
//CMmpDecoderAudio_AAC_PV Member Functions

CMmpDecoderAudio_AAC_PV::CMmpDecoderAudio_AAC_PV(CMmpMediaInfo* pMediaInfo) : CMmpDecoderAudio(pMediaInfo)
,mConfig(NULL)
,mDecoderBuf(NULL)
,m_bADTS(FALSE)
,mInputBufferCount(0)
{
	
}

CMmpDecoderAudio_AAC_PV::CMmpDecoderAudio_AAC_PV(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderAudio(pCreateConfig)
,mConfig(NULL)
,mDecoderBuf(NULL)
,m_bADTS(FALSE)
,mInputBufferCount(0)
{
	if(pCreateConfig->nStreamType == MMP_AUDIO_AACStreamFormatMP4ADTS) {
		m_bADTS = MMP_TRUE;	
	}
}

CMmpDecoderAudio_AAC_PV::~CMmpDecoderAudio_AAC_PV()
{

}


MMP_RESULT CMmpDecoderAudio_AAC_PV::Open(MMP_U8* pStream, MMP_U32 nStreamSize)
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
	UInt32 memRequirements;
	Int err;
    
    mmpResult=CMmpDecoderAudio::Open(pStream, nStreamSize);
    if(mmpResult != MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderAudio_AAC_PV::Close] CMmpDecoderAudio::Close() \n\r")));
    }

	/* memory alloc */
	if(mmpResult == MMP_SUCCESS) {
		mConfig = new tPVMP4AudioDecoderExternal;
		if(mConfig == NULL) {
			mmpResult = MMP_FAILURE;
		}
	}

	if(mmpResult == MMP_SUCCESS) {
		memRequirements = PVMP4AudioDecoderGetMemRequirements();
		mDecoderBuf = malloc(memRequirements);
		if(mDecoderBuf == NULL) {
			mmpResult = MMP_FAILURE;
		}
	}

	/* init decoder */
	if(mmpResult == MMP_SUCCESS) {
		
		memset(mConfig, 0, sizeof(tPVMP4AudioDecoderExternal));
		mConfig->outputFormat = OUTPUTFORMAT_16PCM_INTERLEAVED;
		mConfig->aacPlusEnabled = 1;

		// The software decoder doesn't properly support mono output on
		// AACplus files. Always output stereo.
		mConfig->desiredChannels = 2;

		err = PVMP4AudioDecoderInitLibrary(mConfig, mDecoderBuf);
		if (err != MP4AUDEC_SUCCESS) {
			mmpResult = MMP_FAILURE;
		}
	}

    m_bConfigOK = MMP_FALSE;
	

    return mmpResult;
}

MMP_RESULT CMmpDecoderAudio_AAC_PV::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderAudio::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderAudio_AAC_PV::Close] CMmpDecoderAudio::Close() \n\r")));
        return mmpResult;
    }

	if(mConfig!=NULL) {
		delete mConfig;
		mConfig = NULL;
	}

	if(mDecoderBuf!=NULL) {
		free(mDecoderBuf);
		mDecoderBuf = NULL;
	}

    return MMP_SUCCESS;
}

MMP_U32 CMmpDecoderAudio_AAC_PV::GetAudioSampleRate() {
	
	MMP_U32 samplerate = 0;

	if(mConfig!=NULL) {
		samplerate = mConfig->samplingRate;
	}

	return samplerate;
		 
}
	
MMP_U32 CMmpDecoderAudio_AAC_PV::GetAudioChannelCount() {

	MMP_U32 ch_count = 0;

	if(mConfig!=NULL) {
		ch_count = mConfig->desiredChannels;
	}

	return ch_count;
}

MMP_RESULT CMmpDecoderAudio_AAC_PV::Flush() {

    if(mDecoderBuf != NULL) {
        PVMP4AudioDecoderResetBuffer(mDecoderBuf);
    }
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderAudio_AAC_PV::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    Int err;

    /* init decoder dsi */
	mConfig->pInputBuffer = pStream;
    mConfig->inputBufferCurrentLength = nStreamSize;
    mConfig->inputBufferMaxLength = 0;


	err = PVMP4AudioDecoderConfig(mConfig, mDecoderBuf);
    if (err != MP4AUDEC_SUCCESS) {
        //mSignalledError = true;
        //notify(OMX_EventError, OMX_ErrorUndefined, err, NULL);
        //return;
		mmpResult = MMP_FAILURE;
    }
    else {
        m_bConfigOK = MMP_TRUE;
    }

	mInputBufferCount++;

    return mmpResult;
}

MMP_RESULT CMmpDecoderAudio_AAC_PV::DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult) {
	
	MMP_RESULT mmpResult = MMP_SUCCESS;
	size_t adtsHeaderSize = 0;
	MMP_BOOL protectionAbsent;
	MMP_U8 *adtsHeader;
	MMP_U32 aac_frame_length;
	Int32 prevSamplingRate;
	Int decoderErr = MP4AUDEC_INVALID_FRAME;

	MMP_U8 *pStreamIn = pMediaSample->pAu;
	MMP_U8 *pStreamOut = (MMP_U8 *)pDecResult->uiDecodedBufferLogAddr[0];
	MMP_U32 nStreamInSize = (MMP_U32)pMediaSample->uiAuSize;

	pDecResult->bImage = MMP_FALSE;
	pDecResult->bReconfig = MMP_FALSE;
	pDecResult->uiDecodedSize = 0;

    if(m_bConfigOK == MMP_FALSE) {
        return this->DecodeDSI(pMediaSample->pAu, pMediaSample->uiAuSize);
    }

	//mConfig->pInputBuffer = inHeader->pBuffer + inHeader->nOffset;
    //mConfig->inputBufferCurrentLength = inHeader->nFilledLen;

	if(m_bADTS == MMP_TRUE) {
	
		adtsHeader = pStreamIn;

		if(nStreamInSize < 7) {
			mmpResult = MMP_FAILURE;
		}
		else {
	
			protectionAbsent = (adtsHeader[1] & 1);

			aac_frame_length =  ((adtsHeader[3] & 3) << 11)
								| (adtsHeader[4] << 3)
								| (adtsHeader[5] >> 5);

			if(nStreamInSize < aac_frame_length) {
				mmpResult = MMP_FAILURE;
			}
			else {
			
				adtsHeaderSize = (protectionAbsent ? 7 : 9);

                mConfig->pInputBuffer = (UChar *)adtsHeader + adtsHeaderSize;
                mConfig->inputBufferCurrentLength =  aac_frame_length - adtsHeaderSize;
                    //inHeader->nOffset += adtsHeaderSize;
                    //inHeader->nFilledLen -= adtsHeaderSize;
			}

		}
	}
	else {
	
		mConfig->pInputBuffer = pStreamIn;
        mConfig->inputBufferCurrentLength = nStreamInSize;
	}

	if(mmpResult == MMP_SUCCESS) {

		mConfig->inputBufferMaxLength = 0;
		mConfig->inputBufferUsedLength = 0;
		mConfig->remainderBits = 0;

		mConfig->pOutputBuffer =   reinterpret_cast<Int16 *>(pStreamOut);
		mConfig->pOutputBuffer_plus = &mConfig->pOutputBuffer[2048];
		mConfig->repositionFlag = false;

		prevSamplingRate = mConfig->samplingRate;
		decoderErr = PVMP4AudioDecodeFrame(mConfig, mDecoderBuf);

	}

	/*
         * AAC+/eAAC+ streams can be signalled in two ways: either explicitly
         * or implicitly, according to MPEG4 spec. AAC+/eAAC+ is a dual
         * rate system and the sampling rate in the final output is actually
         * doubled compared with the core AAC decoder sampling rate.
         *
         * Explicit signalling is done by explicitly defining SBR audio object
         * type in the bitstream. Implicit signalling is done by embedding
         * SBR content in AAC extension payload specific to SBR, and hence
         * requires an AAC decoder to perform pre-checks on actual audio frames.
         *
         * Thus, we could not say for sure whether a stream is
         * AAC+/eAAC+ until the first data frame is decoded.
         */

	if (decoderErr == MP4AUDEC_SUCCESS && mInputBufferCount <= 2) {
	
        MMPDEBUGMSG(1, ("audio/extended audio object type: %d + %d",  mConfig->audioObjectType, mConfig->extendedAudioObjectType));
        MMPDEBUGMSG(1, ("aac+ upsampling factor: %d desired channels: %d",   mConfig->aacPlusUpsamplingFactor, mConfig->desiredChannels));

		if(mInputBufferCount == 1) {
		
			mUpsamplingFactor = mConfig->aacPlusUpsamplingFactor;

			if (mConfig->samplingRate != prevSamplingRate) {
			
					// We'll hold onto the input buffer and will decode
                    // it again once the output port has been reconfigured.

                    // We're going to want to revisit this input buffer, but
                    // may have already advanced the offset. Undo that if
                    // necessary.
#if 0
                    inHeader->nOffset -= adtsHeaderSize;
                    inHeader->nFilledLen += adtsHeaderSize;

                    notify(OMX_EventPortSettingsChanged, 1, 0, NULL);
                    mOutputPortSettingsChange = AWAITING_DISABLED;
                    return;
#else

				pDecResult->bReconfig = MMP_TRUE;
				pDecResult->uiAuUsedByte = adtsHeaderSize;
				 
#endif
			}
		}
		else {  // mInputBufferCount == 2
		
			if( (mConfig->extendedAudioObjectType == MP4AUDIO_AAC_LC) ||
                (mConfig->extendedAudioObjectType == MP4AUDIO_LTP) ) {
                    
					if (mUpsamplingFactor == 2) {
                        // The stream turns out to be not aacPlus mode anyway
                        MMPDEBUGMSG(1, ("Disable AAC+/eAAC+ since extended audio object type is %d",  mConfig->extendedAudioObjectType));
                        mConfig->aacPlusEnabled = 0;
                    }

            } 
			else {
                if (mUpsamplingFactor == 1) {
                    // aacPlus mode does not buy us anything, but to cause
                    // 1. CPU load to increase, and
                    // 2. a half speed of decoding
                    MMPDEBUGMSG(1, ("Disable AAC+/eAAC+ since upsampling factor is 1"));
                    mConfig->aacPlusEnabled = 0;
                }
            }
		}

	}

	if(mmpResult == MMP_SUCCESS) {
	
		size_t numOutBytes =  mConfig->frameLength * sizeof(int16_t) * mConfig->desiredChannels;

		if (decoderErr == MP4AUDEC_SUCCESS) {
            //CHECK_LE(mConfig->inputBufferUsedLength, inHeader->nFilledLen);

            //inHeader->nFilledLen -= mConfig->inputBufferUsedLength;
            //inHeader->nOffset += mConfig->inputBufferUsedLength;
			pDecResult->uiAuUsedByte = mConfig->inputBufferUsedLength;

			if (mUpsamplingFactor == 2) {
                if (mConfig->desiredChannels == 1) {
                    memcpy(&mConfig->pOutputBuffer[1024],
                           &mConfig->pOutputBuffer[2048],
                           numOutBytes * 2);
                }
                numOutBytes *= 2;
            }

			pDecResult->uiDecodedSize = numOutBytes;
			pDecResult->uiAudioSampleRate = mConfig->samplingRate;
			pDecResult->uiAudioFrameCount = mConfig->frameLength*mUpsamplingFactor;
            
            //outHeader->nTimeStamp =
            //    mAnchorTimeUs
            //        + (mNumSamplesOutput * 1000000ll) / mConfig->samplingRate;

            //mNumSamplesOutput += mConfig->frameLength * mUpsamplingFactor;

            //outInfo->mOwnedByUs = false;
            //outQueue.erase(outQueue.begin());
            //outInfo = NULL;
            //notifyFillBufferDone(outHeader);
            //outHeader = NULL;


            mInputBufferCount++;
        }
		else {
		
			mmpResult = MMP_FAILURE;
		}

	}

	return mmpResult;
}