#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

#include "sdlincludes.h"


SDL_AudioDeviceID audioid = 0;

#define rnd(n) (rand()%(n+1))

float frnd(float range)
{
    return (float)rnd(10000)/10000*range;
}

int wave_type;

float p_base_freq;
float p_freq_limit;
float p_freq_ramp;
float p_freq_dramp;
float p_duty;
float p_duty_ramp;

float p_vib_strength;
float p_vib_speed;
float p_vib_delay;

float p_env_attack;
float p_env_sustain;
float p_env_decay;
float p_env_punch;

bool filter_on;
float p_lpf_resonance;
float p_lpf_freq;
float p_lpf_ramp;
float p_hpf_freq;
float p_hpf_ramp;

float p_pha_offset;
float p_pha_ramp;

float p_repeat_speed;

float p_arp_speed;
float p_arp_mod;

float master_vol=0.05f;

float sound_vol=0.5f;


bool playing_sample=false;
int phase;
double fperiod;
double fmaxperiod;
double fslide;
double fdslide;
int period;
float square_duty;
float square_slide;
int env_stage;
int env_time;
int env_length[3];
float env_vol;
float fphase;
float fdphase;
int iphase;
float phaser_buffer[1024];
int ipp;
float noise_buffer[32];
float fltp;
float fltdp;
float fltw;
float fltw_d;
float fltdmp;
float fltphp;
float flthp;
float flthp_d;
float vib_phase;
float vib_speed;
float vib_amp;
int rep_time;
int rep_limit;
int arp_time;
int arp_limit;
double arp_mod;

float* vselected=NULL;
int vcurbutton=-1;

int wav_bits=16;
int wav_freq=44100;

int file_sampleswritten;
float filesample=0.0f;
int fileacc=0;

void ResetParams()
{
    wave_type=0;

    p_base_freq=0.3f;
    p_freq_limit=0.0f;
    p_freq_ramp=0.0f;
    p_freq_dramp=0.0f;
    p_duty=0.0f;
    p_duty_ramp=0.0f;

    p_vib_strength=0.0f;
    p_vib_speed=0.0f;
    p_vib_delay=0.0f;

    p_env_attack=0.0f;
    p_env_sustain=0.3f;
    p_env_decay=0.4f;
    p_env_punch=0.0f;

    filter_on=false;
    p_lpf_resonance=0.0f;
    p_lpf_freq=1.0f;
    p_lpf_ramp=0.0f;
    p_hpf_freq=0.0f;
    p_hpf_ramp=0.0f;

    p_pha_offset=0.0f;
    p_pha_ramp=0.0f;

    p_repeat_speed=0.0f;

    p_arp_speed=0.0f;
    p_arp_mod=0.0f;
}

void fread_mem(void *dest, int s1, int s2, uchar *&file)
{
    memcpy(dest, file, s1 * s2);
    file += s1 * s2;
}

struct Sound
{
    bool sfxr;
    uchar *buf;
    size_t len;
};

map<string, Sound> sound_files;
Sound cursnd;
uchar *cursndpos;

SDL_AudioSpec playbackspec;

