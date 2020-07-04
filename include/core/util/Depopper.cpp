/*
 * Depopper.cpp
 *
 *  Created on: 4 июл. 2020 г.
 *      Author: sadko
 */

#include <core/util/Depopper.h>

namespace lsp
{
    
    Depopper::Depopper()
    {
        init();
    }
    
    Depopper::~Depopper()
    {
    }

    void Depopper::init()
    {
        nSampleRate     = DEFAULT_SAMPLE_RATE;
        fFadeTime       = 50.0f;
        fThreshold      = GAIN_AMP_M_80_DB;
        fAttack         = 10.0f;
        fRelease        = 10.0f;
        fEnvelope       = 0.0f;
        fClip           = 1.0f;
        fClipStart      = 0.0f;
        fClipInc        = 0.0f;
        nState          = ST_CLOSED;
        bReconfigure    = true;

        nFadeSamples    = 0;
        fTauAttack      = 0.0f;
        fTauRelease     = 0.0f;
    }

    void Depopper::reconfigure()
    {
        if (!bReconfigure)
            return;

        float att           = millis_to_samples(nSampleRate, fAttack);
        float rel           = millis_to_samples(nSampleRate, fRelease);
        float fade          = millis_to_samples(nSampleRate, fRelease);
        float thresh        = fabs(fThreshold);

        if (thresh > 1.0f)
            thresh              = 1.0f;
        if (att < 1.0f)
            att                 = 1.0f;
        if (rel < 1.0f)
            rel                 = 1.0f;

        fAttack             = 1.0f - expf(logf(1.0f - M_SQRT1_2) / att);
        fRelease            = 1.0f - expf(logf(1.0f - M_SQRT1_2) / rel);
        nFadeSamples        = fade;
        fClipStart          = thresh;
        fClipInc            = (1.0f - fClipStart) / fade;

        bReconfigure        = false;
    }

    size_t Depopper::set_sample_rate(size_t sr)
    {
        size_t old      = nSampleRate;
        if (old == sr)
            return old;

        nSampleRate     = sr;
        bReconfigure    = true;
        return old;
    }

    float Depopper::set_fade_time(float time)
    {
        float old       = fFadeTime;
        if (old == time)
            return old;

        fFadeTime       = time;
        bReconfigure    = true;

        return old;
    }

    float Depopper::set_threshold(float thresh)
    {
        float old       = fThreshold;
        if (old == thresh)
            return old;

        fThreshold      = thresh;
        bReconfigure    = true;

        return old;
    }

    float Depopper::set_attack(float attack)
    {
        float old       = fAttack;
        if (old == attack)
            return old;

        fAttack         = attack;
        bReconfigure    = true;

        return old;
    }

    float Depopper::set_release(float release)
    {
        float old       = fRelease;
        if (old == release)
            return old;

        fRelease        = release;
        bReconfigure    = true;

        return old;
    }

    void Depopper::process(float *dst, const float *src, size_t count)
    {
        // Reconfigure if needed
        reconfigure();

        // Process samples
        for (size_t i=0; i<count; ++i)
        {
            float x         = fabs(src[i]) - fEnvelope;
            fEnvelope      += (x > 0) ? fTauAttack * x : fTauRelease * x;

            // Produce output gain
            switch (nState)
            {
                case ST_CLOSED:
                    dst[i]      = 1.0f; // Normal gain
                    if (fEnvelope < fThreshold)
                        break;

                    fClip       = fabs(fThreshold);
                    nState      = ST_FADING;
                    break;

                case ST_FADING:
                    // Compute gain
                    dst[i]      = (fEnvelope <= fClip) ? 1.0f : fClip / fEnvelope;

                    // Fall-off below threshold ?
                    if (fEnvelope < fThreshold)
                    {
                        nState      = ST_CLOSED;
                        break;
                    }

                    // Fade-in the output gain
                    fClip      += fClipInc;
                    if (fClip >= 1.0f)
                        nState      = ST_OPENED;
                    break;

                case ST_OPENED:
                    dst[i]      = 1.0f; // Normal gain

                    // Fall-off before threshold ?
                    if (fEnvelope < fThreshold)
                        nState      = ST_CLOSED;

                    break;
                default:
                    dst[i]      = 1.0f;
                    break;
            }
        }
    }

} /* namespace lsp */
