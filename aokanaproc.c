#include <stdlib.h>
#include <string.h>
#include <webp/decode.h>
#ifdef WITH_OPUS
#include <opusfile.h>
#define AOKANA_PCM_BUFFER 32000
#endif

// Exporting stuff
#if defined(_MSC_VER)
    #define EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
    #define EXPORT __attribute__((visibility("default")))
#else
    #pragma error Unknown export method
#endif

EXPORT int decodewebp(char* rawdata, size_t rawlen, char* outData, size_t outlen, int stride)
{
	if (rawdata == NULL || rawlen == 0)
	{
		return 0;
	}

	if (outData)
	{
		return WebPDecodeRGBAInto(rawdata, rawlen, outData, outlen, stride) != 0;
	}

	int width, height;
	if ( WebPGetInfo(rawdata, rawlen, &width, &height) )
	{
		return (width << 16) | height;
	}

	return 0;
}

#ifdef WITH_OPUS

typedef struct
{
	OggOpusFile* opus_file;
	int samples;
	int current_pcm_offset;
	int frequency;
	int channels;
	int samples_left_to_read;
	int sample_copy_offset;
	float* pcm_buffer;
} AokanaOpusDecoder;

EXPORT AokanaOpusDecoder* opdec_load(unsigned char* data, int len)
{
	int error;
	OggOpusFile* opus_file = op_open_memory(data, len, &error);
	if(opus_file == NULL)
	{
		return NULL;
	}

	const OpusHead* opus_head = op_head(opus_file, 0);

	AokanaOpusDecoder* aokana_opus_decoder = malloc(32 * opus_head->channel_count);
	aokana_opus_decoder->opus_file = opus_file;
	aokana_opus_decoder->current_pcm_offset = 0;
	aokana_opus_decoder->samples = op_pcm_total(opus_file, 0);
	aokana_opus_decoder->frequency = opus_head->input_sample_rate;
	aokana_opus_decoder->channels = opus_head->channel_count;
	aokana_opus_decoder->samples_left_to_read = 0;
	aokana_opus_decoder->sample_copy_offset = 0;
	aokana_opus_decoder->pcm_buffer = malloc(AOKANA_PCM_BUFFER);
	return aokana_opus_decoder;
}

EXPORT void opdec_free(AokanaOpusDecoder* decptr)
{
	if ( decptr )
	{
		if ( decptr->pcm_buffer )
		{
			free(decptr->pcm_buffer);
			decptr->pcm_buffer = NULL;
		}
		if ( decptr->opus_file )
		{
			op_free(decptr->opus_file);
			decptr->opus_file = NULL;
		}
	}
}

EXPORT int opdec_info(AokanaOpusDecoder* decptr, int* samples, int* freq, int* channels)
{
	if (decptr == NULL)
	{
		return 1;
	}
	*samples = decptr->samples;
	*freq = decptr->frequency;
	*channels = decptr->channels;
	return 0;
}

EXPORT int opdec_seek(AokanaOpusDecoder* decptr, int position)
{
	if (decptr == NULL)
	{
		return 0;
	}
	decptr->current_pcm_offset = position;
	decptr->samples_left_to_read = 0;
	return op_pcm_seek(decptr->opus_file, position) != 0;
}

EXPORT int opdec_read(AokanaOpusDecoder* decptr, float *pcmdata, int length)
{
	if(decptr == NULL)
	{
		return 0;
	}

	int total_samples_to_be_read = decptr->samples - decptr->current_pcm_offset;

	if(total_samples_to_be_read > length)
	{
		total_samples_to_be_read = length;
	}

	if(total_samples_to_be_read <= 0)
	{
		return 0;
	}

	int
		total_read_samples = 0,
		samples_to_copy = decptr->samples_left_to_read,
		channels = decptr->channels
	;
	float* pcm_buffer = decptr->pcm_buffer;
	for(;;)
	{
		int sample_copy_offset;
		if(samples_to_copy > 0)
		{
			sample_copy_offset = decptr->sample_copy_offset;
		}
		else
		{
			samples_to_copy = op_read_float(decptr->opus_file, pcm_buffer, 100, 0);
			if ( samples_to_copy <= 0 )
			{
				decptr->samples_left_to_read = 0;
				return total_read_samples;
			}
			decptr->samples_left_to_read = samples_to_copy;
			pcm_buffer = decptr->pcm_buffer;
			sample_copy_offset = 0;
			decptr->sample_copy_offset = 0;
		}

		if ( samples_to_copy > total_samples_to_be_read )
			samples_to_copy = total_samples_to_be_read;

		total_samples_to_be_read -= samples_to_copy;
		memcpy(pcmdata, &pcm_buffer[channels * sample_copy_offset], 4 * channels * samples_to_copy);
		decptr->current_pcm_offset += samples_to_copy;
		decptr->sample_copy_offset += samples_to_copy;
		total_read_samples += samples_to_copy;
		pcmdata += samples_to_copy * channels;
		decptr->samples_left_to_read -= samples_to_copy;
		samples_to_copy = decptr->samples_left_to_read;
		if ( total_samples_to_be_read <= 0 )
			return total_read_samples;
	}
}

#endif