bool LoadSound(const char* filename, bool sfxr)
{
    auto it = sound_files.find(filename);
    Sound snd;

    if (it == sound_files.end())
    {
        uchar *buf;
        size_t len = 0;
        if (!(buf = LoadFile(filename, &len)))
            return false;

        if (sfxr)
        {
            snd.buf = buf;
            snd.len = len;
        }
        else
        {
            SDL_AudioSpec wav_spec;
            Uint32 wav_len;
            auto wav_spec_ret = SDL_LoadWAV_RW(SDL_RWFromMem(buf, len), true, &wav_spec, &snd.buf, &wav_len);

            free(buf);

            if (!wav_spec_ret)
                return false;

            snd.len = wav_len;

            SDL_AudioCVT  wav_cvt;
            int ret = SDL_BuildAudioCVT(&wav_cvt,
                        wav_spec.format, wav_spec.channels, wav_spec.freq,
                        playbackspec.format, playbackspec.channels, playbackspec.freq);

            if (ret < 0)
            {
                SDL_FreeWAV(snd.buf);
                return false;
            }

            if (ret)
            {
                wav_cvt.buf = (uchar *)malloc(snd.len * wav_cvt.len_mult);
                wav_cvt.len = snd.len;
                memcpy(wav_cvt.buf, snd.buf, snd.len);

                SDL_FreeWAV(snd.buf);

                SDL_ConvertAudio(&wav_cvt);

                snd.buf = wav_cvt.buf;
                snd.len = size_t(wav_cvt.len * wav_cvt.len_ratio);  // caution: len is not size of buffer anymore, but size of valid data.. that works ok with SDL_FreeWAV/free
            }
        }

        snd.sfxr = sfxr;

        sound_files[filename] = snd;
    }
    else
    {
        snd = it->second;

        if (sfxr != snd.sfxr)   // wav file and sfxr file with same name? should be very rare :)
            return false;
    }

    if (sfxr)
    {
        uchar *file = snd.buf;

        int version = 0;
        fread_mem(&version, 1, sizeof(int), file);
        if(version!=102)
            return false;

        fread_mem(&wave_type, 1, sizeof(int), file);

        fread_mem(&sound_vol, 1, sizeof(float), file);

        fread_mem(&p_base_freq, 1, sizeof(float), file);
        fread_mem(&p_freq_limit, 1, sizeof(float), file);
        fread_mem(&p_freq_ramp, 1, sizeof(float), file);
        fread_mem(&p_freq_dramp, 1, sizeof(float), file);
        fread_mem(&p_duty, 1, sizeof(float), file);
        fread_mem(&p_duty_ramp, 1, sizeof(float), file);

        fread_mem(&p_vib_strength, 1, sizeof(float), file);
        fread_mem(&p_vib_speed, 1, sizeof(float), file);
        fread_mem(&p_vib_delay, 1, sizeof(float), file);

        fread_mem(&p_env_attack, 1, sizeof(float), file);
        fread_mem(&p_env_sustain, 1, sizeof(float), file);
        fread_mem(&p_env_decay, 1, sizeof(float), file);
        fread_mem(&p_env_punch, 1, sizeof(float), file);

        fread_mem(&filter_on, 1, sizeof(bool), file);
        fread_mem(&p_lpf_resonance, 1, sizeof(float), file);
        fread_mem(&p_lpf_freq, 1, sizeof(float), file);
        fread_mem(&p_lpf_ramp, 1, sizeof(float), file);
        fread_mem(&p_hpf_freq, 1, sizeof(float), file);
        fread_mem(&p_hpf_ramp, 1, sizeof(float), file);

        fread_mem(&p_pha_offset, 1, sizeof(float), file);
        fread_mem(&p_pha_ramp, 1, sizeof(float), file);
        fread_mem(&p_repeat_speed, 1, sizeof(float), file);
        fread_mem(&p_arp_speed, 1, sizeof(float), file);
        fread_mem(&p_arp_mod, 1, sizeof(float), file);
    }

    cursnd = snd;

    return true;
}

void ResetSample(bool restart)
{
    if(!restart)
        phase=0;
    fperiod=100.0/(p_base_freq*p_base_freq+0.001);
    period=(int)fperiod;
    fmaxperiod=100.0/(p_freq_limit*p_freq_limit+0.001);
    fslide=1.0-pow((double)p_freq_ramp, 3.0)*0.01;
    fdslide=-pow((double)p_freq_dramp, 3.0)*0.000001;
    square_duty=0.5f-p_duty*0.5f;
    square_slide=-p_duty_ramp*0.00005f;
    if(p_arp_mod>=0.0f)
        arp_mod=1.0-pow((double)p_arp_mod, 2.0)*0.9;
    else
        arp_mod=1.0+pow((double)p_arp_mod, 2.0)*10.0;
    arp_time=0;
    arp_limit=(int)(pow(1.0f-p_arp_speed, 2.0f)*20000+32);
    if(p_arp_speed==1.0f)
        arp_limit=0;
    if(!restart)
    {
        // reset filter
        fltp=0.0f;
        fltdp=0.0f;
        fltw=pow(p_lpf_freq, 3.0f)*0.1f;
        fltw_d=1.0f+p_lpf_ramp*0.0001f;
        fltdmp=5.0f/(1.0f+pow(p_lpf_resonance, 2.0f)*20.0f)*(0.01f+fltw);
        if(fltdmp>0.8f) fltdmp=0.8f;
        fltphp=0.0f;
        flthp=pow(p_hpf_freq, 2.0f)*0.1f;
        flthp_d=1.0f+p_hpf_ramp*0.0003f;
        // reset vibrato
        vib_phase=0.0f;
        vib_speed=pow(p_vib_speed, 2.0f)*0.01f;
        vib_amp=p_vib_strength*0.5f;
        // reset envelope
        env_vol=0.0f;
        env_stage=0;
        env_time=0;
        env_length[0]=(int)(p_env_attack*p_env_attack*100000.0f);
        env_length[1]=(int)(p_env_sustain*p_env_sustain*100000.0f);
        env_length[2]=(int)(p_env_decay*p_env_decay*100000.0f);

        fphase=pow(p_pha_offset, 2.0f)*1020.0f;
        if(p_pha_offset<0.0f) fphase=-fphase;
        fdphase=pow(p_pha_ramp, 2.0f)*1.0f;
        if(p_pha_ramp<0.0f) fdphase=-fdphase;
        iphase=abs((int)fphase);
        ipp=0;
        for(int i=0;i<1024;i++)
            phaser_buffer[i]=0.0f;

        for(int i=0;i<32;i++)
            noise_buffer[i]=frnd(2.0f)-1.0f;

        rep_time=0;
        rep_limit=(int)(pow(1.0f-p_repeat_speed, 2.0f)*20000+32);
        if(p_repeat_speed==0.0f)
            rep_limit=0;
    }
}

void SynthSample(int length, float* buffer, FILE* file)
{
    for(int i=0;i<length;i++)
    {
        if(!playing_sample)
            break;

        rep_time++;
        if(rep_limit!=0 && rep_time>=rep_limit)
        {
            rep_time=0;
            ResetSample(true);
        }

        // frequency envelopes/arpeggios
        arp_time++;
        if(arp_limit!=0 && arp_time>=arp_limit)
        {
            arp_limit=0;
            fperiod*=arp_mod;
        }
        fslide+=fdslide;
        fperiod*=fslide;
        if(fperiod>fmaxperiod)
        {
            fperiod=fmaxperiod;
            if(p_freq_limit>0.0f)
                playing_sample=false;
        }
        float rfperiod=(float)fperiod;
        if(vib_amp>0.0f)
        {
            vib_phase+=vib_speed;
            rfperiod=float(fperiod*(1.0+sin(vib_phase)*vib_amp));
        }
        period=(int)rfperiod;
        if(period<8) period=8;
        square_duty+=square_slide;
        if(square_duty<0.0f) square_duty=0.0f;
        if(square_duty>0.5f) square_duty=0.5f;
        // volume envelope
        env_time++;
        if(env_time>env_length[env_stage])
        {
            env_time=0;
            env_stage++;
            if(env_stage==3)
                playing_sample=false;
        }
        if(env_stage==0)
            env_vol=(float)env_time/env_length[0];
        if(env_stage==1)
            env_vol=1.0f+pow(1.0f-(float)env_time/env_length[1], 1.0f)*2.0f*p_env_punch;
        if(env_stage==2)
            env_vol=1.0f-(float)env_time/env_length[2];

        // phaser step
        fphase+=fdphase;
        iphase=abs((int)fphase);
        if(iphase>1023) iphase=1023;

        if(flthp_d!=0.0f)
        {
            flthp*=flthp_d;
            if(flthp<0.00001f) flthp=0.00001f;
            if(flthp>0.1f) flthp=0.1f;
        }

        float ssample=0.0f;
        for(int si=0;si<8;si++) // 8x supersampling
        {
            float sample=0.0f;
            phase++;
            if(phase>=period)
            {
//				phase=0;
                phase%=period;
                if(wave_type==3)
                    for(int i=0;i<32;i++)
                        noise_buffer[i]=frnd(2.0f)-1.0f;
            }
            // base waveform
            float fp=(float)phase/period;
            switch(wave_type)
            {
            case 0: // square
                if(fp<square_duty)
                    sample=0.5f;
                else
                    sample=-0.5f;
                break;
            case 1: // sawtooth
                sample=1.0f-fp*2;
                break;
            case 2: // sine
                sample=(float)sin(fp*2*PI);
                break;
            case 3: // noise
                sample=noise_buffer[phase*32/period];
                break;
            }
            // lp filter
            float pp=fltp;
            fltw*=fltw_d;
            if(fltw<0.0f) fltw=0.0f;
            if(fltw>0.1f) fltw=0.1f;
            if(p_lpf_freq!=1.0f)
            {
                fltdp+=(sample-fltp)*fltw;
                fltdp-=fltdp*fltdmp;
            }
            else
            {
                fltp=sample;
                fltdp=0.0f;
            }
            fltp+=fltdp;
            // hp filter
            fltphp+=fltp-pp;
            fltphp-=fltphp*flthp;
            sample=fltphp;
            // phaser
            phaser_buffer[ipp&1023]=sample;
            sample+=phaser_buffer[(ipp-iphase+1024)&1023];
            ipp=(ipp+1)&1023;
            // final accumulation and envelope application
            ssample+=sample*env_vol;
        }
        ssample=ssample/8*master_vol;

        ssample*=2.0f*sound_vol;

        if(buffer!=NULL)
        {
            if(ssample>1.0f) ssample=1.0f;
            if(ssample<-1.0f) ssample=-1.0f;
            *buffer++=ssample;
        }
        if(file!=NULL)
        {
            // quantize depending on format
            // accumulate/count to accomodate variable sample rate?
            ssample*=4.0f; // arbitrary gain to get reasonable output volume...
            if(ssample>1.0f) ssample=1.0f;
            if(ssample<-1.0f) ssample=-1.0f;
            filesample+=ssample;
            fileacc++;
            if(wav_freq==44100 || fileacc==2)
            {
                filesample/=fileacc;
                fileacc=0;
                if(wav_bits==16)
                {
                    short isample=(short)(filesample*32000);
                    fwrite(&isample, 1, 2, file);
                }
                else
                {
                    uchar isample=(uchar)(filesample*127+128);
                    fwrite(&isample, 1, 1, file);
                }
                filesample=0.0f;
            }
            file_sampleswritten++;
        }
    }
}

static void SDLAudioCallback(void *userdata, Uint8 *stream, int len)
{
    if (playing_sample)
    {
        if (cursnd.sfxr)
        {
            uint l = len/2;
            float *fbuf = new float[l];
            memset(fbuf, 0, sizeof(float)*l);
            SynthSample(l, fbuf, NULL);
            while (l--)
            {
                float f = fbuf[l];
                if (f < -1.0) f = -1.0;
                if (f > 1.0) f = 1.0;
                ((Sint16*)stream)[l] = (Sint16)(f * 32767);
            }
            delete[] fbuf;
        }
        else
        {
            size_t amount = min((size_t)len, cursnd.len - (cursndpos - cursnd.buf));
            memcpy(stream, cursndpos, amount);
            memset(stream + amount, 0, len - amount);
            cursndpos += amount;
            if (cursndpos == cursnd.buf + cursnd.len)
                playing_sample = false;
        }
    }
    else
    {
        memset(stream, 0, len);
        SDL_PauseAudioDevice(audioid, 1);
    }
}

bool SDLSoundInit()
{
    if (audioid) return true;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO))
        return false;

    playbackspec.freq = 44100;
    playbackspec.format = AUDIO_S16SYS;
    playbackspec.channels = 1;
    playbackspec.samples = 512;
    playbackspec.callback = SDLAudioCallback;
    playbackspec.userdata = NULL;
    audioid = SDL_OpenAudioDevice(NULL, 0, &playbackspec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);

    return !!audioid;
}

void SDLSoundClose()
{
    if (audioid) SDL_PauseAudioDevice(audioid, 1);

    for (auto &it : sound_files)
    {
        Sound &snd = it.second;
        if (snd.sfxr) free(snd.buf);
        else SDL_FreeWAV(snd.buf);
    }
    sound_files.clear();

    if (audioid) SDL_CloseAudioDevice(audioid);

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    audioid = 0;
}

bool SDLPlaySound(const char *filename, bool sfxr)
{
    ResetParams();
    bool ok = SDLSoundInit() && LoadSound(filename, sfxr);
    if (ok)
    {
        if (cursnd.sfxr)
        {
            ResetSample(false);
        }
        else
        {
            cursndpos = cursnd.buf;
        }
        SDL_PauseAudioDevice(audioid, 0);
        playing_sample=true;
    }
    return ok;
}
